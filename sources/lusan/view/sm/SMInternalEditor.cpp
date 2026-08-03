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
 *  \file        lusan/view/sm/SMInternalEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM: the internal transitions of one state, edited in place.
 *
 ************************************************************************/

#include "lusan/view/sm/SMInternalEditor.hpp"

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMOperationSummary.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/SMValidationController.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMGuardBar.hpp"
#include "lusan/view/sm/SMKindGlyph.hpp"
#include "lusan/view/sm/SMOperationsEditor.hpp"
#include "lusan/view/sm/SMStimulusPicker.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"

#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMouseEvent>
#include <QScrollArea>
#include <QTabWidget>
#include <QPalette>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
    //!< The transition ID carried by each row of the selector.
    constexpr int RoleTransitionId { Qt::UserRole + 1 };

    //!< How many rows the transition list shows before it scrolls. Its height is FIXED at this many
    //!< whatever it holds, so adding or removing a transition never moves the controls below it.
    constexpr int VisibleRows { 3 };
}

SMInternalEditor::SMInternalEditor(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mListLabel   (nullptr)
    , mList        (nullptr)
    , mBtnUp        (nullptr)
    , mBtnDown      (nullptr)
    , mBtnAdd       (nullptr)
    , mBtnRemove    (nullptr)
    , mStimulus     (nullptr)
    , mSignature    (nullptr)
    , mTabs         (nullptr)
    , mOperations   (nullptr)
    , mGuard        (nullptr)
    , mStateId      (0u)
    , mCurrentId    (0u)
    , mUpdating     (false)
    , mSignatureKind(SMReferences::eTarget::Trigger)
    , mSignatureDecl(0u)
{
    // WHICH transition, and in WHAT ORDER. A list rather than a closed picker, for two reasons that
    // only apply once a state has several: the rows exist to be COMPARED (they share a stimulus and
    // differ by guard, and a picker hides all but one), and priority is now an edit -- pressing the
    // up button must MOVE a row past its neighbour, not silently change a number inside a closed
    // box. It is sized to its content and shown only when there are two, so the four-rows-for-one
    // waste that made the first list box wrong cannot come back.
    mList = new QListWidget(this);
    mList->setObjectName(QStringLiteral("smInternalList"));
    mList->setSelectionMode(QAbstractItemView::SingleSelection);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setUniformItemSizes(true);
    mList->setToolTip(tr("The internal transitions of this state, in priority order: the first one"
                         " whose condition holds is the one that runs. Each runs its operations on"
                         " its stimulus without leaving the state, so no exit or entry action runs."));

    // The height is measured ONCE, from a probe row carrying an icon like the real ones, so it is
    // the true row height rather than an estimate from the font -- an estimate leaves a sliver of a
    // fourth row peeking, which reads as a rendering fault. Measured here, it can never drift when
    // the content changes, which is the whole point of a fixed frame.
    mList->addItem(new QListWidgetItem(SMKindGlyph::icon(SMKindGlyph::eGlyph::Trigger, palette().color(QPalette::Text))
                                      , QStringLiteral("Mg")));
    const int rowHeight = std::max(mList->sizeHintForRow(0), 16);
    mList->clear();
    mList->setFixedHeight((VisibleRows * rowHeight) + (2 * mList->frameWidth()));

    mListLabel = new QLabel(tr("Transitions:"), this);

    mStimulus = new QComboBox(this);
    mStimulus->setObjectName(QStringLiteral("smInternalStimulus"));
    mStimulus->setEditable(false);

    // The signature is a caption under the picker, not a form row of its own: it is one dim line
    // that confirms what was picked, and it disappears entirely when there is nothing to confirm.
    mSignature = new QLabel(this);
    mSignature->setObjectName(QStringLiteral("smInternalSignature"));
    mSignature->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // Dimmed by PALETTE, never by setEnabled(false): a disabled widget is not delivered mouse
    // events at all, so the Ctrl+Shift jump on this line could never fire. The colour is the same
    // one the style would have used for a disabled label, so nothing looks different.
    QPalette dim = mSignature->palette();
    dim.setColor(QPalette::WindowText, dim.color(QPalette::Disabled, QPalette::WindowText));
    dim.setColor(QPalette::Text, dim.color(QPalette::Disabled, QPalette::Text));
    mSignature->setPalette(dim);
    QFont captionFont = mSignature->font();
    captionFont.setPointSizeF(captionFont.pointSizeF() * 0.9);
    mSignature->setFont(captionFont);

    const auto makeButton = [this](const QString& name, const QIcon& icon, const QString& tip) -> QToolButton*
    {
        QToolButton* button = new QToolButton(this);
        button->setObjectName(name);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setIcon(icon);
        button->setToolTip(tip);
        return button;
    };

    // The toolbar is the one every ordered list in this application wears -- add, remove, a rule,
    // then move up / move down, with the same icons and the same Ctrl+Up / Ctrl+Down keys as
    // AttributeListView and its four siblings. Add and remove were tucked beside the Stimulus row
    // before, which is not where an author looks for them.
    mBtnAdd    = makeButton(QStringLiteral("smBtnAddInternal")
                           , QIcon(QStringLiteral(":/icons/entry add"))
                           , tr("Add an internal transition to this state"));
    mBtnRemove = makeButton(QStringLiteral("smBtnRemoveInternal")
                           , QIcon(QStringLiteral(":/icons/entry delete"))
                           , tr("Remove the selected internal transition"));

    // Document order IS priority order -- the first transition whose guard holds is the one that
    // runs -- so raising and lowering a row is a real edit, not a cosmetic one.
    mBtnUp     = makeButton(QStringLiteral("smBtnInternalUp")
                           , QIcon(QStringLiteral(":/icons/move up"))
                           , tr("Try this transition earlier (raise its priority)"));
    mBtnDown   = makeButton(QStringLiteral("smBtnInternalDown")
                           , QIcon(QStringLiteral(":/icons/move down"))
                           , tr("Try this transition later (lower its priority)"));
    mBtnUp->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
    mBtnDown->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down));

    // The SAME operations editor and the SAME guard bar the transition page uses: an internal
    // transition is a transition, and a second, lesser editor for it would drift from the first.
    mOperations = new SMOperationsEditor(mModel, this);
    // In THIS scope only one of its three sections opens at a time. On the Enter/Do/Exit tabs the
    // editor owns the whole tab and can afford all three open; here it shares the height with a
    // picker, a stimulus row and a tab bar, and three open sections pushed its own content out of
    // reach. The section the author wants first is the action.
    mOperations->setSectionsCompact(true);

    mGuard = new SMGuardBar(mModel, this);
    connect(mGuard, &SMGuardBar::signalNavigateToDefinition, this, &SMInternalEditor::signalNavigateToDefinition);

    // Every guard bar names its parts the same way (`smGuardField`, `smGuardAccordion`, ...), and a
    // findChild by name walks children in construction order -- so THIS bar, built while the state
    // page is built, would answer for the transition page's, which is the one the app and its tests
    // mean by those names. Re-prefix ours, unconditionally and in every instance: the transition
    // page's guard bar is the primary one, and an internal transition's guard is reached through
    // this editor, never by name.
    const QList<QObject*> guardParts = mGuard->findChildren<QObject*>();
    for (QObject* part : guardParts)
    {
        const QString name = part->objectName();
        if (name.isEmpty() == false)
        {
            part->setObjectName(QStringLiteral("smInternal") + name.at(0).toUpper() + name.mid(1));
        }
    }

    // The guard bar is built to own a whole tab (it does on the transition page) and its catalog
    // and argument grid are tall. In a dock that is short because another one is open beside it,
    // the mapping rows have to stay REACHABLE, so the bar scrolls as a whole -- the same escape
    // the operations editor already gives itself.
    QScrollArea* guardScroll = new QScrollArea(this);
    guardScroll->setObjectName(QStringLiteral("smInternalGuardScroll"));
    guardScroll->setWidgetResizable(true);
    guardScroll->setFrameShape(QFrame::NoFrame);
    guardScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    guardScroll->setWidget(mGuard);

    // Actions and Conditions are TABS, not accordion sections: each then takes the whole height
    // that is left, and neither can be pushed off the bottom by the other. It is also how the
    // transition page shows the very same two editors, so the two surfaces finally read alike.
    mTabs = new QTabWidget(this);
    mTabs->setObjectName(QStringLiteral("smInternalTabs"));
    mTabs->setDocumentMode(true);       // a nested tab bar reads as a strip, not as a second window
    mTabs->addTab(mOperations, SMToolIcons::icon(SMToolIcons::eIcon::NewAction), tr("Actions"));
    mTabs->addTab(guardScroll, SMToolIcons::icon(SMToolIcons::eIcon::GuardConditions), tr("Conditions"));
    mTabs->setTabToolTip(0, tr("The operations it runs, in order"));
    mTabs->setTabToolTip(1, tr("The guard that must hold for it to run"));

    // Two labelled rows over the tabs: which transition, and what fires it. Both stay visible
    // whichever tab is open -- they are the identity of what is being edited, not a section of it.
    QFrame* rule = new QFrame(this);
    rule->setFrameShape(QFrame::VLine);
    rule->setMaximumSize(24, 24);

    QHBoxLayout* bar = new QHBoxLayout();
    bar->setContentsMargins(0, 0, 0, 0);
    bar->setSpacing(4);
    bar->addWidget(mListLabel);
    bar->addStretch(1);
    bar->addWidget(mBtnAdd);
    bar->addWidget(mBtnRemove);
    bar->addWidget(rule);
    bar->addWidget(mBtnUp);
    bar->addWidget(mBtnDown);

    // The stimulus of the SELECTED row, and what that stimulus is -- the detail half of a
    // master/detail form, and the only part of it that edits anything.
    QGridLayout* detail = new QGridLayout();
    detail->setContentsMargins(0, 0, 0, 0);
    detail->setHorizontalSpacing(6);
    detail->setVerticalSpacing(4);
    detail->addWidget(new QLabel(tr("Stimulus:"), this), 0, 0);
    detail->addWidget(mStimulus, 0, 1);
    detail->addWidget(mSignature, 1, 1);
    detail->setColumnStretch(1, 1);

    QVBoxLayout* head = new QVBoxLayout();
    head->setContentsMargins(6, 6, 6, 2);
    head->setSpacing(4);
    head->addLayout(bar);
    head->addWidget(mList);
    head->addLayout(detail);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addLayout(head);
    layout->addWidget(mTabs, 1);

    connect(mList, &QListWidget::currentRowChanged, this, [this](int) { onSelected(); });
    connect(mStimulus, &QComboBox::activated, this, &SMInternalEditor::onStimulusCommit);
    connect(mBtnAdd, &QToolButton::clicked, this, &SMInternalEditor::onAdd);
    connect(mBtnRemove, &QToolButton::clicked, this, &SMInternalEditor::onRemove);
    connect(mBtnUp, &QToolButton::clicked, this, &SMInternalEditor::onMoveUp);
    connect(mBtnDown, &QToolButton::clicked, this, &SMInternalEditor::onMoveDown);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMInternalEditor::onElementChanged);

    // A reorder changes every row's number, so the picker has to be re-read whoever caused it --
    // this editor, the canvas, or an undo.
    connect(&notifier, &DocModelNotifier::listReordered, this, [this](uint32_t ownerId, eDocElementKind kind)
    {
        if ((kind == eDocElementKind::Transition) && (ownerId == mStateId) && (mStateId != 0u))
        {
            refresh();
        }
    });

    // A transition added anywhere -- the canvas `Add Internal Transition` action, the Design menu,
    // the toolbar, a redo -- becomes the SELECTED one. The author just asked for it, so it is the
    // one they mean to fill in; leaving the previous row active made the new one look like it had
    // not been created. The `+` button in this tab selects it too, and this makes the other routes
    // behave the same.
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t id, eDocElementKind kind)
    {
        if ((kind != eDocElementKind::Transition) || (mStateId == 0u))
        {
            return;
        }

        const SMTransitionEntry* added = mModel.getData().findTransitionById(id);
        const SMTransitionEntry* mine  = (added != nullptr) && added->isInternal() ? added : nullptr;
        if ((mine != nullptr) && (mine->owningState() != nullptr) && (mine->owningState()->getId() == mStateId))
        {
            setCurrentTransition(id);
        }
        else
        {
            refresh();
        }
    });

    // A transition removed elsewhere (or an undo of an add) renumbers this list too.
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind)
    {
        if ((kind == eDocElementKind::Transition) && (mStateId != 0u))
        {
            refresh();
        }
    });
    connect(&notifier, &DocModelNotifier::documentReloaded, this, [this]() { refresh(); });

    // Undo and redo MUST re-read, even while a field here holds focus. onElementChanged deliberately
    // skips a refresh when the focus is inside this editor -- that is the author typing, and a
    // rebuild would clobber it -- but Ctrl+Z is not typing: the field it is putting back is usually
    // the very one with focus, and skipping there left the row label and the status line stale.
    connect(&mModel.getUndoStack(), &QUndoStack::indexChanged, this, [this](int) { refresh(); });

    // A shadowed transition (an earlier unconditional one on the same stimulus already answered the
    // stimulus) is a finding the validator makes; the rows wear it as soon as it is recomputed.
    connect(&mModel.getValidationController(), &SMValidationController::validationUpdated
           , this, [this](const QList<SMIssue>&) { refresh(); });

    mSignature->installEventFilter(this);

    showTransition(0u);
}

