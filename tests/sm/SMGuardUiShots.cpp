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
 *  \file        tests/sm/SMGuardUiShots.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U3) UI verification harness: drives the REAL SMGuardBar and Methods
 *               page widgets programmatically (no input injection), checks the field
 *               invariants 23a/23b and the form-first parity at widget level, and saves
 *               QWidget::grab() screenshots (lens, grid, island, Methods page).
 *
 *               Usage: lusan_sm_guard_ui_shots <doc.fsml> [grab dir]
 *
 ************************************************************************/

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMGuardBar.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"
#include "lusan/view/sm/SMMappingGrid.hpp"
#include "lusan/view/sm/SMMethod.hpp"
#include "lusan/view/common/MethodListView.hpp"
#include "lusan/view/sm/SMOperandField.hpp"
#include "lusan/view/sm/SMStructureLens.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QTextCursor>
#include <QTreeWidget>
#include <cstdio>

/**
 * \class   ProbeField
 * \brief   Lifts the field's clipboard overrides to public, so the copy/paste legs of
 *          acceptance 23a are exercised directly on the real code paths -- the system
 *          clipboard is not reachable from a harness process on every desktop.
 **/
class ProbeField : public SMGuardField
{
public:
    using SMGuardField::SMGuardField;
    using SMGuardField::createMimeDataFromSelection;
    using SMGuardField::insertFromMimeData;
};

