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
 *  \file        lusan/view/sm/SMDesign.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor Design page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDesign.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMCanvasItem.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMStateItem.hpp"

#include <QAction>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPair>
#include <QStringList>
#include <QVBoxLayout>

namespace
{
    //!< Synthetic stress-test element IDs start here, far above any document ID.
    constexpr uint32_t StressIdBase{ 0x40000000u };

    /**
     * \brief   A plain synthetic node used only by the stress populate: a rounded,
     *          movable, selectable box exercising selection, snapping, and repaints.
     **/
    class StressNodeItem : public SMCanvasItem
    {
    public:
        StressNodeItem(uint32_t elementId, const QString& name)
            : SMCanvasItem  (elementId)
            , mName         (name)
        {
            setFlag(QGraphicsItem::ItemIsSelectable, true);
            setFlag(QGraphicsItem::ItemIsMovable, true);
            setFlag(QGraphicsItem::ItemIsFocusable, true);
        }

        QRectF boundingRect() const override
        {
            return QRectF(-2.0, -2.0, 124.0, 64.0);
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* widget) override
        {
            const QPalette palette{ (widget != nullptr) ? widget->palette() : QPalette() };
            const QRectF   body{ 0.0, 0.0, 120.0, 60.0 };

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(QPen(palette.color(QPalette::WindowText), 1.0));
            painter->setBrush(palette.color(QPalette::Window));
            painter->drawRoundedRect(body, 6.0, 6.0);
            painter->setPen(palette.color(QPalette::WindowText));
            painter->drawText(body, Qt::AlignCenter, mName);

            if (isSelected())
            {
                NESMDesign::paintSelectionFrame(painter, body.adjusted(-2.0, -2.0, 2.0, 2.0), palette, hasFocus());
            }
        }

    private:
        QString mName;  //!< The displayed node name.
    };
}

SMDesign::SMDesign(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mView         (new SMGraphicsView(this))
    , mScene        (nullptr)
    , mActZoomIn    (nullptr)
    , mActZoomOut   (nullptr)
    , mActZoomReset (nullptr)
    , mActZoomFit   (nullptr)
    , mActToggleGrid(nullptr)
    , mActToggleSnap(nullptr)
    , mActSelectAll (nullptr)
    , mActAddState  (nullptr)
    , mActAddFinal  (nullptr)
    , mActAddTransition(nullptr)
    , mActDelete    (nullptr)
    , mActRename    (nullptr)
    , mActSetStimulus(nullptr)
    , mActRaisePriority(nullptr)
    , mActLowerPriority(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mView);

    rebuildScene();
    setupActions();
    mView->installEventFilter(this);

    // Minimal canvas context menu over the same action set; the full toolbar and
    // Design menu surfaces replace nothing here, they reuse these actions.
    mView->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* separator1 = new QAction(this);
    separator1->setSeparator(true);
    QAction* separator2 = new QAction(this);
    separator2->setSeparator(true);
    mView->addActions({ mActAddState, mActAddFinal, mActAddTransition, separator1
                      , mActSetStimulus, mActRaisePriority, mActLowerPriority, separator2
                      , mActRename, mActDelete });

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMDesign::onDocumentReloaded);
}

