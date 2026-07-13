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
 *  \file        lusan/view/si/SIMethodList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface method list panel.
 *
 ************************************************************************/
#include "lusan/view/si/SIMethodList.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAction>
#include <QAbstractItemView>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

SIMethodList::SIMethodList(QWidget* parent)
    : QWidget           (parent)
    , mTable            (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonParamAdd   (nullptr)
    , mButtonParamRemove(nullptr)
    , mButtonParamInsert(nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mActNewRequest    (nullptr)
    , mActNewResponse   (nullptr)
    , mActNewBroadcast  (nullptr)
{
    buildUi();
}

void SIMethodList::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Service Methods List:"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new method entry (a request by default)"), QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected method entry")      , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new method entry"), QKeySequence(Qt::CTRL | Qt::Key_T));

    mActNewRequest   = new QAction(tr("Request"), this);
    mActNewResponse  = new QAction(tr("Response"), this);
    mActNewBroadcast = new QAction(tr("Broadcast"), this);
    QMenu* addMenu = new QMenu(mButtonAdd);
    addMenu->addAction(mActNewRequest);
    addMenu->addAction(mActNewResponse);
    addMenu->addAction(mActNewBroadcast);
    NELusanCommon::decorateToolButton(mButtonAdd, addMenu);

    QFrame* sep1 = new QFrame(toolbar);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setMaximumSize(24, 24);

    mButtonParamAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field add")   , tr("Create and add new method parameter entry")   , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    mButtonParamRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field delete"), tr("Delete selected method parameter entry")      , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    mButtonParamInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field insert"), tr("Create and insert new method parameter entry"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));

    QFrame* sep2 = new QFrame(toolbar);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setMaximumSize(24, 24);

    mButtonMoveUp   = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sep1);
    toolbarLayout->addWidget(mButtonParamAdd);
    toolbarLayout->addWidget(mButtonParamRemove);
    toolbarLayout->addWidget(mButtonParamInsert);
    toolbarLayout->addWidget(sep2);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addStretch(1);

    mTable = new QTreeWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    mTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setIconSize(QSize(16, 16));
    mTable->setColumnCount(4);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(QStringList{ tr("Name:"), tr("Data Type:"), tr("Value:"), tr("Response:") });

    QHeaderView* header = mTable->header();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::Stretch);

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

QToolButton* SIMethodList::ctrlButtonAdd() const
{
    return mButtonAdd;
}

QToolButton* SIMethodList::ctrlButtonInsert() const
{
    return mButtonInsert;
}

QToolButton* SIMethodList::ctrlButtonRemove() const
{
    return mButtonRemove;
}

QToolButton* SIMethodList::ctrlButtonParamAdd() const
{
    return mButtonParamAdd;
}

QToolButton* SIMethodList::ctrlButtonParamRemove() const
{
    return mButtonParamRemove;
}

QToolButton* SIMethodList::ctrlButtonParamInsert() const
{
    return mButtonParamInsert;
}

QToolButton* SIMethodList::ctrlButtonMoveUp() const
{
    return mButtonMoveUp;
}

QToolButton* SIMethodList::ctrlButtonMoveDown() const
{
    return mButtonMoveDown;
}

QTreeWidget* SIMethodList::ctrlTableList() const
{
    return mTable;
}

QAction* SIMethodList::actionNewRequest() const
{
    return mActNewRequest;
}

QAction* SIMethodList::actionNewResponse() const
{
    return mActNewResponse;
}

QAction* SIMethodList::actionNewBroadcast() const
{
    return mActNewBroadcast;
}