SMInternalEditor::~SMInternalEditor()
{
    mModel.getNotifier().disconnect(this);
}

void SMInternalEditor::bind(uint32_t stateId)
{
    if (stateId != mStateId)
    {
        mStateId   = stateId;
        mCurrentId = 0u;        // a different state: nothing carried over
    }

    refresh();
}

void SMInternalEditor::refresh()
{
    const bool wasUpdating = mUpdating;
    mUpdating = true;                   // refilling moves the current row; that is not an edit

    mList->clear();
    const StateMachineData&         data  = mModel.getData();
    const QList<SMTransitionEntry*> list  = internals();
    const QList<SMIssue>&           found = mModel.getValidationController().issues();
    const int                       total = static_cast<int>(list.size());
    int selectRow = -1;
    for (int index = 0; index < total; ++index)
    {
        const SMTransitionEntry* transition = list.at(index);
        QString label = rowLabel(*transition, index + 1, total);

        // The chip in the row is a summary by design; the tooltip is where the guard is READ, and
        // where a transition that can never fire says so in the validator's own words -- the same
        // sentence the edge label and the Conditions status line show.
        QString tip = SMGuardRender::guardText(data, transition->getId(), transition->getGuard(), true);
        for (const SMIssue& issue : found)
        {
            if ((issue.elementId == transition->getId()) && (issue.kind == eDocElementKind::Transition)
                && (issue.severity != SMIssue::eSeverity::Info))
            {
                // The same two text markers the canvas edge labels wear, so severity survives a
                // grayscale screen and a colour-blind reader; the sentence itself is the tooltip.
                label += QLatin1Char(' ')
                       + ((issue.severity == SMIssue::eSeverity::Error) ? QStringLiteral("(x)") : QStringLiteral("(!)"));
                tip += (tip.isEmpty() ? QString() : QStringLiteral("\n"))
                     + ((issue.severity == SMIssue::eSeverity::Error) ? tr("err: %1") : tr("warn: %1")).arg(issue.message);
                break;
            }
        }

        QListWidgetItem* item = new QListWidgetItem(
                    SMKindGlyph::icon(SMKindGlyph::stimulusGlyph(transition->getStimulusKind())
                                     , mList->palette().color(QPalette::Text))
                    , label, mList);
        item->setData(RoleTransitionId, transition->getId());
        item->setToolTip(tip);
        if (transition->getId() == mCurrentId)
        {
            selectRow = mList->row(item);
        }
    }

    // Keep editing what was being edited; otherwise land on the first one, so opening the editor
    // shows a filled form rather than an empty one the author has to pick a row to wake up.
    if ((selectRow < 0) && (total > 0))
    {
        selectRow = 0;
    }

    mList->setCurrentRow(selectRow);
    updateListVisibility(total);
    mUpdating = wasUpdating;

    const QListWidgetItem* current = mList->currentItem();
    showTransition((current != nullptr) ? current->data(RoleTransitionId).toUInt() : 0u);
    emit countChanged(total);
}

