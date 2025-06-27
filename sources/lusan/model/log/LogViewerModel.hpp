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
#include <QMetaObject>

#include "areg/base/SharedBuffer.hpp"
#include "areg/base/File.hpp"

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
public:

    //!< The index of columns
    enum class eColumn  : int
    {
          LogColumnInvalid      = -1//!< Invalid column index, used for error checking
        , LogColumnPriority     = 0 //!< Log message priority
        , LogColumnTimestamp        //!< Log message timestamp
        , LogColumnSource           //!< Log message source name
        , LogColumnSourceId         //!< Log message source ID
        , LogColumnThread           //!< Log message thread name
        , LogColumnThreadId         //!< Log message thread ID
        , LogColumnScopeId          //!< Log message scope ID
        , LogColumnMessage          //!< Log message text
        
        , LogColumnCount            //!< Maximum number of columns
    };
    
//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the file extension of the logs database.
     **/
    static QString getFileExtension();

    /**
     * \brief   Generates and returns file name of the log database base on the information set in the initialization file.
     *          Normalizes the file name if it contains a mask, such as "log_%time%.sqlog".
     **/
    static QString generateFileName(void);

    /**
     * \brief   Generates and returns the path to the log database new file.
     *          The log database location and file name are set in the initialization file.
     *          Normalizes the file name if it contains a mask, such as "log_%time%.sqlog".
     **/
    static QString newFileName(void);

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
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
     * \brief   Returns the fixed list of header sizes.
     *          The sizes are in pixels and correspond to the header names.
     **/
    static const QList<int>& getHeaderWidths(void);
    
    /**
     * \brief   Returns the default list of header names.
     **/
    static const QList<LogViewerModel::eColumn>& getDefaultColumns(void);

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

    /**
     * \brief   Return the file name of the log database to set as a title of the log viewer window.
     **/
    inline QString getLogFileName(void) const;

    /**
     * \brief   Returns the address of the log collector service.
     **/
    inline QString getLofServiceAddress(void) const;

    /**
     * \brief   Returns the port of the log collector service.
     **/
    inline uint16_t getLogServicePort(void) const;

    /**
     * \brief   Resets the data in the model.
     *          Clears the list of log messages and resets the model.
     **/
    inline void dataReset(void);

    /**
     * \brief   Returns true if the model is empty, i.e. contains no log messages.
     **/
    inline bool isEmpty(void) const;

    /**
     * \brief   Finds the index of the specified column.
     * \param   col The column to find.
     * \return  The index of the column, or -1 if not found.
     **/
    inline int findColumn(LogViewerModel::eColumn col) const;

    /**
     * \brief   Returns the list of active columns.
     *          The active columns are the ones that are currently visible in the log viewer.
     **/
    inline QList<LogViewerModel::eColumn> getActiveColumns(void) const;

    /**
     * \brief   Adds a column at a given position of active columns list.
     *          If -1, adds a column before the "Log messages" column.
     * \param   col     The column to add.
     * \param   pos     The position of column to add.
     *                  If -1, adds column before "Log messages" column.
     **/
    void addColumn(LogViewerModel::eColumn col, int pos = -1);

    /**
     * \brief   Removes specified column from the active columns list.
     * \param   col     The column to remove.
     **/
    void removeColumn(LogViewerModel::eColumn col);

    /**
     * \brief   Sets list of active columns. If empty, it resets the default columns
     * \param   columns     The list of active columns to set.
     *                      If empty, resets the columns.
     **/
    void setActiveColumns(const QList< LogViewerModel::eColumn>& columns);

    /**
     * \brief   Call to pause logging. When logging is paused,
     *          on resume it continues writing logs in the same database.
     **/
    void pauseLogging(void);

    /**
     * \brief   Call to resume logging. When logging is resumed,
     *          it continues writing logs in the same database.
     *          Only paused logging can be resumed.
     **/
    void resumeLogging(void);

    /**
     * \brief   Call to stop logging. When logging is stopped,
     *          it closes the database and stops writing logs.
     *          On restart, it creates new database.
     **/
    void stopLogging(void);

    /**
     * \brief   Call to restart logging. When logging is restarted,
     *          it creates new database.
     * \param   dbName  The database name to restart logging.
     *                  If empty string, uses the name set in the configuration file.
     *                  The name may have a mask, such as "log_%time%.sqlog",
     **/
    void restartLogging(const QString & dbName = QString());

    /**
     * \brief   Returns maximum number of columns that is possible to set in the log viewer.
     **/
    inline int getMaxColumCount(void) const;

    /**
     * \brief   Converts the column to its index in the active columns list.
     **/
    inline int fromColumnToIndex(LogViewerModel::eColumn col) const;

    /**
     * \brief   Converts the index to its column in the active columns list.
     *          Returns LogColumnInvalid if index is invalid.
     * \param   logicalIndex  The logical index of the column.
     **/
    inline LogViewerModel::eColumn fromIndexToColumn(int logicalIndex) const;
    
//////////////////////////////////////////////////////////////////////////
// Slots.
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   The slot is triggered when receive message to log.
     * \param   logMessage  The structure of the message to log.
     **/
    void slotLogMessage(const SharedBuffer& logMessage);
    
    /**
     * \brief   The slot is triggered when the observer connects or disconnects from the log collector service.
     * \param   isConnected     Flag, indicating whether observer is connected or disconnected.
     * \param   address         The IP address of the log collector service to connect or disconnect.
     * \param   port            The IP port number of the log collector service to connect or disconnect.
     **/
    void slotLogServiceConnected(bool isConnected, const QString& address, uint16_t port);
    
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
    QMetaObject::Connection mConLogger; //!< The connection signal
    QMetaObject::Connection mConLogs;   //!< The connection signal
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

inline QString LogViewerModel::getLogFileName(void) const
{
    return QString(mDbPath.isEmpty() == false ? File::getFileNameWithExtension(mDbPath.toStdString().c_str()).getString() : "");
}

inline QString LogViewerModel::getLofServiceAddress(void) const
{
    return mAddress;
}

inline uint16_t LogViewerModel::getLogServicePort(void) const
{
    return mPort;
}

inline void LogViewerModel::dataReset(void)
{
    beginResetModel();
    mLogs.clear();
    endResetModel();
}

inline bool LogViewerModel::isEmpty(void) const
{
    return mLogs.isEmpty();
}

inline int LogViewerModel::findColumn(LogViewerModel::eColumn col) const
{
    return static_cast<int>(mActiveColumns.indexOf(col));
}

inline QList<LogViewerModel::eColumn> LogViewerModel::getActiveColumns(void) const
{
    return mActiveColumns;
}

inline int LogViewerModel::getMaxColumCount(void) const
{
    return static_cast<int>(eColumn::LogColumnCount);
}

inline int LogViewerModel::fromColumnToIndex(LogViewerModel::eColumn col) const
{
    return mActiveColumns.indexOf(col);
}

inline LogViewerModel::eColumn LogViewerModel::fromIndexToColumn(int logicalIndex) const
{
    return ((logicalIndex >= 0) && (logicalIndex < static_cast<int>(mActiveColumns.size())) ? mActiveColumns[logicalIndex] : LogViewerModel::eColumn::LogColumnInvalid);
}


#endif // LUSAN_MODEL_LOG_LOGVIEWERMODEL_HPP
