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
 *  \file        lusan/view/sm/SMOperationsEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation-list editor (entry/exit/transition actions).
 *
 ************************************************************************/

#include "lusan/view/sm/SMOperationsEditor.hpp"

#include "lusan/data/common/MethodBase.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocCommand.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMArgumentCommands.hpp"
#include "lusan/model/sm/SMOperationCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMSectionChrome.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"

#include <QApplication>
#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QScrollArea>
#include <QSet>
#include <QSignalBlocker>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    using eOp     = SMOperationBase::eOperation;
    using eSource = SMArgumentEntry::eValueSource;

    //!< The Actions/Events source universe: a literal, a stimulus parameter, a machine attribute or
    //!< a constant. The condition and verbatim-C++ kinds belong to Conditions only.
    QList<eSource> kActionSources(void)
    {
        return { eSource::Value, eSource::Param, eSource::Attribute, eSource::Constant };
    }

    //!< Removes and deletes every widget owned by a layout (used to rebuild the timer rows).
    void clearLayout(QLayout* layout)
    {
        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QWidget* w = item->widget())
            {
                w->deleteLater();
            }
            delete item;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMOperationsEditor::SMOperationsEditor(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mOwnerId          (0u)
    , mOwnerKind        (eDocElementKind::State)
    , mScopeTransition  (0u)
    , mAllowParam       (false)
    , mOwner            (nullptr)
    , mList             (nullptr)
    , mApplying         (false)
    , mRebuildQueued    (false)
    , mActionSink       (model)
    , mEventSink        (model)
    , mChrome           (nullptr)
    , mActionCombo      (nullptr)
    , mActionParams     (nullptr)
    , mEventCombo       (nullptr)
    , mEventParams      (nullptr)
    , mTimersList       (nullptr)
    , mTimersEmpty      (nullptr)
{
    buildUi();

    DocModelNotifier& notifier = mModel.getNotifier();
    const auto onChange = [this](uint32_t id, eDocElementKind kind) { onNotifierChanged(id, kind); };
    connect(&notifier, &DocModelNotifier::elementAdded, this, onChange);
    connect(&notifier, &DocModelNotifier::elementRemoved, this
          , [this](uint32_t id, eDocElementKind kind) { onElementRemoved(id, kind); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, onChange);
    connect(&notifier, &DocModelNotifier::listReordered, this, onChange);
    connect(&notifier, &DocModelNotifier::nameChanged, this, [this](uint32_t id, const QString&, const QString&) { onNotifierChanged(id, eDocElementKind::State); });
}

SMOperationsEditor::~SMOperationsEditor()
{
    mModel.getNotifier().disconnect(this);
}

void SMOperationsEditor::buildUi()
{
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    // A scroll area keeps the outer size stable when parameter rows appear or disappear.
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outer->addWidget(scroll);

    QWidget* content = new QWidget(scroll);
    QVBoxLayout* box = new QVBoxLayout(content);
    box->setContentsMargins(6, 6, 6, 6);
    box->setSpacing(8);

    // Action / Event / Timers are three collapsible sections under a shared chrome.
    mChrome = new SMSectionChrome(content);

    // Action section.
    QWidget* actionBody = new QWidget();
    QVBoxLayout* actionBox = new QVBoxLayout(actionBody);
    actionBox->setContentsMargins(6, 6, 6, 6);
    mActionCombo = new QComboBox(actionBody);
    connect(mActionCombo, &QComboBox::activated, this, [this](int) { setActionName(mActionCombo->currentText()); });
    actionBox->addWidget(mActionCombo);
    mActionParams = new SMArgMapTable(mModel, actionBody);
    // One merged cell per parameter rather than a source picker beside a value editor: a parameter
    // binds to a source or to a typed value, never both, and the narrow dock keeps a column.
    mActionParams->setRowStyle(SMArgMapTable::eRowStyle::Compact);
    mActionParams->setAllowedSources(kActionSources());
    actionBox->addWidget(mActionParams);
    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::NewAction), tr("Action"), actionBody
                       , tr("The action call this operation runs"));

    // Event section.
    QWidget* eventBody = new QWidget();
    QVBoxLayout* eventBox = new QVBoxLayout(eventBody);
    eventBox->setContentsMargins(6, 6, 6, 6);
    mEventCombo = new QComboBox(eventBody);
    connect(mEventCombo, &QComboBox::activated, this, [this](int) { setEventName(mEventCombo->currentText()); });
    eventBox->addWidget(mEventCombo);
    mEventParams = new SMArgMapTable(mModel, eventBody);
    mEventParams->setRowStyle(SMArgMapTable::eRowStyle::Compact);    // the same merged cell
    mEventParams->setAllowedSources(kActionSources());
    eventBox->addWidget(mEventParams);
    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::NewEvent), tr("Event"), eventBody
                       , tr("The event this operation sends"));

    // Timers section.
    QWidget* timerBody = new QWidget();
    QVBoxLayout* timerBox = new QVBoxLayout(timerBody);
    timerBox->setContentsMargins(6, 6, 6, 6);
    QHBoxLayout* timerButtons = new QHBoxLayout();
    QToolButton* addStart = new QToolButton(timerBody);
    addStart->setText(tr("+ Start timer"));
    addStart->setToolButtonStyle(Qt::ToolButtonTextOnly);
    connect(addStart, &QToolButton::clicked, this, [this]() { addTimer(true); });
    QToolButton* addStop = new QToolButton(timerBody);
    addStop->setText(tr("+ Stop timer"));
    addStop->setToolButtonStyle(Qt::ToolButtonTextOnly);
    connect(addStop, &QToolButton::clicked, this, [this]() { addTimer(false); });
    timerButtons->addWidget(addStart);
    timerButtons->addWidget(addStop);
    timerButtons->addStretch(1);
    timerBox->addLayout(timerButtons);
    QWidget* timersHost = new QWidget(timerBody);
    mTimersList = new QVBoxLayout(timersHost);
    mTimersList->setContentsMargins(0, 0, 0, 0);
    timerBox->addWidget(timersHost);
    mTimersEmpty = new QLabel(tr("No timers."), timerBody);
    mTimersEmpty->setEnabled(false);
    timerBox->addWidget(mTimersEmpty);
    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::NewTimer), tr("Timers"), timerBody
                       , tr("The timers this operation starts or stops"));

    mChrome->setCompact(false);
    // Every section starts open, so the tab reads as one short form instead of a stack of closed
    // bars. A section appended later opens itself the same way.
    mChrome->openAllSections();

    box->addWidget(mChrome);
    box->addStretch(1);
    scroll->setWidget(content);
}

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

