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
 *  \file        lusan/view/sm/SMPropertiesPanel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Design page properties panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMPropertiesPanel.hpp"

#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMConditionEditor.hpp"

#include <QComboBox>
#include <QCompleter>
#include <QDropEvent>
#include <QEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <functional>

namespace
{
    //!< The transition ID carried by each row of the state page's transition list.
    constexpr int RoleTransitionId { Qt::UserRole + 1 };

    //!< The stimulus kind (int of eStimulusKind) carried by each stimulus-picker row.
    constexpr int RoleStimulusKind { Qt::UserRole };
    //!< The real registry name carried by each stimulus-picker row (the label may be prefixed).
    constexpr int RoleStimulusName { Qt::UserRole + 1 };

    //!< The display label for a stimulus: triggers show their method name (unambiguous), events
    //!< and timers are prefixed so the same name across the two registries stays distinct.
    QString stimulusDisplayLabel(SMTransitionEntry::eStimulusKind kind, const QString& name)
    {
        switch (kind)
        {
        case SMTransitionEntry::eStimulusKind::Event: return QStringLiteral("on_event_") + name;
        case SMTransitionEntry::eStimulusKind::Timer: return QStringLiteral("on_timer_") + name;
        default:                                      return name;
        }
    }

    //!< The label shown for a transition with no target (an internal transition).
    QString internalLabel()
    {
        return QObject::tr("(internal)");
    }

    //!< A drag-reorderable list that never mutates itself: it reports the requested move and
    //!< lets the document model (through undo commands) drive the actual reorder.
    class ReorderList : public QListWidget
    {
    public:
        std::function<void(int, int)> mOnReorder; //!< Called with (from, to) on a drop.

        explicit ReorderList(QWidget* parent = nullptr)
            : QListWidget(parent)
        {
            setDragDropMode(QAbstractItemView::InternalMove);
            setDefaultDropAction(Qt::MoveAction);
            setSelectionMode(QAbstractItemView::SingleSelection);
        }

    protected:
        void dropEvent(QDropEvent* event) override
        {
            const int from = currentRow();
            const QModelIndex index = indexAt(event->position().toPoint());
            int insertRow = count();
            if (index.isValid())
            {
                insertRow = (dropIndicatorPosition() == QAbstractItemView::BelowItem)
                        ? index.row() + 1 : index.row();
            }

            int to = (insertRow > from) ? insertRow - 1 : insertRow;
            to = qBound(0, to, count() - 1);

            // Do not let the base view move rows; the model rebuilds this list on reorder.
            event->accept();
            if ((from >= 0) && (to >= 0) && (from != to) && mOnReorder)
            {
                mOnReorder(from, to);
            }
        }
    };

    //!< Normalizes a container element (held by value or by pointer) to a const raw pointer.
    template<typename T> const T* rawPtr(const T& value) { return &value; }
    template<typename T> const T* rawPtr(T* const& value) { return value; }

    //!< Finds the entry with the given ID in a container's element list and yields its name.
    template<typename E>
    bool findEntryName(const QList<E>& list, uint32_t id, QString& out)
    {
        for (const E& element : list)
        {
            const auto* entry = rawPtr(element);
            if ((entry != nullptr) && (entry->getId() == id))
            {
                out = entry->getName();
                return true;
            }
        }

        return false;
    }
}

