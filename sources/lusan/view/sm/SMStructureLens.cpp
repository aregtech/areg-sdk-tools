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
 *  \file        lusan/view/sm/SMStructureLens.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard structure lens (v7 B7 / S5).
 *
 ************************************************************************/

#include "lusan/view/sm/SMStructureLens.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardLadder.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMFlowLayout.hpp"
#include "lusan/view/sm/SMGuardCatalog.hpp"
#include "lusan/view/sm/SMHoverCard.hpp"
#include "lusan/view/sm/SMOperandField.hpp"

#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <functional>

namespace
{
    using eKind = SMGuardNode::eKind;

    //!< The binding precedence, mirroring the canonical render walk.
    int precedence(const SMGuardNode& node)
    {
        switch (node.getKind())
        {
        case eKind::Or:  return 1;
        case eKind::And: return 2;
        case eKind::Cmp: return 3;
        case eKind::Not: return 4;
        default:         return 5;
        }
    }

    //!< The node reached by the child-index \p path, or nullptr.
    const SMGuardNode* nodeAt(const SMGuardNode* root, const QList<int>& path)
    {
        const SMGuardNode* node = root;
        for (int index : path)
        {
            if (node == nullptr) { return nullptr; }
            node = node->childAt(index);
        }

        return node;
    }

