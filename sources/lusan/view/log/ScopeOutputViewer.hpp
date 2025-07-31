#ifndef LUSAN_VIEW_LOG_SCOPEOUTPUTVIEWER_HPP
#define LUSAN_VIEW_LOG_SCOPEOUTPUTVIEWER_HPP
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
 *  \file        lusan/view/log/ScopeOutputViewer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Scope output viewer widget.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/OutputWindow.hpp"
#include "areg/component/NEService.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/

class ScopeLogViewerFilter;
class LoggingModelBase;
class QCheckBox;
class QTableView;
namespace Ui {
    class ScopeOutputViewer;
}

/**
 * \brief   The scope logs viewer object to display on output window and analyze selected group of logs.
 **/
class ScopeOutputViewer : public OutputWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    ScopeOutputViewer(MdiMainWindow* wndMain, QWidget* parent = nullptr);
    virtual ~ScopeOutputViewer(void);
    
//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    
    /**
     * \brief   Releases the MDI window previously bind with the output window and returns true if succeeded to release.
     *          Nothing happens if the window was not bound, and the return value is false.
     *          There can be only one MDI window bound with output window.
     * \param   mdiChild    The MDI child window to release
     * \return  Returns true if succeeded to release the window and returns false if the window is not bound.
     **/   
    virtual bool releaseWindow(MdiChild& mdiChild) override;
    
//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets up the filter for the log model.
     *          The method sets the scope ID, session IDs, instance IDs, and priority bits to filter log messages.
     * \param   logModel    The pointer to the logging source model to filter.
     * \param   scopeId     The ID of the scope to filter, pass 0 if there is no scope ID is set.
     * \param   sessionIds  The list of session IDs to filter.
     * \param   instanceIds The list of instance IDs to filter.
     * \param   priorityBits The priority bits to filter log messages.
     **/
    void setupFilter(LoggingModelBase* logModel, uint32_t scopeId, uint32_t sessionId, ITEM_ID instance);

    /**
     * \brief   Sets up the filter for the log model based on the given index.
     *          The method sets the scope ID, session IDs, instance IDs, and priority bits to filter log messages.
     * \param   logModel    The pointer to the logging source model to filter.
     * \param   index       The index in the source model to filter.
     **/
    void setupFilter(LoggingModelBase* logModel, const QModelIndex& index);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:

    //!< Triggered when the user clicks on "show all logs of the scope" check box.
    void onScopeChecked(bool checked);

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:

    //!< Triggered when the user double-clicks on a log entry in the table view.
    inline void onMouseDoubleClicked(const QModelIndex& index);
    
    //!< Returns the pointer to the table view control.
    inline QTableView* ctrlTable(void) const;

    //!< Returns the pointer to the "show all logs of the scope" check box control.
    inline QCheckBox* ctrlCheckScope(void) const;

    //!< Updates the viewport of the log table.
    inline void updateLogTable(void);

    //!< Updates the controls, enabling or disabling them based on the current state.
    inline void updateControls(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::ScopeOutputViewer*  ui;         //!< The user interface object, generated by Qt Designer
    ScopeLogViewerFilter*   mFilter;    //!< The filter for scope logs.
    LoggingModelBase*       mLogModel;  //<!< The pointer to the logging model, can be nullptr.
};

#endif  // LUSAN_VIEW_LOG_SCOPEOUTPUTVIEWER_HPP
