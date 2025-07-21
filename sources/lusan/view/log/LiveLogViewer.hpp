#ifndef LUSAN_VIEW_LOG_LOGVIEWER_HPP
#define LUSAN_VIEW_LOG_LOGVIEWER_HPP
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
 *  \file        lusan/view/log/LiveLogViewer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/log/LogViewerBase.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class QToolButton;
class QLabel;
class QWidget;
class MdiMainWindow;

namespace Ui {
    class LiveLogViewer;
}

/**
 * \brief   Log viewer MDI window
 **/
class LiveLogViewer : public LogViewerBase
{
    Q_OBJECT

private:

    static const QString    _tooltipPauseLogging;   //!< Tooltip for the Pause logging button.
    static const QString    _tooltipResumeLogging;  //!< Tooltip for the Resume logging button.
    static const QString    _tooltipStopLogging;    //!< Tooltip for the Stop logging button.
    static const QString    _tooltipRestartLogging; //!< Tooltip for the Restart logging button.

public:
    /**
     * \brief   Returns the file extension of the service interface document.
     **/
    static const QString& fileExtension(void);

    /**
     * \brief   Returns the file name of the log viewer.
     *          The file name is generated based on the current date and time.
     **/
    static QString generateFileName(void);

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor.
     * \param   wndMain    The main window of the application.
     * \param   parent     The parent widget.
     **/
    explicit LiveLogViewer(MdiMainWindow *wndMain, QWidget *parent = nullptr);
    
    virtual ~LiveLogViewer(void);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Called when lusan application is connected to the logging service.
     * \param   isConnected     Flag, indicating that whether the service is connected or disconnected.
     * \param   address         The address of the connected logging service
     * \param   port            The TCP port number of the connected logging service.
     * \param   dbPath          The path to the logging database.
     **/
    void logServiceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

    /**
     * \brief   Called when the log database is created.
     **/
    void logDatabaseCreated(const QString& dbPath);

    /**
     * \brief   Returns true if application is connected to the logging service.
     **/
    bool isServiceConnected(void) const;

    /**
     * \brief   Called to check whether log viewer has log entries.
     **/
    bool isEmpty(void) const;

    /**
     * \brief   Called to detach the log viewer and stop receiving messages.
     **/
    void detachLiveLog(void);

    /**
     * \brief   Returns the database path of the live logs database.
     **/
    QString getDatabasePath(void) const;

/************************************************************************
 * MdiChild overrides
 ************************************************************************/
protected:

    /**
     * \brief   Called when the MDI child window is closed.
     * \param   isActive    Indicates whether the window is active or not.
     **/
    virtual void onWindowClosing(bool isActive) override;

    /**
     * \brief   Slot. which triggered when the selection in the log scopes navigation is changed.
     **/
    virtual void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous) override;
    
//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Triggered when new entry is inserted in the table
     **/
    void onRowsInserted(const QModelIndex &parent, int first, int last);

    /**
     * \brief   Slot, triggered when Pause / Resume toolbutton is clicked.
     * \param   checked     If true, requested to pause. If false, requested to resume.
     **/
    void onPauseClicked(bool checked);

    /**
     * \brief   Slot, triggered when Stop / Restart toolbutton is clicked.
     *          When stops, closes log database.
     *          When restarts, creates new database.
     * \param   checked     If true, requested to stop. If false, requested to restart logging.
     **/
    void onStopClicked(bool checked);

    /**
     * \brief   Slot, triggered when Clear logging window button is clicked.
     **/
    void onClearClicked(void);

private:
    //!< Returns Pause / Resume toolbutton
    QToolButton* ctrlPause(void);

    //!< Returns Stop / Restart toolbutton
    QToolButton* ctrlStop(void);

    //!< Returns Clear logs toolbutton
    QToolButton* ctrlClear(void);

    //!< Returns Logging File name label widget.
    QLabel* ctrlFile(void);
        
    /**
     * \brief   Resets the order of the columns.
     **/
    void resetColumnOrder();

    /**
     * \brief   Updates the tool-buttons based on the current state of logging.
     * \param   isPaused    If true, logging is paused. Show resume button.
     * \param   isStopped   If true, logging is stopped. Show restart button.
     **/
    void updateToolbuttons(bool isPaused, bool isStopped);

    /**
     * \brief   Call to connect or disconnect the signals of the logging model and UI controls.
     * \param   doSetup     If true, connects the signals; if false, disconnects the signals.
     **/
    void setupSignals(bool doSetup);

    /**
     * \brief   Cleans up resources used by the offline log viewer.
     *          This method is called when the viewer is closed or no longer needed.
     **/
    void cleanResources(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::LiveLogViewer*      ui;             //!< User interface object, generated by Qt Designer.
};

#endif // LUSAN_VIEW_LOG_LIVELOGVIEWER_HPP
