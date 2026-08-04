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
#include "lusan/model/sm/SMDocumentIndex.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMOperationSummary.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMAccordion.hpp"
#include "lusan/view/sm/SMGuardBar.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMGuardStatusLine.hpp"
#include "lusan/view/sm/SMKindGlyph.hpp"
#include "lusan/view/sm/SMOperationsEditor.hpp"
#include "lusan/view/sm/SMInternalEditor.hpp"
#include "lusan/view/sm/SMSectionChrome.hpp"
#include "lusan/view/sm/SMStimulusPicker.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QCompleter>
#include <QDropEvent>
#include <QEvent>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <functional>

namespace
{
    //!< The transition ID carried by each row of the state page's transition list.
    constexpr int RoleTransitionId { Qt::UserRole + 1 };

    //!< Greys one row of a closed picker out instead of removing it, so the vocabulary a transition
    //!< kind has stays visible and only what THIS owner can hold is selectable.
    void setComboRowEnabled(QComboBox& combo, int row, bool enabled)
    {
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(combo.model());
        QStandardItem* item = (model != nullptr ? model->item(row) : nullptr);
        if (item != nullptr)
        {
            item->setEnabled(enabled);
        }
    }

    //!< The label shown for an internal transition, which has no target by design.
    QString internalLabel()
    {
        return QObject::tr("(internal)");
    }

