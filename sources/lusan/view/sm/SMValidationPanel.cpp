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

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QFontMetrics>
#include <QHeaderView>
#include <QTreeWidget>
#include <QStringList>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
    //!< The item roles carrying a finding's navigation target (element ID + kind + check).
    constexpr int RoleElementId{ Qt::UserRole + 1 };
    constexpr int RoleKind     { Qt::UserRole + 2 };
    constexpr int RoleOwner    { Qt::UserRole + 3 };
    constexpr int RoleRule     { Qt::UserRole + 4 };

    /**
     * Marks the rows that are findings. Depth cannot answer that: with a single document open
     * the tree is flattened and the findings are the top-level rows themselves, so a check for
     * "has a parent" would call every one of them a document heading.
     **/
    constexpr int RoleIsFinding{ Qt::UserRole + 5 };

    //!< Table columns: what it is, where it is, what is wrong, and why that is wrong.
    constexpr int ColumnSeverity{ 0 };
    constexpr int ColumnWhere   { 1 };
    constexpr int ColumnMessage { 2 };
    constexpr int ColumnDetail  { 3 };

    using eSev = DocIssue::eSeverity;

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
        QString         where;      //!< Which element the finding blames.
        QString         text;       //!< The finding itself.
        QString         detail;     //!< Why it is a finding, and what resolves it.
        uint32_t        elementId;
        eDocElementKind kind;
        int             rule;       //!< The check that produced it, for the field-level landing.
    };

    /**
     * The message names the symbol
     **/
    QString ruleDetail(int rule, SMIssue::eSeverity severity)
    {
        if (rule > SMValidator::WARNING_RULE_BASE)
        {
            switch (rule - SMValidator::WARNING_RULE_BASE)
            {
            case 1:  return QObject::tr("The state cannot be reached by any transition, so its behaviour never runs.");
            case 2:  return QObject::tr("The state has no way out. Once the machine enters it, it stays there.");
            case 3:  return QObject::tr("An earlier transition on the same stimulus always fires, so this one never gets its turn.");
            case 4:  return QObject::tr("Nothing in the machine uses this declaration. Keep it if you are about to, or remove it.");
            case 5:  return QObject::tr("Only one half of the event is here. An event needs something that sends it and a transition that reacts to it.");
            case 6:  return QObject::tr("Only one half of the timer is here. A timer needs something that starts it and a transition that reacts to it.");
            case 7:  return QObject::tr("The transition reacts to the stimulus and then does nothing with it, so it has no visible effect.");
            case 10: return QObject::tr("History restores the substate the machine left last time, but nothing ever comes back to this state to use it.");
            case 11: return QObject::tr("The inline code block generates nothing. Write the code, or remove the block.");
            default: return QObject::tr("Advisory only. The document still generates.");
            }
        }

        switch (rule)
        {
        case 1:  return QObject::tr("Every machine level needs exactly one Start state; it marks where execution begins.");
        case 2:  return QObject::tr("A level may declare only one Start state, otherwise the entry point is ambiguous.");
        case 3:  return QObject::tr("A Final state is terminal and cannot have outgoing transitions.");
        case 4:  return QObject::tr("Two entries of the SAME kind share this name. Names are unique per kind, so a trigger, an action and a condition may all be called the same, but two triggers may not.");
        case 5:  return QObject::tr("Identifiers must be usable in generated code: a letter or underscore first, then letters, digits or underscores.");
        case 6:  return QObject::tr("The name is referenced here but declared nowhere of that kind. Check the spelling, and check the kind: an action and a trigger of the same name are different declarations.");
        case 7:  return QObject::tr("A transition may only target a state of its own level. Cross-level jumps go through the parent.");
        case 8:  return QObject::tr("Every element ID must be unique in the document; a repeat breaks layout and reference tracking.");
        case 9:  return QObject::tr("Start and Final are pseudo-states: they mark entry and termination and cannot own substates or a submachine.");
        case 10: return QObject::tr("The argument does not match the parameter it is bound to.");
        case 11: return QObject::tr("The call passes a different number of arguments than the declaration takes.");
        case 12: return QObject::tr("A Param reference resolves against the stimulus of its own transition; this stimulus declares no such parameter.");
        case 13: return QObject::tr("The literal cannot be read as a value of the target type.");
        case 14: return QObject::tr("The two operands have no common type, so the comparison has no defined result.");
        case 16: return QObject::tr("The declared type is not in the data-type registry.");
        case 18: return QObject::tr("A submachine belongs on a composite state; Start and Final cannot carry one.");
        case 20: return QObject::tr("The condition row is incomplete: an operator needs both operands.");
        case 21: return QObject::tr("A condition that takes parameters may appear as the LEFT operand only. The right side must be a plain value.");
        case 23: return QObject::tr("The value source and the target disagree; pick a source of a compatible kind.");
        case 24: return QObject::tr("The element refers to itself, directly or through a cycle.");
        default: return (severity == SMIssue::eSeverity::Error)
                            ? QObject::tr("The document will not generate until this is resolved.")
                            : QString();
        }
    }

    /**
     * Names the element a finding blames, so the row says WHERE before it says what. A state
     * or transition is resolved to its own name; a registry entry keeps its kind label,
     * because the message already quotes the name that failed to resolve.
     **/
    QString whereLabel(const StateMachineData& data, uint32_t elementId, eDocElementKind kind, const QString& fallback);

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

    QString whereLabel(const StateMachineData& data, uint32_t elementId, eDocElementKind kind, const QString& fallback)
    {
        if (fallback.isEmpty() == false)
        {
            return fallback;    // the guard engine already knows its own location string
        }

        if (elementId != 0)
        {
            if (const SMStateEntry* state = data.findStateById(elementId))
            {
                return QObject::tr("State '%1'").arg(state->getName());
            }

            if (const SMTransitionEntry* tr = data.findTransitionById(elementId))
            {
                // A transition has no name of its own: it is identified by what it reacts to
                // and where it leads, which is how it is labelled on the canvas.
                const SMStateEntry* target = data.findStateById(tr->getToId());
                const QString stimulus = tr->getStimulus().isEmpty() ? QObject::tr("(initial)") : tr->getStimulus();
                return (target != nullptr)
                        ? QObject::tr("Transition %1 -> %2").arg(stimulus, target->getName())
                        : QObject::tr("Transition %1").arg(stimulus);
            }
        }

        return kindLabel(kind);
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMValidationPanel::SMValidationPanel(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mList             (nullptr)
    , mSummary          (nullptr)
    , mSources          ( )
    , mRebuildPending   (false)
    , mPending          (0)
{
    buildUi();
}

SMValidationPanel::SMValidationPanel(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : SMValidationPanel (parent)
{
    addDocument(model, QString());
}

void SMValidationPanel::buildUi()
{
    setObjectName(QStringLiteral("smValidation"));

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(4, 4, 4, 4);
    outer->setSpacing(2);

    mSummary = new QLabel(this);
    mSummary->setObjectName(QStringLiteral("smValidationSummary"));
    outer->addWidget(mSummary);

    mList = new QTreeWidget(this);
    mList->setObjectName(QStringLiteral("smValidationList"));
    mList->setAlternatingRowColors(true);
    mList->setRootIsDecorated(false);
    mList->setUniformRowHeights(true);
    mList->setAllColumnsShowFocus(true);
    mList->setColumnCount(4);
    mList->setHeaderLabels({ tr("Severity"), tr("Element"), tr("Problem"), tr("Details") });

    // A build-style diagnostic list is scanned, not read: one point smaller than the UI font
    // and a tight row height fit noticeably more findings in the same output strip.
    QFont listFont = mList->font();
    listFont.setPointSizeF(std::max(listFont.pointSizeF() - 1.0, 7.0));
    mList->setFont(listFont);
    const int rowHeight = QFontMetrics(listFont).height() + 2;
    mList->setIconSize(QSize(rowHeight - 4, rowHeight - 4));
    mList->setStyleSheet(QStringLiteral("QTreeView::item { padding: 0px; margin: 0px; }"));
    if (mList->header() != nullptr)
    {
        mList->header()->setFont(listFont);
        mList->header()->setStretchLastSection(true);
        mList->header()->setSectionResizeMode(ColumnSeverity, QHeaderView::ResizeToContents);
        mList->header()->setSectionResizeMode(ColumnWhere, QHeaderView::Interactive);
        mList->header()->setSectionResizeMode(ColumnMessage, QHeaderView::Interactive);
    }

    outer->addWidget(mList);

    connect(mList, &QTreeWidget::itemActivated, this, &SMValidationPanel::onItemActivated);
    connect(mList, &QTreeWidget::itemDoubleClicked, this, &SMValidationPanel::onItemActivated);

    // Findings are quoted into reports and issue trackers, so a selected row copies whole.
    mList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mList->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* copy = new QAction(tr("Copy"), mList);
    copy->setShortcut(QKeySequence::Copy);
    copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copy, &QAction::triggered, this, &SMValidationPanel::copySelection);
    mList->addAction(copy);

    // A tree of documents: the roots carry the document names, so only the leaves indent.
    mList->setRootIsDecorated(true);
}

void SMValidationPanel::copySelection() const
{
    QStringList lines;
    for (const QTreeWidgetItem* item : mList->selectedItems())
    {
        QStringList columns;
        for (int column = ColumnSeverity; column <= ColumnDetail; ++column)
        {
            const QString text = item->text(column);
            if (text.isEmpty() == false)
            {
                columns.append(text);
            }
        }

        lines.append(columns.join(QStringLiteral(" | ")));
    }

    if (lines.isEmpty() == false)
    {
        QApplication::clipboard()->setText(lines.join(QLatin1Char('\n')));
    }
}

int SMValidationPanel::indexOf(const StateMachineModel* model) const
{
    for (int i = 0; i < mSources.size(); ++i)
    {
        if (mSources.at(i).model == model)
        {
            return i;
        }
    }

    return -1;
}

void SMValidationPanel::addDocument(StateMachineModel& model, const QString& name, QObject* owner /*= nullptr*/)
{
    const int existing = indexOf(&model);
    if (existing >= 0)
    {
        mSources[existing].name  = name;
        mSources[existing].owner = owner;
        scheduleRebuild();
        return;
    }

    Source source;
    source.model = &model;
    source.owner = owner;
    source.name  = name;

    // Each document drives its own root: its controller publishes findings, and guard-affecting
    // edits schedule a rebuild. The connections are kept so the root can be dropped cleanly.
    SMValidationController& controller = model.getValidationController();
    source.bindings.append(connect(&controller, &SMValidationController::validationUpdated, this
                                  , [this, &model](const QList<SMIssue>& issues)
    {
        const int index = indexOf(&model);
        if (index >= 0)
        {
            mSources[index].issues = issues;
            scheduleRebuild();
        }
    }));

    DocModelNotifier& notifier = model.getNotifier();
    const auto onChanged = [this]() { scheduleRebuild(); };
    source.bindings.append(connect(&notifier, &DocModelNotifier::elementChanged, this, onChanged));
    source.bindings.append(connect(&notifier, &DocModelNotifier::elementRemoved, this, onChanged));
    source.bindings.append(connect(&notifier, &DocModelNotifier::documentReloaded, this, onChanged));

    // The host normally unbinds a document it closes, but a window can also be destroyed without
    // anyone saying so. The findings of a document that no longer exists must not survive it.
    source.bindings.append(connect(&model, &QObject::destroyed, this, [this]() { purgeClosedDocuments(); }));

    controller.validateNow();
    source.issues = controller.issues();
    mSources.append(source);
    rebuild();
}

void SMValidationPanel::removeDocument(StateMachineModel& model)
{
    const int index = indexOf(&model);
    if (index < 0)
    {
        return;
    }

    for (const QMetaObject::Connection& binding : mSources.at(index).bindings)
    {
        disconnect(binding);
    }

    mSources.removeAt(index);
    rebuild();
}

void SMValidationPanel::purgeClosedDocuments()
{
    bool dropped = false;
    for (int i = mSources.size() - 1; i >= 0; --i)
    {
        if (mSources.at(i).model.isNull() == false)
        {
            continue;
        }

        for (const QMetaObject::Connection& binding : mSources.at(i).bindings)
        {
            disconnect(binding);
        }

        mSources.removeAt(i);
        dropped = true;
    }

    if (dropped)
    {
        rebuild();
    }
}

int SMValidationPanel::documentCount() const
{
    return static_cast<int>(mSources.size());
}

int SMValidationPanel::pendingCount() const
{
    return mPending;
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::refreshNow()
{
    purgeClosedDocuments();
    for (Source& source : mSources)
    {
        source.issues = source.model->getValidationController().issues();
    }

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
    // F8 steps findings, not document roots, so it walks the leaves across every document in
    // tree order -- a root is a heading and can never be a destination.
    QList<QTreeWidgetItem*> leaves;
    for (int i = 0; i < mList->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = mList->topLevelItem(i);
        if (top->data(ColumnSeverity, RoleIsFinding).toBool())
        {
            leaves.append(top);     // flattened single-document tree
            continue;
        }

        for (int j = 0; j < top->childCount(); ++j)
        {
            leaves.append(top->child(j));
        }
    }

    const int count = static_cast<int>(leaves.size());
    if (count == 0)
    {
        return;
    }

    const int current = static_cast<int>(leaves.indexOf(mList->currentItem()));
    const int next = (current < 0)
                        ? (delta > 0 ? 0 : count - 1)
                        : (((current + delta) % count) + count) % count;
    mList->setCurrentItem(leaves.at(next));
    onItemActivated(leaves.at(next), ColumnSeverity);
}

//////////////////////////////////////////////////////////////////////////
// Update slots
//////////////////////////////////////////////////////////////////////////

void SMValidationPanel::onItemActivated(QTreeWidgetItem* item, int /*column*/)
{
    // A document heading carries no element, and only the rows tagged as findings do. Depth is
    // not the test: a single open document is listed flat, and its findings have no parent.
    if ((item == nullptr) || (item->data(ColumnSeverity, RoleIsFinding).toBool() == false))
    {
        return;
    }

    const uint32_t elementId = item->data(ColumnSeverity, RoleElementId).toUInt();
    const eDocElementKind kind = static_cast<eDocElementKind>(item->data(ColumnSeverity, RoleKind).toInt());
    const int rule = item->data(ColumnSeverity, RoleRule).toInt();
    QObject* owner = item->data(ColumnSeverity, RoleOwner).value<QObject*>();

    // The row remembers which window owns it as a plain pointer. Match it against the documents
    // still listed before handing it out: a window closed since the last rebuild leaves rows whose
    // owner no longer exists, and navigating to one of those would follow a dangling pointer.
    if (owner != nullptr)
    {
        bool alive = false;
        for (const Source& source : mSources)
        {
            alive = alive || (source.owner.isNull() == false && source.owner.data() == owner);
        }

        if (alive == false)
        {
            return;
        }
    }

    emit navigateRequestedIn(owner, elementId, kind, rule);
    emit navigateRequested(elementId, kind, rule);
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

    int errors = 0, warnings = 0, infos = 0;
    int live = 0;
    for (const Source& source : mSources)
    {
        live += (source.model.isNull() ? 0 : 1);
    }

    const bool single = (live == 1);

    for (const Source& source : mSources)
    {
        if (source.model.isNull())
        {
            continue;   // the window went away; the source is dropped on the next purge
        }

        // One list from one engine. The guard and mapping checks are part of that run now, so
        // this view no longer knows which checker produced a row, or what a rule number means.
        const StateMachineData& data = source.model->getData();
        QList<Row> rows;
        for (const SMIssue& issue : source.issues)
        {
            Row row;
            row.severity  = issue.severity;
            row.where     = whereLabel(data, issue.elementId, issue.kind, issue.location);
            row.text      = issue.message;
            row.detail    = issue.detail.isEmpty() ? ruleDetail(issue.rule, issue.severity) : issue.detail;
            row.elementId = issue.elementId;
            row.kind      = issue.kind;
            row.rule      = issue.rule;
            rows.append(row);
        }

        // Worst first within a document; equal severities keep their discovery order (document
        // order for the engine). The shared ladder ranks Error highest so a worst-of is a max,
        // hence the descending compare.
        std::stable_sort(rows.begin(), rows.end(), [](const Row& a, const Row& b)
        {
            return static_cast<int>(a.severity) > static_cast<int>(b.severity);
        });

        int docErrors = 0, docWarnings = 0;
        for (const Row& row : rows)
        {
            switch (row.severity)
            {
            case eSev::Error:   ++docErrors;   break;
            case eSev::Warning: ++docWarnings; break;
            default:            ++infos;       break;
            }
        }

        errors   += docErrors;
        warnings += docWarnings;

        // With one document open the root would be a lone parent over every row, so the tree is
        // flattened to the rows themselves; the moment a second document appears, roots name them.
        QTreeWidgetItem* root = nullptr;
        if (single == false)
        {
            root = new QTreeWidgetItem(mList);
            const QString name = source.name.isEmpty() ? tr("Untitled") : source.name;
            const int pending = docErrors + docWarnings;
            root->setFirstColumnSpanned(true);
            root->setText(ColumnSeverity, (pending > 0) ? tr("%1 (%2)").arg(name).arg(pending) : name);
            root->setExpanded(true);
        }

        for (const Row& row : rows)
        {
            QTreeWidgetItem* item = (root != nullptr) ? new QTreeWidgetItem(root) : new QTreeWidgetItem(mList);
            item->setIcon(ColumnSeverity, severityIcon(row.severity));
            item->setText(ColumnSeverity, severityWord(row.severity));
            item->setText(ColumnWhere, row.where);
            item->setText(ColumnMessage, row.text);
            item->setText(ColumnDetail, row.detail);
            item->setData(ColumnSeverity, RoleElementId, row.elementId);
            item->setData(ColumnSeverity, RoleKind, static_cast<int>(row.kind));
            item->setData(ColumnSeverity, RoleOwner, QVariant::fromValue(source.owner.data()));
            item->setData(ColumnSeverity, RoleRule, row.rule);
            item->setData(ColumnSeverity, RoleIsFinding, true);

            // The columns elide; the tooltip carries the finding whole, wherever the pointer is.
            const QString whole = QStringLiteral("%1  %2\n%3").arg(row.where, row.text, row.detail);
            for (int column = ColumnSeverity; column <= ColumnDetail; ++column)
            {
                item->setToolTip(column, whole);
            }
        }
    }

    mList->resizeColumnToContents(ColumnWhere);
    mList->resizeColumnToContents(ColumnMessage);

    if ((errors + warnings + infos) == 0)
    {
        mSummary->setText(tr("No issues."));
    }
    else
    {
        mSummary->setText(tr("%1 error(s), %2 warning(s), %3 info").arg(errors).arg(warnings).arg(infos));
    }

    // Advisory notes are excluded on purpose: the tab badge must mean "something is wrong".
    const int pending = errors + warnings;
    if (pending != mPending)
    {
        mPending = pending;
        emit pendingCountChanged(mPending);
    }
}