void SMOperationsEditor::bind( uint32_t ownerId
                             , eDocElementKind ownerKind
                             , uint32_t scopeTransition
                             , ElementBase* owner
                             , SMOperationList* list)
{
    mOwnerId         = ownerId;
    mOwnerKind       = ownerKind;
    mScopeTransition = scopeTransition;
    mAllowParam      = (scopeTransition != 0u);
    mOwner           = owner;
    mList            = list;
    rebuild();
}

void SMOperationsEditor::clearBinding()
{
    mList = nullptr;
    mOwner = nullptr;
    rebuild();
}

void SMOperationsEditor::setSectionsCompact(bool compact)
{
    mChrome->setCompact(compact);
    if (compact)
    {
        mChrome->setCurrentSection(0);      // the Action section: what a host asks for first
    }
    else
    {
        mChrome->openAllSections();
    }
}

//////////////////////////////////////////////////////////////////////////
// Rebuild
//////////////////////////////////////////////////////////////////////////

void SMOperationsEditor::rebuild()
{
    mApplying = true;
    setEnabled(mList != nullptr);
    rebuildAction();
    rebuildEvent();
    rebuildTimers();
    mApplying = false;
}

void SMOperationsEditor::rebuildAction()
{
    const QSignalBlocker block(mActionCombo);
    mActionCombo->clear();
    mActionCombo->addItem(tr("(none)"));
    mActionCombo->addItems(actionNames());

    SMActionCall* action = findAction();
    mActionCombo->setCurrentText(action != nullptr ? action->getAction() : tr("(none)"));
    bindParamRows(mActionParams, mActionSink, action != nullptr ? action->getId() : 0u, false);
}

void SMOperationsEditor::rebuildEvent()
{
    const QSignalBlocker block(mEventCombo);
    mEventCombo->clear();
    mEventCombo->addItem(tr("(none)"));
    mEventCombo->addItems(eventNames());

    SMEventSend* event = findEvent();
    mEventCombo->setCurrentText(event != nullptr ? event->getEvent() : tr("(none)"));
    bindParamRows(mEventParams, mEventSink, event != nullptr ? event->getId() : 0u, true);
}

