#ifndef LUSAN_MODEL_LOG_LOGGINGMODELBASE_HPP
#define LUSAN_MODEL_LOG_LOGGINGMODELBASE_HPP
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
 *  \file        lusan/model/log/LoggingModelBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Logging model base class.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "areg/base/File.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/String.hpp"
#include "areg/base/SynchObjects.hpp"
#include "areg/base/Thread.hpp"
#include "areg/base/IEThreadConsumer.hpp"
#include "areg/component/NEService.hpp"
#include "areg/logging/NELogging.hpp"
#include "aregextend/db/LogSqliteDatabase.hpp"
#include "aregextend/db/SqliteStatement.hpp"

#include <QAbstractTableModel>
#include <QList>
#include <QString>
#include <QVariant>

#include <map>
#include <vector>


/************************************************************************
 * Dependencies
 ************************************************************************/
class LogViewerFilterProxy;

/**
 * \brief   Base class for log viewer models (live and offline).
 *          Provides common data and interface for log models.
 **/
class LoggingModelBase  : public    QAbstractTableModel
                        , protected IEThreadConsumer
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:

    //!< The index of columns (reusing LoggingModelBase columns for compatibility)
    enum class eColumn : int
    {
          LogColumnInvalid  = -1    //!< Invalid column index, used for error checking
        , LogColumnPriority = 0     //!< Log message priority
        , LogColumnTimestamp        //!< Log message timestamp
        , LogColumnSource           //!< Log message source name
        , LogColumnSourceId         //!< Log message source ID
        , LogColumnThread           //!< Log message thread name
        , LogColumnThreadId         //!< Log message thread ID
        , LogColumnScopeId          //!< Log message scope ID
        , LogColumnMessage          //!< Log message text

        , LogColumnCount            //!< Maximum number of columns
    };

    //!< The logging type, indicating the state of the logging model
    enum class eLogging : int
    {
          LoggingUndefined  = 0 // Undefined logging state
        , LoggingLive           // Live logging, connected to the log collector service
        , LoggingOffline        // Offline logging, reading from a database file
        , LoggingDisconneced    // Logging was live, it is disconnected from the log collector service, but still is connected to the database.
    };

    using   ListColumns     = QList<LoggingModelBase::eColumn>;
    using   ListLogs        = std::vector<SharedBuffer>;
    using   ListInstances   = std::vector< NEService::sServiceConnectedInstance>;
    using   ListScopes      = std::vector< NELogging::sScopeInfo>;
    using   MapScopes       = std::map<ITEM_ID, ListScopes>;

