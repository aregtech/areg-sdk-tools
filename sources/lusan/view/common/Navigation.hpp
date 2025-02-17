#ifndef LUSAN_VIEW_COMMON_NAVIGATION_HPP
#define LUSAN_VIEW_COMMON_NAVIGATION_HPP
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
 *  \file        lusan/view/common/Navigation.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/

#include "lusan/view/common/NaviFileSystem.hpp"

#include <QDockWidget>
#include <QTabWidget>

class MdiMainWindow;

class Navigation : public QDockWidget
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    Navigation(MdiMainWindow* parent);
    
//////////////////////////////////////////////////////////////////////////
// Actions and attributes
//////////////////////////////////////////////////////////////////////////
public:
    inline QTabWidget& getTabWidget(void);

    inline NaviFileSystem& getFileSystem(void);

    inline int addTab(QWidget& widget, const QString& tabName);

    QWidget* getTab(const QString& tabName) const;
    
    bool tabExists(const QString& tabName) const;
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    inline Navigation& self(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QTabWidget      mTabs;
    NaviFileSystem  mFileSystem;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline QTabWidget& Navigation::getTabWidget(void)
{
    return mTabs;
}

inline NaviFileSystem& Navigation::getFileSystem(void)
{
    return mFileSystem;
}

inline int Navigation::addTab(QWidget& widget, const QString& tabName)
{
    return mTabs.addTab(&widget, tabName);
}

inline Navigation& Navigation::self(void)
{
    return (*this);
}

#endif  // LUSAN_VIEW_COMMON_NAVIGATION_HPP
