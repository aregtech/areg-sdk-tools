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
#include "lusan/view/common/NaviFsmToolbar.hpp"

#include <QVBoxLayout>
#include <algorithm>


QString  NavigationDock::TabNameFileSystem      {tr("Workspace")};
QString  NavigationDock::TabLiveLogsExplorer    {tr("Live Logs")};
QString  NavigationDock::TabOfflineLogsExplorer {tr("Offline Logs")};
QString  NavigationDock::TabFsmToolbar          {tr("Design Toolbar")};
QString  NavigationDock::TabDesignProperties    {tr("SM Properties")};
QString  NavigationDock::TabDesignOutline       {tr("SM Outline")};

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
        case NavigationDock::eNaviWindow::NaviDesignToolbar:
            return NavigationDock::TabFsmToolbar;
        case NavigationDock::eNaviWindow::NaviDesignProperties:
            return NavigationDock::TabDesignProperties;
        case NavigationDock::eNaviWindow::NaviDesignOutline:
            return NavigationDock::TabDesignOutline;
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
    else if (tabName == NavigationDock::TabFsmToolbar)
        return NavigationDock::eNaviWindow::NaviDesignToolbar;
    else if (tabName == NavigationDock::TabDesignProperties)
        return NavigationDock::eNaviWindow::NaviDesignProperties;
    else if (tabName == NavigationDock::TabDesignOutline)
        return NavigationDock::eNaviWindow::NaviDesignOutline;
    else
        return NavigationDock::eNaviWindow::NaviUnknown;
}


NavigationDock::NavigationDock(MdiMainWindow* parent)
    : QWidget       (parent)

    , mMainWindow   (parent)
    , mTabs         (this)
    , mLiveScopes   (parent, this)
    , mOfflineScopes(parent, this)
    , mFileSystem   (parent, this)
{
    // The FSM drawing toolbar is no longer a navigation tab; it is a global ADS dock owned by
    // the main window (issue #516), reachable near the Design page or dragged to tab in here.
    mTabs.addTab(&mFileSystem   , NELusanCommon::iconViewWorkspace(NELusanCommon::SizeBig)  , NavigationDock::TabNameFileSystem);
    mTabs.addTab(&mLiveScopes   , NELusanCommon::iconViewLiveLogs(NELusanCommon::SizeBig)   , NavigationDock::TabLiveLogsExplorer);
    mTabs.addTab(&mOfflineScopes, NELusanCommon::iconViewOfflineLogs(NELusanCommon::SizeBig), NavigationDock::TabOfflineLogsExplorer);

    // Tooltips shown on every tab when hovered with the mouse.
    mTabs.setTabToolTip(0, tr("Workspace file explorer"));
    mTabs.setTabToolTip(1, tr("Live logs scope explorer"));
    mTabs.setTabToolTip(2, tr("Offline logs scope explorer"));

    mTabs.setTabPosition(QTabWidget::South);
    mTabs.setUsesScrollButtons(true);
    mTabs.setElideMode(Qt::ElideRight);
    mTabs.setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Expanding);

    // The tab widget is this content widget's whole body; the hosting ADS dock provides the
    // title bar and frame (issue #516).
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(&mTabs);

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

int NavigationDock::indexOfNavi(NavigationDock::eNaviWindow navi) const
{
    const QString& name = NavigationDock::getTabName(navi);
    const int count = mTabs.count();
    for (int i = 0; i < count; ++i)
    {
        if (mTabs.tabText(i) == name)
        {
            return i;
        }
    }

    return -1;
}

void NavigationDock::showDesignTab(NavigationDock::eNaviWindow navi, NavigationWindow* content)
{
    if (content == nullptr)
        return;

    int index = indexOfNavi(navi);
    if (index < 0)
    {
        // First placement: pick an icon per widget, but keep the current tab unchanged so
        // startup / document re-syncs do not steal focus from the workspace tab.
        QIcon icon;
        switch (navi)
        {
        case NavigationDock::eNaviWindow::NaviDesignToolbar:
            icon = NELusanCommon::iconViewFsmDesign(NELusanCommon::SizeBig);
            break;
        default:
            icon = NELusanCommon::iconStateMachine(NELusanCommon::SizeBig);
            break;
        }
        index = mTabs.addTab(content, icon, NavigationDock::getTabName(navi));
        mTabs.setTabVisible(index, true);
    }
    else
    {
        mTabs.setTabVisible(index, true);
    }

    // Do NOT force the content visible. Its widget lives in the tab widget's stacked layout,
    // which shows it only while its tab is the current one and hides every other page. An
    // unconditional show() painted the Design content (its "Design" toolbar, etc.) over the
    // current tab -- e.g. bleeding through the top-left of the Workspace tab on startup, since
    // showDesignTab deliberately keeps the current tab unchanged. Let the stacked layout own
    // the page visibility; only mirror it when this tab already happens to be the current one.
    if (mTabs.currentIndex() == index)
    {
        content->show();
    }
    else
    {
        content->hide();
    }
}

void NavigationDock::hideDesignTab(NavigationDock::eNaviWindow navi)
{
    const int index = indexOfNavi(navi);
    if (index < 0)
        return;

    QWidget* content = mTabs.widget(index);
    mTabs.removeTab(index);
    if (content != nullptr)
    {
        // The content is owned by the main window (it may be re-hosted in the Design page or
        // shown again later), so detach it from the tab widget without deleting it.
        content->setParent(mMainWindow);
        content->hide();
    }
}

bool NavigationDock::isDesignTabShown(NavigationDock::eNaviWindow navi) const
{
    return (indexOfNavi(navi) >= 0);
}

void NavigationDock::setNaviTabVisible(NavigationDock::eNaviWindow navi, bool visible)
{
    const int index = indexOfNavi(navi);
    if (index >= 0)
    {
        mTabs.setTabVisible(index, visible);
    }
}

bool NavigationDock::isNaviTabVisible(NavigationDock::eNaviWindow navi) const
{
    const int index = indexOfNavi(navi);
    return (index >= 0) && mTabs.isTabVisible(index);
}

void NavigationDock::initSize()
{
    // The user may shrink the dock down to the absolute minimum width (issue #516), while the
    // default/preferred width stays at MIN_NAVI_WIDTH. The tab widget must accept the same floor,
    // otherwise its own minimum would keep the dock from getting narrow.
    const int absMinWidth = static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH_ABS);
    const int defWidth = static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH);
    const int minHeight = static_cast<int>(NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumWidth(absMinWidth);
    setMinimumHeight(minHeight);
    mTabs.setMinimumWidth(absMinWidth);
    resize(QSize { defWidth, std::max(minHeight, height()) });
    setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
}

QSize NavigationDock::sizeHint() const
{
    QSize result{ QWidget::sizeHint() };
    result.setWidth(static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH));
    return result;
}

QSize NavigationDock::minimumSizeHint() const
{
    QSize result{ QWidget::minimumSizeHint() };
    result.setWidth(static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH_ABS));
    result.setHeight(std::max(result.height(), static_cast<int>(NELusanCommon::MIN_NAVI_HEIGHT)));
    return result;
}

void NavigationDock::onOptionsOpening()
{
    static_cast<NavigationWindow &>(mFileSystem).optionOpenning();
    static_cast<NavigationWindow &>(mLiveScopes).optionOpenning();
    static_cast<NavigationWindow &>(mOfflineScopes).optionOpenning();
}

void NavigationDock::onOptionsApplied()
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