//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the file extension of the logs database.
     **/
    static const QString & getFileExtension();

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
    static const QList<LoggingModelBase::eColumn>& getDefaultColumns(void);

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LoggingModelBase(LoggingModelBase::eLogging logsType, QObject* parent = nullptr);
    virtual ~LoggingModelBase() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    // Header:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // Add/Remove data:
    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the header name of the specified column.
     * \param   colIndex The zero-based index of the column.
     **/
    QString getHeaderName(int colIndex) const;

    /**
     * \brief   Finds the index of the specified column.
     * \param   col The column to find.
     * \return  The index of the column, or -1 if not found.
     **/
    inline int findColumn(LoggingModelBase::eColumn col) const;

    /**
     * \brief   Returns the list of active columns.
     *          The active columns are the ones that are currently visible in the log viewer.
     **/
    inline const QList<LoggingModelBase::eColumn> & getActiveColumns() const;

    /**
     * \brief   Returns maximum number of columns that is possible to set in the log viewer.
     **/
    inline int getMaxColumCount(void) const;

    /**
     * \brief   Returns true if the model is empty, i.e. contains no log messages.
     **/
    inline bool isEmpty(void) const;

    /**
     * \brief   Resets the data in the model.
     *          Clears the list of log messages and resets the model.
     **/
    inline void dataReset(void);

    /**
     * \brief   Converts the column to its index in the active columns list.
     **/
    inline int fromColumnToIndex(LoggingModelBase::eColumn col) const;

    /**
     * \brief   Converts the index to its column in the active columns list.
     *          Returns LogColumnInvalid if index is invalid.
     * \param   logicalIndex  The logical index of the column.
     **/
    inline LoggingModelBase::eColumn fromIndexToColumn(int logicalIndex) const;

    /**
     * \brief   Adds a column at a given position of active columns list.
     *          If -1, adds a column before the "Log messages" column.
     * \param   col     The column to add.
     * \param   pos     The position of column to add.
     *                  If -1, adds column before "Log messages" column.
     **/
    void addColumn(LoggingModelBase::eColumn col, int pos = -1);

    /**
     * \brief   Removes specified column from the active columns list.
     * \param   col     The column to remove.
     **/
    void removeColumn(LoggingModelBase::eColumn col);

    /**
     * \brief   Sets list of active columns. If empty, it resets the default columns
     * \param   columns     The list of active columns to set.
     *                      If empty, resets the columns.
     **/
    void setActiveColumns(const QList< LoggingModelBase::eColumn>& columns);

    /**
     * \brief   Return the file name of the log database to set as a title of the log viewer window.
     **/
    inline QString getLogFileName(void) const;

    /**
     * \brief   Returns the full path to the log file.
     *          If the log file is not set, returns an empty string.
     **/
    inline QString getLogFilePath(void) const;

    /**
     * \brief   Marks the logging model as disconnected logging.
     *          It should be called the live logging is disconnected
     *          from the log collector service.
     **/
    inline void markDisconnected( void );

    /**
     * \brief   Returns the type of logging.
     *          It can be either live, offline or disconnected logging.
     **/
    inline LoggingModelBase::eLogging getLoggingType(void) const;

    /**
     * \brief   Returns true if logging type is live logging.
     *          It means that the log collector service is connected to the log collector service.
     **/
    inline bool isLiveLogging(void) const;

    /**
     * \brief   Returns true if logging type is offline logging.
     *          It means that the log database is opened and ready to read.
     **/
    inline bool isOfflineLogging(void) const;

    /**
     * \brief   Returns true if logging type is disconnected logging.
     *          It means that the log collector service was connected, but now it is disconnected.
     *          The model is still connected to the log database and can read logs from it.
     **/
    inline bool isDisconnectedLogging(void) const;

    inline uint32_t getId(void) const;

/************************************************************************
 * Signals
 ************************************************************************/