QList<SMTransitionEntry*> SMInternalEditor::internals() const
{
    QList<SMTransitionEntry*> result;
    const SMStateEntry* state = mModel.getData().findStateById(mStateId);
    if (state == nullptr)
    {
        return result;
    }

    // Document order, untouched: it IS the priority order, and the numbers the rows wear are
    // positions in it.
    for (SMTransitionEntry* transition : state->getTransitions().getElements())
    {
        if ((transition != nullptr) && transition->isInternal())
        {
            result.append(transition);
        }
    }

    return result;
}

QString SMInternalEditor::rowLabel(const SMTransitionEntry& transition, int ordinal, int total) const
{
    // A row reads as the transition does on the canvas -- `on <stimulus>` -- with the stimulus kind
    // as a drawn mark rather than a word, the same vocabulary the state box and the edge labels use.
    const QString stimulus = transition.getStimulus().isEmpty() ? tr("<stimulus>") : transition.getStimulus();
    QString label;
    if (total > 1)
    {
        // Leading, so the numbers form a column that can be read at a glance and a reorder is
        // visibly a change in that column. A lone transition has no priority to speak of.
        label = QStringLiteral("#") + QString::number(ordinal) + QLatin1Char(' ');
    }

    label += tr("on %1").arg(stimulus);

    // The guard is what distinguishes two transitions on ONE stimulus, so it belongs in the row --
    // shortened by meaning, never chopped mid-token. See SMGuardRender::chipText.
    const QString chip = SMGuardRender::chipText(mModel.getData(), transition.getId()
                                                , transition.getGuard(), SMGuardRender::ChipPicker);
    if (chip.isEmpty() == false)
    {
        label += QStringLiteral(" [") + chip + QLatin1Char(']');
    }

    return label;
}

