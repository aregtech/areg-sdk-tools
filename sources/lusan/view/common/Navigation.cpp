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

Navigation::Navigation(MdiMainWindow* parent)
    : QDockWidget   (tr("Navigation"), parent)
    , mTabs         (this)
    , mFileSystem   (parent, this)
{
    mTabs.addTab(&mFileSystem, tr("Workspace"));
    mTabs.setTabPosition(QTabWidget::South);
    setWidget(&mTabs);
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
    bool result {false};
    int count { mTabs.count()};
    for (int i = 0; i < count; ++i)
    {
        if ((result = mTabs.tabText(i) == tabName))
        {
            break;
        }
    }
    
    return result;
}