void SMOperationsEditor::rebuildTimers()
{
    clearLayout(mTimersList);

    int count = 0;
    if (mList != nullptr)
    {
        const QStringList timers = timerNames();
        for (SMOperationBase* op : mList->getOperations())
        {
            const bool isStart = (op->getOperationType() == eOp::TimerStart);
            const bool isStop  = (op->getOperationType() == eOp::TimerStop);
            if ((isStart == false) && (isStop == false))
            {
                continue;
            }

            ++count;
            const uint32_t opId = op->getId();
            const QString current = isStart ? static_cast<SMTimerStart*>(op)->getTimer() : static_cast<SMTimerStop*>(op)->getTimer();

            QWidget* row = new QWidget();
            QHBoxLayout* rowBox = new QHBoxLayout(row);
            rowBox->setContentsMargins(0, 0, 0, 0);

            QLabel* kind = new QLabel(isStart ? tr("Start") : tr("Stop"), row);
            kind->setMinimumWidth(36);
            QFont f = kind->font();
            f.setBold(true);
            kind->setFont(f);

            QComboBox* combo = new QComboBox(row);
            combo->addItems(timers);
            combo->setCurrentText(current);
            combo->setProperty("opId", opId);
            connect(combo, &QComboBox::activated, this, [this, opId, combo](int) { setTimerName(opId, combo->currentText()); });

            QToolButton* remove = new QToolButton(row);
            remove->setText(tr("x"));
            remove->setToolTip(tr("Remove timer"));
            connect(remove, &QToolButton::clicked, this, [this, opId]() { removeTimer(opId); });

            rowBox->addWidget(kind);
            rowBox->addWidget(combo, 1);
            rowBox->addWidget(remove);
            mTimersList->addWidget(row);
        }
    }

    mTimersEmpty->setVisible(count == 0);
}

void SMOperationsEditor::bindParamRows(SMArgMapTable* table, SMArgSinkOperation& sink, uint32_t opId, bool isEvent)
{
    SMOperationBase* op = ((mList != nullptr) && (opId != 0u)) ? mList->findById(opId) : nullptr;
    const MethodBase* callee = calleeFor(op, isEvent);
    if ((op == nullptr) || (callee == nullptr))
    {
        table->clearBinding();
        sink.clearBinding();
        return;
    }

    QList<SMArgumentEntry>& args = isEvent ? static_cast<SMEventSend*>(op)->getArguments()
                                           : static_cast<SMActionCall*>(op)->getArguments();

    QList<SMArgMapTable::Param> params;
    QSet<QString> formalNames;
    for (const MethodParameter& param : callee->getElements())
    {
        formalNames.insert(param.getName());
        params.append(SMArgMapTable::Param{ param.getName(), param.getType(), param.getValue(), param.hasDefault(), false });
    }

    // A formal removed on the Methods page leaves a stored argument matching no parameter. It is
    // never dropped silently: it becomes a red orphan row after the live parameters.
    for (const SMArgumentEntry& arg : args)
    {
        if (formalNames.contains(arg.getName()) == false)
        {
            params.append(SMArgMapTable::Param{ arg.getName(), QString(), QString(), false, true });
        }
    }

    sink.bind(op, &args);
    table->bind(mScopeTransition, mAllowParam, &sink, params);
}

//////////////////////////////////////////////////////////////////////////
// Finders
//////////////////////////////////////////////////////////////////////////

SMActionCall* SMOperationsEditor::findAction() const
{
    if (mList != nullptr)
    {
        for (SMOperationBase* op : mList->getOperations())
        {
            if (op->getOperationType() == eOp::ActionCall) { return static_cast<SMActionCall*>(op); }
        }
    }

    return nullptr;
}

SMEventSend* SMOperationsEditor::findEvent() const
{
    if (mList != nullptr)
    {
        for (SMOperationBase* op : mList->getOperations())
        {
            if (op->getOperationType() == eOp::EventSend) { return static_cast<SMEventSend*>(op); }
        }
    }

    return nullptr;
}

