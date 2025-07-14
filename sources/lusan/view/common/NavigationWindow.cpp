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
 *  \file        lusan/view/common/NavigationWindow.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation windows elements.
 *
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"
#include "lusan/view/common/Navigation.hpp"

NavigationWindow::NavigationWindow(int naviWindow, MdiMainWindow * wndMain, QWidget* parent)
    : QWidget(parent)

    , mNaviWindowType   (naviWindow)
    , mMainWindow       (wndMain)
{
}

void NavigationWindow::optionOpenning(void)
{
}

void NavigationWindow::optionApplied(void)
{
}

void NavigationWindow::optionClosed(bool OKpressed)
{
}

bool NavigationWindow::isNaviWorkspace(void) const
{
    return (mNaviWindowType == static_cast<int>(Navigation::eNaviWindow::NaviWorkspace));
}

bool NavigationWindow::isNaviLiveLogs(void) const
{
    return (mNaviWindowType == static_cast<int>(Navigation::eNaviWindow::NaviLiveLogs));
}

bool NavigationWindow::isNaviOfflineLogs(void) const
{
    return (mNaviWindowType == static_cast<int>(Navigation::eNaviWindow::NaviOfflineLogs));
}

void NavigationWindow::resetNavigator(QAbstractTableModel * model)
{

}