void SMInternalEditor::updateListVisibility(int total)
{
    // The list is ALWAYS here, at a FIXED height, whether it holds nothing, one row or ten.
    //
    // Sizing it to its content was worse than it sounds: adding a second transition made the list
    // appear, and adding a third grew it, and each time everything below -- the stimulus row, the
    // status line, the whole Actions/Conditions tab -- jumped down the screen. An author who has
    // just pressed `add` is looking at the row they created, not hunting for the field that moved.
    // A fixed frame costs the same pixels every time and never moves anything.
    //
    // Three rows: most states carry one to three internal transitions, so the common case is fully
    // visible without scrolling, and the fourth and beyond scroll into a frame that does not grow.
    const int position = mList->currentRow();
    mBtnUp->setEnabled((position > 0));
    mBtnDown->setEnabled((position >= 0) && (position < (total - 1)));
    mBtnRemove->setEnabled(mCurrentId != 0u);
    mBtnAdd->setEnabled(mModel.getData().findStateById(mStateId) != nullptr);
}

int SMInternalEditor::count() const
{
    return mList->count();
}

void SMInternalEditor::setCurrentTransition(uint32_t transitionId)
{
    mCurrentId = transitionId;
    refresh();
}

void SMInternalEditor::showTransition(uint32_t transitionId)
{
    StateMachineData& data = mModel.getData();
    SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if ((transition != nullptr) && (transition->isInternal() == false))
    {
        transition = nullptr;           // this editor edits internal transitions and nothing else
    }

    const bool wasUpdating = mUpdating;
    mUpdating = true;

    mCurrentId = (transition != nullptr) ? transitionId : 0u;
    mBtnRemove->setEnabled(transition != nullptr);
    mBtnAdd->setEnabled(mModel.getData().findStateById(mStateId) != nullptr);
    mStimulus->setEnabled(transition != nullptr);

    if (transition == nullptr)
    {
        mStimulus->clear();
        mSignatureKind = SMReferences::eTarget::Trigger;
        mSignatureDecl = 0u;
        mSignature->setText(tr("Add one to run operations without leaving this state"));
        mSignature->setToolTip(QString());
        mSignature->setVisible(true);
        mTabs->setEnabled(false);
        mGuard->setTransition(0u);
        mOperations->clearBinding();
        mUpdating = wasUpdating;
        return;
    }

    mTabs->setEnabled(true);

    mStimulus->setCurrentIndex(SMStimulusPicker::fill(*mStimulus, data
                                                     , static_cast<int>(transition->getStimulusKind())
                                                     , transition->getStimulus()));

    showSignature(transition);

    mGuard->setTransition(transitionId);
    // Its own Param scope: an internal transition's operations may map the stimulus parameters,
    // exactly as an external one's do.
    mOperations->bind(transitionId, eDocElementKind::Transition, transitionId, transition, &transition->getOperations());

    mUpdating = wasUpdating;
}

