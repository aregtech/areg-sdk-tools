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
 *  \file        tests/sm/SML8ProofTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The canvas draws the graph the document contains, and a transition into a Start
 *               pseudo-state cannot be drawn at all.
 *
 *               Three things are proved, one per section:
 *
 *                 1. Edge parity. On every machine level of the document, the set of drawn edges
 *                    equals the set of transitions that carry a target -- one for one, by element
 *                    ID -- and each edge's endpoints are the boxes of the owning and target states.
 *                    Most of the transitions in the acceptance document carry no persisted route,
 *                    so this is the auto-routed case: the one the graph was misread from.
 *                 2. No transition may enter a Start. The drawing gesture is refused, on the same
 *                    state as source and on any other, and nothing reaches the undo stack.
 *                 3. A document that already contains one -- only a hand-edited or imported file
 *                    can -- is reported as an error rather than silently drawn.
 *
 *               Run as: lusan_sm_l8_proof <FullFeature.fsml> [screenshot directory]
 *
 ************************************************************************/

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/sm/SMValidator.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QApplication>
#include <QDir>
#include <QMouseEvent>
#include <QPixmap>

#include <algorithm>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int     gChecks   { 0 };
    int     gFailures { 0 };
    QString gGrabDir;

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
    using eKind = SMStateEntry::eStateKind;

    void grab(QWidget& widget, const char* name)
    {
        if (gGrabDir.isEmpty() == false)
        {
            widget.grab().save(gGrabDir + QDir::separator() + QString::fromLatin1(name) + QStringLiteral(".png"));
        }
    }

    //!< Posts a full mouse press/release pair to the view's viewport at a scene position.
    void clickScene(SMGraphicsView& view, const QPointF& scenePos)
    {
        const QPoint vp = view.mapFromScene(scenePos);
        const QPointF global = view.viewport()->mapToGlobal(vp);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(vp), global, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(vp), global, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(view.viewport(), &release);
        QApplication::processEvents();
    }

    //!< Every edge item currently on \p scene, in element-ID order.
    QList<SMEdgeItem*> drawnEdges(SMScene& scene)
    {
        QList<SMEdgeItem*> edges;
        for (QGraphicsItem* item : scene.items())
        {
            if (SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(item))
            {
                edges.append(edge);
            }
        }

        std::sort(edges.begin(), edges.end()
                 , [](const SMEdgeItem* left, const SMEdgeItem* right) { return left->getElementId() < right->getElementId(); });
        return edges;
    }

    //!< True when \p point sits on the border of \p box, within a tolerance that absorbs the
    //!< corner-radius projection and the grid snap.
    bool touchesBox(const QRectF& box, const QPointF& point)
    {
        constexpr double SLACK = 4.0;
        return box.adjusted(-SLACK, -SLACK, SLACK, SLACK).contains(point);
    }

    /**
     * \brief   Compares the edges drawn on the current level against the document's own transition
     *          lists. Returns the number of edges audited.
     **/
    int auditLevel(SMDesign& design, const char* label)
    {
        SMScene&          scene = design.getScene();
        StateMachineData& data  = design.getModel().getData();

        const SMStateData* level = data.findLevel(scene.getLevelId());
        CHECK(level != nullptr);
        if (level == nullptr)
        {
            return 0;
        }

        // What the file says: every transition of this level's states that carries a target.
        QList<uint32_t> fromDocument;
        for (const SMStateEntry* state : level->getElements())
        {
            for (const SMTransitionEntry* tr : state->getTransitions().getElements())
            {
                if ((tr != nullptr) && tr->hasTarget())
                {
                    fromDocument.append(tr->getId());
                }
            }
        }
        std::sort(fromDocument.begin(), fromDocument.end());

        // What the canvas draws.
        const QList<SMEdgeItem*> edges = drawnEdges(scene);
        QList<uint32_t> fromCanvas;
        for (const SMEdgeItem* edge : edges)
        {
            fromCanvas.append(edge->getElementId());
        }
        std::sort(fromCanvas.begin(), fromCanvas.end());

        std::printf("    %-14s document: %d transitions with a target, canvas: %d edges\n"
                   , label, static_cast<int>(fromDocument.size()), static_cast<int>(fromCanvas.size()));

        // One for one. An extra edge is a graph the file does not contain; a missing one is a
        // transition the author cannot see.
        CHECK(fromCanvas == fromDocument);

        for (const SMEdgeItem* edge : edges)
        {
            const uint32_t id = edge->getElementId();
            const SMTransitionEntry* tr = data.findTransitionById(id);
            const SMStateEntry* owner = data.findTransitionOwner(id);
            CHECK((tr != nullptr) && (owner != nullptr));
            if ((tr == nullptr) || (owner == nullptr))
            {
                continue;
            }

            // The two ends the edge believes in are the two the document names.
            CHECK(edge->getSourceId() == owner->getId());
            CHECK(edge->getTargetId() == tr->getToId());

            // ...and the drawn polyline actually runs between those two boxes. This is the check
            // that separates "the model is right" from "the picture is right": an auto-routed edge
            // computes its own endpoints, and only the boxes can say whether it computed them for
            // the states the file names.
            SMStateItem* sourceItem = design.getScene().stateItem(owner->getId());
            SMStateItem* targetItem = design.getScene().stateItem(tr->getToId());
            const QList<QPointF>& path = edge->getPath();
            CHECK((sourceItem != nullptr) && (targetItem != nullptr) && (path.size() >= 2));
            if ((sourceItem == nullptr) || (targetItem == nullptr) || (path.size() < 2))
            {
                continue;
            }

            CHECK(touchesBox(sourceItem->getBoxGeometry(), path.first()));
            CHECK(touchesBox(targetItem->getBoxGeometry(), path.last()));

            const bool selfLoop = (edge->getSourceId() == edge->getTargetId());
            CHECK(selfLoop == (owner->getId() == tr->getToId()));
        }

        return static_cast<int>(edges.size());
    }
}

