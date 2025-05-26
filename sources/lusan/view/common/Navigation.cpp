/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/Navigation.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/
#include "lusan/view/common/Navigation.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"

QString  Navigation::TabNameFileSystem      {tr("Workspace")};
QString  Navigation::TabLiveLogsExplorer    {tr("Live Logs")};
QString  Navigation::TabOfflineLogsExplorer {tr("Offline Logs")};
QSize    Navigation::IconSize               {32, 32};

QIcon Navigation::getWorkspaceExplorerIcon(void)
{
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/workspace-explorer"), Navigation::IconSize, QIcon::Mode::Normal, QIcon::State::Off);
    return icon;
}

QIcon Navigation::getLiveLogIcon(void)
{
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/log-live"), Navigation::IconSize, QIcon::Mode::Normal, QIcon::State::Off);
    return icon;
}

QIcon Navigation::getOfflineLotIcon(void)
{
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/log-offline"), Navigation::IconSize, QIcon::Mode::Normal, QIcon::State::Off);
    return icon;
}

Navigation::Navigation(MdiMainWindow* parent)
    : QDockWidget   (tr("Navigation"), parent)
    , mTabs         (this)
    , mLogExplorer  (parent, this)
    , mFileSystem   (parent, this)
{    
    mTabs.addTab(&mFileSystem, Navigation::getWorkspaceExplorerIcon(), Navigation::TabNameFileSystem);
    mTabs.addTab(&mLogExplorer, Navigation::getLiveLogIcon(), Navigation::TabLiveLogsExplorer);
    mTabs.setTabPosition(QTabWidget::South);
    setWidget(&mTabs);

    initSize();
}

QWidget* Navigation::getTab(const QString& tabName) const
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
    
    return result;
}

bool Navigation::tabExists(const QString& tabName) const
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

bool Navigation::showTab(const QString& tabName)
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

void Navigation::initSize()
{
    resize(QSize { mFileSystem.width() + 10,  mFileSystem.height() });
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
}
