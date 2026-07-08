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
 *  \file        lusan/view/si/SIConstantList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface constant list panel.
 *
 ************************************************************************/
#include "lusan/view/si/SIConstantList.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

SIConstantList::SIConstantList(QWidget* parent)
    : QWidget           (parent)
    , mTable            (nullptr)
    , mButtonAdd        (nullptr)
    , mButtonRemove     (nullptr)
    , mButtonInsert     (nullptr)
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
{
    buildUi();
}

QToolButton* SIConstantList::createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut)
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

void SIConstantList::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Constants List:"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);

    QWidget* toolbar = new QWidget(group);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new constant entry")                    , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected constant entry")                       , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new constant entry at selected position"), QKeySequence(Qt::CTRL | Qt::Key_T));

    QFrame* sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setMaximumSize(24, 24);

    mButtonMoveUp   = createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up.")  , QKeySequence(Qt::CTRL | Qt::Key_Up));
    mButtonMoveDown = createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence(Qt::CTRL | Qt::Key_Down));

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(mButtonInsert);
    toolbarLayout->addWidget(sep);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addStretch(1);

    mTable = new QTableWidget(group);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setMouseTracking(true);
    mTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    mTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setColumnCount(3);
    mTable->verticalHeader()->setVisible(false);
    mTable->setHorizontalHeaderLabels(QStringList{ tr("Name:"), tr("Data Type:"), tr("Value:") });

    QHeaderView* header = mTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);

    groupLayout->addWidget(toolbar);
    groupLayout->addWidget(mTable);
    root->addWidget(group);
}

QToolButton* SIConstantList::ctrlButtonAdd()
{
    return mButtonAdd;
}

QToolButton* SIConstantList::ctrlButtonRemove()
{
    return mButtonRemove;
}

QToolButton* SIConstantList::ctrlButtonInsert()
{
    return mButtonInsert;
}

QToolButton* SIConstantList::ctrlButtonMoveUp()
{
    return mButtonMoveUp;
}

QToolButton* SIConstantList::ctrlButtonMoveDown()
{
    return mButtonMoveDown;
}

QTableWidget* SIConstantList::ctrlTableList()
{
    return mTable;
}