void SMInternalEditor::showSignature(const SMTransitionEntry* transition)
{
    mSignatureKind = SMReferences::eTarget::Trigger;
    mSignatureDecl = 0u;

    const StateMachineData& data = mModel.getData();
    const QString name = (transition != nullptr) ? transition->getStimulus() : QString();
    if ((transition == nullptr) || name.isEmpty())
    {
        mSignature->setText(tr("no stimulus -- nothing fires this transition yet"));
        mSignature->setToolTip(QString());
        mSignature->setVisible(true);
        return;
    }

    // What the stimulus IS, in its own terms. A name alone answers "which one" and nothing else;
    // the author asking "will this fire in time" or "what does it carry" had to leave the tab.
    const SMKindGlyph::eGlyph glyph = SMKindGlyph::stimulusGlyph(transition->getStimulusKind());
    QString text = SMKindGlyph::word(glyph);
    if (text.isEmpty() == false)
    {
        text += QLatin1Char(' ');
    }

    QString detail;
    switch (transition->getStimulusKind())
    {
    case SMTransitionEntry::eStimulusKind::Timer:
    {
        const SMTimerEntry* timer = SMDocumentIndex(data).timer(name);
        text += name;
        if (timer != nullptr)
        {
            mSignatureKind = SMReferences::eTarget::Timer;
            mSignatureDecl = timer->getId();

            // Repeat 0 is continuous; 1 fires once and is the common case. Reading them as
            // "after ... once" and "every ... N times" says what the timer DOES, where a bare
            // `1000/1` pair asks the reader to remember which number is which.
            const uint32_t repeat = timer->getRepeat();
            if (repeat == 1u)
            {
                detail = tr("after %1 ms, once").arg(timer->getTimeout());
            }
            else if (repeat == 0u)
            {
                detail = tr("every %1 ms, repeats forever").arg(timer->getTimeout());
            }
            else
            {
                detail = tr("every %1 ms, %2 times").arg(timer->getTimeout()).arg(repeat);
            }
        }

        break;
    }

    case SMTransitionEntry::eStimulusKind::Event:
    {
        const SMEventEntry* event = SMDocumentIndex(data).event(name);
        text += name;
        if (event != nullptr)
        {
            mSignatureKind = SMReferences::eTarget::Event;
            mSignatureDecl = event->getId();

            // An event is a signal, not a call, so its payload is NOT written as a parameter list
            // (issue #543: empty brackets claimed a call that does not exist). It is named as what
            // it is -- what the event carries.
            QStringList carried;
            for (const MethodParameter& param : event->getElements())
            {
                carried.append(param.getName());
            }

            detail = carried.isEmpty() ? tr("no payload") : tr("carries %1").arg(carried.join(QStringLiteral(", ")));
        }

        break;
    }

    case SMTransitionEntry::eStimulusKind::Trigger:
    default:
    {
        // A trigger IS a declared method, so it reads as a signature -- and its parameters are what
        // the operations on this transition may map, which is the next thing the author does.
        text += SMOperationSummary::stimulusSignature(data, *transition);
        const SMMethodEntry* method = SMDocumentIndex(data).method(name);
        if (method != nullptr)
        {
            mSignatureKind = SMReferences::eTarget::Trigger;
            mSignatureDecl = method->getId();
        }

        break;
    }
    }

    // A stimulus that names nothing declared is the single most useful thing this line can report:
    // the transition looks complete and can never fire.
    if (mSignatureDecl == 0u)
    {
        detail = tr("not declared");
    }

    mSignature->setText(detail.isEmpty() ? text : (text + QStringLiteral("  --  ") + detail));
    mSignature->setToolTip((mSignatureDecl != 0u)
                           ? tr("Ctrl+Shift click to open the declaration of '%1'").arg(name)
                           : tr("'%1' is not declared in this machine").arg(name));
}

