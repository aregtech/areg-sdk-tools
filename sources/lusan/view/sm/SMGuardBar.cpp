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
 *  \brief       Lusan application, FSM guard bar: the Conditions tab content.
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
#include "lusan/view/sm/SMAccordion.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMGuardCallsOutline.hpp"
#include "lusan/view/sm/SMGuardCatalog.hpp"
#include "lusan/view/sm/SMGuardDataPanel.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMGuardHelpCard.hpp"
#include "lusan/view/sm/SMGuardPopout.hpp"
#include "lusan/view/sm/SMGuardStatusLine.hpp"
#include "lusan/view/sm/SMHoverCard.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"
#include "lusan/view/sm/SMSectionChrome.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"
#include "lusan/view/sm/SMTryStrip.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPair>
#include <QPlainTextEdit>
#include <QSet>
#include <QSettings>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    using eKind = SMGuardNode::eKind;

    // The accordion section indices (display order). `Generated` (index 0) comes first, directly
    // under the editor, so the C++ a guard produces reads next to the guard itself.
    constexpr int SectionCalls = 1;
    constexpr int SectionArgs  = 2;
    constexpr int SectionData  = 3;

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
    , mIsland       (nullptr)
    , mTry          (nullptr)
    , mHover        (nullptr)
    , mHelp         (nullptr)
    , mClear        (nullptr)
    , mHelpBtn      (nullptr)
    , mHintBox      (nullptr)
    , mInsertBtn    (nullptr)
    , mPopoutBtn    (nullptr)
    , mChrome       (nullptr)
    , mGenCode      (nullptr)
    , mGenChips     (nullptr)
    , mData         (nullptr)
    , mCalls        (nullptr)
    , mArgs         (nullptr)
    , mArgSink      (model)
    , mDerivedPending(false)
    , mBoundCallValid(false)
    , mPopout       (nullptr)
{

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // The shared chrome carries the title, one jump button per accordion section and the compact
    // toggle; this bar fills its slots and adds the guard-specific icon-only action strip.
    mChrome = new SMSectionChrome(this);
    mChrome->setTitle(tr("Guard"));

    auto makeTool = [this](const QString& name, SMToolIcons::eIcon glyph, const QString& tip)
    {
        QToolButton* button = new QToolButton(this);
        button->setObjectName(name);
        button->setIcon(SMToolIcons::icon(glyph));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(tip);
        return button;
    };

    mInsertBtn  = makeTool(QStringLiteral("smGuardInsert"), SMToolIcons::eIcon::GuardInsert
                          , tr("Insert a symbol reference at the caret"));
    mPopoutBtn  = makeTool(QStringLiteral("smGuardPopout"), SMToolIcons::eIcon::GuardPopout
                          , tr("Open the guard in a larger editor"));
    mClear      = makeTool(QStringLiteral("smGuardClear"), SMToolIcons::eIcon::GuardClear
                          , tr("Clear the guard so the transition always fires"));
    mHelpBtn    = makeTool(QStringLiteral("smGuardHelp"), SMToolIcons::eIcon::GuardHelp
                          , tr("What can a guard use?"));
    mChrome->addHeaderWidget(mInsertBtn);
    mChrome->addHeaderWidget(mPopoutBtn);
    mChrome->addHeaderWidget(mClear);
    mChrome->addHeaderWidget(mHelpBtn);

    mField = new SMGuardField(mModel, this);
    mChrome->addBodyWidget(mField);

    mStatus = new SMGuardStatusLine(this);
    mHintBox = new QCheckBox(tr("Hints"), this);
    mHintBox->setObjectName(QStringLiteral("smGuardHints"));
    mHintBox->setToolTip(tr("Explain a symbol while the pointer rests on it. The (?) button explains the editor itself."));
    QSettings hintSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    mHintBox->setChecked(hintSettings.value(QStringLiteral("sm/guardHints"), true).toBool());
    mField->setHintsEnabled(mHintBox->isChecked());
    connect(mHintBox, &QCheckBox::toggled, this, [this](bool on)
    {
        mField->setHintsEnabled(on);
        if (mPopout != nullptr)
        {
            mPopout->field()->setHintsEnabled(on);
        }

        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue(QStringLiteral("sm/guardHints"), on);
    });

    QWidget* statusRow = new QWidget(this);
    QHBoxLayout* statusBox = new QHBoxLayout(statusRow);
    statusBox->setContentsMargins(0, 0, 0, 0);
    statusBox->setSpacing(6);
    statusBox->addWidget(mHintBox);
    statusBox->addWidget(mStatus, 1);
    mChrome->addBodyWidget(statusRow);

    mChrome->accordion()->setObjectName(QStringLiteral("smGuardAccordion"));

    mCalls = new SMGuardCallsOutline(mModel, this);
    mData  = new SMGuardDataPanel(mModel, this);
    mArgs = new SMArgMapTable(mModel, this);
    mArgs->setObjectName(QStringLiteral("smGuardArgs"));
    mArgs->setRowStyle(SMArgMapTable::eRowStyle::Compact);

    QWidget* generated = new QWidget(this);
    generated->setObjectName(QStringLiteral("smGuardGenerated"));
    QVBoxLayout* genBox = new QVBoxLayout(generated);
    genBox->setContentsMargins(6, 4, 6, 4);
    genBox->setSpacing(2);
    // A read-only code view, not a label
    mGenCode = new QPlainTextEdit(generated);
    mGenCode->setObjectName(QStringLiteral("smGuardGeneratedCode"));
    mGenCode->setReadOnly(true);
    mGenCode->setFrameShape(QFrame::StyledPanel);
    mGenCode->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    mGenCode->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFont mono = mGenCode->font();
    mono.setStyleHint(QFont::Monospace);
    mono.setFamily(QStringLiteral("Consolas"));
    mGenCode->setFont(mono);
    // Three lines of code by default; it grows no further, it scrolls.
    mGenCode->setFixedHeight((QFontMetrics(mono).lineSpacing() * 3) + (mGenCode->frameWidth() * 2) + 8);
    genBox->addWidget(mGenCode);
    mGenChips = new QLabel(generated);
    mGenChips->setObjectName(QStringLiteral("smGuardGeneratedChips"));
    mGenChips->setWordWrap(true);
    mGenChips->hide();
    genBox->addWidget(mGenChips);

    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::GuardPreview), tr("Generated"), generated
                       , tr("Show the C++ code this guard produces"));
    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::GuardConditions), tr("Conditions"), mCalls
                       , tr("Show the condition methods you can insert"));
    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::GuardArguments), tr("Arguments"), mArgs
                       , tr("Map the parameters of the called condition"));
    mChrome->addSection(SMToolIcons::icon(SMToolIcons::eIcon::GuardData), tr("Data"), mData
                       , tr("Browse everything this guard may use, and insert it"));
    mChrome->setCompact(true);
    mChrome->setCurrentSection(SectionCalls);
    outer->addWidget(mChrome);

    // The island editor sits below the accordion.
    mIsland = new SMIslandEditor(this);
    mIsland->setVisible(false);
    mChrome->addFooterWidget(mIsland);

    // The Try-it what-if strip is kept constructed for its logic/tests, but hidden
    mTry = new SMTryStrip(mModel, this);
    mTry->hide();
    mChrome->addFooterWidget(mTry);

    mChrome->addFooterStretch();

    mHover = new SMHoverCard(this);
    mField->setHoverCard(mHover);

    connect(mField, &SMGuardField::statusUpdated, this, &SMGuardBar::onStatusUpdated);
    connect(mField, &SMGuardField::badgeUpdated, this, &SMGuardBar::badgeChanged);
    connect(mField, &SMGuardField::signalNavigateToDefinition, this, &SMGuardBar::signalNavigateToDefinition);

    connect(mField, &SMGuardField::fixesUpdated, this, [this](const QString&, const QList<SMFixBar::Fix>& fixes)
    {
        for (const SMFixBar::Fix& fix : fixes)
        {
            if ((fix.id == QStringLiteral("use")) && fix.enabled)
            {
                mStatus->setSuggestion(fix.id, fix.payload, tr("use %1?").arg(fix.payload));
                return;
            }
        }

        mStatus->setSuggestion(QString(), QString(), QString());
    });
    connect(mStatus, &SMGuardStatusLine::suggestionActivated, this, [this](const QString& fixId, const QString& payload)
    {
        mField->applyFix(fixId, payload);
    });
    connect(mClear, &QToolButton::clicked, this, &SMGuardBar::onClearClicked);
    connect(mHelpBtn, &QToolButton::clicked, this, &SMGuardBar::onHelpClicked);

    // F1 anywhere in the Conditions tab opens the help card.
    QShortcut* helpKey = new QShortcut(QKeySequence(Qt::Key_F1), this);
    helpKey->setContext(Qt::WidgetWithChildrenShortcut);
    connect(helpKey, &QShortcut::activated, this, &SMGuardBar::onHelpClicked);

    // ---- Island wiring ----------------------------------------------------
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

    // ---- Hover card and grid route wiring ---------------------------------
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

    // ---- Top strip --------------------------------------------------------
    connect(mInsertBtn, &QToolButton::clicked, this, [this]()
    {
        mField->setFocus();
        mField->openCompletion();                   // the Insert entry point: all kinds at the caret.
    });
    connect(mPopoutBtn, &QToolButton::clicked, this, &SMGuardBar::openPopout);

    // ---- Accordion: the Conditions outline is a pickup list; double-click inserts a condition ---
    connect(mCalls, &SMGuardCallsOutline::insertRequested, this, &SMGuardBar::insertCondition);
    // The Data catalog inserts ANY symbol kind: a condition goes through the same commit-and-land
    // path as the pickup list; a parameter / attribute / constant is a plain reference.
    connect(mData, &SMGuardDataPanel::insertRequested, this, &SMGuardBar::insertSymbol);
    connect(mData, &SMGuardDataPanel::whereUsedRequested, this, [this](uint32_t symbolId)
    {
        showWhereUsed(symbolId);
    });
    connect(mCalls, &SMGuardCallsOutline::whereUsedRequested, this, [this](uint32_t symbolId)
    {
        showWhereUsed(symbolId);
    });
    // The chrome owns the button<->section sync, the Alt+N jump shortcuts and the compact toggle.
    connect(mChrome, &SMSectionChrome::sectionActivated, this, [this](int index, bool open)
    {
        if (open && (index == SectionData))
        {
            mData->focusSearch();
        }
    });

    // The Arguments table follows the call the caret sits in, or the first call in the guard, so
    // moving the caret in the field re-binds the shared table.
    connect(mField, &QTextEdit::cursorPositionChanged, this, [this]() { syncArgumentsToCaret(); });

    // A foreign rename, add or remove changes the signatures the Arguments rows project without any
    // guard edit, so the shared table re-projects off the model pass.
    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind) { scheduleCatalogRefresh(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind) { scheduleCatalogRefresh(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind) { scheduleCatalogRefresh(); });
}

