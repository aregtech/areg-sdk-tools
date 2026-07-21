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

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardLadder.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMFixBar.hpp"
#include "lusan/view/sm/SMGuardCallsOutline.hpp"
#include "lusan/view/sm/SMGuardCatalog.hpp"
#include "lusan/view/sm/SMGuardCatalogView.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMGuardHelpCard.hpp"
#include "lusan/view/sm/SMGuardPopout.hpp"
#include "lusan/view/sm/SMGuardStatusLine.hpp"
#include "lusan/view/sm/SMHoverCard.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"
#include "lusan/view/sm/SMStructureLens.hpp"
#include "lusan/view/sm/SMTryStrip.hpp"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPair>
#include <QPlainTextEdit>
#include <QSet>
#include <QShortcut>
#include <QTimer>
#include <QToolBox>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    using eKind = SMGuardNode::eKind;

    // The accordion section indices (QToolBox order).
    constexpr int SectionCalls = 0;
    constexpr int SectionArgs  = 1;
    constexpr int SectionData  = 2;

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

    //!< Pre-order collection of every Call node with its child-index path.
    void collectCalls(const SMGuardNode* node, const QList<int>& path, QList<QPair<QList<int>, const SMGuardNode*>>& out)
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->getKind() == eKind::Call)
        {
            out.append(qMakePair(path, node));
        }

        const QList<SMGuardNode*>& kids = node->getChildren();
        for (int i = 0; i < kids.size(); ++i)
        {
            QList<int> childPath = path;
            childPath.append(i);
            collectCalls(kids.at(i), childPath, out);
        }
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
    : QWidget       (parent)
    , mModel        (model)
    , mTransId      (0u)
    , mField        (nullptr)
    , mStatus       (nullptr)
    , mFixBar       (nullptr)
    , mWarnBar      (nullptr)
    , mIsland       (nullptr)
    , mLens         (nullptr)
    , mTry          (nullptr)
    , mHover        (nullptr)
    , mHelp         (nullptr)
    , mClear        (nullptr)
    , mHelpBtn      (nullptr)
    , mInsertBtn    (nullptr)
    , mPreviewBtn   (nullptr)
    , mDataBtn      (nullptr)
    , mPopoutBtn    (nullptr)
    , mAccordion    (nullptr)
    , mCalls        (nullptr)
    , mArgs         (nullptr)
    , mArgSink      (model)
    , mData         (nullptr)
    , mLastSection  (0)
    , mDerivedPending(false)
    , mPopout       (nullptr)
{
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(6);

    // Header row: Guard label + top strip (Insert/Preview/Data>>/Pop-out) + clear + help.
    QHBoxLayout* header = new QHBoxLayout();
    header->addWidget(new QLabel(tr("Guard"), this));
    header->addStretch(1);

    // The top strip: labeled toolbuttons, horizontal (adds function, not height).
    mInsertBtn = new QToolButton(this);
    mInsertBtn->setObjectName(QStringLiteral("smGuardInsert"));
    mInsertBtn->setText(tr("Insert"));
    mInsertBtn->setAutoRaise(true);
    mInsertBtn->setToolTip(tr("Insert a symbol reference at the caret"));
    header->addWidget(mInsertBtn);

    mPreviewBtn = new QToolButton(this);
    mPreviewBtn->setObjectName(QStringLiteral("smGuardPreview"));
    mPreviewBtn->setText(tr("Preview"));
    mPreviewBtn->setAutoRaise(true);
    mPreviewBtn->setToolTip(tr("Show the generated C++ for this guard"));
    header->addWidget(mPreviewBtn);

    mDataBtn = new QToolButton(this);
    mDataBtn->setObjectName(QStringLiteral("smGuardDataToggle"));
    mDataBtn->setText(tr("Data>>"));
    mDataBtn->setAutoRaise(true);
    mDataBtn->setToolTip(tr("Browse and insert symbols"));
    header->addWidget(mDataBtn);

    mPopoutBtn = new QToolButton(this);
    mPopoutBtn->setObjectName(QStringLiteral("smGuardPopout"));
    mPopoutBtn->setText(tr("Pop-out"));
    mPopoutBtn->setAutoRaise(true);
    mPopoutBtn->setToolTip(tr("Open the guard in a larger editor"));
    header->addWidget(mPopoutBtn);

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

    // The one advisory warning channel (D-WARN): unmapped-argument entries; click jumps to the
    // call. Reuses the SMFixBar widget (icon + message + link buttons), driven by the bar (not
    // the field), so adding a warning rule never adds a widget.
    mWarnBar = new SMFixBar(this);
    mWarnBar->setObjectName(QStringLiteral("smGuardWarnBar"));
    outer->addWidget(mWarnBar);

    // The accordion (spec 10 / design 8.1): Calls (outline) drives the single Arguments table;
    // Data is the symbol catalog. Exactly one section is expanded at a time (QToolBox).
    mAccordion = new QToolBox(this);
    mAccordion->setObjectName(QStringLiteral("smGuardAccordion"));

    mCalls = new SMGuardCallsOutline(mModel, this);
    mArgs = new SMArgMapTable(mModel, this);
    mArgs->setObjectName(QStringLiteral("smGuardArgs"));
    mArgs->setRowStyle(SMArgMapTable::eRowStyle::Detailed);
    mData = new SMGuardCatalogView(mModel, this);

    mAccordion->addItem(mCalls, tr("Calls"));
    mAccordion->addItem(mArgs, tr("Arguments"));
    mAccordion->addItem(mData, tr("Data"));
    outer->addWidget(mAccordion);

    // S4: the island editor sits between the status block and the lens (B0 screen map).
    mIsland = new SMIslandEditor(this);
    mIsland->setVisible(false);
    outer->addWidget(mIsland);

    // S5: the structure lens.
    mLens = new SMStructureLens(mModel, this);
    outer->addWidget(mLens);

    // S6: the Try-it strip (collapsed by default).
    mTry = new SMTryStrip(mModel, this);
    outer->addWidget(mTry);

    outer->addStretch(1);

    // The hover card is the one remaining top-level popover; the mapping popover (SMMappingGrid)
    // is retired in favour of the inline accordion Arguments table.
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

    // F1 anywhere in the Conditions tab opens the help card (B16).
    QShortcut* helpKey = new QShortcut(QKeySequence(Qt::Key_F1), this);
    helpKey->setContext(Qt::WidgetWithChildrenShortcut);
    connect(helpKey, &QShortcut::activated, this, &SMGuardBar::onHelpClicked);

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
    connect(mLens, &SMStructureLens::gridRequested, this, [this](const QList<int>& callPath, const QPoint& /*globalPos*/)
    {
        jumpToCall(callPath);       // the mapping popover is retired: drive the inline Arguments table.
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

    // ---- U4 wiring: Try-it truth tints the lens pills while the strip is open ----
    connect(mTry, &SMTryStrip::truthTintsChanged, mLens, &SMStructureLens::setTruthTints);

    // ---- Top strip ---------------------------------------------------------
    connect(mInsertBtn, &QToolButton::clicked, this, [this]()
    {
        mField->setFocus();
        mField->openCompletion();                   // the Insert entry point: all kinds at the caret.
    });
    connect(mPreviewBtn, &QToolButton::clicked, this, &SMGuardBar::showPreviewDialog);
    connect(mDataBtn, &QToolButton::clicked, this, &SMGuardBar::toggleDataSection);
    connect(mPopoutBtn, &QToolButton::clicked, this, &SMGuardBar::openPopout);

    // ---- Accordion: Calls outline drives the single Arguments table --------
    connect(mCalls, &SMGuardCallsOutline::callSelected, this, &SMGuardBar::onCallSelected);
    connect(mCalls, &SMGuardCallsOutline::islandActivated, this, [this](int islandIndex)
    {
        // Open the island editor for the outline's `raw C++` row (same path as the field).
        mIsland->openFor(mModel, mTransId, islandIndex, mField->islandBody(islandIndex));
    });
    connect(mCalls, &SMGuardCallsOutline::whereUsedRequested, this, [this](uint32_t symbolId)
    {
        showWhereUsed(symbolId);
    });
    connect(mAccordion, &QToolBox::currentChanged, this, [this](int index)
    {
        if (index >= 0)
        {
            mLastSection = index;                   // D-ACCORDION remember-last (per tab).
        }
    });

    // ---- Data catalog: double-click inserts, right-click asks where-used ----
    connect(mData, &SMGuardCatalogView::insertRequested, this, [this](const SMGuardSymbol& symbol)
    {
        mField->insertReference(symbol);
    });
    connect(mData, &SMGuardCatalogView::whereUsedRequested, this, [this](uint32_t symbolId)
    {
        showWhereUsed(symbolId);
    });

    // ---- Warning channel: unmapped-argument jumps to the call; orphan removes the stale binding ----
    connect(mWarnBar, &SMFixBar::triggered, this, [this](const QString& id, const QString& payload)
    {
        // W1 "keep raw" (SM-21-08): silence this token for the rest of the guard, then rebuild the
        // channel without it. The dismissal is advisory UI state, never written to the document.
        if (id == QStringLiteral("keepRaw"))
        {
            mDismissedRaw.insert(payload);
            refreshWarnings();
            return;
        }

        // Payload: `p0,p1,...` (jumpGhost) or `p0,p1,...;formalId` (removeOrphan) or
        // `p0,p1,...;name` (bindRaw). An empty path section is a VALID address (the root node).
        const QString pathText = payload.section(QLatin1Char(';'), 0, 0);
        QList<int> path;
        const QStringList parts = pathText.split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (const QString& part : parts)
        {
            path.append(part.toInt());
        }

        if (id == QStringLiteral("bindRaw"))
        {
            // W1 [bind]: route to the field (single kind binds directly; several kinds open the
            // same completer picker used for bare typing). Never auto-binds an ambiguous name.
            const QString name = payload.section(QLatin1Char(';'), 1, 1);
            mField->bindRaw(path, name);
        }
        else if (id == QStringLiteral("jumpGhost"))
        {
            jumpToCall(path);
        }
        else if (id == QStringLiteral("removeOrphan"))
        {
            const uint32_t formalId = payload.section(QLatin1Char(';'), 1, 1).toUInt();
            SMSetGuardCommand* command = SMGuardCommands::clearArgByFormal(mModel.getData(), mModel.getNotifier(), mTransId, path, formalId, tr("Remove orphaned argument"));
            if (command != nullptr)
            {
                mModel.getUndoStack().push(command);
            }
        }
    });

    // A foreign rename / add / remove changes the catalog names and the use counts with NO guard
    // edit (D-SYNC): re-enumerate the catalog and recompute the derived views off the model pass.
    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind) { scheduleCatalogRefresh(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind) { scheduleCatalogRefresh(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind) { scheduleCatalogRefresh(); });
}

void SMGuardBar::setTransition(uint32_t transitionId)
{
    // A pop-out is bound to one transition; if the bar retargets, close it (its closed() handler
    // restores the base field to editable before the rebind below).
    if (mPopout != nullptr)
    {
        mPopout->close();
    }

    mTransId = transitionId;
    // W1 "keep raw" dismissals are per guard: a fresh transition starts with a clean advisory
    // slate (the set is silenced-token state, never persisted to the document).
    mDismissedRaw.clear();
    mIsland->hide();
    mHover->hide();
    mField->setTransition(transitionId);
    mLens->setTransition(transitionId);
    mTry->setTransition(transitionId);

    // The accordion + catalog follow the transition; the Arguments table clears until a call
    // is selected in the outline.
    mArgs->clearBinding();
    mArgSink.clearBinding();
    mCalls->setTransition(transitionId);
    mData->setTransition(transitionId);

    // D-ACCORDION: re-open the section the user last had open (persisted per tab, not per
    // transition), clamped to a valid index.
    const int section = ((mLastSection >= 0) && (mLastSection < mAccordion->count())) ? mLastSection : SectionCalls;
    mAccordion->setCurrentIndex(section);

    refreshDerived();
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

    // A commit reflows the tree: recompute the catalog use-counts and the warning channel from
    // the committed guard (cheap; the catalog symbols themselves change only on a model edit).
    refreshDerived();
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
    // The mapping popover is retired: `Map arguments...` now drives the inline accordion.
    const SMGuardNode* tree = guardTree();
    if (tree == nullptr)
    {
        return;
    }

    const QList<int> path = firstCallPath(methodId);
    if (path.isEmpty() && (tree->getKind() != SMGuardNode::eKind::Call))
    {
        return;
    }

    jumpToCall(path);
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

//////////////////////////////////////////////////////////////////////////
// Accordion + Arguments (SM-21-02 phase 2)
//////////////////////////////////////////////////////////////////////////

void SMGuardBar::onCallSelected(const QList<int>& callPath, uint32_t methodId, int unmappedCount)
{
    const StateMachineData& data = mModel.getData();
    const SMMethodEntry* method = SMGuardSymbols::method(data, methodId);
    if (method == nullptr)
    {
        // methodId 0 means no call is selected; an empty callPath is still valid (a root call).
        mArgs->clearBinding();
        mArgSink.clearBinding();
        return;
    }

    QList<SMArgMapTable::Param> params;
    for (const MethodParameter& formal : method->getElements())
    {
        params.append(SMArgMapTable::Param{ formal.getName(), formal.getType(), formal.getValue(), formal.hasDefault() });
    }

    mArgSink.bind(mTransId, callPath);
    mArgs->bind(mTransId, true, &mArgSink, params);

    // D-ACCORDION: a call with an unmapped formal auto-opens Arguments on the first ghost slot;
    // a fully-mapped call leaves the currently-open section as-is.
    if (unmappedCount > 0)
    {
        mAccordion->setCurrentIndex(SectionArgs);

        int firstUnmapped = -1;
        for (int i = 0; i < params.size(); ++i)
        {
            if (mArgSink.argFor(params.at(i).name) == nullptr)
            {
                firstUnmapped = i;
                break;
            }
        }

        if ((firstUnmapped >= 0) && (firstUnmapped < mArgs->rowCount()))
        {
            QComboBox* combo = mArgs->sourceCombo(firstUnmapped);
            if (combo != nullptr)
            {
                combo->setFocus();
            }
        }
    }
}

void SMGuardBar::jumpToCall(const QList<int>& callPath)
{
    // An empty path is the guard's root call (a single-call guard) -- still a valid selection.
    mCalls->selectCallPath(callPath);       // emits callSelected -> binds the Arguments table.
    mAccordion->setCurrentIndex(SectionArgs);
}

void SMGuardBar::toggleDataSection()
{
    if (mAccordion->currentIndex() == SectionData)
    {
        mAccordion->setCurrentIndex(SectionCalls);
    }
    else
    {
        mAccordion->setCurrentIndex(SectionData);
        mData->focusSearch();
    }
}

void SMGuardBar::showPreviewDialog()
{
    QString text;
    const SMTransitionEntry* transition = (mTransId != 0u) ? mModel.getData().findTransitionById(mTransId) : nullptr;
    if (transition != nullptr)
    {
        text = SMGuardCodegenPreview::ifStatement(mModel.getData(), mTransId, transition->getGuard());
    }

    if (text.isEmpty())
    {
        text = tr("// no generated code -- the guard is empty or an unresolved draft");
    }

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("smGuardPreviewDialog"));
    dialog.setWindowTitle(tr("Generated guard code"));

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QPlainTextEdit* view = new QPlainTextEdit(&dialog);
    view->setObjectName(QStringLiteral("smGuardPreviewText"));
    view->setReadOnly(true);
    view->setPlainText(text);
    layout->addWidget(view);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);

    dialog.resize(560, 240);
    dialog.exec();
}

//////////////////////////////////////////////////////////////////////////
// Pop-out editor (SM-21-05)
//////////////////////////////////////////////////////////////////////////

SMGuardPopout* SMGuardBar::popout() const
{
    return mPopout;
}

void SMGuardBar::openPopout()
{
    if (mTransId == 0u)
    {
        return;
    }

    if (mPopout != nullptr)
    {
        // Already open: raise and focus it rather than spawning a second window.
        mPopout->raise();
        mPopout->activateWindow();
        return;
    }

    // Seed the pop-out from the base field's committable text: commit the base first (a no-op when
    // it is already committed), so the pop-out's field reflows from the model and opens showing
    // exactly what the base showed. The base field goes read-only while the pop-out owns editing.
    mField->commitNow();
    mField->setReadOnly(true);
    mPopoutBtn->setEnabled(false);

    SMGuardPopout* popout = new SMGuardPopout(mModel, mTransId, this);
    mPopout = popout;

    // The `Name it...` / `Move to handler...` ladder is bar-owned; run it over the shared model
    // (the island must be committed first so it exists in the tree the path is resolved against).
    connect(popout, &SMGuardPopout::nameIslandRequested, this, [this, popout](int islandIndex, const QString& body, bool moveToHandler)
    {
        popout->field()->commitNow();
        const SMMethodEntry::eImplement implement = moveToHandler ? SMMethodEntry::eImplement::Handler
                                                                  : SMMethodEntry::eImplement::Embedded;
        runNameIsland(nthIslandPath(islandIndex), body, implement);
    });

    // On close (OK / Cancel / the window box): the base becomes editable again and regains focus.
    // An OK commit reflowed the base from the model already (elementChanged); a Cancel left it as-is.
    connect(popout, &SMGuardPopout::closed, this, [this]()
    {
        mField->setReadOnly(false);
        mPopoutBtn->setEnabled(true);
        mField->setFocus();
    });

    // Center the pop-out over the bar so it opens where the developer is looking.
    popout->move(mapToGlobal(rect().center()) - popout->rect().center());
    popout->show();
    popout->raise();
    popout->activateWindow();
    popout->field()->setFocus();
}

//////////////////////////////////////////////////////////////////////////
// Use-count + warning channel (SM-21-04 phase 4)
//////////////////////////////////////////////////////////////////////////

void SMGuardBar::refreshDerived()
{
    // The `used-N` column: bound references by symbol id over the committed guard (headless walk).
    mData->setUseCounts(SMGuardCatalog::useCounts(guardTree()));
    refreshWarnings();
}

void SMGuardBar::scheduleCatalogRefresh()
{
    if (mDerivedPending)
    {
        return;
    }

    mDerivedPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mDerivedPending = false;
        mData->refresh();       // re-enumerate the catalog (a foreign rename/add/remove changed names).
        refreshDerived();       // recompute the use-counts + warnings off the same pass.
    });
}

