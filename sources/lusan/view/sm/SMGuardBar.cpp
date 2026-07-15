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
 *  \file        lusan/view/sm/SMGuardBar.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard bar: the Conditions tab content (v7 B1 / S1).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardBar.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardLadder.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMFixBar.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMGuardHelpCard.hpp"
#include "lusan/view/sm/SMGuardStatusLine.hpp"
#include "lusan/view/sm/SMHoverCard.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"
#include "lusan/view/sm/SMMappingGrid.hpp"
#include "lusan/view/sm/SMStructureLens.hpp"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    using eKind = SMGuardNode::eKind;

    //!< Pre-order search for the \p target-th Lambda node; returns true when found.
    bool findNthLambda(const SMGuardNode* node, int target, int& counted, QList<int>& path)
    {
        if (node == nullptr)
        {
            return false;
        }

        if (node->getKind() == eKind::Lambda)
        {
            if (counted == target)
            {
                return true;
            }

            ++counted;
        }

        const QList<SMGuardNode*>& kids = node->getChildren();
        for (int i = 0; i < kids.size(); ++i)
        {
            path.append(i);
            if (findNthLambda(kids.at(i), target, counted, path))
            {
                return true;
            }

            path.removeLast();
        }

        return false;
    }

    //!< Pre-order search for the first Call node (of \p methodId when nonzero).
    bool findFirstCall(const SMGuardNode* node, uint32_t methodId, QList<int>& path)
    {
        if (node == nullptr)
        {
            return false;
        }

        if ((node->getKind() == eKind::Call) && ((methodId == 0u) || (node->getSymbolId() == methodId)))
        {
            return true;
        }

        const QList<SMGuardNode*>& kids = node->getChildren();
        for (int i = 0; i < kids.size(); ++i)
        {
            path.append(i);
            if (findFirstCall(kids.at(i), methodId, path))
            {
                return true;
            }

            path.removeLast();
        }

        return false;
    }

    //!< The copyable handler stub the user pastes into their handler class.
    QString handlerStub(const StateMachineData& data, uint32_t transitionId, const QString& name, const QString& body)
    {
        QString params;
        const QStringList referenced = SMGuardLadder::referencedParams(data, transitionId, body);
        // Parameter types ride with the declaration; the stub restates them by name only.
        for (int i = 0; i < referenced.size(); ++i)
        {
            if (i > 0)
            {
                params += QStringLiteral(", ");
            }

            params += referenced.at(i);
        }

        return QStringLiteral("bool %1(%2)\n{\n    %3\n}\n").arg(name, params, body.trimmed());
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardBar::SMGuardBar(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mModel    (model)
    , mTransId  (0u)
    , mField    (nullptr)
    , mStatus   (nullptr)
    , mFixBar   (nullptr)
    , mIsland   (nullptr)
    , mLens     (nullptr)
    , mGrid     (nullptr)
    , mHover    (nullptr)
    , mHelp     (nullptr)
    , mClear    (nullptr)
    , mHelpBtn  (nullptr)
{
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(6);

    // Header row: Guard label + clear + help.
    QHBoxLayout* header = new QHBoxLayout();
    header->addWidget(new QLabel(tr("Guard"), this));
    header->addStretch(1);

    mClear = new QToolButton(this);
    mClear->setObjectName(QStringLiteral("smGuardClear"));
    mClear->setText(QStringLiteral("{x}"));
    mClear->setAutoRaise(true);
    mClear->setToolTip(tr("Clear the guard (the transition always fires)"));
    header->addWidget(mClear);

    mHelpBtn = new QToolButton(this);
    mHelpBtn->setObjectName(QStringLiteral("smGuardHelp"));
    mHelpBtn->setText(QStringLiteral("(?)"));
    mHelpBtn->setAutoRaise(true);
    mHelpBtn->setToolTip(tr("What can a guard use?"));
    header->addWidget(mHelpBtn);
    outer->addLayout(header);

    mField = new SMGuardField(mModel, this);
    outer->addWidget(mField);

    mStatus = new SMGuardStatusLine(this);
    outer->addWidget(mStatus);

    mFixBar = new SMFixBar(this);
    outer->addWidget(mFixBar);

    // S4: the island editor sits between the status block and the lens (B0 screen map).
    mIsland = new SMIslandEditor(this);
    mIsland->setVisible(false);
    outer->addWidget(mIsland);

    // S5: the structure lens.
    mLens = new SMStructureLens(mModel, this);
    outer->addWidget(mLens);

    // Reserved: the Try-it strip lands in U4.
    QToolButton* tryIt = new QToolButton(this);
    tryIt->setObjectName(QStringLiteral("smGuardTry"));
    tryIt->setText(tr("> Try it"));
    tryIt->setAutoRaise(true);
    tryIt->setEnabled(false);
    tryIt->setToolTip(tr("Try-it strip -- Session U4"));
    outer->addWidget(tryIt);

    outer->addStretch(1);

    // Popovers owned by the bar; both live as top-level tool windows.
    mGrid = new SMMappingGrid(mModel, this);
    mHover = new SMHoverCard(this);
    mField->setHoverCard(mHover);
    mLens->setHoverCard(mHover);

    // ---- U2 wiring: status / fixes / badge --------------------------------
    connect(mField, &SMGuardField::statusUpdated, this, &SMGuardBar::onStatusUpdated);
    connect(mField, &SMGuardField::fixesUpdated, mFixBar, &SMFixBar::setFixes);
    connect(mField, &SMGuardField::badgeUpdated, this, &SMGuardBar::badgeChanged);
    connect(mFixBar, &SMFixBar::triggered, mField, &SMGuardField::applyFix);
    connect(mClear, &QToolButton::clicked, this, &SMGuardBar::onClearClicked);
    connect(mHelpBtn, &QToolButton::clicked, this, &SMGuardBar::onHelpClicked);

    // ---- U3 wiring: islands ------------------------------------------------
    connect(mField, &SMGuardField::islandEditRequested, this, [this](int islandIndex, const QString& body)
    {
        mIsland->openFor(mModel, mTransId, islandIndex, body);
    });
    connect(mIsland, &SMIslandEditor::bodyCommitted, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mField->commitNow();
    });
    connect(mIsland, &SMIslandEditor::closeRequested, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mIsland->hide();
        mField->commitNow();
        mField->setFocus();
    });
    connect(mIsland, &SMIslandEditor::nameRequested, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mField->commitNow();
        runNameIsland(nthIslandPath(islandIndex), body, SMMethodEntry::eImplement::Embedded);
    });
    connect(mIsland, &SMIslandEditor::moveToHandlerRequested, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mField->commitNow();
        runNameIsland(nthIslandPath(islandIndex), body, SMMethodEntry::eImplement::Handler);
    });

    // ---- U3 wiring: lens ----------------------------------------------------
    connect(mLens, &SMStructureLens::caretRequested, mField, &SMGuardField::selectSpan);
    connect(mLens, &SMStructureLens::gridRequested, this, [this](const QList<int>& callPath, const QPoint& globalPos)
    {
        mGrid->openFor(mTransId, callPath, globalPos);
    });
    connect(mLens, &SMStructureLens::clauseTextBuilt, mField, &SMGuardField::appendClause);
    connect(mLens, &SMStructureLens::lambdaAppendRequested, mField, &SMGuardField::appendIsland);
    connect(mLens, &SMStructureLens::nameIslandRequested, this, [this](const QList<int>& islandPath, const QString& body)
    {
        runNameIsland(islandPath, body, SMMethodEntry::eImplement::Embedded);
    });
    connect(mLens, &SMStructureLens::moveToHandlerRequested, this, [this](uint32_t methodId)
    {
        runMoveToHandler(methodId);
    });
    connect(mLens, &SMStructureLens::adoptBodyRequested, this, [this](uint32_t methodId)
    {
        runAdoptBody(methodId);
    });
    connect(mLens, &SMStructureLens::inlineBodyRequested, this, [this](const QList<int>& callPath)
    {
        QUndoCommand* command = SMGuardLadder::inlineBody(mModel.getData(), mModel.getNotifier(), mTransId, callPath, tr("Inline body"));
        if (command != nullptr)
        {
            mModel.getUndoStack().push(command);
        }
    });

    // ---- U3 wiring: hover card + fix-bar grid route --------------------------
    connect(mField, &SMGuardField::mapArgumentsRequested, this, [this]()
    {
        openGridForCall(0u);
    });
    connect(mHover, &SMHoverCard::mapArgsRequested, this, [this](uint32_t symbolId)
    {
        openGridForCall(symbolId);
    });
    connect(mHover, &SMHoverCard::whereUsedRequested, this, [this](uint32_t symbolId)
    {
        showWhereUsed(symbolId);
    });
}

