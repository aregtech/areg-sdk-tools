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
class QLineEdit;
class QRadioButton;
class QTableView;
class QToolButton;
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
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:
    //!< The radio buttons to select the type of logs to filter and display.
    enum eRadioType
    {
          RadioNone     =-1 //!< No radio button selected
        , RadioSession  = 0 //!< Radio button to filter logs by session
        , RadioSublogs  = 1 //!< Radio button to filter logs by sublogs (logs from nested methods call) of the thread
        , RadioScope    = 2 //!< Radio button to filter logs by scope
        , RadioThread   = 3 //!< Radio button to filter logs by thread
        , RadioProcess  = 4 //!< Radio button to filter logs by process
    };
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

    /**
     * \brief   Triggered when the user clicks on a radio button to select the type of logs to filter and display.
     * \param   checked     True if the radio button is checked, false otherwise.
     * \param   radio       The type of radio button that was clicked.
     **/
    void onRadioChecked(bool checked, eRadioType radio);

    //<!< Triggered when the filter scope selection indexes are changed, updates the log table view.
    void onFilterChanged(const QModelIndex & indexStart, const QModelIndex& indexEnd);

    /**
     * \brief   Shows and select the log in the log view of specified entry in the output window.
     * \param   idxTarget   The index of entry in the  output window
     **/
    inline void onShowLog(const QModelIndex& idxTarget);

    /**
     * \brief   Shows the log of the next nearest scope of the currently selected log row in output window.
     *          If the currently selected row is invalid, it selects the first log entry in the output window.
     *          If the currently selected row is the last scope message entry, moves to the end and does nothing.
     **/
    inline void onShowNextLog(void);

    /**
     * \brief   Shows the log of the previous nearest scope of the currently selected log row in output window.
     *          If the currently selected row is invalid, it selects the last log entry in the output window.
     *          If the currently selected row is the first scope message entry, moves to the start and does nothing.
     **/
    inline void onShowPrevLog(void);

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:

    //!< Updates the viewport of the log table.
    inline void updateLogTable(void);

    /**
     * \brief   Updates the controls, enabling or disabling them based on the current state.
     * \param   selectSession   If true and output window has logs, the select log session will be selected.
     */
    inline void updateControls(bool selectSession);
    
    /**
     * \brief   Updates the enable state of the tool-buttons.
     * \param   rowCount    The number of log rows in the output window
     * \param   selIndex    The index of currently selected row.
     **/
    inline void updateToolbuttons(int rowCount, const QModelIndex& selIndex);

    //!< Returns the index of the selected element of the logs in the output window.
    //!< No log is selected if return value is invalid.
    inline QModelIndex getSelectedIndex(void) const;

private:

    //!< Returns the pointer to the table view control.
    inline QTableView* ctrlTable(void) const;

    //!< Returns the pointer to the "show logs of the session" radio button control.
    inline QRadioButton* ctrlRadioSession(void) const;

    //!< Returns the pointer to the "show the session logs and sub-logs of the nested methods calls" radio button control.
    inline QRadioButton* ctrlRadioSublogs(void) const;

    //!< Returns the pointer to the "show all logs of the scope" check box control.
    inline QRadioButton* ctrlRadioScope(void) const;

    //!< Returns the pointer to the "show all logs of the thread" radio button control.
    inline QRadioButton* ctrlRadioThread(void) const;

    //!< Returns the pointer to the "show all logs of the process" radio button control.
    inline QRadioButton* ctrlRadioProcess(void) const;
    
    //!< The read-only edit control to display scope run duration since scope message has been activated.
    inline QLineEdit* ctrlDuration(void) const;

    //!< The tool button to show the logs of the selected entry in the log view window.
    inline QToolButton* ctrlLogShow(void) const;

    //!< The tool button to show the first log of the activated scope in the output window.
    inline QToolButton* ctrlScopeBegin(void) const;

    //!< The tool button to show the last log of the activated scope in the output window.
    inline QToolButton* ctrlScopeEnd(void) const;

    //!< The tool button to show the log of the next scope in the output window.
    inline QToolButton* ctrlScopeNext(void) const;

    //!< The tool button to show the log of the previous scope in the output window.
    inline QToolButton* ctrlScopePrev(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::ScopeOutputViewer*  ui;         //!< The user interface object, generated by Qt Designer
    ScopeLogViewerFilter*   mFilter;    //!< The filter for scope logs.
    LoggingModelBase*       mLogModel;  //<!< The pointer to the logging model, can be nullptr.
};

#endif  // LUSAN_VIEW_LOG_SCOPEOUTPUTVIEWER_HPP