//////////////////////////////////////////////////////////////////////////
// Section 1 -- every drawn edge is a transition of the document
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testEdgeParity(const QString& path)
    {
        std::printf("- edge parity: the canvas draws the TransitionList and nothing else\n");

        StateMachineModel model;
        CHECK(model.loadFromFile(path));

        SMDesign design(model);
        design.resize(1400, 900);
        design.show();
        QApplication::processEvents();

        StateMachineData& data = model.getData();
        int audited = auditLevel(design, "root");
        grab(design, "l8-root");

        // Every painted sublevel, entered the way the user enters it.
        for (const SMStateEntry* state : data.findLevel(design.getScene().getLevelId())->getElements())
        {
            if ((state == nullptr) || (state->hasNestedStates() == false))
                continue;

            const uint32_t compositeId = state->getId();
            design.getSceneManager().enterSubmachine(compositeId);
            QApplication::processEvents();
            audited += auditLevel(design, state->getName().toLatin1().constData());
            grab(design, "l8-nested");
            design.getSceneManager().navigateTo(design.getSceneManager().getRootLevel());
            QApplication::processEvents();
        }

        // The two faults the document was reported to have, checked directly against the picture.
        // Neither is in the file, and after the audit above neither can be on the canvas either.
        int selfLoops = 0;
        int intoMaintenance = 0;
        const SMStateEntry* maintenance = data.findState(QStringLiteral("Maintenance"));
        for (SMEdgeItem* edge : drawnEdges(design.getScene()))
        {
            if (edge->getSourceId() == edge->getTargetId())
                ++selfLoops;
            if ((maintenance != nullptr) && (edge->getTargetId() == maintenance->getId()))
                ++intoMaintenance;
        }

        CHECK(selfLoops == 0);                      // no Start self-loop, and no other self-loop
        CHECK(maintenance != nullptr);
        CHECK(intoMaintenance == 1);                // Maintenance IS entered: transition 48

        std::printf("    %d edges audited, %d self-loops, %d edges into Maintenance\n"
                   , audited, selfLoops, intoMaintenance);
    }
}

