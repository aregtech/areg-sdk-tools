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
 *  \file        lusan/view/common/IncludeListView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "include list" panel implementation.
 *
 ************************************************************************/

#include "lusan/view/common/IncludeListView.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QEvent>
#include <QFileInfo>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QModelIndex>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
    constexpr int IncludeIdRole { Qt::UserRole };
}

IncludeListView::IncludeListView(const IncludeTypeConfig& config, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mConfig           (config)
    , mTable            (nullptr)
    , mGroupSource      (nullptr)
    , mGroupDataType    (nullptr)
    , mGroupDocument    (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mButtonUpdate     (nullptr)
{
    buildUi();
}

void IncludeListView::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Include Files:"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new include entry")   , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected include entry")      , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new include entry"), QKeySequence(Qt::CTRL | Qt::Key_T));

    // Add has no kind menu: the extension of the location decides the group. A new entry starts
    // under Include Files and moves as soon as its location says otherwise.
    QFrame* sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setMaximumSize(24, 24);

    mButtonMoveUp   = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    QFrame* sepUpdate = new QFrame(toolbar);
    sepUpdate->setFrameShape(QFrame::VLine);
    sepUpdate->setMaximumSize(24, 24);

    mButtonUpdate = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/Update Item"), tr("Refresh the type, name and version from the include files."), QKeySequence(Qt::CTRL | Qt::Key_R));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sep);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addWidget(sepUpdate);
    toolbarLayout->addWidget(mButtonUpdate);
    toolbarLayout->addStretch(1);

    mTable = new QTreeWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setAllColumnsShowFocus(false);
    mTable->setColumnCount(4);
    mTable->header()->setStretchLastSection(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(QStringList{ tr("Location:"), tr("Type:"), tr("Name:"), tr("Version:") });
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColLocation), QHeaderView::Stretch);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColType)    , QHeaderView::ResizeToContents);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColName)    , QHeaderView::ResizeToContents);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColVersion) , QHeaderView::ResizeToContents);

    mGroupSource   = new QTreeWidgetItem(mTable);
    mGroupDataType = new QTreeWidgetItem(mTable);
    // A host that includes no document of its own kind gets no heading for one: an empty group
    // that can never fill reads as a place to put something, and there is nothing to put there.
    mGroupDocument = (mConfig.hasDocuments() ? new QTreeWidgetItem(mTable) : nullptr);
    for (eIncludeKind kind : { eIncludeKind::Source, eIncludeKind::DataType, eIncludeKind::Document })
    {
        QTreeWidgetItem* item = ctrlGroup(kind);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // Show the expand indicator even while empty, so a heading reads as a container.
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        item->setExpanded(true);
        item->setIcon(static_cast<int>(eColumn::ColLocation), iconForKind(kind));
    }

    updateGroupCounts(0, 0, 0);

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

void IncludeListView::decorateGroup(QTreeWidgetItem* group)
{
    QFont font{ mTable->font() };
    font.setBold(true);
    group->setFont(static_cast<int>(eColumn::ColLocation), font);
    group->setFirstColumnSpanned(true);

    QColor tint{ mTable->palette().color(QPalette::Highlight) };
    tint.setAlpha(28);
    group->setBackground(static_cast<int>(eColumn::ColLocation), tint);
}

QList<QTreeWidgetItem*> IncludeListView::groups() const
{
    QList<QTreeWidgetItem*> result{ mGroupSource, mGroupDataType };
    if (mGroupDocument != nullptr)
    {
        result.append(mGroupDocument);
    }

    return result;
}

void IncludeListView::updateGroupCounts(int sourceCount, int dataTypeCount, int documentCount)
{
    // Re-applied on every refresh: removing the last child otherwise takes the expand indicator
    // with it, and an empty heading has to keep reading as a place to put something.
    for (QTreeWidgetItem* group : groups())
    {
        group->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        decorateGroup(group);
    }

    mGroupSource->setText(static_cast<int>(eColumn::ColLocation), tr("Include Files (%1)").arg(sourceCount));
    // The Data Types group is a placeholder until `.dtml` becomes a real document type: entries are
    // listed and classified, never resolved, version-checked or validated.
    mGroupDataType->setText(static_cast<int>(eColumn::ColLocation), tr("Data Types (%1)").arg(dataTypeCount));
    if (mGroupDocument != nullptr)
    {
        mGroupDocument->setText(static_cast<int>(eColumn::ColLocation), QStringLiteral("%1 (%2)").arg(mConfig.groupDocLabel).arg(documentCount));
    }
}

void IncludeListView::refreshGroupCounts()
{
    updateGroupCounts(mGroupSource->childCount()
                      , mGroupDataType->childCount()
                      , (mGroupDocument != nullptr ? mGroupDocument->childCount() : 0));
}

