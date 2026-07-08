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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMIncludeList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page — include list panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMIncludeList.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

SMIncludeList::SMIncludeList(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mTable            (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mButtonUpdate     (nullptr)
{
    buildUi();
}

void SMIncludeList::buildUi()
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
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setRootIsDecorated(false);
    mTable->setAllColumnsShowFocus(false);
    mTable->setColumnCount(4);
    mTable->header()->setStretchLastSection(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(QStringList{ tr("Location:"), tr("Type:"), tr("Name:"), tr("Version:") });
    mTable->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mTable->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mTable->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    mTable->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

QTreeWidget* SMIncludeList::ctrlTableList() const
{
    return mTable;
}

QToolButton* SMIncludeList::ctrlButtonAdd() const
{
    return mButtonAdd;
}

QToolButton* SMIncludeList::ctrlButtonInsert() const
{
    return mButtonInsert;
}

QToolButton* SMIncludeList::ctrlButtonRemove() const
{
    return mButtonRemove;
}

QToolButton* SMIncludeList::ctrlButtonMoveUp() const
{
    return mButtonMoveUp;
}

QToolButton* SMIncludeList::ctrlButtonMoveDown() const
{
    return mButtonMoveDown;
}

QToolButton* SMIncludeList::ctrlButtonUpdate() const
{
    return mButtonUpdate;
}