void SMGuardBar::refreshWarnings()
{
    const StateMachineData& data = mModel.getData();

    QList<QPair<QList<int>, const SMGuardNode*>> calls;
    collectCalls(guardTree(), QList<int>(), calls);

    QList<SMFixBar::Fix> fixes;
    for (const QPair<QList<int>, const SMGuardNode*>& entry : calls)
    {
        const SMGuardNode* call = entry.second;
        const SMMethodEntry* method = SMGuardSymbols::method(data, call->getSymbolId());
        if (method == nullptr)
        {
            continue;
        }

        // The path packed as a comma-separated payload the warn-bar click unpacks (jumpGhost).
        QStringList pathParts;
        for (int index : entry.first)
        {
            pathParts.append(QString::number(index));
        }

        const QString pathPayload = pathParts.join(QLatin1Char(','));
        const QList<MethodParameter>& formals = method->getElements();

        QSet<uint32_t> formalIds;
        for (const MethodParameter& formal : formals)
        {
            if (formal.getId() != 0u)
            {
                formalIds.insert(formal.getId());
            }
        }

        // Unmapped formals -> amber "jump to ghost" entries (ghost slot, spec 4.3).
        for (int i = 0; i < formals.size(); ++i)
        {
            const uint32_t formalId = formals.at(i).getId();
            bool mapped = false;
            for (int c = 0; c < call->getCount(); ++c)
            {
                if ((formalId != 0u) && (call->childAt(c)->getArgFormalId() == formalId))
                {
                    mapped = true;
                    break;
                }
            }

            if (mapped == false)
            {
                const SMGuardNode* positional = call->childAt(i);
                if ((positional != nullptr) && (positional->getArgFormalId() == 0u))
                {
                    mapped = true;      // legacy positional arg.
                }
            }

            if (mapped == false)
            {
                SMFixBar::Fix fix;
                fix.id      = QStringLiteral("jumpGhost");
                fix.label   = tr("%1: %2 not mapped").arg(method->getName(), formals.at(i).getName());
                fix.payload = pathPayload;
                fix.tooltip = tr("Jump to the unmapped argument");
                fixes.append(fix);
            }
        }

        // Orphan args -> a bound formal was removed on the Methods page: never silently drop the
        // value (12.9); offer a remove quick-fix that keeps the value until the user acts.
        for (int c = 0; c < call->getCount(); ++c)
        {
            const uint32_t argFormal = call->childAt(c)->getArgFormalId();
            if ((argFormal != 0u) && (formalIds.contains(argFormal) == false))
            {
                SMFixBar::Fix fix;
                fix.id      = QStringLiteral("removeOrphan");
                fix.label   = tr("%1: remove orphaned argument").arg(method->getName());
                fix.payload = pathPayload + QLatin1Char(';') + QString::number(argFormal);
                fix.tooltip = tr("The mapped parameter no longer exists; remove the stale binding");
                fixes.append(fix);
            }
        }
    }

    // W2 (assignment-in-guard) is delivered by the parser's existing D-NOSET warning on the field
    // (`=` in a boolean position parses as `==`); it is not re-derived here to avoid any raw-text
    // scan.
    const bool hasArgFixes = (fixes.isEmpty() == false);

    // W1 (SM-21-08): a raw bare identifier that exactly matches an in-scope symbol name -> a quiet,
    // dismissible "bind as `@kind:name`" courtesy. Advisory only; never auto-binds; "keep raw"
    // silences the token for the rest of this guard (mDismissedRaw). Recomputed every pass, so a
    // rename that removes the collision drops the hint with no guard edit (D-SYNC).
    int rawHintCount = 0;
    const QList<SMGuardRawCollision> collisions = SMGuardCatalog::rawCollisions(data, mTransId, guardTree());
    for (const SMGuardRawCollision& hit : collisions)
    {
        if (mDismissedRaw.contains(hit.name))
        {
            continue;       // "keep raw" already silenced this token for the current guard.
        }

        // The path packed as a comma-separated payload (bindRaw unpacks it back to a node path).
        QStringList pathParts;
        for (int index : hit.path)
        {
            pathParts.append(QString::number(index));
        }

        const QString bindPayload = pathParts.join(QLatin1Char(',')) + QLatin1Char(';') + hit.name;

        SMFixBar::Fix bind;
        bind.id      = QStringLiteral("bindRaw");
        bind.payload = bindPayload;
        if (hit.matches.size() == 1)
        {
            const SMGuardSymbol& only = hit.matches.first();
            const QString noun = only.kindNoun();
            const QString article = QStringLiteral("aeiou").contains(noun.at(0)) ? QStringLiteral("an") : QStringLiteral("a");
            bind.label   = tr("bind '%1' as %2").arg(hit.name, only.mention());
            bind.tooltip = tr("There is %1 %2 named '%3'; bind the raw reference to it.").arg(article, noun, hit.name);
        }
        else
        {
            bind.label   = tr("bind '%1'...").arg(hit.name);
            bind.tooltip = tr("'%1' matches several symbols; pick which one to bind.").arg(hit.name);
        }
        fixes.append(bind);

        SMFixBar::Fix keep;
        keep.id      = QStringLiteral("keepRaw");
        keep.label   = tr("keep '%1' raw").arg(hit.name);
        keep.payload = hit.name;
        keep.tooltip = tr("Leave it as raw C++; do not suggest binding '%1' again.").arg(hit.name);
        fixes.append(keep);

        ++rawHintCount;
    }

    if (fixes.isEmpty())
    {
        mWarnBar->dismiss();
    }
    else
    {
        // One channel, several rules (D-WARN): the message names the dominant rule; each button
        // carries its own self-describing label + tooltip.
        const QString message = hasArgFixes ? tr("unmapped arguments")
                              : (rawHintCount == 1) ? tr("bind suggestion")
                                                    : tr("bind suggestions");
        mWarnBar->setFixes(message, fixes);
    }
}
