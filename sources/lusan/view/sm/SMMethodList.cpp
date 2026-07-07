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
    , mButtonMoveUp     (nullptr)
    , mButtonMoveDown   (nullptr)
    , mActNewMethod     (nullptr)
    , mActNewParam      (nullptr)
{
    buildUi();
}

QToolButton* SMMethodList::createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut)
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

    mButtonAdd    = createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Create and add new method entry")  , QKeySequence(Qt::CTRL | Qt::Key_A));
    mButtonRemove = createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Delete selected entry")            , QKeySequence(Qt::CTRL | Qt::Key_D));
    mButtonInsert = createToolButton(toolbar, QStringLiteral(":/icons/entry insert"), tr("Create and insert new entry above"), QKeySequence(Qt::CTRL | Qt::Key_T));

    mActNewMethod = new QAction(QIcon(QStringLiteral(":/icons/entry add")), tr("New Method"), this);
    mActNewParam = new QAction(QIcon(QStringLiteral(":/icons/field add")), tr("New Parameter"), this);
    QMenu* addMenu = new QMenu(mButtonAdd);
    addMenu->addAction(mActNewMethod);
    addMenu->addSeparator();
    addMenu->addAction(mActNewParam);
    mButtonAdd->setMenu(addMenu);
    mButtonAdd->setPopupMode(QToolButton::MenuButtonPopup);
    // The arrow zone needs extra width on top of the icon-sized button.
    mButtonAdd->setMaximumSize(38, 24);

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

void SMMethodList::setAddTarget(eAddTarget target)
{
    switch (target)
    {
    case eAddTarget::AddParam:
        mButtonAdd->setIcon(QIcon(QStringLiteral(":/icons/field add")));
        mButtonAdd->setToolTip(tr("Create and add new parameter entry"));
        break;
    case eAddTarget::AddMethod:
    default:
        mButtonAdd->setIcon(QIcon(QStringLiteral(":/icons/entry add")));
        mButtonAdd->setToolTip(tr("Create and add new method entry"));
        break;
    }
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

QAction* SMMethodList::actionNewMethod() const
{
    return mActNewMethod;
}

QAction* SMMethodList::actionNewParam() const
{
    return mActNewParam;
}