bool SMInternalEditor::eventFilter(QObject* watched, QEvent* event)
{
    // The same gesture the guard field and the state-box rows carry: Ctrl+Shift click opens the
    // declaration. The status line is where the author is already looking when asking "which timer
    // is this?", so it is where the answer should be one click away.
    if ((watched == mSignature) && (event->type() == QEvent::MouseButtonPress) && (mSignatureDecl != 0u))
    {
        QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
        if ((mouse->button() == Qt::LeftButton)
            && mouse->modifiers().testFlag(Qt::ControlModifier)
            && mouse->modifiers().testFlag(Qt::ShiftModifier))
        {
            emit signalNavigateToDefinition(mSignatureKind, mSignatureDecl);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SMInternalEditor::moveBy(int delta)
{
    StateMachineData& data = mModel.getData();
    SMStateEntry* state = data.findStateById(mStateId);
    const QList<SMTransitionEntry*> list = internals();
    const int here = static_cast<int>(list.indexOf(data.findTransitionById(mCurrentId)));
    const int there = here + delta;
    if ((state == nullptr) || (here < 0) || (there < 0) || (there >= static_cast<int>(list.size())))
    {
        return;
    }

    // Priority is a position in the state's whole transition list, external transitions included --
    // that is the list the shadowing rule reads. So the move is expressed in that list's indices:
    // the row takes the place of the internal neighbour it is swapping priority with.
    const int from = state->getTransitions().findIndex(list.at(here)->getId());
    const int to   = state->getTransitions().findIndex(list.at(there)->getId());
    if ((from < 0) || (to < 0))
    {
        return;
    }

    mModel.getUndoStack().push(new SMMoveTransitionCommand(data, mModel.getNotifier(), *state, mCurrentId, to
                                                          , tr("Reorder internal transitions of %1").arg(state->getName())));
    refresh();
}

void SMInternalEditor::onMoveUp()
{
    moveBy(-1);
}

void SMInternalEditor::onMoveDown()
{
    moveBy(1);
}

void SMInternalEditor::onSelected()
{
    if (mUpdating)
    {
        return;
    }

    const QListWidgetItem* current = mList->currentItem();
    showTransition((current != nullptr) ? current->data(RoleTransitionId).toUInt() : 0u);
}

void SMInternalEditor::onStimulusCommit()
{
    if (mUpdating || (mCurrentId == 0u))
    {
        return;
    }

    SMStimulusPicker::apply(mModel, *mStimulus, mCurrentId);
    // The row reads `on <stimulus>` and the signature line spells the kind out, so both follow the
    // pick immediately. onElementChanged cannot do it: the picker still holds focus, and a rebuild
    // from a notifier while an editor has focus is what clobbers typing elsewhere.
    refresh();
}

void SMInternalEditor::onAdd()
{
    StateMachineData& data = mModel.getData();
    SMStateEntry* state = data.findStateById(mStateId);
    if ((state == nullptr) || state->isPseudoStart())
    {
        return;
    }

    // No target and no edge geometry -- an internal transition renders as a state-body row, which
    // is the same command the canvas `Add Internal Transition` action pushes.
    SMCreateTransitionCommand* command =
            new SMCreateTransitionCommand(  data, mModel.getNotifier(), *state
                                          , SMTransitionEntry::eStimulusKind::Trigger, QString()
                                          , 0u, QList<QPointF>()
                                          , tr("Add internal transition to %1").arg(state->getName())
                                          , nullptr, SMTransitionEntry::eTransitionKind::Internal);
    mModel.getUndoStack().push(command);

    // Select the new one: the author asked for it, so it is what the editors below must show.
    setCurrentTransition(command->getTransitionId());
    mStimulus->setFocus();              // a new one has no stimulus yet, and that is the first thing
}

void SMInternalEditor::onRemove()
{
    StateMachineData& data = mModel.getData();
    SMStateEntry* state = data.findStateById(mStateId);
    const SMTransitionEntry* transition = data.findTransitionById(mCurrentId);
    if ((state == nullptr) || (transition == nullptr))
    {
        return;
    }

    // Where the selection lands after the delete, decided BEFORE it: the row above the one being
    // removed, or the new first row when the first one goes. Falling back to the first row -- which
    // is what happens when the selection is simply dropped -- threw the author to the top of the
    // list every time they deleted from the bottom of it.
    const QList<SMTransitionEntry*> list = internals();
    const int position = static_cast<int>(list.indexOf(data.findTransitionById(mCurrentId)));
    const int follow   = (position > 0) ? (position - 1) : ((list.size() > 1) ? 1 : -1);
    const uint32_t followId = (follow >= 0) ? list.at(follow)->getId() : 0u;

    const QString stimulus = transition->getStimulus();
    mCurrentId = followId;
    mModel.getUndoStack().push(new SMRemoveTransitionCommand(data, mModel.getNotifier(), *state, transition->getId()
                                                            , stimulus.isEmpty()
                                                              ? tr("Remove internal transition")
                                                              : tr("Remove internal transition on %1").arg(stimulus)));
    refresh();
}

void SMInternalEditor::onElementChanged(uint32_t id, eDocElementKind kind)
{
    // The transition being edited changed under the editor (its guard was committed, its stimulus
    // was set from the canvas): its row label and signature line have to follow. Not while an
    // editor here has focus -- that is the author typing, and a rebuild would clobber it.
    QWidget* focus = QApplication::focusWidget();
    const bool editing = (focus != nullptr) && isAncestorOf(focus);
    if ((mStateId != 0u) && (kind == eDocElementKind::Transition) && (id == mCurrentId) && (id != 0u) && (editing == false))
    {
        refresh();
    }
}
