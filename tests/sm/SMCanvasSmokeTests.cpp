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
 *  \file        tests/sm/SMCanvasSmokeTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-13 smoke tests: drives the design canvas offscreen with synthetic
 *               mouse/key events through the real SMDesign/SMScene/SMStateItem stack --
 *               state rendering, placement tool, inline rename validation, move,
 *               resize, collapse, and undo of each. Saves widget grabs as PNG when an
 *               output directory is passed as the second argument.
 *
 *  Usage: lusan_sm_canvas_tests <TrafficLight.fsml path> [grab output dir]
 *
 ************************************************************************/

#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMMethodKind.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMClipboard.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMAutoPlacer.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMGuardHelpCard.hpp"
#include "lusan/view/sm/SMNoteItem.hpp"
#include "lusan/view/sm/SMOutlinePanel.hpp"
#include "lusan/view/sm/SMAccordion.hpp"
#include "lusan/view/sm/SMInternalDialog.hpp"
#include "lusan/view/sm/SMInternalEditor.hpp"
#include "lusan/view/sm/SMSectionChrome.hpp"
#include "lusan/view/sm/SMPropertiesPanel.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QAction>
#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QGraphicsPathItem>
#include <QGraphicsProxyWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QSet>
#include <QTreeWidget>
#include <QMimeData>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QDockWidget>
#include <QShortcut>
#include <QToolBar>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QUndoStack>

#include <cmath>
#include <cstdio>

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

    QString gGrabDir;

    void grab(QWidget& widget, const char* name)
    {
        if (gGrabDir.isEmpty() == false)
        {
            widget.grab().save(gGrabDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
        }
    }

    //!< Posts a full mouse press/release pair to the view's viewport at a scene position.
    void clickScene(  SMGraphicsView& view, const QPointF& scenePos, Qt::MouseButton button = Qt::LeftButton
                    , Qt::KeyboardModifiers modifiers = Qt::NoModifier)
    {
        const QPoint vp = view.mapFromScene(scenePos);
        const QPointF global = view.viewport()->mapToGlobal(vp);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(vp), global, button, button, modifiers);
        QApplication::sendEvent(view.viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(vp), global, button, Qt::NoButton, modifiers);
        QApplication::sendEvent(view.viewport(), &release);
        QApplication::processEvents();
    }

    //!< Drags with the left button between two scene positions in small steps.
    void dragScene(SMGraphicsView& view, const QPointF& from, const QPointF& to)
    {
        const QPoint vpFrom = view.mapFromScene(from);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(vpFrom), view.viewport()->mapToGlobal(vpFrom)
                          , Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &press);

        constexpr int steps{ 8 };
        for (int i = 1; i <= steps; ++i)
        {
            const QPointF scenePos = from + (to - from) * (static_cast<double>(i) / steps);
            const QPoint vp = view.mapFromScene(scenePos);
            QMouseEvent move(QEvent::MouseMove, QPointF(vp), view.viewport()->mapToGlobal(vp)
                             , Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(view.viewport(), &move);
        }

        const QPoint vpTo = view.mapFromScene(to);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(vpTo), view.viewport()->mapToGlobal(vpTo)
                            , Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &release);
        QApplication::processEvents();
    }

    void keyClick(QWidget* target, Qt::Key key, const QString& text = QString())
    {
        QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, text);
        QApplication::sendEvent(target, &press);
        QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier, text);
        QApplication::sendEvent(target, &release);
        QApplication::processEvents();
    }

    //!< Posts a double-click sequence (press, release, dbl-click, release) at a scene point.
    void dblClickScene(SMGraphicsView& view, const QPointF& scenePos, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
    {
        const QPoint vp = view.mapFromScene(scenePos);
        const QPointF global = view.viewport()->mapToGlobal(vp);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(vp), global, Qt::LeftButton, Qt::LeftButton, modifiers);
        QApplication::sendEvent(view.viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(vp), global, Qt::LeftButton, Qt::NoButton, modifiers);
        QApplication::sendEvent(view.viewport(), &release);
        QMouseEvent dbl(QEvent::MouseButtonDblClick, QPointF(vp), global, Qt::LeftButton, Qt::LeftButton, modifiers);
        QApplication::sendEvent(view.viewport(), &dbl);
        QMouseEvent release2(QEvent::MouseButtonRelease, QPointF(vp), global, Qt::LeftButton, Qt::NoButton, modifiers);
        QApplication::sendEvent(view.viewport(), &release2);
        QApplication::processEvents();
    }

    //!< Posts a key press/release pair directly to a graphics scene.
    void keyClickScene(SMScene& scene, Qt::Key key)
    {
        QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
        QApplication::sendEvent(&scene, &press);
        QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier);
        QApplication::sendEvent(&scene, &release);
        QApplication::processEvents();
    }

    QByteArray fileBytes(const QString& path)
    {
        QFile file(path);
        return (file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray());
    }

    bool writeBytes(const QString& path, const QByteArray& content)
    {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && (file.write(content) == content.size());
    }

    //!< The box a state occupies, with the minimum size the canvas enforces.
    QRectF nodeBox(const SMLayoutNode& node)
    {
        return QRectF(  node.x, node.y
                      , std::max(node.width, NESMDesign::StateMinWidth)
                      , std::max(node.height, NESMDesign::StateMinHeight));
    }

    //!< True when every state of every level has a Node entry and no two boxes of one level overlap.
    bool levelsArePlaced(const SMStateData& level, const SMLayoutData& layout)
    {
        QList<QRectF> boxes;
        for (const SMStateEntry* state : level.getElements())
        {
            const SMLayoutNode* node = layout.findNode(state->getId());
            if (node == nullptr)
            {
                return false;
            }

            const QRectF box{ nodeBox(*node) };
            for (const QRectF& other : boxes)
            {
                if (other.intersects(box))
                {
                    return false;
                }
            }

            boxes.append(box);
        }

        for (const SMStateEntry* state : level.getElements())
        {
            if (state->hasNestedStates() && (levelsArePlaced(*state->getNestedStates(), layout) == false))
            {
                return false;
            }
        }

        return true;
    }
}

#define CHECK(cond)  check((cond), #cond)

