#ifndef LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
#define LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
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
 *  \file        lusan/model/log/LogViewerModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QAbstractTableModel>
#include <QList>

#include "areg/base/SharedBuffer.hpp"

class LogObserverComp;

/**
 * \brief   The model for the log viewer window.
 **/
class LogViewerModel : public QAbstractTableModel
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:
    enum class eColumn  : int
    {
          LogColumnPriority     = 0
        , LogColumnTimestamp
        , LogColumnSource
        , LogColumnSourceId
        , LogColumnThread
        , LogColumnThreadId
        , LogColumnScopeId
        , LogColumnMessage
    };
    
public:
    explicit LogViewerModel(QObject *parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    // Header:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add data:
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    
//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the fixed list of header names.
     **/
    static const QStringList&  getHeaderList(void);
    
    /**
     * \brief   Returns the default list of header names.
     **/
    static const QList<int>& getDefaultColumns(void);

    /**
     * \brief   Returns the header name of the specified column.
     * \param   colIndex The zero-based index of the column.
     **/
    QString getHeaderName(int colIndex) const;

    /**
     * \brief   Call to start connection to the log collector service.
     * \param   hostName    The host name of the log collector service.
     * \param   portNr      The port number of the log collector service.
     * \return  True if the connection process started, false otherwise.
     **/
    bool connectService(const QString& hostName = "", unsigned short portNr = 0u);

    /**
     * \brief   Call to start disconnection from the log collector service.
     **/
    void disconnectService(void);

    /**
     * \brief   Triggered when service is connected.
     * \param   isConnected  True if the service is connected, false otherwise.
     * \param   address      The address of the service.
     * \param   port         The port of the service.
     * \param   dbPath       The path to the database.
     **/
    void serviceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath);

    /**
     * \brief   Returns true if application is connected to the log collector service.
     **/
    inline bool isConnected(void) const;

    /**
     * \brief   Sets the path to the log data database.
     * \param   dbPath  The path to the database.
     **/
    inline void setDatabasePath(const QString& dbPath);

    /**
     * \brief   Returns the path to the log data database.
     **/
    inline const QString& getDabasePath(void) const;
    
//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   The callback of the event triggered when receive message to log.
     * \param   logMessage  The structure of the message to log.
     **/
    void slotLogMessage(const SharedBuffer& logMessage);

//////////////////////////////////////////////////////////////////////////
// Member variable
//////////////////////////////////////////////////////////////////////////
private:
    bool                mIsConnected;   //!< Flag to indicate whether application is connected to log collector service
    QString             mAddress;       //!< The address of the log collector service
    uint16_t            mPort;          //!< The port of the log collector service
    QString             mDbPath;        //!< The path to the database
    QList<eColumn>      mActiveColumns; //!< The list of active columns
    QList<SharedBuffer> mLogs;          //!< The list of log messages
};

//////////////////////////////////////////////////////////////////////////
// LogViewerModel class inline methods.
//////////////////////////////////////////////////////////////////////////

inline bool LogViewerModel::isConnected(void) const
{
    return mIsConnected;
}

inline void LogViewerModel::setDatabasePath(const QString& dbPath)
{
    mDbPath = dbPath;
}

inline const QString& LogViewerModel::getDabasePath(void) const
{
    return mDbPath;
}

#endif // LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
