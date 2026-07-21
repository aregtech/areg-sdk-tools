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
 *  \file        lusan/view/common/MethodListView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method list" panel implementation.
 *
 ************************************************************************/

#include "lusan/view/common/MethodListView.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

MethodListView::MethodListView(const MethodListConfig& config, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mConfig           (config)
    , mTable            (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonParamAdd   (nullptr)
    , mButtonParamRemove(nullptr)
    , mButtonParamInsert(nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mTypeActions      ( )
{
    buildUi();
}

void MethodListView::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(mConfig.groupTitle, this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new method entry")   , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected method entry")      , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new method entry"), QKeySequence(Qt::CTRL | Qt::Key_T));

    // The Add split button: a plain click creates the default (the first kind), the drop-down
    // offers the method kinds explicitly. Adding a parameter is a separate toolbar button.
    QMenu* addMenu = new QMenu(mButtonAdd);
    for (const QString& label : mConfig.typeMenuLabels)
    {
        QAction* action = new QAction(label, this);
        addMenu->addAction(action);
        mTypeActions.append(action);
    }
    NELusanCommon::decorateToolButton(mButtonAdd, addMenu);

    QFrame* sepParam = new QFrame(toolbar);
    sepParam->setFrameShape(QFrame::VLine);
    sepParam->setMaximumSize(24, 24);

    mButtonParamAdd = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field add"), tr("Create and add new parameter to the selected method"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    if (mConfig.hasParamInsertRemove)
    {
        mButtonParamRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field delete"), tr("Delete selected method parameter entry")      , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
        mButtonParamInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field insert"), tr("Create and insert new method parameter entry"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    }

    QFrame* sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setMaximumSize(24, 24);

    mButtonMoveUp   = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sepParam);
    toolbarLayout->addWidget(mButtonParamAdd);
    if (mConfig.hasParamInsertRemove)
    {
        toolbarLayout->addWidget(mButtonParamRemove);
        toolbarLayout->addWidget(mButtonParamInsert);
    }
    toolbarLayout->addWidget(sep);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addStretch(1);

    QStringList headers{ tr("Name:"), mConfig.typeColumnLabel, tr("Value:") };
    if (mConfig.hasResponseColumn)
    {
        headers.append(tr("Response:"));
    }

    mTable = new QTreeWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    mTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setAnimated(true);
    mTable->setAllColumnsShowFocus(false);
    mTable->setColumnCount(headers.size());
    mTable->header()->setCascadingSectionResizes(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(headers);

    QHeaderView* header = mTable->header();
    // Service Interface and State Machine share ONE column policy so the two Methods tables behave
    // identically: the Name column takes all the horizontal slack (auto-stretch) so method and
    // parameter names stay readable, while every other column fits its content so types, values and
    // (SI only) response links are shown in full and never clipped. The type editor drop-down is
    // widened separately in TableCell so a narrow Type column still lists every type in full.
    header->setSectionResizeMode(ColName , QHeaderView::Stretch);
    header->setSectionResizeMode(ColType , QHeaderView::ResizeToContents);
    header->setSectionResizeMode(ColValue, QHeaderView::ResizeToContents);
    if (mConfig.hasResponseColumn)
    {
        header->setSectionResizeMode(ColResponse, QHeaderView::ResizeToContents);
    }

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

QTreeWidget* MethodListView::ctrlTableList() const
{
    return mTable;
}

QToolButton* MethodListView::ctrlButtonAdd() const
{
    return mButtonAdd;
}

QToolButton* MethodListView::ctrlButtonInsert() const
{
    return mButtonInsert;
}

QToolButton* MethodListView::ctrlButtonRemove() const
{
    return mButtonRemove;
}

QToolButton* MethodListView::ctrlButtonParamAdd() const
{
    return mButtonParamAdd;
}

QToolButton* MethodListView::ctrlButtonParamRemove() const
{
    return mButtonParamRemove;
}

QToolButton* MethodListView::ctrlButtonParamInsert() const
{
    return mButtonParamInsert;
}

QToolButton* MethodListView::ctrlButtonMoveUp() const
{
    return mButtonMoveUp;
}

QToolButton* MethodListView::ctrlButtonMoveDown() const
{
    return mButtonMoveDown;
}

QAction* MethodListView::typeAction(int index) const
{
    return ((index >= 0) && (index < mTypeActions.size())) ? mTypeActions.at(index) : nullptr;
}
