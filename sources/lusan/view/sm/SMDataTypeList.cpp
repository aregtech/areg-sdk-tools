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
 *  \file        lusan/view/sm/SMDataTypeList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page — data type list panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDataTypeList.hpp"

#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

SMDataTypeList::SMDataTypeList(QWidget* parent /*= nullptr*/)
    : QWidget               (parent)
    , mTable                (nullptr)
    , mButtonAdd            (nullptr)
    , mButtonInsert         (nullptr)
    , mButtonRemove         (nullptr)
    , mButtonAddField       (nullptr)
    , mButtonInsertField    (nullptr)
    , mButtonRemoveField    (nullptr)
    , mButtonMoveUp         (nullptr)
    , mButtonMoveDown       (nullptr)
{
    buildUi();
}

QToolButton* SMDataTypeList::createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut)
{
    QToolButton* button = new QToolButton(parent);
    button->setMaximumSize(24, 24);
    button->setCursor(Qt::PointingHandCursor);
    button->setMouseTracking(true);
    button->setToolTip(toolTip);
    button->setIcon(QIcon(iconName));
    button->setIconSize(QSize(25, 25));
    button->setShortcut(shortcut);
    return button;
}

void SMDataTypeList::buildUi(void)
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Data Types List:"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new data type entry")   , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected data type entry")      , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new data type entry"), QKeySequence(Qt::CTRL | Qt::Key_T));

    QFrame* sep1 = new QFrame(toolbar);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setMaximumSize(24, 24);

    mButtonAddField    = createToolButton(toolbar, QStringLiteral(":/icons/field add")   , tr("Create and add new field entry")   , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    mButtonRemoveField = createToolButton(toolbar, QStringLiteral(":/icons/field delete"), tr("Delete selected field entry")      , QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    mButtonInsertField = createToolButton(toolbar, QStringLiteral(":/icons/field insert"), tr("Create and insert new field entry"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));

    QFrame* sep2 = new QFrame(toolbar);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setMaximumSize(24, 24);

    mButtonMoveUp   = createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sep1);
    toolbarLayout->addWidget(mButtonAddField);
    toolbarLayout->addWidget(mButtonRemoveField);
    toolbarLayout->addWidget(mButtonInsertField);
    toolbarLayout->addWidget(sep2);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addStretch(1);

    mTable = new QTreeWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    mTable->setDropIndicatorShown(false);
    mTable->setIconSize(QSize(16, 16));
    mTable->setSortingEnabled(false);
    mTable->setAnimated(true);
    mTable->setAllColumnsShowFocus(false);
    mTable->setColumnCount(3);
    mTable->header()->setCascadingSectionResizes(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->setHeaderLabels(QStringList{ tr("Name:"), tr("Data Type:"), tr("Default Value:") });

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

QTreeWidget* SMDataTypeList::ctrlTableList(void) const
{
    return mTable;
}

QToolButton* SMDataTypeList::ctrlButtonAdd(void) const
{
    return mButtonAdd;
}

QToolButton* SMDataTypeList::ctrlButtonInsert(void) const
{
    return mButtonInsert;
}

QToolButton* SMDataTypeList::ctrlButtonRemove(void) const
{
    return mButtonRemove;
}

QToolButton* SMDataTypeList::ctrlButtonMoveUp(void) const
{
    return mButtonMoveUp;
}

QToolButton* SMDataTypeList::ctrlButtonMoveDown(void) const
{
    return mButtonMoveDown;
}

QToolButton* SMDataTypeList::ctrlButtonAddField(void) const
{
    return mButtonAddField;
}

QToolButton* SMDataTypeList::ctrlButtonInsertField(void) const
{
    return mButtonInsertField;
}

QToolButton* SMDataTypeList::ctrlButtonRemoveField(void) const
{
    return mButtonRemoveField;
}