void SMGuardBar::setTransition(uint32_t transitionId)
{
    mTransId = transitionId;
    mIsland->hide();
    mGrid->hide();
    mHover->hide();
    mField->setTransition(transitionId);
    mLens->setTransition(transitionId);
}

//////////////////////////////////////////////////////////////////////////
// U2 slots
//////////////////////////////////////////////////////////////////////////

void SMGuardBar::onStatusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& chips)
{
    if (verdict.isEmpty())
    {
        mStatus->clearStatus();
    }
    else
    {
        mStatus->setStatus(static_cast<NEGuardStyle::eSeverity>(severity), verdict, preview, chips);
    }

    // The B6 two-way rule: a text edit while the grid is open refreshes the grid.
    if ((mGrid != nullptr) && mGrid->isVisible())
    {
        mGrid->refreshLive(mField->committableText());
    }
}

void SMGuardBar::onClearClicked()
{
    const uint32_t transitionId = mField->transitionId();
    if (transitionId == 0u)
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if ((transition == nullptr) || transition->getGuard().isEmpty())
    {
        return;
    }

    SMSetGuardCommand* command = SMGuardCommands::clearGuard(data, mModel.getNotifier(), transitionId, tr("Clear guard"));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }
}

void SMGuardBar::onHelpClicked()
{
    if (mHelp == nullptr)
    {
        mHelp = new SMGuardHelpCard(this);
    }

    mHelp->popupAt(*mHelpBtn);
}