void IncludeListView::changeEvent(QEvent* event)
{
    if ((event->type() == QEvent::PaletteChange) && (mGroupSource != nullptr))
    {
        for (QTreeWidgetItem* group : groups())
        {
            decorateGroup(group);
        }
    }

    QWidget::changeEvent(event);
}

void IncludeListView::clearRows()
{
    for (QTreeWidgetItem* group : groups())
    {
        while (group->childCount() > 0)
        {
            delete group->takeChild(0);
        }
    }
}

QTreeWidgetItem* IncludeListView::placeRow(QTreeWidgetItem* existing, eIncludeKind kind, uint32_t id)
{
    QTreeWidgetItem* target = ctrlGroup(kind);
    QTreeWidgetItem* item   = existing;
    if (item == nullptr)
    {
        item = new QTreeWidgetItem();
        // The editable flag lets the delegate open the inline Location editor on double-click;
        // the page's editable-check keeps every other column read-only.
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    else if (item->parent() == target)
    {
        return item;
    }
    else if (item->parent() != nullptr)
    {
        item->parent()->removeChild(item);
    }

    item->setData(static_cast<int>(eColumn::ColLocation), IncludeIdRole, id);
    target->addChild(item);
    target->setExpanded(true);
    return item;
}

QTreeWidgetItem* IncludeListView::findRow(uint32_t id) const
{
    if (id == 0)
    {
        return nullptr;
    }

    for (QTreeWidgetItem* group : groups())
    {
        for (int i = 0; i < group->childCount(); ++i)
        {
            if (rowId(group->child(i)) == id)
            {
                return group->child(i);
            }
        }
    }

    return nullptr;
}

uint32_t IncludeListView::rowId(const QTreeWidgetItem* item)
{
    return (((item != nullptr) && (item->parent() != nullptr))
            ? item->data(static_cast<int>(eColumn::ColLocation), IncludeIdRole).toUInt()
            : 0u);
}

bool IncludeListView::isGroup(const QTreeWidgetItem* item) const
{
    return (item != nullptr) && ((item == mGroupSource) || (item == mGroupDataType) || (item == mGroupDocument));
}

QTreeWidgetItem* IncludeListView::itemAt(const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return nullptr;
    }

    const QModelIndex parent = index.parent();
    if (parent.isValid() == false)
    {
        return mTable->topLevelItem(index.row());
    }

    QTreeWidgetItem* group = mTable->topLevelItem(parent.row());
    return ((group != nullptr) && (index.row() < group->childCount()) ? group->child(index.row()) : nullptr);
}

QTreeWidgetItem* IncludeListView::ctrlGroup(eIncludeKind kind) const
{
    switch (kind)
    {
    case eIncludeKind::DataType:    return mGroupDataType;
    case eIncludeKind::Document:    return (mGroupDocument != nullptr ? mGroupDocument : mGroupSource);
    default:                        return mGroupSource;
    }
}

QIcon IncludeListView::iconForKind(eIncludeKind kind) const
{
    switch (kind)
    {
    case eIncludeKind::DataType:    return NELusanCommon::iconStructure(NELusanCommon::SizeSmall);
    case eIncludeKind::Document:    return mConfig.docIcon;
    default:                        return NELusanCommon::iconInclude(NELusanCommon::SizeSmall);
    }
}

eIncludeKind IncludeListView::kindForLocation(const QString& location) const
{
    return includeKindOf(location, mConfig.docExtension);
}

QString IncludeListView::typeForLocation(const QString& location) const
{
    switch (kindForLocation(location))
    {
    case eIncludeKind::Document:    return mConfig.docTypeLabel;
    case eIncludeKind::DataType:    return tr("Data Type");
    default:                        return tr("Source");
    }
}

QString IncludeListView::nameForLocation(const QString& location) const
{
    const QFileInfo info(location);
    // A document or data type include carries a declared name; until the file is parsed the
    // base name (no extension) is its best proxy. A source include is shown by its file name.
    return (kindForLocation(location) == eIncludeKind::Source ? info.fileName() : info.completeBaseName());
}

QTreeWidget* IncludeListView::ctrlTableList() const
{
    return mTable;
}

QToolButton* IncludeListView::ctrlButtonAdd() const
{
    return mButtonAdd;
}

QToolButton* IncludeListView::ctrlButtonInsert() const
{
    return mButtonInsert;
}

QToolButton* IncludeListView::ctrlButtonRemove() const
{
    return mButtonRemove;
}

QToolButton* IncludeListView::ctrlButtonMoveUp() const
{
    return mButtonMoveUp;
}

QToolButton* IncludeListView::ctrlButtonMoveDown() const
{
    return mButtonMoveDown;
}

QToolButton* IncludeListView::ctrlButtonUpdate() const
{
    return mButtonUpdate;
}