    //!< The stable widget-name suffix of a node path (`root`, `0`, `0_2`, ...).
    QString pathName(const QList<int>& path)
    {
        if (path.isEmpty())
        {
            return QStringLiteral("root");
        }

        QStringList parts;
        for (int index : path)
        {
            parts.append(QString::number(index));
        }

        return parts.join(QLatin1Char('_'));
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMStructureLens::SMStructureLens(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mTransitionId     (0u)
    , mToggle           (nullptr)
    , mExplainBtn       (nullptr)
    , mAddBtn           (nullptr)
    , mContent          (nullptr)
    , mFlow             (nullptr)
    , mExplain          (nullptr)
    , mEmptyNote        (nullptr)
    , mHover            (nullptr)
    , mHoverTimer       (nullptr)
    , mHoverPill        (nullptr)
    , mPills            ( )
    , mPillIndex        ( )
    , mLastClicked      ( )
    , mRebuildPending   (false)
{
    setObjectName(QStringLiteral("smGuardLens"));

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(2);

    QHBoxLayout* header = new QHBoxLayout();
    mToggle = new QToolButton(this);
    mToggle->setObjectName(QStringLiteral("smGuardLensToggle"));
    mToggle->setText(tr("v Structure"));
    mToggle->setAutoRaise(true);
    mToggle->setCheckable(true);
    mToggle->setChecked(true);
    header->addWidget(mToggle);
    header->addStretch(1);

    mExplainBtn = new QToolButton(this);
    mExplainBtn->setObjectName(QStringLiteral("smLensExplain"));
    mExplainBtn->setText(tr("explain"));
    mExplainBtn->setAutoRaise(true);
    mExplainBtn->setCheckable(true);
    header->addWidget(mExplainBtn);
    outer->addLayout(header);

    mContent = new QWidget(this);
    mFlow = new SMFlowLayout(mContent, 2, 4, 4);
    outer->addWidget(mContent);

    // The [+ and v] split button lives at the end of the pill flow (B7 element 2).
    mAddBtn = new QToolButton(mContent);
    mAddBtn->setObjectName(QStringLiteral("smLensAdd"));
    mAddBtn->setText(tr("+ and"));
    mAddBtn->setAutoRaise(true);
    mAddBtn->setPopupMode(QToolButton::MenuButtonPopup);

    QMenu* addMenu = new QMenu(mAddBtn);
    QAction* addOr = addMenu->addAction(tr("+ or"));
    QAction* addGroup = addMenu->addAction(tr("+ group"));
    QAction* addLambda = addMenu->addAction(tr("+ lambda"));
    mAddBtn->setMenu(addMenu);

    mEmptyNote = new QLabel(this);
    mEmptyNote->setVisible(false);
    outer->addWidget(mEmptyNote);

    mExplain = new QLabel(this);
    mExplain->setObjectName(QStringLiteral("smLensExplainList"));
    mExplain->setWordWrap(true);
    mExplain->setVisible(false);
    outer->addWidget(mExplain);

    mHoverTimer = new QTimer(this);
    mHoverTimer->setSingleShot(true);
    mHoverTimer->setInterval(300);

    connect(mToggle, &QToolButton::toggled, this, [this](bool open)
    {
        mToggle->setText(open ? tr("v Structure") : tr("> Structure"));
        mContent->setVisible(open);
        mExplain->setVisible(open && mExplainBtn->isChecked());
    });
    connect(mExplainBtn, &QToolButton::toggled, this, [this](bool on)
    {
        if (on)
        {
            rebuildExplain();
        }

        mExplain->setVisible(on && mToggle->isChecked());
    });
    connect(mAddBtn, &QToolButton::clicked, this, [this]()
    {
        openClausePopover(QStringLiteral("&&"), false);
    });
    connect(addOr, &QAction::triggered, this, [this]()
    {
        openClausePopover(QStringLiteral("||"), false);
    });
    connect(addGroup, &QAction::triggered, this, [this]()
    {
        openClausePopover(QStringLiteral("&&"), true);
    });
    connect(addLambda, &QAction::triggered, this, [this]()
    {
        emit lambdaAppendRequested();
    });
    connect(mHoverTimer, &QTimer::timeout, this, [this]()
    {
        const Pill* pill = (mHoverPill != nullptr) ? pillOf(mHoverPill) : nullptr;
        if ((pill == nullptr) || (mHover == nullptr))
        {
            return;
        }

        const QPoint pos = mHoverPill->mapToGlobal(QPoint(0, mHoverPill->height() + 2));
        if (pill->kind == eKind::Call)
        {
            mHover->showCall(mModel, mTransitionId, pill->path, pos);
        }
        else if ((pill->kind == eKind::Attr) || (pill->kind == eKind::Const) || (pill->kind == eKind::Param))
        {
            for (const SMGuardSymbol& sym : SMGuardCatalog::build(mModel.getData(), mTransitionId))
            {
                if (sym.symbolId == pill->symbolId)
                {
                    mHover->showSymbol(mModel, mTransitionId, sym, pos);
                    break;
                }
            }
        }
    });

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMStructureLens::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMStructureLens::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMStructureLens::onDocumentReloaded);
}

void SMStructureLens::setTransition(uint32_t transitionId)
{
    mTransitionId = transitionId;
    mLastClicked.clear();
    rebuild();
}

void SMStructureLens::setHoverCard(SMHoverCard* card)
{
    mHover = card;
}

//////////////////////////////////////////////////////////////////////////
// Notifications
//////////////////////////////////////////////////////////////////////////

void SMStructureLens::onElementChanged(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    // Any change can rename a symbol a pill shows; the lens is read-only, so a full
    // deferred rebuild is always safe.
    scheduleRebuild();
}

void SMStructureLens::onElementRemoved(uint32_t id, eDocElementKind /*kind*/)
{
    if (id == mTransitionId)
    {
        mTransitionId = 0u;
    }

    scheduleRebuild();
}

void SMStructureLens::onDocumentReloaded()
{
    mTransitionId = 0u;
    scheduleRebuild();
}

void SMStructureLens::scheduleRebuild()
{
    if (mRebuildPending)
    {
        return;
    }

    mRebuildPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildPending = false;
        rebuild();
    });
}

//////////////////////////////////////////////////////////////////////////
// Build
//////////////////////////////////////////////////////////////////////////