signals:

    /**
     * \brief   Signal emitted when connected to the logging service.
     *          This signal is emitted during live mode.
     *          Ignore in case of offline or disconnected modes.
     **/
    void signalLogServiceConnected(void);

    /**
     * \brief   Signal emitted when disconnected from the logging service.
     *          This signal is emitted during live mode.
     *          Ignore in case of offline or disconnected modes.
     **/
    void signalLogServiceDisconnected(void);

    /**
     * \brief   Signal emitted when one or more instances are available.
     *          This can be either instances connected to the log collector service in live mode
     *          of instances read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instances   The list of instances available.
     **/
    void signalInstanceAvailable(const std::vector< NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   Signal emitted when one or more instances are disconnected.
     *          This can be instances disconnected from the log collector service in live mode.
     *          In case of offline or disconnected modes this signal is not triggered.
     * \param   instIds     The list of IDs of disconnected instances.
     **/
    void signalInstanceUnavailable(const std::vector<ITEM_ID>& instIds);

    /**
     * \brief   Signal emitted when scopes of the specified instance are available.
     *          This can be either scopes received in the live mode 
     *          or scopes read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instId      The ID of an instance, which scopes are available.
     * \param   scopes      The list of scopes available for the specified instance.
     **/
    void signalScopesAvailable(ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes);

    /**
     * \brief   Signal emitted when scopes of the specified instance are updated.
     *          This can be scopes received in the live mode.
     *          In case of offline or disconnected modes this signal is not triggered.
     * \param   instId      The ID of an instance, which scopes are updated.
     * \param   scopes      The list of scopes available for the specified instance.
     **/
    void signalScopesUpdated(ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes);

     /**
      * \brief   Signal emitted when one or more log messages are available.
      * \param   instId      The ID of the instance that sent the log messages.
      * \param   logs        The list of log messages available.
      **/
    void signalLogsAvailable(ITEM_ID instId, const std::vector<SharedBuffer>& logs);

//////////////////////////////////////////////////////////////////////////
// LoggingModelBase overrider
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Resets the model to refresh the view.
     **/
    virtual void refresh(void);
    
    /**
     * \brief   Setup logging model.
     **/    
    virtual void setupModel(void);
    
    /**
     * \brief   Release logging model.
     **/    
    virtual void releaseModel(void);

    /**
     * \brief   Opens logging database file.
     * \param   dbPath      The path to the database.
     * \param   readOnly    If true, the database is opened in read-only mode.
     **/
    virtual void openDatabase(const QString& dbPath, bool readOnly);

    /**
     * \brief   Returns the path to the log data database.
     **/
    virtual QString getDatabasePath(void) const;

    /**
     * \brief   Closes the currently opened database.
     **/
    virtual void closeDatabase(void);

    /**
     * \brief   Returns true if the model is operable, i.e. it can perform operations like querying log messages.
     **/
    virtual bool isOperable(void) const;

    /**
     * \brief   Call to query and get list of names of connected instances from log database.
     * \param   names   On output, contains the list of names of connected instances.
     **/
    virtual void getLogInstanceNames(std::vector<String>& names);

    /**
     * \brief   Call to query and get list of IDs of connected instances from log database
     * \param   ids     On output, contains the list of IDs of connected instances.
     **/
    virtual void getLogInstanceIds(std::vector<ITEM_ID>& ids);

    /**
     * \brief   Call to query and get list of names of threads of the connected instances from log database.
     * \param   names   On output, contains the list of all thread names that sent messages.
     **/
    virtual void getLogThreadNames(std::vector<String>& names);

    /**
     * \brief   Call to query and get list of IDs of threads of the connected instances from log database.
     * \param   ids     On output, contains the list of all thread IDs that sent messages.
     **/
    virtual void getLogThreads(std::vector<ITEM_ID>& ids);

    /**
     * \brief   Call to get the list of log priorities.
     * \param   names   On output, contains the names of all priorities.
     **/
    virtual void getPriorityNames(std::vector<String>& names);

    /**
     * \brief   Call to query and get information of connected instances from log database.
     *          This query will receive list of all registered instances.
     * \param   infos   On output, contains the list of information of all registered instances in database.
     **/
    virtual const std::vector<NEService::sServiceConnectedInstance> & getLogInstances(void);

    /**
     * \brief   Call to query and get information of log scopes of specified instance from log database.
     *          This query will receive list of all registered scopes.
     * \param   scopes  On output, contains the list of all registered scopes in database related with the specified instance ID.
     * \param   instID  The ID of the instance.
     **/
    virtual const std::vector<NELogging::sScopeInfo>& getLogInstScopes(ITEM_ID instId);

    /**
     * \brief   Call to get all log messages from log database.
     * \param   messages   On output, contains the list of all log messages.
     **/
    virtual const std::vector<SharedBuffer>& getLogMessages(void);

    /**
     * \brief   Call to get log messages of the specified instance from log database.
     *          If `instId` is `NEService::COOKIE_ANY` it receives the list of all instances
     *          similar to the call to `getLogMessages()`.
     * \param   messages    On output, contains the list of log messages of the specified instance.
     * \param   instId  The ID of the instance to get log messages.
     *                  If `NEService::COOKIE_ANY` it receives log messages of all instances.
     **/
    virtual void getLogInstMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId = NEService::COOKIE_ANY);

    /**
     * \brief   Call to get log messages of the specified scope from log database.
     *          If `scopeId` is `0` it receives the list of all scopes
     *          similar to the call to `getLogMessages()`.
     * \param   messages    On output, contains the list of log messages of the specified scope.
     * \param   scopeId     The ID of the scope to get log messages.
     *                      If `0` it receives log messages of all scopes.
     **/
    virtual void getLogScopeMessages(std::vector<SharedBuffer>& messages, uint32_t scopeId = 0);

    /**
     * \brief   Call to get log messages of the specified instance and log scope ID from log database.
     *          If `instId` is `NEService::COOKIE_ANY` and `scopeId` is `0`, it receives the list of all logs
     *          similar to the call to `getLogMessages()`.
     * \param   messages    On output, contains the list of log messages of the specified instance and scope.
     * \param   instId      The ID of the instance to get log messages.
     *                      If `NEService::COOKIE_ANY` it receives log messages of all instances.
     * \param   scopeId     The ID of the scope to get log messages.
     *                      If `0` it receives log messages of all scopes.
     **/
    virtual void getLogMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId);

    /**
     * \brief   Find the instance with the given ID and returns valid index if found.
     *          Otherwise returns NECommon::INVALID_INDEX (-1).
     * \param   instId  The ID of the instance to find.
     **/
    virtual int findInstanceEntry(ITEM_ID instId);

    /**
     * \brief   Returns the instance entry with the given ID.
     *          If not found, returns instance object with empty and invalid data.
     * \param   instId  The ID of the instance to get.
     **/
    virtual const NEService::sServiceConnectedInstance& getInstanceEntry(ITEM_ID instId);

    /**
     * \brief   Adds an instance entry to the model.
     *          If `unique` is true, it checks if the instance already exists and does not add it again.
     * \param   instance   The instance to add.
     * \param   unique     If true, adds the instance only if it is not already present.
     * \return  True if the instance was added successfully, false otherwise.
     **/
    virtual bool addInstanceEntry(const NEService::sServiceConnectedInstance& instance, bool unique);

    /**
     * \brief   Removes the instance entry with the given ID from the model.
     * \param   instId  The ID of the instance to remove.
     * \return  Returns NECommon::INVALID_INDEX if not found, otherwise returns index of removed instance.
     **/
    virtual int removeInstanceEntry(ITEM_ID instId);

    /**
     * \brief   Adds list of instances to the list. Triggered, when instances are connected.
     * \param   instances   The list of connected instances.
     * \param   unique      Flag, indicating whether each instance should be unique in the list.
     * \return  Returns number of instances added to the list.
     **/
    virtual int addInstances(const std::vector<NEService::sServiceConnectedInstance>& instances, bool unique);

    /**
     * \brief   Removes list of instances from the list. Triggered, when instances are disconnected.
     * \param   instances   The list of disconnected instances.
     * \return  Returns number of instances removed from the list.
     **/
    virtual int removeInstances(const std::vector<NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   Transfers the data from given model. Copies the list of connected instances, scopes and logs.
     * \param   logModel    The source of data to copy. On output the list of existing data may be empty.
     **/
    virtual void dataTransfer(LoggingModelBase& logModel);

    /**
     * \brief   Reads logs from the database asynchronously in a separate thread.
     * @param   maxEntries  The maximum number of log entries to read in one loop.
     *          If -1, reads all available entries.
     **/
    virtual void readLogsAsynchronous(int maxEntries = -1);

//////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Closes currently opened log database file without triggering signal.
     **/
    inline void _closeDatabase(void);

    /**
     * \brief   Quits the worker thread, which reads log messages from database.
     **/
    inline void _quitThread(void);

    /**
     * \brief   Helper to get display data for a log message and column.
     **/
    QVariant getDisplayData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get background color data for a log message and column.
     **/
    QVariant getBackgroundData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get foreground color data for a log message and column.
     **/
    QVariant getForegroundData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get decoration (icon) data for a log message and column.
     **/
    QVariant getDecorationData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get text alignment data for a column.
     **/
    QVariant getAlignmentData(eColumn column) const;

/************************************************************************/
// IEThreadConsumer interface overrides
/************************************************************************/
protected:

    /**
     * \brief   This callback function is called from Thread object, when it is 
     *          running and fully operable. If thread needs run in loop, the loop 
     *          should be implemented here. When consumer exits this function, 
     *          the thread will complete work. To restart thread running, 
     *          createThread() method should be called again.
     **/
    virtual void onThreadRuns( void ) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    inline LoggingModelBase& self(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    eLogging                mLoggingType;   //!< The type of logging, either live or offline.
    LogSqliteDatabase       mDatabase;      //!< The SQLite database object to read log data.
    SqliteStatement         mStatement;     //!< The SQLite statement to query log data.
    ListColumns             mActiveColumns; //!< The list of active columns.
    ListLogs                mLogs;          //!< The list of log messages.
    ListInstances           mInstances;     //!< The list of connected instances.
    MapScopes               mScopes;        //!< The map of scopes, where key is instance ID and value is the list of scopes.
    int                     mLogChunk;      //!< The position in the log messages list to read from.
    uint32_t                mLogCount;      //!< The position of updated log.
    Thread                  mReadThread;    //!< The thread to run the model operations.
    Mutex                   mQuitThread;    //!< The event to notify when data is ready.

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    const uint32_t          mModelId;       //!< The unique ID of logging model
};

//////////////////////////////////////////////////////////////////////////
// LoggingModelBase class inline methods.
//////////////////////////////////////////////////////////////////////////

inline int LoggingModelBase::findColumn(LoggingModelBase::eColumn col) const
{
    return static_cast<int>(mActiveColumns.indexOf(col));
}

inline const QList<LoggingModelBase::eColumn>& LoggingModelBase::getActiveColumns() const
{
    return mActiveColumns;
}

inline int LoggingModelBase::getMaxColumCount(void) const
{
    return static_cast<int>(eColumn::LogColumnCount);
}

inline bool LoggingModelBase::isEmpty(void) const
{
    return mLogs.empty();
}

inline void LoggingModelBase::dataReset(void)
{
    beginResetModel();
    mLogs.clear();
    endResetModel();
}

inline int LoggingModelBase::fromColumnToIndex(LoggingModelBase::eColumn col) const
{
    return mActiveColumns.indexOf(col);
}

inline LoggingModelBase::eColumn LoggingModelBase::fromIndexToColumn(int logicalIndex) const
{
    return ((logicalIndex >= 0) && (logicalIndex < static_cast<int>(mActiveColumns.size())) ? mActiveColumns[logicalIndex] : LoggingModelBase::eColumn::LogColumnInvalid);
}

inline QString LoggingModelBase::getLogFileName(void) const
{
    String dbPath = mDatabase.getDatabasePath();
    return QString(dbPath.isEmpty() == false ? File::getFileNameWithExtension(dbPath).getString() : "");
}

inline QString LoggingModelBase::getLogFilePath(void) const
{
    return QString::fromStdString(mDatabase.getDatabasePath().getData());
}

inline void LoggingModelBase::markDisconnected()
{
    mLoggingType = LoggingModelBase::eLogging::LoggingDisconneced;
}

inline LoggingModelBase::eLogging LoggingModelBase::getLoggingType(void) const
{
    return mLoggingType;
}

inline bool LoggingModelBase::isLiveLogging(void) const
{
    return (mLoggingType == eLogging::LoggingLive);
}

inline bool LoggingModelBase::isOfflineLogging(void) const
{
    return (mLoggingType == eLogging::LoggingOffline);
}

inline bool LoggingModelBase::isDisconnectedLogging(void) const
{
    return (mLoggingType == eLogging::LoggingDisconneced);
}

inline uint32_t LoggingModelBase::getId(void) const
{
    return mModelId;
}

inline void LoggingModelBase::_closeDatabase(void)
{
    mDatabase.disconnect();
    dataReset();
}

inline void LoggingModelBase::_quitThread(void)
{
    if (mReadThread.isValid())
    {
        mQuitThread.lock(NECommon::WAIT_INFINITE);
        mReadThread.shutdownThread(NECommon::WAIT_INFINITE);
        mQuitThread.unlock();
    }
}

inline LoggingModelBase& LoggingModelBase::self(void)
{
    return (*this);
}

#endif // LUSAN_MODEL_LOG_LOGGINGMODELBASE_HPP
