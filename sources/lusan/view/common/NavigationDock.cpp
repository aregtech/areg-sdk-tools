/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/NavigationDock.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/
#include "lusan/view/common/NavigationDock.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/MdiChild.hpp"


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
    QSize szFS{mFileSystem.width()      ,  mFileSystem.height()};
    QSize szLL{mLiveScopes.width()      , mLiveScopes.height()};
    QSize szOL{mOfflineScopes.width()   , mOfflineScopes.height()};
    int maxWidth    = std::max(szFS.width() , std::max(szLL.width() , szOL.width() ));
    int maxHeight   = std::max(szFS.height(), std::max(szLL.height(), szOL.height()));
    
    resize(QSize { maxWidth - 100,  maxHeight + 100 });
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
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