void SMStructureLens::rebuild()
{
    // The add button survives the flow clear (it is re-appended after the pills).
    mFlow->removeWidget(mAddBtn);
    mFlow->clearItems();
    mPills.clear();
    mPillIndex.clear();
    mHoverPill = nullptr;

    const SMTransitionEntry* transition = (mTransitionId != 0u)
                                          ? mModel.getData().findTransitionById(mTransitionId)
                                          : nullptr;
    const SMGuard* guard = (transition != nullptr) ? &transition->getGuard() : nullptr;
    const SMGuardNode* tree = ((guard != nullptr) && guard->isOk()) ? guard->getTree() : nullptr;

    if (tree != nullptr)
    {
        mEmptyNote->setVisible(false);
        buildNode(*tree, {}, 1);
    }
    else if ((guard != nullptr) && guard->isDraft())
    {
        mEmptyNote->setText(tr("draft -- resolve the guard text to see its structure"));
        mEmptyNote->setVisible(true);
    }
    else
    {
        mEmptyNote->setText((transition != nullptr)
                            ? tr("empty guard -- the transition always fires")
                            : QString());
        mEmptyNote->setVisible(transition != nullptr);
    }

    mFlow->addWidget(mAddBtn);
    mAddBtn->setVisible(transition != nullptr);
    mContent->updateGeometry();

    if (mExplainBtn->isChecked())
    {
        rebuildExplain();
    }
}

void SMStructureLens::buildNode(const SMGuardNode& node, const QList<int>& path, int ctx)
{
    const bool wrap = (precedence(node) < ctx);
    if (wrap)
    {
        addPunct(QStringLiteral("("));
    }

    switch (node.getKind())
    {
    case eKind::And:
    case eKind::Or:
        {
            const QString joiner = (node.getKind() == eKind::And) ? tr("and") : tr("or");
            const int childCtx = (node.getKind() == eKind::And) ? 3 : 2;
            const QList<SMGuardNode*>& kids = node.getChildren();
            for (int i = 0; i < kids.size(); ++i)
            {
                if (i > 0)
                {
                    QToolButton* join = addWord(joiner, QStringLiteral("smLensJoin_") + pathName(path));
                    join->setToolTip(tr("flip the whole group to '%1'")
                                     .arg(node.getKind() == eKind::And ? tr("or") : tr("and")));
                    const QList<int> groupPath = path;
                    connect(join, &QToolButton::clicked, this, [this, groupPath]()
                    {
                        SMSetGuardCommand* command = SMGuardCommands::flipCombinator(mModel.getData(), mModel.getNotifier(), mTransitionId, groupPath, tr("Flip and/or"));
                        if (command != nullptr)
                        {
                            mModel.getUndoStack().push(command);
                        }
                    });
                }

                QList<int> childPath = path;
                childPath.append(i);
                buildNode(*kids.at(i), childPath, childCtx);
            }
        }
        break;

    case eKind::Not:
        {
            QToolButton* word = addWord(tr("not"), QStringLiteral("smLensNot_") + pathName(path));
            word->setToolTip(tr("remove the negation"));
            const QList<int> notPath = path;
            connect(word, &QToolButton::clicked, this, [this, notPath]()
            {
                SMSetGuardCommand* command = SMGuardCommands::setNegated(mModel.getData(), mModel.getNotifier(), mTransitionId, notPath, false, tr("Remove not"));
                if (command != nullptr)
                {
                    mModel.getUndoStack().push(command);
                }
            });

            if (node.getCount() == 1)
            {
                QList<int> childPath = path;
                childPath.append(0);
                buildNode(*node.childAt(0), childPath, 4);
            }
        }
        break;

    default:
        addPill(node, path);
        break;
    }

    if (wrap)
    {
        addPunct(QStringLiteral(")"));
    }
}