    //!< The label for a transition that should have a target and does not, an edge the author
    //!< started drawing. Deliberately distinct from `(internal)`, which used to read the same.
    QString unconnectedLabel()
    {
        return QObject::tr("(not connected)");
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
    , mStateForm    (nullptr)
    , mBtnEnterSubmachine(nullptr)
    , mBtnGoToParent(nullptr)
    , mBtnAddSubmachine(nullptr)
    , mBtnRemoveSubmachine(nullptr)
    , mStateName    (nullptr)
    , mStateKind    (nullptr)
    , mStateHistory (nullptr)
    , mStateSubmachine(nullptr)
    , mStateOnFinal (nullptr)
    , mStateDesc    (nullptr)
    , mEnterOps     (nullptr)
    , mExitOps      (nullptr)
    , mDoOps        (nullptr)
    , mDoInterval   (nullptr)
    , mDoUntil      (nullptr)
    , mDoUntilStatus(nullptr)
    , mTransitions  (nullptr)
    , mInternal     (nullptr)
    , mInternalTab  (-1)
    , mTransGeneral (nullptr)
    , mStimulusSig  (nullptr)
    , mStimulusName (nullptr)
    , mTransForm    (nullptr)
    , mTransKind    (nullptr)
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
    // The panel states its own minimum (minimumSizeHint below); the layout must not overwrite it
    // with the sum of everything the pages contain, or the dock is back to being content-driven.
    layout->setSizeConstraint(QLayout::SetNoConstraint);
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

    // The name and the two description boxes carry document text. Typing in them marks the document
    // changed at once, even though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mStateName, notifier);
    PendingEditWatcher::watchField(mStateDesc, notifier);
    PendingEditWatcher::watchField(mTransDesc, notifier);

    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMPropertiesPanel::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMPropertiesPanel::onElementRemoved);
    // A newly added trigger, event or timer expands the stimulus vocabulary. Its id is never
    // mCurrentId, so rebuild the picker here to make the new stimulus selectable at once.
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) {
        if (isEditing())
        {
            return;
        }

        if ((mPage == PageTransition)
            && ((kind == eDocElementKind::Method) || (kind == eDocElementKind::Event) || (kind == eDocElementKind::Timer)))
        {
            refresh();
        }
        else if ((mPage == PageState) && (kind == eDocElementKind::Import))
        {
            // A machine registered on the Includes page has to be selectable here at once;
            // otherwise the Add Submachine button looks like it did nothing.
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

QSize SMPropertiesPanel::minimumSizeHint() const
{
    const QSize base = QWidget::minimumSizeHint();
    return QSize(qMin(base.width(), NESMDesign::PanelMinWidth), base.height());
}

void SMPropertiesPanel::buildStatePage()
{
    mStateName = new QLineEdit(this);
    mStateName->setMaxLength(StateMachineData::MAX_IDENTIFIER_LENGTH);
    mStateName->setValidator(new QRegularExpressionValidator(QRegularExpression(StateMachineData::identifierPattern()), mStateName));
    mStateKind = new QLabel(this);
    mStateHistory = new QComboBox(this);
    mStateHistory->setObjectName(QStringLiteral("smStateHistory"));
    mStateHistory->addItem(tr("None"), static_cast<int>(SMStateEntry::eHistory::None));
    mStateHistory->addItem(tr("Shallow"), static_cast<int>(SMStateEntry::eHistory::Shallow));
    mStateHistory->addItem(tr("Deep"), static_cast<int>(SMStateEntry::eHistory::Deep));
    mStateHistory->setItemData(1, tr("Coming back activates the substate that was active last time"), Qt::ToolTipRole);
    mStateHistory->setItemData(2, tr("Coming back restores the whole path that was active last time, down to the leaf"), Qt::ToolTipRole);
    mStateSubmachine = new QComboBox(this);
    mStateSubmachine->setObjectName(QStringLiteral("smStateSubmachine"));
    mStateOnFinal = new QComboBox(this);
    mStateOnFinal->setObjectName(QStringLiteral("smStateOnFinal"));
    mTransitions = new ReorderList(this);
    mStateDesc = new QPlainTextEdit(this);

    // The transitions list stays compact; the multi-line description takes the room below it.
    mTransitions->setMaximumHeight(120);
    mStateDesc->setObjectName(QStringLiteral("smStateDescription"));
    mStateDesc->setPlaceholderText(tr("Description"));
    mStateDesc->installEventFilter(this);   // commit on focus-out (no editingFinished signal)

    // General tab: the scalar fields and the transitions list are two accordion sections under the
    // shared chrome. Compact defaults unchecked, so the details and the transitions read together.
    QWidget* details = new QWidget(this);
    QFormLayout* form = new QFormLayout(details);
    mStateForm = form;
    form->setContentsMargins(6, 6, 6, 6);
    form->addRow(tr("Name:"), mStateName);
    form->addRow(tr("Kind:"), mStateKind);
    form->addRow(tr("Submachine:"), mStateSubmachine);
    form->addRow(tr("On Final:"), mStateOnFinal);
    form->addRow(tr("History:"), mStateHistory);
    form->addRow(tr("Description:"), mStateDesc);

    mStateGeneral = new SMSectionChrome(this);
    mStateGeneral->setTitle(tr("State"));
    mStateGeneral->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionDetails), tr("Details"), details
                             , tr("The state name, kind and description"));
    mStateGeneral->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionList), tr("Transitions"), mTransitions
                             , tr("The transitions leaving this state"));
    // The submachine controls, put where the user already is when they decide a state needs one. A
    // separator keeps them from reading as more section-jump buttons.
    QFrame* submachineSep = new QFrame(this);
    submachineSep->setFrameShape(QFrame::VLine);
    submachineSep->setMaximumSize(12, 20);
    const auto makeSubmachineButton = [this](const QString& name) -> QToolButton*
    {
        QToolButton* button = new QToolButton(this);
        button->setObjectName(name);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        button->setCursor(Qt::PointingHandCursor);
        return button;
    };

    // Icon-only, like every other button in this header: a label such as "Open Imported Machine"
    // is wider than half the dock. The tooltip spells the current meaning out.
    mBtnEnterSubmachine  = makeSubmachineButton(QStringLiteral("smBtnEnterSubmachine"));
    mBtnGoToParent       = makeSubmachineButton(QStringLiteral("smBtnGoToParent"));
    mBtnAddSubmachine    = makeSubmachineButton(QStringLiteral("smBtnAddSubmachine"));
    mBtnRemoveSubmachine = makeSubmachineButton(QStringLiteral("smBtnRemoveSubmachine"));
    mStateGeneral->addHeaderWidget(submachineSep);
    mStateGeneral->addHeaderWidget(mBtnEnterSubmachine);
    mStateGeneral->addHeaderWidget(mBtnGoToParent);
    mStateGeneral->addHeaderWidget(mBtnAddSubmachine);
    mStateGeneral->addHeaderWidget(mBtnRemoveSubmachine);

    mStateGeneral->setCompact(false);
    // Both sections start open, so selecting a state lands on an editable name, a readable kind and
    // the description without a click. They stay collapsible by hand.
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
    connect(mStateHistory, &QComboBox::activated, this, &SMPropertiesPanel::onStateHistoryCommit);
    connect(mStateSubmachine, &QComboBox::activated, this, &SMPropertiesPanel::onStateSubmachineCommit);
    connect(mStateOnFinal, &QComboBox::activated, this, &SMPropertiesPanel::onStateOnFinalCommit);
    connect(mTransitions, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) { onTransitionActivated(); });

    static_cast<ReorderList*>(mTransitions)->mOnReorder = [this](int from, int to)
    {
        // Defer: the drop is being handled by the list; reorder + repopulate on the next turn.
        QTimer::singleShot(0, this, [this, from, to]() { reorderTransition(from, to); });
    };

    // One tab per state activity, Enter, Do and Exit, each hosting the shared SMOperationsEditor.
    // The Do page is built by hand, since it adds a repeat interval and a stop condition.
    mEnterOps = new SMOperationsEditor(mModel, this);
    mDoOps    = new SMOperationsEditor(mModel, this);
    mExitOps  = new SMOperationsEditor(mModel, this);

    // The Do repeat policy is its own collapsible `Repeat` section appended to the Do editor's
    // accordion, so the interval and stop condition sit beside the other sections.
    mDoInterval = new QSpinBox(this);
    // The minimum is 1, not 0: a Do activity is a timer loop and a loop has no zero period.
    // Reacting to one named stimulus without leaving the state is an internal transition instead.
    mDoInterval->setRange(static_cast<int>(SMStateEntry::MIN_DO_INTERVAL), 3600000);
    mDoInterval->setSingleStep(50);
    mDoInterval->setSuffix(tr(" ms"));
    mDoInterval->setToolTip(tr("How often the Do operations run while the state is active. "
                               "The first tick is one interval after entry, never at entry -- "
                               "work that must happen on arrival belongs in Enter."));

    // The stop condition is the same editing surface a transition guard uses, pointed at this
    // state's activity: one grammar, one completer, and a rename re-renders instead of breaking it.
    mDoUntil = new SMGuardField(mModel, this);
    // The panel hosts two guard surfaces that would otherwise share one object name. Naming each
    // for what it edits keeps every by-name lookup unambiguous.
    mDoUntil->setObjectName(QStringLiteral("smDoUntilField"));
    mDoUntil->setHeightLines(1, 4);
    mDoUntil->setPlaceholderText(tr("Stop condition (optional)"));
    mDoUntil->setToolTip(tr("Tested BEFORE each tick. Once it holds, the activity stops for the rest "
                            "of the visit -- only a fresh entry to the state starts it again."));
    mDoUntilStatus = new SMGuardStatusLine(this);
    mDoUntilStatus->setObjectName(QStringLiteral("smDoUntilStatusLine"));
    // The verdict LABEL inside it carries the name the lookups actually reach for, so it is the
    // one that has to differ; the line itself is renamed above only so the pair reads as a pair.
    if (QLabel* verdict = mDoUntilStatus->findChild<QLabel*>(QStringLiteral("smGuardStatus")))
    {
        verdict->setObjectName(QStringLiteral("smDoUntilStatus"));
    }

    QWidget* repeatBody = new QWidget(this);
    QFormLayout* repeatForm = new QFormLayout(repeatBody);
    repeatForm->setContentsMargins(6, 6, 6, 6);
    repeatForm->addRow(tr("Repeat every:"), mDoInterval);
    repeatForm->addRow(tr("Until:"), mDoUntil);
    repeatForm->addRow(QString(), mDoUntilStatus);
    mDoOps->addSection(SMToolIcons::icon(SMToolIcons::eIcon::SectionDo), tr("Repeat"), repeatBody);

    connect(mDoInterval, &QSpinBox::editingFinished, this, &SMPropertiesPanel::onDoIntervalCommit);
    // The field commits itself (Enter / focus-out) through SMGuardCommands, so there is no
    // commit slot here -- only the verdict to relay, exactly as the Conditions tab does.
    connect(mDoUntil, &SMGuardField::statusUpdated, this
           , [this](int severity, const QString& verdict, const QString& preview, const QStringList& chips)
    {
        if (verdict.isEmpty())
        {
            mDoUntilStatus->clearStatus();
        }
        else
        {
            mDoUntilStatus->setStatus(static_cast<NEGuardStyle::eSeverity>(severity), verdict, preview, chips);
        }
    });
    connect(mDoUntil, &SMGuardField::signalNavigateToDefinition, this, &SMPropertiesPanel::signalNavigateToDefinition);

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

    buildInternalTab();

    mStack->insertWidget(PageState, mStateTabs);
}

