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
#include "lusan/model/sm/SMOperationSummary.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/view/sm/SMAccordion.hpp"
#include "lusan/view/sm/SMGuardBar.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMOperationsEditor.hpp"
#include "lusan/view/sm/SMSectionChrome.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QCompleter>
#include <QDropEvent>
#include <QEvent>
#include <QFont>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QSpinBox>
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

    //!< A one-line summary of an operation list for a State-Actions section header: the operation
    //!< one-liners joined, or `not set` when the list is empty.
    QString operationsSummary(const StateMachineData& data, const SMOperationList& list)
    {
        if (list.isEmpty())
        {
            return QObject::tr("not set");
        }

        QStringList parts;
        for (const SMOperationBase* op : list.getOperations())
        {
            if (op != nullptr)
            {
                parts.append(SMOperationSummary::text(data, *op));
            }
        }

        return parts.join(QStringLiteral(", "));
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
    , mStateTabs    (nullptr)
    , mStateGeneral (nullptr)
    , mStateName    (nullptr)
    , mStateKind    (nullptr)
    , mStateDesc    (nullptr)
    , mEnterOps     (nullptr)
    , mExitOps      (nullptr)
    , mDoOps        (nullptr)
    , mDoInterval   (nullptr)
    , mDoUntil      (nullptr)
    , mTransitions  (nullptr)
    , mTransGeneral (nullptr)
    , mStimulusSig  (nullptr)
    , mStimulusName (nullptr)
    , mTarget       (nullptr)
    , mSource       (nullptr)
    , mTransDesc    (nullptr)
    , mConditions   (nullptr)
    , mTransOps     (nullptr)
    , mTransTabs    (nullptr)
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
    // A newly added trigger method / event / timer expands the stimulus vocabulary; if a
    // transition is on screen, rebuild its stimulus picker so the new stimulus is selectable
    // immediately (the added element's id is never mCurrentId, so onElementChanged misses it).
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) {
        if ((mPage == PageTransition) && (isEditing() == false)
            && ((kind == eDocElementKind::Method) || (kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)))
        {
            refresh();
        }
    });
    connect(&notifier, &DocModelNotifier::nameChanged, this, &SMPropertiesPanel::onNameChanged);
    connect(&notifier, &DocModelNotifier::listReordered, this, &SMPropertiesPanel::onListReordered);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMPropertiesPanel::onDocumentReloaded);
    connect(&mModel, &StateMachineModel::signalStateNamePreview, this, &SMPropertiesPanel::onStateNamePreview);

    refresh();
}

SMPropertiesPanel::~SMPropertiesPanel()
{
    mModel.getNotifier().disconnect(this);
    mModel.getSelectionModel().disconnect(this);
}