void SMStructureLens::addPill(const SMGuardNode& node, const QList<int>& path)
{
    const StateMachineData& data = mModel.getData();

    QString text = SMGuardRender::text(data, mTransitionId, node);
    QString qssClass = QStringLiteral("pill-fsm");
    int owner = clauseOwner(node);

    if (node.getKind() == eKind::Call)
    {
        const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
        const bool isLambda = (method != nullptr) && method->isLambdaCondition();
        text = (isLambda ? QStringLiteral("{} ") : QStringLiteral("h ")) + text;
        qssClass = isLambda ? QStringLiteral("pill-lambda") : QStringLiteral("pill-handler");
    }
    else if (node.getKind() == eKind::Lambda)
    {
        qssClass = QStringLiteral("pill-lambda");
    }
    else if (node.getKind() == eKind::Raw)
    {
        qssClass = QStringLiteral("pill-raw");
    }
    else if (owner == static_cast<int>(NEGuardStyle::eOwner::Stimulus))
    {
        qssClass = QStringLiteral("pill-stim");
    }

    if (text.length() > 32)
    {
        text = text.left(29) + QStringLiteral("...");
    }

    QToolButton* pill = new QToolButton(mContent);
    pill->setObjectName(QStringLiteral("smLensPill_") + pathName(path));
    pill->setText(text);
    pill->setFocusPolicy(Qt::StrongFocus);
    pill->setContextMenuPolicy(Qt::CustomContextMenu);
    pill->setProperty("class", qssClass);
    stylePill(pill, owner, qssClass);

    // The node's caret span in the canonical text, from the same render walk.
    Pill info;
    info.path = path;
    info.kind = node.getKind();
    info.symbolId = node.getSymbolId();
    info.body = (node.getKind() == eKind::Lambda) ? node.getText() : QString();
    info.spanStart = 0;
    info.spanLength = 0;
    info.groupPath = path;
    info.indexInGroup = -1;
    info.groupCount = 0;
    if (path.isEmpty() == false)
    {
        QList<int> parentPath = path;
        info.indexInGroup = parentPath.takeLast();
        const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
        const SMGuardNode* parent = (transition != nullptr) ? nodeAt(transition->getGuard().getTree(), parentPath) : nullptr;
        if ((parent != nullptr) && parent->isGroup())
        {
            info.groupPath = parentPath;
            info.groupCount = parent->getCount();
        }
        else
        {
            info.indexInGroup = -1;
        }
    }

    const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
    if ((transition != nullptr) && transition->getGuard().isOk())
    {
        for (const SMGuardRender::NodeSpan& span : SMGuardRender::nodeSpans(data, mTransitionId, *transition->getGuard().getTree()))
        {
            if (span.path == path)
            {
                info.spanStart = span.start;
                info.spanLength = span.length;
                break;
            }
        }
    }

    mPills.append(info);
    mPillIndex.insert(pill, static_cast<int>(mPills.size()) - 1);
    pill->installEventFilter(this);

    connect(pill, &QToolButton::clicked, this, [this, pill]()
    {
        onPillClicked(pill);
    });
    connect(pill, &QToolButton::customContextMenuRequested, this, [this, pill](const QPoint& pos)
    {
        showPillMenu(pill, pill->mapToGlobal(pos));
    });

    mFlow->addWidget(pill);
}

QToolButton* SMStructureLens::addWord(const QString& text, const QString& objectName)
{
    QToolButton* word = new QToolButton(mContent);
    word->setObjectName(objectName);
    word->setText(text);
    word->setAutoRaise(true);
    word->setCursor(Qt::PointingHandCursor);
    mFlow->addWidget(word);
    return word;
}

void SMStructureLens::addPunct(const QString& text)
{
    QLabel* label = new QLabel(text, mContent);
    mFlow->addWidget(label);
}

