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
 *  \file        tests/sm/SMPerformanceTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-27 responsiveness gate: builds a large synthetic document (hundreds of
 *               states across four levels), drives the real design page offscreen, and
 *               measures the operations spec 9.9 calls "frequent interactions".
 *
 *               A run is a debug build drawing into an offscreen raster surface, so an
 *               absolute millisecond figure says as much about the harness as about the
 *               editor. Every measure is therefore a comparison, and each one compares the
 *               figure against what it is actually made of:
 *
 *                 - a one-off document operation (load, save, first scene build, a full
 *                   validation sweep) against BUDGET_ONESHOT_MS, which is wide enough to
 *                   survive a busy machine;
 *                 - a gesture with no design page -- the command, the notifier fan-out, the
 *                   reference rewrite -- against BUDGET_INTERACTION_MS, over what the same
 *                   gesture costs on a ten-state document;
 *                 - the same gesture with the page drawing, against what one full redraw of
 *                   that page costs in the same run;
 *                 - zoom, pan and a full repaint, per item visible in the viewport.
 *
 *               Nothing is left unjudged: a measure that has nothing to compare against says
 *               so on its own line and is counted separately.
 *
 *  Usage: lusan_sm_perf [output dir]
 *
 ************************************************************************/

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMMethodKind.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMValidator.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QScrollBar>
#include <QUndoStack>

#include <cstdio>
#include <algorithm>
#include <functional>
#include <limits>

namespace
{
    //!< Spec 9.9: a frequent interaction feels immediate. Model-side interactions (selection,
    //!< commands, notifier fan-out) are compared against this directly.
    constexpr qint64 BUDGET_INTERACTION_MS  { 100 };
    //!< Opening, saving, first paint of a level and a full validation sweep happen once per
    //!< document or once per level, not per gesture.
    constexpr qint64 BUDGET_ONESHOT_MS      { 3000 };

    //!< Shape of the synthetic document: a deliberately oversized root level plus three
    //!< nested levels, so both "one big scene" and "deep hierarchy" are covered.
    constexpr int ROOT_STATES       { 200 };
    constexpr int COMPOSITE_COUNT   { 4   };
    constexpr int NESTED_STATES     { 50  };
    constexpr int DEEP_STATES       { 50  };
    //!< The small document the large one is compared against.
    constexpr int SMALL_STATES      { 10  };

    int gChecks = 0;
    int gFailures = 0;
    int gUnfit = 0;

    //!< A one-shot cost -- opening, saving, a validation sweep, the first paint of a level --
    //!< is judged against its own budget, which is wide enough to survive a busy machine.
    void report(const char* what, qint64 elapsed, qint64 budget)
    {
        ++gChecks;
        const bool ok = (elapsed <= budget);
        if (ok == false)
        {
            ++gFailures;
        }

        std::printf("  %-46s %6lld ms   (budget %lld ms) %s\n",
                    what, static_cast<long long>(elapsed), static_cast<long long>(budget),
                    ok ? "ok" : "OVER");
    }

    //!< The middle sample of a run, or zero when nothing was measured.
    qint64 medianOf(QList<qint64> samples)
    {
        if (samples.isEmpty())
        {
            return 0;
        }

        std::sort(samples.begin(), samples.end());
        return samples.at(samples.size() / 2);
    }

    //!< How many times a repeatable gesture is measured before it is judged.
    constexpr int GESTURE_RUNS { 5 };