int main(int argc, char* argv[])
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    if (argc < 2)
    {
        std::printf("Usage: %s <TrafficLight.fsml> [grab dir]\n", argv[0]);
        return 2;
    }

    if (argc >= 3)
    {
        gGrabDir = QString::fromLocal8Bit(argv[2]);
        QDir().mkpath(gGrabDir);
    }

    std::printf("SM-13 canvas smoke tests\n");

    StateMachineModel model;
    CHECK(model.loadFromFile(QString::fromLocal8Bit(argv[1])));

    SMDesign design(model);
    design.resize(1400, 900);
    design.show();
    QApplication::processEvents();

    SMScene&        scene = design.getScene();
    SMGraphicsView& view  = design.getView();
    StateMachineData& data = model.getData();

    // --- Rendering: one box item per root-level state ---
    const SMStateData* level = data.findLevel(scene.getLevelId());
    CHECK(level != nullptr);
    int boxes = 0;
    for (const SMStateEntry* state : level->getElements())
    {
        if (dynamic_cast<SMStateItem*>(scene.findCanvasItem(state->getId())) != nullptr)
        {
            ++boxes;
        }
    }
    CHECK(boxes == level->getElementCount());

    SMStateEntry* lightOff = data.findState("LightOff");
    CHECK(lightOff != nullptr);
    SMStateItem* offItem = dynamic_cast<SMStateItem*>(scene.findCanvasItem(lightOff->getId()));
    CHECK(offItem != nullptr);
    view.centerOn(offItem);
    QApplication::processEvents();
    grab(design, "g1-rendered");

    std::printf("sect: z-order + Start/Final endpoint rules\n");
    // Isolated in its own model/design so undo-stack accounting never perturbs the sections below.
    {
        StateMachineModel zmodel;
        CHECK(zmodel.loadFromFile(QString::fromLocal8Bit(argv[1])));
        SMDesign zdesign(zmodel);
        zdesign.resize(1000, 700);
        zdesign.show();
        QApplication::processEvents();

        SMScene&          zscene = zdesign.getScene();
        StateMachineData& zdata  = zmodel.getData();

        // --- Issue 1: an inactive transition line paints ABOVE an inactive state box ---
        SMStateEntry* zoff = zdata.findState("LightOff");
        CHECK(zoff != nullptr);
        SMStateItem* zoffItem = zscene.stateItem(zoff->getId());
        SMEdgeItem*  zedge     = dynamic_cast<SMEdgeItem*>(zscene.findCanvasItem(27));
        CHECK((zoffItem != nullptr) && (zedge != nullptr));
        if ((zoffItem != nullptr) && (zedge != nullptr))
        {
            CHECK(zoffItem->zValue() == 0.0);       // inactive box: bottom band
            CHECK(zedge->zValue() == 1.0);          // inactive edge: above the boxes
            // A selected (active) box raises to cover the inactive edges...
            zscene.clearSelection();
            zoffItem->setSelected(true);
            QApplication::processEvents();
            CHECK(zoffItem->zValue() == 2.0);
            // ...and a selected edge tops everything so its handles stay grabbable.
            zscene.clearSelection();
            zedge->setSelected(true);
            QApplication::processEvents();
            CHECK(zedge->zValue() == 3.0);
            zscene.clearSelection();
            QApplication::processEvents();
            CHECK(zoffItem->zValue() == 0.0);       // back to the inactive bands
            CHECK(zedge->zValue() == 1.0);
        }

        // --- Issue 2: a Start state is a source only; a Final state is a target only ---
        const SMLayoutEdge geom = *zdata.getLayout().findEdge(27);

        // Retargeting a transition onto the Start state is rejected (no incoming into Start).
        // The Start is the level's pseudo-state, NOT the first ordinary state: this used to name
        // `LightOff`, which carried Kind="Start" before the pseudo-state form.
        const SMStateData* zlevel = zdata.findLevel(zscene.getLevelId());
        const SMStateEntry* zstart = (zlevel != nullptr ? zlevel->getStartState() : nullptr);
        CHECK(zstart != nullptr);
        const uint32_t to27Before = zdata.findTransitionById(27)->getToId();
        const int      undoA = zmodel.getUndoStack().count();
        zscene.reconnectTransitionTarget(27, (zstart != nullptr ? zstart->getId() : 0u), geom);
        CHECK(zdata.findTransitionById(27)->getToId() == to27Before);    // unchanged
        CHECK(zmodel.getUndoStack().count() == undoA);                   // nothing pushed

        // Reparenting a transition onto a Final state is rejected (no outgoing from Final).
        SMStateData& zroot = *zdata.findLevel(zscene.getLevelId());
        SMCreateStateCommand* mkFinal = new SMCreateStateCommand(  zdata, zmodel.getNotifier(), zroot
                                                                 , QStringLiteral("Final"), SMStateEntry::eStateKind::Final
                                                                 , QRectF(600.0, 40.0, 96.0, 48.0), QStringLiteral("final"));
        zmodel.getUndoStack().push(mkFinal);
        SMStateEntry* zfinal = zdata.findStateById(mkFinal->getStateId());
        CHECK((zfinal != nullptr) && (zfinal->getKind() == SMStateEntry::eStateKind::Final));
        const SMStateEntry* ownerBefore = zdata.findTransitionOwner(27);
        const int           undoB = zmodel.getUndoStack().count();
        zscene.reparentTransition(27, zfinal->getId(), geom);
        CHECK(zdata.findTransitionOwner(27) == ownerBefore);            // unchanged
        CHECK(zmodel.getUndoStack().count() == undoB);                  // nothing pushed
    }

    std::printf("sect: placement\n");
    // --- Placement tool ---
    const int undoBase = model.getUndoStack().count();
    scene.setActiveTool(NESMDesign::eCanvasTool::AddState);
    const QPointF dropPos = offItem->pos() + QPointF(-320.0, -160.0);
    clickScene(view, dropPos);

    SMStateEntry* placed = data.findState("NewState");
    CHECK(placed != nullptr);
    CHECK(model.getUndoStack().count() == undoBase + 1);                        // one undo step
    CHECK(scene.getActiveTool() == NESMDesign::eCanvasTool::Select);            // single-shot
    const SMLayoutNode* placedNode = data.getLayout().findNode(placed->getId());
    CHECK(placedNode != nullptr);
    CHECK(placedNode->x == NESMDesign::snapValue(placedNode->x, scene.getGridSize()));   // grid-snapped

    SMStateItem* placedItem = dynamic_cast<SMStateItem*>(scene.findCanvasItem(placed->getId()));
    CHECK((placedItem != nullptr) && placedItem->isRenameActive());
    grab(design, "g2-placed-rename-open");

    std::printf("sect: rename\n");
    // --- Inline rename ---
    const auto findRenameEditor = [&scene]() -> QLineEdit*
    {
        // The editor lives in a QGraphicsProxyWidget, not the widget hierarchy.
        for (QGraphicsItem* item : scene.items())
        {
            QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if (proxy != nullptr)
            {
                QLineEdit* lineEdit = qobject_cast<QLineEdit*>(proxy->widget());
                if (lineEdit != nullptr)
                {
                    return lineEdit;
                }
            }
        }

        return nullptr;
    };

    QLineEdit* editor = findRenameEditor();

    CHECK(editor != nullptr);
    if (editor != nullptr)
    {
        const int renameLength = editor->text().size();
        CHECK(editor->maxLength() == StateMachineData::MAX_IDENTIFIER_LENGTH);
        const auto checkRenameArrow = [&](Qt::Key key, int expectedCursor) {
            editor->selectAll();
            QApplication::processEvents();
            const SMLayoutNode* before = data.getLayout().findNode(placed->getId());
            CHECK(before != nullptr);
            if (before == nullptr)
            {
                return;
            }

            const QPointF beforePos(before->x, before->y);
            const int undoBefore = model.getUndoStack().count();
            keyClick(&view, key);
            editor = findRenameEditor();
            CHECK(editor != nullptr);
            CHECK((placedItem != nullptr) && placedItem->isRenameActive());
            if (editor != nullptr)
            {
                CHECK(editor->selectedText().isEmpty());
                CHECK(editor->cursorPosition() == expectedCursor);
            }

            const SMLayoutNode* after = data.getLayout().findNode(placed->getId());
            CHECK((after != nullptr) && (QPointF(after->x, after->y) == beforePos));
            CHECK(model.getUndoStack().count() == undoBefore);
        };

        checkRenameArrow(Qt::Key_Left , 0);
        checkRenameArrow(Qt::Key_Right, renameLength);
        checkRenameArrow(Qt::Key_Up   , 0);
        checkRenameArrow(Qt::Key_Down , renameLength);

        const int stateCountBeforeDelete = data.getStateCount();
        editor->selectAll();
        keyClick(editor, Qt::Key_Delete);
        QApplication::processEvents();
        editor = findRenameEditor();
        CHECK(editor != nullptr);
        if (editor != nullptr)
        {
            CHECK(editor->text().isEmpty());
        }
        CHECK(data.getStateCount() == stateCountBeforeDelete);

        if (editor != nullptr)
        {
            editor->setText(QStringLiteral("NewState"));
            QApplication::processEvents();
            editor->selectAll();
            keyClick(editor, Qt::Key_Backspace);
            QApplication::processEvents();
            editor = findRenameEditor();
            CHECK(editor != nullptr);
            if (editor != nullptr)
            {
                CHECK(editor->text().isEmpty());
                editor->setText(QStringLiteral("NewState"));
                QApplication::processEvents();
            }
        }

        editor->setText(QStringLiteral("LightOn"));                             // duplicate
        QApplication::processEvents();
        CHECK(editor->toolTip().isEmpty() == false);                            // rejection reason shown
        keyClick(editor, Qt::Key_Return);
        CHECK(placedItem->isRenameActive());                                    // rejected: still editing
        CHECK(data.findState("NewState") != nullptr);

        editor->setText(QStringLiteral("Standby"));
        QApplication::processEvents();
        CHECK(editor->toolTip().isEmpty());
        keyClick(editor, Qt::Key_Return);
        CHECK(placedItem->isRenameActive() == false);
        CHECK(data.findState("Standby") != nullptr);
        CHECK(model.getUndoStack().count() == undoBase + 2);
        model.getUndoStack().undo();
        CHECK(data.findState("NewState") != nullptr);
        model.getUndoStack().redo();
    }

    grab(design, "g3-renamed");

    std::printf("sect: move\n");
    // --- Move ---
    placedItem = dynamic_cast<SMStateItem*>(scene.findCanvasItem(placed->getId()));
    CHECK(placedItem != nullptr);
    const int undoBeforeMove = model.getUndoStack().count();
    model.getSelectionModel().setSelection(QList<uint32_t>{ placed->getId() });
    QApplication::processEvents();
    const QPointF oldPos = placedItem->pos();
    // Grab in the body area, below the header band.
    dragScene(view, oldPos + QPointF(40.0, 40.0), oldPos + QPointF(40.0 + 96.0, 40.0 + 64.0));

    const SMLayoutNode* movedNode = data.getLayout().findNode(placed->getId());
    CHECK(movedNode != nullptr);
    CHECK(QPointF(movedNode->x, movedNode->y) != oldPos);
    CHECK(movedNode->x == NESMDesign::snapValue(movedNode->x, scene.getGridSize()));
    CHECK(model.getUndoStack().count() == undoBeforeMove + 1);
    grab(design, "g4-moved");

    model.getUndoStack().undo();
    const SMLayoutNode* backNode = data.getLayout().findNode(placed->getId());
    CHECK((backNode != nullptr) && (QPointF(backNode->x, backNode->y) == oldPos));
    model.getUndoStack().redo();

    std::printf("sect: resize\n");
    // --- Resize ---
    placedItem = dynamic_cast<SMStateItem*>(scene.findCanvasItem(placed->getId()));
    CHECK(placedItem != nullptr);
    movedNode = data.getLayout().findNode(placed->getId());
    const int undoBeforeResize = model.getUndoStack().count();
    const QRectF boxRect{ placedItem->pos(), QSizeF(movedNode->width, movedNode->height) };
    dragScene(view, boxRect.bottomRight(), boxRect.bottomRight() + QPointF(64.0, 32.0));
    const SMLayoutNode* resized = data.getLayout().findNode(placed->getId());
    CHECK((resized != nullptr) && (resized->width > boxRect.width()));
    CHECK(model.getUndoStack().count() == undoBeforeResize + 1);
    model.getUndoStack().undo();
    const SMLayoutNode* unresized = data.getLayout().findNode(placed->getId());
    CHECK((unresized != nullptr) && (unresized->width == boxRect.width()));
    model.getUndoStack().redo();    // pushing while undone would truncate the stack

    std::printf("sect: collapse\n");
    // --- Collapse ---
    // Start/Final marker boxes have no body rows and no chevron; collapse the placed
    // Normal state instead, after giving it a body row (an internal transition).
    SMStateEntry* standby = data.findState("Standby");
    CHECK(standby != nullptr);
    model.getUndoStack().push(new SMCreateTransitionCommand(  data, model.getNotifier(), *standby
                                                            , SMTransitionEntry::eStimulusKind::Trigger, QString()
                                                            , 0u, QList<QPointF>(), QStringLiteral("internal row")));
    QApplication::processEvents();
    placedItem = dynamic_cast<SMStateItem*>(scene.findCanvasItem(standby->getId()));
    CHECK(placedItem != nullptr);
    const SMLayoutNode* rowNode = data.getLayout().findNode(standby->getId());
    CHECK((rowNode != nullptr));
    const int undoBeforeCollapse = model.getUndoStack().count();
    const QRectF colBox{ placedItem->pos(), QSizeF(rowNode->width, rowNode->height) };
    // The chevron sits 18..6 px left of the box's right edge in the header band.
    clickScene(view, QPointF(colBox.right() - 12.0, colBox.top() + 12.0));
    const SMLayoutNode* collapsed = data.getLayout().findNode(standby->getId());
    CHECK((collapsed != nullptr) && collapsed->hasExpanded && (collapsed->expanded == false));
    CHECK(model.getUndoStack().count() == undoBeforeCollapse + 1);
    grab(design, "g5-collapsed");
    model.getUndoStack().undo();
    const SMLayoutNode* expanded = data.getLayout().findNode(standby->getId());
    CHECK((expanded != nullptr) && (expanded->expanded || (expanded->hasExpanded == false)));
    model.getUndoStack().undo();    // remove the temporary internal row again

    std::printf("sect: selection\n");
    // --- Selection sync ---
    scene.clearSelection();
    QApplication::processEvents();
    clickScene(view, offItem->pos() + QPointF(20.0, 10.0));
    CHECK(model.getSelectionModel().isSelected(lightOff->getId()));
    grab(design, "g6-selected");

    std::printf("sect: SM-14 edges render\n");
    // --- Edge items exist for the root-level external transitions ---
    SMStateEntry* onState = data.findState("LightOn");
    CHECK(onState != nullptr);
    SMTransitionEntry* t27 = data.findTransitionById(27);
    CHECK((t27 != nullptr) && t27->isExternal());
    SMEdgeItem* edgeItem27 = dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(27));
    CHECK(edgeItem27 != nullptr);                                            // has stored layout
    CHECK(dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(29)) != nullptr);   // auto-anchored (no layout)
    grab(design, "g7-edges");

    std::printf("sect: SM-14 waypoints\n");
    // --- Waypoint insert then remove on edge 27, double-clicking its true midpoint ---
    // Raise it above the antiparallel edge 29 so the double-click routes to it.
    edgeItem27->setZValue(10.0);
    CHECK(edgeItem27->getPath().size() == 2);
    const QPointF edgeMid = (edgeItem27->getPath().first() + edgeItem27->getPath().last()) / 2.0;
    CHECK(data.getLayout().findEdge(27)->points.size() == 2);

    dblClickScene(view, edgeMid);
    CHECK(data.getLayout().findEdge(27)->points.size() == 3);               // waypoint inserted
    model.getUndoStack().undo();
    CHECK(data.getLayout().findEdge(27)->points.size() == 2);               // insert is undoable
    model.getUndoStack().redo();
    CHECK(data.getLayout().findEdge(27)->points.size() == 3);

    const QPointF inserted = data.getLayout().findEdge(27)->points.at(1);
    dblClickScene(view, inserted);
    CHECK(data.getLayout().findEdge(27)->points.size() == 2);               // waypoint removed
    model.getUndoStack().undo();
    CHECK(data.getLayout().findEdge(27)->points.size() == 3);               // remove is undoable
    model.getUndoStack().redo();
    CHECK(data.getLayout().findEdge(27)->points.size() == 2);
    edgeItem27->setZValue(-1.0);
    grab(design, "g9-waypoints");

    std::printf("sect: SM-14 create transition\n");
    // --- Add Transition tool: click source, click target ---
    scene.clearSelection();
    QApplication::processEvents();
    SMStateItem* onItem = scene.stateItem(onState->getId());
    offItem = scene.stateItem(lightOff->getId());
    CHECK((onItem != nullptr) && (offItem != nullptr));
    const int undoBeforeTx = model.getUndoStack().count();
    const int offTxBefore  = lightOff->getTransitions().getElementCount();
    scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
    clickScene(view, offItem->getBoxGeometry().center());
    clickScene(view, onItem->getBoxGeometry().center());
    CHECK(lightOff->getTransitions().getElementCount() == offTxBefore + 1);
    CHECK(model.getUndoStack().count() == undoBeforeTx + 1);
    CHECK(scene.getActiveTool() == NESMDesign::eCanvasTool::Select);         // single-shot
    const uint32_t newTx = lightOff->getTransitions().getElements().last()->getId();
    CHECK(dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(newTx)) != nullptr);
    model.getUndoStack().undo();
    CHECK(lightOff->getTransitions().getElementCount() == offTxBefore);
    CHECK(scene.findCanvasItem(newTx) == nullptr);                           // edge removed with it
    model.getUndoStack().redo();
    grab(design, "g8-created");

    // Self-transition: click the same state as source and target.
    const int selfBefore = onState->getTransitions().getElementCount();
    scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
    clickScene(view, onItem->getBoxGeometry().center());
    clickScene(view, onItem->getBoxGeometry().center());
    CHECK(onState->getTransitions().getElementCount() == selfBefore + 1);
    const uint32_t selfTx = onState->getTransitions().getElements().last()->getId();
    const SMTransitionEntry* self = data.findTransitionById(selfTx);
    CHECK((self != nullptr) && self->isExternal() && (self->getToId() == onState->getId()));
    CHECK(dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(selfTx)) != nullptr);
    model.getUndoStack().undo();

    // Internal transition (no target): the path the tool takes on Enter / empty drop.
    model.getUndoStack().push(new SMCreateTransitionCommand(  data, model.getNotifier(), *onState
                                                           , SMTransitionEntry::eStimulusKind::Trigger, QString(), 0u
                                                           , QList<QPointF>(), QStringLiteral("internal"), nullptr
                                                           , SMTransitionEntry::eTransitionKind::Internal));
    const uint32_t internalTx = onState->getTransitions().getElements().last()->getId();
    CHECK(data.findTransitionById(internalTx)->isInternal());
    CHECK(scene.findCanvasItem(internalTx) == nullptr);                      // no edge for an internal
    model.getUndoStack().undo();

    std::printf("sect: straight transition follows the pressed/released row\n");
    // --- A straight (no-waypoint) transition drawn by press-drag-release starts where the
    // user pressed on the source and ends where the pointer is released on the target: the
    // endpoint sticks to the facing side but slides along it to the grid-snapped pointer row,
    // rather than snapping to the middle of the vertical borders regardless of the pressed row. ---
    {
        SMStateData* rootLevel = data.findLevel(scene.getLevelId());
        CHECK(rootLevel != nullptr);

        // Grid 10 so the along-edge snap step is 5 (half a cell): the pressed row rounds to a
        // value clearly distinct from the box center row.
        model.getUndoStack().push(new SMSetGridSizeCommand(data, model.getNotifier(), 10, QStringLiteral("grid 10")));
        QApplication::processEvents();
        CHECK(scene.getGridSize() == 10);
        CHECK(scene.isSnapToGrid());

        const QRectF srcBox(2000.0, 2000.0, 160.0, 80.0);   // right = 2160, center y = 2040
        const QRectF tgtBox(2400.0, 2000.0, 160.0, 80.0);   // left  = 2400, center y = 2040
        SMCreateStateCommand* mkSrc = new SMCreateStateCommand(  data, model.getNotifier(), *rootLevel
                                                              , QStringLiteral("PWR_OFF"), SMStateEntry::eStateKind::Normal
                                                              , srcBox, QStringLiteral("src"));
        model.getUndoStack().push(mkSrc);
        SMCreateStateCommand* mkTgt = new SMCreateStateCommand(  data, model.getNotifier(), *rootLevel
                                                              , QStringLiteral("PWR_ON"), SMStateEntry::eStateKind::Normal
                                                              , tgtBox, QStringLiteral("tgt"));
        model.getUndoStack().push(mkTgt);
        const uint32_t srcId = mkSrc->getStateId();
        const uint32_t tgtId = mkTgt->getStateId();
        QApplication::processEvents();

        SMStateItem* srcItem = scene.stateItem(srcId);
        SMStateItem* tgtItem = scene.stateItem(tgtId);
        CHECK((srcItem != nullptr) && (tgtItem != nullptr));

        // Draw at 1:1 with both boxes centered so synthetic scene->viewport->scene mapping is exact.
        view.resetTransform();
        view.centerOn((srcBox.center() + tgtBox.center()) / 2.0);
        QApplication::processEvents();

        const QRectF sbox = srcItem->getBoxGeometry();
        const QRectF tbox = tgtItem->getBoxGeometry();
        const double pressRow = sbox.top() + 22.0;      // 2022 -> snaps to 2020 (not the center 2040)
        const double snapRow  = 2020.0;
        const QPointF from(sbox.right() - 3.0, pressRow);   // inside the source, near its right border
        const QPointF to  (tbox.left()  + 3.0, pressRow);   // inside the target, near its left border

        scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);

        // Drive the gesture by hand (press, moves, release) so the LIVE dashed preview can be
        // inspected mid-drag: the preview's start anchor must already sit at the pressed row, not
        // jump to the border middle while moving and only correct itself once the solid edge is
        // committed on release. This is the bug reported for the transition-in-progress painting.
        const auto previewPath = [&scene]() -> QList<QPointF>
        {
            for (QGraphicsItem* it : scene.items())
            {
                QGraphicsPathItem* p = dynamic_cast<QGraphicsPathItem*>(it);
                if ((p != nullptr) && (p->pen().style() == Qt::DashLine))
                {
                    QList<QPointF> pts;
                    const QPainterPath path = p->path();
                    for (int i = 0; i < path.elementCount(); ++i)
                    {
                        const QPainterPath::Element e = path.elementAt(i);
                        pts.append(QPointF(e.x, e.y));
                    }

                    return pts;
                }
            }

            return QList<QPointF>();
        };

        const QPoint vpFrom = view.mapFromScene(from);
        QMouseEvent pvPress(QEvent::MouseButtonPress, QPointF(vpFrom), view.viewport()->mapToGlobal(vpFrom)
                          , Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &pvPress);
        QApplication::processEvents();

        constexpr int pvSteps{ 8 };
        for (int i = 1; i <= pvSteps; ++i)
        {
            const QPointF sp = from + (to - from) * (static_cast<double>(i) / pvSteps);
            const QPoint  vp = view.mapFromScene(sp);
            QMouseEvent move(QEvent::MouseMove, QPointF(vp), view.viewport()->mapToGlobal(vp)
                             , Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(view.viewport(), &move);
        }

        QApplication::processEvents();

        // Mid-drag: the dashed preview starts on the source's right border at the snapped pressed
        // row, NOT at the box-center row - this is the regression the fix targets.
        grab(design, "g8c-straight-preview-midway");
        const QList<QPointF> livePts = previewPath();
        CHECK(livePts.size() >= 2);
        if (livePts.size() >= 2)
        {
            CHECK(std::abs(livePts.first().x() - sbox.right()) < 1.0);
            CHECK(std::abs(livePts.first().y() - snapRow) < 1.0);
            CHECK(std::abs(livePts.first().y() - sbox.center().y()) > 3.0);   // NOT the mid-border row
        }

        const QPoint vpTo = view.mapFromScene(to);
        QMouseEvent pvRelease(QEvent::MouseButtonRelease, QPointF(vpTo), view.viewport()->mapToGlobal(vpTo)
                            , Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &pvRelease);
        QApplication::processEvents();

        SMStateEntry* srcState = data.findStateById(srcId);
        CHECK((srcState != nullptr) && (srcState->getTransitions().getElementCount() == 1));
        const uint32_t txId = srcState->getTransitions().getElements().last()->getId();
        const SMLayoutEdge* edge = data.getLayout().findEdge(txId);
        CHECK((edge != nullptr) && (edge->points.size() == 2));
        const QPointF begin = edge->points.first();
        const QPointF end   = edge->points.last();

        // Begin clings to the source's right border (facing the target) at the snapped pressed row.
        CHECK(std::abs(begin.x() - sbox.right()) < 1.0);
        CHECK(std::abs(begin.y() - snapRow) < 1.0);
        CHECK(std::abs(begin.y() - sbox.center().y()) > 3.0);   // regression guard: NOT the mid-border row
        // End clings to the target's left border at the same snapped row -> a straight horizontal edge.
        CHECK(std::abs(end.x() - tbox.left()) < 1.0);
        CHECK(std::abs(end.y() - snapRow) < 1.0);
        CHECK(std::abs(begin.y() - end.y()) < 1.0);

        // The drawn polyline reflects the stored anchors (endpoints projected onto the live border).
        SMEdgeItem* edgeItem = dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(txId));
        CHECK((edgeItem != nullptr) && (edgeItem->getPath().size() == 2));
        CHECK(std::abs(edgeItem->getPath().first().y() - snapRow) < 1.0);
        CHECK(std::abs(edgeItem->getPath().last().y()  - snapRow) < 1.0);
        grab(design, "g8b-straight-follows-row");

        model.getUndoStack().undo();    // remove transition
        model.getUndoStack().undo();    // remove target state
        model.getUndoStack().undo();    // remove source state
        model.getUndoStack().undo();    // restore grid size
        QApplication::processEvents();
    }

    std::printf("sect: fix-514 waypoint clicks while creating\n");
    // --- Add Transition: each click on empty canvas drops a polyline waypoint; the
    // gesture continues until a state is clicked (no dialog on empty-canvas clicks) ---
    const int wpTxBefore = lightOff->getTransitions().getElementCount();
    scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
    clickScene(view, offItem->getBoxGeometry().center());
    clickScene(view, offItem->getBoxGeometry().center() + QPointF(0.0, 240.0));  // waypoint 1
    clickScene(view, onItem->getBoxGeometry().center() + QPointF(0.0, 240.0));   // waypoint 2
    clickScene(view, onItem->getBoxGeometry().center());
    CHECK(lightOff->getTransitions().getElementCount() == wpTxBefore + 1);
    const uint32_t polyTx = lightOff->getTransitions().getElements().last()->getId();
    const SMLayoutEdge* polyEdge = data.getLayout().findEdge(polyTx);
    CHECK((polyEdge != nullptr) && (polyEdge->points.size() == 4));          // begin + 2 waypoints + end
    model.getUndoStack().undo();
    CHECK(lightOff->getTransitions().getElementCount() == wpTxBefore);

    std::printf("sect: rectangular self-loop keeps its right angles\n");
    // --- A self-transition drawn as a rectangle out of one side (press the box, out, along, back
    // into the same box) must come back with horizontal and vertical legs: each anchor lines up
    // with its adjacent corner instead of aiming at the box center, which used to bend the first
    // and last legs and turn the rectangle into a trapezoid. Snap-to-grid is off so the hand
    // drift in the corner clicks is not rounded away before the alignment rules see it. ---
    {
        const bool snapBefore = scene.isSnapToGrid();
        scene.setSnapToGrid(false);

        // Both anchors must stay on the straight part of the right side: a border point cannot
        // slide onto a rounded corner, and a clamped anchor would bend the leg it belongs to.
        const QRectF box  = onItem->getBoxGeometry();
        const double rad  = onItem->boxCornerRadius();
        const double outY = box.top() + rad + 8.0;
        const double back = box.bottom() - rad - 8.0;

        // Turn the corners clear of every box, or a corner click would land on a state and end
        // the gesture there instead of dropping a waypoint.
        double corner = box.right() + 120.0;
        for (const SMStateEntry* sibling : level->getElements())
        {
            SMStateItem* item = scene.stateItem(sibling->getId());
            if (item != nullptr)
            {
                corner = std::max(corner, item->getBoxGeometry().right() + 120.0);
            }
        }

        const QPointF w1(corner, outY + 3.0);                       // out, drifting down a little
        const QPointF w2(corner - 3.0, back - 2.0);                 // along, drifting left a little
        const QPointF press(box.right() - rad - 4.0, outY);         // act 1, inside the right side
        const QPointF close(box.right() - rad - 4.0, back);         // act 8, back onto the same box
        CHECK(scene.stateAt(w1) == nullptr);                        // the corners must be on empty canvas
        CHECK(scene.stateAt(w2) == nullptr);
        CHECK(scene.stateAt(press) == onItem);
        CHECK(scene.stateAt(close) == onItem);

        const auto same = [](double a, double b) -> bool { return std::abs(a - b) < 0.5; };
        const int loopBefore = onState->getTransitions().getElementCount();
        scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
        clickScene(view, press);
        clickScene(view, w1);
        clickScene(view, w2);
        clickScene(view, close);

        CHECK(onState->getTransitions().getElementCount() == loopBefore + 1);
        const uint32_t loopTx = onState->getTransitions().getElements().last()->getId();
        const SMTransitionEntry* loop = data.findTransitionById(loopTx);
        CHECK((loop != nullptr) && (loop->getToId() == onState->getId()));       // it is a self-loop
        const SMLayoutEdge* loopEdge = data.getLayout().findEdge(loopTx);
        CHECK((loopEdge != nullptr) && (loopEdge->points.size() == 4));
        if ((loopEdge != nullptr) && (loopEdge->points.size() == 4))
        {
            const QPointF p0 = loopEdge->points.at(0);
            const QPointF p1 = loopEdge->points.at(1);
            const QPointF p2 = loopEdge->points.at(2);
            const QPointF p3 = loopEdge->points.at(3);
            CHECK(same(p0.x(), box.right()));                       // both anchors on the pressed side
            CHECK(same(p3.x(), box.right()));
            CHECK(same(p0.y(), p1.y()));                            // the leg out is horizontal
            CHECK(same(p1.x(), p2.x()));                            // the leg along is vertical
            CHECK(same(p2.y(), p3.y()));                            // the leg back is horizontal
            CHECK(same(p0.y(), outY + 3.0));                        // each anchor followed its corner
            CHECK(same(p3.y(), back - 2.0));
            CHECK(std::abs(p0.y() - p3.y()) > 1.0);                 // neither slid to the box center
        }

        SMEdgeItem* loopItem = dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(loopTx));
        CHECK(loopItem != nullptr);
        if (loopItem != nullptr)
        {
            // What is drawn matches what is stored: the renderer applies the same anchor rule.
            const QList<QPointF>& drawn = loopItem->getPath();
            CHECK(drawn.size() == 4);
            if (drawn.size() == 4)
            {
                CHECK(same(drawn.first().y(), drawn.at(1).y()));
                CHECK(same(drawn.at(2).y(), drawn.last().y()));
            }
        }

        model.getUndoStack().undo();
        CHECK(onState->getTransitions().getElementCount() == loopBefore);
        scene.setSnapToGrid(snapBefore);
        QApplication::processEvents();
    }

    std::printf("sect: fix-514 draw state by drag\n");
    // --- Add State: press-drag-release draws the box between press and release ---
    const int drawnBefore = level->getElementCount();
    scene.setActiveTool(NESMDesign::eCanvasTool::AddState);
    const QPointF drawFrom = offItem->getBoxGeometry().center() + QPointF(-320.0, 320.0);
    dragScene(view, drawFrom, drawFrom + QPointF(200.0, 120.0));
    CHECK(level->getElementCount() == drawnBefore + 1);
    CHECK(scene.getActiveTool() == NESMDesign::eCanvasTool::Select);         // single-shot
    const SMStateEntry* drawn = level->getElements().last();
    const SMLayoutNode* drawnNode = data.getLayout().findNode(drawn->getId());
    CHECK(drawnNode != nullptr);
    CHECK((drawnNode->width >= 184.0) && (drawnNode->height >= 104.0));      // drawn size (snapped)
    clickScene(view, drawFrom + QPointF(-120.0, -120.0));                    // close the rename editor
    model.getUndoStack().undo();
    CHECK(level->getElementCount() == drawnBefore);

    std::printf("sect: fix-514 endpoint glue\n");
    // --- Dragging an endpoint and releasing on empty canvas glues it to the nearest
    // point of its own state's border (no dialog, no revert), one undoable step ---
    SMEdgeItem* glueEdge = dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(27));
    CHECK(glueEdge != nullptr);
    scene.clearSelection();
    glueEdge->setSelected(true);        // selection raises the edge above the boxes
    QApplication::processEvents();
    const QPointF endBefore = glueEdge->getPath().last();
    const QPointF storedEndBefore = data.getLayout().findEdge(27)->points.last();
    // Press just inside the target box: within the endpoint pick radius AND inside the
    // state's border-drag band - the edge handle must win over the border drag.
    dragScene(view, endBefore + QPointF(2.0, 0.0), endBefore + QPointF(60.0, 200.0));
    const SMLayoutEdge* gluedEdge = data.getLayout().findEdge(27);
    CHECK(gluedEdge != nullptr);
    const QPointF endAfter = gluedEdge->points.last();
    CHECK(endAfter != endBefore);                                            // the endpoint moved
    const QRectF tgtBox = scene.stateItem(onState->getId())->getBoxGeometry();
    const QPointF onBorder = NESMDesign::nearestBorderPoint(tgtBox, NESMDesign::StateCornerRadius, endAfter);
    CHECK(std::hypot(endAfter.x() - onBorder.x(), endAfter.y() - onBorder.y()) < 0.5); // glued to the border
    model.getUndoStack().undo();
    const SMLayoutEdge* revertedEdge = data.getLayout().findEdge(27);
    CHECK((revertedEdge != nullptr) && (revertedEdge->points.last() == storedEndBefore));  // undoable
    scene.clearSelection();
    QApplication::processEvents();

    std::printf("sect: SM-14 stimulus\n");
    // --- Stimulus assignment over the shared registry ---
    const uint32_t createdTx = lightOff->getTransitions().getElements().last()->getId();
    model.getUndoStack().push(new SMSetStimulusCommand(  data, model.getNotifier(), createdTx
                                                       , SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("power_on")
                                                       , QStringLiteral("set stimulus")));
    CHECK(data.findTransitionById(createdTx)->getStimulus() == QStringLiteral("power_on"));

    std::printf("sect: SM-14 arc geometry\n");
    // --- Arc renders from two points + bulge (apex bows off the chord) ---
    const QList<QPointF> arc = NESMDesign::arcPolyline(QPointF(0.0, 0.0), QPointF(100.0, 0.0), 0.4, NESMDesign::EdgeArcSamples);
    CHECK(arc.size() == NESMDesign::EdgeArcSamples + 1);
    CHECK(std::abs(arc.at(arc.size() / 2).y()) > 1.0);

    std::printf("sect: SM-14 priority round-trip\n");
    // --- Priority reorder changes document order and survives save/load ---
    SMStateEntry* yellow = data.findState("Yellow");
    CHECK((yellow != nullptr) && (yellow->getTransitions().getElementCount() >= 2));
    const uint32_t firstToBefore = yellow->getTransitions().getElements().at(0)->getToId();
    model.getUndoStack().push(new TDocReorderCommand<SMTransitionEntry*, DocumentElem>(  model.getNotifier(), yellow->getTransitions()
                                                                                       , 0, 1, yellow->getId(), eDocElementKind::Transition
                                                                                       , QStringLiteral("reorder")));
    const uint32_t firstToAfter = yellow->getTransitions().getElements().at(0)->getToId();
    CHECK(firstToAfter != firstToBefore);
    const QString roundtripPath = QDir::tempPath() + QStringLiteral("/sm14_roundtrip.fsml");
    CHECK(model.saveToFile(roundtripPath));
    StateMachineModel reloaded;
    CHECK(reloaded.loadFromFile(roundtripPath));
    SMStateEntry* yellowReloaded = reloaded.getData().findState("Yellow");
    CHECK((yellowReloaded != nullptr) && (yellowReloaded->getTransitions().getElements().at(0)->getToId() == firstToAfter));

    std::printf("sect: SM-14 reconnect (target + source)\n");
    // --- Target-endpoint reconnection retargets `To`, undoable ---
    const uint32_t lightOffId  = data.findState("LightOff")->getId();
    const uint32_t to27Before  = data.findTransitionById(27)->getToId();
    model.getUndoStack().push(new SMSetTransitionTargetCommand(  data, model.getNotifier(), 27
                                                              , lightOffId, QStringLiteral("retarget")));
    CHECK(data.findTransitionById(27)->getToId() == lightOffId);
    model.getUndoStack().undo();
    CHECK(data.findTransitionById(27)->getToId() == to27Before);
    model.getUndoStack().redo();
    model.getUndoStack().undo();     // leave 27 as it was

    // --- Source-endpoint reconnection moves the transition to a new owner, undoable ---
    const int onCountBefore = onState->getTransitions().getElementCount();
    model.getUndoStack().push(new SMReparentTransitionCommand(  data, model.getNotifier()
                                                             , *lightOff, *onState, createdTx, QStringLiteral("reparent")));
    CHECK(lightOff->getTransitions().findElement(createdTx) == nullptr);         // left the old owner
    CHECK(onState->getTransitions().getElementCount() == onCountBefore + 1);     // joined the new owner
    model.getUndoStack().undo();
    CHECK(lightOff->getTransitions().findElement(createdTx) != nullptr);         // restored under its old ID
    CHECK(onState->getTransitions().getElementCount() == onCountBefore);

    std::printf("sect: SM-15 level path\n");
    SMSceneManager& manager = design.getSceneManager();
    const uint32_t rootLevel = manager.getRootLevel();
    CHECK(manager.getCurrentLevel() == rootLevel);
    SMStateEntry* function = data.findState("Function");
    CHECK(function != nullptr);
    CHECK(data.getLevelPath(rootLevel) == (QList<uint32_t>{ rootLevel }));
    CHECK(data.getLevelPath(function->getId()) == (QList<uint32_t>{ rootLevel, onState->getId(), function->getId() }));
    CHECK(data.getLevelPath(9999u).isEmpty());

    std::printf("sect: SM-15 enter via double-click\n");
    // --- Double-click a painted composite descends; the breadcrumb shows the path ---
    design.getScene().clearSelection();
    SMStateItem* compositeItem = design.getScene().stateItem(onState->getId());
    CHECK(compositeItem != nullptr);
    dblClickScene(view, compositeItem->getBoxGeometry().center());
    CHECK(manager.getCurrentLevel() == onState->getId());
    CHECK(&design.getScene() != &scene);                                     // level scene swapped in
    CHECK(model.getSelectionModel().getActiveLevel() == onState->getId());
    SMStateEntry* initialize = data.findState("Initialize");
    CHECK((initialize != nullptr) && (design.getScene().stateItem(initialize->getId()) != nullptr));
    QList<QToolButton*> crumbs = design.getBreadcrumb()->findChildren<QToolButton*>();
    CHECK(crumbs.size() == 1);                                               // one clickable ancestor
    CHECK(crumbs.first()->text() == QStringLiteral("TrafficLight"));
    grab(design, "g10-sublevel");

    std::printf("sect: SM-15 enter via Enter key\n");
    design.getScene().clearSelection();
    SMStateItem* functionItem = design.getScene().stateItem(function->getId());
    CHECK(functionItem != nullptr);
    functionItem->setSelected(true);
    keyClickScene(design.getScene(), Qt::Key_Return);
    CHECK(manager.getCurrentLevel() == function->getId());
    CHECK(design.getBreadcrumb()->findChildren<QToolButton*>().size() == 2); // two clickable ancestors

    std::printf("sect: SM-15 per-level viewport\n");
    // --- The zoom set here persists into the level's View entry and restores on re-entry ---
    view.setZoom(150);
    QApplication::processEvents();
    SMLayoutView* functionView = data.getLayout().findView(function->getId());
    CHECK((functionView != nullptr) && (functionView->zoom == 150));

    keyClickScene(design.getScene(), Qt::Key_Backspace);                     // to LightOn
    CHECK(manager.getCurrentLevel() == onState->getId());
    CHECK(view.getZoom() == 100);                                            // LightOn's own View entry

    CHECK(manager.navigateTo(function->getId()));
    CHECK(view.getZoom() == 150);                                            // restored on re-entry

    // Persisted across save/reload.
    const QString viewportPath = QDir::tempPath() + QStringLiteral("/sm15_viewport.fsml");
    CHECK(model.saveToFile(viewportPath));
    {
        StateMachineData reopened;
        CHECK(reopened.readFromFile(viewportPath));
        SMLayoutView* reopenedView = reopened.getLayout().findView(function->getId());
        CHECK((reopenedView != nullptr) && (reopenedView->zoom == 150));
    }

    std::printf("sect: SM-15 leave via Alt+double-click and breadcrumb\n");
    dblClickScene(view, QPointF(-4000.0, -4000.0), Qt::AltModifier);         // empty canvas
    CHECK(manager.getCurrentLevel() == onState->getId());
    QList<QToolButton*> rootCrumbs = design.getBreadcrumb()->findChildren<QToolButton*>();
    CHECK(rootCrumbs.size() == 1);
    rootCrumbs.first()->click();                                             // ancestor click
    QApplication::processEvents();
    CHECK(manager.getCurrentLevel() == rootLevel);
    CHECK(design.actionGoToParent()->isEnabled() == false);                  // root has no parent

    std::printf("sect: SM-15 add substate (painted)\n");
    // --- Convert a plain state: nested list + auto Start + Node layout, one undo step ---
    // Start and Final states can never become composite.
    // The Start is the level's pseudo-state, not the first ordinary state: this used to name
    // `LightOff`, which carried Kind="Start" before the pseudo-state form.
    const SMStateData* addLevel = data.findLevel(design.getScene().getLevelId());
    const SMStateEntry* addStart = (addLevel != nullptr ? addLevel->getStartState() : nullptr);
    CHECK(addStart != nullptr);
    design.getScene().clearSelection();
    if (addStart != nullptr)
    {
        design.getScene().stateItem(addStart->getId())->setSelected(true);
        CHECK(design.actionAddSubstate()->isEnabled() == false);
    }

    standby = data.findState("Standby");
    CHECK((standby != nullptr) && (standby->hasNestedStates() == false));
    design.getScene().clearSelection();
    design.getScene().stateItem(standby->getId())->setSelected(true);
    CHECK(design.actionAddSubstate()->isEnabled());
    CHECK(design.actionEnterSubmachine()->isEnabled());                      // fix #514: enter creates a submachine on the fly

    const int undoBeforeConvert = model.getUndoStack().count();
    design.actionAddSubstate()->trigger();
    CHECK(model.getUndoStack().count() == undoBeforeConvert + 1);            // one undo step
    CHECK(standby->hasNestedStates());
    CHECK(standby->getNestedStates()->getElementCount() == 1);               // exactly one state
    SMStateEntry* standbyStart = standby->getNestedStates()->getStartState();
    CHECK((standbyStart != nullptr) && (standbyStart->getKind() == SMStateEntry::eStateKind::Start));
    CHECK(data.findState(standbyStart->getName()) == standbyStart);          // document-unique name
    CHECK(data.getLayout().findNode(standbyStart->getId()) != nullptr);      // placed on the new level
    CHECK(manager.getCurrentLevel() == standby->getId());                    // entered the new level

    // Undo while inside: the level dies, navigation falls back to the parent.
    const uint32_t standbyStartId = standbyStart->getId();
    model.getUndoStack().undo();
    CHECK(standby->hasNestedStates() == false);
    CHECK(manager.getCurrentLevel() == rootLevel);
    model.getUndoStack().redo();
    CHECK(standby->hasNestedStates());
    CHECK(standby->getNestedStates()->getStartState()->getId() == standbyStartId); // same ID replayed

    std::printf("sect: SM-15 arbitrary depth\n");
    // --- Convert a state inside the new submachine: depth grows, one Start per level ---
    manager.enterSubmachine(standby->getId());
    SMCreateStateCommand* createDeep = new SMCreateStateCommand(  data, model.getNotifier(), *standby->getNestedStates()
                                                                , QStringLiteral("DeepChild"), SMStateEntry::eStateKind::Normal
                                                                , QRectF(320.0, 140.0, 160.0, 96.0), QStringLiteral("add"));
    model.getUndoStack().push(createDeep);
    const uint32_t deepId = createDeep->getStateId();
    model.getSelectionModel().setSelection(QList<uint32_t>{ deepId });
    CHECK(design.actionAddSubstate()->isEnabled());
    design.actionAddSubstate()->trigger();
    SMStateEntry* deep = data.findStateById(deepId);
    CHECK((deep != nullptr) && deep->hasNestedStates());
    CHECK(manager.getCurrentLevel() == deepId);
    CHECK(data.getLevelPath(deepId).size() == 3);                            // root -> Standby -> DeepChild
    SMStateEntry* deepStart = deep->getNestedStates()->getStartState();
    CHECK((deepStart != nullptr) && (deepStart->getName() != standbyStart->getName()));
    CHECK(design.getBreadcrumb()->findChildren<QToolButton*>().size() == 2);
    CHECK(design.actionGoToParent()->isEnabled());
    grab(design, "g11-deep-level");

    std::printf("sect: SM-15 recursive composite delete\n");
    // --- Deleting the composite removes its painted subtree as one undo step ---
    manager.navigateTo(rootLevel);
    const int statesBefore = data.getStateCount();
    const uint32_t deepStartId = deepStart->getId();
    const int undoBeforeDelete = model.getUndoStack().count();
    model.getUndoStack().push(new SMRemoveStateCommand(  data, model.getNotifier(), data.getStates()
                                                       , standby->getId(), QStringLiteral("delete composite")));
    CHECK(model.getUndoStack().count() == undoBeforeDelete + 1);             // one undo step
    CHECK(data.findState("Standby") == nullptr);
    CHECK(data.findStateById(deepId) == nullptr);                            // recursive
    CHECK(data.findStateById(deepStartId) == nullptr);
    CHECK(data.getLayout().findNode(standbyStartId) == nullptr);             // subtree layout gone
    model.getUndoStack().undo();
    CHECK(data.getStateCount() == statesBefore);
    CHECK(data.findState("Standby") != nullptr);
    CHECK(data.findStateById(deepId) != nullptr);
    CHECK(data.findStateById(deepStartId) != nullptr);
    CHECK(data.getLayout().findNode(standbyStartId) != nullptr);
    grab(design, "g12-root-after-levels");

    const QString sourcePath{ QString::fromLocal8Bit(argv[1]) };

    std::printf("sect: SM-16 layout round-trip\n");
    {
        const QString savedOnce{ QDir::tempPath() + QStringLiteral("/sm16_once.fsml") };
        const QString savedTwice{ QDir::tempPath() + QStringLiteral("/sm16_twice.fsml") };

        StateMachineModel first;
        CHECK(first.loadFromFile(sourcePath));
        CHECK(SMAutoPlacer::missingNodes(first.getData()).isEmpty());        // reference doc is fully laid out
        CHECK(first.saveToFile(savedOnce));

        StateMachineModel second;
        CHECK(second.loadFromFile(savedOnce));
        CHECK(second.saveToFile(savedTwice));
        CHECK(fileBytes(savedOnce) == fileBytes(savedTwice));

        const SMLayoutData& source = first.getData().getLayout();
        const SMLayoutData& reread = second.getData().getLayout();
        CHECK(source.getGridSize() == reread.getGridSize());
        CHECK(source.isGridVisible() == reread.isGridVisible());

        bool nodesMatch = (source.getNodes().size() == reread.getNodes().size());
        for (const SMLayoutNode& node : source.getNodes())
        {
            const SMLayoutNode* other = reread.findNode(node.owner);
            nodesMatch = nodesMatch && (other != nullptr)
                         && (other->x == node.x) && (other->y == node.y)
                         && (other->width == node.width) && (other->height == node.height)
                         && (other->color == node.color) && (other->headerColor == node.headerColor)
                         && (other->hasExpanded == node.hasExpanded) && (other->expanded == node.expanded);
        }
        CHECK(nodesMatch);

        bool edgesMatch = (source.getEdges().size() == reread.getEdges().size());
        for (const SMLayoutEdge& edge : source.getEdges())
        {
            const SMLayoutEdge* other = reread.findEdge(edge.owner);
            edgesMatch = edgesMatch && (other != nullptr)
                         && (other->shape == edge.shape) && (other->bulge == edge.bulge)
                         && (other->color == edge.color) && (other->points == edge.points)
                         && (other->hasLabel == edge.hasLabel) && (other->label == edge.label);
        }
        CHECK(edgesMatch);

        bool viewsMatch = (source.getViews().size() == reread.getViews().size());
        for (const SMLayoutView& entry : source.getViews())
        {
            const SMLayoutView* other = reread.findView(entry.owner);
            viewsMatch = viewsMatch && (other != nullptr) && (other->zoom == entry.zoom)
                         && (other->x == entry.x) && (other->y == entry.y);
        }
        CHECK(viewsMatch);
    }

    std::printf("sect: SM-16 auto-placement of a stripped document\n");
    {
        QByteArray text{ fileBytes(sourcePath) };
        const int begin = text.indexOf("<Layout");
        const int end   = text.indexOf("</Layout>");
        CHECK((begin > 0) && (end > begin));
        text.remove(begin, end + 9 - begin);

        const QString strippedPath{ QDir::tempPath() + QStringLiteral("/sm16_stripped.fsml") };
        CHECK(writeBytes(strippedPath, text));

        StateMachineModel stripped;
        CHECK(stripped.loadFromFile(strippedPath));
        CHECK(stripped.getData().getLayout().getNodes().isEmpty());
        CHECK(stripped.getData().getLayout().getEdges().isEmpty());

        SMDesign page(stripped);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& blank = stripped.getData();
        CHECK(levelsArePlaced(blank.getStates(), blank.getLayout()));
        CHECK(stripped.getUndoStack().count() >= 1);

        // Every root box exists, is non-empty, and sits where its Node entry says.
        SMScene& blankScene = page.getScene();
        const SMStateData* rootStates = blank.findLevel(blankScene.getLevelId());
        CHECK(rootStates != nullptr);
        bool boxesVisible = true;
        for (const SMStateEntry* state : rootStates->getElements())
        {
            SMStateItem* box = dynamic_cast<SMStateItem*>(blankScene.findCanvasItem(state->getId()));
            const SMLayoutNode* node = blank.getLayout().findNode(state->getId());
            boxesVisible = boxesVisible && (box != nullptr) && (node != nullptr)
                           && (box->getBoxGeometry().isEmpty() == false)
                           && (box->pos() == QPointF(node->x, node->y));
        }
        CHECK(boxesVisible);
        grab(page, "g13-auto-placed");

        // The placement is an ordinary edit: rolling the stack back leaves the document bare.
        stripped.getUndoStack().setIndex(0);
        CHECK(blank.getLayout().getNodes().isEmpty());
    }

    std::printf("sect: SM-16 delete/undo layout identity\n");
    {
        const QString baseline{ QDir::tempPath() + QStringLiteral("/sm16_baseline.fsml") };
        const QString resaved{ QDir::tempPath() + QStringLiteral("/sm16_resaved.fsml") };

        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        CHECK(doc.saveToFile(baseline));

        StateMachineData& docData = doc.getData();
        SMStateEntry* victim = docData.findState("LightOn");
        CHECK(victim != nullptr);
        const uint32_t victimId = victim->getId();

        doc.getUndoStack().push(new SMRemoveStateCommand(  docData, doc.getNotifier(), docData.getStates()
                                                         , victimId, QStringLiteral("delete state")));
        CHECK(docData.getLayout().findNode(victimId) == nullptr);
        CHECK(docData.getLayout().findView(victimId) == nullptr);           // its sublevel viewport too

        doc.getUndoStack().undo();
        CHECK(docData.getLayout().findNode(victimId) != nullptr);
        CHECK(docData.getLayout().findView(victimId) != nullptr);
        CHECK(doc.saveToFile(resaved));
        CHECK(fileBytes(baseline) == fileBytes(resaved));
    }

    std::printf("sect: SM-17 notes\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& pageData  = doc.getData();
        SMScene&          pageScene = page.getScene();
        SMGraphicsView&   pageView  = page.getView();

        // --- Place: click with the Add Note tool creates a note and opens inline edit ---
        const int notesBefore = pageData.getLayout().getNotes().size();
        const int undoBase    = doc.getUndoStack().count();
        pageScene.setActiveTool(NESMDesign::eCanvasTool::AddNote);
        const QPointF dropPos{ pageScene.contentBounds().left() - 260.0, pageScene.contentBounds().top() };
        clickScene(pageView, dropPos);

        CHECK(pageData.getLayout().getNotes().size() == notesBefore + 1);
        CHECK(doc.getUndoStack().count() == undoBase + 1);                      // one undo step
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::Select);     // single-shot

        const uint32_t noteId = pageData.getLayout().getNotes().last().id;
        SMNoteItem* noteItem = pageScene.noteItem(noteId);
        CHECK((noteItem != nullptr) && noteItem->isEditActive());

        // --- Inline (multi-line) text edit: commits on focus-out, not Enter ---
        QPlainTextEdit* editor = nullptr;
        for (QGraphicsItem* item : pageScene.items())
        {
            QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
            if ((proxy != nullptr) && (editor = qobject_cast<QPlainTextEdit*>(proxy->widget())) != nullptr)
            {
                break;
            }
        }

        CHECK(editor != nullptr);
        if (editor != nullptr)
        {
            editor->setPlainText(QStringLiteral("line one\nline two"));
            // A proxy-embedded widget's Qt-focus-chain interaction with clearFocus() is not
            // reliable offscreen; deliver the FocusOut event directly, exactly as a real
            // focus change would, to exercise NoteEdit::focusOutEvent deterministically.
            QFocusEvent focusOut(QEvent::FocusOut, Qt::OtherFocusReason);
            QApplication::sendEvent(editor, &focusOut);
            QApplication::processEvents();
        }

        CHECK((noteItem != nullptr) && (noteItem->isEditActive() == false));
        const SMLayoutNote* committed = pageData.getLayout().findNote(noteId);
        CHECK((committed != nullptr) && (committed->text == QStringLiteral("line one\nline two")));
        CHECK(doc.getUndoStack().count() == undoBase + 2);                      // place + text commit

        // --- Move ---
        const QRectF beforeMove = noteItem->getBoxGeometry();
        dragScene(pageView, beforeMove.center(), beforeMove.center() + QPointF(90.0, 60.0));
        const SMLayoutNote* movedNote = pageData.getLayout().findNote(noteId);
        CHECK((movedNote != nullptr) && (QPointF(movedNote->x, movedNote->y) != beforeMove.topLeft()));

        // --- Resize via the single corner handle (only hit-testable while selected) ---
        pageScene.clearSelection();
        noteItem->setSelected(true);
        QApplication::processEvents();
        CHECK(noteItem->isSelected());
        const QRectF beforeResize = noteItem->getBoxGeometry();
        const QPointF handlePos = beforeResize.bottomRight() - QPointF(2.0, 2.0);
        dragScene(pageView, handlePos, handlePos + QPointF(48.0, 32.0));
        CHECK(noteItem->getBoxGeometry().size() != beforeResize.size());        // diagnostic: item-level resize
        const SMLayoutNote* resizedNote = pageData.getLayout().findNote(noteId);
        CHECK((resizedNote != nullptr) && (resizedNote->width > beforeResize.width())
              && (resizedNote->height > beforeResize.height()));

        // --- Color ---
        const int undoBeforeColor = doc.getUndoStack().count();
        doc.getUndoStack().push(new SMSetNoteColorCommand(  pageData, doc.getNotifier()
                                                           , noteId, QStringLiteral("#FFCC00"), QStringLiteral("note color")));
        CHECK(pageData.getLayout().findNote(noteId)->color == QStringLiteral("#FFCC00"));
        CHECK(doc.getUndoStack().count() == undoBeforeColor + 1);
        doc.getUndoStack().undo();
        CHECK(pageData.getLayout().findNote(noteId)->color.isEmpty());

        // --- Delete + undo, round-trip ---
        // (Not asserting stack count here: the preceding undo() left a pending redo entry
        // that this push legitimately truncates - QUndoStack::count() never shrinks on its
        // own from undo(), only a following push() can change it.)
        doc.getUndoStack().push(new SMRemoveNoteCommand(pageData, doc.getNotifier(), noteId, QStringLiteral("delete note")));
        CHECK(pageData.getLayout().findNote(noteId) == nullptr);
        doc.getUndoStack().undo();
        const SMLayoutNote* restoredNote = pageData.getLayout().findNote(noteId);
        CHECK((restoredNote != nullptr) && (restoredNote->text == QStringLiteral("line one\nline two")));

        const QString savedNotes{ QDir::tempPath() + QStringLiteral("/sm17_notes.fsml") };
        CHECK(doc.saveToFile(savedNotes));
        StateMachineModel reloadedNotes;
        CHECK(reloadedNotes.loadFromFile(savedNotes));
        const SMLayoutNote* reread = reloadedNotes.getData().getLayout().findNote(noteId);
        CHECK((reread != nullptr) && (reread->text == QStringLiteral("line one\nline two"))
              && (reread->level == restoredNote->level));

        grab(page, "g14-note");

        // --- Deleting a composite state removes the notes drawn on its sublevel too ---
        SMStateEntry* composite = pageData.findState(QStringLiteral("LightOn"));
        CHECK((composite != nullptr) && composite->hasNestedStates());
        if ((composite != nullptr) && composite->hasNestedStates())
        {
            const uint32_t compositeId = composite->getId();
            page.getSceneManager().enterSubmachine(compositeId);
            SMScene& subScene = page.getScene();
            subScene.setActiveTool(NESMDesign::eCanvasTool::AddNote);
            clickScene(page.getView(), subScene.contentBounds().topLeft() - QPointF(160.0, 0.0));
            const uint32_t subNoteId = pageData.getLayout().getNotes().last().id;
            CHECK(pageData.getLayout().findNote(subNoteId) != nullptr);

            page.getSceneManager().navigateTo(page.getSceneManager().getRootLevel());
            StateMachineData& rootData = pageData;
            SMStateData* rootLevelPtr = rootData.findLevel(page.getScene().getLevelId());
            CHECK(rootLevelPtr != nullptr);
            doc.getUndoStack().push(new SMRemoveStateCommand(  rootData, doc.getNotifier(), *rootLevelPtr
                                                             , compositeId, QStringLiteral("delete composite")));
            CHECK(pageData.getLayout().findNote(subNoteId) == nullptr);         // sublevel note deleted with it
            doc.getUndoStack().undo();
            CHECK(pageData.getLayout().findNote(subNoteId) != nullptr);         // restored with the composite
        }
    }

    std::printf("sect: SM-17 colors\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        StateMachineData& pageData = doc.getData();

        const SMStateData* rootLevel2 = pageData.findLevel(pageData.getOverview().getId());
        CHECK((rootLevel2 != nullptr) && (rootLevel2->getElementCount() >= 2));
        QList<uint32_t> stateIds;
        for (const SMStateEntry* st : rootLevel2->getElements())
        {
            stateIds.append(st->getId());
        }

        // Multi-selection color-apply is one undo step; header shade stays derived
        // (SMStateItem::updateFromModel already reads Color and derives the header).
        const int undoBase = doc.getUndoStack().count();
        SMCompositeCommand* composite = new SMCompositeCommand(pageData, doc.getNotifier(), QStringLiteral("color"));
        new SMSetNodeColorCommand(pageData, doc.getNotifier(), stateIds.at(0), QStringLiteral("#3366CC"), QStringLiteral("color"), composite);
        new SMSetNodeColorCommand(pageData, doc.getNotifier(), stateIds.at(1), QStringLiteral("#3366CC"), QStringLiteral("color"), composite);
        doc.getUndoStack().push(composite);
        CHECK(doc.getUndoStack().count() == undoBase + 1);                      // one undo step
        CHECK(pageData.getLayout().findNode(stateIds.at(0))->color == QStringLiteral("#3366CC"));
        CHECK(pageData.getLayout().findNode(stateIds.at(1))->color == QStringLiteral("#3366CC"));
        CHECK(pageData.getLayout().findNode(stateIds.at(0))->headerColor.isEmpty()); // derived, not stored
        doc.getUndoStack().undo();
        CHECK(pageData.getLayout().findNode(stateIds.at(0))->color.isEmpty());

        // Edge color, analogous.
        SMTransitionEntry* transition = nullptr;
        for (const SMStateEntry* st : rootLevel2->getElements())
        {
            for (SMTransitionEntry* tr : st->getTransitions().getElements())
            {
                if (tr->isExternal())
                {
                    transition = tr;
                    break;
                }
            }
            if (transition != nullptr)
            {
                break;
            }
        }

        CHECK(transition != nullptr);
        if (transition != nullptr)
        {
            const uint32_t transitionId = transition->getId();
            doc.getUndoStack().push(new SMSetEdgeColorCommand(  pageData, doc.getNotifier()
                                                               , transitionId, QStringLiteral("#CC3333"), QStringLiteral("edge color")));
            CHECK(pageData.getLayout().findEdge(transitionId)->color == QStringLiteral("#CC3333"));
            doc.getUndoStack().undo();
            CHECK(pageData.getLayout().findEdge(transitionId)->color.isEmpty());
        }
    }

    std::printf("sect: SM-17 align and distribute\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& pageData = doc.getData();

        // The root level only has two states (LightOff/LightOn); descend to "Function"
        // (StartCycle/Yellow/Red/Green) for a level with enough states to distribute.
        SMStateEntry* lightOn = pageData.findState(QStringLiteral("LightOn"));
        CHECK((lightOn != nullptr) && lightOn->hasNestedStates());
        page.getSceneManager().enterSubmachine(lightOn->getId());
        SMStateEntry* function = pageData.findState(QStringLiteral("Function"));
        CHECK((function != nullptr) && function->hasNestedStates());
        page.getSceneManager().enterSubmachine(function->getId());

        SMScene& pageScene = page.getScene();
        const SMStateData* funcLevel = pageData.findLevel(function->getId());
        CHECK((funcLevel != nullptr) && (funcLevel->getElementCount() >= 3));

        QList<uint32_t> ids;
        for (const SMStateEntry* st : funcLevel->getElements())
        {
            ids.append(st->getId());
            if (ids.size() == 3)
            {
                break;
            }
        }
        CHECK(ids.size() == 3);

        pageScene.clearSelection();
        for (uint32_t id : ids)
        {
            SMStateItem* item = pageScene.stateItem(id);
            if (item != nullptr)
            {
                item->setSelected(true);
            }
        }

        QList<double> originalX;
        for (uint32_t id : ids)
        {
            originalX.append(pageData.getLayout().findNode(id)->x);
        }

        CHECK(page.actionAlignLeft()->isEnabled());
        const int undoBase = doc.getUndoStack().count();
        page.actionAlignLeft()->trigger();
        CHECK(doc.getUndoStack().count() == undoBase + 1);                      // one undo step
        const double leftX = pageData.getLayout().findNode(ids.first())->x;
        bool allAligned = true;
        for (uint32_t id : ids)
        {
            allAligned = allAligned && (pageData.getLayout().findNode(id)->x == leftX);
        }
        CHECK(allAligned);
        doc.getUndoStack().undo();
        bool restored = true;
        for (int i = 0; i < ids.size(); ++i)
        {
            restored = restored && (pageData.getLayout().findNode(ids.at(i))->x == originalX.at(i));
        }
        CHECK(restored);                                                        // undo restores each original X

        // Distribute horizontally: first/last keep their center, one undo step overall.
        pageScene.clearSelection();
        for (uint32_t id : ids)
        {
            SMStateItem* item = pageScene.stateItem(id);
            if (item != nullptr)
            {
                item->setSelected(true);
            }
        }

        CHECK(page.actionDistributeHorizontal()->isEnabled());
        const int undoBeforeDistribute = doc.getUndoStack().count();
        page.actionDistributeHorizontal()->trigger();
        CHECK(doc.getUndoStack().count() <= undoBeforeDistribute + 1);          // zero or one undo step
        grab(page, "g15-aligned");
    }

    std::printf("sect: SM-17 grid settings persist\n");
    {
        const QString savedGrid{ QDir::tempPath() + QStringLiteral("/sm17_grid.fsml") };

        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        const int  originalSize    = doc.getData().getLayout().getGridSize();
        const bool originalVisible = doc.getData().getLayout().isGridVisible();
        const int  newSize         = (originalSize == 24 ? 32 : 24);

        doc.getUndoStack().push(new SMSetGridSizeCommand(doc.getData(), doc.getNotifier(), newSize, QStringLiteral("grid size")));
        doc.getUndoStack().push(new SMSetGridVisibleCommand(doc.getData(), doc.getNotifier(), !originalVisible, QStringLiteral("grid visible")));
        CHECK(doc.getData().getLayout().getGridSize() == newSize);
        CHECK(doc.getData().getLayout().isGridVisible() == !originalVisible);
        CHECK(doc.saveToFile(savedGrid));

        StateMachineModel reloaded;
        CHECK(reloaded.loadFromFile(savedGrid));
        CHECK(reloaded.getData().getLayout().getGridSize() == newSize);
        CHECK(reloaded.getData().getLayout().isGridVisible() == !originalVisible);

        // Undo restores the original settings.
        doc.getUndoStack().undo();
        CHECK(doc.getData().getLayout().isGridVisible() == originalVisible);
        doc.getUndoStack().undo();
        CHECK(doc.getData().getLayout().getGridSize() == originalSize);

        // SMDesign re-syncs the scene and the checked action from the document on every
        // layout change (grid commands included) - this is what makes undo/redo of a grid
        // change keep the canvas and the toolbar's Show Grid state correct.
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();
        doc.getUndoStack().redo();
        doc.getUndoStack().redo();
        CHECK(page.getScene().getGridSize() == newSize);
        CHECK(page.actionToggleGrid()->isChecked() == !originalVisible);
    }

    std::printf("sect: SM-18 toolbar/menu/shortcut parity\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        // Every canvas action exists exactly once and is reachable without the toolbar:
        // hiding it must not remove any action from the page (spec 9.3 rule 3).
        CHECK(page.isToolbarVisible());
        page.setToolbarVisible(false);
        CHECK(page.isToolbarVisible() == false);
        const QList<QAction*> allActions = page.actions();
        CHECK(allActions.contains(page.actionAddState()));
        CHECK(allActions.contains(page.actionAddNote()));
        CHECK(allActions.contains(page.actionStateColor()));
        CHECK(allActions.contains(page.actionAlignLeft()));
        CHECK(allActions.contains(page.actionUndo()));
        CHECK(page.declareActions().size() == 8);
        page.setToolbarVisible(true);
        CHECK(page.isToolbarVisible());

        // Toolbar-free operation (spec 9.3): every toolbar command must also be in the Design
        // menu. Undo/Redo/Cut/Copy/Paste/Select All belong to the Edit menu, and the toolbar's
        // selection-aware "Set Color..." is presented there as its three explicit variants.
        {
            QMenu designMenu;
            page.populateDesignMenu(designMenu);
            QSet<QAction*> inMenu;
            for (QAction* action : designMenu.actions())
            {
                if (action->menu() != nullptr)
                {
                    for (QAction* sub : action->menu()->actions())
                        inMenu.insert(sub);
                }
                else
                {
                    inMenu.insert(action);
                }
            }

            for (const SMDesign::ToolGroup& group : page.toolGroups())
            {
                if (group.title == SMDesign::tr("Edit"))
                    continue;

                for (QAction* action : group.actions)
                {
                    if (action == page.actionSetColor())
                        continue;
                    if (inMenu.contains(action) == false)
                        std::printf("  design menu misses '%s'\n", action->text().toUtf8().constData());
                    CHECK(inMenu.contains(action));
                }
            }
        }

        // Shortcuts S/F/T/N activate the placement tools while the canvas has focus.
        SMGraphicsView& pageView = page.getView();
        pageView.setFocus();
        QApplication::processEvents();
        keyClick(&pageView, Qt::Key_N);
        CHECK(page.getScene().getActiveTool() == NESMDesign::eCanvasTool::AddNote);
        keyClick(&pageView, Qt::Key_Escape);
        CHECK(page.getScene().getActiveTool() == NESMDesign::eCanvasTool::Select);

        // Context-sensitive menus are built on demand from SMGraphicsView::contextMenuEvent,
        // which emits signalContextMenuRequested (a QGraphicsView routes context-menu events
        // through contextMenuEvent(), so a viewport CustomContextMenu policy never fires). The
        // page's menu is modal (QMenu::exec), so verify the view's plumbing on a standalone view
        // instead of the shown content.
        {
            // Expose the protected override so the emission is tested deterministically,
            // free of the view/viewport event-routing that delivers it in the live app.
            struct ProbeView : SMGraphicsView { using SMGraphicsView::contextMenuEvent; };
            ProbeView probe;
            bool emitted = false;
            QPoint at;
            QObject::connect(&probe, &SMGraphicsView::signalContextMenuRequested, &probe,
                             [&emitted, &at](const QPoint& pos) { emitted = true; at = pos; });
            QContextMenuEvent ctx(QContextMenuEvent::Mouse, QPoint(10, 12));
            probe.contextMenuEvent(&ctx);
            CHECK(emitted);
            CHECK(at == QPoint(10, 12));         // the viewport position is forwarded verbatim
            CHECK(ctx.isAccepted());
        }
    }

    std::printf("sect: SM-19 issue #514 toolbar order, markers, center machine\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        // Toolbar groups: Design first (state -> transition -> note -> final, the same order
        // as the canvas context menu), Declare second with icons on every entry. There is no
        // Add Start State button: every level is born with its Start and cannot lose it.
        const QList<SMDesign::ToolGroup> groups = page.toolGroups();
        CHECK(groups.size() == 8);
        CHECK(groups.at(0).title == QStringLiteral("Design"));
        const QList<QAction*> designOrder{ page.actionAddState(), page.actionAddTransition()
                                         , page.actionAddInternal()
                                         , page.actionAddNote(), page.actionAddFinalState() };
        CHECK(groups.at(0).actions == designOrder);
        CHECK(groups.at(1).title == QStringLiteral("Declare"));
        CHECK(groups.at(1).actions == page.declareActions());
        bool declareIcons = true;
        for (QAction* action : groups.at(1).actions)
        {
            declareIcons = declareIcons && (action->icon().isNull() == false);
        }
        CHECK(declareIcons);
        CHECK(groups.at(3).title == QStringLiteral("Navigate"));
        CHECK(groups.at(3).actions.contains(page.actionCenterMachine()));

        // The unbound Design Toolbar tab shows the same structure as disabled stand-ins.
        QObject owner;
        const QList<SMDesign::ToolGroup> standIns = SMDesign::placeholderToolGroups(owner);
        CHECK(standIns.size() == groups.size());
        bool mirrored = true;
        bool inactive = true;
        bool withIcon = true;
        for (int i = 0; i < standIns.size(); ++i)
        {
            mirrored = mirrored && (standIns.at(i).title == groups.at(i).title)
                                && (standIns.at(i).actions.size() == groups.at(i).actions.size());
            for (QAction* action : standIns.at(i).actions)
            {
                inactive = inactive && (action->isEnabled() == false);
                withIcon = withIcon && (action->icon().isNull() == false);
            }
        }
        CHECK(mirrored);
        CHECK(inactive);
        CHECK(withIcon);

        // A placed final state is a 4x2-grid-cell marker named Final, the next one Final2.
        SMScene& pageScene = page.getScene();
        const auto cancelRename = [&pageScene]()
        {
            for (QGraphicsItem* item : pageScene.items())
            {
                QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                QLineEdit* editor = (proxy != nullptr ? qobject_cast<QLineEdit*>(proxy->widget()) : nullptr);
                if (editor != nullptr)
                {
                    keyClick(editor, Qt::Key_Escape);
                    break;
                }
            }
        };

        pageScene.setActiveTool(NESMDesign::eCanvasTool::AddFinalState);
        clickScene(page.getView(), QPointF(600.0, 420.0));
        cancelRename();
        const SMStateEntry* final1 = doc.getData().findState("Final");
        CHECK(final1 != nullptr);
        const SMLayoutNode* finalNode = (final1 != nullptr ? doc.getData().getLayout().findNode(final1->getId()) : nullptr);
        CHECK(finalNode != nullptr);
        CHECK((finalNode != nullptr) && (finalNode->width  == 4.0 * NESMDesign::GridSizeDefault));
        CHECK((finalNode != nullptr) && (finalNode->height == 2.0 * NESMDesign::GridSizeDefault));

        pageScene.setActiveTool(NESMDesign::eCanvasTool::AddFinalState);
        clickScene(page.getView(), QPointF(760.0, 420.0));
        cancelRename();
        CHECK(doc.getData().findState("Final2") != nullptr);

        // Center Machine brings a lost diagram back into the viewport, zoom unchanged.
        SMGraphicsView& pageView = page.getView();
        pageView.centerOn(6000.0, 6000.0);
        QApplication::processEvents();
        const QRectF lost = pageView.mapToScene(pageView.viewport()->rect()).boundingRect();
        CHECK(lost.intersects(pageScene.contentBounds()) == false);
        page.actionCenterMachine()->trigger();
        QApplication::processEvents();
        const QRectF found = pageView.mapToScene(pageView.viewport()->rect()).boundingRect();
        CHECK(found.intersects(pageScene.contentBounds()));
    }

    std::printf("sect: issue #541 the armed tool is visible, and Ctrl repeats it\n");
    // --- Picking a drawing tool must be visible wherever it was picked from: the placement
    // actions are checkable and the toolbar, the Design menu and the context menu share the
    // very same action objects, so one checked flag lights all three. The canvas cursor
    // doubles the hint. Ctrl held through the gesture keeps the tool armed for one more. ---
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        SMScene& pageScene = page.getScene();
        SMGraphicsView& pageView = page.getView();

        const QList<QAction*> placement{ page.actionAddState(), page.actionAddFinalState()
                                       , page.actionAddTransition(), page.actionAddNote() };
        for (QAction* action : placement)
        {
            CHECK(action->isCheckable());
            CHECK(action->isChecked() == false);        // nothing armed on a fresh page
        }

        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::Select);
        CHECK(pageView.cursor().shape() == Qt::ArrowCursor);

        const auto cancelRename = [&pageScene]()
        {
            for (QGraphicsItem* item : pageScene.items())
            {
                QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                QLineEdit* editor = (proxy != nullptr ? qobject_cast<QLineEdit*>(proxy->widget()) : nullptr);
                if (editor != nullptr)
                {
                    keyClick(editor, Qt::Key_Escape);
                    break;
                }
            }
        };

        // Only the armed tool's action is checked; the others must clear even though the
        // user never touched them.
        const auto checkedOnly = [&placement](const QAction* armed) -> bool
        {
            for (const QAction* action : placement)
            {
                if (action->isChecked() != (action == armed))
                {
                    return false;
                }
            }

            return true;
        };

        // Armed from the Design menu, which hands out this page's own action objects: the
        // toolbar button of the same action shows the arming without a second code path.
        QMenu designMenu;
        page.populateDesignMenu(designMenu);
        CHECK(designMenu.actions().contains(page.actionAddState()));
        page.actionAddState()->trigger();
        QApplication::processEvents();
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::AddState);
        CHECK(checkedOnly(page.actionAddState()));

        // The crosshair is drawn, not the fixed system Qt::CrossCursor, so its span follows
        // the NESMDesign::ToolCursorSize constant (odd, to keep a center pixel).
        CHECK(pageView.cursor().shape() == Qt::BitmapCursor);
        CHECK(static_cast<int>(pageView.cursor().pixmap().deviceIndependentSize().width())
              == (NESMDesign::ToolCursorSize | 1));

        // The arms actually got painted: an empty pixmap would still have the right size and
        // would leave the canvas with an invisible cursor.
        const QImage cursorImage = pageView.cursor().pixmap().toImage();
        CHECK(cursorImage.pixelColor(cursorImage.width() / 2, cursorImage.height() / 2).alpha() > 0);
        CHECK(cursorImage.pixelColor(0, 0).alpha() == 0);    // corners stay transparent

        // Switching tools moves the check rather than adding one.
        page.actionAddNote()->trigger();
        QApplication::processEvents();
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::AddNote);
        CHECK(checkedOnly(page.actionAddNote()));

        // Triggering the armed action again unchecks it and disarms: the checked button is a
        // real toggle, not a one-way indicator.
        page.actionAddNote()->trigger();
        QApplication::processEvents();
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::Select);
        CHECK(checkedOnly(nullptr));
        CHECK(pageView.cursor().shape() == Qt::ArrowCursor);

        // A finished placement disarms, the way the issue asks: click Add State, draw one
        // state, and the button is no longer highlighted.
        const int before = d.getStates().getElementCount();
        page.actionAddState()->trigger();
        clickScene(pageView, QPointF(560.0, 660.0));
        cancelRename();
        CHECK(d.getStates().getElementCount() == (before + 1));
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::Select);
        CHECK(checkedOnly(nullptr));
        CHECK(pageView.cursor().shape() == Qt::ArrowCursor);

        // Ctrl held through the click repeats: the tool stays armed (and visibly checked)
        // for the next placement without going back to the toolbar.
        page.actionAddState()->trigger();
        clickScene(pageView, QPointF(760.0, 660.0), Qt::LeftButton, Qt::ControlModifier);
        cancelRename();
        CHECK(d.getStates().getElementCount() == (before + 2));
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::AddState);
        CHECK(checkedOnly(page.actionAddState()));
        CHECK(pageScene.isToolSticky() == false);   // armed for the next gesture, not pinned

        clickScene(pageView, QPointF(960.0, 660.0), Qt::LeftButton, Qt::ControlModifier);
        cancelRename();
        CHECK(d.getStates().getElementCount() == (before + 3));
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::AddState);

        // The first click without Ctrl ends the run.
        clickScene(pageView, QPointF(1160.0, 660.0));
        cancelRename();
        CHECK(d.getStates().getElementCount() == (before + 4));
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::Select);
        CHECK(checkedOnly(nullptr));

        // A double-clicked toolbar button pins the tool (issue #516); the check must survive
        // the finished gesture that Ctrl-repeat would only have survived once.
        page.armStickyTool(NESMDesign::eCanvasTool::AddState);
        QApplication::processEvents();
        CHECK(pageScene.isToolSticky());
        CHECK(checkedOnly(page.actionAddState()));
        clickScene(pageView, QPointF(560.0, 860.0));
        cancelRename();
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::AddState);
        CHECK(checkedOnly(page.actionAddState()));

        // Esc is the way out of a pinned tool, and it clears the check as well.
        keyClickScene(pageScene, Qt::Key_Escape);
        QApplication::processEvents();
        CHECK(pageScene.getActiveTool() == NESMDesign::eCanvasTool::Select);
        CHECK(checkedOnly(nullptr));
        CHECK(pageView.cursor().shape() == Qt::ArrowCursor);
    }

    std::printf("sect: SM-19 grid dots startup sync\n");
    {
        // The stored dotted-grid preference must reach the scene at page construction, so
        // the canvas and the checked Dotted Grid button agree from the very first paint.
        QCoreApplication::setOrganizationName(QStringLiteral("LusanSmokeTest"));
        QCoreApplication::setApplicationName(QStringLiteral("Issue514"));
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue(QStringLiteral("smDesign/gridDots"), true);
        settings.sync();

        {
            StateMachineModel doc;
            CHECK(doc.loadFromFile(sourcePath));
            SMDesign page(doc);
            page.resize(1400, 900);
            page.show();
            QApplication::processEvents();
            CHECK(page.actionGridDots()->isChecked());
            CHECK(page.getScene().getGridStyle() == NESMDesign::eGridStyle::Dots);

            page.actionGridDots()->setChecked(false);
            CHECK(page.getScene().getGridStyle() == NESMDesign::eGridStyle::Lines);
        }

        settings.clear();
        settings.sync();
        QCoreApplication::setOrganizationName(QString());
        QCoreApplication::setApplicationName(QString());
    }

    std::printf("sect: SM-19 outline and properties panels\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();

        // The Outline and Properties panels are now global ADS docks the main window creates for
        // the active Design page (issue #516); they are no longer owned by SMDesign. A headless
        // test drives standalone instances bound to the same document + scene manager -- exactly
        // what MdiMainWindow::updateDesignPanels() constructs per active Design page.
        SMOutlinePanel outlinePanel(doc, page.getSceneManager());
        SMPropertiesPanel propsPanel(doc);

        // issue #542: how narrow the user may drag the right dock column is the WORST minimum in
        // it, so both docks are held to the panel contract -- otherwise one of them silently
        // becomes the floor for the other and the separator stops moving.
        QDockWidget* propsDock = page.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        QDockWidget* outlineDock = page.findChild<QDockWidget*>(QStringLiteral("SMOutlineDock"));
        CHECK((propsDock != nullptr) && (outlineDock != nullptr));
        if ((propsDock != nullptr) && (outlineDock != nullptr))
        {
            CHECK(propsDock->minimumSizeHint().width() <= NESMDesign::PanelMinWidth);
            CHECK(outlineDock->minimumSizeHint().width() <= NESMDesign::PanelMinWidth);
        }
        outlinePanel.resize(320, 600);
        outlinePanel.show();
        propsPanel.resize(320, 600);
        propsPanel.show();
        QApplication::processEvents();
        SMOutlinePanel* outline = &outlinePanel;
        SMPropertiesPanel* props = &propsPanel;
        CHECK(outline != nullptr);
        CHECK(props != nullptr);

        // Pick a Normal state and the first state that owns a transition.
        const SMStateData* root = d.findLevel(page.getScene().getLevelId());
        CHECK(root != nullptr);
        const SMStateEntry* normal = nullptr;
        uint32_t ownerWithTxId = 0;
        uint32_t firstTxId = 0;
        for (const SMStateEntry* s : root->getElements())
        {
            if ((normal == nullptr) && (s->getKind() == SMStateEntry::eStateKind::Normal))
            {
                normal = s;
            }

            // Skip the Start: its transitions are the level's INITIAL ones, which name no
            // stimulus and whose picker is disabled, so they cannot exercise the stimulus path.
            if ((firstTxId == 0) && (s->isPseudoStart() == false) && (s->getTransitions().getElementCount() >= 1))
            {
                firstTxId = s->getTransitions().getElements().first()->getId();
                ownerWithTxId = s->getId();
            }
        }
        CHECK(normal != nullptr);
        CHECK(firstTxId != 0);

        // AC1: selecting a state is reflected in the properties page and the outline.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ normal->getId() });
        QApplication::processEvents();
        grab(page, "g19-panels");
        CHECK(props->currentPage() == SMPropertiesPanel::PageState);
        CHECK(props->currentElementId() == normal->getId());
        CHECK(props->stateNameEdit()->text() == normal->getName());
        CHECK(props->stateNameEdit()->maxLength() == StateMachineData::MAX_IDENTIFIER_LENGTH);
        CHECK(outline->getTree()->selectedItems().isEmpty() == false);

        // Live canvas rename preview mirrors into every Properties panel instance before commit.
        const QString originalName = normal->getName();
        SMStateItem* normalItem = page.getScene().stateItem(normal->getId());
        CHECK(normalItem != nullptr);
        if (normalItem != nullptr)
        {
            normalItem->startInlineRename();
            QApplication::processEvents();

            QLineEdit* inlineEditor = nullptr;
            for (QGraphicsItem* item : page.getScene().items())
            {
                QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
                QLineEdit* candidate = (proxy != nullptr ? qobject_cast<QLineEdit*>(proxy->widget()) : nullptr);
                if (candidate != nullptr)
                {
                    inlineEditor = candidate;
                    break;
                }
            }

            CHECK(inlineEditor != nullptr);
            if (inlineEditor != nullptr)
            {
                CHECK(inlineEditor->maxLength() == StateMachineData::MAX_IDENTIFIER_LENGTH);
                inlineEditor->setText(QStringLiteral("LivePreviewName"));
                QApplication::processEvents();
                CHECK(props->stateNameEdit()->text() == QStringLiteral("LivePreviewName"));
                keyClick(inlineEditor, Qt::Key_Escape);
                QApplication::processEvents();
                CHECK(props->stateNameEdit()->text() == originalName);
            }
        }

        // AC3: editing the name in properties is the same atomic, undoable rename as canvas F2.
        const int undoBeforeRename = doc.getUndoStack().count();
        props->stateNameEdit()->setText(QStringLiteral("RenamedByPanel"));
        QMetaObject::invokeMethod(props->stateNameEdit(), "editingFinished");
        CHECK(d.findState("RenamedByPanel") != nullptr);
        CHECK(doc.getUndoStack().count() == undoBeforeRename + 1);
        doc.getUndoStack().undo();
        CHECK(d.findState("RenamedByPanel") == nullptr);

        // AC3: an invalid identifier is rejected inline (no command, the field reverts).
        doc.getSelectionModel().setSelection(QList<uint32_t>{ normal->getId() });
        const int undoBeforeReject = doc.getUndoStack().count();
        props->stateNameEdit()->setText(QStringLiteral("9 invalid"));
        QMetaObject::invokeMethod(props->stateNameEdit(), "editingFinished");
        CHECK(doc.getUndoStack().count() == undoBeforeReject);
        CHECK(props->stateNameEdit()->text() == normal->getName());

        // The state page lists the owner's transitions.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ ownerWithTxId });
        QApplication::processEvents();
        CHECK(props->transitionList()->count() == d.findStateById(ownerWithTxId)->getTransitions().getElementCount());

        // AC1: selecting a transition switches the properties page to the transition editor.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ firstTxId });
        QApplication::processEvents();
        CHECK(props->currentPage() == SMPropertiesPanel::PageTransition);
        CHECK(props->currentElementId() == firstTxId);

        QToolButton* tryToggle = props->findChild<QToolButton*>(QStringLiteral("smGuardTryToggle"));
        CHECK((tryToggle != nullptr) && (tryToggle->icon().isNull() == false));
        CHECK((tryToggle != nullptr) && (tryToggle->text() == QStringLiteral("Try it")));

        QTextEdit* guardField = props->findChild<QTextEdit*>(QStringLiteral("smGuardField"));
        QLabel* guardStatus = props->findChild<QLabel*>(QStringLiteral("smGuardStatus"));
        QPlainTextEdit* guardGenCode = props->findChild<QPlainTextEdit*>(QStringLiteral("smGuardGeneratedCode"));
        CHECK((guardField != nullptr) && (guardStatus != nullptr) && (guardGenCode != nullptr));
        const QString immediateAttr = QStringLiteral("ImmediateAttrSmoke");
        CHECK(doc.getAttributeModel().findAttribute(immediateAttr) == nullptr);
        if ((guardField != nullptr) && (guardStatus != nullptr) && (guardGenCode != nullptr))
        {
            // An undefined name is NOT silent raw C++: attributes/parameters/constants are typed data
            // objects that must be declared, so a bare name resolving to none of them is an error.
            guardField->setPlainText(immediateAttr);
            QMetaObject::invokeMethod(guardField, "onDebounce");
            QApplication::processEvents();
            CHECK(guardStatus->text().contains(QStringLiteral("err")));

            // Once the attribute exists the guard resolves: no error, the generated code shows the
            // attribute call, and the status tooltip stays empty for a valid guard (the generated C++
            // lives in the Generated-code section, not the status tooltip).
            doc.getAttributeModel().createAttribute(immediateAttr);
            QApplication::processEvents();
            CHECK(guardStatus->text().contains(QStringLiteral("err")) == false);
            CHECK(guardGenCode->toPlainText().contains(immediateAttr + QStringLiteral("()")));
            CHECK(guardStatus->toolTip().isEmpty());
        }

        if (SMSetGuardCommand* clearGuard = SMGuardCommands::clearGuard(doc.getData(), doc.getNotifier(), firstTxId, QStringLiteral("Clear guard")))
        {
            doc.getUndoStack().push(clearGuard);
            QApplication::processEvents();
        }

        CHECK(tryToggle != nullptr);
        if (tryToggle != nullptr)
        {
            tryToggle->click();
            QApplication::processEvents();
            QLabel* tryNote = props->findChild<QLabel*>(QStringLiteral("smTryNote"));
            CHECK(tryNote != nullptr);
            if (tryNote != nullptr)
            {
                CHECK(tryNote->wordWrap());
                CHECK(tryNote->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored);
            }
            tryToggle->click();
            QApplication::processEvents();
        }

        // AC2: a priority reorder keeps the properties list in sync and is undoable. (The drag
        // gesture builds the same reorder command; here it is issued directly.)
        if (d.findStateById(ownerWithTxId)->getTransitions().getElementCount() >= 2)
        {
            doc.getSelectionModel().setSelection(QList<uint32_t>{ ownerWithTxId });
            QApplication::processEvents();
            SMTransitionData& list = d.findStateById(ownerWithTxId)->getTransitions();
            const uint32_t topId = list.getElements().first()->getId();
            const int rowsBefore = props->transitionList()->count();
            doc.getUndoStack().push(new TDocReorderCommand<SMTransitionEntry*, DocumentElem>(doc.getNotifier(), list, 0, 1, ownerWithTxId, eDocElementKind::Transition, QStringLiteral("Reorder")));
            QApplication::processEvents();
            CHECK(props->transitionList()->count() == rowsBefore);
            CHECK(list.getElements().at(1)->getId() == topId);
            doc.getUndoStack().undo();
            QApplication::processEvents();
            CHECK(list.getElements().first()->getId() == topId);
        }

        // The stimulus picker is a read-only, closed list of triggers/events/timers (row 0 is
        // "(none)"; each row carries its real name at Qt::UserRole+1). Picking a listed entry sets
        // it undoably; the panel never creates or renames a registry entry, and free text is
        // impossible (the combo is not editable).
        doc.getSelectionModel().setSelection(QList<uint32_t>{ firstTxId });
        QApplication::processEvents();
        QComboBox* stim = props->stimulusNameCombo();
        CHECK(stim->isEditable() == false);

        const QString curStim = d.findTransitionById(firstTxId)->getStimulus();
        int pickRow = -1;
        for (int i = 0; i < stim->count(); ++i)
        {
            const QString name = stim->itemData(i, Qt::UserRole + 1).toString();
            if ((name.isEmpty() == false) && (name != curStim))
            {
                pickRow = i;
                break;
            }
        }

        if (pickRow >= 0)
        {
            const QString pickName = stim->itemData(pickRow, Qt::UserRole + 1).toString();
            const int indexBeforePick = doc.getUndoStack().index();  // index, not count: prior undo left a redoable
            stim->setCurrentIndex(pickRow);
            QMetaObject::invokeMethod(stim, "activated", Q_ARG(int, pickRow));
            CHECK(d.findTransitionById(firstTxId)->getStimulus() == pickName);
            CHECK(doc.getUndoStack().index() == indexBeforePick + 1);

            // Row 0 ("(none)") detaches the stimulus, undoably.
            const int indexBeforeClear = doc.getUndoStack().index();
            stim->setCurrentIndex(0);
            QMetaObject::invokeMethod(stim, "activated", Q_ARG(int, 0));
            CHECK(d.findTransitionById(firstTxId)->getStimulus().isEmpty());
            CHECK(doc.getUndoStack().index() == indexBeforeClear + 1);
            doc.getUndoStack().undo();
            doc.getUndoStack().undo();
            CHECK(d.findTransitionById(firstTxId)->getStimulus() == curStim);
        }

        // The guard help popup must stay readable whether the Properties panel sits on the left
        // or on the right side of its host window.
        auto checkGuardHelpPopup = [&](bool panelOnRight)
        {
            const QRect screenRect = (QGuiApplication::primaryScreen() != nullptr)
                    ? QGuiApplication::primaryScreen()->availableGeometry()
                    : QRect(0, 0, 1200, 800);

            QWidget host;
            host.resize(  qMin(screenRect.width(), qMax(520, screenRect.width() - 80))
                        , qMin(screenRect.height(), qMax(520, screenRect.height() - 80)));
            host.move(  panelOnRight
                            ? screenRect.x() + screenRect.width() - host.width()
                            : screenRect.x()
                      , screenRect.top() + qMax(0, (screenRect.height() - host.height()) / 2));

            QHBoxLayout* layout = new QHBoxLayout(&host);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            SMPropertiesPanel* dockedProps = new SMPropertiesPanel(doc, &host);
            dockedProps->setFixedWidth(qMin(360, qMax(260, host.width() / 3)));

            if (panelOnRight)
            {
                layout->addStretch(1);
                layout->addWidget(dockedProps);
            }
            else
            {
                layout->addWidget(dockedProps);
                layout->addStretch(1);
            }

            host.show();
            QApplication::processEvents();

            doc.getSelectionModel().setSelection(QList<uint32_t>{ firstTxId });
            QApplication::processEvents();

            QTabWidget* tabs = dockedProps->findChild<QTabWidget*>(QStringLiteral("smTransTabs"));
            CHECK(tabs != nullptr);
            if (tabs == nullptr)
            {
                return;
            }

            tabs->setCurrentIndex(1);
            QApplication::processEvents();

            QToolButton* helpBtn = dockedProps->findChild<QToolButton*>(QStringLiteral("smGuardHelp"));
            CHECK((helpBtn != nullptr) && helpBtn->isVisible());
            if (helpBtn == nullptr)
            {
                return;
            }

            helpBtn->click();
            QApplication::processEvents();

            SMGuardHelpCard* helpCard = dockedProps->findChild<SMGuardHelpCard*>();
            CHECK((helpCard != nullptr) && helpCard->isVisible());
            if (helpCard == nullptr)
            {
                return;
            }

            const QRect buttonRect(helpBtn->mapToGlobal(QPoint(0, 0)), helpBtn->size());
            const QRect popupRect = helpCard->frameGeometry();
            QScreen* screen = QGuiApplication::screenAt(buttonRect.center());
            if (screen == nullptr)
            {
                screen = helpCard->screen();
            }

            CHECK(screen != nullptr);
            if (screen != nullptr)
            {
                CHECK(screen->availableGeometry().contains(popupRect));
            }

            if (panelOnRight)
            {
                CHECK(popupRect.right() <= buttonRect.right());
            }
            else
            {
                CHECK(popupRect.left() >= buttonRect.left());
            }
        };

        checkGuardHelpPopup(false);
        checkGuardHelpPopup(true);
    }

    std::printf("sect: SM-issue516 substate marker size + transition point nudge\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        SMScene&        scene = page.getScene();

        // --- Bug 1: a substate's auto-created Start is a compact marker box (same size as the
        // root level's Start marker), not a full normal-state box. Add a plain Normal state to
        // convert, so the check does not depend on the fixture already having one. ---
        SMStateData* rootLevel = d.findLevel(scene.getLevelId());
        CHECK(rootLevel != nullptr);
        SMCreateStateCommand* addParent = new SMCreateStateCommand(  d, doc.getNotifier(), *rootLevel
                                                                  , QStringLiteral("NudgeParent"), SMStateEntry::eStateKind::Normal
                                                                  , QRectF(320.0, 320.0, NESMDesign::StateDefaultWidth, NESMDesign::StateDefaultHeight)
                                                                  , QStringLiteral("Add parent"));
        doc.getUndoStack().push(addParent);
        const uint32_t plainId = addParent->getStateId();
        CHECK(plainId != 0);

        doc.getSelectionModel().setSelection(QList<uint32_t>{ plainId });
        QApplication::processEvents();
        CHECK(page.actionAddSubstate()->isEnabled());
        page.actionAddSubstate()->trigger();
        QApplication::processEvents();

        const SMStateEntry* composite = d.findStateById(plainId);
        CHECK((composite != nullptr) && composite->hasNestedStates());
        const SMStateData*  nested      = (composite != nullptr ? composite->getNestedStates() : nullptr);
        const SMStateEntry* nestedStart = (nested != nullptr ? nested->getStartState() : nullptr);
        CHECK(nestedStart != nullptr);
        const SMLayoutNode* startNode = (nestedStart != nullptr ? d.getLayout().findNode(nestedStart->getId()) : nullptr);
        CHECK(startNode != nullptr);
        if (startNode != nullptr)
        {
            CHECK(startNode->width  == NESMDesign::MarkerStateWidth);
            CHECK(startNode->height == NESMDesign::MarkerStateHeight);
        }
    }

    std::printf("sect: SM-issue516 transition/internal action enablement\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1200, 800);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        SMScene& scene = page.getScene();

        // Bug 5: Add Internal Transition is disabled for a Start state (no entry/exit/internal
        // behaviour), enabled for a Normal state.
        const SMStateData* root = d.findLevel(scene.getLevelId());
        CHECK(root != nullptr);
        const SMStateEntry* start = (root != nullptr ? root->getStartState() : nullptr);
        CHECK(start != nullptr);
        if (start != nullptr)
        {
            doc.getSelectionModel().setSelection(QList<uint32_t>{ start->getId() });
            QApplication::processEvents();
            CHECK(page.actionAddInternal()->isEnabled() == false);
            // Bug 2: a level that also has non-Start states can still draw a transition.
            CHECK(page.actionAddTransition()->isEnabled());
        }

        const SMStateEntry* normal = nullptr;
        for (const SMStateEntry* s : root->getElements())
        {
            if (s->getKind() == SMStateEntry::eStateKind::Normal) { normal = s; break; }
        }

        if (normal != nullptr)
        {
            doc.getSelectionModel().setSelection(QList<uint32_t>{ normal->getId() });
            QApplication::processEvents();
            CHECK(page.actionAddInternal()->isEnabled());
        }

        // Bug 2: a brand-new level that holds only its Start offers no transition target, so
        // Add Transition is disabled until a non-Start state exists.
        StateMachineModel fresh;
        CHECK(fresh.createNewDocument(QStringLiteral("Fresh")));
        SMDesign freshPage(fresh);
        freshPage.resize(1000, 700);
        freshPage.show();
        QApplication::processEvents();
        StateMachineData& fd = fresh.getData();
        SMScene& freshScene = freshPage.getScene();
        const SMStateData* freshRoot = fd.findLevel(freshScene.getLevelId());
        CHECK(freshRoot != nullptr);
        // A default new document holds exactly its Start state.
        int nonStart = 0;
        for (const SMStateEntry* s : freshRoot->getElements())
        {
            if (s->getKind() != SMStateEntry::eStateKind::Start) { ++nonStart; }
        }

        if (nonStart == 0)
        {
            CHECK(freshPage.actionAddTransition()->isEnabled() == false);

            // Adding a Normal state enables it.
            SMStateData* level = fd.findLevel(freshScene.getLevelId());
            SMCreateStateCommand* addState = new SMCreateStateCommand(  fd, fresh.getNotifier(), *level
                                                                     , QStringLiteral("S1"), SMStateEntry::eStateKind::Normal
                                                                     , QRectF(240.0, 240.0, NESMDesign::StateDefaultWidth, NESMDesign::StateDefaultHeight)
                                                                     , QStringLiteral("Add S1"));
            fresh.getUndoStack().push(addState);
            QApplication::processEvents();
            CHECK(freshPage.actionAddTransition()->isEnabled());
        }
    }

    std::printf("sect: SM-issue516 transition waypoint keyboard nudge\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        SMScene&        scene = page.getScene();
        SMGraphicsView& view  = page.getView();

        // The modulo-snap expectation must mirror SMEdgeItem's nudgeAxis exactly.
        auto expectAxis = [](double v, int dir, int base, bool pixel) -> double {
            if (pixel) { return v + static_cast<double>(dir); }
            const double cells = v / static_cast<double>(base);
            const double t = (dir > 0) ? (std::floor(cells + 1e-6) + 1.0) : (std::ceil(cells - 1e-6) - 1.0);
            return t * static_cast<double>(base);
        };
        auto sendArrow = [&scene](Qt::Key key, Qt::KeyboardModifiers mods) {
            QKeyEvent press(QEvent::KeyPress, key, mods);
            QApplication::sendEvent(&scene, &press);
            QApplication::processEvents();
        };

        // Find a straight external transition (a 2-point layout edge with a live edge item).
        const SMStateData* root = d.findLevel(scene.getLevelId());
        CHECK(root != nullptr);
        uint32_t edgeTxId = 0;
        for (const SMStateEntry* s : root->getElements())
        {
            for (const SMTransitionEntry* t : s->getTransitions().getElements())
            {
                const SMLayoutEdge* le = d.getLayout().findEdge(t->getId());
                SMEdgeItem* ei = dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(t->getId()));
                if ((ei != nullptr) && (le != nullptr) && (le->points.size() == 2))
                {
                    edgeTxId = t->getId();
                    break;
                }
            }

            if (edgeTxId != 0) { break; }
        }
        CHECK(edgeTxId != 0);

        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(edgeTxId));
        CHECK(edge != nullptr);
        if (edge != nullptr)
        {
            // Raise the edge so a double-click at its midpoint routes to it, then insert a waypoint.
            edge->setZValue(10.0);
            doc.getSelectionModel().setSelection(QList<uint32_t>{ edgeTxId });
            QApplication::processEvents();
            CHECK(edge->isSelected());

            const QPointF mid = (edge->getPath().first() + edge->getPath().last()) / 2.0;
            dblClickScene(view, mid);
            QApplication::processEvents();
            CHECK(d.getLayout().findEdge(edgeTxId)->points.size() == 3);     // waypoint inserted

            // Click the waypoint to make it the active (keyboard-movable) point.
            const QPointF wp = d.getLayout().findEdge(edgeTxId)->points.at(1);
            clickScene(view, wp);
            QApplication::processEvents();
            CHECK(edge->hasSelectedPoint());

            // Normal step (5, modulo 5): x snaps to the next multiple of 5, y unchanged.
            const QPointF p0 = d.getLayout().findEdge(edgeTxId)->points.at(1);
            sendArrow(Qt::Key_Right, Qt::NoModifier);
            const QPointF p1 = d.getLayout().findEdge(edgeTxId)->points.at(1);
            CHECK(p1.x() == expectAxis(p0.x(), 1, 5, false));
            CHECK(p1.y() == p0.y());

            // Coarse step (Ctrl, modulo 10) upward.
            sendArrow(Qt::Key_Up, Qt::ControlModifier);
            const QPointF p2 = d.getLayout().findEdge(edgeTxId)->points.at(1);
            CHECK(p2.y() == expectAxis(p1.y(), -1, 10, false));
            CHECK(p2.x() == p1.x());

            // Pixel step (Shift): exactly one unit left, no snapping.
            sendArrow(Qt::Key_Left, Qt::ShiftModifier);
            const QPointF p3 = d.getLayout().findEdge(edgeTxId)->points.at(1);
            CHECK(p3.x() == p2.x() - 1.0);
            CHECK(p3.y() == p2.y());

            // Each nudge is its own undo step; undo restores the pre-nudge waypoint position.
            doc.getUndoStack().undo();
            CHECK(d.getLayout().findEdge(edgeTxId)->points.at(1) == p2);

            // The endpoints stayed glued to the state borders (never moved by the nudge).
            CHECK(d.getLayout().findEdge(edgeTxId)->points.size() == 3);
        }
    }

    std::printf("sect: SM-20 copy/paste/duplicate/cut\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        SMScene& pageScene = page.getScene();
        SMStateData* level = doc.getData().findLevel(pageScene.getLevelId());
        SMStateEntry* source = nullptr;
        for (SMStateEntry* state : level->getElements())
        {
            if (state->getKind() == SMStateEntry::eStateKind::Normal)
            {
                source = state;
                break;
            }
        }
        CHECK(source != nullptr);

        // Copy fills the clipboard with the FSM payload.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ source->getId() });
        QApplication::processEvents();
        page.actionCopy()->trigger();
        const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
        CHECK((mime != nullptr) && mime->hasFormat(QString::fromLatin1(SMClipboard::MIME_TYPE)));

        // Paste: new state under a fresh ID, ID-suffixed name, node offset by one grid
        // step, item on the scene, and the pasted element selected.
        const int stateCount = level->getElementCount();
        page.actionPaste()->trigger();
        QApplication::processEvents();
        CHECK(level->getElementCount() == (stateCount + 1));
        const QList<uint32_t> pastedSel = doc.getSelectionModel().getSelection();
        CHECK((pastedSel.size() == 1) && (pastedSel.first() != source->getId()));
        const uint32_t pastedId = pastedSel.isEmpty() ? 0u : pastedSel.first();
        CHECK(pageScene.stateItem(pastedId) != nullptr);
        const SMStateEntry* pasted = doc.getData().findStateById(pastedId);
        CHECK((pasted != nullptr)
              && (pasted->getName() == (source->getName() + QStringLiteral("_") + QString::number(pastedId))));
        const SMLayoutNode* sourceNode = doc.getData().getLayout().findNode(source->getId());
        const SMLayoutNode* pastedNode = doc.getData().getLayout().findNode(pastedId);
        const double grid = static_cast<double>(pageScene.getGridSize());
        CHECK((sourceNode != nullptr) && (pastedNode != nullptr)
              && (pastedNode->x == sourceNode->x + grid) && (pastedNode->y == sourceNode->y + grid));

        // Paste is one undo step; redo restores the identical ID and its item.
        doc.getUndoStack().undo();
        QApplication::processEvents();
        CHECK(level->getElementCount() == stateCount);
        CHECK(pageScene.stateItem(pastedId) == nullptr);
        doc.getUndoStack().redo();
        QApplication::processEvents();
        CHECK(pageScene.stateItem(pastedId) != nullptr);

        // Duplicate works without touching the clipboard payload.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ source->getId() });
        page.actionDuplicate()->trigger();
        QApplication::processEvents();
        CHECK(level->getElementCount() == (stateCount + 2));
        grab(page, "g20-paste");

        // Cut copies and deletes in one step, without confirmation.
        const int beforeCut = level->getElementCount();
        doc.getSelectionModel().setSelection(QList<uint32_t>{ source->getId() });
        page.actionCut()->trigger();
        QApplication::processEvents();
        CHECK(level->getElementCount() == (beforeCut - 1));
        CHECK(doc.getData().findStateById(source->getId()) == nullptr);
        doc.getUndoStack().undo();
        QApplication::processEvents();
        CHECK(doc.getData().findStateById(source->getId()) != nullptr);
    }

    std::printf("sect: SM-21-08 canvas search / go-to\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        QLineEdit* box = page.findChild<QLineEdit*>(QStringLiteral("smCanvasSearch"));
        QLabel* status = page.findChild<QLabel*>(QStringLiteral("smCanvasSearchStatus"));
        CHECK(box != nullptr);
        CHECK(status != nullptr);

        SMStateEntry* off = d.findState("LightOff");
        SMStateEntry* on  = d.findState("LightOn");
        CHECK((off != nullptr) && (on != nullptr));

        if ((box != nullptr) && (off != nullptr) && (on != nullptr))
        {
            // A state-name match selects the state and reveals it (highlight/selection path).
            box->setText(QStringLiteral("LightOff"));
            QApplication::processEvents();
            CHECK(doc.getSelectionModel().getSelection().contains(off->getId()));

            // A broad query matches several states; Enter cycles to a different match.
            box->setText(QStringLiteral("Light"));
            QApplication::processEvents();
            const uint32_t firstHit = doc.getSelectionModel().getSelection().isEmpty()
                                        ? 0u : doc.getSelectionModel().getSelection().first();
            CHECK((firstHit == off->getId()) || (firstHit == on->getId()));
            QKeyEvent enterPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            QApplication::sendEvent(box, &enterPress);
            QApplication::processEvents();
            const uint32_t secondHit = doc.getSelectionModel().getSelection().isEmpty()
                                        ? 0u : doc.getSelectionModel().getSelection().first();
            CHECK((secondHit != 0u) && (secondHit != firstHit));    // cycled to the next match

            // A transition stimulus matches its transition.
            box->setText(QStringLiteral("power_on"));
            QApplication::processEvents();
            SMStateEntry* offOwner = d.findState("LightOff");
            uint32_t powerOnId = 0;
            for (const SMTransitionEntry* t : offOwner->getTransitions().getElements())
            {
                if (t->getStimulus() == QStringLiteral("power_on")) { powerOnId = t->getId(); break; }
            }
            CHECK(powerOnId != 0);
            CHECK(doc.getSelectionModel().getSelection().contains(powerOnId));

            // No match leaves the selection unchanged and shows a clear affordance.
            const QList<uint32_t> before = doc.getSelectionModel().getSelection();
            box->setText(QStringLiteral("Nonexistent_zzz"));
            QApplication::processEvents();
            CHECK((status == nullptr) || (status->text() == QStringLiteral("No match")));
            CHECK(doc.getSelectionModel().getSelection() == before);

            // Esc abandons the search box.
            box->setText(QStringLiteral("Light"));
            QApplication::processEvents();
            QKeyEvent escPress(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QApplication::sendEvent(box, &escPress);
            QApplication::processEvents();
            CHECK(box->text().isEmpty());
        }
    }

    std::printf("sect: SM-26 canvas search options (case / word / regex / exact id)\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        QLineEdit* box       = page.findChild<QLineEdit*>(QStringLiteral("smCanvasSearch"));
        QLabel* status       = page.findChild<QLabel*>(QStringLiteral("smCanvasSearchStatus"));
        QToolButton* opCase  = page.findChild<QToolButton*>(QStringLiteral("smCanvasSearchCase"));
        QToolButton* opWord  = page.findChild<QToolButton*>(QStringLiteral("smCanvasSearchWord"));
        QToolButton* opRegex = page.findChild<QToolButton*>(QStringLiteral("smCanvasSearchRegex"));
        SMStateEntry* off = d.findState("LightOff");
        CHECK((box != nullptr) && (status != nullptr));
        CHECK((opCase != nullptr) && (opWord != nullptr) && (opRegex != nullptr));
        CHECK(off != nullptr);

        if ((box != nullptr) && (status != nullptr) && (off != nullptr)
            && (opCase != nullptr) && (opWord != nullptr) && (opRegex != nullptr))
        {
            // A no-match query deliberately leaves the selection where it was, so the verdict --
            // not the selection -- is what says whether the query matched.
            const QString noMatch = QStringLiteral("No match");
            const auto hit = [&doc, off](void) -> bool
            {
                return doc.getSelectionModel().getSelection().contains(off->getId());
            };

            // A purely numeric query matches the element carrying exactly that id (spec 11).
            box->setText(QString::number(off->getId()));
            QApplication::processEvents();
            CHECK(status->text() != noMatch);
            CHECK(hit());

            // Match case off (default) accepts any casing; on, it does not.
            box->setText(QStringLiteral("lightoff"));
            QApplication::processEvents();
            CHECK(hit());
            opCase->setChecked(true);
            QApplication::processEvents();
            CHECK(status->text() == noMatch);
            opCase->setChecked(false);
            QApplication::processEvents();
            CHECK(status->text() != noMatch);

            // Whole word makes a substring of a longer name stop matching.
            box->setText(QStringLiteral("Light"));
            QApplication::processEvents();
            CHECK(status->text() != noMatch);
            opWord->setChecked(true);
            QApplication::processEvents();
            CHECK(status->text() == noMatch);
            opWord->setChecked(false);

            // Regular expression mode interprets the query as a pattern.
            box->setText(QStringLiteral("^Light.*ff$"));
            QApplication::processEvents();
            CHECK(status->text() == noMatch);    // literal substring: no such name
            opRegex->setChecked(true);
            QApplication::processEvents();
            CHECK(status->text() != noMatch);
            CHECK(hit());
            opRegex->setChecked(false);
        }
    }

    std::printf("sect: issue #542 editing a condition never resizes the Properties panel\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));

        // The panel lives in a dock whose width the user owns. A QMainWindow can never make a dock
        // narrower than the hosted widget's minimum, so ANY growth of that minimum while typing
        // silently drags the dock wider -- which is exactly what the user sees. The minimum is
        // therefore the contract, and it is asserted against the panel's whole working life.
        SMPropertiesPanel props(doc);
        props.resize(280, 700);
        props.show();
        QApplication::processEvents();

        const int emptyMin = props.minimumSizeHint().width();
        CHECK(emptyMin <= NESMDesign::PanelMinWidth);

        StateMachineData& d = doc.getData();
        uint32_t txId = 0;
        for (const SMStateEntry* s : d.getStates().getElements())
        {
            if ((txId == 0) && (s->getTransitions().getElementCount() >= 1))
            {
                txId = s->getTransitions().getElements().first()->getId();
            }
        }
        CHECK(txId != 0);

        doc.getSelectionModel().setSelection(QList<uint32_t>{ txId });
        QApplication::processEvents();
        CHECK(props.currentPage() == SMPropertiesPanel::PageTransition);
        CHECK(props.minimumSizeHint().width() <= NESMDesign::PanelMinWidth);

        // The Conditions tab must be the CURRENT one: the status line is only laid out while its
        // tab is shown, and a hidden widget asks a box layout for nothing. The bug lives on the
        // visible path.
        QTabWidget* transTabs = props.findChild<QTabWidget*>(QStringLiteral("smTransTabs"));
        CHECK(transTabs != nullptr);
        if (transTabs != nullptr)
        {
            for (int i = 0; i < transTabs->count(); ++i)
            {
                if (transTabs->tabText(i).contains(QStringLiteral("Condition")))
                {
                    transTabs->setCurrentIndex(i);
                }
            }
        }

        QApplication::processEvents();
        const int armedMin = props.minimumSizeHint().width();
        CHECK(armedMin <= NESMDesign::PanelMinWidth);

        QTextEdit* guardField = props.findChild<QTextEdit*>(QStringLiteral("smGuardField"));
        QLabel* guardStatus = props.findChild<QLabel*>(QStringLiteral("smGuardStatus"));
        CHECK((guardField != nullptr) && (guardStatus != nullptr));
        if ((guardField != nullptr) && (guardStatus != nullptr))
        {
            // The verdict of a long, unresolved guard is the widest text the panel ever shows.
            guardField->setPlainText(QStringLiteral("ThisNameIsDeliberatelyVeryLongAndUndeclaredSoTheVerdictRunsWide == 1"));
            QMetaObject::invokeMethod(guardField, "onDebounce");
            QApplication::processEvents();
            CHECK(guardStatus->text().contains(QStringLiteral("err")));
            CHECK(guardStatus->isVisible());

            // The heart of the issue: the verdict appeared, and the panel asks for exactly what it
            // asked for before. Nothing the user types moves the edge the user placed.
            CHECK(props.minimumSizeHint().width() == armedMin);

            // ... because it is the tooltip, not the panel, that carries the full sentence.
            CHECK(guardStatus->toolTip().contains(QStringLiteral("ThisNameIsDeliberatelyVeryLong")));

            // A narrow panel stays narrow: the label elides into the room it is given.
            props.resize(200, 700);
            QApplication::processEvents();
            CHECK(props.minimumSizeHint().width() <= NESMDesign::PanelMinWidth);
            CHECK(guardStatus->text().contains(QChar(0x2026)));      // the elision mark
            CHECK(guardStatus->text().length() < guardStatus->toolTip().length());

            // Clearing the guard is just as quiet.
            guardField->setPlainText(QString());
            QMetaObject::invokeMethod(guardField, "onDebounce");
            QApplication::processEvents();
            CHECK(props.minimumSizeHint().width() <= NESMDesign::PanelMinWidth);
        }
    }

    std::printf("sect: issue #543 kind marks -- an event is not a method, a band mark keeps its row\n");
    // --- The three surfaces that name a stimulus or an operation (state box, edge label,
    // Properties picker) must agree on ONE vocabulary: a declared method wears parentheses, an
    // event and a timer wear a mark, and nothing wears a name Lusan synthesized. ---
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        CHECK(d.getEvents().createEvent(QStringLiteral("NewEvent")) != nullptr);
        CHECK(d.getTimers().createTimer(QStringLiteral("NewTimer")) != nullptr);

        SMStateEntry* host = nullptr;
        for (SMStateEntry* s : d.getStates().getElements())
        {
            if ((host == nullptr) && (s->getKind() == SMStateEntry::eStateKind::Normal))
            {
                host = s;
            }
        }

        CHECK(host != nullptr);
        if (host != nullptr)
        {
            // A clean slate: an Enter list holding ONLY an event, and an Exit list holding only a
            // timer -- the exact shape that used to lose its band mark to the kind mark.
            const auto clear = [](SMOperationList& list)
            {
                while (list.getCount() > 0)
                {
                    delete list.takeAt(0);
                }
            };

            clear(host->getEntryList());
            clear(host->getExitList());
            host->getEntryList().addOperation(new SMEventSend(0, QStringLiteral("NewEvent")));
            host->getExitList().addOperation(new SMTimerStart(0, QStringLiteral("NewTimer")));

            SMStateItem* box = dynamic_cast<SMStateItem*>(page.getScene().findCanvasItem(host->getId()));
            CHECK(box != nullptr);
            if (box != nullptr)
            {
                box->updateFromModel();
                const QList<SMStateItem::BodyRow> rows = box->getBodyRows();
                CHECK(rows.size() == 4);
                if (rows.size() == 4)
                {
                    // Row 1 is the action row even with no action: it holds the `->|` band mark and
                    // says `...`, so row 2 is free to carry the lightning bolt.
                    CHECK(rows.at(0).icon == SMKindGlyph::eGlyph::Entry);
                    CHECK(rows.at(0).text == QStringLiteral("..."));
                    CHECK(rows.at(0).continues);                  // the group reads as one block
                    CHECK(rows.at(1).icon == SMKindGlyph::eGlyph::Event);
                    CHECK(rows.at(1).text == QStringLiteral("NewEvent"));    // no brackets, no verb

                    CHECK(rows.at(2).icon == SMKindGlyph::exitGlyph());
                    CHECK(rows.at(2).text == QStringLiteral("..."));
                    CHECK(rows.at(3).icon == SMKindGlyph::eGlyph::TimerStart);
                    CHECK(rows.at(3).text == QStringLiteral("NewTimer"));
                }

                // With a real action the placeholder is gone: the band mark rides the action row,
                // and the event keeps its own row and its own mark.
                host->getEntryList().insertOperation(0, new SMActionCall(0, QStringLiteral("doWork")));
                box->updateFromModel();
                const QList<SMStateItem::BodyRow> withAction = box->getBodyRows();
                CHECK(withAction.size() == 4);
                if (withAction.size() == 4)
                {
                    CHECK(withAction.at(0).icon == SMKindGlyph::eGlyph::Entry);
                    CHECK(withAction.at(0).text.startsWith(QStringLiteral("doWork(")));   // a method DOES call
                    CHECK(withAction.at(1).icon == SMKindGlyph::eGlyph::Event);
                    CHECK(withAction.at(1).text == QStringLiteral("NewEvent"));
                }
            }
        }

        // The edge label: the stimulus name is bare and the kind is a mark, for every kind.
        SMTransitionEntry* tx = nullptr;
        for (SMStateEntry* s : d.getStates().getElements())
        {
            for (SMTransitionEntry* t : s->getTransitions().getElements())
            {
                if ((tx == nullptr) && t->isExternal())
                {
                    tx = t;
                }
            }
        }

        CHECK(tx != nullptr);
        SMEdgeItem* edge = (tx != nullptr)
                            ? dynamic_cast<SMEdgeItem*>(page.getScene().findCanvasItem(tx->getId())) : nullptr;
        CHECK(edge != nullptr);
        if ((tx != nullptr) && (edge != nullptr))
        {
            tx->setStimulusKind(SMTransitionEntry::eStimulusKind::Event);
            tx->setStimulus(QStringLiteral("NewEvent"));
            edge->updateFromModel();
            CHECK(edge->getStimulusText() == QStringLiteral("NewEvent"));
            CHECK(edge->getStimulusText().contains(QLatin1Char('(')) == false);
            CHECK(edge->getStimulusGlyph() == SMKindGlyph::eGlyph::Event);

            tx->setStimulusKind(SMTransitionEntry::eStimulusKind::Timer);
            tx->setStimulus(QStringLiteral("NewTimer"));
            edge->updateFromModel();
            CHECK(edge->getStimulusText() == QStringLiteral("NewTimer"));
            CHECK(edge->getStimulusGlyph() == SMKindGlyph::eGlyph::TimerStart);

            // A trigger is a declared method, so it alone keeps the signature.
            QString triggerName;
            for (const MethodEntry* m : d.getMethods().getElements())
            {
                if (triggerName.isEmpty() && (m != nullptr)
                    && (m->getKind() == NEMethod::SmTrigger))
                {
                    triggerName = m->getName();
                }
            }

            if (triggerName.isEmpty() == false)
            {
                tx->setStimulusKind(SMTransitionEntry::eStimulusKind::Trigger);
                tx->setStimulus(triggerName);
                edge->updateFromModel();
                CHECK(edge->getStimulusText().startsWith(triggerName + QLatin1Char('(')));
                CHECK(edge->getStimulusGlyph() == SMKindGlyph::eGlyph::Trigger);
            }

            // The Properties picker offers DECLARED names, never a synthesized handler name, and
            // the row is found by its (kind, name) data rather than by that text.
            tx->setStimulusKind(SMTransitionEntry::eStimulusKind::Event);
            tx->setStimulus(QStringLiteral("NewEvent"));

            SMPropertiesPanel props(doc);
            props.resize(320, 700);
            props.show();
            doc.getSelectionModel().setSelection(QList<uint32_t>{ tx->getId() });
            QApplication::processEvents();
            CHECK(props.currentPage() == SMPropertiesPanel::PageTransition);

            QComboBox* picker = props.stimulusNameCombo();
            CHECK(picker != nullptr);
            if (picker != nullptr)
            {
                bool synthesized = false;
                int eventRow = -1;
                int timerRow = -1;
                for (int row = 1; row < picker->count(); ++row)
                {
                    synthesized = synthesized
                               || picker->itemText(row).startsWith(QStringLiteral("on_event_"))
                               || picker->itemText(row).startsWith(QStringLiteral("on_timer_"));
                    if (picker->itemText(row) == QStringLiteral("NewEvent"))
                    {
                        eventRow = row;
                    }
                    else if (picker->itemText(row) == QStringLiteral("NewTimer"))
                    {
                        timerRow = row;
                    }
                }

                CHECK(synthesized == false);
                CHECK(eventRow > 0);
                CHECK(timerRow > 0);

                // Each row carries its own mark, so an event and a timer of the same name would
                // still be told apart on sight.
                CHECK((eventRow < 0) || (picker->itemIcon(eventRow).isNull() == false));
                CHECK((timerRow < 0) || (picker->itemIcon(timerRow).isNull() == false));

                // The transition's own stimulus is the selected row -- matched by data, which is
                // what the dropped prefixes used to do by text.
                CHECK(picker->currentIndex() == eventRow);
                CHECK(picker->currentText() == QStringLiteral("NewEvent"));
            }
        }

        // The pointer belongs to the user: only a tool that drops a NEW element where the click
        // lands takes the crosshair, and disarming gives the pointer back for good.
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::AddState));
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::AddFinalState));
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::AddTransition));
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::Select) == false);
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::AddNote) == false);
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::Waypoint) == false);
        CHECK(NESMDesign::toolAims(NESMDesign::eCanvasTool::ColorApply) == false);

        SMGraphicsView& toolView = page.getView();
        page.actionAddNote()->trigger();
        QApplication::processEvents();
        CHECK(page.getScene().getActiveTool() == NESMDesign::eCanvasTool::AddNote);
        CHECK(toolView.cursor().shape() == Qt::ArrowCursor);        // a note does not aim

        page.actionAddState()->trigger();
        QApplication::processEvents();
        CHECK(toolView.cursor().shape() == Qt::BitmapCursor);

        // The heart of the cursor bug: an item's hover cursor makes QGraphicsView remember the
        // viewport's cursor -- the crosshair -- and write it back when the hover ends. Disarming
        // must clear that copy, or the crosshair outlives the tool.
        toolView.viewport()->setCursor(toolView.cursor());
        page.actionAddState()->trigger();                           // toggles the tool back off
        QApplication::processEvents();
        CHECK(page.getScene().getActiveTool() == NESMDesign::eCanvasTool::Select);
        CHECK(toolView.cursor().shape() == Qt::ArrowCursor);
        CHECK(toolView.viewport()->cursor().shape() == Qt::ArrowCursor);
    }

    std::printf("sect: issue #546 design panel placement survives a close\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        QDockWidget* propsDock = page.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        QDockWidget* outlineDock = page.findChild<QDockWidget*>(QStringLiteral("SMOutlineDock"));
        CHECK((propsDock != nullptr) && (outlineDock != nullptr));

        // Left or right only: a panel dropped on the top edge squashed the canvas and had no
        // usable height of its own.
        const Qt::DockWidgetAreas sides = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;
        CHECK((propsDock != nullptr) && (propsDock->allowedAreas() == sides));
        CHECK((outlineDock != nullptr) && (outlineDock->allowedAreas() == sides));

        // The toolbar keeps all four edges.
        QToolBar* bar = page.findChild<QToolBar*>(QStringLiteral("SMDesignToolBar"));
        CHECK((bar != nullptr) && (bar->allowedAreas() == Qt::AllToolBarAreas));

        // The dock's own close button must announce the placement change; the main window
        // persists it and stops re-showing the panel on the next activation.
        int closedWidget = -1;
        int closedPlace = -1;
        QObject::connect(&page, &SMDesign::signalPlaceDesignWidget, [&closedWidget, &closedPlace](int w, int p) {
            closedWidget = w;
            closedPlace = p;
        });

        if (outlineDock != nullptr)
        {
            outlineDock->close();
            QApplication::processEvents();
        }

        CHECK(closedWidget == 2);       // eDesignWidget::Outline
        CHECK(closedPlace == 0);        // eDesignPlace::Hidden

        closedWidget = -1;
        closedPlace = -1;
        if (propsDock != nullptr)
        {
            propsDock->close();
            QApplication::processEvents();
        }

        CHECK(closedWidget == 1);       // eDesignWidget::Properties
        CHECK(closedPlace == 0);

        // The stock dock/toolbar right-click list would hide a widget behind the coordinator's
        // back, so the page refuses to build one.
        CHECK(page.createPopupMenu() == nullptr);
    }

    std::printf("sect: SM-26 verify -- the Validation dock has a way back, and the no-document menu matches\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        // Findings live in the output window's Validation tab, not in a page dock. The page
        // owns no validation widget at all; F8 / Shift+F8 and the canvas View entry can only
        // ask the window for it, which is what keeps one findings list for the whole app.
        CHECK(page.findChild<QDockWidget*>(QStringLiteral("SMValidationDock")) == nullptr);
        CHECK(page.findChild<QWidget*>(QStringLiteral("docValidation")) == nullptr);

        bool hasNextIssue = false;
        bool hasPrevIssue = false;
        for (QShortcut* shortcut : page.findChildren<QShortcut*>())
        {
            hasNextIssue = hasNextIssue || (shortcut->key() == QKeySequence(Qt::Key_F8));
            hasPrevIssue = hasPrevIssue || (shortcut->key() == QKeySequence(Qt::SHIFT | Qt::Key_F8));
        }

        CHECK(hasNextIssue);
        CHECK(hasPrevIssue);

        // The Design menu built with no document open is a hand-written copy of toolGroups().
        // Drift there is invisible in the running app (the two are never on screen together),
        // so compare titles and labels position by position.
        const QList<SMDesign::ToolGroup> live = page.toolGroups();
        const QList<SMDesign::ToolGroup> standIn = SMDesign::placeholderToolGroups(page);
        CHECK(live.size() == standIn.size());
        for (int i = 0; (i < live.size()) && (i < standIn.size()); ++i)
        {
            if (live.at(i).title != standIn.at(i).title)
                std::printf("  group %d: '%s' vs '%s'\n", i, live.at(i).title.toUtf8().constData()
                                                           , standIn.at(i).title.toUtf8().constData());
            CHECK(live.at(i).title == standIn.at(i).title);
            CHECK(live.at(i).actions.size() == standIn.at(i).actions.size());
            for (int j = 0; (j < live.at(i).actions.size()) && (j < standIn.at(i).actions.size()); ++j)
            {
                const QString liveText = live.at(i).actions.at(j)->text();
                const QString standInText = standIn.at(i).actions.at(j)->text();
                if (liveText != standInText)
                    std::printf("  %s[%d]: '%s' vs '%s'\n", live.at(i).title.toUtf8().constData(), j
                                                          , liveText.toUtf8().constData(), standInText.toUtf8().constData());
                CHECK(liveText == standInText);
            }
        }

        // Ctrl+F belongs to the window's Edit > Find. A second, page-local shortcut would make
        // both ambiguous and neither would fire.
        for (QShortcut* shortcut : page.findChildren<QShortcut*>())
        {
            CHECK(shortcut->key() != QKeySequence(QKeySequence::Find));
        }
    }

    std::printf("sect: issue 8 arming a tool ends an open in-place edit\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        SMScene& canvas = page.getScene();
        SMStateEntry* off = doc.getData().findState("LightOff");
        CHECK(off != nullptr);
        SMStateItem* item = (off != nullptr) ? dynamic_cast<SMStateItem*>(canvas.findCanvasItem(off->getId())) : nullptr;
        CHECK(item != nullptr);

        if (item != nullptr)
        {
            item->startInlineRename();
            QApplication::processEvents();
            CHECK(item->isRenameActive());

            // Arming a placement tool is a mode switch: the editor ends here. Left open, its proxy
            // widget's cursor is restored onto the viewport when the pointer leaves it and masks
            // the tool's crosshair -- the pointer-instead-of-cross report.
            canvas.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
            QApplication::processEvents();
            CHECK(item->isRenameActive() == false);
            CHECK(canvas.isInlineEditorActive() == false);

            // The name is committed, not discarded: a mode switch must not lose typed work.
            CHECK(off->getName() == QStringLiteral("LightOff"));
        }
    }

    std::printf("sect: issue 9 a transition can be curved, and the arc is a shape of the edge\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        SMScene& canvas = page.getScene();
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(canvas.findCanvasItem(27));
        CHECK(edge != nullptr);

        if (edge != nullptr)
        {
            // Curving is reachable at all: before this the format carried Arc, the renderer drew
            // it, and nothing in the editor could ever set it.
            edge->setShape(SMLayoutEdge::eShape::Arc);
            CHECK(edge->getShape() == SMLayoutEdge::eShape::Arc);
            edge->setSelected(true);
            QApplication::processEvents();
            grab(page, "g-arc-curved");

            const SMLayoutEdge* stored = doc.getData().getLayout().findEdge(27);
            CHECK(stored != nullptr);
            CHECK((stored != nullptr) && (stored->shape == SMLayoutEdge::eShape::Arc));
            // A zero bulge would still draw a straight line, so curving seeds a visible one.
            CHECK((stored != nullptr) && (std::abs(stored->bulge) > 1e-6));
            // An arc is its two endpoints plus a bulge.
            CHECK((stored != nullptr) && (stored->points.size() == 2));

            // One undo step, and it restores the polyline.
            doc.getUndoStack().undo();
            QApplication::processEvents();
            const SMLayoutEdge* undone = doc.getData().getLayout().findEdge(27);
            CHECK((undone != nullptr) && (undone->shape == SMLayoutEdge::eShape::Line));

            doc.getUndoStack().redo();
            QApplication::processEvents();

            // Straightening clears the curvature rather than leaving a bulge on a Line edge.
            edge->setShape(SMLayoutEdge::eShape::Line);
            const SMLayoutEdge* flat = doc.getData().getLayout().findEdge(27);
            CHECK((flat != nullptr) && (flat->shape == SMLayoutEdge::eShape::Line));
            CHECK((flat != nullptr) && (std::abs(flat->bulge) < 1e-6));
        }

        // A self-loop curves too: its two anchors sit on the same box but are still distinct
        // points, so a chord and a bulge describe the loop exactly as they describe any other
        // transition. Refusing it was the bug.
        StateMachineData& d = doc.getData();
        SMStateEntry* on = d.findState("LightOn");
        SMGraphicsView* view = page.findChild<SMGraphicsView*>();
        SMStateItem* onItem = (on != nullptr) ? canvas.stateItem(on->getId()) : nullptr;
        CHECK((on != nullptr) && (view != nullptr) && (onItem != nullptr));
        if ((on != nullptr) && (view != nullptr) && (onItem != nullptr))
        {
            // Drawn out of the right side, so the loop has two DISTINCT anchors to preserve: the
            // two-click default puts both at the same border point, which is a separate case below.
            const QRectF box  = onItem->getBoxGeometry();
            const double rad  = onItem->boxCornerRadius();
            const double outY = box.top() + rad + 8.0;
            const double back = box.bottom() - rad - 8.0;
            const SMStateData* level = d.findLevel(canvas.getLevelId());
            double corner = box.right() + 120.0;
            if (level != nullptr)
            {
                for (const SMStateEntry* sibling : level->getElements())
                {
                    SMStateItem* item = canvas.stateItem(sibling->getId());
                    if (item != nullptr)
                    {
                        corner = std::max(corner, item->getBoxGeometry().right() + 120.0);
                    }
                }
            }

            canvas.setSnapToGrid(false);
            canvas.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
            clickScene(*view, QPointF(box.right() - rad - 4.0, outY));   // press inside the right side
            clickScene(*view, QPointF(corner, outY));                    // out
            clickScene(*view, QPointF(corner, back));                    // along
            clickScene(*view, QPointF(box.right() - rad - 4.0, back));   // back onto the same box
            QApplication::processEvents();

            const uint32_t selfTx = on->getTransitions().getElements().last()->getId();
            SMEdgeItem* loop = dynamic_cast<SMEdgeItem*>(canvas.findCanvasItem(selfTx));
            CHECK(loop != nullptr);
            if (loop != nullptr)
            {
                const QPointF from = loop->getPath().first();
                const QPointF to   = loop->getPath().last();
                CHECK(std::hypot(from.x() - to.x(), from.y() - to.y()) > 1.0);   // two anchors, not one

                loop->setShape(SMLayoutEdge::eShape::Arc);
                CHECK(loop->getShape() == SMLayoutEdge::eShape::Arc);
                loop->setSelected(true);
                view->centerOn(box.center() + QPointF(NESMDesign::EdgeSelfLoopStandoff, 0.0));
                QApplication::processEvents();
                grab(page, "g-arc-self-loop");

                const SMLayoutEdge* curved = d.getLayout().findEdge(selfTx);
                CHECK((curved != nullptr) && (curved->shape == SMLayoutEdge::eShape::Arc));
                // An arc is its two endpoints plus a bulge -- the loop's corners are gone, and the
                // self-loop default must not seed them back in behind the shape's back.
                CHECK((curved != nullptr) && (curved->points.size() == 2));
                CHECK((curved != nullptr) && (std::abs(curved->bulge) > 1e-6));
                // Where the loop left and re-entered the box is kept, not re-derived.
                CHECK((curved != nullptr) && (std::hypot(curved->points.first().x() - from.x()
                                                       , curved->points.first().y() - from.y()) < 0.5));
                CHECK((curved != nullptr) && (std::hypot(curved->points.last().x() - to.x()
                                                       , curved->points.last().y() - to.y()) < 0.5));

                // The curve bows AWAY from its own state: no drawn point falls inside the box, so
                // the loop never reads as a line crossing the state it belongs to.
                const QList<QPointF> drawn = loop->getPath();
                CHECK(drawn.size() > 2);
                bool   outside   = true;
                double apexStand = 0.0;
                for (const QPointF& p : drawn)
                {
                    outside   = outside && (box.adjusted(0.5, 0.5, -0.5, -0.5).contains(p) == false);
                    apexStand = std::max(apexStand, p.x() - box.right());
                }

                CHECK(outside);
                // It stands off the border about as far as the polyline loop's corners did, so
                // switching the shape does not resize the loop.
                CHECK(apexStand > (NESMDesign::EdgeSelfLoopStandoff / 2.0));

                // Back to a polyline: a two-point run between anchors on the same box would draw
                // nothing, so straightening restores the rectangle -- begin, two corners, end.
                loop->setShape(SMLayoutEdge::eShape::Line);
                CHECK(loop->getShape() == SMLayoutEdge::eShape::Line);
                const SMLayoutEdge* flat = d.getLayout().findEdge(selfTx);
                CHECK((flat != nullptr) && (flat->points.size() == 4));
                CHECK((flat != nullptr) && (std::abs(flat->bulge) < 1e-6));
                CHECK(loop->getPath().size() == 4);
                if ((flat != nullptr) && (flat->points.size() == 4))
                {
                    // Each corner stands straight out from the anchor it belongs to: the legs that
                    // leave and re-enter the box are square, which is what makes it a rectangle.
                    CHECK(std::abs(flat->points.at(1).y() - flat->points.first().y()) < 0.5);
                    CHECK(std::abs(flat->points.at(2).y() - flat->points.last().y()) < 0.5);
                    CHECK(std::abs((flat->points.at(1).x() - box.right()) - NESMDesign::EdgeSelfLoopStandoff) < 0.5);
                    CHECK(std::abs((flat->points.at(2).x() - box.right()) - NESMDesign::EdgeSelfLoopStandoff) < 0.5);
                }
            }

            // The two-click default loop puts both anchors on the same border point. A chord of
            // zero length is not a curve, so curving it falls back to a symmetric pair -- it must
            // still come out drawable rather than collapsing to nothing.
            canvas.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
            clickScene(*view, box.center());
            clickScene(*view, box.center());
            QApplication::processEvents();
            SMEdgeItem* plain = dynamic_cast<SMEdgeItem*>(
                        canvas.findCanvasItem(on->getTransitions().getElements().last()->getId()));
            CHECK(plain != nullptr);
            if (plain != nullptr)
            {
                plain->setShape(SMLayoutEdge::eShape::Arc);
                CHECK(plain->getShape() == SMLayoutEdge::eShape::Arc);
                const QPointF a = plain->getPath().first();
                const QPointF b = plain->getPath().last();
                CHECK(std::hypot(a.x() - b.x(), a.y() - b.y()) > 1.0);
                CHECK(plain->getPath().size() > 2);      // a real curve, not a collapsed chord
            }
        }
    }

    std::printf("sect: issue 9-1 a deliberate angle keeps its anchor, a near-straight leg is squared\n");
    {
        // The box the leg leaves: left 100, top 100, right 260, bottom 164.
        const QRectF box(100.0, 100.0, 160.0, 64.0);
        const QPointF press(150.0, 100.0);          // where the user pressed on the top border

        CHECK(NESMDesign::isNearAxis(QPointF(4.0, -200.0)));         // ~1 deg off vertical
        CHECK(NESMDesign::isNearAxis(QPointF(-200.0, 6.0)));         // ~2 deg off horizontal
        CHECK(NESMDesign::isNearAxis(QPointF(250.0, -200.0)) == false);  // ~39 deg: deliberate

        // Almost straight up: tidied to exactly straight, so the anchor takes the waypoint's column.
        const QPointF nearAxis(154.0, -100.0);
        const QPointF squared = NESMDesign::polylineAnchorPoint(box, 0.0, press, nearAxis, 16, false);
        CHECK(std::abs(squared.x() - nearAxis.x()) < 0.5);

        // A deliberate diagonal keeps the pressed anchor -- it must NOT jump onto the waypoint's
        // column, which is what made an angled transition snap straight (issue 9-1).
        const QPointF diagonal(400.0, -100.0);
        const QPointF kept = NESMDesign::polylineAnchorPoint(box, 0.0, press, diagonal, 16, false);
        CHECK(std::abs(kept.x() - press.x()) < 0.5);
        CHECK(std::abs(kept.x() - diagonal.x()) > 1.0);
    }

    std::printf("sect: issue 9-1 a press just outside a state still starts a transition\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        SMScene& canvas = page.getScene();
        SMStateEntry* off = doc.getData().findState("LightOff");
        SMStateItem* item = (off != nullptr) ? canvas.stateItem(off->getId()) : nullptr;
        CHECK((off != nullptr) && (item != nullptr));

        if ((off != nullptr) && (item != nullptr))
        {
            const QRectF box = item->getBoxGeometry();
            // Two units outside the left border: aiming at the exact border is not reasonable.
            const QPointF outside(box.left() - 2.0, box.center().y());
            CHECK(canvas.stateAt(outside) == nullptr);                                  // strictly outside
            CHECK(canvas.stateNear(outside, NESMDesign::StatePickMargin) == item);       // still picks it

            // Far away stays nothing, so the margin does not swallow the empty canvas.
            CHECK(canvas.stateNear(QPointF(box.left() - 400.0, box.center().y())
                                  , NESMDesign::StatePickMargin) == nullptr);
        }
    }

    std::printf("sect: issue 9-2 one flat entry states the shape it would switch to\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        SMScene& canvas = page.getScene();
        SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(canvas.findCanvasItem(27));
        CHECK(edge != nullptr);

        // The Design menu carries it too, so it is reachable without right-clicking the edge.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ 27u });
        QApplication::processEvents();
        QMenu designMenu;
        page.populateDesignMenu(designMenu);
        QAction* shapeEntry = nullptr;
        for (QAction* action : designMenu.actions())
        {
            if (action->text() == QStringLiteral("Make Arc"))
            {
                shapeEntry = action;
            }
        }

        CHECK(shapeEntry != nullptr);
        CHECK((shapeEntry != nullptr) && shapeEntry->isEnabled());   // exactly one transition selected

        if ((shapeEntry != nullptr) && (edge != nullptr))
        {
            // Triggering it curves the edge, and the entry then offers the way back.
            shapeEntry->trigger();
            QApplication::processEvents();
            CHECK(edge->getShape() == SMLayoutEdge::eShape::Arc);

            QMenu again;
            page.populateDesignMenu(again);
            bool offersPolyline = false;
            for (QAction* action : again.actions())
            {
                if (action->text() == QStringLiteral("Make Polyline"))
                {
                    offersPolyline = true;
                }
            }

            CHECK(offersPolyline);
        }

        // Nothing selected: the entry is present but dead, never a silent no-op on a guess.
        doc.getSelectionModel().setSelection(QList<uint32_t>{});
        QApplication::processEvents();
        QMenu empty;
        page.populateDesignMenu(empty);
        for (QAction* action : empty.actions())
        {
            if ((action->text() == QStringLiteral("Make Arc")) || (action->text() == QStringLiteral("Make Polyline")))
            {
                CHECK(action->isEnabled() == false);
            }
        }
    }

    std::printf("sect: SM-28 history mode -- offered only on a composite, badge survives a collapse\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        SMPropertiesPanel props(doc);
        props.resize(320, 600);
        props.show();
        QApplication::processEvents();

        StateMachineData& d = doc.getData();
        SMScene& canvas = page.getScene();
        SMStateEntry* composite = d.findState("LightOn");
        SMStateEntry* leaf = d.findState("LightOff");
        CHECK((composite != nullptr) && composite->isComposite());
        CHECK((leaf != nullptr) && (leaf->isComposite() == false));

        // The plain state gets the field, greyed: the mode stays discoverable, never appliable.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ leaf->getId() });
        QApplication::processEvents();
        CHECK(props.currentPage() == SMPropertiesPanel::PageState);
        CHECK(props.stateHistoryCombo()->isEnabled() == false);

        QMenu leafMenu;
        page.populateDesignMenu(leafMenu);
        QAction* leafEntry = nullptr;
        for (QAction* action : leafMenu.actions())
        {
            if (action->text().startsWith(QStringLiteral("History")))
            {
                leafEntry = action;
            }
        }

        CHECK((leafEntry != nullptr) && (leafEntry->isEnabled() == false));

        // The composite gets a live field, starting at the document's value.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ composite->getId() });
        QApplication::processEvents();
        CHECK(props.stateHistoryCombo()->isEnabled());
        CHECK(props.stateHistoryCombo()->currentData().toInt() == static_cast<int>(composite->getHistory()));

        // Applying Deep through the Design menu is one undo step, and the canvas picks it up.
        const int before = doc.getUndoStack().count();
        QMenu compMenu;
        page.populateDesignMenu(compMenu);
        QAction* historyEntry = nullptr;
        QAction* deep = nullptr;
        for (QAction* action : compMenu.actions())
        {
            if (action->text().startsWith(QStringLiteral("History")) == false)
            {
                continue;
            }

            historyEntry = action;
            for (QAction* mode : action->menu()->actions())
            {
                if (mode->text() == QStringLiteral("Deep"))
                {
                    deep = mode;
                }
            }
        }

        // The entry has to be LIVE, not merely present: triggering a mode action succeeds even
        // inside a disabled submenu, so asserting the effect alone hid a dead menu once already.
        CHECK((historyEntry != nullptr) && historyEntry->isEnabled());
        CHECK((historyEntry != nullptr) && (historyEntry->text() == QStringLiteral("History")));
        CHECK(deep != nullptr);
        if (deep != nullptr)
        {
            deep->trigger();
            QApplication::processEvents();
        }

        CHECK(doc.getUndoStack().count() == (before + 1));
        CHECK(composite->getHistory() == SMStateEntry::eHistory::Deep);
        CHECK(props.stateHistoryCombo()->currentData().toInt() == static_cast<int>(SMStateEntry::eHistory::Deep));

        SMStateItem* box = canvas.stateItem(composite->getId());
        CHECK((box != nullptr) && (box->getHistoryBadge() == SMStateEntry::eHistory::Deep));

        // Collapsing hides the body, never the header -- the badge belongs to the header.
        if (box != nullptr)
        {
            const bool wasExpanded = box->isExpanded();
            box->toggleExpanded();
            QApplication::processEvents();
            CHECK(box->isExpanded() != wasExpanded);
            CHECK(box->getHistoryBadge() == SMStateEntry::eHistory::Deep);
            grab(page, "g28-history-collapsed");
        }

        doc.getUndoStack().undo();                  // the collapse
        doc.getUndoStack().undo();                  // the history change
        QApplication::processEvents();
        CHECK(composite->getHistory() != SMStateEntry::eHistory::Deep);
        SMStateItem* again = canvas.stateItem(composite->getId());
        CHECK((again != nullptr) && (again->getHistoryBadge() != SMStateEntry::eHistory::Deep));
    }

    // --- SM-29-EXT: one control, three meanings, and the label says which ---
    {
        std::printf("[SM-29-EXT] the enter/add button renames itself with the selection\n");

        QAction* enterAction = design.actionEnterSubmachine();
        CHECK(enterAction != nullptr);

        // A state of its own rather than one borrowed from the fixture: the sections above have
        // already made every root Normal state composite.
        SMCreateStateCommand* create = new SMCreateStateCommand(data, model.getNotifier(), data.getStates()
                                                                , QStringLiteral("ExtProbe"), SMStateEntry::eStateKind::Normal
                                                                , QRectF(900.0, 600.0, 160.0, 80.0), QStringLiteral("Add state"));
        model.getUndoStack().push(create);
        const uint32_t plainId = create->getStateId();
        CHECK(data.findStateById(plainId) != nullptr);
        if (plainId != 0)
        {
            model.getSelectionModel().setSelection(QList<uint32_t>{ plainId });
            QApplication::processEvents();
            CHECK(enterAction->isEnabled());
            CHECK(enterAction->text() == QStringLiteral("Add Substate"));

            // Painted: the only thing left to do with it is descend.
            SMConvertToCompositeCommand* convert =
                    new SMConvertToCompositeCommand(data, model.getNotifier(), plainId, QStringLiteral("ExtStart")
                                                    , QRectF(32.0, 32.0, 48.0, 48.0), QStringLiteral("Add substate"));
            CHECK(convert->isEffective());
            model.getUndoStack().push(convert);
            model.getSelectionModel().setSelection(QList<uint32_t>{ plainId });
            QApplication::processEvents();
            CHECK(enterAction->text() == QStringLiteral("Enter Submachine"));

            // Removing the painted subtree gives the state back its plain meaning, in one step.
            SMRemoveCompositeCommand* flatten =
                    new SMRemoveCompositeCommand(data, model.getNotifier(), plainId, QStringLiteral("Remove submachine"));
            CHECK(flatten->isEffective());
            CHECK(flatten->removedStateCount() == 1);
            model.getUndoStack().push(flatten);
            model.getSelectionModel().setSelection(QList<uint32_t>{ plainId });
            QApplication::processEvents();
            CHECK(data.findStateById(plainId)->hasNestedStates() == false);
            CHECK(enterAction->text() == QStringLiteral("Add Substate"));

            // Imported: the machine lives in another file, so the button opens it instead.
            IncludeEntry* import = data.getIncludes().createInclude(QStringLiteral("./ExtCycle.fsml"));
            CHECK(import != nullptr);
            if (import != nullptr)
            {
                import->setAlias(QStringLiteral("ExtCycle"));
            }

            SMSetSubmachineCommand* link = new SMSetSubmachineCommand(data, model.getNotifier(), plainId
                                                                      , QStringLiteral("ExtCycle"), QStringLiteral("Host"));
            CHECK(link->isEffective());
            model.getUndoStack().push(link);
            model.getSelectionModel().setSelection(QList<uint32_t>{ plainId });
            QApplication::processEvents();
            CHECK(enterAction->text() == QStringLiteral("Open Imported Machine"));
            CHECK(design.actionRemoveSubmachine()->isEnabled());

            // Start states host nothing, so every one of them is dead here.
            const SMStateEntry* start = data.findState(QStringLiteral("Off"));
            if (start != nullptr)
            {
                model.getSelectionModel().setSelection(QList<uint32_t>{ start->getId() });
                QApplication::processEvents();
                CHECK(enterAction->isEnabled() == false);
                CHECK(design.actionRemoveSubmachine()->isEnabled() == false);
            }

            model.getUndoStack().undo();            // the link
            model.getUndoStack().undo();            // the flatten
            model.getUndoStack().undo();            // the convert
            model.getUndoStack().undo();            // the probe state
            model.getSelectionModel().clearSelection();
            QApplication::processEvents();
            CHECK(data.findStateById(plainId) == nullptr);
        }
    }

    // --- The Properties dock keeps its width when the selection changes ---
    {
        std::printf("[panel] the selection fills the Properties panel, it does not resize it\n");

        // Its own window: the sections above have dragged this page's docks around.
        StateMachineModel wmodel;
        CHECK(wmodel.loadFromFile(QString::fromLocal8Bit(argv[1])));
        SMDesign wdesign(wmodel);
        wdesign.resize(1400, 900);
        wdesign.show();
        QApplication::processEvents();

        QDockWidget* dock = wdesign.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        SMPropertiesPanel* panel = (dock != nullptr ? qobject_cast<SMPropertiesPanel*>(dock->widget()) : nullptr);
        CHECK(dock != nullptr);
        CHECK(panel != nullptr);
        if ((dock != nullptr) && (panel != nullptr))
        {
            const int width = dock->width();
            const int empty = panel->sizeHint().width();
            CHECK(width == NESMDesign::PanelDefaultWidth);

            // Whatever the panel puts on screen, it must not ask for more room than it did with
            // nothing selected -- that request is what used to widen the dock over the canvas.
            const SMStateEntry* wstate = wmodel.getData().findState(QStringLiteral("LightOff"));
            CHECK(wstate != nullptr);
            if (wstate != nullptr)
            {
                wmodel.getSelectionModel().setSelection(QList<uint32_t>{ wstate->getId() });
                QApplication::processEvents();
                CHECK(dock->width() == width);
                CHECK(panel->sizeHint().width() <= empty);
            }

            CHECK(wmodel.getData().findTransitionById(27) != nullptr);
            if (wmodel.getData().findTransitionById(27) != nullptr)
            {
                wmodel.getSelectionModel().setSelection(QList<uint32_t>{ 27u });
                QApplication::processEvents();
                CHECK(dock->width() == width);
                CHECK(panel->sizeHint().width() <= empty);
            }

            wmodel.getSelectionModel().clearSelection();
            QApplication::processEvents();
            CHECK(dock->width() == width);

            // A width the user set stays set, selection or not.
            wdesign.resizeDocks(QList<QDockWidget*>{ dock }, QList<int>{ 640 }, Qt::Horizontal);
            QApplication::processEvents();
            const int dragged = dock->width();
            wmodel.getSelectionModel().setSelection(QList<uint32_t>{ 27u });
            QApplication::processEvents();
            CHECK(dock->width() == dragged);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // L1: the Start pseudo-state offers nothing to act with
    //////////////////////////////////////////////////////////////////////////
    {
        std::printf("[L1] the Start pseudo-state offers no operations, and its transition no stimulus\n");

        StateMachineModel lmodel;
        CHECK(lmodel.loadFromFile(QString::fromLocal8Bit(argv[1])));
        SMDesign ldesign(lmodel);
        ldesign.resize(1400, 900);
        ldesign.show();
        QApplication::processEvents();

        QDockWidget* ldock = ldesign.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        SMPropertiesPanel* lpanel = (ldock != nullptr ? qobject_cast<SMPropertiesPanel*>(ldock->widget()) : nullptr);
        StateMachineData& ldata = lmodel.getData();
        const SMStateData* lroot = ldata.findLevel(ldesign.getScene().getLevelId());
        const SMStateEntry* start = (lroot != nullptr ? lroot->getStartState() : nullptr);
        const SMStateEntry* plain = ldata.findState(QStringLiteral("LightOff"));
        CHECK(lpanel != nullptr);
        CHECK(start != nullptr);
        CHECK(plain != nullptr);

        QTabWidget* stateTabs = (lpanel != nullptr ? lpanel->findChild<QTabWidget*>(QStringLiteral("smStateTabs")) : nullptr);
        CHECK(stateTabs != nullptr);
        if ((lpanel != nullptr) && (stateTabs != nullptr) && (start != nullptr) && (plain != nullptr))
        {
            // An ordinary state offers all four tabs: General, Enter, Exit, Internal. The three
            // after General are the three things a state does without leaving itself, and Internal
            // used to be missing from that set -- reachable only by double-clicking a row in a
            // collapsible list on the General tab.
            lmodel.getSelectionModel().setSelection(QList<uint32_t>{ plain->getId() });
            QApplication::processEvents();
            CHECK(stateTabs->count() == 4);
            CHECK(stateTabs->isTabVisible(1) && stateTabs->isTabVisible(2));
            CHECK(stateTabs->isTabVisible(3));

            // The Start offers only General: a pseudo-state performs nothing, so the editor must
            // not present a place to put operations, and there is no behaviour to describe.
            lmodel.getSelectionModel().setSelection(QList<uint32_t>{ start->getId() });
            QApplication::processEvents();
            CHECK(stateTabs->isTabVisible(1) == false);
            CHECK(stateTabs->isTabVisible(2) == false);
            CHECK(stateTabs->isTabVisible(3) == false);     // everything a Start owns is initial
            CHECK(stateTabs->currentIndex() == 0);
            QPlainTextEdit* desc = lpanel->findChild<QPlainTextEdit*>(QStringLiteral("smStateDescription"));
            CHECK(desc != nullptr);
            CHECK((desc != nullptr) && (desc->isVisibleTo(lpanel) == false));

            // Its outgoing transition is the level's initial one: the stimulus picker has nothing
            // to offer and the source may not be moved off the Start.
            CHECK(start->getTransitions().getElementCount() >= 1);
            if (start->getTransitions().getElementCount() >= 1)
            {
                const uint32_t initialId = start->getTransitions().getElements().first()->getId();
                lmodel.getSelectionModel().setSelection(QList<uint32_t>{ initialId });
                QApplication::processEvents();
                CHECK(lpanel->currentElementId() == initialId);
                CHECK(lpanel->stimulusNameCombo() != nullptr);
                CHECK((lpanel->stimulusNameCombo() != nullptr) && (lpanel->stimulusNameCombo()->isEnabled() == false));
                CHECK((lpanel->sourceCombo() != nullptr) && (lpanel->sourceCombo()->isEnabled() == false));

                // And the document keeps saying so: committing a stimulus onto it is refused.
                const QString before = ldata.findTransitionById(initialId)->getStimulus();
                if ((lpanel->stimulusNameCombo() != nullptr) && (lpanel->stimulusNameCombo()->count() > 1))
                {
                    lpanel->stimulusNameCombo()->setCurrentIndex(1);
                    QMetaObject::invokeMethod(lpanel, "onStimulusCommit");
                    QApplication::processEvents();
                }

                CHECK(ldata.findTransitionById(initialId)->getStimulus() == before);
            }
        }

        grab(ldesign, "l1-pseudo-start");
    }

    //////////////////////////////////////////////////////////////////////////
    // L2: a transition's kind is a visible, editable property
    //////////////////////////////////////////////////////////////////////////
    {
        std::printf("[L2] the transition Kind is shown and editable, and the canvas follows it\n");

        StateMachineModel kmodel;
        CHECK(kmodel.loadFromFile(QString::fromLocal8Bit(argv[1])));
        SMDesign kdesign(kmodel);
        kdesign.resize(1400, 900);
        kdesign.show();
        QApplication::processEvents();

        QDockWidget* kdock = kdesign.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        SMPropertiesPanel* kpanel = (kdock != nullptr ? qobject_cast<SMPropertiesPanel*>(kdock->widget()) : nullptr);
        StateMachineData& kdata = kmodel.getData();
        SMScene& kscene = kdesign.getScene();
        const SMStateData* kroot = kdata.findLevel(kscene.getLevelId());
        const SMStateEntry* kstart = (kroot != nullptr ? kroot->getStartState() : nullptr);
        CHECK(kpanel != nullptr);
        CHECK(kstart != nullptr);

        if ((kpanel != nullptr) && (kstart != nullptr) && (kroot != nullptr))
        {
            QComboBox* kindCombo = kpanel->transitionKindCombo();
            CHECK(kindCombo != nullptr);

            // An ordinary external transition: the kind is shown, and it is editable.
            uint32_t externalId = 0;
            for (const SMStateEntry* st : kroot->getElements())
            {
                if ((st == nullptr) || st->isPseudoStart())
                    continue;
                for (const SMTransitionEntry* tr : st->getTransitions().getElements())
                {
                    if ((externalId == 0) && (tr != nullptr) && tr->isExternal())
                        externalId = tr->getId();
                }
            }

            CHECK(externalId != 0);
            if ((externalId != 0) && (kindCombo != nullptr))
            {
                kmodel.getSelectionModel().setSelection(QList<uint32_t>{ externalId });
                QApplication::processEvents();
                CHECK(kpanel->currentElementId() == externalId);
                CHECK(kindCombo->isEnabled());
                CHECK(kindCombo->currentData().toInt() == static_cast<int>(SMTransitionEntry::eTransitionKind::External));

                // An external transition has an edge on the canvas; making it internal takes the
                // edge away, and undo brings both the kind and the target back.
                const uint32_t oldTarget = kdata.findTransitionById(externalId)->getToId();
                CHECK(kscene.findCanvasItem(externalId) != nullptr);
                kindCombo->setCurrentIndex(1);
                QMetaObject::invokeMethod(kpanel, "onTransKindCommit");
                QApplication::processEvents();
                CHECK(kdata.findTransitionById(externalId)->isInternal());
                CHECK(kdata.findTransitionById(externalId)->hasTarget() == false);
                CHECK(kscene.findCanvasItem(externalId) == nullptr);

                kmodel.getUndoStack().undo();
                QApplication::processEvents();
                CHECK(kdata.findTransitionById(externalId)->isExternal());
                CHECK(kdata.findTransitionById(externalId)->getToId() == oldTarget);
                CHECK(kscene.findCanvasItem(externalId) != nullptr);
            }

            // The level's initial transition: the kind is Initial, and it is not editable -- a
            // Start owns nothing else, so there is no other kind to offer.
            CHECK(kstart->getTransitions().getElementCount() >= 1);
            if ((kstart->getTransitions().getElementCount() >= 1) && (kindCombo != nullptr))
            {
                const uint32_t initialId = kstart->getTransitions().getElements().first()->getId();
                CHECK(kdata.findTransitionById(initialId)->isInitial());
                kmodel.getSelectionModel().setSelection(QList<uint32_t>{ initialId });
                QApplication::processEvents();
                CHECK(kindCombo->currentData().toInt() == static_cast<int>(SMTransitionEntry::eTransitionKind::Initial));
                CHECK(kindCombo->isEnabled() == false);
            }
        }

        grab(kdesign, "l2-transition-kind");
    }

    //////////////////////////////////////////////////////////////////////////
    // L4: one glyph per concept, and the internal transition is editable where
    //     the author is looking
    //////////////////////////////////////////////////////////////////////////
    {
        std::printf("[L4] cause and effect wear different marks, and the Internal tab edits in place\n");

        StateMachineModel imodel;
        CHECK(imodel.loadFromFile(QString::fromLocal8Bit(argv[1])));
        SMDesign idesign(imodel);
        idesign.resize(1400, 900);
        idesign.show();
        QApplication::processEvents();

        StateMachineData& idata = imodel.getData();
        QDockWidget* idock = idesign.findChild<QDockWidget*>(QStringLiteral("SMPropertiesDock"));
        SMPropertiesPanel* ipanel = (idock != nullptr ? qobject_cast<SMPropertiesPanel*>(idock->widget()) : nullptr);
        CHECK(ipanel != nullptr);

        // No two constructs share a mark. This is the whole of defects 1, 2 and 4: `Trigger` used
        // to draw nothing at all, so `on <timer>` and `<action>()` were one and the same row to a
        // reader.
        const QList<SMKindGlyph::eGlyph> vocabulary
        {
              SMKindGlyph::eGlyph::Entry,   SMKindGlyph::eGlyph::Exit
            , SMKindGlyph::eGlyph::Internal
            , SMKindGlyph::eGlyph::Action,  SMKindGlyph::eGlyph::Trigger
            , SMKindGlyph::eGlyph::Event,   SMKindGlyph::eGlyph::TimerStart
            , SMKindGlyph::eGlyph::TimerStop
        };
        for (SMKindGlyph::eGlyph glyph : vocabulary)
        {
            CHECK(SMKindGlyph::isDrawn(glyph));     // every one of them is actually drawn
        }

        CHECK(SMKindGlyph::icon(SMKindGlyph::eGlyph::Trigger, QColor(Qt::black)).isNull() == false);
        CHECK(SMKindGlyph::icon(SMKindGlyph::eGlyph::Action, QColor(Qt::black)).isNull() == false);

        // Find (or make) a state carrying an internal transition on a TIMER with one action, the
        // TRAFFIC_LIGHT_RED shape: `on <timer>` over `<action>()`.
        SMStateEntry* host = nullptr;
        for (SMStateEntry* s : idata.getStates().getElements())
        {
            if ((host == nullptr) && (s != nullptr) && (s->getKind() == SMStateEntry::eStateKind::Normal))
            {
                host = s;
            }
        }

        CHECK(host != nullptr);
        if ((host != nullptr) && (ipanel != nullptr))
        {
            CHECK(idata.getTimers().createTimer(QStringLiteral("L4Timer")) != nullptr);
            imodel.getSelectionModel().setSelection(QList<uint32_t>{ host->getId() });
            QApplication::processEvents();

            SMInternalEditor* ieditor = ipanel->internalEditor();
            CHECK(ieditor != nullptr);
            const int before = (ieditor != nullptr ? ieditor->count() : -1);

            // Add one from the tab itself, then give it a timer stimulus and one action.
            QToolButton* addButton = ipanel->findChild<QToolButton*>(QStringLiteral("smBtnAddInternal"));
            CHECK(addButton != nullptr);
            if ((addButton != nullptr) && (ieditor != nullptr))
            {
                addButton->click();
                QApplication::processEvents();
                CHECK(ieditor->count() == (before + 1));
                CHECK(ipanel->internalEditor()->currentTransition() != 0u);

                const uint32_t internalId = ipanel->internalEditor()->currentTransition();
                SMTransitionEntry* added = idata.findTransitionById(internalId);
                CHECK(added != nullptr);
                CHECK((added != nullptr) && added->isInternal());

                // The stimulus, picked in place -- no trip to the transition page.
                QComboBox* picker = ipanel->internalEditor()->stimulusCombo();
                CHECK(picker != nullptr);
                int timerRow = -1;
                for (int row = 1; (picker != nullptr) && (row < picker->count()); ++row)
                {
                    if (picker->itemData(row, Qt::UserRole + 1).toString() == QStringLiteral("L4Timer"))
                    {
                        timerRow = row;
                    }
                }

                CHECK(timerRow > 0);
                if ((timerRow > 0) && (picker != nullptr))
                {
                    picker->setCurrentIndex(timerRow);
                    QMetaObject::invokeMethod(ipanel->internalEditor(), "onStimulusCommit");
                    QApplication::processEvents();
                    CHECK(idata.findTransitionById(internalId)->getStimulus() == QStringLiteral("L4Timer"));
                    CHECK(idata.findTransitionById(internalId)->getStimulusKind() == SMTransitionEntry::eStimulusKind::Timer);
                }

                added = idata.findTransitionById(internalId);
                if (added != nullptr)
                {
                    added->getOperations().addOperation(new SMActionCall(0, QStringLiteral("doWork")));
                }

                // The box now draws the pair, and the two rows do NOT read the same. The header
                // carries the band mark (internal) AND the stimulus kind (a clock); the operation
                // below carries the gear.
                SMStateItem* box = dynamic_cast<SMStateItem*>(idesign.getScene().findCanvasItem(host->getId()));
                CHECK(box != nullptr);
                if (box != nullptr)
                {
                    box->updateFromModel();
                    const QList<SMStateItem::BodyRow> rows = box->getBodyRows();
                    int headerRow = -1;
                    for (int i = 0; i < rows.size(); ++i)
                    {
                        if (rows.at(i).transitionId == internalId)
                        {
                            headerRow = i;
                        }
                    }

                    CHECK(headerRow >= 0);
                    if ((headerRow >= 0) && ((headerRow + 1) < rows.size()))
                    {
                        CHECK(rows.at(headerRow).icon == SMKindGlyph::eGlyph::Internal);
                        CHECK(rows.at(headerRow).kindIcon == SMKindGlyph::eGlyph::TimerStart);
                        CHECK(rows.at(headerRow).text.startsWith(QStringLiteral("on ")));
                        // The effect wears a different mark from the cause -- the defect in one line.
                        CHECK(rows.at(headerRow + 1).icon == SMKindGlyph::eGlyph::Action);
                        CHECK(rows.at(headerRow + 1).icon != rows.at(headerRow).icon);
                        CHECK(rows.at(headerRow + 1).icon != rows.at(headerRow).kindIcon);
                    }
                }

                // The canvas row routes to the Internal tab, not to the stimulus declaration.
                QTabWidget* itabs = ipanel->findChild<QTabWidget*>(QStringLiteral("smStateTabs"));
                CHECK(itabs != nullptr);
                if (itabs != nullptr)
                {
                    itabs->setCurrentIndex(0);
                    idesign.getScene().requestInternalEdit(internalId);
                    QApplication::processEvents();
                    CHECK(ipanel->currentPage() == SMPropertiesPanel::PageState);
                    CHECK(ipanel->currentElementId() == host->getId());
                    CHECK(ipanel->internalEditor()->currentTransition() == internalId);
                    CHECK(itabs->currentIndex() == (itabs->count() - 1));    // the Internal tab
                }

                // The sections read in the order the author reads a transition: what fires it,
                // what it does, then when it is allowed to.
                QTabWidget* iinner = ipanel->internalEditor()->tabs();
                CHECK(iinner != nullptr);
                if (iinner != nullptr)
                {
                    CHECK(iinner->count() == 2);
                    CHECK(iinner->tabText(0) == QStringLiteral("Actions"));
                    CHECK(iinner->tabText(1) == QStringLiteral("Conditions"));
                }

                // The context-menu path opens the SAME editor in a dialog, exactly as Enter/Exit
                // Actions do -- and it is offered for a state that has none, because that is where
                // the author goes to make the first one.
                {
                    SMInternalDialog dialog(imodel, QStringLiteral("Internal"), host->getId(), internalId);
                    CHECK(dialog.editor() != nullptr);
                    CHECK((dialog.editor() != nullptr) && (dialog.editor()->stateId() == host->getId()));
                    CHECK((dialog.editor() != nullptr) && (dialog.editor()->currentTransition() == internalId));
                    // Its guard bar must NOT answer to the name the TRANSITION page owns: the
                    // hosted bar is re-prefixed in every instance of this editor.
                    CHECK(dialog.findChild<QWidget*>(QStringLiteral("smGuardField")) == nullptr);
                    CHECK(dialog.findChild<QWidget*>(QStringLiteral("smInternalSmGuardField")) != nullptr);
                }

                // And it can be taken away again from the same place.
                QToolButton* removeButton = ipanel->findChild<QToolButton*>(QStringLiteral("smBtnRemoveInternal"));
                CHECK(removeButton != nullptr);
                if ((removeButton != nullptr) && (ieditor != nullptr))
                {
                    removeButton->click();
                    QApplication::processEvents();
                    CHECK(idata.findTransitionById(internalId) == nullptr);
                    CHECK(ieditor->count() == before);

                    // A state with none still offers the editor: an empty list plus the Add button
                    // is the answer to "how do I make one?".
                    SMInternalDialog empty(imodel, QStringLiteral("Internal"), host->getId());
                    CHECK(empty.editor() != nullptr);
                    CHECK((empty.editor() != nullptr) && (empty.editor()->currentTransition() == 0u));
                    CHECK(empty.findChild<QToolButton*>(QStringLiteral("smBtnAddInternal")) != nullptr);
                }
            }
        }

        grab(idesign, "l4-internal-and-glyphs");
    }

    std::printf("sect: issue-550 parallel move, edge nudge, sticky anchors, collapse\n");
    // --- The four move/anchor rules, on a pair of boxes joined by one straight transition:
    //  1. every selected element travels the identical step, boxes and transitions alike;
    //  2. selected transitions answer the arrow keys on their own;
    //  3. an anchor stays on the border it was placed on, however far its box travels;
    //  4. collapsing a box pulls the endpoint onto the header, expanding gives it back. ---
    {
        StateMachineModel gmodel;
        CHECK(gmodel.loadFromFile(QString::fromLocal8Bit(argv[1])));
        SMDesign gdesign(gmodel);
        gdesign.resize(1200, 800);
        gdesign.show();
        QApplication::processEvents();

        SMScene&          gscene = gdesign.getScene();
        SMGraphicsView&   gview  = gdesign.getView();
        StateMachineData& gdata  = gmodel.getData();
        SMStateData*      groot  = gdata.findLevel(gscene.getLevelId());
        CHECK(groot != nullptr);

        // Two boxes well clear of the loaded diagram, joined left-border to right-border.
        const QRectF srcBox{ 3008.0, 3008.0, 208.0, 128.0 };
        const QRectF tgtBox{ 3408.0, 3008.0, 208.0, 128.0 };
        SMCreateStateCommand* mkSrc = new SMCreateStateCommand(  gdata, gmodel.getNotifier(), *groot
                                                               , QStringLiteral("MOVE_SRC"), SMStateEntry::eStateKind::Normal
                                                               , srcBox, QStringLiteral("src"));
        gmodel.getUndoStack().push(mkSrc);
        SMCreateStateCommand* mkTgt = new SMCreateStateCommand(  gdata, gmodel.getNotifier(), *groot
                                                               , QStringLiteral("MOVE_TGT"), SMStateEntry::eStateKind::Normal
                                                               , tgtBox, QStringLiteral("tgt"));
        gmodel.getUndoStack().push(mkTgt);
        const uint32_t srcId = mkSrc->getStateId();
        const uint32_t tgtId = mkTgt->getStateId();
        QApplication::processEvents();

        SMStateEntry* srcState = gdata.findStateById(srcId);
        CHECK(srcState != nullptr);
        const QPointF anchorBegin{ srcBox.right(), srcBox.center().y() };
        const QPointF anchorEnd  { tgtBox.left() , tgtBox.center().y() };
        SMCreateTransitionCommand* mkTx = new SMCreateTransitionCommand(  gdata, gmodel.getNotifier(), *srcState
                                                                        , SMTransitionEntry::eStimulusKind::Trigger
                                                                        , QStringLiteral("go"), tgtId
                                                                        , QList<QPointF>{ anchorBegin, anchorEnd }
                                                                        , QStringLiteral("tx"));
        gmodel.getUndoStack().push(mkTx);
        const uint32_t txId = mkTx->getTransitionId();
        QApplication::processEvents();

        SMStateItem* srcItem = gscene.stateItem(srcId);
        SMStateItem* tgtItem = gscene.stateItem(tgtId);
        SMEdgeItem*  txItem  = dynamic_cast<SMEdgeItem*>(gscene.findCanvasItem(txId));
        CHECK((srcItem != nullptr) && (tgtItem != nullptr) && (txItem != nullptr));

        gview.resetTransform();
        gview.centerOn((srcBox.center() + tgtBox.center()) / 2.0);
        QApplication::processEvents();

        if ((srcItem != nullptr) && (tgtItem != nullptr) && (txItem != nullptr))
        {
            CHECK(txItem->getPath().size() == 2);
            const double midRow = tgtBox.center().y();

            // --- Bug 3: the box travels UP past its own height; the endpoint must stay on the
            // LEFT border at the same point of it, never flip to the border that came nearest ---
            gscene.clearSelection();
            tgtItem->setSelected(true);
            QApplication::processEvents();
            for (int i = 0; i < 12; ++i)                            // 12 * 16 = 192 > box height
            {
                keyClickScene(gscene, Qt::Key_Up);
            }

            const QRectF liftedBox = tgtItem->getBoxGeometry();
            CHECK(std::abs(liftedBox.top() - (tgtBox.top() - 192.0)) < 1e-6);   // 12 identical steps
            CHECK(std::abs(txItem->getPath().last().x() - liftedBox.left()) < 1e-6);            // still LEFT
            CHECK(std::abs(txItem->getPath().last().y() - liftedBox.center().y()) < 1e-6);      // same point of it
            CHECK(std::abs(txItem->getPath().first().x() - srcBox.right()) < 1e-6);             // source untouched
            CHECK(std::abs(txItem->getPath().first().y() - srcBox.center().y()) < 1e-6);
            // The stored anchor moved with the box, so a reload cannot put it back on a stale point.
            const SMLayoutEdge* liftedEdge = gdata.getLayout().findEdge(txId);
            CHECK((liftedEdge != nullptr) && (std::abs(liftedEdge->points.last().y() - liftedBox.center().y()) < 1e-6));

            for (int i = 0; i < 12; ++i)
            {
                keyClickScene(gscene, Qt::Key_Down);
            }

            CHECK(std::abs(tgtItem->getBoxGeometry().top() - tgtBox.top()) < 1e-6);
            CHECK(std::abs(txItem->getPath().last().y() - midRow) < 1e-6);

            // --- Bug 1 (keyboard): select everything; boxes AND transition travel one step,
            // in the same direction, and the line keeps its length ---
            gscene.clearSelection();
            srcItem->setSelected(true);
            tgtItem->setSelected(true);
            txItem->setSelected(true);
            QApplication::processEvents();
            const QPointF beginBefore = txItem->getPath().first();
            const QPointF endBefore   = txItem->getPath().last();
            keyClickScene(gscene, Qt::Key_Left);
            const QPointF step{ -static_cast<double>(gscene.getGridSize()), 0.0 };
            CHECK(srcItem->getBoxGeometry().topLeft() == (srcBox.topLeft() + step));
            CHECK(tgtItem->getBoxGeometry().topLeft() == (tgtBox.topLeft() + step));
            CHECK(txItem->getPath().first() == (beginBefore + step));   // the transition kept pace
            CHECK(txItem->getPath().last()  == (endBefore   + step));
            keyClickScene(gscene, Qt::Key_Right);
            CHECK(srcItem->getBoxGeometry().topLeft() == srcBox.topLeft());
            CHECK(txItem->getPath().last() == endBefore);

            // --- Bug 2: the transition alone answers the arrow keys. Both anchors sit on
            // vertical borders, so a vertical step slides them and a horizontal one cannot ---
            gscene.clearSelection();
            txItem->setSelected(true);
            QApplication::processEvents();
            keyClickScene(gscene, Qt::Key_Down);
            CHECK(std::abs(txItem->getPath().first().y() - (beginBefore.y() + gscene.getGridSize())) < 1e-6);
            CHECK(std::abs(txItem->getPath().last().y()  - (endBefore.y()   + gscene.getGridSize())) < 1e-6);
            CHECK(std::abs(txItem->getPath().first().x() - beginBefore.x()) < 1e-6);     // stayed on its border
            CHECK(srcItem->getBoxGeometry().topLeft() == srcBox.topLeft());              // the boxes stood still
            const SMLayoutEdge* slidEdge = gdata.getLayout().findEdge(txId);
            CHECK((slidEdge != nullptr)
                  && (std::abs(slidEdge->points.last().y() - (endBefore.y() + gscene.getGridSize())) < 1e-6));
            gmodel.getUndoStack().undo();                                                // one undo step
            QApplication::processEvents();
            CHECK(std::abs(txItem->getPath().last().y() - endBefore.y()) < 1e-6);

            // --- Bug 1 (mouse): a two-box drag moves both by the identical step, however the
            // grid would round each of them on its own ---
            gscene.clearSelection();
            srcItem->setSelected(true);
            tgtItem->setSelected(true);
            QApplication::processEvents();
            const QPointF srcAt = srcItem->getBoxGeometry().topLeft();
            const QPointF tgtAt = tgtItem->getBoxGeometry().topLeft();
            const QPointF grab  = srcItem->getBoxGeometry().center();
            dragScene(gview, grab, grab + QPointF(96.0, 64.0));
            const QPointF srcStep = srcItem->getBoxGeometry().topLeft() - srcAt;
            const QPointF tgtStep = tgtItem->getBoxGeometry().topLeft() - tgtAt;
            CHECK((srcStep.x() != 0.0) || (srcStep.y() != 0.0));
            CHECK(srcStep == tgtStep);                                  // parallel, to the unit
            gmodel.getUndoStack().undo();
            QApplication::processEvents();
            CHECK(srcItem->getBoxGeometry().topLeft() == srcAt);
            CHECK(tgtItem->getBoxGeometry().topLeft() == tgtAt);

            // --- Bug 4: collapsing the target pulls the endpoint onto the header it can still
            // see; expanding gives the endpoint its old place back ---
            gscene.clearSelection();
            const int undoBeforeCollapse = gmodel.getUndoStack().index();
            tgtItem->toggleExpanded();
            QApplication::processEvents();
            CHECK(tgtItem->isExpanded() == false);
            CHECK(gmodel.getUndoStack().index() == (undoBeforeCollapse + 1));    // no extra move step
            const QRectF header = tgtItem->getVisibleGeometry();
            CHECK(std::abs(header.height() - NESMDesign::StateHeaderHeight) < 1e-6);
            const QPointF collapsedEnd = txItem->getPath().last();
            CHECK(std::abs(collapsedEnd.x() - header.left()) < 1e-6);            // still the left border
            CHECK(collapsedEnd.y() <= (header.bottom() + 1e-6));                 // and on the drawn part of it
            CHECK(collapsedEnd.y() >= (header.top() - 1e-6));
            CHECK(collapsedEnd != endBefore);                                    // it did move onto the header

            tgtItem->toggleExpanded();
            QApplication::processEvents();
            CHECK(tgtItem->isExpanded());
            CHECK(txItem->getPath().last() == endBefore);                        // restored exactly

            // --- Bug 1, self-loop: the corners a loop draws for itself are scene points too,
            // and both of its ends sit on the one box -- so they travel with it ---
            SMCreateTransitionCommand* mkLoop = new SMCreateTransitionCommand(  gdata, gmodel.getNotifier(), *srcState
                                                                              , SMTransitionEntry::eStimulusKind::Trigger
                                                                              , QStringLiteral("again"), srcId
                                                                              , QList<QPointF>(), QStringLiteral("loop"));
            gmodel.getUndoStack().push(mkLoop);
            QApplication::processEvents();
            SMEdgeItem* loopItem = dynamic_cast<SMEdgeItem*>(gscene.findCanvasItem(mkLoop->getTransitionId()));
            CHECK(loopItem != nullptr);
            if (loopItem != nullptr)
            {
                const QList<QPointF> loopBefore = loopItem->getPath();
                CHECK(loopBefore.size() == 4);                                  // begin, two corners, end
                gscene.clearSelection();
                srcItem->setSelected(true);
                QApplication::processEvents();
                keyClickScene(gscene, Qt::Key_Left);
                const QPointF loopStep{ -static_cast<double>(gscene.getGridSize()), 0.0 };
                const QList<QPointF> loopAfter = loopItem->getPath();
                CHECK(loopAfter.size() == loopBefore.size());
                for (int i = 0; (i < loopAfter.size()) && (i < loopBefore.size()); ++i)
                {
                    CHECK(loopAfter.at(i) == (loopBefore.at(i) + loopStep));    // the whole loop travelled
                }
            }
        }
    }

    std::printf("sect: a validation finding activates the document view and scrolls only when needed\n");
    {
        StateMachineModel doc;
        CHECK(doc.loadFromFile(sourcePath));
        SMDesign page(doc);
        page.resize(1400, 900);
        page.show();
        QApplication::processEvents();

        SMScene&          revealScene = page.getScene();
        SMGraphicsView&   revealView  = page.getView();
        StateMachineData& revealData  = doc.getData();

        SMStateEntry* target = revealData.findState("LightOff");
        CHECK(target != nullptr);
        SMCanvasItem* targetItem = revealScene.findCanvasItem(target->getId());
        CHECK(targetItem != nullptr);

        // Visible but off-centre: centring the view here (the old behaviour) would move the
        // scrollbars even though nothing needed to scroll, which is what this section catches.
        revealView.centerOn(targetItem);
        QApplication::processEvents();
        revealView.horizontalScrollBar()->setValue(revealView.horizontalScrollBar()->value() + 40);
        revealView.verticalScrollBar()->setValue(revealView.verticalScrollBar()->value() + 40);
        QApplication::processEvents();
        const QRect stillVisible = revealView.mapFromScene(targetItem->sceneBoundingRect()).boundingRect();
        CHECK(revealView.viewport()->rect().contains(stillVisible));
        revealView.clearFocus();
        const int hVisible = revealView.horizontalScrollBar()->value();
        const int vVisible = revealView.verticalScrollBar()->value();

        page.navigateToIssue(target->getId(), eDocElementKind::State);
        QApplication::processEvents();

        CHECK(revealView.hasFocus());
        CHECK(revealView.horizontalScrollBar()->value() == hVisible);
        CHECK(revealView.verticalScrollBar()->value() == vVisible);

        revealView.centerOn(QPointF(4000.0, 4000.0));
        QApplication::processEvents();
        revealView.clearFocus();
        const QRect farRect = revealView.mapFromScene(targetItem->sceneBoundingRect()).boundingRect();
        CHECK(revealView.viewport()->rect().intersects(farRect) == false);

        page.navigateToIssue(target->getId(), eDocElementKind::State);
        QApplication::processEvents();

        const QRect broughtBackRect = revealView.mapFromScene(targetItem->sceneBoundingRect()).boundingRect();
        CHECK(revealView.hasFocus());
        CHECK(revealView.viewport()->rect().intersects(broughtBackRect));
    }

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
