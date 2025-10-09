#ifndef LUSAN_VIEW_COMMON_OUTPUTDOCK_HPP
#define LUSAN_VIEW_COMMON_OUTPUTDOCK_HPP
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
 *  \file        lusan/view/common/OutputDock.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Output docking window.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDockWidget>
#include "areg/base/GEGlobal.h"

#include "lusan/view/log/ScopeOutputViewer.hpp"

#include <QTabWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;

/**
 * \brief   The OutputDock class is a dockable widget that contains windows for analyzes
 **/
class OutputDock : public QDockWidget
{
    Q_OBJECT

////////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////////
public:
    /**
     * /brief   Defines the possible status values for a window.
     **/
    enum eOutputDock
    {
          OutputUnknown     //!< Unknown output window
        , OutputLogging     //!< Status window for log analyzes
    };

    //!< The tab name for the logging output window
    static const QString    TabNameLogging;

    //!< Returns the tab name of the specified output window
    static const QString& getTabName(OutputDock::eOutputDock wndStatus);

    //!< Returns the output window type by specified tab name.
    static OutputDock::eOutputDock getOutputDock(const QString& tabName);

//////////////////////////////////////////////////////////////////////////
// Constructor and destructor
//////////////////////////////////////////////////////////////////////////
public:

    OutputDock(MdiMainWindow* parent);
    
    virtual ~OutputDock(void);

//////////////////////////////////////////////////////////////////////////
// Actions and attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the tab widget of the navigation.
     **/
    inline QTabWidget& getTabWidget(void);

    /**
     * \brief   Returns the scope output viewer.
     **/
    inline ScopeOutputViewer& getScopeLogsView(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    //!< Initializes the size of the output dock.
    void initSize(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MdiMainWindow*          mMainWindow;    //!< Main window
    QTabWidget              mTabs;          //!< The tab widget of the output windows.
    ScopeOutputViewer       mScopeOutput;   //<!< The scope output viewer for displaying logs from scopes.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(OutputDock);
};

//////////////////////////////////////////////////////////////////////////
// OutputDock class inline methods
//////////////////////////////////////////////////////////////////////////

inline QTabWidget& OutputDock::getTabWidget(void)
{
    return mTabs;
}

inline ScopeOutputViewer& OutputDock::getScopeLogsView(void)
{
    return mScopeOutput;
}

#endif  // LUSAN_VIEW_COMMON_OUTPUTDOCK_HPP