void SMPropertiesPanel::buildStatePage()
{
    mStateName = new QLineEdit(this);
    mStateName->setMaxLength(StateMachineData::MAX_IDENTIFIER_LENGTH);
    // State names must be enum-friendly identifiers: reject spaces and other invalid symbols
    // as the user types, the same rule the canvas in-place editor enforces.
    mStateName->setValidator(new QRegularExpressionValidator(QRegularExpression(StateMachineData::identifierPattern()), mStateName));
    mStateKind = new QLabel(this);
    mTransitions = new ReorderList(this);
    mStateDesc = new QPlainTextEdit(this);

    // The transitions list stays compact; the multi-line description takes the room below it.
    mTransitions->setMaximumHeight(120);
    mStateDesc->setPlaceholderText(tr("Description"));
    mStateDesc->installEventFilter(this);   // commit on focus-out (no editingFinished signal)

    // General tab (R21): the scalar fields and the transitions list are two accordion sections under
    // the shared chrome, so this tab wears the same header/section/compact language as Conditions.
    // Compact defaults UNCHECKED here (few sections): the details and the transitions read together.
    QWidget* details = new QWidget(this);
    QFormLayout* form = new QFormLayout(details);
    form->setContentsMargins(6, 6, 6, 6);
    form->addRow(tr("Name:"), mStateName);
    form->addRow(tr("Kind:"), mStateKind);
    form->addRow(tr("Description:"), mStateDesc);

    mStateGeneral = new SMSectionChrome(this);
    mStateGeneral->setTitle(tr("State"));
    mStateGeneral->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionDetails), tr("Details"), details
                             , tr("The state name, kind and description"));
    mStateGeneral->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionList), tr("Transitions"), mTransitions
                             , tr("The transitions leaving this state"));
    mStateGeneral->setCompact(false);
    // Both sections start OPEN: selecting a state must land on an editable name, a readable kind and
    // the description without a click, which is what a General tab is for. Sections are still
    // collapsible by hand -- only the initial state changed.
    mStateGeneral->openAllSections();
    mStateGeneral->addFooterStretch();

    connect(mStateName, &QLineEdit::editingFinished, this, &SMPropertiesPanel::onStateNameCommit);
    // Real-time mirror onto the canvas box while the user types here (no model change yet).
    connect(mStateName, &QLineEdit::textEdited, this, [this](const QString& text)
    {
        if ((mUpdating == false) && (mPage == PageState) && (mCurrentId != 0))
        {
            mModel.publishStateNamePreview(mCurrentId, text);
        }
    });
    connect(mTransitions, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { onTransitionActivated(); });

    static_cast<ReorderList*>(mTransitions)->mOnReorder = [this](int from, int to)
    {
        // Defer: the drop is being handled by the list; reorder + repopulate on the next turn.
        QTimer::singleShot(0, this, [this, from, to]() { reorderTransition(from, to); });
    };

    // Actions (R22/R24, redesigned): one tab per state activity -- Enter, Do, Exit -- instead of a
    // single crowded accordion, so the panel stays navigable when every part is open. Each tab hosts
    // the shared SMOperationsEditor, whose Action/Event/Timer accordion is identical in every scope
    // (the reuse the redesign asked for). Enter and Exit are symmetric; the Do activity is not --
    // besides its operation list it carries a repeat interval (0 = trigger-driven, >0 = a timer loop)
    // and an optional stop-condition, so its tab page is built by hand with those two fields above the
    // editor. Each tab's tooltip carries the per-list summary the old section headers used to show.
    mEnterOps = new SMOperationsEditor(mModel, this);
    mDoOps    = new SMOperationsEditor(mModel, this);
    mExitOps  = new SMOperationsEditor(mModel, this);

    // The Do repeat policy is its own collapsible `Repeat` section appended to the Do editor's
    // accordion, under the same expand/collapse toolbar, so the interval and stop-condition sit
    // beside the Action/Event/Timers sections instead of floating in a form above them. The circular
    // repeat glyph (SectionDo) marks it.
    mDoInterval = new QSpinBox(this);
    mDoInterval->setRange(0, 3600000);
    mDoInterval->setSingleStep(50);
    mDoInterval->setSuffix(tr(" ms"));
    mDoInterval->setToolTip(tr("Repeat interval; 0 runs the Do actions on each trigger while in the state"));
    mDoUntil = new QLineEdit(this);
    mDoUntil->setPlaceholderText(tr("Stop condition (optional)"));
    mDoUntil->setToolTip(tr("When this expression holds the repetition stops without leaving the state"));

    QWidget* repeatBody = new QWidget(this);
    QFormLayout* repeatForm = new QFormLayout(repeatBody);
    repeatForm->setContentsMargins(6, 6, 6, 6);
    repeatForm->addRow(tr("Repeat every:"), mDoInterval);
    repeatForm->addRow(tr("Until:"), mDoUntil);
    mDoOps->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionDo), tr("Repeat"), repeatBody);

    connect(mDoInterval, &QSpinBox::editingFinished, this, &SMPropertiesPanel::onDoIntervalCommit);
    connect(mDoUntil, &QLineEdit::editingFinished, this, &SMPropertiesPanel::onDoUntilCommit);

    mStateTabs = new QTabWidget(this);
    mStateTabs->setObjectName(QStringLiteral("smStateTabs"));
    mStateGeneral->setObjectName(QStringLiteral("smStateGeneral"));
    mStateTabs->addTab(mStateGeneral, tr("General"));
    const int enterTab = mStateTabs->addTab(mEnterOps, tr("Enter"));
    const int doTab    = mStateTabs->addTab(mDoOps, tr("Do"));
    const int exitTab  = mStateTabs->addTab(mExitOps, tr("Exit"));
    mActionSlots.append({ eOpList::Entry, mEnterOps, enterTab });
    mActionSlots.append({ eOpList::Do,    mDoOps,    doTab });
    mActionSlots.append({ eOpList::Exit,  mExitOps,  exitTab });

    mStack->insertWidget(PageState, mStateTabs);
}