void SMGuardBar::setTransition(uint32_t transitionId)
{
    if (mPopout != nullptr)
    {
        mPopout->close();
    }

    mTransId = transitionId;
    mBoundCallValid = false;
    mBoundCallPath.clear();
    mIsland->hide();
    mHover->hide();
    mField->setTarget(transitionId);
    mTry->setTransition(transitionId);       // collapses the hidden strip.

    // The Arguments table clears until the caret sits in a call (or a call exists in the guard).
    mArgs->clearBinding();
    mArgSink.clearBinding();
    mCalls->setTransition(transitionId);
    mData->setTransition(transitionId);

    // Re-open the section the user last had open (persisted per tab, not per
    // transition), clamped to a valid index.
    const int last = mChrome->lastSection();
    const int section = ((last >= 0) && (last < mChrome->accordion()->count())) ? last : SectionCalls;
    mChrome->setCurrentSection(section);

    scheduleCatalogRefresh();       // bind the Arguments table once the field has reflowed the tree.
}

//////////////////////////////////////////////////////////////////////////
// Status slots
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

    // The generated C++ and the handler list now live in their own accordion section, so the status
    // line can stay a single short verdict.
    mGenCode->setPlainText(preview.isEmpty()
                           ? tr("// no generated code: the guard is empty or an unresolved draft")
                           : preview);
    if (chips.isEmpty())
    {
        mGenChips->hide();
    }
    else
    {
        const QColor color = NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Handler);
        mGenChips->setText(QStringLiteral("<span style='color:%1;'>uses handler:</span> %2")
                              .arg(color.name(), chips.join(QStringLiteral(", ")).toHtmlEscaped()));
        mGenChips->show();
    }

    // A commit reflows the tree: re-bind the Arguments table to the caret's call (deferred so it
    // never runs inside the field's own signal).
    scheduleCatalogRefresh();
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
        QMessageBox::information(this, tr("Name the lambda"), tr("Commit the guard first. The island has to be part of a resolved guard before it can be named."));
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
                                              , tr("Condition name (%1)").arg(hint)
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
                                , tr("'%1' is now a handler condition, so your handler implements it.\n\n"
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
                            , tr("'%1' is now a handler condition, so Lusan no longer owns the body.\n\n"
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
    // The mapping popover is retired: opening a call's arguments now binds the inline accordion
    // Arguments table to that call and shows the Arguments section.
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

    const SMGuardNode* call = nodeAtPath(path);
    if (call != nullptr)
    {
        bindArgumentsTo(path, call->getSymbolId());
        mChrome->setCurrentSection(SectionArgs);
    }
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
        // Selecting by the OWNING element's id: a Do stop condition lives on its state page,
        // and selecting a transition id that is really a state id would land nowhere.
        const uint32_t ownerId = use.target.getId();
        connect(action, &QAction::triggered, this, [this, ownerId]()
        {
            mModel.getSelectionModel().setSelection({ ownerId });
        });
    }

    menu.exec(QCursor::pos());
}