SMPropertiesPanel::SMPropertiesPanel(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mStack        (new QStackedWidget(this))
    , mPage         (PageEmpty)
    , mCurrentId    (0u)
    , mUpdating     (false)
    , mStateName    (nullptr)
    , mStateKind    (nullptr)
    , mStateDesc    (nullptr)
    , mStateEntry   (nullptr)
    , mStateExit    (nullptr)
    , mTransitions  (nullptr)
    , mStimulusName (nullptr)
    , mTarget       (nullptr)
    , mTransDesc    (nullptr)
    , mConditions   (nullptr)
    , mRegistryInfo (nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mStack);

    QLabel* empty = new QLabel(tr("No selection"), this);
    empty->setAlignment(Qt::AlignCenter);
    empty->setEnabled(false);
    mStack->insertWidget(PageEmpty, empty);

    buildStatePage();
    buildTransitionPage();
    buildRegistryPage();

    connect(&mModel.getSelectionModel(), &SMSelectionModel::signalSelectionChanged, this, &SMPropertiesPanel::onModelSelectionChanged);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMPropertiesPanel::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMPropertiesPanel::onElementRemoved);
    connect(&notifier, &DocModelNotifier::nameChanged, this, &SMPropertiesPanel::onNameChanged);
    connect(&notifier, &DocModelNotifier::listReordered, this, &SMPropertiesPanel::onListReordered);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMPropertiesPanel::onDocumentReloaded);

    refresh();
}

SMPropertiesPanel::~SMPropertiesPanel()
{
    mModel.getNotifier().disconnect(this);
    mModel.getSelectionModel().disconnect(this);
}

void SMPropertiesPanel::buildStatePage()
{
    QWidget* page = new QWidget(this);
    QFormLayout* form = new QFormLayout(page);

    mStateName = new QLineEdit(page);
    mStateKind = new QLabel(page);
    mStateEntry = new QLabel(page);
    mStateExit = new QLabel(page);
    mTransitions = new ReorderList(page);
    mStateDesc = new QPlainTextEdit(page);

    // The transitions list stays compact; the multi-line description takes the room below it.
    mTransitions->setMaximumHeight(120);
    mStateDesc->setPlaceholderText(tr("Description"));
    mStateDesc->installEventFilter(this);   // commit on focus-out (no editingFinished signal)

    form->addRow(tr("Name:"), mStateName);
    form->addRow(tr("Kind:"), mStateKind);
    form->addRow(tr("Entry:"), mStateEntry);
    form->addRow(tr("Exit:"), mStateExit);
    form->addRow(tr("Transitions:"), mTransitions);
    form->addRow(tr("Description:"), mStateDesc);

    connect(mStateName, &QLineEdit::editingFinished, this, &SMPropertiesPanel::onStateNameCommit);
    connect(mTransitions, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { onTransitionActivated(); });

    static_cast<ReorderList*>(mTransitions)->mOnReorder = [this](int from, int to)
    {
        // Defer: the drop is being handled by the list; reorder + repopulate on the next turn.
        QTimer::singleShot(0, this, [this, from, to]() { reorderTransition(from, to); });
    };

    mStack->insertWidget(PageState, page);
}

void SMPropertiesPanel::buildTransitionPage()
{
    // The stimulus/target/description form becomes the General tab; a Conditions tab hosts the
    // guard builder. The stimulus/target/list accessors keep pointing at the same widgets.
    QWidget* page = new QWidget(this);
    QFormLayout* form = new QFormLayout(page);

    // One picker over the whole stimulus vocabulary (triggers, events, timers). The kind is
    // encoded per row (and by the on_event_/on_timer_ prefix), so a separate "kind" combo is
    // redundant. Editing is search-only: the row must come from the list, never a free name.
    mStimulusName = new QComboBox(page);
    mStimulusName->setEditable(true);
    mStimulusName->setInsertPolicy(QComboBox::NoInsert);
    if (mStimulusName->completer() != nullptr)
    {
        mStimulusName->completer()->setCompletionMode(QCompleter::PopupCompletion);
        mStimulusName->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    }

    mTarget = new QComboBox(page);
    mTarget->setEditable(true);
    mTarget->setInsertPolicy(QComboBox::NoInsert);

    mTransDesc = new QPlainTextEdit(page);
    mTransDesc->setPlaceholderText(tr("Description"));
    mTransDesc->installEventFilter(this);   // commit on focus-out (no editingFinished signal)

    form->addRow(tr("Stimulus:"), mStimulusName);
    form->addRow(tr("Target:"), mTarget);
    form->addRow(tr("Description:"), mTransDesc);

    connect(mStimulusName, &QComboBox::activated, this, &SMPropertiesPanel::onStimulusCommit);
    connect(mStimulusName->lineEdit(), &QLineEdit::editingFinished, this, &SMPropertiesPanel::onStimulusCommit);
    connect(mTarget, &QComboBox::activated, this, &SMPropertiesPanel::onTargetCommit);
    connect(mTarget->lineEdit(), &QLineEdit::editingFinished, this, &SMPropertiesPanel::onTargetCommit);

    QTabWidget* tabs = new QTabWidget(this);
    tabs->addTab(page, tr("General"));
    mConditions = new SMConditionEditor(mModel, this);
    tabs->addTab(mConditions, tr("Conditions"));

    mStack->insertWidget(PageTransition, tabs);
}

