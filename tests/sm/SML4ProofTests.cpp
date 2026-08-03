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
 *  \file        tests/sm/SML4ProofTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Task L4 acceptance: opens the converted TrafficLight document, edits the internal
 *               transition of TRAFFIC_LIGHT_RED through BOTH routes the task asks for -- the state
 *               page's new Internal tab, and the `on <stimulus>` row inside the state box -- saves,
 *               reloads, and proves each change survived. Runs on the REAL platform plugin (not
 *               offscreen) so the grabbed PNGs carry real text beside the glyphs.
 *
 *               Usage: lusan_sm_l4_proof <TrafficLight.fsml> <output directory>
 *               The document is COPIED into the output directory and the copy is what is edited,
 *               so the source tree is never written to.
 *
 ************************************************************************/

#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMKindGlyph.hpp"
#include "lusan/view/sm/SMOperationsEditor.hpp"
#include "lusan/view/sm/SMAccordion.hpp"
#include "lusan/view/sm/SMInternalDialog.hpp"
#include "lusan/view/sm/SMInternalEditor.hpp"
#include "lusan/view/sm/SMSectionChrome.hpp"
#include "lusan/view/sm/SMPropertiesPanel.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QListWidget>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleFactory>
#include <QLabel>
#include <QSet>
#include <QToolButton>
#include <QUndoStack>
#include <QTabWidget>

#include <cstdio>

namespace
{
    int gChecks   { 0 };
    int gFailures { 0 };

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    QString gGrabDir;

