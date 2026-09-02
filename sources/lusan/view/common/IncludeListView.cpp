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

namespace
{
    ElementListConfig listConfigOf(void)
    {
        return ElementListConfig{ QObject::tr("Include Files:")
                                , QStringList{ QObject::tr("Location:"), QObject::tr("Type:"), QObject::tr("Name:"), QObject::tr("Version:") }
                                , QObject::tr("include")
                                , QString()
                                , false };
    }
}

IncludeListView::IncludeListView(const IncludeTypeConfig& config, QWidget* parent /*= nullptr*/)
    : ElementListView (listConfigOf(), parent)
    , mConfig         (config)
    , mGroupSource    (nullptr)
    , mGroupDataType  (nullptr)
    , mGroupDocument  (nullptr)
    , mButtonUpdate   (nullptr)
{
    // Add has no kind menu: the extension of the location decides the group. A new entry starts
    // under Include Files and moves as soon as its location says otherwise.
    addToolbarSeparator();
    mButtonUpdate = NELusanCommon::createToolButton(ctrlToolbar(), QStringLiteral(":/icons/Update Item"), tr("Refresh the type, name and version from the include files."), QKeySequence(Qt::CTRL | Qt::Key_R));
    addToolbarButton(mButtonUpdate);

    QTreeWidget* table = ctrlTableList();
    table->header()->setStretchLastSection(false);

    mGroupSource   = new QTreeWidgetItem(table);
    // A host that includes no document of a given kind gets no heading for it: an empty group
    // that can never fill reads as a place to put something, and there is nothing to put there.
    mGroupDataType = (mConfig.hasDataTypes() ? new QTreeWidgetItem(table) : nullptr);
    mGroupDocument = (mConfig.hasDocuments() ? new QTreeWidgetItem(table) : nullptr);
    // Each heading is marked with its own kind, and only the headings this page actually built.
    // ctrlGroup() answers the source heading for a kind that has none, so driving this from the
    // kinds would let an absent kind overwrite the mark the source heading already carries.
    auto prepareGroup = [this](eIncludeKind kind, QTreeWidgetItem* item)
    {
        if (item == nullptr)
            return;

        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // Show the expand indicator even while empty, so a heading reads as a container.
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        item->setExpanded(true);
        item->setIcon(static_cast<int>(eColumn::ColLocation), iconForKind(kind));
    };

    prepareGroup(eIncludeKind::Source  , mGroupSource);
    prepareGroup(eIncludeKind::DataType, mGroupDataType);
    prepareGroup(eIncludeKind::Document, mGroupDocument);

    updateGroupCounts(0, 0, 0);
}

void IncludeListView::decorateGroup(QTreeWidgetItem* group)
{
    QFont font{ ctrlTableList()->font() };
    font.setBold(true);
    group->setFont(static_cast<int>(eColumn::ColLocation), font);
    group->setFirstColumnSpanned(true);

    QColor tint{ ctrlTableList()->palette().color(QPalette::Highlight) };
    tint.setAlpha(28);
    group->setBackground(static_cast<int>(eColumn::ColLocation), tint);
}

QList<QTreeWidgetItem*> IncludeListView::groups() const
{
    QList<QTreeWidgetItem*> result{ mGroupSource };
    if (mGroupDataType != nullptr)
    {
        result.append(mGroupDataType);
    }

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
    if (mGroupDataType != nullptr)
    {
        mGroupDataType->setText(static_cast<int>(eColumn::ColLocation), tr("Data Types (%1)").arg(dataTypeCount));
    }

    if (mGroupDocument != nullptr)
    {
        mGroupDocument->setText(static_cast<int>(eColumn::ColLocation), QStringLiteral("%1 (%2)").arg(mConfig.groupDocLabel).arg(documentCount));
    }
}

void IncludeListView::refreshGroupCounts()
{
    updateGroupCounts(mGroupSource->childCount()
                      , (mGroupDataType != nullptr ? mGroupDataType->childCount() : 0)
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

    ElementListView::changeEvent(event);
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
        return ctrlTableList()->topLevelItem(index.row());
    }

    QTreeWidgetItem* group = ctrlTableList()->topLevelItem(parent.row());
    return ((group != nullptr) && (index.row() < group->childCount()) ? group->child(index.row()) : nullptr);
}

QTreeWidgetItem* IncludeListView::ctrlGroup(eIncludeKind kind) const
{
    switch (kind)
    {
    case eIncludeKind::DataType:    return (mGroupDataType != nullptr ? mGroupDataType : mGroupSource);
    case eIncludeKind::Document:    return (mGroupDocument != nullptr ? mGroupDocument : mGroupSource);
    default:                        return mGroupSource;
    }
}

QIcon IncludeListView::iconForKind(eIncludeKind kind) const
{
    switch (kind)
    {
    case eIncludeKind::DataType:    return NELusanCommon::iconDataTypeDocument(NELusanCommon::SizeSmall);
    case eIncludeKind::Document:    return mConfig.docIcon;
    default:                        return NELusanCommon::iconInclude(NELusanCommon::SizeSmall);
    }
}

eIncludeKind IncludeListView::kindForLocation(const QString& location) const
{
    const eIncludeKind kind = includeKindOf(location, mConfig.docExtension);
    // Where the host takes no data type document, a `.dtml` typed into the location field is
    // just a file it does not know: one answer for the group, the icon and the Type column.
    return ((kind == eIncludeKind::DataType) && (mConfig.hasDataTypes() == false)) ? eIncludeKind::Source : kind;
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

QToolButton* IncludeListView::ctrlButtonUpdate() const
{
    return mButtonUpdate;
}