//////////////////////////////////////////////////////////////////////////
// Accordion + Arguments
//////////////////////////////////////////////////////////////////////////

const SMGuardNode* SMGuardBar::nodeAtPath(const QList<int>& path) const
{
    const SMGuardNode* node = guardTree();
    for (int index : path)
    {
        if ((node == nullptr) || (index < 0) || (index >= node->getCount()))
        {
            return nullptr;
        }

        node = node->childAt(index);
    }

    return node;
}

void SMGuardBar::bindArgumentsTo(const QList<int>& callPath, uint32_t methodId)
{
    const SMMethodEntry* method = SMGuardSymbols::method(mModel.getData(), methodId);
    if (method == nullptr)
    {
        mArgs->clearBinding();
        mArgSink.clearBinding();
        mBoundCallValid = false;
        mBoundCallPath.clear();
        return;
    }

    QList<SMArgMapTable::Param> params;
    for (const MethodParameter& formal : method->getElements())
    {
        params.append(SMArgMapTable::Param{ formal.getName(), formal.getType(), formal.getValue(), formal.hasDefault() });
    }

    mArgSink.bind(mTransId, callPath);
    mArgs->bind(mTransId, true, &mArgSink, params);
    mBoundCallPath  = callPath;
    mBoundCallValid = true;
}

