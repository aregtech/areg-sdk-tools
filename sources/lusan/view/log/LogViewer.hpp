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
 *  \file        lusan/view/log/LogViewer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/IEMdiWindow.hpp"

class QHeaderView;
class QTableView;
class QWidget;
class LogViewerModel;

namespace Ui {
    class LogViewer;
}

class LogViewer : public MdiChild
                , public IEMdiWindow
{
    Q_OBJECT

public:
    /**
     * \brief   Returns the file extension of the service interface document.
     **/
    static const QString& fileExtension(void);

    static QString generateFileName(void);

public:
    explicit LogViewer(QWidget *parent = nullptr);
    
    /**
     * \brief   Called when lusan application is connected to the logging service.
     * \param   isConnected     Flag, indicating that whether the service is connected or disconnected.
     * \param   address         The address of the connected logging service
     * \param   port            The TCP port number of the connected logging service.
     * \param   dbPath          The path to the logging database.
     **/
    void logServiceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

    /**
     * \brief   Returns true if application is connected to the logging service.
     **/
    bool isServiceConnected(void) const;

public:

    QString newLogFile(void) const;

private:
    /**
     * \brief   Returns the pointer to the log table object.
     **/
    QTableView* getTable(void);

    /**
     * \brief   Returns the pointer to the header object.
     **/
    QHeaderView* getHeader(void);
    
//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Triggered when new entry is inserted in the table
     **/
    void onRowsInserted(const QModelIndex &parent, int first, int last);

private:
    Ui::LogViewer*      ui;
    LogViewerModel*     mLogModel;
    QWidget*            mMdiWindow;
};

#endif // LUSAN_VIEW_LOG_LOGVIEWER_HPP