    void grab(QWidget& widget, const char* name)
    {
        if (gGrabDir.isEmpty() == false)
        {
            widget.grab().save(gGrabDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
        }
    }

    //!< The whole mark vocabulary on one sheet, each drawn at the size a state-body row gives it
    //!< (\c SMKindGlyph::GlyphSize) and again magnified, so "no two constructs share a glyph" can
    //!< be read off a single picture rather than hunted for across boxes.
    void grabGlyphSheet(const char* name)
    {
        if (gGrabDir.isEmpty())
        {
            return;
        }

        struct Row
        {
            SMKindGlyph::eGlyph glyph;
            const char*         label;
        };

        const Row rows[]
        {
              { SMKindGlyph::eGlyph::Entry,      "Entry -- an entry operation" }
            , { SMKindGlyph::eGlyph::Do,         "Do -- the repeated activity" }
            , { SMKindGlyph::eGlyph::Internal,   "Internal -- an internal transition" }
            , { SMKindGlyph::exitGlyph(),        "Exit -- an exit operation" }
            , { SMKindGlyph::eGlyph::Trigger,    "Trigger -- a trigger stimulus" }
            , { SMKindGlyph::eGlyph::Event,      "Event -- an event stimulus or send" }
            , { SMKindGlyph::eGlyph::TimerStart, "Timer start -- a timer stimulus or start" }
            , { SMKindGlyph::eGlyph::TimerStop,  "Timer stop" }
            , { SMKindGlyph::eGlyph::Action,     "Action -- the operation it runs" }
        };

        const int count   = static_cast<int>(sizeof(rows) / sizeof(rows[0]));
        const int rowH    = 62;
        const int magnify = 4;
        QImage image(QSize(470, (count * rowH) + 20), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QColor ink(0x1A, 0x1A, 0x1A);
        for (int i = 0; i < count; ++i)
        {
            const double y = 10.0 + (i * rowH);
            SMKindGlyph::paint(painter, QRectF(14.0, y + ((rowH - 14) / 2.0), SMKindGlyph::GlyphSize, 14.0), rows[i].glyph, ink);

            // The same mark, drawn into a magnified box so its shape is readable on paper.
            painter.save();
            painter.translate(48.0, y + ((rowH - (14 * magnify)) / 2.0));
            painter.scale(magnify, magnify);
            SMKindGlyph::paint(painter, QRectF(0.0, 0.0, SMKindGlyph::GlyphSize, 14.0), rows[i].glyph, ink);
            painter.restore();

            painter.setPen(ink);
            painter.drawText(QRectF(130.0, y, 330.0, rowH), Qt::AlignVCenter | Qt::AlignLeft
                            , QString::fromLatin1(rows[i].label));
        }

        painter.end();
        image.save(gGrabDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
    }

    //!< A close-up of one state box: the marks are 8 to 12 px on the canvas, so the acceptance
    //!< picture renders the box alone at \p scale. Nothing is selected while it is taken -- a
    //!< selected box paints its dashed overlay over the very rows the picture is about.
    void grabBox(SMScene& scene, SMStateItem& box, const char* name, double scale = 5.0)
    {
        if (gGrabDir.isEmpty())
        {
            return;
        }

        const bool wasSelected = box.isSelected();
        box.setSelected(false);

        const QRectF area = box.mapToScene(box.boundingRect()).boundingRect().adjusted(-6.0, -6.0, 6.0, 6.0);
        QImage image(QSize(static_cast<int>(area.width() * scale), static_cast<int>(area.height() * scale))
                    , QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        scene.render(&painter, QRectF(image.rect()), area);
        painter.end();
        image.save(gGrabDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));

        box.setSelected(wasSelected);
    }

    //!< The state named \p name anywhere in the document, with the level (owning composite, or the
    //!< root) it lives on. The document has no parent pointer, so the levels are walked.
    SMStateEntry* findStateOnLevel(SMStateData* level, uint32_t levelId, const QString& name, uint32_t& foundLevel)
    {
        if (level == nullptr)
        {
            return nullptr;
        }

        for (SMStateEntry* state : level->getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            if (state->getName() == name)
            {
                foundLevel = levelId;
                return state;
            }

            if (state->hasNestedStates())
            {
                SMStateEntry* nested = findStateOnLevel(state->getNestedStates(), state->getId(), name, foundLevel);
                if (nested != nullptr)
                {
                    return nested;
                }
            }
        }

        return nullptr;
    }

    //!< The one internal transition of \p state, or null.
    SMTransitionEntry* internalOf(SMStateEntry* state)
    {
        if (state == nullptr)
        {
            return nullptr;
        }

        for (SMTransitionEntry* transition : state->getTransitions().getElements())
        {
            if ((transition != nullptr) && transition->isInternal())
            {
                return transition;
            }
        }

        return nullptr;
    }

    //!< The name of the single action call in \p transition's operation list, or empty.
    QString actionOf(const SMTransitionEntry* transition)
    {
        if (transition == nullptr)
        {
            return QString();
        }

        for (const SMOperationBase* op : transition->getOperations().getOperations())
        {
            if ((op != nullptr) && (op->getOperationType() == SMOperationBase::eOperation::ActionCall))
            {
                return op->getName();
            }
        }

        return QString();
    }

    SMPropertiesPanel* panelOf(SMDesign& design)
    {
        QDockWidget* dock = design.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        return (dock != nullptr ? qobject_cast<SMPropertiesPanel*>(dock->widget()) : nullptr);
    }

    //!< Picks an action name from the editor's closed list that is NOT the one already set, and
    //!< commits it exactly as a click on that combo would -- the row is `activated`, not typed.
    QString retargetAction(SMOperationsEditor& editor, const QString& current)
    {
        QComboBox* combo = editor.actionCombo();
        if (combo == nullptr)
        {
            return QString();
        }

        for (int row = 1; row < combo->count(); ++row)
        {
            const QString candidate = combo->itemText(row);
            if ((candidate.isEmpty() == false) && (candidate != current))
            {
                combo->setCurrentIndex(row);
                QMetaObject::invokeMethod(combo, "activated", Q_ARG(int, row));
                QApplication::processEvents();
                return candidate;
            }
        }

        return QString();
    }

    //!< The point inside \p box that draws the row standing for \p transitionId, or a null point.
    //!< Found through the public hit test, so it proves the row is clickable where it is drawn.
    QPointF pointOfInternalRow(const SMStateItem& box, uint32_t transitionId)
    {
        const QRectF bounds = box.boundingRect();
        for (double y = bounds.top(); y < bounds.bottom(); y += 1.0)
        {
            const QPointF probe(bounds.left() + 30.0, y);
            const SMStateItem::BodyRow* row = box.bodyRowAtPos(probe);
            if ((row != nullptr) && (row->transitionId == transitionId))
            {
                return probe;
            }
        }

        return QPointF();
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::printf("usage: lusan_sm_l4_proof <TrafficLight.fsml> [output directory]\n");
        return 2;
    }

    QApplication app(argc, argv);
    // The canvas takes every colour it paints from the palette (a state body is AlternateBase, its
    // text the contrasting shade). A test binary gets no application style of its own, and an
    // unstyled palette is all black -- which paints white text into an unreadable picture. Fusion
    // is the standard palette the real application starts from.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    const QString source = QString::fromLocal8Bit(argv[1]);
    gGrabDir = (argc > 2) ? QString::fromLocal8Bit(argv[2]) : QString();

    // Edit a COPY. The acceptance is about the document round trip, not about writing into a
    // checked-out source tree.
    const QString work = (gGrabDir.isEmpty() ? QDir::tempPath() : gGrabDir) + QDir::separator() + QStringLiteral("TrafficLight.fsml");
    QFile::remove(work);
    check(QFile::copy(source, work), "the document is copied to the working directory");

    QString firstAction;
    QString secondAction;
    uint32_t internalId = 0;

    //////////////////////////////////////////////////////////////////////////
    // Route 1 -- the state page's Internal tab
    //////////////////////////////////////////////////////////////////////////
    {
        StateMachineModel model;
        check(model.loadFromFile(work), "TrafficLight.fsml loads");

        SMDesign design(model);
        design.resize(1500, 950);
        design.show();
        QApplication::processEvents();

        uint32_t levelId = 0;
        SMStateEntry* red = findStateOnLevel(&model.getData().getStates(), 0u, QStringLiteral("TRAFFIC_LIGHT_RED"), levelId);
        check(red != nullptr, "TRAFFIC_LIGHT_RED is in the document");
        if (red == nullptr)
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        SMTransitionEntry* internal = internalOf(red);
        check(internal != nullptr, "TRAFFIC_LIGHT_RED carries an internal transition");
        check((internal != nullptr) && (internal->getStimulus() == QStringLiteral("PedestrianWalk"))
             , "it is the one on the PedestrianWalk timer");
        if (internal == nullptr)
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        internalId = internal->getId();
        const QString before = actionOf(internal);
        check(before.isEmpty() == false, "it runs one action today");

        design.getSceneManager().navigateTo(levelId);
        model.getSelectionModel().setSelection(QList<uint32_t>{ red->getId() });
        QApplication::processEvents();

        SMStateItem* box = dynamic_cast<SMStateItem*>(design.getScene().findCanvasItem(red->getId()));
        check(box != nullptr, "the state box is on the canvas");

        // Defect 1 and 2: the cause and the effect no longer read as the same kind of row, and the
        // four bands of the box each wear their own mark.
        if (box != nullptr)
        {
            const QList<SMStateItem::BodyRow> rows = box->getBodyRows();
            int header = -1;
            for (int i = 0; i < rows.size(); ++i)
            {
                if (rows.at(i).transitionId == internalId)
                {
                    header = i;
                }
            }

            check(header >= 0, "the `on PedestrianWalk` row stands for the transition");
            if ((header >= 0) && ((header + 1) < rows.size()))
            {
                check(rows.at(header).icon == SMKindGlyph::eGlyph::Internal, "the header wears the internal band mark");
                check(rows.at(header).kindIcon == SMKindGlyph::eGlyph::TimerStart, "and the timer that fires it");
                check(rows.at(header + 1).icon == SMKindGlyph::eGlyph::Action, "the action below it wears the gear");
                check(rows.at(header).icon != rows.at(header + 1).icon, "cause and effect no longer share a mark");
            }

            bool sawEntry = false;
            bool sawExit  = false;
            for (const SMStateItem::BodyRow& row : rows)
            {
                sawEntry = sawEntry || (row.icon == SMKindGlyph::eGlyph::Entry);
                sawExit  = sawExit  || (row.icon == SMKindGlyph::exitGlyph());
            }

            check(sawEntry && sawExit, "the entry and exit bands still wear their own marks");
        }

        grabGlyphSheet("l4-00-glyph-vocabulary");
        grab(design, "l4-01-designer");
        if (box != nullptr)
        {
            grabBox(design.getScene(), *box, "l4-01-state-box");
        }

        // Defect 3: the Internal tab, where the author is already looking.
        SMPropertiesPanel* panel = panelOf(design);
        QTabWidget* tabs = (panel != nullptr ? panel->findChild<QTabWidget*>(QStringLiteral("smStateTabs")) : nullptr);
        check((panel != nullptr) && (tabs != nullptr), "the state page is shown");
        if ((panel == nullptr) || (tabs == nullptr))
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        const int internalTab = tabs->count() - 1;
        check(tabs->tabText(internalTab).startsWith(QStringLiteral("Internal")), "the last tab is Internal");
        check(tabs->tabText(internalTab).contains(QStringLiteral("(1)")), "and it says the state has one");
        tabs->setCurrentIndex(internalTab);
        QApplication::processEvents();

        QListWidget* picker = panel->internalEditor()->list();
        check((picker != nullptr) && (panel->internalEditor()->count() == 1)
             , "the tab offers the internal transition");
        check(panel->internalEditor()->currentTransition() == internalId, "and has it selected without a click");

        // What fires it stays on screen whichever editor is open -- it is the identity of the
        // transition, not a section of it -- and the two editors are tabs, so neither can push the
        // other off the bottom.
        QTabWidget* inner = panel->internalEditor()->tabs();
        check((inner != nullptr) && (inner->count() == 2), "Actions and Conditions are tabs");
        if ((inner != nullptr) && (inner->count() == 2))
        {
            check(inner->tabText(0) == QStringLiteral("Actions"), "Actions first");
            check(inner->tabText(1) == QStringLiteral("Conditions"), "then Conditions");
        }

        check(panel->internalEditor()->stimulusCombo()->isVisibleTo(panel), "the stimulus row stays visible");

        grab(design, "l4-02-internal-tab");

        SMOperationsEditor* ops = panel->internalEditor()->operations();
        check(ops != nullptr, "its Actions section is bound to that transition");
        if (ops != nullptr)
        {
            firstAction = retargetAction(*ops, before);
            check(firstAction.isEmpty() == false, "an action is picked from the closed list");
            check(actionOf(internalOf(red)) == firstAction, "the model took the edit");
            std::printf("  route 1 (Internal tab): action %s -> %s\n"
                       , before.toLocal8Bit().constData(), firstAction.toLocal8Bit().constData());
        }

        grab(design, "l4-03-internal-tab-edited");

        // The canvas context menu opens the SAME editor in a dialog, beside `Enter Actions...` and
        // `Exit Actions...`, and it is offered wherever those are -- including a right-click on the
        // state title, where nothing about a body row is under the pointer.
        {
            SMInternalDialog dialog(model, QStringLiteral("Internal: TRAFFIC_LIGHT_RED"), red->getId(), internalId);
            dialog.resize(520, 520);
            dialog.show();
            QApplication::processEvents();
            check(dialog.editor() != nullptr, "the dialog hosts the internal editor");
            check((dialog.editor() != nullptr) && (dialog.editor()->currentTransition() == internalId)
                 , "opened on the transition it was asked for");
            check((dialog.editor() != nullptr) && (dialog.editor()->count() == 1)
                 , "offering the same one transition the tab offers");
            grab(dialog, "l4-03b-internal-dialog");
        }

        check(model.saveToFile(work), "the document saves");
    }

    //////////////////////////////////////////////////////////////////////////
    // The change survived the round trip
    //////////////////////////////////////////////////////////////////////////
    {
        StateMachineModel reloaded;
        check(reloaded.loadFromFile(work), "the saved document reloads");
        uint32_t levelId = 0;
        SMStateEntry* red = findStateOnLevel(&reloaded.getData().getStates(), 0u, QStringLiteral("TRAFFIC_LIGHT_RED"), levelId);
        check(actionOf(internalOf(red)) == firstAction, "the Internal-tab edit survived save and reload");
    }

    //////////////////////////////////////////////////////////////////////////
    // Route 2 -- the `on <stimulus>` row inside the state box
    //////////////////////////////////////////////////////////////////////////
    {
        StateMachineModel model;
        check(model.loadFromFile(work), "the document loads again");

        SMDesign design(model);
        design.resize(1500, 950);
        design.show();
        QApplication::processEvents();

        uint32_t levelId = 0;
        SMStateEntry* red = findStateOnLevel(&model.getData().getStates(), 0u, QStringLiteral("TRAFFIC_LIGHT_RED"), levelId);
        SMTransitionEntry* internal = internalOf(red);
        check((red != nullptr) && (internal != nullptr), "TRAFFIC_LIGHT_RED still carries it");
        if ((red == nullptr) || (internal == nullptr))
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        internalId = internal->getId();
        design.getSceneManager().navigateTo(levelId);
        model.getSelectionModel().setSelection(QList<uint32_t>{ red->getId() });
        QApplication::processEvents();

        SMPropertiesPanel* panel = panelOf(design);
        QTabWidget* tabs = (panel != nullptr ? panel->findChild<QTabWidget*>(QStringLiteral("smStateTabs")) : nullptr);
        SMStateItem* box = dynamic_cast<SMStateItem*>(design.getScene().findCanvasItem(red->getId()));
        SMGraphicsView* view = design.findChild<SMGraphicsView*>();
        check((panel != nullptr) && (tabs != nullptr) && (box != nullptr) && (view != nullptr), "the canvas and the panel are up");
        if ((panel == nullptr) || (tabs == nullptr) || (box == nullptr) || (view == nullptr))
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        // Start away from the Internal tab, so landing on it is the click's doing and not the
        // selection's: the author reading the box has the General tab open.
        tabs->setCurrentIndex(0);
        QApplication::processEvents();

        // The click the author makes: Ctrl+Shift on the `on PedestrianWalk` row. It used to navigate
        // to the PedestrianWalk TIMER declaration; it must now open the transition itself.
        const QPointF hit = pointOfInternalRow(*box, internalId);
        check(hit.isNull() == false, "the row is hit-testable where it is drawn");
        if (hit.isNull() == false)
        {
            const QPointF scenePos = box->mapToScene(hit);
            view->centerOn(scenePos);       // the row has to be on screen to be clicked
            QApplication::processEvents();
            const QPoint vp = view->mapFromScene(scenePos);
            const QPointF global = view->viewport()->mapToGlobal(vp);
            QMouseEvent press(QEvent::MouseButtonPress, QPointF(vp), global, Qt::LeftButton, Qt::LeftButton
                             , Qt::ControlModifier | Qt::ShiftModifier);
            QApplication::sendEvent(view->viewport(), &press);
            QApplication::processEvents();
            QMouseEvent release(QEvent::MouseButtonRelease, QPointF(vp), global, Qt::LeftButton, Qt::NoButton
                               , Qt::ControlModifier | Qt::ShiftModifier);
            QApplication::sendEvent(view->viewport(), &release);
            QApplication::processEvents();
            check(panel->currentPage() == SMPropertiesPanel::PageState, "the click lands on the state page");
            check(panel->currentElementId() == red->getId(), "on the state that draws the row");
            check(tabs->currentIndex() == (tabs->count() - 1), "with the Internal tab open");
            check(panel->internalEditor()->currentTransition() == internalId, "on the transition the row stands for");
        }

        grab(design, "l4-04-row-opened-internal-tab");

        SMOperationsEditor* ops = panel->internalEditor()->operations();
        const QString before = actionOf(internalOf(red));
        if (ops != nullptr)
        {
            secondAction = retargetAction(*ops, before);
            check(secondAction.isEmpty() == false, "an action is picked from there too");
            check(actionOf(internalOf(red)) == secondAction, "the model took the second edit");
            std::printf("  route 2 (in-box row):    action %s -> %s\n"
                       , before.toLocal8Bit().constData(), secondAction.toLocal8Bit().constData());
        }

        grab(design, "l4-05-row-route-edited");
        grabBox(design.getScene(), *box, "l4-05-state-box-edited");
        check(model.saveToFile(work), "the document saves again");
    }

    {
        StateMachineModel reloaded;
        check(reloaded.loadFromFile(work), "the saved document reloads");
        uint32_t levelId = 0;
        SMStateEntry* red = findStateOnLevel(&reloaded.getData().getStates(), 0u, QStringLiteral("TRAFFIC_LIGHT_RED"), levelId);
        check(actionOf(internalOf(red)) == secondAction, "the in-box-row edit survived save and reload");

        // One last picture, of the reloaded document: what the box looks like after both round trips.
        SMDesign design(reloaded);
        design.resize(1500, 950);
        design.show();
        QApplication::processEvents();
        design.getSceneManager().navigateTo(levelId);
        reloaded.getSelectionModel().setSelection(QList<uint32_t>{ red->getId() });
        QApplication::processEvents();
        grab(design, "l4-06-reloaded");
        SMStateItem* reloadedBox = dynamic_cast<SMStateItem*>(design.getScene().findCanvasItem(red->getId()));
        if (reloadedBox != nullptr)
        {
            grabBox(design.getScene(), *reloadedBox, "l4-06-state-box-reloaded");
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // The Internal tab at REAL dock sizes: nothing may be clipped or unreachable
    //////////////////////////////////////////////////////////////////////////
    {
        StateMachineModel model;
        check(model.loadFromFile(work), "the document loads for the layout checks");
        uint32_t levelId = 0;
        SMStateEntry* red = findStateOnLevel(&model.getData().getStates(), 0u, QStringLiteral("TRAFFIC_LIGHT_RED"), levelId);
        if (red == nullptr)
        {
            std::printf("Checks: %d, Failures: %d (layout abort)\n", gChecks, gFailures);
            return 1;
        }

        // The panel on its own, at the sizes the Properties dock actually gets: its own minimum
        // width, and a height that is what is left once the Outline dock has taken its share.
        // Everything the tab offers has to be reachable at the SHORT height -- having to close
        // another dock to see a section is the defect this checks for.
        const QList<QSize> sizes { QSize(320, 380), QSize(320, 560), QSize(420, 760) };
        for (const QSize& size : sizes)
        {
            SMPropertiesPanel panel(model);
            panel.resize(size);
            panel.show();
            model.getSelectionModel().setSelection(QList<uint32_t>{ red->getId() });
            QApplication::processEvents();

            QTabWidget* tabs = panel.findChild<QTabWidget*>(QStringLiteral("smStateTabs"));
            SMInternalEditor* editor = panel.internalEditor();
            check((tabs != nullptr) && (editor != nullptr), "the state page is up");
            if ((tabs == nullptr) || (editor == nullptr))
            {
                continue;
            }

            tabs->setCurrentIndex(tabs->count() - 1);
            QApplication::processEvents();

            QTabWidget* inner = editor->tabs();
            const QString tag = QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
            for (int page = 0; page < inner->count(); ++page)
            {
                inner->setCurrentIndex(page);
                QApplication::processEvents();
                grab(panel, QStringLiteral("l4-10-internal-%1-s%2").arg(tag).arg(page).toLatin1().constData());

                // Whichever editor is open, the picker, the stimulus row and BOTH tabs stay on
                // screen. A control pushed off the bottom is one the author cannot get back to
                // without resizing the window or closing another dock, which is the defect here.
                const auto reachable = [&panel](QWidget* widget) -> bool
                {
                    return (widget != nullptr) && widget->isVisibleTo(&panel)
                           && panel.rect().contains(widget->mapTo(&panel, QPoint(0, widget->height() - 1)));
                };

                // With ONE internal transition the picker is deliberately gone: it would echo the
                // Stimulus row under it and there is nothing to order. With two or more it must be
                // there AND on screen.
                check(reachable(editor->list())
                     , QStringLiteral("at %1, the transition list is reachable on tab %2")
                       .arg(tag).arg(page).toLatin1().constData());
                check(reachable(editor->stimulusCombo())
                     , QStringLiteral("at %1, the stimulus row is reachable on tab %2")
                       .arg(tag).arg(page).toLatin1().constData());
                check(reachable(inner->tabBar())
                     , QStringLiteral("at %1, both editors stay one click away on tab %2")
                       .arg(tag).arg(page).toLatin1().constData());
            }

            // The whole tab has to FIT the dock it is given, not force it wider or taller.
            check(panel.size().width() <= size.width()
                 , QStringLiteral("at %1, the panel does not force the dock wider").arg(tag).toLatin1().constData());
            check(panel.size().height() <= size.height()
                 , QStringLiteral("at %1, the panel does not force the dock taller").arg(tag).toLatin1().constData());
        }
    }


    //////////////////////////////////////////////////////////////////////////
    // Priority -- several internal transitions on ONE stimulus, told apart,
    // reordered, and reflected on the canvas.
    //////////////////////////////////////////////////////////////////////////
    {
        std::printf("sect: priority, guard chips and reordering\n");
        StateMachineModel model;
        check(model.loadFromFile(work), "the working copy reopens");

        SMDesign design(model);
        design.resize(1500, 950);
        design.show();
        QApplication::processEvents();

        uint32_t levelId = 0;
        SMStateEntry* red = findStateOnLevel(&model.getData().getStates(), 0u, QStringLiteral("TRAFFIC_LIGHT_RED"), levelId);
        check(red != nullptr, "TRAFFIC_LIGHT_RED is still there");
        if (red == nullptr)
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        design.getSceneManager().navigateTo(levelId);
        model.getSelectionModel().setSelection(QList<uint32_t>{ red->getId() });
        QApplication::processEvents();

        SMPropertiesPanel* panel = design.findChild<SMPropertiesPanel*>();
        QTabWidget* tabs = (panel != nullptr) ? panel->findChild<QTabWidget*>(QStringLiteral("smStateTabs")) : nullptr;
        SMInternalEditor* editor = (panel != nullptr) ? panel->internalEditor() : nullptr;
        check((panel != nullptr) && (tabs != nullptr) && (editor != nullptr), "the state page is up");
        if ((panel == nullptr) || (tabs == nullptr) || (editor == nullptr))
        {
            std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
            return 1;
        }

        tabs->setCurrentIndex(tabs->count() - 1);
        QApplication::processEvents();

        // With one internal transition there is no priority to speak of and nothing to choose.
        check(editor->count() == 1, "the state starts with a single internal transition");
        check(editor->list()->isVisibleTo(panel), "the list is there from the start, not conjured by the second one");
        check(editor->list()->item(0)->text().startsWith(QStringLiteral("on ")), "and the row carries no number");

        // The toolbar does NOT come and go with the list: with nothing to list, `add` is the only
        // thing on this tab that does anything, and a control that moves is a control that is hunted
        // for.
        QToolButton* addBtn = editor->findChild<QToolButton*>(QStringLiteral("smBtnAddInternal"));
        QToolButton* upBtn  = editor->findChild<QToolButton*>(QStringLiteral("smBtnInternalUp"));
        check((addBtn != nullptr) && addBtn->isVisibleTo(panel) && addBtn->isEnabled()
             , "but the toolbar stays, with add live");
        check((upBtn != nullptr) && upBtn->isVisibleTo(panel) && (upBtn->isEnabled() == false)
             , "and the priority buttons present but dead with nothing to order");

        // Two more on the SAME timer, differing only by their guards -- the case the whole change
        // is about. One long, one a raw C++ block: the shapes chipText has to handle.
        const uint32_t firstId = editor->currentTransition();
        QToolButton* addButton = editor->findChild<QToolButton*>(QStringLiteral("smBtnAddInternal"));
        check(addButton != nullptr, "the add button is there");

        // A fixed frame, so nothing below the list moves as the list fills. This is the whole
        // reason it is not sized to its content: an author who just pressed `add` is looking at the
        // row they made, not hunting for the field that jumped.
        const int listHeightAtOne = editor->list()->height();
        const QPoint stimulusAtOne = editor->stimulusCombo()->mapTo(editor, QPoint(0, 0));
        const QPoint tabsAtOne     = editor->tabs()->mapTo(editor, QPoint(0, 0));
        const QString longGuard = QStringLiteral("mWaitingCount > 3 && mIsEastWest && isNightMode");
        const QString rawGuard  = QStringLiteral("{ return mWaitingCount > 3; }");
        for (int extra = 0; (extra < 2) && (addButton != nullptr); ++extra)
        {
            addButton->click();
            QApplication::processEvents();
            const uint32_t id = editor->currentTransition();
            SMTransitionEntry* fresh = model.getData().findTransitionById(id);
            check(fresh != nullptr, "a new internal transition is added and selected");
            if (fresh == nullptr)
            {
                continue;
            }

            fresh->setStimulusKind(SMTransitionEntry::eStimulusKind::Timer);
            fresh->setStimulus(QStringLiteral("PedestrianWalk"));
            fresh->getGuard().setDraft((extra == 0) ? longGuard : rawGuard);
            model.getNotifier().notifyElementChanged(id, eDocElementKind::Transition);
        }

        // These two were written straight into the model rather than through the UI, and the editor
        // deliberately does NOT rebuild from a notification while one of its own fields has focus
        // (that is the author typing). Ask it to re-read, which is what the next real interaction
        // would do anyway.
        editor->refresh();
        QApplication::processEvents();
        check(editor->count() == 3, "the state now has three internal transitions");
        check(editor->list()->isVisibleTo(panel), "the list still holds them");

        // THE point of the exercise: three rows on one stimulus, and no two of them read alike.
        QStringList rows;
        for (int row = 0; row < editor->list()->count(); ++row)
        {
            rows.append(editor->list()->item(row)->text());
        }

        for (const QString& row : rows)
        {
            std::printf("  picker row: %s\n", row.toLatin1().constData());
        }

        check(editor->list()->height() == listHeightAtOne, "the list frame did not grow with its content");
        check(editor->stimulusCombo()->mapTo(editor, QPoint(0, 0)) == stimulusAtOne, "the stimulus row did not move");
        check(editor->tabs()->mapTo(editor, QPoint(0, 0)) == tabsAtOne, "and neither did the editors below it");

        check(rows.size() == 3, "three rows are offered");
        check(QSet<QString>(rows.begin(), rows.end()).size() == rows.size(), "and no two of them read alike");
        for (int row = 0; row < rows.size(); ++row)
        {
            check(rows.at(row).startsWith(QStringLiteral("#") + QString::number(row + 1) + QStringLiteral(" on "))
                 , QStringLiteral("row %1 leads with its priority number").arg(row + 1).toLatin1().constData());
        }

        // The long guard is shortened, and NOT in the middle of an identifier; the raw block says
        // what it is rather than showing a wall of C++ that does not fit.
        check(rows.at(1).contains(QLatin1Char('[')) && rows.at(1).contains(QStringLiteral("...")), "the long guard is cut");
        check(rows.at(1).contains(QStringLiteral("isNight")) == false, "and never mid-identifier");
        check(rows.at(2).contains(QStringLiteral("{ C++ }")), "a raw C++ guard reads as what it is");

        // The same rows, in the same order, inside the state box on the canvas.
        SMStateItem* box = dynamic_cast<SMStateItem*>(design.getScene().findCanvasItem(red->getId()));
        check(box != nullptr, "the state box is on the canvas");

        QStringList painted;
        if (box != nullptr)
        {
            for (const SMStateItem::BodyRow& row : box->getBodyRows())
            {
                if (row.transitionId != 0u)
                {
                    painted.append(row.text);
                }
            }
        }

        check(painted.size() == 3, "the box paints one row per internal transition");
        check(QSet<QString>(painted.begin(), painted.end()).size() == painted.size(), "no two box rows read alike either");
        const QString wasFirst = painted.isEmpty() ? QString() : painted.at(0);

        grab(design, "l4-11-three-internals");

        // Adding from OUTSIDE this tab -- the canvas action, the Design menu, the toolbar -- must
        // select the new transition too. It used to leave the previous row active, so the thing the
        // author had just asked for looked like it had not been created.
        const int beforeAdd = editor->count();
        SMStateEntry* owner = model.getData().findStateById(red->getId());
        SMCreateTransitionCommand* fromCanvas =
                new SMCreateTransitionCommand(  model.getData(), model.getNotifier(), *owner
                                              , SMTransitionEntry::eStimulusKind::Trigger, QString()
                                              , 0u, QList<QPointF>(), QStringLiteral("Add internal transition")
                                              , nullptr, SMTransitionEntry::eTransitionKind::Internal);
        model.getUndoStack().push(fromCanvas);
        QApplication::processEvents();
        check(editor->count() == (beforeAdd + 1), "an internal transition added elsewhere reaches the tab");
        check(editor->currentTransition() == fromCanvas->getTransitionId()
             , "and it is the one selected, not the row that was active before");

        model.getUndoStack().undo();
        QApplication::processEvents();
        check(editor->count() == beforeAdd, "undoing that add takes it away again");

        // Deleting steps BACK one, it does not throw the author to the top of the list.
        QToolButton* removeButton = editor->findChild<QToolButton*>(QStringLiteral("smBtnRemoveInternal"));
        check(removeButton != nullptr, "the remove button is there");
        if (removeButton != nullptr)
        {
            const int last = editor->count() - 1;
            editor->list()->setCurrentRow(last);
            QApplication::processEvents();
            removeButton->click();
            QApplication::processEvents();
            check(editor->list()->currentRow() == (last - 1), "deleting the last row selects the one above it");

            editor->list()->setCurrentRow(0);
            QApplication::processEvents();
            const uint32_t wasSecond = editor->list()->item(1)->data(Qt::UserRole + 1).toUInt();
            removeButton->click();
            QApplication::processEvents();
            check(editor->list()->currentRow() == 0, "deleting the first row stays at the top");
            check(editor->currentTransition() == wasSecond, "on what was the second one, now the first");

            model.getUndoStack().undo();
            model.getUndoStack().undo();
            QApplication::processEvents();
            check(editor->count() == 3, "and both deletes undo");
        }

        // ---- reordering ----------------------------------------------------
        // Capture what a reorder must NOT disturb: the IDs themselves, and the layout geometry that
        // is keyed by them. This is the check that would have caught a swap-based reorder.
        QSet<uint32_t> idsBefore;
        uint32_t edgeOwner = 0u;
        QPointF  edgeLabel;
        for (const SMTransitionEntry* transition : red->getTransitions().getElements())
        {
            idsBefore.insert(transition->getId());
            const SMLayoutEdge* edge = model.getData().getLayout().findEdge(transition->getId());
            if ((edge != nullptr) && (edgeOwner == 0u))
            {
                edgeOwner = transition->getId();
                edgeLabel = edge->label;
            }
        }

        check(edgeOwner != 0u, "an external transition of this state owns layout geometry");

        editor->setCurrentTransition(firstId);
        QApplication::processEvents();
        QToolButton* down = editor->findChild<QToolButton*>(QStringLiteral("smBtnInternalDown"));
        check((down != nullptr) && down->isEnabled(), "the first row can be lowered");
        if (down != nullptr)
        {
            down->click();
            QApplication::processEvents();
        }

        check(editor->list()->item(1)->text().startsWith(QStringLiteral("#2 ")), "the moved row is now second");
        check(editor->currentTransition() == firstId, "and it is still the one being edited");

        QSet<uint32_t> idsAfter;
        for (const SMTransitionEntry* transition : red->getTransitions().getElements())
        {
            idsAfter.insert(transition->getId());
        }

        check(idsAfter == idsBefore, "every transition keeps its own ID across the move");
        const SMLayoutEdge* keptEdge = model.getData().getLayout().findEdge(edgeOwner);
        check((keptEdge != nullptr) && (keptEdge->label == edgeLabel)
             , "and the layout geometry keyed by ID still belongs to the same transition");

        // The canvas is not asked to refresh; it must already have followed.
        QStringList repainted;
        if (box != nullptr)
        {
            for (const SMStateItem::BodyRow& row : box->getBodyRows())
            {
                if (row.transitionId != 0u)
                {
                    repainted.append(row.text);
                }
            }
        }

        QString expectSecond = wasFirst;
        expectSecond.replace(QStringLiteral("#1"), QStringLiteral("#2"));
        check((repainted.size() == 3) && (repainted.at(1) == expectSecond)
             , "the state box renumbered itself without being asked");

        grab(design, "l4-12-after-reorder");

        // ---- undo ----------------------------------------------------------
        model.getUndoStack().undo();
        QApplication::processEvents();
        check(editor->list()->item(0)->text().startsWith(QStringLiteral("#1 ")), "undo puts the order back");
        check(model.getData().findTransitionById(firstId) != nullptr, "with the same IDs intact");

        // ---- the status line knows what the stimulus IS --------------------
        editor->setCurrentTransition(firstId);
        QApplication::processEvents();
        check(editor->signature()->text().startsWith(QStringLiteral("timer PedestrianWalk"))
             , "the status line names the timer");
        check(editor->signature()->text().contains(QStringLiteral("ms")), "and says when it fires");
        check(editor->signature()->toolTip().contains(QStringLiteral("Ctrl+Shift"))
             , "and offers the jump to its declaration");

        // Not the tooltip -- the GESTURE. A label dimmed with setEnabled(false) looks identical and
        // is never delivered a mouse event, so only a real click proves the jump is reachable.
        uint32_t jumpedTo = 0u;
        SMReferences::eTarget jumpedKind = SMReferences::eTarget::State;
        QObject::connect(editor, &SMInternalEditor::signalNavigateToDefinition
                        , [&jumpedTo, &jumpedKind](SMReferences::eTarget kind, uint32_t declId)
        {
            jumpedKind = kind;
            jumpedTo   = declId;
        });

        QLabel* line = editor->signature();
        QMouseEvent jump( QEvent::MouseButtonPress, QPointF(4.0, 4.0), line->mapToGlobal(QPoint(4, 4))
                        , Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::ShiftModifier);
        QApplication::sendEvent(line, &jump);
        QApplication::processEvents();
        check(jumpedTo != 0u, "a Ctrl+Shift click on the status line asks for the declaration");
        check(jumpedKind == SMReferences::eTarget::Timer, "and names it as the timer it is");

        // ---- the stimulus drives the row label -----------------------------
        const QString labelBefore = editor->list()->item(0)->text();
        QComboBox* stimulus = editor->stimulusCombo();
        int redRow = -1;
        for (int row = 0; (stimulus != nullptr) && (row < stimulus->count()); ++row)
        {
            if (stimulus->itemData(row, Qt::UserRole + 1).toString() == QStringLiteral("Red"))
            {
                redRow = row;
                break;
            }
        }

        check(redRow > 0, "the Red timer is offered as a stimulus");
        if (redRow > 0)
        {
            stimulus->setCurrentIndex(redRow);
            QMetaObject::invokeMethod(editor, "onStimulusCommit");
            QApplication::processEvents();
            check(editor->list()->item(0)->text().contains(QStringLiteral("Red"))
                 , "changing the stimulus relabels the transition row at once");

            model.getUndoStack().undo();
            QApplication::processEvents();
            check(editor->list()->item(0)->text() == labelBefore, "undo restores the label with the stimulus");
        }

        // ---- an action with arguments can be mapped here too ---------------
        SMOperationsEditor* ops = editor->operations();
        QComboBox* actions = (ops != nullptr) ? ops->actionCombo() : nullptr;
        check(actions != nullptr, "the Actions tab offers the action list");
        if (actions != nullptr)
        {
            int withArgs = -1;
            for (int row = 0; row < actions->count(); ++row)
            {
                if (actions->itemText(row).contains(QStringLiteral("vehicle_green")))
                {
                    withArgs = row;
                    break;
                }
            }

            check(withArgs >= 0, "including one that takes an argument");
            if (withArgs >= 0)
            {
                actions->setCurrentIndex(withArgs);
                emit actions->activated(withArgs);
                QApplication::processEvents();

                SMArgMapTable* table = ops->findChild<SMArgMapTable*>();
                check(table != nullptr, "the argument map is present in this scope");
                check((table != nullptr) && (table->rowCount() == 1), "with one row for the action's one parameter");
                check((table != nullptr) && (table->rowCount() == 1) && (table->nameLabel(0) != nullptr)
                      && table->nameLabel(0)->text().contains(QStringLiteral("isEastWest"))
                     , "naming that parameter, so it maps exactly as on a normal transition");
            }
        }

        grab(design, "l4-13-internal-mapping");
    }

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