    //!< One run of a gesture is dominated by scheduler noise on a debug build, and a gesture
    //!< sitting near its budget then passes or fails by luck. The gesture is measured several
    //!< times on each document and judged on the medians, and the spread is printed so a real
    //!< regression -- where the whole range moves -- is told apart from one slow sample.
    //!<
    //!< The same gesture on the ten-state document carries the fixed cost of the harness: a
    //!< debug Qt on the offscreen backend, one design page, one notifier fan-out. Subtracting
    //!< it leaves what the document size added, and that is what the interaction budget covers.
    //!< A busy machine slows both figures, so their difference stays readable.
    void reportGrowth(const char* what, QList<qint64> large, const QList<qint64>& small)
    {
        ++gChecks;
        if (large.isEmpty() || small.isEmpty())
        {
            ++gFailures;
            std::printf("  %-46s no samples\n", what);
            return;
        }

        std::sort(large.begin(), large.end());
        const qint64 median = large.at(large.size() / 2);
        const qint64 base   = medianOf(small);
        const qint64 growth = (median > base) ? (median - base) : 0;
        const bool ok = (growth <= BUDGET_INTERACTION_MS);
        if (ok == false)
        {
            ++gFailures;
        }

        std::printf("  %-46s %6lld ms median (small doc %lld ms, growth %lld of %lld ms,"
                    " %d runs %lld..%lld) %s\n",
                    what, static_cast<long long>(median), static_cast<long long>(base),
                    static_cast<long long>(growth), static_cast<long long>(BUDGET_INTERACTION_MS),
                    static_cast<int>(large.size()),
                    static_cast<long long>(large.first()), static_cast<long long>(large.last()),
                    ok ? "ok" : "OVER");
    }

    //!< Painting is judged per visible item, not per document. Two things differ between the
    //!< two documents: the fixed cost of the harness (a debug Qt on the offscreen raster
    //!< backend painting the whole design page with its docks), and how many items the viewport
    //!< happens to hold. Subtracting the small document's timing removes the first; dividing by
    //!< the visible-item count removes the second. What is left is what criterion 17 asks --
    //!< whether an item costs more to draw when the document is large. It must not.
    void reportPerItem(const char* what, qint64 large, int largeItems, qint64 small, int smallItems)
    {
        ++gChecks;
        const double largePer = (largeItems > 0) ? (static_cast<double>(large) / largeItems) : 0.0;
        const double smallPer = (smallItems > 0) ? (static_cast<double>(small) / smallItems) : 0.0;
        // A larger scene legitimately costs a little more per item (a deeper spatial index, more
        // culling); 2.5x is generous for that and still far below anything super-linear.
        constexpr double MAX_RATIO { 2.5 };
        // With nothing in the comparison viewport there is nothing to divide by, and the
        // measure would report a ratio of zero: a pass that measured nothing.
        const bool unfit = (smallItems <= 0) || (largeItems <= 0);
        const double ratio = (smallPer > 0.0) ? (largePer / smallPer) : 0.0;
        const bool ok = (ratio <= MAX_RATIO);
        if (unfit)
        {
            ++gUnfit;
        }
        else if (ok == false)
        {
            ++gFailures;
        }

        std::printf("  %-46s %6lld ms / %3d items = %5.2f ms/item   (small %5.2f, ratio %.2f of %.1f) %s\n",
                    what, static_cast<long long>(large), largeItems, largePer, smallPer, ratio, MAX_RATIO,
                    unfit ? "NOT JUDGED" : (ok ? "ok" : "OVER"));
        if (unfit)
        {
            std::printf("  %-46s   the comparison viewport held %d item(s): nothing to compare against\n",
                        "", smallItems);
        }
    }

    //!< How many full repaints of the same page a single gesture may cost.
    constexpr double MAX_REPAINTS_PER_GESTURE { 2.0 };

