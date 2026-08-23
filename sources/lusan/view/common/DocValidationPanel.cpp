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
 *  \file        lusan/view/common/DocValidationPanel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Document validation results panel.
 *
 ************************************************************************/

#include "lusan/view/common/DocValidationPanel.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/model/common/DocRules.hpp"
#include "lusan/model/common/DocValidationController.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QMainWindow>
#include <QStatusBar>
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

    //!< A short, human-readable label for the owning page of a finding, used when the document
    //!< itself has nothing finer to say about the element.
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
        case eDocElementKind::Include:    return QObject::tr("Include");
        case eDocElementKind::Import:     return QObject::tr("Import");
        case eDocElementKind::Overview:   return QObject::tr("Overview");
        default:                          return QObject::tr("Document");
        }
    }

    /**
     * Names the element a finding blames, so the row says where before it says what. The check
     * may already carry its own location; failing that the document is asked to name the element,
     * and failing that the kind is the answer -- those messages quote the name themselves.
     **/
    QString whereLabel(const IEDocumentModel& document, uint32_t elementId, eDocElementKind kind, const QString& fallback)
    {
        if (fallback.isEmpty() == false)
        {
            return fallback;
        }

        if (elementId != 0)
        {
            const QString described = document.describeElement(elementId, kind);
            if (described.isEmpty() == false)
            {
                return described;
            }
        }

        return kindLabel(kind);
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

DocValidationPanel::DocValidationPanel(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mList             (nullptr)
    , mSummary          (nullptr)
    , mSources          ( )
    , mRebuildPending   (false)
    , mPending          (0)
{
    buildUi();
}

DocValidationPanel::DocValidationPanel(IEDocumentModel& document, QWidget* parent /*= nullptr*/)
    : DocValidationPanel (parent)
{
    addDocument(document, QString());
}

void DocValidationPanel::buildUi()
{
    setObjectName(QStringLiteral("docValidation"));

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(4, 4, 4, 4);
    outer->setSpacing(2);

    mSummary = new QLabel(this);
    mSummary->setObjectName(QStringLiteral("docValidationSummary"));
    outer->addWidget(mSummary);

    mList = new QTreeWidget(this);
    mList->setObjectName(QStringLiteral("docValidationList"));
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

    connect(mList, &QTreeWidget::itemActivated, this, &DocValidationPanel::onItemActivated);
    connect(mList, &QTreeWidget::itemDoubleClicked, this, &DocValidationPanel::onItemActivated);

    // Findings are quoted into reports and issue trackers, so a selected row copies whole.
    mList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mList->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* copy = new QAction(tr("Copy"), mList);
    copy->setShortcut(QKeySequence::Copy);
    copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copy, &QAction::triggered, this, &DocValidationPanel::copySelection);
    mList->addAction(copy);

    // A tree of documents: the roots carry the document names, so only the leaves indent.
    mList->setRootIsDecorated(true);
}

void DocValidationPanel::copySelection() const
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

int DocValidationPanel::indexOf(const IEDocumentModel* document) const
{
    for (int i = 0; i < mSources.size(); ++i)
    {
        const Source& source = mSources.at(i);
        if ((source.controller.isNull() == false) && (&source.controller->getDocument() == document))
        {
            return i;
        }
    }

    return -1;
}

void DocValidationPanel::addDocument(IEDocumentModel& document, const QString& name, QObject* owner /*= nullptr*/)
{
    const int existing = indexOf(&document);
    if (existing >= 0)
    {
        mSources[existing].name  = name;
        mSources[existing].owner = owner;
        scheduleRebuild();
        return;
    }

    // Each document drives its own root: its controller publishes findings, and edits that reach
    // beyond one element schedule a rebuild. The connections are kept so the root drops cleanly.
    DocValidationController& controller = document.getValidationController();

    Source source;
    source.controller = &controller;
    source.owner = owner;
    source.name  = name;

    source.bindings.append(connect(&controller, &DocValidationController::validationUpdated, this
                                  , [this, &document](const QList<DocIssue>& issues)
    {
        const int index = indexOf(&document);
        if (index >= 0)
        {
            mSources[index].issues = issues;
            scheduleRebuild();
        }
    }));

    DocModelNotifier& notifier = document.getNotifier();
    const auto onChanged = [this]() { scheduleRebuild(); };
    source.bindings.append(connect(&notifier, &DocModelNotifier::elementChanged, this, onChanged));
    source.bindings.append(connect(&notifier, &DocModelNotifier::elementRemoved, this, onChanged));
    source.bindings.append(connect(&notifier, &DocModelNotifier::documentReloaded, this, onChanged));

    // The host normally unbinds a document it closes, but a window can also be destroyed without
    // anyone saying so. The findings of a document that no longer exists must not survive it.
    source.bindings.append(connect(&controller, &QObject::destroyed, this, [this]() { purgeClosedDocuments(); }));

    controller.validateNow();
    source.issues = controller.issues();
    mSources.append(source);
    rebuild();
}

void DocValidationPanel::removeDocument(IEDocumentModel& document)
{
    const int index = indexOf(&document);
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

void DocValidationPanel::purgeClosedDocuments()
{
    bool dropped = false;
    for (int i = mSources.size() - 1; i >= 0; --i)
    {
        if (mSources.at(i).controller.isNull() == false)
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

int DocValidationPanel::documentCount() const
{
    return static_cast<int>(mSources.size());
}

int DocValidationPanel::pendingCount() const
{
    return mPending;
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void DocValidationPanel::refreshNow()
{
    purgeClosedDocuments();
    for (Source& source : mSources)
    {
        source.issues = source.controller->issues();
    }

    rebuild();
}

void DocValidationPanel::focusNextIssue()
{
    step(+1);
}

void DocValidationPanel::focusPreviousIssue()
{
    step(-1);
}

void DocValidationPanel::step(int delta)
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

void DocValidationPanel::onItemActivated(QTreeWidgetItem* item, int /*column*/)
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

    // The block and the attribute the format does not define are not built into the model, so
    // there is nothing to select. The status line says so instead of leaving the click silent.
    const int bareRule = DocRuleChecks::bareRule(rule);
    if ((bareRule == DocRules::RULE_UNKNOWN_ATTRIBUTE) || (bareRule == DocRules::RULE_DROPPED_ELEMENT))
    {
        if (QMainWindow* host = qobject_cast<QMainWindow*>(window()))
        {
            host->statusBar()->showMessage(tr("This element cannot be displayed in the editor."), 4000);
        }

        return;
    }

    // The row remembers its owning window as a plain pointer, so match it against the documents
    // still listed: a window closed since the last rebuild leaves rows with a dangling owner.
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

void DocValidationPanel::scheduleRebuild()
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

void DocValidationPanel::rebuild()
{
    mList->clear();

    int errors = 0, warnings = 0, infos = 0;
    int live = 0;
    for (const Source& source : mSources)
    {
        live += (source.controller.isNull() ? 0 : 1);
    }

    const bool single = (live == 1);

    for (const Source& source : mSources)
    {
        if (source.controller.isNull())
        {
            continue;   // the window went away; the source is dropped on the next purge
        }

        // One list per document, from that document's own engine. Every row arrives with its
        // severity, its message and the reason behind it, so this view knows neither which
        // checker produced a row nor what a rule number means.
        const IEDocumentModel& document = source.controller->getDocument();
        QList<Row> rows;
        for (const DocIssue& issue : source.issues)
        {
            Row row;
            row.severity  = issue.severity;
            row.where     = whereLabel(document, issue.elementId, issue.kind, issue.location);
            row.text      = issue.message;
            row.detail    = issue.detail;
            row.elementId = issue.elementId;
            row.kind      = issue.kind;
            row.rule      = issue.rule;
            rows.append(row);
        }

        // Worst first within a document; equal severities keep their discovery order. The ladder
        // ranks Error highest, hence the descending compare.
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
