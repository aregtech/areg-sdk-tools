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
 *  \file        lusan/view/sm/SMMappingGrid.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard argument-mapping grid popover (v7 B6 / S9).
 *
 ************************************************************************/

#include "lusan/view/sm/SMMappingGrid.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMGuardCatalog.hpp"
#include "lusan/view/sm/SMOperandField.hpp"

#include <QFontDatabase>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
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

    //!< The first Call node of \p symbolId in the tree (pre-order), with its path in \p path.
    const SMGuardNode* findCall(const SMGuardNode* node, uint32_t symbolId, QList<int>& path)
    {
        if (node == nullptr)
        {
            return nullptr;
        }

        if ((node->getKind() == SMGuardNode::eKind::Call) && (node->getSymbolId() == symbolId))
        {
            return node;
        }

        const QList<SMGuardNode*>& kids = node->getChildren();
        for (int i = 0; i < kids.size(); ++i)
        {
            path.append(i);
            const SMGuardNode* found = findCall(kids.at(i), symbolId, path);
            if (found != nullptr)
            {
                return found;
            }

            path.removeLast();
        }

        return nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMMappingGrid::SMMappingGrid(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QFrame    (parent)
    , mModel    (model)
    , mTransId  (0u)
    , mPath     ( )
    , mMethodId (0u)
    , mTitle    (nullptr)
    , mGrid     (nullptr)
    , mGenerated(nullptr)
    , mFields   ( )
    , mStatus   ( )
    , mApplying (false)
{
    setObjectName(QStringLiteral("smMapGrid"));
    // A tool window, NOT Qt::Popup: the field must stay editable while the grid is open
    // (the B6 two-way rule); Esc and Done close it.
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 8, 10, 8);
    outer->setSpacing(6);

    mTitle = new QLabel(this);
    outer->addWidget(mTitle);

    mGrid = new QGridLayout();
    mGrid->setHorizontalSpacing(8);
    mGrid->setVerticalSpacing(4);
    outer->addLayout(mGrid);

    mGenerated = new QLabel(this);
    mGenerated->setObjectName(QStringLiteral("smMapGridGen"));
    mGenerated->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mGenerated->setTextInteractionFlags(Qt::TextSelectableByMouse);
    outer->addWidget(mGenerated);

    QHBoxLayout* footer = new QHBoxLayout();
    footer->addStretch(1);
    QPushButton* done = new QPushButton(tr("Done"), this);
    done->setDefault(true);
    footer->addWidget(done);
    outer->addLayout(footer);

    connect(done, &QPushButton::clicked, this, &QWidget::hide);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMMappingGrid::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMMappingGrid::onElementRemoved);
}

//////////////////////////////////////////////////////////////////////////
// Open / rebuild
//////////////////////////////////////////////////////////////////////////

void SMMappingGrid::openFor(uint32_t transitionId, const QList<int>& callPath, const QPoint& globalPos)
{
    mTransId = transitionId;
    mPath = callPath;
    mMethodId = 0u;

    if (rebuild() == false)
    {
        hide();
        return;
    }

    adjustSize();
    move(globalPos);
    show();
    raise();
}

bool SMMappingGrid::rebuild()
{
    // Clear the previous rows.
    while (mGrid->count() > 0)
    {
        QLayoutItem* item = mGrid->takeAt(0);
        delete item->widget();
        delete item;
    }
    mFields.clear();
    mStatus.clear();

    const StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mTransId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return false;
    }

    const SMGuardNode* call = nodeAt(transition->getGuard().getTree(), mPath);
    if ((call == nullptr) || (call->getKind() != SMGuardNode::eKind::Call))
    {
        return false;
    }

    const SMMethodEntry* method = SMGuardSymbols::method(data, call->getSymbolId());
    if (method == nullptr)
    {
        return false;
    }

    mMethodId = call->getSymbolId();
    mTitle->setText(tr("map arguments -- %1 -- %2")
                    .arg(method->getName())
                    .arg(method->isLambdaCondition() ? tr("lambda") : tr("handler")));

    const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(data, mTransId);
    const QList<MethodParameter>& params = method->getElements();
    for (int i = 0; i < params.size(); ++i)
    {
        const QString paramType = params.at(i).getType();
        QLabel* name = new QLabel(params.at(i).getName() + QStringLiteral(" : ") + paramType, this);
        QLabel* arrow = new QLabel(QStringLiteral("<-"), this);

        SMOperandField* field = new SMOperandField(this);
        field->setObjectName(QStringLiteral("smMapGridField_%1").arg(i));
        field->setSymbols(catalog, paramType);

        const QString argText = (i < call->getCount())
                                ? SMGuardRender::text(data, mTransId, *call->childAt(i))
                                : QString();
        field->setTextSilent(argText);

        QLabel* status = new QLabel(this);
        int severity = 0;
        status->setText(statusFor(argText, paramType, severity));
        status->setStyleSheet(QStringLiteral("color: %1;")
                              .arg(NEGuardStyle::severityColor(static_cast<NEGuardStyle::eSeverity>(severity)).name()));

        mGrid->addWidget(name, i, 0);
        mGrid->addWidget(arrow, i, 1);
        mGrid->addWidget(field, i, 2);
        mGrid->addWidget(status, i, 3);

        mFields.append(field);
        mStatus.append(status);

        connect(field, &SMOperandField::committed, this, [this, i](const QString& text)
        {
            applyOperand(i, text);
        });
    }

    mGenerated->setText(tr("generated: %1").arg(SMGuardCodegenPreview::expression(data, mTransId, *call)));
    return true;
}