//////////////////////////////////////////////////////////////////////////
// Section 2 -- the gesture refuses to draw into a Start
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testStartCannotBeTargeted(const QString& path)
    {
        std::printf("- the drawing gesture refuses a Start as target\n");

        StateMachineModel model;
        CHECK(model.loadFromFile(path));

        SMDesign design(model);
        design.resize(1400, 900);
        design.show();
        QApplication::processEvents();

        SMScene&          scene = design.getScene();
        SMGraphicsView&   view  = design.getView();
        StateMachineData& data  = model.getData();

        const SMStateData* level = data.findLevel(scene.getLevelId());
        SMStateEntry* start = (level != nullptr ? level->getStartState() : nullptr);
        CHECK(start != nullptr);
        if (start == nullptr)
        {
            return;
        }

        SMStateItem* startItem = scene.stateItem(start->getId());
        CHECK(startItem != nullptr);
        if (startItem == nullptr)
        {
            return;
        }

        view.centerOn(startItem);
        QApplication::processEvents();

        // Dragging the Start's own arrow back onto the Start: refused at the gesture, so the level
        // can never be left uninitialised by something the designer drew.
        const int transitionsBefore = start->getTransitions().getElementCount();
        const int undoBefore = model.getUndoStack().count();
        scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
        clickScene(view, startItem->getBoxGeometry().center());
        clickScene(view, startItem->getBoxGeometry().center());
        CHECK(start->getTransitions().getElementCount() == transitionsBefore);
        CHECK(model.getUndoStack().count() == undoBefore);
        grab(design, "l8-start-refused");
        scene.setActiveTool(NESMDesign::eCanvasTool::Select);

        // ...and so is any other state's edge aimed at it: a Start is a marker the machine passes
        // through once, so nothing at all may enter one.
        SMStateEntry* other = nullptr;
        for (SMStateEntry* candidate : level->getElements())
        {
            if ((candidate != nullptr) && (candidate->getKind() == eKind::Normal))
            {
                other = candidate;
                break;
            }
        }

        CHECK(other != nullptr);
        if (other != nullptr)
        {
            SMStateItem* otherItem = scene.stateItem(other->getId());
            CHECK(otherItem != nullptr);
            if (otherItem != nullptr)
            {
                const int otherBefore = other->getTransitions().getElementCount();
                const int undoNow = model.getUndoStack().count();
                scene.setActiveTool(NESMDesign::eCanvasTool::AddTransition);
                clickScene(view, otherItem->getBoxGeometry().center());
                clickScene(view, startItem->getBoxGeometry().center());
                CHECK(other->getTransitions().getElementCount() == otherBefore);
                CHECK(model.getUndoStack().count() == undoNow);
                scene.setActiveTool(NESMDesign::eCanvasTool::Select);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Section 3 -- a document that already carries one is reported
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testImportedSelfLoopIsReported()
    {
        std::printf("- a Start self-loop already in a document is an error\n");

        StateMachineData doc;
        SMStateEntry* start = doc.getStates().createState(QStringLiteral("Begin"), eKind::Start);
        doc.getStates().createState(QStringLiteral("Work"), eKind::Normal);
        start->getTransitions().createTransition(  SMTransitionEntry::eStimulusKind::Trigger, QString()
                                                 , start->getId(), SMTransitionEntry::eTransitionKind::Initial);

        const QList<SMIssue> issues = SMValidator::validate(doc);
        bool reported = false;
        for (const SMIssue& issue : issues)
        {
            reported = reported || ((issue.rule == SMValidator::RULE_PSEUDO_START)
                                    && (issue.severity == SMIssue::eSeverity::Error)
                                    && issue.message.contains(QStringLiteral("never initialises")));
        }

        CHECK(reported);
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
        std::printf("Usage: %s <FullFeature.fsml> [screenshot directory]\n", argv[0]);
        return 2;
    }

    const QString path = QString::fromLocal8Bit(argv[1]);
    if (argc >= 3)
    {
        gGrabDir = QString::fromLocal8Bit(argv[2]);
        QDir().mkpath(gGrabDir);
    }

    std::printf("=== L8: reachability and unrouted edges ===\n");

    testEdgeParity(path);
    testStartCannotBeTargeted(path);
    testImportedSelfLoopIsReported();

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