void SMStructureLens::stylePill(QToolButton* pill, int owner, const QString& /*qssClass*/) const
{
    // B15 pill contract: 1 px border in the owner hue at 60%, fill at 12%, 4 px radius.
    // Colors come from NEGuardStyle roles (one place), never literals.
    const QColor hue = NEGuardStyle::ownerColor(static_cast<NEGuardStyle::eOwner>(owner));
    QColor border(hue);
    border.setAlphaF(0.6);
    QColor fill(hue);
    fill.setAlphaF(0.12);

    pill->setStyleSheet(QStringLiteral("QToolButton { border: 1px solid rgba(%1,%2,%3,%4); border-radius: 4px;"
                                       " background: rgba(%1,%2,%3,%5); padding: 2px 6px; color: %6; }")
                        .arg(hue.red()).arg(hue.green()).arg(hue.blue())
                        .arg(border.alphaF()).arg(fill.alphaF())
                        .arg(hue.name()));
}

int SMStructureLens::clauseOwner(const SMGuardNode& node) const
{
    switch (node.getKind())
    {
    case eKind::Param:  return static_cast<int>(NEGuardStyle::eOwner::Stimulus);
    case eKind::Attr:
    case eKind::Const:  return static_cast<int>(NEGuardStyle::eOwner::Fsm);
    case eKind::Lambda: return static_cast<int>(NEGuardStyle::eOwner::Fsm);
    case eKind::Raw:    return static_cast<int>(NEGuardStyle::eOwner::Raw);
    case eKind::Lit:    return static_cast<int>(NEGuardStyle::eOwner::Literal);

    case eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(mModel.getData(), node.getSymbolId());
            return static_cast<int>(((method != nullptr) && method->isLambdaCondition())
                                    ? NEGuardStyle::eOwner::Fsm
                                    : NEGuardStyle::eOwner::Handler);
        }

    default:
        for (const SMGuardNode* child : node.getChildren())
        {
            const int owner = clauseOwner(*child);
            if (owner != static_cast<int>(NEGuardStyle::eOwner::Literal))
            {
                return owner;
            }
        }
        return static_cast<int>(NEGuardStyle::eOwner::Literal);
    }
}

//////////////////////////////////////////////////////////////////////////
// Interaction
//////////////////////////////////////////////////////////////////////////

const SMStructureLens::Pill* SMStructureLens::pillOf(QObject* widget) const
{
    const int index = mPillIndex.value(widget, -1);
    return ((index >= 0) && (index < mPills.size())) ? &mPills.at(index) : nullptr;
}

void SMStructureLens::onPillClicked(QToolButton* pillBtn)
{
    const Pill* pill = pillOf(pillBtn);
    if (pill == nullptr)
    {
        return;
    }

    // First click places the caret on the span; a second click on a call pill opens S9.
    if ((pill->kind == eKind::Call) && (mLastClicked == pill->path))
    {
        emit gridRequested(pill->path, pillBtn->mapToGlobal(QPoint(0, pillBtn->height() + 2)));
        return;
    }

    mLastClicked = pill->path;
    emit caretRequested(pill->spanStart, pill->spanLength);
}

