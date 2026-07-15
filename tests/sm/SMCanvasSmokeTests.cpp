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
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMClipboard.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
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
#include "lusan/view/sm/SMPropertiesPanel.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QApplication>
#include <QComboBox>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QGraphicsProxyWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QMimeData>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QScreen>
#include <QSettings>
#include <QTabWidget>
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
    void clickScene(SMGraphicsView& view, const QPointF& scenePos, Qt::MouseButton button = Qt::LeftButton)
    {
        const QPoint vp = view.mapFromScene(scenePos);
        const QPointF global = view.viewport()->mapToGlobal(vp);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(vp), global, button, button, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(vp), global, button, Qt::NoButton, Qt::NoModifier);
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
                                                            , QString(), QList<QPointF>(), QStringLiteral("internal row")));
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
    CHECK((self != nullptr) && self->isExternal() && (self->getTo() == onState->getName()));
    CHECK(dynamic_cast<SMEdgeItem*>(scene.findCanvasItem(selfTx)) != nullptr);
    model.getUndoStack().undo();

    // Internal transition (no target): the path the tool takes on Enter / empty drop.
    model.getUndoStack().push(new SMCreateTransitionCommand(  data, model.getNotifier(), *onState
                                                           , SMTransitionEntry::eStimulusKind::Trigger, QString(), QString()
                                                           , QList<QPointF>(), QStringLiteral("internal")));
    const uint32_t internalTx = onState->getTransitions().getElements().last()->getId();
    CHECK(data.findTransitionById(internalTx)->isExternal() == false);
    CHECK(scene.findCanvasItem(internalTx) == nullptr);                      // no edge for an internal
    model.getUndoStack().undo();

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
                                                       , SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("PowerOn")
                                                       , QStringLiteral("set stimulus")));
    CHECK(data.findTransitionById(createdTx)->getStimulus() == QStringLiteral("PowerOn"));

    std::printf("sect: SM-14 arc geometry\n");
    // --- Arc renders from two points + bulge (apex bows off the chord) ---
    const QList<QPointF> arc = NESMDesign::arcPolyline(QPointF(0.0, 0.0), QPointF(100.0, 0.0), 0.4, NESMDesign::EdgeArcSamples);
    CHECK(arc.size() == NESMDesign::EdgeArcSamples + 1);
    CHECK(std::abs(arc.at(arc.size() / 2).y()) > 1.0);

    std::printf("sect: SM-14 priority round-trip\n");
    // --- Priority reorder changes document order and survives save/load ---
    SMStateEntry* yellow = data.findState("Yellow");
    CHECK((yellow != nullptr) && (yellow->getTransitions().getElementCount() >= 2));
    const QString firstToBefore = yellow->getTransitions().getElements().at(0)->getTo();
    model.getUndoStack().push(new TDocReorderCommand<SMTransitionEntry*, DocumentElem>(  model.getNotifier(), yellow->getTransitions()
                                                                                       , 0, 1, yellow->getId(), eDocElementKind::Transition
                                                                                       , QStringLiteral("reorder")));
    const QString firstToAfter = yellow->getTransitions().getElements().at(0)->getTo();
    CHECK(firstToAfter != firstToBefore);
    const QString roundtripPath = QDir::tempPath() + QStringLiteral("/sm14_roundtrip.fsml");
    CHECK(model.saveToFile(roundtripPath));
    StateMachineModel reloaded;
    CHECK(reloaded.loadFromFile(roundtripPath));
    SMStateEntry* yellowReloaded = reloaded.getData().findState("Yellow");
    CHECK((yellowReloaded != nullptr) && (yellowReloaded->getTransitions().getElements().at(0)->getTo() == firstToAfter));

    std::printf("sect: SM-14 reconnect (target + source)\n");
    // --- Target-endpoint reconnection retargets `To`, undoable ---
    const QString to27Before = data.findTransitionById(27)->getTo();
    model.getUndoStack().push(new SMSetTransitionTargetCommand(  data, model.getNotifier(), 27
                                                              , QStringLiteral("LightOff"), QStringLiteral("retarget")));
    CHECK(data.findTransitionById(27)->getTo() == QStringLiteral("LightOff"));
    model.getUndoStack().undo();
    CHECK(data.findTransitionById(27)->getTo() == to27Before);
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
    design.getScene().clearSelection();
    design.getScene().stateItem(lightOff->getId())->setSelected(true);      // the Start state
    CHECK(design.actionAddSubstate()->isEnabled() == false);

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

        // Toolbar groups: Design first (state -> transition -> note -> final, the same
        // order as the canvas context menu), Declare second with icons on every entry.
        const QList<SMDesign::ToolGroup> groups = page.toolGroups();
        CHECK(groups.size() == 8);
        CHECK(groups.at(0).title == QStringLiteral("Design"));
        const QList<QAction*> designOrder{ page.actionAddState(), page.actionAddTransition()
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

            if ((firstTxId == 0) && (s->getTransitions().getElementCount() >= 1))
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
        CHECK(outline->getTree()->selectedItems().isEmpty() == false);

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

        // The stimulus picker is a fixed list of triggers/events/timers (each row carries its
        // real name at Qt::UserRole+1). Picking a listed entry sets it undoably; a value that is
        // not on the list is rejected - the panel never creates or renames a registry entry.
        doc.getSelectionModel().setSelection(QList<uint32_t>{ firstTxId });
        QApplication::processEvents();
        QComboBox* stim = props->stimulusNameCombo();

        const QString curStim = d.findTransitionById(firstTxId)->getStimulus();
        int pickRow = -1;
        for (int i = 0; i < stim->count(); ++i)
        {
            if (stim->itemData(i, Qt::UserRole + 1).toString() != curStim)
            {
                pickRow = i;
                break;
            }
        }

        if (pickRow >= 0)
        {
            const QString pickName = stim->itemData(pickRow, Qt::UserRole + 1).toString();
            const int indexBeforePick = doc.getUndoStack().index();  // index, not count: prior undo left a redoable
            stim->setEditText(stim->itemText(pickRow));
            QMetaObject::invokeMethod(stim->lineEdit(), "editingFinished");
            CHECK(d.findTransitionById(firstTxId)->getStimulus() == pickName);
            CHECK(doc.getUndoStack().index() == indexBeforePick + 1);
            doc.getUndoStack().undo();
            CHECK(d.findTransitionById(firstTxId)->getStimulus() == curStim);
        }

        // A name that is not on the list is rejected: no registry entry created, no change pushed.
        const QString notListed = QStringLiteral("NotAListedStimulus");
        CHECK(d.isStimulusName(notListed) == false);
        const int indexBeforeReject = doc.getUndoStack().index();
        const QString beforeReject = d.findTransitionById(firstTxId)->getStimulus();
        stim->setEditText(notListed);
        QMetaObject::invokeMethod(stim->lineEdit(), "editingFinished");
        CHECK(d.isStimulusName(notListed) == false);
        CHECK(d.findTransitionById(firstTxId)->getStimulus() == beforeReject);
        CHECK(doc.getUndoStack().index() == indexBeforeReject);

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
            box->setText(QStringLiteral("PowerOn"));
            QApplication::processEvents();
            SMStateEntry* offOwner = d.findState("LightOff");
            uint32_t powerOnId = 0;
            for (const SMTransitionEntry* t : offOwner->getTransitions().getElements())
            {
                if (t->getStimulus() == QStringLiteral("PowerOn")) { powerOnId = t->getId(); break; }
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

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
