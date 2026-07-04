/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/NavigationDock.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/
#include "lusan/view/common/NavigationDock.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/MdiChild.hpp"
#include <algorithm>


QString  NavigationDock::TabNameFileSystem      {tr("Workspace")};
QString  NavigationDock::TabLiveLogsExplorer    {tr("Live Logs")};
QString  NavigationDock::TabOfflineLogsExplorer {tr("Offline Logs")};

const QString& NavigationDock::getTabName(NavigationDock::eNaviWindow navi)
{
    static QString _empty("");
    switch (navi)
    {
        case NavigationDock::eNaviWindow::NaviWorkspace:
            return NavigationDock::TabNameFileSystem;
        case NavigationDock::eNaviWindow::NaviLiveLogs:
            return NavigationDock::TabLiveLogsExplorer;
        case NavigationDock::eNaviWindow::NaviOfflineLogs:
            return NavigationDock::TabOfflineLogsExplorer;
        default:
            return _empty;
    }
}

NavigationDock::eNaviWindow NavigationDock::getNaviWindow(const QString& tabName)
{
    if (tabName == NavigationDock::TabLiveLogsExplorer)
        return NavigationDock::eNaviWindow::NaviLiveLogs;
    else if (tabName == NavigationDock::TabOfflineLogsExplorer)
        return NavigationDock::eNaviWindow::NaviOfflineLogs;
    else if (tabName == NavigationDock::TabNameFileSystem)
        return NavigationDock::eNaviWindow::NaviWorkspace;
    else
        return NavigationDock::eNaviWindow::NaviUnknown;
}


NavigationDock::NavigationDock(MdiMainWindow* parent)
    : QDockWidget   (tr("Navigation"), parent)

    , mMainWindow   (parent)
    , mTabs         (this)
    , mLiveScopes   (parent, this)
    , mOfflineScopes(parent, this)
    , mFileSystem   (parent, this)
{    
    mTabs.addTab(&mFileSystem   , NELusanCommon::iconViewWorkspace(NELusanCommon::SizeBig)  , NavigationDock::TabNameFileSystem);
    mTabs.addTab(&mLiveScopes   , NELusanCommon::iconViewLiveLogs(NELusanCommon::SizeBig)   , NavigationDock::TabLiveLogsExplorer);
    mTabs.addTab(&mOfflineScopes, NELusanCommon::iconViewOfflineLogs(NELusanCommon::SizeBig), NavigationDock::TabOfflineLogsExplorer);
    mTabs.setTabPosition(QTabWidget::South);
    mTabs.setUsesScrollButtons(true);
    mTabs.setElideMode(Qt::ElideRight);
    mTabs.setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Expanding);
    setWidget(&mTabs);

    initSize();

    connect(mMainWindow, &MdiMainWindow::signalOptionsOpening   , this, &NavigationDock::onOptionsOpening);
    connect(mMainWindow, &MdiMainWindow::signalOptionsApplied   , this, &NavigationDock::onOptionsApplied);
    connect(mMainWindow, &MdiMainWindow::signalOptionsClosed    , this, &NavigationDock::onOptionsClosed);
}

NavigationWindow* NavigationDock::getTab(const QString& tabName) const
{
    QWidget * result {nullptr};
    int count { mTabs.count()};
    for (int i = 0; i < count; ++i)
    {
        if (mTabs.tabText(i) == tabName)
        {
            result = mTabs.widget(i);
            break;
        }
    }
    
    return static_cast<NavigationWindow *>(result);
}

NavigationWindow* NavigationDock::getTab(NavigationDock::eNaviWindow navi) const
{
    return getTab(NavigationDock::getTabName(navi));
}


bool NavigationDock::tabExists(const QString& tabName) const
{
    int count { mTabs.count()};
    for (int i = 0; i < count; ++i)
    {
        if (mTabs.tabText(i) == tabName)
        {
            return true;
        }
    }
    
    return false;
}

bool NavigationDock::tabExists(NavigationDock::eNaviWindow navi) const
{
    return tabExists(NavigationDock::getTabName(navi));
}

bool NavigationDock::showTab(const QString& tabName)
{
    int count{ mTabs.count() };
    for (int i = 0; i < count; ++i)
    {
        if (mTabs.tabText(i) == tabName)
        {
            if (mTabs.isTabVisible(i) == false)
                mTabs.setTabVisible(i, true);
            if (mTabs.currentIndex() != i)
                mTabs.setCurrentIndex(i);
            
            return true;
        }
    }

    return false;
}

bool NavigationDock::showTab(NavigationDock::eNaviWindow navi)
{
    return showTab(NavigationDock::getTabName(navi));    
}

void NavigationDock::initSize()
{
    const int minWidth = static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH);
    const int minHeight = static_cast<int>(NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumWidth(minWidth);
    setMinimumHeight(minHeight);
    resize(QSize { minWidth, std::max(minHeight, height()) });
    setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
}

QSize NavigationDock::sizeHint(void) const
{
    QSize result{ QDockWidget::sizeHint() };
    result.setWidth(static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH));
    return result;
}

QSize NavigationDock::minimumSizeHint(void) const
{
    QSize result{ QDockWidget::minimumSizeHint() };
    result.setWidth(static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH));
    result.setHeight(std::max(result.height(), static_cast<int>(NELusanCommon::MIN_NAVI_HEIGHT)));
    return result;
}

void NavigationDock::onOptionsOpening(void)
{
    static_cast<NavigationWindow &>(mFileSystem).optionOpenning();
    static_cast<NavigationWindow &>(mLiveScopes).optionOpenning();
    static_cast<NavigationWindow &>(mOfflineScopes).optionOpenning();
}

void NavigationDock::onOptionsApplied(void)
{
    static_cast<NavigationWindow &>(mFileSystem).optionApplied();
    static_cast<NavigationWindow &>(mLiveScopes).optionApplied();
    static_cast<NavigationWindow &>(mOfflineScopes).optionApplied();
}

void NavigationDock::onOptionsClosed(bool pressedOK)
{
    static_cast<NavigationWindow &>(mFileSystem).optionClosed(pressedOK);
    static_cast<NavigationWindow &>(mLiveScopes).optionClosed(pressedOK);
    static_cast<NavigationWindow &>(mOfflineScopes).optionClosed(pressedOK);
}
