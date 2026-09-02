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
 *  \file        lusan/view/common/ElementListView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \brief       Lusan application, the shared document editor list panel implementation.
 *
 ************************************************************************/

#include "lusan/view/common/ElementListView.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

ElementListView::ElementListView(const ElementListConfig& config, QWidget* parent /*= nullptr*/)
    : QWidget            (parent)
    , mListConfig        (config)
    , mGroup             (nullptr)
    , mToolbar           (nullptr)
    , mToolbarLayout     (nullptr)
    , mTable             (nullptr)
    , mButtonAdd         (nullptr)
    , mButtonRemove      (nullptr)
    , mButtonInsert      (nullptr)
    , mButtonAddChild    (nullptr)
    , mButtonRemoveChild (nullptr)
    , mButtonInsertChild (nullptr)
    , mButtonMoveUp      (nullptr)
    , mButtonMoveDown    (nullptr)
{
    buildUi();
}

void ElementListView::buildUi(void)
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    mGroup = new QGroupBox(mListConfig.groupTitle, this);
    QVBoxLayout* groupLayout = new QVBoxLayout(mGroup);

    mToolbar = new QWidget(mGroup);
    mToolbarLayout = new QHBoxLayout(mToolbar);
    mToolbarLayout->setSpacing(5);
    mToolbarLayout->setContentsMargins(2, 2, 2, 2);

    const QString& entry = mListConfig.entryName;
    mButtonAdd    = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new %1 entry").arg(entry)      , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected %1 entry").arg(entry)         , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/entry insert"), tr("Insert new %1 entry above the selected one").arg(entry), QKeySequence(Qt::CTRL | Qt::Key_T));

    mToolbarLayout->addWidget(mButtonAdd);
    mToolbarLayout->addWidget(mButtonRemove);
    mToolbarLayout->addWidget(mButtonInsert);
    mToolbarLayout->addStretch(1);

    if (mListConfig.childName.isEmpty() == false)
    {
        const QString& child = mListConfig.childName;
        addToolbarSeparator();
        mButtonAddChild    = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/field add")   , tr("Create and add new %1 to the selected entry").arg(child), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
        mButtonRemoveChild = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/field delete"), tr("Delete selected %1").arg(child)                         , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
        mButtonInsertChild = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/field insert"), tr("Insert new %1 above the selected one").arg(child)       , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
        addToolbarButton(mButtonAddChild);
        addToolbarButton(mButtonRemoveChild);
        addToolbarButton(mButtonInsertChild);
    }

    addToolbarSeparator();
    mButtonMoveUp   = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = NELusanCommon::createToolButton(mToolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));
    addToolbarButton(mButtonMoveUp);
    addToolbarButton(mButtonMoveDown);

    mTable = new QTreeWidget(mGroup);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    mTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setAllColumnsShowFocus(false);
    mTable->setRootIsDecorated(mListConfig.flatList == false);
    mTable->setAnimated(mListConfig.flatList == false);
    mTable->setColumnCount(mListConfig.headers.size());
    mTable->header()->setCascadingSectionResizes(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(mListConfig.headers);

    // The column policy every list page shares: the first column takes the horizontal slack so
    // names stay readable, and every other column fits its content.
    QHeaderView* header = mTable->header();
    for (int i = 0; i < mListConfig.headers.size(); ++i)
    {
        header->setSectionResizeMode(i, (i == 0) ? QHeaderView::Stretch : QHeaderView::ResizeToContents);
    }

    mTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mTable, &QTreeWidget::customContextMenuRequested, this, &ElementListView::onContextMenuRequested);

    groupLayout->addWidget(mToolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(mGroup);
}

void ElementListView::addToolbarSeparator(void)
{
    QFrame* separator = new QFrame(mToolbar);
    separator->setFrameShape(QFrame::VLine);
    separator->setMaximumSize(24, 24);
    mToolbarLayout->insertWidget(mToolbarLayout->count() - 1, separator);
}

void ElementListView::addToolbarButton(QToolButton* button)
{
    if (button != nullptr)
    {
        mToolbarLayout->insertWidget(mToolbarLayout->count() - 1, button);
    }
}

QTreeWidget* ElementListView::ctrlTableList(void) const
{
    return mTable;
}

QToolButton* ElementListView::ctrlButtonAdd(void) const
{
    return mButtonAdd;
}

QToolButton* ElementListView::ctrlButtonRemove(void) const
{
    return mButtonRemove;
}

QToolButton* ElementListView::ctrlButtonInsert(void) const
{
    return mButtonInsert;
}

QToolButton* ElementListView::ctrlButtonAddChild(void) const
{
    return mButtonAddChild;
}

QToolButton* ElementListView::ctrlButtonRemoveChild(void) const
{
    return mButtonRemoveChild;
}