void SMGuardBar::syncArgumentsToCaret()
{
    if (mTransId == 0u)
    {
        return;
    }

    QList<QPair<QList<int>, const SMGuardNode*>> calls;
    collectCalls(guardTree(), QList<int>(), calls);
    if (calls.isEmpty())
    {
        mArgs->clearBinding();
        mArgSink.clearBinding();
        mBoundCallValid = false;
        mBoundCallPath.clear();
        return;
    }

    // Default target: the single (or first) call in the guard -- the common case is one condition.
    QList<int>  targetPath   = calls.first().first;
    uint32_t    targetMethod = calls.first().second->getSymbolId();

    // With more than one call, follow the call the caret sits in (matched by callee name).
    if (calls.size() > 1)
    {
        QString callee;
        if (mField->caretCallee(callee) && (callee.isEmpty() == false))
        {
            for (const QPair<QList<int>, const SMGuardNode*>& entry : calls)
            {
                const SMMethodEntry* method = SMGuardSymbols::method(mModel.getData(), entry.second->getSymbolId());
                if ((method != nullptr) && (method->getName() == callee))
                {
                    targetPath   = entry.first;
                    targetMethod = entry.second->getSymbolId();
                    break;
                }
            }
        }
        else if (mBoundCallValid)
        {
            // Caret is not inside a call: keep the current binding if it still addresses a call.
            const SMGuardNode* current = nodeAtPath(mBoundCallPath);
            if ((current != nullptr) && (current->getKind() == SMGuardNode::eKind::Call))
            {
                targetPath   = mBoundCallPath;
                targetMethod = current->getSymbolId();
            }
        }
    }

    const SMMethodEntry* boundMethod = SMGuardSymbols::method(mModel.getData(), targetMethod);
    const int formalCount = (boundMethod != nullptr) ? static_cast<int>(boundMethod->getElements().size()) : 0;
    if (mBoundCallValid && (targetPath == mBoundCallPath) && (mArgs->rowCount() == formalCount))
    {
        mArgs->refresh();
        return;
    }

    bindArgumentsTo(targetPath, targetMethod);
}

