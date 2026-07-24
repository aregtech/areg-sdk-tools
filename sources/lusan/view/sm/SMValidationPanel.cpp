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
 *  \brief       Lusan application, FSM document validation results panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMValidationPanel.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/SMValidationController.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include <QApplication>
#include <QLabel>
#include <QListWidget>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
    //!< The item roles carrying a finding's navigation target (element ID + kind).
    constexpr int RoleElementId{ Qt::UserRole + 1 };
    constexpr int RoleKind     { Qt::UserRole + 2 };

    //!< A single severity ladder both engines map onto, ordered worst first for sorting.
    enum class eSev { Error = 0, Warning = 1, Info = 2 };

    eSev fromEngine(SMIssue::eSeverity severity)
    {
        switch (severity)
        {
        case SMIssue::eSeverity::Error:     return eSev::Error;
        case SMIssue::eSeverity::Warning:   return eSev::Warning;
        default:                            return eSev::Info;
        }
    }

    eSev fromGuard(SMGuardValidation::eSeverity severity)
    {
        switch (severity)
        {
        case SMGuardValidation::eSeverity::Error:   return eSev::Error;
        case SMGuardValidation::eSeverity::Warning: return eSev::Warning;
        default:                                    return eSev::Info;
        }
    }

    QString severityWord(eSev severity)
    {
        switch (severity)
        {
        case eSev::Error:   return QObject::tr("Error");
        case eSev::Warning: return QObject::tr("Warning");
        default:            return QObject::tr("Info");
        }
    }

    QIcon severityIcon(eSev severity)
    {
        // A standard icon per severity: the row conveys severity by icon and word, not color alone.
        QStyle::StandardPixmap pixmap = QStyle::SP_MessageBoxInformation;
        if (severity == eSev::Error)        pixmap = QStyle::SP_MessageBoxCritical;
        else if (severity == eSev::Warning) pixmap = QStyle::SP_MessageBoxWarning;
        return QApplication::style()->standardIcon(pixmap);
    }

    //!< One unified row, whatever engine produced it, ordered by severity then discovery.
    struct Row
    {
        eSev            severity;
        QString         text;
        uint32_t        elementId;
        eDocElementKind kind;
    };

    //!< A short, human-readable label for the owning page of an engine finding.
    QString kindLabel(eDocElementKind kind)
    {
        switch (kind)
        {
        case eDocElementKind::State:      return QObject::tr("State");
        case eDocElementKind::Transition: return QObject::tr("Transition");
        case eDocElementKind::Condition:  return QObject::tr("Condition");
        case eDocElementKind::Operation:  return QObject::tr("Operation");
        case eDocElementKind::Method:     return QObject::tr("Method");
        case eDocElementKind::Event:      return QObject::tr("Event");
        case eDocElementKind::Timer:      return QObject::tr("Timer");
        case eDocElementKind::Attribute:  return QObject::tr("Attribute");
        case eDocElementKind::Constant:   return QObject::tr("Constant");
        case eDocElementKind::DataType:   return QObject::tr("Data type");
        case eDocElementKind::Import:     return QObject::tr("Import");
        default:                          return QObject::tr("Machine");
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
    , mEngineIssues     ( )
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

    // The engine runs in the facade's controller; the panel is a consumer of its findings, and
    // rebuilds on both engine updates and guard-affecting model changes.
    SMValidationController& controller = mModel.getValidationController();
    connect(&controller, &SMValidationController::validationUpdated, this, &SMValidationPanel::onEngineIssues);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMValidationPanel::onModelChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMValidationPanel::onModelChanged);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMValidationPanel::onModelChanged);

    controller.validateNow();       // seed mEngineIssues (and schedule the first rebuild).
    mEngineIssues = controller.issues();
    rebuild();
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::refreshNow()
{
    mEngineIssues = mModel.getValidationController().issues();
    rebuild();
}

void SMValidationPanel::focusNextIssue()
{
    step(+1);
}

void SMValidationPanel::focusPreviousIssue()
{
    step(-1);
}

void SMValidationPanel::step(int delta)
{
    const int count = mList->count();
    if (count == 0)
    {
        return;
    }

    const int current = mList->currentRow();
    const int next = (current < 0)
                        ? (delta > 0 ? 0 : count - 1)
                        : (((current + delta) % count) + count) % count;
    mList->setCurrentRow(next);
    onItemActivated(mList->item(next));
}

//////////////////////////////////////////////////////////////////////////
// Update slots
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::onEngineIssues(const QList<SMIssue>& issues)
{
    mEngineIssues = issues;
    scheduleRebuild();
}

void SMValidationPanel::onModelChanged()
{
    scheduleRebuild();
}

void SMValidationPanel::onItemActivated(QListWidgetItem* item)
{
    if (item == nullptr)
    {
        return;
    }

    const uint32_t elementId = item->data(RoleElementId).toUInt();
    const eDocElementKind kind = static_cast<eDocElementKind>(item->data(RoleKind).toInt());
    emit navigateRequested(elementId, kind);
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
    QList<Row> rows;

    for (const SMIssue& issue : mEngineIssues)
    {
        Row row;
        row.severity  = fromEngine(issue.severity);
        row.text      = QStringLiteral("%1: %2").arg(kindLabel(issue.kind), issue.message);
        row.elementId = issue.elementId;
        row.kind      = issue.kind;
        rows.append(row);
    }

    for (const SMGuardValidation::Finding& finding : SMGuardValidation::validate(mModel.getData()))
    {
        Row row;
        row.severity  = fromGuard(finding.severity);
        row.text      = QStringLiteral("%1: %2").arg(finding.location, finding.message);
        row.elementId = finding.transitionId;
        row.kind      = eDocElementKind::Transition;
        rows.append(row);
    }

    // Worst first; equal severities keep their discovery order (document order for the engine).
    std::stable_sort(rows.begin(), rows.end(), [](const Row& a, const Row& b)
    {
        return static_cast<int>(a.severity) < static_cast<int>(b.severity);
    });

    mList->clear();
    int errors = 0, warnings = 0, infos = 0;
    for (const Row& row : rows)
    {
        switch (row.severity)
        {
        case eSev::Error:   ++errors;   break;
        case eSev::Warning: ++warnings; break;
        default:            ++infos;    break;
        }

        QListWidgetItem* item = new QListWidgetItem(severityIcon(row.severity),
                                                    QStringLiteral("%1  %2").arg(severityWord(row.severity), row.text));
        item->setData(RoleElementId, row.elementId);
        item->setData(RoleKind, static_cast<int>(row.kind));
        item->setToolTip(row.text);
        mList->addItem(item);
    }

    if (rows.isEmpty())
    {
        mSummary->setText(tr("No issues."));
    }
    else
    {
        mSummary->setText(tr("%1 error(s), %2 warning(s), %3 info").arg(errors).arg(warnings).arg(infos));
    }
}