void SMPropertiesPanel::buildInternalTab()
{
    // The tab hosts the shared editor, not a copy: the canvas context menu opens the same widget in
    // SMInternalDialog. One implementation, one undo path, and the two access paths cannot drift.
    mInternal = new SMInternalEditor(mModel, this);
    connect(mInternal, &SMInternalEditor::countChanged, this, &SMPropertiesPanel::onInternalCountChanged);
    connect(mInternal, &SMInternalEditor::signalNavigateToDefinition, this, &SMPropertiesPanel::signalNavigateToDefinition);

    mInternalTab = mStateTabs->addTab(mInternal, tr("Internal"));
    mStateTabs->setTabToolTip(mInternalTab, tr("The transitions this state takes without leaving itself"));
}

void SMPropertiesPanel::onInternalCountChanged(int count)
{
    // The tab itself carries the count. A tab reading `Internal (2)` is the discoverability the
    // construct never had: the only route used to be a double-click on a row of a collapsible list.
    if (mInternalTab >= 0)
    {
        mStateTabs->setTabText(mInternalTab, (count > 0) ? tr("Internal (%1)").arg(count) : tr("Internal"));
    }
}

void SMPropertiesPanel::bindSubmachineActions(QAction* enterOrAdd, QAction* goToParent, QAction* addSubmachine, QAction* removeSubmachine)
{
    // setDefaultAction makes each button mirror its action's text, icon, tooltip and enabled
    // state, which is how the first button's label follows the selection for free.
    mBtnEnterSubmachine->setDefaultAction(enterOrAdd);
    mBtnGoToParent->setDefaultAction(goToParent);
    mBtnAddSubmachine->setDefaultAction(addSubmachine);
    mBtnRemoveSubmachine->setDefaultAction(removeSubmachine);
}