//////////////////////////////////////////////////////////////////////////
// Tree lookups
//////////////////////////////////////////////////////////////////////////

const SMGuardNode* SMGuardBar::guardTree() const
{
    const SMTransitionEntry* transition = (mTransId != 0u) ? mModel.getData().findTransitionById(mTransId) : nullptr;
    return ((transition != nullptr) && transition->getGuard().isOk()) ? transition->getGuard().getTree() : nullptr;
}

QList<int> SMGuardBar::nthIslandPath(int islandIndex) const
{
    QList<int> path;
    int counted = 0;
    if (findNthLambda(guardTree(), islandIndex, counted, path) == false)
    {
        path.clear();
        return path;
    }

    return path;
}

QList<int> SMGuardBar::firstCallPath(uint32_t methodId) const
{
    QList<int> path;
    if (findFirstCall(guardTree(), methodId, path) == false)
    {
        path.clear();
    }

    return path;
}

//////////////////////////////////////////////////////////////////////////
// Ladder flows (one implementation each; lens, island editor, hover share them)
//////////////////////////////////////////////////////////////////////////

void SMGuardBar::runNameIsland(const QList<int>& islandPath, const QString& body, SMMethodEntry::eImplement implement)
{
    const SMGuardNode* tree = guardTree();
    if ((tree == nullptr) || (mTransId == 0u))
    {
        QMessageBox::information(this, tr("Name the lambda"), tr("Commit the guard first -- the island must be part of a resolved guard."));
        return;
    }

    StateMachineData& data = mModel.getData();
    const QStringList params = SMGuardLadder::referencedParams(data, mTransId, body);
    const QString hint = params.isEmpty()
                         ? tr("no stimulus parameters referenced")
                         : tr("parameters (from the body): %1").arg(params.join(QStringLiteral(", ")));

    bool accepted = false;
    const QString title = (implement == SMMethodEntry::eImplement::Embedded) ? tr("Name the lambda") : tr("Move to a handler condition");
    const QString name = QInputDialog::getText(this, title
                                              , tr("Condition name -- %1").arg(hint)
                                              , QLineEdit::Normal, QString(), &accepted).trimmed();
    if ((accepted == false) || name.isEmpty())
    {
        return;
    }

    if (data.getMethods().findMethod(name) != nullptr)
    {
        QMessageBox::warning(this, title, tr("A method named '%1' already exists.").arg(name));
        return;
    }

    SMNameIslandCommand* command = SMGuardLadder::nameIsland(data, mModel.getNotifier(), mTransId, islandPath, name, implement, title);
    if (command == nullptr)
    {
        QMessageBox::warning(this, title, tr("The island could not be located in the committed guard."));
        return;
    }

    mModel.getUndoStack().push(command);
    mIsland->hide();

    if (implement == SMMethodEntry::eImplement::Handler)
    {
        // Lusan stops owning the body: hand it to the user as a copyable stub.
        const QString stub = handlerStub(data, mTransId, name, body);
        QApplication::clipboard()->setText(stub);
        QMessageBox::information(this, title
                                , tr("'%1' is now a handler condition -- your handler implements it.\n\n"
                                     "A stub was copied to the clipboard:\n\n%2").arg(name, stub));
    }
}