void SMStructureLens::showPillMenu(QToolButton* pillBtn, const QPoint& globalPos)
{
    const Pill* pill = pillOf(pillBtn);
    if (pill == nullptr)
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
    if (transition == nullptr)
    {
        return;
    }

    // Negation state: is this pill's direct parent a Not node?
    QList<int> parentPath = pill->path;
    bool negated = false;
    if (parentPath.isEmpty() == false)
    {
        parentPath.takeLast();
        const SMGuardNode* parent = nodeAt(transition->getGuard().getTree(), parentPath);
        negated = (parent != nullptr) && (parent->getKind() == eKind::Not);
    }

    QMenu menu(this);
    QAction* negate = menu.addAction(negated ? tr("Remove negation") : tr("Negate"));
    QAction* wrapGroup = menu.addAction(tr("Wrap in group"));
    menu.addSeparator();

    QAction* nameIt = nullptr;
    QAction* moveHandler = nullptr;
    QAction* adopt = nullptr;
    QAction* inlineHere = nullptr;
    QAction* mapArgs = nullptr;
    if (pill->kind == eKind::Lambda)
    {
        nameIt = menu.addAction(tr("Name it..."));
    }
    else if (pill->kind == eKind::Call)
    {
        mapArgs = menu.addAction(tr("Map arguments..."));
        const SMMethodEntry* method = SMGuardSymbols::method(data, pill->symbolId);
        if ((method != nullptr) && method->isLambdaCondition())
        {
            moveHandler = menu.addAction(tr("Move to handler..."));
            inlineHere = menu.addAction(tr("Inline body here"));
            inlineHere->setEnabled(SMGuardLadder::isSingleReturnBody(method->getBody()));
            if (inlineHere->isEnabled() == false)
            {
                inlineHere->setToolTip(tr("only a single-return body can be inlined"));
            }
        }
        else if ((method != nullptr) && method->isHandlerCondition())
        {
            adopt = menu.addAction(tr("Adopt body..."));
        }
    }

    menu.addSeparator();
    QAction* del = menu.addAction(tr("Delete"));

    QAction* chosen = menu.exec(globalPos);
    if (chosen == nullptr)
    {
        return;
    }

    if (chosen == negate)
    {
        const QList<int>& target = negated ? parentPath : pill->path;
        SMSetGuardCommand* command = SMGuardCommands::setNegated(data, mModel.getNotifier(), mTransitionId, target, negated == false, negated ? tr("Remove not") : tr("Negate clause"));
        if (command != nullptr)
        {
            mModel.getUndoStack().push(command);
        }
    }
    else if (chosen == wrapGroup)
    {
        SMSetGuardCommand* command = SMGuardCommands::wrapInGroup(data, mModel.getNotifier(), mTransitionId, pill->path, eKind::And, tr("Wrap in group"));
        if (command != nullptr)
        {
            mModel.getUndoStack().push(command);
        }
    }
    else if ((nameIt != nullptr) && (chosen == nameIt))
    {
        emit nameIslandRequested(pill->path, pill->body);
    }
    else if ((mapArgs != nullptr) && (chosen == mapArgs))
    {
        emit gridRequested(pill->path, globalPos);
    }
    else if ((moveHandler != nullptr) && (chosen == moveHandler))
    {
        emit moveToHandlerRequested(pill->symbolId);
    }
    else if ((adopt != nullptr) && (chosen == adopt))
    {
        emit adoptBodyRequested(pill->symbolId);
    }
    else if ((inlineHere != nullptr) && (chosen == inlineHere))
    {
        emit inlineBodyRequested(pill->path);
    }
    else if (chosen == del)
    {
        SMSetGuardCommand* command = (pill->indexInGroup >= 0)
                                     ? SMGuardCommands::removeClause(data, mModel.getNotifier(), mTransitionId, pill->groupPath, pill->indexInGroup, tr("Delete clause"))
                                     : SMGuardCommands::clearGuard(data, mModel.getNotifier(), mTransitionId, tr("Clear guard"));
        if (command != nullptr)
        {
            mModel.getUndoStack().push(command);
        }
    }
}