void SMGuardBar::insertSymbol(const SMGuardSymbol& symbol)
{
    if (symbol.isCall)
    {
        insertCondition(symbol);
        return;
    }

    if (mTransId == 0u)
    {
        return;
    }

    mField->setFocus();
    mField->insertReference(symbol);
    mField->commitNow();
}

void SMGuardBar::insertCondition(const SMGuardSymbol& symbol)
{
    // The Conditions pickup list chose a condition method
    if (mTransId == 0u)
    {
        return;
    }

    mField->setFocus();
    mField->insertReference(symbol);
    mField->commitNow();
    mField->reflowNow();        // the chip appears within the gesture, not a turn later

    mField->selectFirstGhost();

    // Show the Arguments section so the developer can map the just-inserted call's parameters.
    mChrome->setCurrentSection(SectionArgs);
    syncArgumentsToCaret();
}

//////////////////////////////////////////////////////////////////////////
// Pop-out editor
//////////////////////////////////////////////////////////////////////////

SMGuardPopout* SMGuardBar::popout() const
{
    return mPopout;
}

SMAccordion* SMGuardBar::accordion() const
{
    return mChrome->accordion();
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

    mField->commitNow();
    mField->setReadOnly(true);
    mPopoutBtn->setEnabled(false);

    SMGuardPopout* popout = new SMGuardPopout(mModel, mTransId, this);
    mPopout = popout;
    popout->field()->setHintsEnabled(mHintBox->isChecked());   // one preference, both surfaces

    connect(popout, &SMGuardPopout::nameIslandRequested, this, [this, popout](int islandIndex, const QString& body, bool moveToHandler)
    {
        popout->field()->commitNow();
        const SMMethodEntry::eImplement implement = moveToHandler ? SMMethodEntry::eImplement::Handler
                                                                  : SMMethodEntry::eImplement::Embedded;
        runNameIsland(nthIslandPath(islandIndex), body, implement);
    });

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
// Use-count + warning channel
//////////////////////////////////////////////////////////////////////////

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
        syncArgumentsToCaret();
    });
}