void SMPropertiesPanel::buildTransitionPage()
{
    // The General tab wears the shared chrome: the trigger form and the description are two
    // accordion sections. Compact defaults unchecked here.
    QWidget* trigger = new QWidget(this);
    QFormLayout* form = new QFormLayout(trigger);
    form->setContentsMargins(6, 6, 6, 6);

    // One picker over the whole stimulus vocabulary of triggers, events and timers; the kind is
    // encoded per row. It is read-only, so typing a letter jumps to the matching row.
    mStimulusName = new QComboBox(trigger);
    mStimulusName->setEditable(false);

    // What the transition is, said outright. It sits above the endpoints because it decides what
    // they may say: Internal has no target and Initial has no stimulus.
    mTransKind = new QComboBox(trigger);
    mTransKind->setEditable(false);
    mTransKind->addItem(tr("External (leaves the state)"), static_cast<int>(SMTransitionEntry::eTransitionKind::External));
    mTransKind->setItemData(0, tr("Moves the machine to the target state on its stimulus."), Qt::ToolTipRole);
    mTransKind->addItem(tr("Internal (stays in the state)"), static_cast<int>(SMTransitionEntry::eTransitionKind::Internal));
    mTransKind->setItemData(1, tr("Runs its operations on its stimulus without leaving the state, so no entry or exit action runs."), Qt::ToolTipRole);
    mTransKind->addItem(tr("Initial (entering the level)"), static_cast<int>(SMTransitionEntry::eTransitionKind::Initial));
    mTransKind->setItemData(2, tr("Taken as the machine enters this level. Nothing fires it; only a Start state has one."), Qt::ToolTipRole);

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

    form->addRow(tr("Kind:"), mTransKind);
    form->addRow(tr("Stimulus:"), mStimulusName);
    form->addRow(tr("Signature:"), mStimulusSig);
    form->addRow(tr("Source:"), mSource);
    form->addRow(tr("Target:"), mTarget);
    mTransForm = form;

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
    connect(mTransKind, &QComboBox::activated, this, &SMPropertiesPanel::onTransKindCommit);
    connect(mTarget, &QComboBox::activated, this, &SMPropertiesPanel::onTargetCommit);
    connect(mSource, &QComboBox::activated, this, &SMPropertiesPanel::onSourceCommit);

    mTransTabs = new QTabWidget(this);
    mTransTabs->setObjectName(QStringLiteral("smTransTabs"));
    mTransTabs->addTab(mTransGeneral, tr("General"));
    mConditions = new SMGuardBar(mModel, this);
    mTransTabs->addTab(mConditions, tr("Conditions"));
    connect(mConditions, &SMGuardBar::badgeChanged, this, &SMPropertiesPanel::onGuardBadgeChanged);
    connect(mConditions, &SMGuardBar::signalNavigateToDefinition, this, &SMPropertiesPanel::signalNavigateToDefinition);

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
    // Use the application's active focus, not QWidget::focusWidget(): the latter keeps returning
    // the last-focused descendant, which reported "editing" forever after the first click.
    QWidget* focus = QApplication::focusWidget();
    return (focus != nullptr) && isAncestorOf(focus);
}