void SMPropertiesPanel::buildTransitionPage()
{
    // The General tab wears the shared chrome (R21): the trigger form and the description are two
    // accordion sections, so this tab matches Conditions' header/section/compact language. The
    // stimulus/target accessors keep pointing at the same widgets. Compact defaults UNCHECKED here.
    QWidget* trigger = new QWidget(this);
    QFormLayout* form = new QFormLayout(trigger);
    form->setContentsMargins(6, 6, 6, 6);

    // One picker over the whole stimulus vocabulary (triggers, events, timers). The kind is
    // encoded per row (and by the on_event_/on_timer_ prefix), so a separate "kind" combo is
    // redundant. The picker is read-only (a closed list, like the Actions tab): the user cannot
    // type a free name; typing a letter jumps to the matching row (Qt's built-in type-ahead).
    mStimulusName = new QComboBox(trigger);
    mStimulusName->setEditable(false);

    mTarget = new QComboBox(trigger);
    mTarget->setEditable(false);

    mSource = new QComboBox(trigger);
    mSource->setEditable(false);

    mStimulusSig = new QLabel(trigger);
    mStimulusSig->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mStimulusSig->setEnabled(false);

    mTransDesc = new QPlainTextEdit(this);
    mTransDesc->setPlaceholderText(tr("Description"));
    mTransDesc->installEventFilter(this);   // commit on focus-out (no editingFinished signal)

    form->addRow(tr("Stimulus:"), mStimulusName);
    form->addRow(tr("Signature:"), mStimulusSig);
    form->addRow(tr("Source:"), mSource);
    form->addRow(tr("Target:"), mTarget);

    mTransGeneral = new SMSectionChrome(this);
    mTransGeneral->setTitle(tr("Transition"));
    mTransGeneral->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionDetails), tr("Trigger"), trigger
                             , tr("The stimulus, its signature and the target state"));
    mTransGeneral->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionText), tr("Description"), mTransDesc
                             , tr("A free-text note on this transition"));
    mTransGeneral->setCompact(false);
    mTransGeneral->openAllSections();   // same as the state General tab: open and editable at once
    mTransGeneral->addFooterStretch();

    connect(mStimulusName, &QComboBox::activated, this, &SMPropertiesPanel::onStimulusCommit);
    connect(mTarget, &QComboBox::activated, this, &SMPropertiesPanel::onTargetCommit);
    connect(mSource, &QComboBox::activated, this, &SMPropertiesPanel::onSourceCommit);

    mTransTabs = new QTabWidget(this);
    mTransTabs->setObjectName(QStringLiteral("smTransTabs"));
    mTransTabs->addTab(mTransGeneral, tr("General"));
    mConditions = new SMGuardBar(mModel, this);
    mTransTabs->addTab(mConditions, tr("Conditions"));
    connect(mConditions, &SMGuardBar::badgeChanged, this, &SMPropertiesPanel::onGuardBadgeChanged);

    mTransOps = new SMOperationsEditor(mModel, this);
    mTransTabs->addTab(mTransOps, tr("Actions"));

    mStack->insertWidget(PageTransition, mTransTabs);
}

void SMPropertiesPanel::onGuardBadgeChanged(bool isDraft, bool hasWarnings)
{
    if (mTransTabs == nullptr)
    {
        return;
    }

    // The Conditions tab is the second page; a draft adds `*`, a warning adds the `(!)` glyph.
    QString label = tr("Conditions");
    if (isDraft)
    {
        label += QStringLiteral(" *");
    }

    if (hasWarnings)
    {
        label += QStringLiteral(" (!)");
    }

    mTransTabs->setTabText(1, label);
}