bool SMDesign::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == mView)
    {
        if (event->type() == QEvent::ShortcutOverride)
        {
            if (matchAction(*static_cast<QKeyEvent*>(event)) != nullptr)
            {
                event->accept();
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QAction* action = matchAction(*static_cast<QKeyEvent*>(event));
            if (action != nullptr)
            {
                action->trigger();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

QAction* SMDesign::matchAction(const QKeyEvent& event) const
{
    if (event.key() == 0 || event.key() == Qt::Key_unknown)
    {
        return nullptr;
    }

    QList<QKeySequence> pressed;
    pressed.append(QKeySequence(event.keyCombination()));

    // Shift+digit arrives as the shifted symbol (e.g. ')'); also match the digit itself.
    const quint32 nativeKey = event.nativeVirtualKey();
    if ((nativeKey >= '0') && (nativeKey <= '9'))
    {
        const Qt::Key digit = static_cast<Qt::Key>(Qt::Key_0 + static_cast<int>(nativeKey - '0'));
        pressed.append(QKeySequence(QKeyCombination(event.modifiers(), digit)));
    }

    for (QAction* action : actions())
    {
        for (const QKeySequence& shortcut : action->shortcuts())
        {
            if (pressed.contains(shortcut))
            {
                return action;
            }
        }
    }

    return nullptr;
}

void SMDesign::onDocumentReloaded()
{
    rebuildScene();
}

void SMDesign::setupActions()
{
    mActZoomIn = new QAction(tr("Zoom In"), this);
    mActZoomIn->setShortcuts({ QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal) });
    connect(mActZoomIn, &QAction::triggered, mView, &SMGraphicsView::zoomIn);

    mActZoomOut = new QAction(tr("Zoom Out"), this);
    mActZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(mActZoomOut, &QAction::triggered, mView, &SMGraphicsView::zoomOut);

    mActZoomReset = new QAction(tr("Zoom 100%"), this);
    mActZoomReset->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(mActZoomReset, &QAction::triggered, mView, &SMGraphicsView::zoomReset);

    mActZoomFit = new QAction(tr("Zoom to Fit"), this);
    mActZoomFit->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_0));
    connect(mActZoomFit, &QAction::triggered, mView, &SMGraphicsView::zoomToFit);

    mActToggleGrid = new QAction(tr("Show Grid"), this);
    mActToggleGrid->setCheckable(true);
    connect(mActToggleGrid, &QAction::toggled, this, [this](bool checked) {
        getScene().setGridVisible(checked);
    });

    mActToggleSnap = new QAction(tr("Snap to Grid"), this);
    mActToggleSnap->setCheckable(true);
    mActToggleSnap->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
    connect(mActToggleSnap, &QAction::toggled, this, [this](bool checked) {
        getScene().setSnapToGrid(checked);
    });

    mActSelectAll = new QAction(tr("Select All"), this);
    mActSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(mActSelectAll, &QAction::triggered, this, [this]() {
        getScene().selectAll();
    });

    mActAddState = new QAction(tr("Add State"), this);
    connect(mActAddState, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddState);
    });

    mActAddFinal = new QAction(tr("Add Final State"), this);
    connect(mActAddFinal, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddFinalState);
    });

    mActAddTransition = new QAction(tr("Add Transition"), this);
    connect(mActAddTransition, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddTransition);
    });

    mActSetStimulus = new QAction(tr("Set Stimulus…"), this);
    connect(mActSetStimulus, &QAction::triggered, this, &SMDesign::setStimulusOfSelection);

    mActRaisePriority = new QAction(tr("Raise Priority"), this);
    connect(mActRaisePriority, &QAction::triggered, this, [this]() { reorderSelectedTransition(true); });

    mActLowerPriority = new QAction(tr("Lower Priority"), this);
    connect(mActLowerPriority, &QAction::triggered, this, [this]() { reorderSelectedTransition(false); });

    mActDelete = new QAction(tr("Delete"), this);
    mActDelete->setShortcut(QKeySequence::Delete);
    connect(mActDelete, &QAction::triggered, this, &SMDesign::deleteSelection);

    mActRename = new QAction(tr("Rename"), this);
    mActRename->setShortcut(QKeySequence(Qt::Key_F2));
    connect(mActRename, &QAction::triggered, this, [this]() {
        getScene().startRenameOfSelection();
    });

    const QList<QAction*> actions{ mActZoomIn, mActZoomOut, mActZoomReset, mActZoomFit
                                 , mActToggleGrid, mActToggleSnap, mActSelectAll
                                 , mActAddState, mActAddFinal, mActAddTransition, mActDelete, mActRename
                                 , mActSetStimulus, mActRaisePriority, mActLowerPriority };
    for (QAction* action : actions)
    {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(action);
    }

    mActToggleGrid->setChecked(getScene().isGridVisible());
    mActToggleSnap->setChecked(getScene().isSnapToGrid());
}