void SMPropertiesPanel::buildRegistryPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* box = new QVBoxLayout(page);
    mRegistryInfo = new QLabel(page);
    mRegistryInfo->setWordWrap(true);
    mRegistryInfo->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    box->addWidget(mRegistryInfo);
    box->addStretch(1);
    mStack->insertWidget(PageRegistry, page);
}

bool SMPropertiesPanel::isEditing() const
{
    QWidget* focus = focusWidget();
    return (focus != nullptr) && isAncestorOf(focus);
}

bool SMPropertiesPanel::eventFilter(QObject* watched, QEvent* event)
{
    // QPlainTextEdit has no editing-finished signal; commit its edit when it loses focus,
    // matching the single-line fields' commit-on-editing-finished contract.
    if (event->type() == QEvent::FocusOut)
    {
        if (watched == mStateDesc)
        {
            onStateDescriptionCommit();
        }
        else if (watched == mTransDesc)
        {
            onTransitionDescriptionCommit();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SMPropertiesPanel::onModelSelectionChanged()
{
    refresh();
}

void SMPropertiesPanel::refresh()
{
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        showEmpty();
        return;
    }

    const uint32_t id = selection.first();
    StateMachineData& data = mModel.getData();
    if (data.findStateById(id) != nullptr)
    {
        showState(id);
    }
    else if (data.findTransitionById(id) != nullptr)
    {
        showTransition(id);
    }
    else
    {
        showRegistry(id);
    }
}

void SMPropertiesPanel::showEmpty()
{
    mPage = PageEmpty;
    mCurrentId = 0u;
    if (mConditions != nullptr)
    {
        mConditions->setTransition(0u);
    }

    mStack->setCurrentIndex(PageEmpty);
}

void SMPropertiesPanel::showState(uint32_t stateId)
{
    const SMStateEntry* state = mModel.getData().findStateById(stateId);
    if (state == nullptr)
    {
        showEmpty();
        return;
    }

    mUpdating = true;
    mPage = PageState;
    mCurrentId = stateId;

    mStateName->setText(state->getName());
    mStateName->setReadOnly(state->getKind() == SMStateEntry::eStateKind::Start);
    mStateKind->setText(QString::fromLatin1(SMStateEntry::toString(state->getKind())));
    mStateDesc->setPlainText(state->getDescription());
    mStateEntry->setText(tr("%n operation(s)", nullptr, state->getEntryList().getCount()));
    mStateExit->setText(tr("%n operation(s)", nullptr, state->getExitList().getCount()));
    populateTransitionList(stateId);

    mStack->setCurrentIndex(PageState);
    mUpdating = false;
}

void SMPropertiesPanel::populateTransitionList(uint32_t stateId)
{
    mTransitions->clear();
    const SMStateEntry* state = mModel.getData().findStateById(stateId);
    if (state == nullptr)
    {
        return;
    }

    for (SMTransitionEntry* transition : state->getTransitions().getElements())
    {
        if (transition == nullptr)
        {
            continue;
        }

        const QString stimulus = transition->getStimulus().isEmpty() ? tr("<stimulus>") : transition->getStimulus();
        const QString label = transition->isExternal()
                ? (stimulus + QStringLiteral(" -> ") + transition->getTo())
                : (stimulus + QStringLiteral(" ") + internalLabel());
        QListWidgetItem* item = new QListWidgetItem(label, mTransitions);
        item->setData(RoleTransitionId, transition->getId());
    }
}

void SMPropertiesPanel::showTransition(uint32_t transitionId)
{
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        showEmpty();
        return;
    }

    mUpdating = true;
    mPage = PageTransition;
    mCurrentId = transitionId;

    // Fill the picker with every trigger/event/timer and select the transition's current one by
    // its (kind, name) display label.
    const QString currentLabel = populateStimulusPicker(static_cast<int>(transition->getStimulusKind()), transition->getStimulus());
    mStimulusName->setEditText(currentLabel);

    // Populate the target picker from the sibling states of the transition's level.
    mTarget->clear();
    mTarget->addItem(internalLabel());
    const SMStateData* level = data.findLevel(mModel.getSelectionModel().getActiveLevel());
    if (level != nullptr)
    {
        for (SMStateEntry* sibling : level->getElements())
        {
            if (sibling != nullptr) mTarget->addItem(sibling->getName());
        }
    }

    mTarget->setEditText(transition->isExternal() ? transition->getTo() : internalLabel());
    mTransDesc->setPlainText(transition->getDescription());

    mConditions->setTransition(transitionId);

    mStack->setCurrentIndex(PageTransition);
    mUpdating = false;
}

void SMPropertiesPanel::showRegistry(uint32_t elementId)
{
    StateMachineData& data = mModel.getData();
    QString kind;
    QString name;

    const bool found =
           (findEntryName(data.getDataTypes().getElements(), elementId, name)  && (kind = tr("Data type"), true))
        || (findEntryName(data.getAttributes().getElements(), elementId, name) && (kind = tr("Attribute"), true))
        || (findEntryName(data.getEvents().getElements(), elementId, name)     && (kind = tr("Event"), true))
        || (findEntryName(data.getTimers().getElements(), elementId, name)     && (kind = tr("Timer"), true))
        || (findEntryName(data.getMethods().getElements(), elementId, name)    && (kind = tr("Method"), true))
        || (findEntryName(data.getConstants().getElements(), elementId, name)  && (kind = tr("Constant"), true))
        || (findEntryName(data.getIncludes().getElements(), elementId, name)   && (kind = tr("Include"), true))
        || (findEntryName(data.getImports().getElements(), elementId, name)    && (kind = tr("Import"), true));

    if (found)
    {
        mPage = PageRegistry;
        mCurrentId = elementId;
        mRegistryInfo->setText(tr("%1: %2\n\nEdit this entry on its document page.").arg(kind, name));
        mStack->setCurrentIndex(PageRegistry);
    }
    else
    {
        showEmpty();
    }
}

void SMPropertiesPanel::onStateNameCommit()
{
    if (mUpdating || (mPage != PageState))
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMStateEntry* state = data.findStateById(mCurrentId);
    if (state == nullptr)
    {
        return;
    }

    const QString name = mStateName->text().trimmed();
    if (name == state->getName())
    {
        return;
    }

    const SMStateEntry* clash = data.findState(name);
    if ((StateMachineData::isValidIdentifier(name) == false) || ((clash != nullptr) && (clash->getId() != mCurrentId)))
    {
        mStateName->setText(state->getName());   // reject: same rule as canvas F2
        return;
    }

    mModel.getUndoStack().push(new SMRenameStateCommand(data, mModel.getNotifier(), mCurrentId, name, tr("Rename state")));
}

void SMPropertiesPanel::onStateDescriptionCommit()
{
    if (mUpdating || (mPage != PageState))
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMStateEntry* state = data.findStateById(mCurrentId);
    if ((state == nullptr) || (mStateDesc->toPlainText() == state->getDescription()))
    {
        return;
    }

    const uint32_t id = mCurrentId;
    StateMachineData* doc = &data;
    auto getter = [doc, id]() -> QString { SMStateEntry* e = doc->findStateById(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [doc, id](const QString& value) { SMStateEntry* e = doc->findStateById(id); if (e != nullptr) e->setDescription(value); };
    mModel.getUndoStack().push(new TDocSetPropertyCommand<QString>(mModel.getNotifier(), id, eDocElementKind::State, getter, setter, mStateDesc->toPlainText(), tr("Set description")));
}

void SMPropertiesPanel::onTransitionDescriptionCommit()
{
    if (mUpdating || (mPage != PageTransition))
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mCurrentId);
    if ((transition == nullptr) || (mTransDesc->toPlainText() == transition->getDescription()))
    {
        return;
    }

    const uint32_t id = mCurrentId;
    StateMachineData* doc = &data;
    auto getter = [doc, id]() -> QString { SMTransitionEntry* e = doc->findTransitionById(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [doc, id](const QString& value) { SMTransitionEntry* e = doc->findTransitionById(id); if (e != nullptr) e->setDescription(value); };
    mModel.getUndoStack().push(new TDocSetPropertyCommand<QString>(mModel.getNotifier(), id, eDocElementKind::Transition, getter, setter, mTransDesc->toPlainText(), tr("Set description")));
}

void SMPropertiesPanel::onStimulusCommit()
{
    if (mUpdating || (mPage != PageTransition))
    {
        return;
    }

    applyStimulus();
}

QString SMPropertiesPanel::populateStimulusPicker(int currentKind, const QString& currentName)
{
    StateMachineData& data = mModel.getData();
    mStimulusName->clear();

    const auto addRow = [this](SMTransitionEntry::eStimulusKind kind, const QString& name)
    {
        if (name.isEmpty())
        {
            return;
        }

        const int row = mStimulusName->count();
        mStimulusName->addItem(stimulusDisplayLabel(kind, name));
        mStimulusName->setItemData(row, static_cast<int>(kind), RoleStimulusKind);
        mStimulusName->setItemData(row, name, RoleStimulusName);
    };

    // Triggers are the FSM's methods of trigger type (checked against the Methods page).
    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if ((method != nullptr) && (method->getMethodType() == SMMethodEntry::eMethodType::Trigger))
        {
            addRow(SMTransitionEntry::eStimulusKind::Trigger, method->getName());
        }
    }

    for (const SMEventEntry* event : data.getEvents().getElements())
    {
        if (event != nullptr)
        {
            addRow(SMTransitionEntry::eStimulusKind::Event, event->getName());
        }
    }

    for (const SMTimerEntry& timer : data.getTimers().getElements())
    {
        addRow(SMTransitionEntry::eStimulusKind::Timer, timer.getName());
    }

    return stimulusDisplayLabel(static_cast<SMTransitionEntry::eStimulusKind>(currentKind), currentName);
}

void SMPropertiesPanel::applyStimulus()
{
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mCurrentId);
    if (transition == nullptr)
    {
        return;
    }

    const QString typed = mStimulusName->currentText().trimmed();
    const QString currentLabel = stimulusDisplayLabel(transition->getStimulusKind(), transition->getStimulus());

    // A cleared field detaches the stimulus (allowed); anything else must be one of the listed
    // rows - the picker never renames or creates a registry entry (fixed-list only).
    if (typed.isEmpty())
    {
        if (transition->getStimulus().isEmpty() == false)
        {
            mModel.getUndoStack().push(new SMSetStimulusCommand(data, mModel.getNotifier(), mCurrentId, transition->getStimulusKind(), QString(), tr("Clear stimulus")));
        }

        return;
    }

    const int row = mStimulusName->findText(typed, Qt::MatchFixedString);
    if (row < 0)
    {
        mStimulusName->setEditText(currentLabel);   // reject a value that is not on the list
        return;
    }

    const SMTransitionEntry::eStimulusKind kind =
            static_cast<SMTransitionEntry::eStimulusKind>(mStimulusName->itemData(row, RoleStimulusKind).toInt());
    const QString name = mStimulusName->itemData(row, RoleStimulusName).toString();
    if ((kind == transition->getStimulusKind()) && (name == transition->getStimulus()))
    {
        return;
    }

    mModel.getUndoStack().push(new SMSetStimulusCommand(data, mModel.getNotifier(), mCurrentId, kind, name, tr("Set stimulus")));
}

void SMPropertiesPanel::onTargetCommit()
{
    if (mUpdating || (mPage != PageTransition))
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mCurrentId);
    if (transition == nullptr)
    {
        return;
    }

    const QString text = mTarget->currentText().trimmed();
    if ((text.isEmpty()) || (text == internalLabel()))
    {
        if (transition->isExternal())
        {
            mModel.getUndoStack().push(new SMSetTransitionTargetCommand(data, mModel.getNotifier(), mCurrentId, QString(), tr("Make internal")));
        }

        return;
    }

    const SMStateData* level = data.findLevel(mModel.getSelectionModel().getActiveLevel());
    const bool sibling = (level != nullptr) && (level->findState(text) != nullptr);
    if (sibling == false)
    {
        mTarget->setEditText(transition->isExternal() ? transition->getTo() : internalLabel());   // reject unknown target
        return;
    }

    if (text != transition->getTo())
    {
        mModel.getUndoStack().push(new SMSetTransitionTargetCommand(data, mModel.getNotifier(), mCurrentId, text, tr("Set target")));
    }
}

void SMPropertiesPanel::onTransitionActivated()
{
    QListWidgetItem* item = mTransitions->currentItem();
    if ((item == nullptr) || mUpdating)
    {
        return;
    }

    const uint32_t transitionId = item->data(RoleTransitionId).toUInt();
    if (transitionId != 0)
    {
        mModel.getSelectionModel().setSelection(QList<uint32_t>{ transitionId });
    }
}

void SMPropertiesPanel::reorderTransition(int from, int to)
{
    StateMachineData& data = mModel.getData();
    SMStateEntry* state = data.findStateById(mCurrentId);
    if ((state == nullptr) || (mPage != PageState))
    {
        return;
    }

    SMTransitionData& list = state->getTransitions();
    if ((from < 0) || (to < 0) || (from >= list.getElementCount()) || (to >= list.getElementCount()) || (from == to))
    {
        return;
    }

    DocModelNotifier& notifier = mModel.getNotifier();
    SMCompositeCommand* composite = new SMCompositeCommand(data, notifier, tr("Reorder transition priority"));
    const int step = (to > from) ? 1 : -1;
    for (int i = from; i != to; i += step)
    {
        new TDocReorderCommand<SMTransitionEntry*, DocumentElem>(notifier, list, i, i + step, mCurrentId, eDocElementKind::Transition, tr("Reorder transition priority"), composite);
    }

    mModel.getUndoStack().push(composite);
}

void SMPropertiesPanel::onElementChanged(uint32_t id, eDocElementKind /*kind*/)
{
    if ((id == mCurrentId) && (isEditing() == false))
    {
        refresh();
    }
}

void SMPropertiesPanel::onElementRemoved(uint32_t id, eDocElementKind /*kind*/)
{
    if (id == mCurrentId)
    {
        showEmpty();
    }
}

void SMPropertiesPanel::onNameChanged(uint32_t id, const QString& /*oldName*/, const QString& /*newName*/)
{
    if ((id == mCurrentId) && (isEditing() == false))
    {
        refresh();
    }
    else if ((mPage == PageTransition) && (isEditing() == false))
    {
        // A renamed sibling state may be this transition's target label.
        refresh();
    }
}

void SMPropertiesPanel::onListReordered(uint32_t ownerId, eDocElementKind kind)
{
    if ((mPage == PageState) && (ownerId == mCurrentId) && (kind == eDocElementKind::Transition))
    {
        populateTransitionList(mCurrentId);
    }
}

void SMPropertiesPanel::onDocumentReloaded()
{
    showEmpty();
}