    //!< A gesture that ends in a repaint is judged against what one full repaint of the same
    //!< page costs in the same run. Both figures come from one document on one machine, so the
    //!< debug build and the offscreen backend cancel out and what is left is the question that
    //!< matters: does the gesture ask the page to draw more than a redraw would.
    void reportRepaints(const char* what, qint64 elapsed, qint64 fullRepaint)
    {
        ++gChecks;
        // With nothing to compare against the measure would report a share of zero: a pass
        // that measured nothing.
        const bool nothingToCompare = (fullRepaint <= 0);
        const double share = nothingToCompare ? 0.0 : (static_cast<double>(elapsed) / fullRepaint);
        const bool ok = (share <= MAX_REPAINTS_PER_GESTURE);
        if (nothingToCompare)
        {
            ++gUnfit;
        }
        else if (ok == false)
        {
            ++gFailures;
        }

        std::printf("  %-46s %6lld ms   (full repaint %lld ms, %.2f of %.1f repaints) %s\n",
                    what, static_cast<long long>(elapsed), static_cast<long long>(fullRepaint),
                    share, MAX_REPAINTS_PER_GESTURE,
                    nothingToCompare ? "NOT COMPARED" : (ok ? "ok" : "OVER"));
    }

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    //!< Fills a level with `count` states in a grid, each with a transition to its successor,
    //!< and gives every state a Node layout entry so nothing is auto-placed while measuring.
    void fillLevel(StateMachineData& data, SMStateData& level, const QString& prefix, int count)
    {
        QList<SMStateEntry*> created;
        created.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            const SMStateEntry::eStateKind kind = (i == 0) ? SMStateEntry::eStateKind::Start
                                                           : SMStateEntry::eStateKind::Normal;
            SMStateEntry* state = level.createState(prefix + QString::number(i), kind);
            if (state == nullptr)
            {
                continue;
            }

            created.append(state);
            SMLayoutNode& node = data.getLayout().addNode(state->getId());
            node.x      = 40.0 + (i % 20) * 220.0;
            node.y      = 40.0 + (i / 20) * 140.0;
            node.width  = 180.0;
            node.height = 80.0;
        }