void SMPropertiesPanel::onStateNamePreview(uint32_t stateId, const QString& text)
{
    if ((mPage != PageState) || (mCurrentId != stateId) || (mStateName == nullptr) || mStateName->hasFocus())
    {
        return;
    }

    const QSignalBlocker block(mStateName);
    mStateName->setText(text);
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
    // Use the application's ACTIVE focus, not QWidget::focusWidget(): the latter returns the
    // last-focused DESCENDANT and stays non-null once any field here was clicked, so it reported
    // "editing" permanently and blocked the live stimulus-picker refresh after the first click.
    QWidget* focus = QApplication::focusWidget();
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

void SMPropertiesPanel::focusConditions(uint32_t transitionId)
{
    if (mModel.getData().findTransitionById(transitionId) == nullptr)
    {
        return;
    }

    // Selecting refreshes synchronously to the transition page; then land on Conditions.
    mModel.getSelectionModel().setSelection({ transitionId });
    if (mPage == PageTransition)
    {
        mTransTabs->setCurrentIndex(1);
        if (mConditions->field() != nullptr)
        {
            mConditions->field()->setFocus();
        }
    }
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
    // Entry/exit operations have no transition scope, so the Param source is not offered here. The
    // Actions sections are bound from the slot table, so a `Do` list joins by adding one slot.
    SMStateEntry* mutableState = mModel.getData().findStateById(stateId);
    for (const ActionSlot& slot : mActionSlots)
    {
        SMOperationList* list = nullptr;
        switch (slot.role)
        {
        case eOpList::Entry:    list = &mutableState->getEntryList();  break;
        case eOpList::Do:       list = &mutableState->getDoList();     break;
        case eOpList::Exit:     list = &mutableState->getExitList();   break;
        }
        slot.editor->bind(stateId, eDocElementKind::State, 0u, mutableState, list);
    }
    mDoInterval->setValue(static_cast<int>(state->getDoInterval()));
    mDoUntil->setText(state->getDoUntil());
    refreshActionSummaries();
    populateTransitionList(stateId);

    mStack->setCurrentIndex(PageState);
    mUpdating = false;
}

void SMPropertiesPanel::refreshActionSummaries()
{
    const SMStateEntry* state = mModel.getData().findStateById(mCurrentId);
    if ((state == nullptr) || (mStateTabs == nullptr))
    {
        return;
    }

    for (const ActionSlot& slot : mActionSlots)
    {
        // The tab tooltip carries the summary the old collapsed section headers used to show:
        // `On Enter: doWork(), send evGo` or `On Enter: not set`, so hovering answers "what happens
        // around this state?" without switching tabs. The Do tooltip also folds in its repeat policy
        // -- `Do (every 200 ms)` or `Do (on trigger)`.
        const SMOperationList* list = nullptr;
        QString title;
        switch (slot.role)
        {
        case eOpList::Entry:
            list = &state->getEntryList();
            title = tr("On Enter");
            break;
        case eOpList::Do:
            list = &state->getDoList();
            title = list->isEmpty() ? tr("Do")
                  : (state->getDoInterval() > 0u ? tr("Do (every %1 ms)").arg(state->getDoInterval())
                                                 : tr("Do (on trigger)"));
            break;
        case eOpList::Exit:
            list = &state->getExitList();
            title = tr("On Exit");
            break;
        }

        mStateTabs->setTabToolTip(slot.tabIndex, title + QStringLiteral(": ") + operationsSummary(mModel.getData(), *list));
    }
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
                ? (stimulus + QStringLiteral(" -> ") + transition->getTargetName())
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
    const int stimRow = mStimulusName->findText(currentLabel, Qt::MatchFixedString);
    mStimulusName->setCurrentIndex(stimRow >= 0 ? stimRow : 0);
    mStimulusSig->setText(SMOperationSummary::stimulusSignature(data, *transition));

    // Populate the target and source pickers from the sibling states of the transition's level.
    // Each item carries the sibling's element ID as its data, so the endpoint is committed by ID --
    // robust even when several states share a display name (Start/Final).
    //
    // Spec rule (mirrors the canvas endpoint-drag guard): a Start state has no incoming transition,
    // so it is never offered as a Target; a Final state has no outgoing transition, so it is never
    // offered as a Source. Omitting them from the lists is the primary enforcement; the commit slots
    // add a backstop in case a stale selection slips through.
    mTarget->clear();
    mTarget->addItem(internalLabel(), 0u);
    mSource->clear();
    const SMStateEntry* owner = data.findTransitionOwner(transitionId);
    const uint32_t sourceId = (owner != nullptr ? owner->getId() : 0u);
    const SMStateData* level = data.findLevel(mModel.getSelectionModel().getActiveLevel());
    if (level != nullptr)
    {
        for (SMStateEntry* sibling : level->getElements())
        {
            if (sibling == nullptr)
            {
                continue;
            }

            if (sibling->getKind() != SMStateEntry::eStateKind::Start)
            {
                mTarget->addItem(sibling->getName(), sibling->getId());
            }

            if (sibling->getKind() != SMStateEntry::eStateKind::Final)
            {
                mSource->addItem(sibling->getName(), sibling->getId());
            }
        }
    }

    const int targetRow = mTarget->findData(transition->getToId());
    mTarget->setCurrentIndex(targetRow >= 0 ? targetRow : 0);
    const int sourceRow = mSource->findData(sourceId);
    mSource->setCurrentIndex(sourceRow >= 0 ? sourceRow : -1);
    mTransDesc->setPlainText(transition->getDescription());

    mConditions->setTransition(transitionId);
    // A transition operation may map the stimulus parameters, so it is its own Param scope.
    SMTransitionEntry* mutableTransition = data.findTransitionById(transitionId);
    mTransOps->bind(transitionId, eDocElementKind::Transition, transitionId, mutableTransition, &mutableTransition->getOperations());

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
        mModel.publishStateNamePreview(mCurrentId, state->getName());   // restore the mirrored canvas box name
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

void SMPropertiesPanel::onDoIntervalCommit()
{
    if (mUpdating || (mPage != PageState))
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMStateEntry* state = data.findStateById(mCurrentId);
    const uint32_t value = static_cast<uint32_t>(mDoInterval->value());
    if ((state == nullptr) || (value == state->getDoInterval()))
    {
        return;
    }

    const uint32_t id = mCurrentId;
    StateMachineData* doc = &data;
    auto getter = [doc, id]() -> uint32_t { SMStateEntry* e = doc->findStateById(id); return (e != nullptr ? e->getDoInterval() : 0u); };
    auto setter = [doc, id](const uint32_t& v) { SMStateEntry* e = doc->findStateById(id); if (e != nullptr) e->setDoInterval(v); };
    mModel.getUndoStack().push(new TDocSetPropertyCommand<uint32_t>(mModel.getNotifier(), id, eDocElementKind::State, getter, setter, value, tr("Set Do interval")));
}

void SMPropertiesPanel::onDoUntilCommit()
{
    if (mUpdating || (mPage != PageState))
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMStateEntry* state = data.findStateById(mCurrentId);
    if ((state == nullptr) || (mDoUntil->text() == state->getDoUntil()))
    {
        return;
    }

    const uint32_t id = mCurrentId;
    StateMachineData* doc = &data;
    auto getter = [doc, id]() -> QString { SMStateEntry* e = doc->findStateById(id); return (e != nullptr ? e->getDoUntil() : QString()); };
    auto setter = [doc, id](const QString& v) { SMStateEntry* e = doc->findStateById(id); if (e != nullptr) e->setDoUntil(v); };
    mModel.getUndoStack().push(new TDocSetPropertyCommand<QString>(mModel.getNotifier(), id, eDocElementKind::State, getter, setter, mDoUntil->text(), tr("Set Do stop condition")));
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

    // Row 0 detaches the stimulus (an internal/initial transition may have none).
    mStimulusName->addItem(tr("(none)"));
    mStimulusName->setItemData(0, static_cast<int>(SMTransitionEntry::eStimulusKind::Trigger), RoleStimulusKind);
    mStimulusName->setItemData(0, QString(), RoleStimulusName);

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

    return currentName.isEmpty()
            ? tr("(none)")
            : stimulusDisplayLabel(static_cast<SMTransitionEntry::eStimulusKind>(currentKind), currentName);
}

void SMPropertiesPanel::applyStimulus()
{
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mCurrentId);
    if (transition == nullptr)
    {
        return;
    }

    // The picker is a closed list (row 0 = "(none)"); read the selected row's real kind + name.
    const int row = mStimulusName->currentIndex();
    if (row < 0)
    {
        return;
    }

    const SMTransitionEntry::eStimulusKind kind =
            static_cast<SMTransitionEntry::eStimulusKind>(mStimulusName->itemData(row, RoleStimulusKind).toInt());
    const QString name = mStimulusName->itemData(row, RoleStimulusName).toString();

    // "(none)" (empty name) detaches the stimulus; the picker never renames or creates a registry
    // entry.
    if (name.isEmpty())
    {
        if (transition->getStimulus().isEmpty() == false)
        {
            mModel.getUndoStack().push(new SMSetStimulusCommand(data, mModel.getNotifier(), mCurrentId, transition->getStimulusKind(), QString(), tr("Clear stimulus")));
        }

        return;
    }

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

    // The picker is a closed list; each row carries its state's element ID (0 = internal).
    const uint32_t targetId = mTarget->currentData().toUInt();
    if (targetId == 0)
    {
        if (transition->isExternal())
        {
            mModel.getUndoStack().push(new SMSetTransitionTargetCommand(data, mModel.getNotifier(), mCurrentId, 0u, tr("Make internal")));
        }

        return;
    }

    // Backstop for the spec rule: a Start state has no incoming transition. The picker already omits
    // Start rows, so this only fires if a stale/programmatic selection slips through.
    const SMStateEntry* targetState = data.findStateById(targetId);
    if ((targetState != nullptr) && (targetState->getKind() == SMStateEntry::eStateKind::Start))
    {
        return;
    }

    if (targetId != transition->getToId())
    {
        mModel.getUndoStack().push(new SMSetTransitionTargetCommand(data, mModel.getNotifier(), mCurrentId, targetId, tr("Set target")));
    }
}

void SMPropertiesPanel::onSourceCommit()
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

    // The picker is a closed list; each row carries its state's element ID. Reparenting the
    // transition to a new owner state changes its begin endpoint (source).
    const uint32_t newSourceId = mSource->currentData().toUInt();
    SMStateEntry* newSource = data.findStateById(newSourceId);
    SMStateEntry* oldSource = data.findTransitionOwner(mCurrentId);
    if ((newSource == nullptr) || (oldSource == nullptr) || (newSource == oldSource))
    {
        return;
    }

    // Backstop for the spec rule: a Final state has no outgoing transition. The picker already omits
    // Final rows, so this only fires if a stale/programmatic selection slips through.
    if (newSource->getKind() == SMStateEntry::eStateKind::Final)
    {
        return;
    }

    mModel.getUndoStack().push(new SMReparentTransitionCommand(data, mModel.getNotifier(), *oldSource, *newSource, mCurrentId, tr("Set source")));
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

void SMPropertiesPanel::onElementChanged(uint32_t id, eDocElementKind kind)
{
    // The State-Actions headers summarize the live entry/exit lists; re-label them even mid-edit --
    // this only re-titles the collapsed headers, it never rebinds the editors, so it cannot clobber
    // typing. Any operation edit fires elementChanged, and the summary re-reads the state's lists.
    if (mPage == PageState)
    {
        refreshActionSummaries();
    }

    if (isEditing())
    {
        return;
    }

    if (id == mCurrentId)
    {
        refresh();
    }
    else if ((mPage == PageTransition)
             && ((kind == eDocElementKind::Method) || (kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)))
    {
        // A trigger method changing type (trigger <-> action/condition), or an event/timer edit,
        // changes the stimulus vocabulary; the changed element's id is never mCurrentId, so rebuild
        // the transition page (and its picker) here so the Stimulus combo always reflects the
        // current triggers (live sync). A rename already routes through onNameChanged.
        refresh();
    }
}

void SMPropertiesPanel::onElementRemoved(uint32_t id, eDocElementKind kind)
{
    if (id == mCurrentId)
    {
        showEmpty();
    }
    else if ((mPage == PageTransition) && (isEditing() == false) && (kind == eDocElementKind::Method))
    {
        // Removing a parameter shortens the trigger stimulus signature shown in the General/Trigger
        // section; the notifier carries the parameter's id (never mCurrentId), so refresh the page.
        refresh();
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
    else if ((mPage == PageTransition) && (isEditing() == false) && (kind == eDocElementKind::Method))
    {
        // Reordering a method's parameters reorders the trigger stimulus signature in the
        // General/Trigger section; the owner id is the method's, not mCurrentId, so refresh the page.
        refresh();
    }
}

void SMPropertiesPanel::onDocumentReloaded()
{
    showEmpty();
}
