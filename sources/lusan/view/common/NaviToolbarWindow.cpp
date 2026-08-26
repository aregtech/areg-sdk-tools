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
 *  \file        lusan/view/common/NaviToolbarWindow.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The base of navigation windows built of a tool button row and a tree view.
 *
 ************************************************************************/

#include "lusan/view/common/NaviToolbarWindow.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QSizePolicy>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

NaviToolbarWindow::NaviToolbarWindow(int naviWindow, MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow  (naviWindow, wndMain, parent)

    , mNaviLayout       (new QVBoxLayout(this))
    , mToolbar          (new QWidget(this))
    , mToolLayout       (new QHBoxLayout(mToolbar))
    , mNaviTree         (new QTreeView(this))
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    mNaviLayout->setContentsMargins(0, 0, 0, 0);
    mNaviLayout->setSpacing(7);
    mNaviLayout->addWidget(mToolbar, 0, Qt::AlignmentFlag::AlignLeft);
    mNaviLayout->addWidget(mNaviTree);

    mToolbar->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
}

QToolButton* NaviToolbarWindow::addToolButton(const QIcon& icon, const QString& toolTip, const QString& statusTip, bool checkable /*= false*/)
{
    QToolButton* button = new QToolButton(mToolbar);
    button->setText(QStringLiteral("..."));
    button->setIcon(icon);
    button->setToolTip(toolTip);
    button->setStatusTip(statusTip);
    // The buttons show no text, so without this a screen reader announces the placeholder text.
    button->setAccessibleName(toolTip);
    button->setCheckable(checkable);
    button->setAutoRaise(true);
    // One extent for every navigation panel toolbar, so their marks carry the same stroke
    // weight as the rail beside them instead of whatever the active style would pick.
    button->setIconSize(QSize(NAVI_TOOL_ICON, NAVI_TOOL_ICON));
    mToolLayout->addWidget(button);
    return button;
}

void NaviToolbarWindow::addToolSeparator(void)
{
    QFrame* line = new QFrame(mToolbar);
    line->setFrameShape(QFrame::Shape::VLine);
    line->setFrameShadow(QFrame::Shadow::Sunken);
    mToolLayout->addWidget(line);
}

void NaviToolbarWindow::setNaviHeader(QWidget* header)
{
    if (header != nullptr)
    {
        header->setParent(this);
        mNaviLayout->insertWidget(0, header);
    }
}

void NaviToolbarWindow::setupTreeView(const QSize& iconSize)
{
    mNaviTree->setAutoFillBackground(false);
    mNaviTree->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    mNaviTree->setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);
    mNaviTree->setDropIndicatorShown(false);
    mNaviTree->setAlternatingRowColors(false);
    mNaviTree->setIconSize(iconSize);
    mNaviTree->setHeaderHidden(true);
}

void NaviToolbarWindow::capToolButtonIconSizes(int iconExtent /*= 12*/)
{
    const QSize extent(iconExtent, iconExtent);
    const QList<QToolButton*> buttons = findChildren<QToolButton*>();
    for (QToolButton* button : buttons)
    {
        button->setIconSize(extent);
    }
}