namespace
{
    int gChecks = 0;
    int gFailures = 0;

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    void checkEq(const QString& actual, const QString& expected, const char* what)
    {
        ++gChecks;
        if (actual != expected)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n         expected: %s\n         actual  : %s\n",
                        what, expected.toStdString().c_str(), actual.toStdString().c_str());
        }
    }

    //!< Spins the event loop for \p ms (deferred rebuilds, debounce, delayed commits).
    void pump(int ms)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < ms)
        {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
        }
    }

    void grab(QWidget* widget, const QString& dir, const char* name)
    {
        const QString path = dir + QLatin1Char('/') + QString::fromLatin1(name);
        if (widget->grab().save(path))
        {
            std::printf("  [SHOT] %s\n", path.toStdString().c_str());
        }
        else
        {
            std::printf("  [WARN] could not save %s\n", path.toStdString().c_str());
        }
    }

    //!< The first transition of the document (the demo machine has exactly one).
    uint32_t firstTransition(const StateMachineData& data)
    {
        for (const SMStateEntry* state : data.getStates().getElements())
        {
            if ((state != nullptr) && (state->getTransitions().getElementCount() > 0))
            {
                return state->getTransitions().getElements().at(0)->getId();
            }
        }

        return 0u;
    }

    //!< Installs guard \p text through the same command path the field commit uses.
    void setGuard(StateMachineModel& model, uint32_t transitionId, const QString& text)
    {
        SMGuard guard = SMGuardParser::parseToGuard(model.getData(), transitionId, text);
        model.getUndoStack().push(SMGuardCommands::setGuard(model.getData(), model.getNotifier(), transitionId, guard, QStringLiteral("set")));
    }

    QString guardText(const StateMachineModel& model, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = model.getData().findTransitionById(transitionId);
        return (transition != nullptr) ? SMGuardRender::guardText(model.getData(), transitionId, transition->getGuard()) : QString();
    }
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    QApplication app(argc, argv);
    if (argc < 2)
    {
        std::printf("usage: lusan_sm_guard_ui_shots <doc.fsml> [grab dir]\n");
        return 2;
    }

    const QString docPath = QString::fromLocal8Bit(argv[1]);
    const QString grabDir = (argc > 2) ? QString::fromLocal8Bit(argv[2]) : QStringLiteral(".");
    QDir().mkpath(grabDir);

    StateMachineModel model;
    if (model.loadFromFile(docPath) == false)
    {
        std::printf("cannot load %s\n", docPath.toStdString().c_str());
        return 2;
    }

    const uint32_t transId = firstTransition(model.getData());
    check(transId != 0u, "the demo document has a transition");
    if (transId == 0u)
    {
        return 1;
    }


    SMGuardBar bar(model);
    bar.resize(640, 520);
    bar.show();
    bar.setTransition(transId);
    pump(150);

    // ---- S5: the lens over the running example B ---------------------------
    std::printf("[ RUN  ] lens\n");
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));
    pump(300);
    check(bar.lens()->findChild<QWidget*>(QStringLiteral("smLensPill_0")) != nullptr, "lens renders the first clause pill");
    check(bar.lens()->findChild<QWidget*>(QStringLiteral("smLensJoin_root")) != nullptr, "lens renders the root joiner");
    grab(&bar, grabDir, "u3-lens-open.png");

    // ---- 23a: the island token is ONE character to caret/selection/clipboard --
    std::printf("[ RUN  ] islandAtomicity\n");
    const QString body = QStringLiteral(" const uint16_t effective = count + 2u;\n return effective >= MIN_WAITING; ");
    const QString islandText = QStringLiteral("WalkRequested && {") + body + QStringLiteral("}");
    setGuard(model, transId, islandText);
    pump(300);

    SMGuardField* field = bar.field();
    check(field->islandCount() == 1, "the island folded to one token");
    checkEq(field->islandBody(0), body, "the token carries the body byte-exact");

    // Caret: crossing the token is one step.
    const QString docText = field->toPlainText();
    const int tokenAt = docText.indexOf(QChar(0xFFFC));
    check(tokenAt >= 0, "the token is one character in the document");
    QTextCursor cursor = field->textCursor();
    cursor.setPosition(tokenAt);
    field->setTextCursor(cursor);
    cursor.movePosition(QTextCursor::Right);
    check(cursor.position() == tokenAt + 1, "caret crosses the island in ONE step");

    // Selection + copy: the mime data built from the selection expands the island
    // byte-exact (the real createMimeDataFromSelection override, no system clipboard).
    ProbeField probe(model);
    probe.resize(600, 60);
    probe.show();
    probe.setTransition(transId);
    pump(300);
    check(probe.islandCount() == 1, "the probe field folded the island");

    probe.selectAll();
    QMimeData* copied = probe.createMimeDataFromSelection();
    check(copied != nullptr, "copy builds mime data");
    if (copied != nullptr)
    {
        checkEq(copied->text(), islandText, "copy round-trips the island byte-exact");
    }

    // Paste: the balanced block re-enters as a token with the same body (the real
    // insertFromMimeData override).
    probe.selectAll();
    probe.textCursor().removeSelectedText();
    check(probe.islandCount() == 0, "the probe field was cleared");
    if (copied != nullptr)
    {
        probe.insertFromMimeData(copied);
        pump(250);
        check(probe.islandCount() == 1, "paste recreated the island token");
        checkEq(probe.islandBody(0), body, "pasted body is byte-exact");
        checkEq(probe.committableText(), islandText, "pasted committable text is byte-exact");
        delete copied;
    }

    probe.hide();
    checkEq(field->committableText(), islandText, "committable text round-trips byte-exact");

    // ---- S4: the island editor -----------------------------------------------
    std::printf("[ RUN  ] islandEditor\n");
    bar.islandEditor()->openFor(model, transId, 0, body);
    pump(150);
    check(bar.islandEditor()->isVisible(), "the island editor is open");
    grab(&bar, grabDir, "u3-island-open.png");
    bar.islandEditor()->hide();

    // ---- 23b: a grid edit changes the visible text; ONE undo restores both ----
    std::printf("[ RUN  ] gridTwoWay\n");
    setGuard(model, transId, QStringLiteral("HasWaiting(count)"));
    pump(300);

    bar.grid()->openFor(transId, {}, bar.mapToGlobal(QPoint(20, 200)));
    pump(150);
    check(bar.grid()->isVisible(), "the grid is open for the call");
    grab(bar.grid(), grabDir, "u3-grid-open.png");

    SMOperandField* operand = bar.grid()->findChild<SMOperandField*>(QStringLiteral("smMapGridField_0"));
    check(operand != nullptr, "the grid has the first operand field");
    if (operand != nullptr)
    {
        operand->setEditText(QStringLiteral("MIN_WAITING"));
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(operand->lineEdit(), &enter);
        pump(300);

        checkEq(guardText(model, transId), QStringLiteral("HasWaiting(MIN_WAITING)"), "grid edit rewrote the tree");
        checkEq(field->toPlainText(), QStringLiteral("HasWaiting(MIN_WAITING)"), "the field text visibly updated");

        model.getUndoStack().undo();
        pump(300);
        checkEq(guardText(model, transId), QStringLiteral("HasWaiting(count)"), "ONE undo restored the tree");
        checkEq(field->toPlainText(), QStringLiteral("HasWaiting(count)"), "and the visible field text");
    }

    bar.grid()->hide();

    // ---- form-first parity: the clause popover path equals the typed path -----
    std::printf("[ RUN  ] formFirstParity\n");
    setGuard(model, transId, QString());
    pump(300);
    field->appendClause(QStringLiteral("&&"), QStringLiteral("HasWaiting(count)"));
    pump(600);      // the visible-insert delay plus the deferred rebuild
    checkEq(guardText(model, transId), QStringLiteral("HasWaiting(count)"), "the popover-built clause committed");

    const SMTransitionEntry* transition = model.getData().findTransitionById(transId);
    SMGuardParser::Result typed = SMGuardParser::parse(model.getData(), transId, QStringLiteral("HasWaiting(count)"));
    check((transition != nullptr) && typed.resolved() && (typed.tree != nullptr)
          && transition->getGuard().isOk()
          && transition->getGuard().getTree()->equals(*typed.tree)
        , "the stored tree equals the typed-path tree");
    delete typed.tree;

    // ---- S14: the Methods page kind split -------------------------------------
    std::printf("[ RUN  ] methodsPage\n");
    SMMethod page(model.getMethodModel());
    page.resize(980, 620);
    page.show();
    pump(250);

    QTreeWidget* tree = page.getList()->ctrlTableList();
    bool sawHandler = false;
    bool sawLambda = false;
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
    {
        const QString kind = tree->topLevelItem(i)->text(1);
        sawHandler = sawHandler || (kind == QStringLiteral("h handler"));
        sawLambda = sawLambda || (kind == QStringLiteral("{} lambda"));
        if ((kind == QStringLiteral("h handler")) || (kind == QStringLiteral("{} lambda")))
        {
            tree->setCurrentItem(tree->topLevelItem(i));
        }
    }

    check(sawHandler, "the Methods page shows an 'h handler' condition");
    check(sawLambda, "the Methods page shows a '{} lambda' condition");
    pump(150);
    grab(&page, grabDir, "u3-methods-page.png");

    std::printf("Done: %d checks, %d failure%s\n", gChecks, gFailures, (gFailures == 1) ? "" : "s");
    return (gFailures == 0) ? 0 : 1;
}
