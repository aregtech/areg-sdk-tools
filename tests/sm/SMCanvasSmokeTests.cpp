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
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QApplication>
#include <QDir>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QMouseEvent>
#include <QUndoStack>

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
    const SMLayoutNode* offNode = data.getLayout().findNode(lightOff->getId());
    CHECK((offNode != nullptr));
    const int undoBeforeCollapse = model.getUndoStack().count();
    const QRectF offBox{ offItem->pos(), QSizeF(offNode->width, offNode->height) };
    // The chevron sits 18..6 px left of the box's right edge in the header band.
    clickScene(view, QPointF(offBox.right() - 12.0, offBox.top() + 12.0));
    const SMLayoutNode* collapsed = data.getLayout().findNode(lightOff->getId());
    CHECK((collapsed != nullptr) && collapsed->hasExpanded && (collapsed->expanded == false));
    CHECK(model.getUndoStack().count() == undoBeforeCollapse + 1);
    grab(design, "g5-collapsed");
    model.getUndoStack().undo();
    const SMLayoutNode* expanded = data.getLayout().findNode(lightOff->getId());
    CHECK((expanded != nullptr) && (expanded->expanded || (expanded->hasExpanded == false)));

    std::printf("sect: selection\n");
    // --- Selection sync ---
    scene.clearSelection();
    QApplication::processEvents();
    clickScene(view, offItem->pos() + QPointF(20.0, 10.0));
    CHECK(model.getSelectionModel().isSelected(lightOff->getId()));
    grab(design, "g6-selected");

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
