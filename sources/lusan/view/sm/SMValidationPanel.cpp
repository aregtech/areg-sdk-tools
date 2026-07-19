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
 *  \file        lusan/view/sm/SMValidationPanel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document validation results panel (v7 B12 / S15).
 *
 ************************************************************************/

#include "lusan/view/sm/SMValidationPanel.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
    //!< The item role carrying the finding's transition ID.
    constexpr int RoleTransition{ Qt::UserRole + 1 };

    QString severityWord(SMGuardValidation::eSeverity severity)
    {
        switch (severity)
        {
        case SMGuardValidation::eSeverity::Error:   return QStringLiteral("ERR ");
        case SMGuardValidation::eSeverity::Warning: return QStringLiteral("WARN");
        case SMGuardValidation::eSeverity::Info:
        default:                                    return QStringLiteral("INFO");
        }
    }

    QColor severityColor(SMGuardValidation::eSeverity severity)
    {
        switch (severity)
        {
        case SMGuardValidation::eSeverity::Error:   return NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Err);
        case SMGuardValidation::eSeverity::Warning: return NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Warn);
        case SMGuardValidation::eSeverity::Info:
        default:                                    return NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Ok);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMValidationPanel::SMValidationPanel(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mList             (nullptr)
    , mSummary          (nullptr)
    , mRebuildPending   (false)
{
    setObjectName(QStringLiteral("smValidation"));

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(4, 4, 4, 4);
    outer->setSpacing(2);

    mSummary = new QLabel(this);
    mSummary->setObjectName(QStringLiteral("smValidationSummary"));
    outer->addWidget(mSummary);

    mList = new QListWidget(this);
    mList->setObjectName(QStringLiteral("smValidationList"));
    mList->setAlternatingRowColors(true);
    outer->addWidget(mList);

    connect(mList, &QListWidget::itemActivated, this, &SMValidationPanel::onItemActivated);
    connect(mList, &QListWidget::itemDoubleClicked, this, &SMValidationPanel::onItemActivated);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMValidationPanel::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMValidationPanel::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMValidationPanel::onDocumentReloaded);

    rebuild();
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::refreshNow()
{
    rebuild();
}

//////////////////////////////////////////////////////////////////////////
// Model change slots
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::onElementChanged(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMValidationPanel::onElementRemoved(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMValidationPanel::onDocumentReloaded()
{
    scheduleRebuild();
}

void SMValidationPanel::onItemActivated(QListWidgetItem* item)
{
    const uint32_t transitionId = (item != nullptr) ? item->data(RoleTransition).toUInt() : 0u;
    if (transitionId != 0u)
    {
        emit navigateRequested(transitionId);
    }
}

//////////////////////////////////////////////////////////////////////////
// Build
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::scheduleRebuild()
{
    if (mRebuildPending)
    {
        return;
    }

    // Deferred: never rebuild inside the emitting command / notifier slot.
    mRebuildPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildPending = false;
        rebuild();
    });
}

void SMValidationPanel::rebuild()
{
    mList->clear();

    const QList<SMGuardValidation::Finding> findings = SMGuardValidation::validate(mModel.getData());
    int errors = 0;
    int warnings = 0;
    int infos = 0;
    for (const SMGuardValidation::Finding& finding : findings)
    {
        switch (finding.severity)
        {
        case SMGuardValidation::eSeverity::Error:   ++errors;   break;
        case SMGuardValidation::eSeverity::Warning: ++warnings; break;
        case SMGuardValidation::eSeverity::Info:
        default:                                    ++infos;    break;
        }

        QListWidgetItem* item = new QListWidgetItem(QStringLiteral("%1  %2: %3")
                                                        .arg(severityWord(finding.severity), finding.location, finding.message));
        item->setForeground(severityColor(finding.severity));
        item->setData(RoleTransition, finding.transitionId);
        item->setToolTip(finding.message);
        mList->addItem(item);
    }

    if (findings.isEmpty())
    {
        mSummary->setText(tr("No guard findings."));
    }
    else
    {
        mSummary->setText(tr("%1 error(s), %2 warning(s), %3 info").arg(errors).arg(warnings).arg(infos));
    }
}