void SMMappingGrid::refreshLive(const QString& fieldText)
{
    if ((isVisible() == false) || mApplying || (mMethodId == 0u))
    {
        return;
    }

    SMGuardParser::Result result = SMGuardParser::parse(mModel.getData(), mTransId, fieldText, true);
    if (result.tree == nullptr)
    {
        return;
    }

    QList<int> livePath;
    const SMGuardNode* call = findCall(result.tree, mMethodId, livePath);
    if (call != nullptr)
    {
        const StateMachineData& data = mModel.getData();
        for (int i = 0; (i < mFields.size()); ++i)
        {
            const QString argText = (i < call->getCount())
                                    ? SMGuardRender::text(data, mTransId, *call->childAt(i))
                                    : QString();
            if (mFields.at(i)->hasFocus() == false)
            {
                mFields.at(i)->setTextSilent(argText);
            }
        }

        mGenerated->setText(tr("generated: %1").arg(SMGuardCodegenPreview::expression(data, mTransId, *call)));
    }

    delete result.tree;
}

//////////////////////////////////////////////////////////////////////////
// Edits
//////////////////////////////////////////////////////////////////////////

void SMMappingGrid::applyOperand(int argIndex, const QString& text)
{
    StateMachineData& data = mModel.getData();

    SMGuardParser::Result result = SMGuardParser::parse(data, mTransId, text, false);
    const bool resolved = result.resolved();
    SMGuardNode* node = result.tree;
    result.tree = nullptr;

    if (resolved == false)
    {
        // The grid never stores an unresolved operand (D4): flag the row, keep the tree.
        delete node;
        if (argIndex < mStatus.size())
        {
            mStatus.at(argIndex)->setText(tr("type mismatch"));
            mStatus.at(argIndex)->setStyleSheet(QStringLiteral("color: %1;")
                                                .arg(NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Err).name()));
        }

        return;
    }

    mApplying = true;
    SMSetGuardCommand* command = SMGuardCommands::replaceArg(data, mModel.getNotifier(), mTransId, mPath, argIndex, node, tr("Map argument"));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }

    mApplying = false;

    // Deferred: rebuilding now would delete the operand field that is still emitting
    // the committed signal this call runs in (use-after-free on unwind).
    QTimer::singleShot(0, this, [this]()
    {
        if (isVisible() && (rebuild() == false))
        {
            hide();
        }
    });
}

QString SMMappingGrid::statusFor(const QString& argText, const QString& paramType, int& severity) const
{
    severity = static_cast<int>(NEGuardStyle::eSeverity::Ok);

    // The argument's declared type: a catalog symbol's type, a literal's guessed type,
    // or unknown (unjudgeable -- treated as ok, the SMTypeCompat philosophy).
    QString argType;
    const QString trimmed = argText.trimmed();
    if ((trimmed == QStringLiteral("true")) || (trimmed == QStringLiteral("false")))
    {
        argType = QStringLiteral("bool");
    }
    else if ((trimmed.isEmpty() == false) && trimmed.at(0).isDigit())
    {
        argType = trimmed.contains(QLatin1Char('.')) ? QStringLiteral("double") : QStringLiteral("int32");
    }
    else
    {
        for (const SMGuardSymbol& sym : SMGuardCatalog::build(mModel.getData(), mTransId))
        {
            if (sym.name == trimmed)
            {
                argType = sym.typeText;
                break;
            }
        }
    }

    switch (SMTypeCompat::rank(argType, paramType))
    {
    case SMTypeCompat::eRank::Exact:
        return tr("ok");

    case SMTypeCompat::eRank::Converts:
        return tr("ok, converts");

    case SMTypeCompat::eRank::Narrows:
        severity = static_cast<int>(NEGuardStyle::eSeverity::Warn);
        return tr("(!) narrows");

    case SMTypeCompat::eRank::Mismatch:
        severity = static_cast<int>(NEGuardStyle::eSeverity::Err);
        return tr("type mismatch");

    case SMTypeCompat::eRank::Unknown:
    default:
        return tr("ok");
    }
}

//////////////////////////////////////////////////////////////////////////
// Notifications / keys
//////////////////////////////////////////////////////////////////////////

void SMMappingGrid::onElementChanged(uint32_t id, eDocElementKind /*kind*/)
{
    if ((isVisible() == false) || mApplying || (id != mTransId))
    {
        return;
    }

    // Deferred for the same reason as applyOperand: the notifier may deliver this while
    // one of the grid's own widgets is mid-signal.
    QTimer::singleShot(0, this, [this]()
    {
        if (isVisible() && (rebuild() == false))
        {
            hide();
        }
    });
}

void SMMappingGrid::onElementRemoved(uint32_t id, eDocElementKind /*kind*/)
{
    if (isVisible() && ((id == mTransId) || (id == mMethodId)))
    {
        hide();
    }
}

void SMMappingGrid::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
        return;
    }

    QFrame::keyPressEvent(event);
}