bool SMStructureLens::eventFilter(QObject* watched, QEvent* event)
{
    const Pill* pill = pillOf(watched);
    if (pill == nullptr)
    {
        return QWidget::eventFilter(watched, event);
    }

    switch (event->type())
    {
    case QEvent::KeyPress:
        {
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
            const bool alt = (key->modifiers() & Qt::AltModifier);
            if (alt && ((key->key() == Qt::Key_Up) || (key->key() == Qt::Key_Down)) && (pill->indexInGroup >= 0))
            {
                const int other = pill->indexInGroup + ((key->key() == Qt::Key_Up) ? -1 : 1);
                if ((other >= 0) && (other < pill->groupCount))
                {
                    SMSetGuardCommand* command = SMGuardCommands::reorderClause(mModel.getData(), mModel.getNotifier(), mTransitionId, pill->groupPath, pill->indexInGroup, other, tr("Reorder clauses"));
                    if (command != nullptr)
                    {
                        mModel.getUndoStack().push(command);
                    }
                }

                return true;
            }

            if (key->key() == Qt::Key_Delete)
            {
                SMSetGuardCommand* command = (pill->indexInGroup >= 0)
                                             ? SMGuardCommands::removeClause(mModel.getData(), mModel.getNotifier(), mTransitionId, pill->groupPath, pill->indexInGroup, tr("Delete clause"))
                                             : SMGuardCommands::clearGuard(mModel.getData(), mModel.getNotifier(), mTransitionId, tr("Clear guard"));
                if (command != nullptr)
                {
                    mModel.getUndoStack().push(command);
                }

                return true;
            }
        }
        break;

    case QEvent::Enter:
        if (mHover != nullptr)
        {
            mHover->cancelHide();
            mHoverPill = qobject_cast<QToolButton*>(watched);
            mHoverTimer->start();
        }
        break;

    case QEvent::Leave:
        mHoverTimer->stop();
        if (mHover != nullptr)
        {
            mHover->scheduleHide();
        }
        break;

    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

//////////////////////////////////////////////////////////////////////////
// Clause popover + explain
//////////////////////////////////////////////////////////////////////////

void SMStructureLens::openClausePopover(const QString& combiner, bool grouped)
{
    if (mTransitionId == 0u)
    {
        return;
    }

    QFrame* pop = new QFrame(this, Qt::Popup);
    pop->setObjectName(QStringLiteral("smLensClausePopover"));
    pop->setAttribute(Qt::WA_DeleteOnClose);
    pop->setFrameShape(QFrame::StyledPanel);

    QVBoxLayout* outer = new QVBoxLayout(pop);
    outer->setContentsMargins(10, 8, 10, 8);
    outer->setSpacing(6);
    outer->addWidget(new QLabel(tr("add a clause -- pick or type each side"), pop));

    const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(mModel.getData(), mTransitionId);

    QHBoxLayout* row = new QHBoxLayout();
    SMOperandField* lhs = new SMOperandField(pop);
    lhs->setObjectName(QStringLiteral("smLensClauseLhs"));
    lhs->setSymbols(catalog, QString());

    QComboBox* op = new QComboBox(pop);
    op->setObjectName(QStringLiteral("smLensClauseOp"));
    op->addItems({ tr("(is true)"), QStringLiteral("=="), QStringLiteral("!="), QStringLiteral("<")
                 , QStringLiteral("<="), QStringLiteral(">"), QStringLiteral(">=") });

    SMOperandField* rhs = new SMOperandField(pop);
    rhs->setObjectName(QStringLiteral("smLensClauseRhs"));
    rhs->setSymbols(catalog, QString());
    rhs->setEnabled(false);

    row->addWidget(lhs, 2);
    row->addWidget(op, 1);
    row->addWidget(rhs, 2);
    outer->addLayout(row);

    QLabel* preview = new QLabel(pop);
    preview->setObjectName(QStringLiteral("smLensClausePreview"));
    outer->addWidget(preview);

    QHBoxLayout* footer = new QHBoxLayout();
    footer->addStretch(1);
    QPushButton* insert = new QPushButton(tr("Insert"), pop);
    insert->setEnabled(false);
    footer->addWidget(insert);
    outer->addLayout(footer);

    auto buildText = [lhs, op, rhs, grouped]() -> QString
    {
        const QString left = lhs->currentText().trimmed();
        if (left.isEmpty())
        {
            return QString();
        }

        QString text = left;
        if (op->currentIndex() > 0)
        {
            const QString right = rhs->currentText().trimmed();
            if (right.isEmpty())
            {
                return QString();
            }

            text += QLatin1Char(' ') + op->currentText() + QLatin1Char(' ') + right;
        }

        return grouped ? (QStringLiteral("(") + text + QStringLiteral(")")) : text;
    };
    auto refresh = [buildText, preview, insert]()
    {
        const QString text = buildText();
        preview->setText(text.isEmpty() ? QString() : QStringLiteral("-> ") + text);
        insert->setEnabled(text.isEmpty() == false);
    };

    connect(lhs, &QComboBox::editTextChanged, pop, [refresh](const QString&) { refresh(); });
    connect(rhs, &QComboBox::editTextChanged, pop, [refresh](const QString&) { refresh(); });
    connect(op, QOverload<int>::of(&QComboBox::currentIndexChanged), pop, [rhs, refresh](int index)
    {
        rhs->setEnabled(index > 0);
        refresh();
    });
    connect(insert, &QPushButton::clicked, pop, [this, buildText, combiner, pop]()
    {
        const QString text = buildText();
        pop->close();
        if (text.isEmpty() == false)
        {
            emit clauseTextBuilt(combiner, text);
        }
    });

    pop->adjustSize();
    pop->move(mAddBtn->mapToGlobal(QPoint(0, mAddBtn->height() + 2)));
    pop->show();
    lhs->setFocus();
}

void SMStructureLens::rebuildExplain()
{
    const StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = (mTransitionId != 0u) ? data.findTransitionById(mTransitionId) : nullptr;
    const SMGuardNode* tree = ((transition != nullptr) && transition->getGuard().isOk())
                              ? transition->getGuard().getTree()
                              : nullptr;
    if (tree == nullptr)
    {
        mExplain->setText(tr("nothing to explain -- the guard has no resolved tree"));
        return;
    }

    // One bullet per distinct symbol, rebuilt from the tree (S13).
    QStringList bullets;
    QList<uint32_t> seen;
    std::function<void(const SMGuardNode&)> walk = [&](const SMGuardNode& node)
    {
        switch (node.getKind())
        {
        case eKind::Attr:
            if (seen.contains(node.getSymbolId()) == false)
            {
                seen.append(node.getSymbolId());
                const QString name = SMGuardSymbols::attributeName(data, node.getSymbolId());
                bullets.append(tr("- %1 -- FSM attribute, read as %1().").arg(name));
            }
            break;

        case eKind::Const:
            if (seen.contains(node.getSymbolId()) == false)
            {
                seen.append(node.getSymbolId());
                const QString name = SMGuardSymbols::constantName(data, node.getSymbolId());
                bullets.append(tr("- %1 -- constant, <FsmData>::%1.").arg(name));
            }
            break;

        case eKind::Param:
            if (seen.contains(node.getSymbolId()) == false)
            {
                seen.append(node.getSymbolId());
                const QString name = SMGuardSymbols::paramName(data, mTransitionId, node.getSymbolId());
                bullets.append(tr("- %1 -- stimulus parameter, passed by the trigger.").arg(name));
            }
            break;

        case eKind::Call:
            if (seen.contains(node.getSymbolId()) == false)
            {
                seen.append(node.getSymbolId());
                const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
                if (method != nullptr)
                {
                    bullets.append(method->isLambdaCondition()
                                   ? tr("- %1 -- named lambda; body written in Lusan, generated as std::function member m%1.").arg(method->getName())
                                   : tr("- %1 -- YOUR handler implements this; Lusan generates the declaration.").arg(method->getName()));
                }
            }
            break;

        case eKind::Lambda:
            bullets.append(tr("- { ... } -- inline lambda; body written in Lusan, generated as an IIFE."));
            break;

        case eKind::Raw:
            bullets.append(tr("- raw C++ fragment -- emitted verbatim, checked only by your compiler."));
            break;

        default:
            break;
        }

        for (const SMGuardNode* child : node.getChildren())
        {
            walk(*child);
        }
    };

    walk(*tree);
    mExplain->setText(bullets.join(QLatin1Char('\n')));
}