const MethodBase* SMOperationsEditor::calleeFor(const SMOperationBase* op, bool isEvent) const
{
    if (op == nullptr)
    {
        return nullptr;
    }

    const QString name = isEvent ? static_cast<const SMEventSend*>(op)->getEvent()
                                 : static_cast<const SMActionCall*>(op)->getAction();
    if (isEvent)
    {
        for (const SMEventEntry* e : mModel.getData().getEvents().getElements())
        {
            if ((e != nullptr) && (e->getName() == name)) { return e; }
        }
    }
    else
    {
        for (const SMMethodEntry* m : mModel.getData().getMethods().getElements())
        {
            if ((m != nullptr) && (m->getName() == name)) { return m; }
        }
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////

void SMOperationsEditor::setActionName(const QString& name)
{
    if (mApplying || (mList == nullptr))
    {
        return;
    }

    const QString clean = (name == tr("(none)")) ? QString() : name;
    SMActionCall* action = findAction();
    DocModelNotifier& notifier = mModel.getNotifier();

    if (clean.isEmpty())
    {
        if (action != nullptr)
        {
            mModel.getUndoStack().push(new SMRemoveOperationCommand(notifier, *mList, action->getId(), tr("Remove action")));
        }
    }
    else if (action != nullptr)
    {
        if (action->getAction() == clean)
        {
            return;
        }

        // Changing the callee changes the parameter set; drop stale mappings in one undo step.
        DocCompositeCommand* composite = new DocCompositeCommand(notifier, tr("Set action"));
        for (const SMArgumentEntry& arg : action->getArguments())
        {
            new SMSetArgumentCommand(notifier, *action, action->getArguments(), arg.getName(), false, eSource::Value, QString(), QString(), tr("Clear argument"), composite);
        }
        auto getter = [](SMOperationBase& o) -> QString { return static_cast<SMActionCall&>(o).getAction(); };
        auto setter = [](SMOperationBase& o, const QString& v) { static_cast<SMActionCall&>(o).setAction(v); };
        new SMSetOperationPropertyCommand<QString>(notifier, *mList, action->getId(), getter, setter, clean, tr("Set action"), composite);
        mModel.getUndoStack().push(composite);
    }
    else
    {
        SMActionCall* call = new SMActionCall(nullptr);
        call->setAction(clean);
        mModel.getUndoStack().push(new SMAddOperationCommand(notifier, *mList, call, 0, tr("Add action")));
    }

    // The combo already shows the pick, so refresh only the parameter rows; a full rebuild would
    // destroy the combo just clicked. The canvas edge follows the command's own notification.
    SMActionCall* updated = findAction();
    bindParamRows(mActionParams, mActionSink, updated != nullptr ? updated->getId() : 0u, false);
}

void SMOperationsEditor::setEventName(const QString& name)
{
    if (mApplying || (mList == nullptr))
    {
        return;
    }

    const QString clean = (name == tr("(none)")) ? QString() : name;
    SMEventSend* event = findEvent();
    DocModelNotifier& notifier = mModel.getNotifier();

    if (clean.isEmpty())
    {
        if (event != nullptr)
        {
            mModel.getUndoStack().push(new SMRemoveOperationCommand(notifier, *mList, event->getId(), tr("Remove event")));
        }
    }
    else if (event != nullptr)
    {
        if (event->getEvent() == clean)
        {
            return;
        }

        DocCompositeCommand* composite = new DocCompositeCommand(notifier, tr("Set event"));
        for (const SMArgumentEntry& arg : event->getArguments())
        {
            new SMSetArgumentCommand(notifier, *event, event->getArguments(), arg.getName(), false, eSource::Value, QString(), QString(), tr("Clear argument"), composite);
        }
        auto getter = [](SMOperationBase& o) -> QString { return static_cast<SMEventSend&>(o).getEvent(); };
        auto setter = [](SMOperationBase& o, const QString& v) { static_cast<SMEventSend&>(o).setEvent(v); };
        new SMSetOperationPropertyCommand<QString>(notifier, *mList, event->getId(), getter, setter, clean, tr("Set event"), composite);
        mModel.getUndoStack().push(composite);
    }
    else
    {
        // The event runs after the action: insert it right after the action (or first if none).
        SMActionCall* action = findAction();
        const int index = (action != nullptr) ? (mList->indexOf(action->getId()) + 1) : 0;
        SMEventSend* send = new SMEventSend(nullptr);
        send->setEvent(clean);
        mModel.getUndoStack().push(new SMAddOperationCommand(notifier, *mList, send, index, tr("Add event")));
    }

    SMEventSend* updated = findEvent();
    bindParamRows(mEventParams, mEventSink, updated != nullptr ? updated->getId() : 0u, true);
}

void SMOperationsEditor::addTimer(bool start)
{
    if (mList == nullptr)
    {
        return;
    }

    const QStringList timers = timerNames();
    const QString name = timers.isEmpty() ? QString() : timers.first();
    SMOperationBase* op = start ? static_cast<SMOperationBase*>(new SMTimerStart(nullptr)) : static_cast<SMOperationBase*>(new SMTimerStop(nullptr));
    if (start) { static_cast<SMTimerStart*>(op)->setTimer(name); } else { static_cast<SMTimerStop*>(op)->setTimer(name); }

    mModel.getUndoStack().push(new SMAddOperationCommand(mModel.getNotifier(), *mList, op, mList->getCount(), start ? tr("Add start timer") : tr("Add stop timer")));
    rebuildTimers();
}

void SMOperationsEditor::removeTimer(uint32_t opId)
{
    if (mList != nullptr)
    {
        mModel.getUndoStack().push(new SMRemoveOperationCommand(mModel.getNotifier(), *mList, opId, tr("Remove timer")));
        rebuildTimers();
    }
}

void SMOperationsEditor::setTimerName(uint32_t opId, const QString& name)
{
    if (mApplying || (mList == nullptr))
    {
        return;
    }

    SMOperationBase* op = mList->findById(opId);
    if (op == nullptr)
    {
        return;
    }

    const QString current = (op->getOperationType() == eOp::TimerStart) ? static_cast<SMTimerStart*>(op)->getTimer() : static_cast<SMTimerStop*>(op)->getTimer();
    if (current == name)
    {
        return;
    }

    auto getter = [](SMOperationBase& o) -> QString { return (o.getOperationType() == eOp::TimerStart) ? static_cast<SMTimerStart&>(o).getTimer() : static_cast<SMTimerStop&>(o).getTimer(); };
    auto setter = [](SMOperationBase& o, const QString& v) { if (o.getOperationType() == eOp::TimerStart) { static_cast<SMTimerStart&>(o).setTimer(v); } else { static_cast<SMTimerStop&>(o).setTimer(v); } };
    mModel.getUndoStack().push(new SMSetOperationPropertyCommand<QString>(mModel.getNotifier(), *mList, opId, getter, setter, name, tr("Set timer")));
}

//////////////////////////////////////////////////////////////////////////
// Registry helpers
//////////////////////////////////////////////////////////////////////////

QStringList SMOperationsEditor::actionNames() const
{
    QStringList names;
    for (const SMMethodEntry* m : mModel.getData().getMethods().getElements())
    {
        if ((m != nullptr) && (m->getMethodType() == SMMethodEntry::eMethodType::Action)) { names.append(m->getName()); }
    }

    return names;
}

QStringList SMOperationsEditor::eventNames() const
{
    QStringList names;
    for (const SMEventEntry* e : mModel.getData().getEvents().getElements())
    {
        if (e != nullptr) { names.append(e->getName()); }
    }

    return names;
}

QStringList SMOperationsEditor::timerNames() const
{
    QStringList names;
    for (const SMTimerEntry& t : mModel.getData().getTimers().getElements())
    {
        names.append(t.getName());
    }

    return names;
}

//////////////////////////////////////////////////////////////////////////
// Notifications
//////////////////////////////////////////////////////////////////////////

void SMOperationsEditor::onElementRemoved(uint32_t id, eDocElementKind kind)
{
    // The bound owner was deleted, so mOwner and mList point into freed memory that a queued
    // rebuild would iterate. Drop the binding; the panel re-binds on the next selection.
    if ((id == mOwnerId) && (kind == mOwnerKind))
    {
        clearBinding();
    }
    else
    {
        onNotifierChanged(id, kind);
    }
}

void SMOperationsEditor::onNotifierChanged(uint32_t /*id*/, eDocElementKind kind)
{
    // Operation edits, and any registry change that shifts what the combos or the argument cells
    // show. Refresh, but never while the user is mid-edit in a field.
    if ((kind == eDocElementKind::Operation) || (kind == eDocElementKind::Method) || (kind == eDocElementKind::Event)
        || (kind == eDocElementKind::Timer) || (kind == eDocElementKind::Attribute) || (kind == eDocElementKind::Constant)
        || (kind == eDocElementKind::DataType) || (kind == eDocElementKind::State) || (kind == eDocElementKind::Transition))
    {
        scheduleRebuild();
    }
}

void SMOperationsEditor::scheduleRebuild()
{
    if (mApplying || mRebuildQueued)
    {
        return;
    }

    mRebuildQueued = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildQueued = false;
        if (isEditing() == false)
        {
            rebuild();
        }
    });
}

bool SMOperationsEditor::isEditing() const
{
    // Use the application's active focus, not QWidget::focusWidget(): the latter keeps returning
    // the last-focused descendant, which reported "editing" forever and skipped every rebuild.
    QWidget* focus = QApplication::focusWidget();
    return (focus != nullptr) && isAncestorOf(focus);
}
