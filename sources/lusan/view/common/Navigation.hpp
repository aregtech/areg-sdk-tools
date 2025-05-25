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

#include "lusan/view/common/LogExplorer.hpp"
#include "lusan/view/common/NaviFileSystem.hpp"

#include <QDockWidget>
#include <QIcon>
#include <QSize>
#include <QTabWidget>

class MdiMainWindow;

class Navigation : public QDockWidget
{
//////////////////////////////////////////////////////////////////////////
// Constants, types and static methods
//////////////////////////////////////////////////////////////////////////
public:

    //!< The name of the tab for workspace explorer.
    static QString  TabNameFileSystem;
    //!< The name of the tab for live logs explorer.
    static QString  TabLiveLogsExplorer;
    //!< The name of the tab for offline logs explorer.
    static QString  TabOfflineLogsExplorer;
    //<!< The size of icons.
    static QSize    IconSize;

    //!< Returns the icon for the workspace explorer tab.
    static QIcon getWorkspaceExplorerIcon(void);

    //!< Returns the icon for the live logs explorer tab.
    static QIcon getLiveLogIcon(void);

    //!< Returns the icon for the offline logs explorer tab.
    static QIcon getOfflineLotIcon(void);
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    Navigation(MdiMainWindow* parent);
    
//////////////////////////////////////////////////////////////////////////
// Actions and attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the tab widget of the navigation.
     **/
    inline QTabWidget& getTabWidget(void);

    /**
     * \brief   Returns the file system widget.
     **/
    inline NaviFileSystem& getFileSystem(void);

    /**
     * \brief   Returns the live mode log explorer widget.
     **/
    inline LogExplorer& getLiveLogs(void);

    /**
     * \brief   Adds a new tab with the widget to the tab-control.
     * \param   widget      The widget to add to the tab-control.
     * \brief   tabName     The name of the tab to add.
     * \return  The index of the new added tab.
     **/
    inline int addTab(QWidget& widget, const QString& tabName);

    /**
     * \brief   Returns the pointer to the widget of the given tab name.
     *          Returns nullptr if the name does not exist.
     * \param   tabName     The name of tab to return the widget.
     * \return  Returns the valid pointer of the Widget of the given tab name.
     *          Returns nullptr if tab name does not exist.
     **/
    QWidget* getTab(const QString& tabName) const;

    /**
     * \brief   Check if the tab with the given name exists.
     * @param   tabName     The name of the tab to check.
     * @return  Returns true if the tab with the given name exists. False, otherwise.
     **/
    bool tabExists(const QString& tabName) const;
    
    bool showTab(const QString& tabName);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns the instance of Navigation window.
     **/
    inline Navigation& self(void);

    /**
     * \brief   Initializes the size of tab widgets.
     **/
    void initSize();
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QTabWidget      mTabs;          //!< The tab widget of the navigation.
    LogExplorer     mLogExplorer;   //!< The log explorer widget.
    NaviFileSystem  mFileSystem;    //!< The file system widget.
};

//////////////////////////////////////////////////////////////////////////
// Navigation class inline methods
//////////////////////////////////////////////////////////////////////////

inline QTabWidget& Navigation::getTabWidget(void)
{
    return mTabs;
}

inline NaviFileSystem& Navigation::getFileSystem(void)
{
    return mFileSystem;
}

inline LogExplorer& Navigation::getLiveLogs(void)
{
    return mLogExplorer;
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
