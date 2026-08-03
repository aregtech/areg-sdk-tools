/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        tests/sm/SMDirtyMarkTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The modified mark of a document follows the first keystroke, and the save point.
 *
 *               The editor fields give their text to the document when they lose the focus, so
 *               for a while the author has changed the document and the title still says it is
 *               saved. What is proved here, on a real document and a real editor panel:
 *
 *                 1. The first keystroke reports the document as changed, before anything has
 *                    reached the undo history.
 *                 2. Typing the original text back withdraws it again.
 *                 3. Text a page puts into the field the caret sits in is not an author's edit.
 *                 4. A save clears the mark with the caret still in the field.
 *                 5. After the save the mark follows the distance to the save point: it comes
 *                    back on the next change, goes away on undoing back to that point, and
 *                    comes back on undoing past it.
 *
 *               Run as: lusan_sm_dirty_mark <TrafficLight.fsml>
 *
 ************************************************************************/

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/sm/SMPropertiesPanel.hpp"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int gChecks  { 0 };
    int gFailures{ 0 };

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////

namespace
{
    void typeText(QWidget* field, const QString& text)
    {
        for (const QChar ch : text)
        {
            QKeyEvent press(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, QString(ch));
            QApplication::sendEvent(field, &press);
        }

        QApplication::processEvents();
    }

    void pressBackspace(QWidget* field, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            QKeyEvent press(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
            QApplication::sendEvent(field, &press);
        }

        QApplication::processEvents();
    }

    void focus(QWidget* field)
    {
        field->setFocus(Qt::OtherFocusReason);
        QApplication::processEvents();
    }

    //!< The first ordinary state of the document: a Start carries no description to type into.
    uint32_t firstStateId(StateMachineData& data)
    {
        for (const SMStateEntry* state : data.getStates().getElements())
        {
            if ((state != nullptr) && (state->getKind() == SMStateEntry::eStateKind::Normal))
            {
                return state->getId();
            }
        }

        return 0u;
    }
}

//////////////////////////////////////////////////////////////////////////
// The proof
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDirtyMarkFollowsTypingAndSavePoint(const QString& documentPath, const QString& savePath)
    {
        StateMachineModel model;
        if (model.loadFromFile(documentPath) == false)
        {
            std::printf("  [FAIL] cannot read %s\n", qPrintable(documentPath));
            ++gFailures;
            return;
        }

        QWidget window;
        QVBoxLayout* layout = new QVBoxLayout(&window);
        SMPropertiesPanel* panel = new SMPropertiesPanel(model, &window);
        layout->addWidget(panel);

        // What the document window does with the two answers: the history stands away from the
        // save point, or a field holds text the document has not been given yet.
        PendingEditWatcher watcher(model.getNotifier());
        bool marked{ false };
        auto refresh = [&]() { marked = (model.isDirty() || watcher.hasPendingEdit()); };
        QObject::connect(&model, &StateMachineModel::signalDirtyChanged, &window, [&](bool) { refresh(); });
        QObject::connect(&watcher, &PendingEditWatcher::signalPendingEditChanged, &window, [&](bool) { refresh(); });

        window.show();
        window.activateWindow();
        QApplication::processEvents();

        const uint32_t stateId = firstStateId(model.getData());
        CHECK(stateId != 0);
        model.getSelectionModel().select(stateId);
        QApplication::processEvents();

        QPlainTextEdit* description = panel->findChild<QPlainTextEdit*>(QStringLiteral("smStateDescription"));
        CHECK(description != nullptr);
        if ((stateId == 0) || (description == nullptr))
        {
            return;
        }

        const QString original = description->toPlainText();
        refresh();
        CHECK(marked == false);         // a freshly opened document is not modified

        // 1. The first keystroke, long before anything reaches the undo history.
        focus(description);
        CHECK(QApplication::focusWidget() == description);
        typeText(description, QStringLiteral("Xy"));
        CHECK(description->toPlainText() != original);
        CHECK(marked);
        CHECK(model.getUndoStack().isClean());  // nothing committed yet -- this is the whole point

        // 2. Typing it back is not a change any more.
        pressBackspace(description, 2);
        CHECK(description->toPlainText() == original);
        CHECK(marked == false);

        // 3. Text the panel puts there is the document's own text, not an edit.
        typeText(description, QStringLiteral("Zz"));
        CHECK(marked);
        description->setPlainText(original + QStringLiteral(" (from the document)"));
        QApplication::processEvents();
        CHECK(marked == false);
        description->setPlainText(original);
        QApplication::processEvents();

        // The field hands its text over when the focus leaves it, and that is a command.
        typeText(description, QStringLiteral("Watched"));
        CHECK(marked);
        focus(panel->stateNameEdit());
        CHECK(model.getUndoStack().isClean() == false);
        CHECK(watcher.hasPendingEdit() == false);
        CHECK(marked);

        // 4. A save with the caret inside the box: the text goes into the file and the mark goes.
        focus(description);
        typeText(description, QStringLiteral("Saved"));
        CHECK(marked);
        panel->commitPendingEdits();    // what the document window does before it writes
        watcher.acceptPendingEdit();
        CHECK(model.saveToFile(savePath));
        refresh();
        CHECK(marked == false);
        CHECK(QApplication::focusWidget() == description);   // the caret never moved

        // 5. From here the mark is the distance to the save point.
        typeText(description, QStringLiteral("More"));
        CHECK(marked);                  // typed, not committed
        focus(panel->stateNameEdit());  // committed
        CHECK(marked);

        model.getUndoStack().undo();
        QApplication::processEvents();
        CHECK(model.getUndoStack().isClean());
        CHECK(marked == false);         // back at the point the file was written from

        model.getUndoStack().undo();
        QApplication::processEvents();
        CHECK(marked);                  // past it, so the document differs from the file again

        model.getUndoStack().redo();
        QApplication::processEvents();
        CHECK(marked == false);
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    if (argc < 2)
    {
        std::printf("Usage: %s <TrafficLight.fsml> [output directory]\n", argv[0]);
        return 2;
    }

    const QString documentPath = QString::fromLocal8Bit(argv[1]);
    const QString outDir = (argc >= 3) ? QString::fromLocal8Bit(argv[2]) : QDir::tempPath();
    QDir().mkpath(outDir);
    const QString savePath = outDir + QDir::separator() + QStringLiteral("dirty-mark-save.fsml");
    QFile::remove(savePath);

    std::printf("=== the modified mark follows typing and the save point ===\n");
    testDirtyMarkFollowsTypingAndSavePoint(documentPath, savePath);
    QFile::remove(savePath);

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