        for (int i = 0; i + 1 < created.size(); ++i)
        {
            created.at(i)->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger,
                                                             QStringLiteral("Step"),
                                                             created.at(i + 1)->getId());
        }
    }

    //!< Builds the whole synthetic document: registries, a wide root level, four composites
    //!< with nested levels, and one level nested two deep.
    std::unique_ptr<StateMachineData> buildLargeDocument()
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(QStringLiteral("LargeMachine"));
        if (doc == nullptr)
        {
            return doc;
        }

        doc->getStates().removeAll();
        doc->getMethods().createMethod(QStringLiteral("Step"), NEMethod::SmTrigger);

        fillLevel(*doc, doc->getStates(), QStringLiteral("Root"), ROOT_STATES);

        const QList<SMStateEntry*> rootStates = doc->getStates().getElements();
        for (int c = 0; c < COMPOSITE_COUNT; ++c)
        {
            const int index = 1 + c * 7;
            if (index >= rootStates.size())
            {
                break;
            }

            SMStateEntry* composite = rootStates.at(index);
            SMStateData* nested = composite->getOrCreateNestedStates();
            if (nested == nullptr)
            {
                continue;
            }

            fillLevel(*doc, *nested, QStringLiteral("L2_") + QString::number(c) + QLatin1Char('_'), NESTED_STATES);

            if (c == 0)
            {
                SMStateEntry* deepOwner = nested->getElements().value(1, nullptr);
                SMStateData* deep = (deepOwner != nullptr) ? deepOwner->getOrCreateNestedStates() : nullptr;
                if (deep != nullptr)
                {
                    fillLevel(*doc, *deep, QStringLiteral("L3_"), DEEP_STATES);
                }
            }
        }

        return doc;
    }

    //!< Runs \p action several times and returns the fastest run. The first touch of a
    //!< viewport operation pays one-off setup (scroll range, scene rect, font cache) that says
    //!< nothing about how the editor behaves while the user works, and a shared build machine
    //!< adds outliers in both directions; the fastest run is the one that reflects the code.
    qint64 timeSteady(const std::function<void()>& action)
    {
        constexpr int RUNS { 4 };
        qint64 best = std::numeric_limits<qint64>::max();
        QElapsedTimer timer;
        for (int run = 0; run < RUNS; ++run)
        {
            timer.restart();
            action();
            QApplication::processEvents();
            best = std::min(best, timer.elapsed());
        }

        return best;
    }

    //!< The comparison document: the same construction, one small level.
    std::unique_ptr<StateMachineData> buildSmallDocument()
    {
        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument(QStringLiteral("SmallMachine"));
        if (doc != nullptr)
        {
            doc->getStates().removeAll();
            doc->getMethods().createMethod(QStringLiteral("Step"), NEMethod::SmTrigger);
            fillLevel(*doc, doc->getStates(), QStringLiteral("Root"), SMALL_STATES);

            // One composite, so the level-switch baseline is measured the same way as the
            // large document's -- otherwise the two figures are not comparable.
            SMStateEntry* composite = doc->getStates().getElements().value(1, nullptr);
            SMStateData* nested = (composite != nullptr) ? composite->getOrCreateNestedStates() : nullptr;
            if (nested != nullptr)
            {
                fillLevel(*doc, *nested, QStringLiteral("L2_"), SMALL_STATES);
            }
        }

        return doc;
    }

    //!< Puts the view in the state a breadcrumb navigation leaves it in: the root level,
    //!< viewport restored from the layout (or anchored on the content when there is none).
    void settleViewport(SMDesign& design)
    {
        SMSceneManager& manager = design.getSceneManager();
        const uint32_t rootLevel = manager.getRootLevel();
        for (const SMStateEntry* state : design.getModel().getData().getStates().getElements())
        {
            if ((state != nullptr) && state->hasNestedStates())
            {
                manager.navigateTo(state->getId());
                QApplication::processEvents();
                break;
            }
        }

        manager.navigateTo(rootLevel);
        QApplication::processEvents();

        // A level switch leaves the scroll bars where the nested level put them, which on a
        // small document is off its content entirely. Both documents are brought onto their
        // own items, so the two visible-item counts describe the same gesture.
        SMGraphicsView& view = design.getView();
        if (view.scene() != nullptr)
        {
            view.centerOn(view.scene()->itemsBoundingRect().center());
            QApplication::processEvents();
        }
    }

    //!< The gestures the user repeats, timed on one document. The large document and the
    //!< small one run the same routine, so a figure from one is comparable with the other.
    struct GestureTimings
    {
        qint64        select { 0 };  //!< Selecting a state, with the outline and properties following.
        QList<qint64> moves;         //!< Moving a state's box.
        QList<qint64> undos;         //!< Taking the move back.
        QList<qint64> redos;         //!< Putting the move again.
        QList<qint64> renames;       //!< Renaming a state, with the reference rewrite.
    };

    //!< The paint-bound timings of one document, in the order they are reported.
    struct PaintTimings
    {
        qint64 firstPaint { 0 };
        qint64 zoom       { 0 };
        qint64 pan        { 0 };
        qint64 repaint    { 0 };
        qint64 levelSwap  { 0 };
        int    visibleItems { 0 };
        GestureTimings gestures;
    };

    //!< Runs the repeatable gestures on a document whose design page is already built and
    //!< settled. The state at index 5 is a plain root-level state in both documents.
    GestureTimings measureGestures(StateMachineModel& model)
    {
        GestureTimings timings;
        StateMachineData& data = model.getData();
        SMStateEntry* target = data.getStates().getElements().value(5, nullptr);
        check(target != nullptr, "a root-level state to interact with");
        if (target == nullptr)
        {
            return timings;
        }

        const uint32_t id = target->getId();
        QElapsedTimer timer;

        timer.start();
        model.getSelectionModel().setSelection(QList<uint32_t>{ id });
        QApplication::processEvents();
        timings.select = timer.elapsed();

        const SMLayoutNode* node = data.getLayout().findNode(id);
        check(node != nullptr, "the state has a layout node");
        if (node != nullptr)
        {
            const SMLayoutNode before = *node;
            for (int run = 0; run < GESTURE_RUNS; ++run)
            {
                timer.restart();
                model.getUndoStack().push(new SMMoveNodeCommand(data, model.getNotifier(), id,
                                                                SMMoveNodeCommand::takeNextGesture(),
                                                                before.x + 320.0 + run, before.y + 180.0 + run,
                                                                before.width, before.height,
                                                                QStringLiteral("Move state")));
                QApplication::processEvents();
                timings.moves.append(timer.elapsed());

                timer.restart();
                model.getUndoStack().undo();
                QApplication::processEvents();
                timings.undos.append(timer.elapsed());

                timer.restart();
                model.getUndoStack().redo();
                QApplication::processEvents();
                timings.redos.append(timer.elapsed());
            }
        }

        for (int run = 0; run < GESTURE_RUNS; ++run)
        {
            timer.restart();
            model.getUndoStack().push(new SMRenameStateCommand(data, model.getNotifier(), id,
                                                               QStringLiteral("RenamedState%1").arg(run),
                                                               QStringLiteral("Rename state")));
            QApplication::processEvents();
            timings.renames.append(timer.elapsed());
        }

        return timings;
    }

    //!< Builds a design page over \p model and times the paint-bound operations on it.
    PaintTimings measurePaint(StateMachineModel& model)
    {
        PaintTimings timings;
        QElapsedTimer timer;

        timer.start();
        SMDesign design(model);
        design.resize(1600, 1000);
        design.show();
        QApplication::processEvents();
        timings.firstPaint = timer.elapsed();

        SMGraphicsView& view = design.getView();

        // Both documents are measured in the state a level switch leaves behind, so the
        // visible-item counts are comparable and neither depends on where the first paint
        // happened to leave the scroll bars.
        settleViewport(design);
        timings.visibleItems = static_cast<int>(view.items(view.viewport()->rect()).size());

        timings.zoom    = timeSteady([&view]() { view.scale(1.25, 1.25); });
        timings.pan     = timeSteady([&view]() {
                              view.horizontalScrollBar()->setValue(view.horizontalScrollBar()->value() + 900);
                          });
        timings.repaint = timeSteady([&design]() { design.grab(); });

        SMSceneManager& manager = design.getSceneManager();
        const uint32_t rootLevel = manager.getRootLevel();
        uint32_t nestedLevel = 0;
        for (const SMStateEntry* state : model.getData().getStates().getElements())
        {
            if ((state != nullptr) && state->hasNestedStates())
            {
                nestedLevel = state->getId();
                break;
            }
        }

        if (nestedLevel != 0)
        {
            manager.navigateTo(nestedLevel);
            QApplication::processEvents();
            manager.navigateTo(rootLevel);
            QApplication::processEvents();
            timings.levelSwap = timeSteady([&manager, nestedLevel, rootLevel]() {
                manager.navigateTo(nestedLevel);
                QApplication::processEvents();
                manager.navigateTo(rootLevel);
            });
        }

        // Measured while this document's page is alive, so the gesture figures of the two
        // documents come from the same code path.
        timings.gestures = measureGestures(model);
        return timings;
    }

    int countStates(const SMStateData& level)
    {
        int total = 0;
        for (const SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            ++total;
            if (state->hasNestedStates())
            {
                total += countStates(*state->getNestedStates());
            }
        }

        return total;
    }
}