void SMPropertiesPanel::commitPendingEdits(void)
{
    // Each handler checks the page it belongs to, so only the one on show can do anything.
    onStateDescriptionCommit();
    onTransitionDescriptionCommit();
}

bool SMPropertiesPanel::eventFilter(QObject* watched, QEvent* event)
{
    // QPlainTextEdit has no editing-finished signal; commit its edit when it loses focus,
    // matching the single-line fields' commit-on-editing-finished contract.
    if (event->type() == QEvent::FocusOut)
    {
        if ((watched == mStateDesc) || (watched == mTransDesc))
        {
            commitPendingEdits();
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

void SMPropertiesPanel::focusStimulus(uint32_t transitionId)
{
    if (mModel.getData().findTransitionById(transitionId) == nullptr)
    {
        return;
    }

    mModel.getSelectionModel().setSelection({ transitionId });
    if (mPage == PageTransition)
    {
        mTransTabs->setCurrentIndex(0);
        mStimulusName->setFocus();
        mStimulusName->showPopup();
    }
}

void SMPropertiesPanel::focusInternal(uint32_t transitionId)
{
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    const SMStateEntry* owner = data.findTransitionOwner(transitionId);
    if ((transition == nullptr) || (owner == nullptr) || (transition->isInternal() == false))
    {
        return;
    }

    // Selecting the state (not the transition) is the point -- the author clicked a row inside that
    // state's box and must land on the state, on the tab that owns the construct.
    mModel.getSelectionModel().setSelection(QList<uint32_t>{ owner->getId() });
    if ((mPage == PageState) && (mCurrentId == owner->getId()) && (mInternalTab >= 0))
    {
        // Select the row unconditionally: when that state was already the selection, the selection
        // model announces nothing and showState() never runs.
        mInternal->setCurrentTransition(transitionId);
        mStateTabs->setCurrentIndex(mInternalTab);
        mInternal->list()->setFocus();
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

    // The Do tab's stop-condition surface is bound to the shown state and its rebuild is deferred,
    // so it is released here: a deferred rebuild must not outlive the element it reads.
    if (mDoUntil != nullptr)
    {
        mDoUntil->setTarget(SMGuardRef());
    }

    // The Internal tab's editors hold a live pointer into the shown state's transition; drop it with
    // the page, or a deferred rebuild can outlive the document that owns it.
    if (mInternal != nullptr)
    {
        mInternal->bind(0u);
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
    mStateName->setReadOnly(state->isPseudoStart());
    mStateKind->setText(QString::fromLatin1(SMStateEntry::toString(state->getKind())));

    // A Start is a pseudo-state that the machine never occupies, so it cannot act. The activity
    // tabs and the rows only a real state can use are hidden rather than left disabled.
    const bool pseudoStart = state->isPseudoStart();
    for (const ActionSlot& slot : mActionSlots)
    {
        mStateTabs->setTabVisible(slot.tabIndex, pseudoStart == false);
    }

    // Everything a Start owns is an initial transition, so it has no internal one to edit either.
    if (mInternalTab >= 0)
    {
        mStateTabs->setTabVisible(mInternalTab, pseudoStart == false);
    }

    if (mStateForm != nullptr)
    {
        mStateForm->setRowVisible(mStateSubmachine, pseudoStart == false);
        mStateForm->setRowVisible(mStateOnFinal, pseudoStart == false);
        mStateForm->setRowVisible(mStateHistory, pseudoStart == false);
        mStateForm->setRowVisible(mStateDesc, pseudoStart == false);
    }

    if (pseudoStart)
    {
        mStateTabs->setCurrentIndex(0);
    }
    // Only a composite has substates to remember, so a plain state gets a disabled field that says
    // why rather than a missing one. It comes alive the moment the state grows a submachine.
    const bool composite = state->isComposite();

    // A state hosts an import or paints its own substates, never both. Once substates are painted
    // the picker is closed: swapping to an import would delete a subtree that undo cannot rebuild.
    const bool canHost = (state->getKind() == SMStateEntry::eStateKind::Normal) && (state->hasNestedStates() == false);
    mStateSubmachine->clear();
    mStateSubmachine->addItem(tr("(none)"), QString());
    for (const QString& alias : mModel.getIncludeModel().getAliases())
    {
        mStateSubmachine->addItem(alias, alias);
    }

    // A hand-written file may name an import that is no longer registered; showing it keeps the
    // panel honest about what the document says (validation reports the missing declaration).
    const QString submachine = state->getSubmachine();
    if ((submachine.isEmpty() == false) && (mStateSubmachine->findData(submachine) < 0))
    {
        mStateSubmachine->addItem(tr("%1 (not registered)").arg(submachine), submachine);
    }

    mStateSubmachine->setCurrentIndex(qMax(0, mStateSubmachine->findData(submachine)));
    mStateSubmachine->setEnabled(canHost);
    mStateSubmachine->setToolTip(canHost
                                 ? tr("The imported machine this state runs")
                                 : tr("This state paints its own substates, so it cannot host an imported machine"));

    mStateOnFinal->clear();
    mStateOnFinal->addItem(tr("(none)"), QString());
    for (const SMEventEntry* event : mModel.getData().getEvents().getElements())
    {
        if (event != nullptr)
        {
            mStateOnFinal->addItem(event->getName(), event->getName());
        }
    }

    const QString onFinal = state->getOnFinal();
    if ((onFinal.isEmpty() == false) && (mStateOnFinal->findData(onFinal) < 0))
    {
        mStateOnFinal->addItem(tr("%1 (not declared)").arg(onFinal), onFinal);
    }

    mStateOnFinal->setCurrentIndex(qMax(0, mStateOnFinal->findData(onFinal)));
    mStateOnFinal->setEnabled(composite);
    mStateOnFinal->setToolTip(composite
                              ? tr("The event sent when the submachine reaches its Final state")
                              : tr("Only a state with a submachine can finish"));

    mStateHistory->setCurrentIndex(mStateHistory->findData(static_cast<int>(state->getHistory())));
    mStateHistory->setEnabled(composite);
    mStateHistory->setToolTip(composite
                              ? tr("What happens when the machine comes back to this state")
                              : tr("Only a state with a submachine can remember where it was"));
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
    // A document that names no interval reads back as 0, which the spin box cannot show and must
    // not silently "correct" into a value the file does not contain
    mDoInterval->setValue(static_cast<int>(qMax(state->getDoInterval(), SMStateEntry::MIN_DO_INTERVAL)));
    mDoUntil->setTarget(SMGuardRef::doActivity(stateId));
    refreshActionSummaries();
    populateTransitionList(stateId);
    mInternal->bind(stateId);

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
        // The tab tooltip carries the per-list summary, so hovering says what happens around this
        // state without switching tabs. The Do tooltip also folds in its repeat policy.
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
            // A Do is a timer loop, its label is its period
            title = list->isEmpty() ? tr("Do")
                  : (state->getDoInterval() >= SMStateEntry::MIN_DO_INTERVAL
                        ? tr("Do (every %1 ms)").arg(state->getDoInterval())
                        : tr("Do (no interval set)"));
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

    // A Start's transitions are the level's initial ones: nothing triggers them, so there is no
    // stimulus to show and the row carries the condition that decides between them instead.
    const bool pseudoStart = state->isPseudoStart();
    int internalCount = 0;
    for (const SMTransitionEntry* transition : state->getTransitions().getElements())
    {
        if ((transition != nullptr) && transition->isInternal())
        {
            ++internalCount;
        }
    }

    int internalIndex = 0;
    for (SMTransitionEntry* transition : state->getTransitions().getElements())
    {
        if (transition == nullptr)
        {
            continue;
        }

        QString label;
        if (pseudoStart || transition->isInitial())
        {
            label = transition->hasTarget()
                    ? tr("initial -> %1").arg(transition->getTargetName())
                    : (tr("initial") + QStringLiteral(" ") + unconnectedLabel());
            if (transition->hasCondition())
            {
                label += QStringLiteral(" ") + tr("[when]");
            }
        }
        else
        {
            const QString stimulus = transition->getStimulus().isEmpty() ? tr("<stimulus>") : transition->getStimulus();
            if (transition->hasTarget())
            {
                label = stimulus + QStringLiteral(" -> ") + transition->getTargetName();
            }
            else if (transition->isInternal())
            {
                // The same shorthand the Internal tab and the state box use: the priority number,
                // and the guard that tells two transitions on one stimulus apart.
                ++internalIndex;
                label = (internalCount > 1)
                        ? (QStringLiteral("#") + QString::number(internalIndex) + QLatin1Char(' ') + stimulus)
                        : stimulus;
                label += QStringLiteral(" ") + internalLabel();
                const QString chip = SMGuardRender::chipText(mModel.getData(), transition->getId()
                                                            , transition->getGuard(), SMGuardRender::ChipPicker);
                if (chip.isEmpty() == false)
                {
                    label += QStringLiteral(" [") + chip + QLatin1Char(']');
                }
            }
            else
            {
                label = stimulus + QStringLiteral(" ") + unconnectedLabel();
            }
        }

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
    mStimulusName->setCurrentIndex(SMStimulusPicker::fill(*mStimulusName, data
                                                         , static_cast<int>(transition->getStimulusKind())
                                                         , transition->getStimulus()));

    // The read-only signature spells the kind out (`event NewEvent`): a QLabel carries no mark, and
    // this line is the one place that has room for the word.
    const QString signature = SMOperationSummary::stimulusSignature(data, *transition);
    const QString kindWord  = signature.isEmpty()
                              ? QString()
                              : SMKindGlyph::word(SMKindGlyph::stimulusGlyph(transition->getStimulusKind()));
    mStimulusSig->setText(kindWord.isEmpty() ? signature : (kindWord + QLatin1Char(' ') + signature));

    // Fill the target and source pickers from the sibling states of the transition's level. Each
    // item carries the sibling's element id, so the endpoint is committed by id and not by name.
    mTarget->clear();
    const SMStateEntry* owner = data.findTransitionOwner(transitionId);
    const uint32_t sourceId = (owner != nullptr ? owner->getId() : 0u);

    const bool initial = transition->isInitial() || ((owner != nullptr) && owner->isPseudoStart());

    // `To` means the target and nothing else, so the empty row says only that no target is named
    // yet. The Kind combo is where an internal transition is asked for.
    mTarget->addItem(unconnectedLabel(), 0u);

    mSource->clear();
    const SMStateData* level = data.findLevel(mModel.getSelectionModel().getActiveLevel());
    if (level != nullptr)
    {
        for (SMStateEntry* sibling : level->getElements())
        {
            if (sibling == nullptr)
            {
                continue;
            }

            if (sibling->isPseudoStart() == false)
            {
                mTarget->addItem(sibling->getName(), sibling->getId());
            }

            // A Start is offered as a source only when it already owns this transition: moving an
            // ordinary transition onto one would hand a pseudo-state a stimulus to react to.
            const bool selectableSource = sibling->isPseudoStart()
                                        ? (initial && (sibling->getId() == sourceId))
                                        : (sibling->getKind() != SMStateEntry::eStateKind::Final);
            if (selectableSource)
            {
                mSource->addItem(sibling->getName(), sibling->getId());
            }
        }
    }

    // A Start owns nothing but initial transitions and a real state owns no initial one, so the
    // combo offers only what the owner can hold.
    const bool internal = transition->isInternal();
    setComboRowEnabled(*mTransKind, 0, initial == false);   // External: on a real state only
    setComboRowEnabled(*mTransKind, 1, initial == false);   // Internal: on a real state only
    setComboRowEnabled(*mTransKind, 2, initial);            // Initial: on a Start only
    const int kindRow = mTransKind->findData(static_cast<int>(transition->getKind()));
    mTransKind->setCurrentIndex(kindRow >= 0 ? kindRow : 0);
    mTransKind->setEnabled(initial == false);
    mTransKind->setToolTip(initial
                           ? tr("Everything a Start state owns is an initial transition")
                           : QString());

    mStimulusName->setEnabled(initial == false);
    mSource->setEnabled(initial == false);
    if (initial)
    {
        mStimulusName->setToolTip(tr("An initial transition is taken on entering the level, so nothing triggers it"));
        mStimulusSig->setText(tr("initial transition (no stimulus)"));
    }
    else
    {
        mStimulusName->setToolTip(QString());
    }

    // An internal transition has no target at all -- the row is hidden rather than disabled,
    // because a greyed picker reads as "not yet" and this one is "never".
    if (mTransForm != nullptr)
    {
        mTransForm->setRowVisible(mTarget, internal == false);
        mTransForm->setRowVisible(mStimulusName, initial == false);
        mTransForm->setRowVisible(mStimulusSig, initial == false);
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
        || (findEntryName(data.getIncludes().getElements(), elementId, name)   && (kind = tr("Include"), true));

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

void SMPropertiesPanel::onStateHistoryCommit()
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

    const SMStateEntry::eHistory history = static_cast<SMStateEntry::eHistory>(mStateHistory->currentData().toInt());
    if ((history == state->getHistory()) || (state->isComposite() == false))
    {
        return;
    }

    mModel.getUndoStack().push(new SMSetHistoryCommand(data, mModel.getNotifier(), mCurrentId, history, tr("Set history mode")));
}

void SMPropertiesPanel::onStateSubmachineCommit()
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

    const QString alias = mStateSubmachine->currentData().toString();
    if (alias == state->getSubmachine())
    {
        return;
    }

    SMSetSubmachineCommand* command = new SMSetSubmachineCommand(data, mModel.getNotifier(), mCurrentId, alias
                                                               , alias.isEmpty() ? tr("Remove submachine") : tr("Set submachine"));
    if (command->isEffective())
    {
        mModel.getUndoStack().push(command);
    }
    else
    {
        delete command;
        showState(mCurrentId);
    }
}

void SMPropertiesPanel::onStateOnFinalCommit()
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

    const QString event = mStateOnFinal->currentData().toString();
    if ((event == state->getOnFinal()) || (state->isComposite() == false))
    {
        return;
    }

    mModel.getUndoStack().push(new SMSetOnFinalCommand(data, mModel.getNotifier(), mCurrentId, event, tr("Set completion event")));
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
    const uint32_t value = qMax(static_cast<uint32_t>(mDoInterval->value()), SMStateEntry::MIN_DO_INTERVAL);
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

    // Backstop for the pseudo-state rule: an initial transition names no stimulus. The picker is
    // disabled for one, so this only fires if a stale or programmatic change slips through.
    const SMStateEntry* owner = mModel.getData().findTransitionOwner(mCurrentId);
    if ((owner != nullptr) && owner->isPseudoStart())
    {
        return;
    }

    SMStimulusPicker::apply(mModel, *mStimulusName, mCurrentId);
}

void SMPropertiesPanel::onTransKindCommit()
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

    const int row = mTransKind->currentIndex();
    if (row < 0)
    {
        return;
    }

    const SMTransitionEntry::eTransitionKind kind =
            static_cast<SMTransitionEntry::eTransitionKind>(mTransKind->itemData(row).toInt());
    if (kind == transition->getKind())
    {
        return;
    }

    // Backstop for what the greyed rows already prevent: a Start owns nothing but initial
    // transitions, and no real state owns one.
    const SMStateEntry* owner = data.findTransitionOwner(mCurrentId);
    const bool startOwned = (owner != nullptr) && owner->isPseudoStart();
    if (startOwned != (kind == SMTransitionEntry::eTransitionKind::Initial))
    {
        showTransition(mCurrentId);     // put the picker back on what the document says
        return;
    }

    mModel.getUndoStack().push(new SMSetTransitionKindCommand(data, mModel.getNotifier(), mCurrentId, kind, tr("Set transition kind")));
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

    // The picker is a closed list; each row carries its state's element ID (0 = not connected).
    const uint32_t targetId = mTarget->currentData().toUInt();
    if (targetId == 0)
    {
        if (transition->hasTarget())
        {
            mModel.getUndoStack().push(new SMSetTransitionTargetCommand(data, mModel.getNotifier(), mCurrentId, 0u, tr("Disconnect target")));
        }

        return;
    }

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

    if (newSource->getKind() == SMStateEntry::eStateKind::Final)
    {
        return;
    }

    if (newSource->isPseudoStart() || oldSource->isPseudoStart())
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
    // Re-label the State-Actions headers even mid-edit: this only re-titles them and never rebinds
    // the editors, so it cannot clobber typing.
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
        // A method changing type, or an event or timer edit, changes the stimulus vocabulary. The
        // changed id is never mCurrentId, so rebuild the transition page and its picker here.
        refresh();
    }
    else if ((mPage == PageState) && (kind == eDocElementKind::Import))
    {
        // An alias rename rewrites the hosting states through the reference-rewrite command, but
        // the combo item text is built here and stays stale until the page is rebuilt.
        refresh();
    }
}

void SMPropertiesPanel::onElementRemoved(uint32_t id, eDocElementKind kind)
{
    if (id == mCurrentId)
    {
        showEmpty();
    }
    else if ((mPage == PageState) && (isEditing() == false) && (kind == eDocElementKind::Import))
    {
        refresh();
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
        mInternal->refresh();
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
