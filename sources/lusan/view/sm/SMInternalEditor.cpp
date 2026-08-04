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
    // A list rather than a closed picker: the rows exist to be compared, and priority is an edit,
    // so the up button moves a row past its neighbour. Sized to its content, shown from two rows on.
    mList = new QListWidget(this);
    mList->setObjectName(QStringLiteral("smInternalList"));
    mList->setSelectionMode(QAbstractItemView::SingleSelection);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setUniformItemSizes(true);
    mList->setToolTip(tr("The internal transitions of this state, in priority order: the first one"
                         " whose condition holds is the one that runs. Each runs its operations on"
                         " its stimulus without leaving the state, so no exit or entry action runs."));

    // The height is measured once from a probe row carrying an icon, so it is the true row height.
    // An estimate from the font left a sliver of a fourth row peeking.
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
    // Dimmed by palette, never by setEnabled(false): a disabled widget receives no mouse events, so
    // the Ctrl+Shift jump on this line could never fire. The colour is the style's disabled one.
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

    // The same toolbar every ordered list in the application wears: add, remove, a rule, then move
    // up and move down, with the same icons and the same Ctrl+Up / Ctrl+Down keys.
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
    // Here the editor shares its height with a picker, a stimulus row and a tab bar, so only one
    // section opens at a time. On the Enter/Do/Exit tabs it owns the tab and keeps all three open.
    mOperations->setSectionsCompact(true);

    mGuard = new SMGuardBar(mModel, this);
    connect(mGuard, &SMGuardBar::signalNavigateToDefinition, this, &SMInternalEditor::signalNavigateToDefinition);

    // Every guard bar names its parts the same way, and findChild walks children in construction
    // order, so this bar would answer for the transition page's. Re-prefix ours in every instance.
    const QList<QObject*> guardParts = mGuard->findChildren<QObject*>();
    for (QObject* part : guardParts)
    {
        const QString name = part->objectName();
        if (name.isEmpty() == false)
        {
            part->setObjectName(QStringLiteral("smInternal") + name.at(0).toUpper() + name.mid(1));
        }
    }

    // The guard bar is built to own a whole tab and its catalog and argument grid are tall, so in
    // a short dock it scrolls as a whole and the mapping rows stay reachable.
    QScrollArea* guardScroll = new QScrollArea(this);
    guardScroll->setObjectName(QStringLiteral("smInternalGuardScroll"));
    guardScroll->setWidgetResizable(true);
    guardScroll->setFrameShape(QFrame::NoFrame);
    guardScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    guardScroll->setWidget(mGuard);

    // Actions and Conditions are tabs, not accordion sections, so each takes the whole remaining
    // height and neither can push the other off the bottom. The transition page does the same.
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

    // A transition added from anywhere, the canvas action, the Design menu, the toolbar or a redo,
    // becomes the selected one, so every route behaves like the `+` button in this tab.
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

    // Undo and redo must re-read even while a field here holds focus. onElementChanged skips a
    // refresh when the author is typing, but Ctrl+Z is not typing and left the labels stale.
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

        // The chip in the row is a summary; the tooltip is where the guard is read, and where a
        // transition that can never fire says so in the validator's own words.
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
    // The list keeps a fixed height whether it holds nothing, one row or ten. Sizing it to content
    // pushed the stimulus row and everything below it down the screen on every add.
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

            // Repeat 0 is continuous and 1 fires once. Spelling it out says what the timer does,
            // where a bare `1000/1` pair asks the reader which number is which.
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

            // An event is a signal, not a call, so its payload is not written as a parameter list.
            // It is named as what the event carries.
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
    // declaration. The status line is where the author is already looking.
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

    // Priority is a position in the state's whole transition list, external ones included, so the
    // move is expressed in that list's indices.
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
    // The row and the signature line follow the pick at once. onElementChanged cannot do it: the
    // picker still holds focus, and rebuilding from a notifier then clobbers typing.
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

    // Where the selection lands after the delete, decided before it: the row above, or the new
    // first row. Simply dropping the selection threw the author to the top of the list.
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
    // The edited transition changed underneath, so its row label and signature line must follow.
    // Not while an editor here has focus: that is the author typing and a rebuild would clobber it.
    QWidget* focus = QApplication::focusWidget();
    const bool editing = (focus != nullptr) && isAncestorOf(focus);
    if ((mStateId != 0u) && (kind == eDocElementKind::Transition) && (id == mCurrentId) && (id != 0u) && (editing == false))
    {
        refresh();
    }
}