int main(int argc, char* argv[])
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);

    QString outDir = (argc >= 2) ? QString::fromLocal8Bit(argv[1]) : QDir::tempPath();
    QDir().mkpath(outDir);
    const QString docPath = outDir + QDir::separator() + QStringLiteral("sm27_large.fsml");

    std::printf("SM-27 responsiveness gate (spec 9.9)\n");

    QElapsedTimer timer;

    // ---- document construction and persistence -------------------------------------------
    std::unique_ptr<StateMachineData> generated = buildLargeDocument();
    check(generated != nullptr, "synthetic document built");
    if (generated == nullptr)
    {
        return 1;
    }

    const int totalStates = countStates(generated->getStates());
    std::printf("  document: %d states over 4 levels, root level %d states\n",
                totalStates, generated->getStates().getElementCount());
    check(totalStates >= 400, "synthetic document has hundreds of states");

    timer.start();
    check(generated->writeToFile(docPath), "large document saved");
    report("save (whole document)", timer.elapsed(), BUDGET_ONESHOT_MS);
    generated.reset();

    StateMachineModel model;
    timer.restart();
    check(model.loadFromFile(docPath), "large document loaded");
    report("open (parse + build model)", timer.elapsed(), BUDGET_ONESHOT_MS);

    StateMachineData& data = model.getData();

    timer.restart();
    const QList<SMIssue> issues = SMValidator::validate(data);
    report("full validation sweep (headless)", timer.elapsed(), BUDGET_ONESHOT_MS);
    std::printf("  validation reported %d issue(s)\n", static_cast<int>(issues.size()));

    // ---- baseline: the same paint work on a ten-state document ---------------------------
    std::unique_ptr<StateMachineData> smallDoc = buildSmallDocument();
    check(smallDoc != nullptr, "comparison document built");
    const QString smallPath = outDir + QDir::separator() + QStringLiteral("sm27_small.fsml");
    check((smallDoc != nullptr) && smallDoc->writeToFile(smallPath), "comparison document saved");
    smallDoc.reset();

    // The comparison model and its page are torn down completely before the subject is built.
    // Leaving them alive costs the large document a multiple of every timing: two live
    // documents means two validation controllers, two autosave timers and two notifier fan-outs
    // pumped by every processEvents() in the measured section.
    PaintTimings small;
    GestureTimings smallModelSide;
    {
        StateMachineModel smallModel;
        check(smallModel.loadFromFile(smallPath), "comparison document loaded");
        // Once with no page, so the command, the notifier fan-out and the reference rewrite
        // are timed on their own, and once with the page, which adds the drawing.
        smallModelSide = measureGestures(smallModel);
        small = measurePaint(smallModel);
    }

    for (int drain = 0; drain < 3; ++drain)
    {
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QApplication::processEvents();
    }
    std::printf("  baseline (%d states + a %d-state sublevel): first paint %lld ms, zoom %lld ms,"
                " pan %lld ms, repaint %lld ms, level round trip %lld ms\n",
                SMALL_STATES, SMALL_STATES,
                static_cast<long long>(small.firstPaint), static_cast<long long>(small.zoom),
                static_cast<long long>(small.pan), static_cast<long long>(small.repaint),
                static_cast<long long>(small.levelSwap));
    std::printf("  the same gestures on those ten states: select %lld ms, move %lld ms,"
                " undo %lld ms, redo %lld ms, rename %lld ms\n",
                static_cast<long long>(small.gestures.select),
                static_cast<long long>(medianOf(small.gestures.moves)),
                static_cast<long long>(medianOf(small.gestures.undos)),
                static_cast<long long>(medianOf(small.gestures.redos)),
                static_cast<long long>(medianOf(small.gestures.renames)));

    // ---- what the gestures cost the model alone -------------------------------------------
    // No design page yet, so nothing here is drawing: this is the command, the notifier
    // fan-out and the reference rewrite over four hundred states.
    const GestureTimings modelSide = measureGestures(model);
    reportGrowth("select a state (model side)"   , QList<qint64>{ modelSide.select }, QList<qint64>{ smallModelSide.select });
    reportGrowth("move a state (model side)"     , modelSide.moves  , smallModelSide.moves);
    reportGrowth("undo the move (model side)"    , modelSide.undos  , smallModelSide.undos);
    reportGrowth("redo the move (model side)"    , modelSide.redos  , smallModelSide.redos);
    reportGrowth("rename a state (model side)"   , modelSide.renames, smallModelSide.renames);

    // ---- design page over the large document ----------------------------------------------
    timer.restart();
    SMDesign design(model);
    const qint64 buildMs = timer.elapsed();
    design.resize(1600, 1000);
    const qint64 resizeMs = timer.elapsed() - buildMs;
    design.show();
    const qint64 showMs = timer.elapsed() - buildMs - resizeMs;
    QApplication::processEvents();
    report("first paint of the 200-node root level", timer.elapsed(), BUDGET_ONESHOT_MS);
    std::printf("    of which: construct %lld ms, resize %lld ms, show %lld ms, events %lld ms\n",
                static_cast<long long>(buildMs), static_cast<long long>(resizeMs),
                static_cast<long long>(showMs),
                static_cast<long long>(timer.elapsed() - buildMs - resizeMs - showMs));

    SMScene& scene = design.getScene();
    SMGraphicsView& view = design.getView();
    SMSceneManager& manager = design.getSceneManager();
    const uint32_t rootLevel = manager.getRootLevel();

    // Measured in the same settled state as the baseline, so the two visible-item counts
    // describe the same gesture on documents of different size.
    settleViewport(design);
    const int visibleItems = static_cast<int>(view.items(view.viewport()->rect()).size());
    const qint64 fullRepaint = timeSteady([&design]() { design.grab(); });
    std::printf("  the settled viewport holds %d of the level's %d items (baseline: %d),"
                " and drawing them costs %lld ms\n",
                visibleItems, static_cast<int>(scene.items().size()), small.visibleItems,
                static_cast<long long>(fullRepaint));

    // ---- interactions -----------------------------------------------------------------------
    SMStateEntry* composite = nullptr;
    for (SMStateEntry* state : data.getStates().getElements())
    {
        if ((state != nullptr) && state->hasNestedStates())
        {
            composite = state;
            break;
        }
    }

    // The same gestures again, this time with the page drawing. What they add over the model
    // side above is the drawing, so each one is judged against what a full redraw costs.
    const GestureTimings gestures = measureGestures(model);
    reportRepaints("select a state (outline + properties follow)", gestures.select          , fullRepaint);
    reportRepaints("move a state (command + notify + repaint)"   , medianOf(gestures.moves)  , fullRepaint);
    reportRepaints("undo the move"                               , medianOf(gestures.undos)  , fullRepaint);
    reportRepaints("redo the move"                               , medianOf(gestures.redos)  , fullRepaint);
    reportRepaints("rename a state (reference rewrite)"          , medianOf(gestures.renames), fullRepaint);

    check(composite != nullptr, "a composite state exists to navigate into");
    if (composite != nullptr)
    {
        timer.restart();
        check(manager.navigateTo(composite->getId()), "navigated into the nested level");
        QApplication::processEvents();
        report("enter submachine (cold scene build)", timer.elapsed(), BUDGET_ONESHOT_MS);

        check(manager.navigateTo(rootLevel), "navigated back to the root level");
        QApplication::processEvents();

        // Both scenes are built now, so this is the steady-state cost of swapping the view
        // between a 200-node level and a 50-node one -- the gesture the breadcrumb performs.
        const uint32_t nestedLevel = composite->getId();
        // Timed as a round trip (down and back) for a stable reading, reported per switch --
        // one switch is what the user performs with the breadcrumb.
        const qint64 roundTrip = timeSteady([&manager, nestedLevel, rootLevel]() {
                                     manager.navigateTo(nestedLevel);
                                     QApplication::processEvents();
                                     manager.navigateTo(rootLevel);
                                 });
        reportRepaints("switch level (cached scenes, per switch)", roundTrip / 2, fullRepaint);
    }

    reportPerItem("zoom the 200-node level",
                  timeSteady([&view]() { view.scale(1.25, 1.25); }), visibleItems,
                  small.zoom, small.visibleItems);
    reportPerItem("pan the 200-node level",
                  timeSteady([&view]() {
                      view.horizontalScrollBar()->setValue(view.horizontalScrollBar()->value() + 900);
                  }), visibleItems, small.pan, small.visibleItems);
    reportPerItem("full repaint of the visible viewport",
                  fullRepaint, visibleItems, small.repaint, small.visibleItems);

    check(scene.items().isEmpty() == false, "the root scene holds items");

    if (gUnfit != 0)
    {
        std::printf("  %d measure(s) had nothing to compare against; each line says why.\n", gUnfit);
    }

    std::printf("---- %d checks, %d failure(s), %d not compared ----\n", gChecks, gFailures, gUnfit);
    return (gFailures == 0) ? 0 : 1;
}
