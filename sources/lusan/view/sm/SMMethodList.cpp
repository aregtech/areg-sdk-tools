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
 *  \file        lusan/view/sm/SMMethodList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page — methods and parameters list panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMMethodList.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QAction>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

SMMethodList::SMMethodList(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mTable            (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonAddParam   (nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mActNewTrigger    (nullptr)
    , mActNewAction     (nullptr)
    , mActNewCondition  (nullptr)
{
    buildUi();
}

void SMMethodList::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Methods:"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new method (a trigger by default)"), QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected entry")            , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new entry above"), QKeySequence(Qt::CTRL | Qt::Key_T));

    // The Add split button: a plain click creates the default (a trigger method), the
    // drop-down offers the three method kinds explicitly. Adding a parameter is a separate
    // toolbar button, never an Add drop-down entry.
    mActNewTrigger   = new QAction(tr("Trigger")  , this);
    mActNewAction    = new QAction(tr("Action")   , this);
    mActNewCondition = new QAction(tr("Condition"), this);
    QMenu* addMenu = new QMenu(mButtonAdd);
    addMenu->addAction(mActNewTrigger);
    addMenu->addAction(mActNewAction);
    addMenu->addAction(mActNewCondition);
    NELusanCommon::decorateToolButton(mButtonAdd, addMenu);

    QFrame* sepParam = new QFrame(toolbar);
    sepParam->setFrameShape(QFrame::VLine);
    sepParam->setMaximumSize(24, 24);

    mButtonAddParam = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/field add"), tr("Create and add new parameter to the selected method"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));

    QFrame* sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setMaximumSize(24, 24);

    mButtonMoveUp   = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sepParam);
    toolbarLayout->addWidget(mButtonAddParam);
    toolbarLayout->addWidget(sep);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addStretch(1);

    mTable = new QTreeWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setAnimated(true);
    mTable->setAllColumnsShowFocus(false);
    mTable->setColumnCount(3);
    mTable->header()->setCascadingSectionResizes(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(QStringList{ tr("Name:"), tr("Type:"), tr("Value:") });

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

QTreeWidget* SMMethodList::ctrlTableList() const
{
    return mTable;
}

QToolButton* SMMethodList::ctrlButtonAdd() const
{
    return mButtonAdd;
}

QToolButton* SMMethodList::ctrlButtonInsert() const
{
    return mButtonInsert;
}

QToolButton* SMMethodList::ctrlButtonRemove() const
{
    return mButtonRemove;
}

QToolButton* SMMethodList::ctrlButtonMoveUp() const
{
    return mButtonMoveUp;
}

QToolButton* SMMethodList::ctrlButtonMoveDown() const
{
    return mButtonMoveDown;
}

QToolButton* SMMethodList::ctrlButtonAddParam() const
{
    return mButtonAddParam;
}

QAction* SMMethodList::actionNewTrigger() const
{
    return mActNewTrigger;
}

QAction* SMMethodList::actionNewAction() const
{
    return mActNewAction;
}

QAction* SMMethodList::actionNewCondition() const
{
    return mActNewCondition;
}