void SMDesign::deleteSelection()
{
    SMScene& scene = getScene();
    const QList<SMStateItem*> selection{ scene.selectedStateItems() };
    if (selection.isEmpty())
    {
        deleteSelectedEdges();
        return;
    }

    StateMachineModel& model = mModel;
    StateMachineData&  data  = model.getData();
    SMStateData* level = data.findLevel(scene.getLevelId());
    if (level == nullptr)
    {
        return;
    }

    // The Start state is auto-created with its level and cannot be recreated by a tool.
    QStringList names;
    QList<uint32_t> deletable;
    bool skippedStart{ false };
    int  substates{ 0 };
    for (const SMStateItem* item : selection)
    {
        const SMStateEntry* state = data.findStateById(item->getElementId());
        if (state == nullptr)
        {
            continue;
        }

        if (state->getKind() == SMStateEntry::eStateKind::Start)
        {
            skippedStart = true;
            continue;
        }

        names.append(state->getName());
        deletable.append(state->getId());
        if (state->hasNestedStates())
        {
            substates += state->getNestedStates()->countStatesRecursive();
        }
    }

    if (deletable.isEmpty())
    {
        QMessageBox::information(this, tr("Delete States")
                                 , tr("The Start state cannot be deleted — every machine level needs exactly one."));
        return;
    }

    QString message = (deletable.size() == 1)
            ? tr("Delete state '%1'?").arg(names.first())
            : tr("Delete %1 states (%2)?").arg(deletable.size()).arg(names.join(QStringLiteral(", ")));
    if (substates > 0)
    {
        message += QStringLiteral("\n") + tr("%1 painted substate(s) are deleted with it.").arg(substates);
    }

    message += QStringLiteral("\n") + tr("Transitions from and to the deleted state(s) are deleted too.");
    if (skippedStart)
    {
        message += QStringLiteral("\n") + tr("The selected Start state is kept — every level needs one.");
    }

    const QMessageBox::StandardButton answer =
            QMessageBox::question(this, tr("Delete States"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
    {
        return;
    }

    const QString text = (deletable.size() == 1)
            ? tr("Delete state %1").arg(names.first())
            : tr("Delete %1 states").arg(deletable.size());
    if (deletable.size() == 1)
    {
        model.getUndoStack().push(new SMRemoveStateCommand(data, model.getNotifier(), *level, deletable.first(), text));
    }
    else
    {
        SMCompositeCommand* composite = new SMCompositeCommand(data, model.getNotifier(), text);
        for (uint32_t stateId : deletable)
        {
            new SMRemoveStateCommand(data, model.getNotifier(), *level, stateId, text, composite);
        }

        model.getUndoStack().push(composite);
    }
}

void SMDesign::deleteSelectedEdges()
{
    SMScene& scene = getScene();
    const QList<SMEdgeItem*> edges{ scene.selectedEdgeItems() };
    if (edges.isEmpty())
    {
        return;
    }

    StateMachineData& data = mModel.getData();

    // Pair each edge's transition with its owning state.
    QList<QPair<SMStateEntry*, uint32_t>> targets;
    for (const SMEdgeItem* edge : edges)
    {
        SMStateEntry* owner = data.findTransitionOwner(edge->getElementId());
        if (owner != nullptr)
        {
            targets.append(qMakePair(owner, edge->getElementId()));
        }
    }

    if (targets.isEmpty())
    {
        return;
    }

    const QString question = (targets.size() == 1)
            ? tr("Delete the selected transition?")
            : tr("Delete %1 selected transitions?").arg(targets.size());
    if (QMessageBox::question(this, tr("Delete Transitions"), question
                              , QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    const QString text = (targets.size() == 1) ? tr("Delete transition") : tr("Delete %1 transitions").arg(targets.size());
    if (targets.size() == 1)
    {
        mModel.getUndoStack().push(new SMRemoveTransitionCommand(data, mModel.getNotifier(), *targets.first().first, targets.first().second, text));
    }
    else
    {
        SMCompositeCommand* composite = new SMCompositeCommand(data, mModel.getNotifier(), text);
        for (const QPair<SMStateEntry*, uint32_t>& target : targets)
        {
            new SMRemoveTransitionCommand(data, mModel.getNotifier(), *target.first, target.second, text, composite);
        }

        mModel.getUndoStack().push(composite);
    }
}

void SMDesign::reorderSelectedTransition(bool raise)
{
    const QList<SMEdgeItem*> edges{ getScene().selectedEdgeItems() };
    if (edges.size() != 1)
    {
        return;
    }

    const uint32_t transitionId = edges.first()->getElementId();
    StateMachineData& data = mModel.getData();
    SMStateEntry* owner = data.findTransitionOwner(transitionId);
    if (owner == nullptr)
    {
        return;
    }

    SMTransitionData& list = owner->getTransitions();
    const int index = list.findIndex(transitionId);
    const int other = (raise ? index - 1 : index + 1);
    if ((index < 0) || (other < 0) || (other >= list.getElementCount()))
    {
        return;
    }

    // The swap keeps IDs position-keyed; the moved content ends up under the other slot's
    // ID, so re-select that ID to keep the selection on the transition the user moved.
    const uint32_t followId = list.getElements().at(other)->getId();
    const QString text = raise ? tr("Raise transition priority") : tr("Lower transition priority");
    mModel.getUndoStack().push(new TDocReorderCommand<SMTransitionEntry*, DocumentElem>(  mModel.getNotifier(), list
                                                                                        , index, other, owner->getId()
                                                                                        , eDocElementKind::Transition, text));
    mModel.getSelectionModel().setSelection(QList<uint32_t>{ followId });
}

void SMDesign::setStimulusOfSelection()
{
    const QList<SMEdgeItem*> edges{ getScene().selectedEdgeItems() };
    if (edges.size() != 1)
    {
        return;
    }

    const uint32_t transitionId = edges.first()->getElementId();
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return;
    }

    // Gather the shared stimulus name space: triggers, events, timers.
    QStringList labels;
    QList<QPair<SMTransitionEntry::eStimulusKind, QString>> options;
    const auto add = [&labels, &options](SMTransitionEntry::eStimulusKind kind, const QString& name)
    {
        labels.append(QString::fromLatin1(SMTransitionEntry::toString(kind)) + QStringLiteral(" — ") + name);
        options.append(qMakePair(kind, name));
    };

    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if (method->getMethodType() == SMMethodEntry::eMethodType::Trigger)
        {
            add(SMTransitionEntry::eStimulusKind::Trigger, method->getName());
        }
    }

    for (const SMEventEntry* event : data.getEvents().getElements())
    {
        add(SMTransitionEntry::eStimulusKind::Event, event->getName());
    }

    for (const SMTimerEntry& timer : data.getTimers().getElements())
    {
        add(SMTransitionEntry::eStimulusKind::Timer, timer.getName());
    }

    if (options.isEmpty())
    {
        QMessageBox::information(this, tr("Set Stimulus")
                                 , tr("Declare a trigger, event, or timer first — the stimulus is picked from those registries."));
        return;
    }

    int current = 0;
    for (int i = 0; i < options.size(); ++i)
    {
        if ((options.at(i).first == transition->getStimulusKind()) && (options.at(i).second == transition->getStimulus()))
        {
            current = i;
            break;
        }
    }

    bool accepted = false;
    const QString chosen = QInputDialog::getItem(this, tr("Set Stimulus"), tr("Stimulus:"), labels, current, false, &accepted);
    if (accepted == false)
    {
        return;
    }

    const int index = labels.indexOf(chosen);
    if (index >= 0)
    {
        mModel.getUndoStack().push(new SMSetStimulusCommand(  data, mModel.getNotifier(), transitionId
                                                           , options.at(index).first, options.at(index).second
                                                           , tr("Set stimulus")));
    }
}

void SMDesign::rebuildScene()
{
    SMScene* previous = mScene;
    const uint32_t rootLevel = mModel.getData().getOverview().getId();

    mScene = new SMScene(mModel, rootLevel, this);
    applyGridSettings();
    populateStressContent();
    mView->setScene(mScene);
    mModel.getSelectionModel().setActiveLevel(rootLevel);
    mView->zoomReset();

    if (mActToggleGrid != nullptr)
    {
        mActToggleGrid->setChecked(mScene->isGridVisible());
        mActToggleSnap->setChecked(mScene->isSnapToGrid());
    }

    delete previous;
}

void SMDesign::applyGridSettings()
{
    const SMLayoutData& layout = mModel.getData().getLayout();
    mScene->setGridSize(layout.getGridSize());
    mScene->setGridVisible(layout.isGridVisible());
}

void SMDesign::populateStressContent()
{
    bool valid{ false };
    int  count = qEnvironmentVariableIntValue("LUSAN_SM_CANVAS_STRESS", &valid);
    if (valid == false)
    {
        return;
    }

    count = (count > 0 ? count : 200);
    constexpr int    columns { 20 };
    constexpr double spacingX{ 180.0 };
    constexpr double spacingY{ 110.0 };

    for (int i = 0; i < count; ++i)
    {
        StressNodeItem* node = new StressNodeItem(StressIdBase + static_cast<uint32_t>(i), QString("S%1").arg(i + 1));
        node->setPos((i % columns) * spacingX, (i / columns) * spacingY);
        mScene->addItem(node);
    }
}