QToolButton* ElementListView::ctrlButtonInsertChild(void) const
{
    return mButtonInsertChild;
}

QToolButton* ElementListView::ctrlButtonMoveUp(void) const
{
    return mButtonMoveUp;
}

QToolButton* ElementListView::ctrlButtonMoveDown(void) const
{
    return mButtonMoveDown;
}

//////////////////////////////////////////////////////////////////////////
// The row context menu
//////////////////////////////////////////////////////////////////////////

void ElementListView::addButtonEntry(QMenu& menu, QToolButton* button, const QString& text)
{
    if (button == nullptr)
        return;

    QAction* action = menu.addAction(button->icon(), text);
    action->setShortcut(button->shortcut());
    action->setEnabled(button->isEnabled());
    action->setToolTip(button->toolTip());
    QObject::connect(action, &QAction::triggered, button, [button]() { button->click(); });
}

void ElementListView::buildContextMenu(QMenu& menu)
{
    menu.setToolTipsVisible(true);
    fillContextMenu(menu);
}

void ElementListView::fillContextMenu(QMenu& menu)
{
    const QString& entry = mListConfig.entryName;
    const QString& child = mListConfig.childName;

    // The kinds the Add split button offers are the same choice here, as a sub-menu.
    if ((mButtonAdd != nullptr) && (mButtonAdd->menu() != nullptr))
    {
        QMenu* kinds = menu.addMenu(mButtonAdd->icon(), tr("Add %1").arg(entry));
        kinds->setEnabled(mButtonAdd->isEnabled());
        kinds->addActions(mButtonAdd->menu()->actions());
    }
    else
    {
        addButtonEntry(menu, mButtonAdd, tr("Add %1").arg(entry));
    }

    addButtonEntry(menu, mButtonInsert, tr("Insert %1 Above").arg(entry));
    addButtonEntry(menu, mButtonRemove, tr("Delete %1").arg(entry));

    if (hasChildRows())
    {
        menu.addSeparator();
        addButtonEntry(menu, mButtonAddChild   , tr("Add %1").arg(child));
        addButtonEntry(menu, mButtonInsertChild, tr("Insert %1 Above").arg(child));
        addButtonEntry(menu, mButtonRemoveChild, tr("Delete %1").arg(child));
    }

    menu.addSeparator();
    addButtonEntry(menu, mButtonMoveUp  , tr("Move Up"));
    addButtonEntry(menu, mButtonMoveDown, tr("Move Down"));

    menu.addSeparator();
    QAction* rename = menu.addAction(NELusanCommon::iconRename(NELusanCommon::SizeSmall), tr("Rename"));
    rename->setShortcut(QKeySequence(Qt::Key_F2));
    rename->setEnabled(mTable->currentItem() != nullptr);
    connect(rename, &QAction::triggered, this, &ElementListView::signalRenameRequested);

    QAction* copy = menu.addAction(NELusanCommon::iconCopy(NELusanCommon::SizeSmall), tr("Copy Row"));
    copy->setShortcut(QKeySequence::Copy);
    copy->setEnabled(mTable->currentItem() != nullptr);
    connect(copy, &QAction::triggered, this, &ElementListView::copyCurrentRow);

    if (mListConfig.flatList == false)
    {
        menu.addSeparator();
        QAction* expand = menu.addAction(NELusanCommon::iconListExpand(NELusanCommon::SizeSmall), tr("Expand All"));
        connect(expand, &QAction::triggered, mTable, &QTreeWidget::expandAll);

        QAction* collapse = menu.addAction(NELusanCommon::iconListCollapse(NELusanCommon::SizeSmall), tr("Collapse All"));
        connect(collapse, &QAction::triggered, mTable, &QTreeWidget::collapseAll);
    }
}

void ElementListView::onContextMenuRequested(const QPoint& pos)
{
    // The row under the cursor becomes the current one first, so every entry acts on what was
    // right-clicked rather than on what was selected before.
    if (QTreeWidgetItem* under = mTable->itemAt(pos))
    {
        if (under != mTable->currentItem())
        {
            mTable->setCurrentItem(under);
        }
    }

    QMenu menu(mTable);
    buildContextMenu(menu);
    if (menu.isEmpty() == false)
    {
        menu.exec(mTable->viewport()->mapToGlobal(pos));
    }
}

void ElementListView::copyCurrentRow(void) const
{
    QTreeWidgetItem* current = mTable->currentItem();
    if (current == nullptr)
        return;

    QStringList cells;
    cells.reserve(mTable->columnCount());
    for (int i = 0; i < mTable->columnCount(); ++i)
    {
        cells.append(current->text(i));
    }

    // Trailing empty cells only add tabs to what is pasted.
    while ((cells.isEmpty() == false) && cells.last().isEmpty())
    {
        cells.removeLast();
    }

    QApplication::clipboard()->setText(cells.join(QLatin1Char('\t')));
}
