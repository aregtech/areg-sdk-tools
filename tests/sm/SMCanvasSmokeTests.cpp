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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        tests/sm/SMCanvasSmokeTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-13 smoke tests: drives the design canvas offscreen with synthetic
 *               mouse/key events through the real SMDesign/SMScene/SMStateItem stack —
 *               state rendering, placement tool, inline rename validation, move,
 *               resize, collapse, and undo of each. Saves widget grabs as PNG when an
 *               output directory is passed as the second argument.
 *
 *  Usage: lusan_sm_canvas_tests <TrafficLight.fsml path> [grab output dir]
 *
 ************************************************************************/

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMAutoPlacer.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QMouseEvent>
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
    // The editor lives in a QGraphicsProxyWidget, not the widget hierarchy.
    QLineEdit* editor = nullptr;
    for (QGraphicsItem* item : scene.items())
    {
        QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
        if (proxy != nullptr)
        {
            editor = qobject_cast<QLineEdit*>(proxy->widget());
            if (editor != nullptr)
            {
                break;
            }
        }
    }

    CHECK(editor != nullptr);
    if (editor != nullptr)
    {
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
    // state's border-drag band — the edge handle must win over the border drag.
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
    QList<QToolButton*> crumbs = design.findChildren<QToolButton*>();
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
    CHECK(design.findChildren<QToolButton*>().size() == 2);                  // two clickable ancestors

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
    QList<QToolButton*> rootCrumbs = design.findChildren<QToolButton*>();
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
    CHECK(design.actionEnterSubmachine()->isEnabled() == false);             // not composite yet

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
    CHECK(data.getLevelPath(deepId).size() == 3);                            // root → Standby → DeepChild
    SMStateEntry* deepStart = deep->getNestedStates()->getStartState();
    CHECK((deepStart != nullptr) && (deepStart->getName() != standbyStart->getName()));
    CHECK(design.findChildren<QToolButton*>().size() == 2);
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

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