void SMGuardBar::runMoveToHandler(uint32_t methodId)
{
    StateMachineData& data = mModel.getData();
    SMMethodEntry* method = data.getMethods().findMethod(methodId);
    if ((method == nullptr) || (method->isLambdaCondition() == false))
    {
        return;
    }

    const QString name = method->getName();
    const QString body = method->getBody();
    QUndoCommand* command = SMGuardLadder::moveToHandler(data, mModel.getNotifier(), methodId, tr("Move to handler"));
    if (command == nullptr)
    {
        return;
    }

    mModel.getUndoStack().push(command);

    const QString stub = handlerStub(data, mTransId, name, body);
    QApplication::clipboard()->setText(stub);
    QMessageBox::information(this, tr("Move to handler")
                            , tr("'%1' is now a handler condition -- Lusan no longer owns the body.\n\n"
                                 "A stub was copied to the clipboard:\n\n%2").arg(name, stub));
}

void SMGuardBar::runAdoptBody(uint32_t methodId)
{
    StateMachineData& data = mModel.getData();
    SMMethodEntry* method = data.getMethods().findMethod(methodId);
    if ((method == nullptr) || (method->isHandlerCondition() == false))
    {
        return;
    }

    bool accepted = false;
    const QString body = QInputDialog::getMultiLineText(this, tr("Adopt body")
                                                       , tr("The boolean body of '%1' (written in Lusan from now on):").arg(method->getName())
                                                       , QStringLiteral("return true;"), &accepted);
    if (accepted == false)
    {
        return;
    }

    QUndoCommand* command = SMGuardLadder::adoptBody(data, mModel.getNotifier(), methodId, body, tr("Adopt body"));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }
}

void SMGuardBar::openGridForCall(uint32_t methodId)
{
    const QList<int> path = firstCallPath(methodId);
    if (path.isEmpty() && (guardTree() == nullptr))
    {
        return;
    }

    const SMGuardNode* tree = guardTree();
    if ((tree == nullptr) || ((path.isEmpty()) && (tree->getKind() != SMGuardNode::eKind::Call)))
    {
        return;
    }

    const QPoint pos = mField->mapToGlobal(QPoint(0, mField->height() + 2));
    mGrid->openFor(mTransId, path, pos);
}

void SMGuardBar::showWhereUsed(uint32_t symbolId)
{
    const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(mModel.getData(), symbolId);
    if (uses.isEmpty())
    {
        QMessageBox::information(this, tr("Where used"), tr("No guard references this symbol."));
        return;
    }

    QMenu menu(this);
    for (const SMGuardWhereUsed::Use& use : uses)
    {
        QAction* action = menu.addAction(use.location);
        const uint32_t transitionId = use.transitionId;
        connect(action, &QAction::triggered, this, [this, transitionId]()
        {
            mModel.getSelectionModel().setSelection({ transitionId });
        });
    }

    menu.exec(QCursor::pos());
}
