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

#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMFixBar.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMGuardHelpCard.hpp"
#include "lusan/view/sm/SMGuardStatusLine.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

SMGuardBar::SMGuardBar(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mModel    (model)
    , mField    (nullptr)
    , mStatus   (nullptr)
    , mFixBar   (nullptr)
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

    // Reserved slots -- the Structure lens (U3) and Try-it strip (U4) land here.
    QToolButton* lens = new QToolButton(this);
    lens->setObjectName(QStringLiteral("smGuardLens"));
    lens->setText(tr("v Structure"));
    lens->setAutoRaise(true);
    lens->setEnabled(false);
    lens->setToolTip(tr("Structure lens -- Session U3"));
    outer->addWidget(lens);

    QToolButton* tryIt = new QToolButton(this);
    tryIt->setObjectName(QStringLiteral("smGuardTry"));
    tryIt->setText(tr("> Try it"));
    tryIt->setAutoRaise(true);
    tryIt->setEnabled(false);
    tryIt->setToolTip(tr("Try-it strip -- Session U4"));
    outer->addWidget(tryIt);

    outer->addStretch(1);

    connect(mField, &SMGuardField::statusUpdated, this, &SMGuardBar::onStatusUpdated);
    connect(mField, &SMGuardField::fixesUpdated, mFixBar, &SMFixBar::setFixes);
    connect(mField, &SMGuardField::badgeUpdated, this, &SMGuardBar::badgeChanged);
    connect(mFixBar, &SMFixBar::triggered, mField, &SMGuardField::applyFix);
    connect(mClear, &QToolButton::clicked, this, &SMGuardBar::onClearClicked);
    connect(mHelpBtn, &QToolButton::clicked, this, &SMGuardBar::onHelpClicked);
}

void SMGuardBar::setTransition(uint32_t transitionId)
{
    mField->setTransition(transitionId);
}

void SMGuardBar::onStatusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& chips)
{
    if (verdict.isEmpty())
    {
        mStatus->clearStatus();
        return;
    }

    mStatus->setStatus(static_cast<NEGuardStyle::eSeverity>(severity), verdict, preview, chips);
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

    const QPoint pos = mHelpBtn->mapToGlobal(QPoint(0, mHelpBtn->height() + 2));
    mHelp->popupAt(pos);
}
