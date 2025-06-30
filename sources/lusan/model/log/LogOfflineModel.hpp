#ifndef LUSAN_MODEL_LOG_LOGOFFLINEMODEL_HPP
#define LUSAN_MODEL_LOG_LOGOFFLINEMODEL_HPP
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
 *  \file        lusan/model/log/LogOfflineModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Navigation Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QAbstractTableModel>
#include <QList>
#include <QString>

#include "areg/base/SharedBuffer.hpp"
#include "areg/base/File.hpp"
#include "areg/component/NEService.hpp"
#include "areg/logging/NELogging.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogViewerFilterProxy;

// Forward declaration from AREG framework  
class LogSqliteDatabase;

/**
 * \brief   The offline log navigation model for reading log data from local database files.
 *          This model provides offline access to historical log data stored in database files
 *          using the LogSqliteDatabase class from the AREG Framework.
 **/
class LogOfflineModel : public QAbstractTableModel
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:

    //!< The index of columns (reusing LogViewerModel columns for compatibility)
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

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogOfflineModel(QObject *parent = nullptr);
    virtual ~LogOfflineModel();

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
    static const QList<LogOfflineModel::eColumn>& getDefaultColumns(void);

    /**
     * \brief   Returns the header name of the specified column.
     * \param   colIndex The zero-based index of the column.
     **/
    QString getHeaderName(int colIndex) const;

    /**
     * \brief   Opens a log database file for offline analysis.
     * \param   filePath    The path to the log database file to open.
     * \return  True if the database was opened successfully, false otherwise.
     **/
    bool openDatabase(const QString& filePath);

    /**
     * \brief   Closes the currently opened database.
     **/
    void closeDatabase(void);

    /**
     * \brief   Returns true if a database is currently open.
     **/
    bool isDatabaseOpen(void) const;

    /**
     * \brief   Returns the full path of the currently loaded log database file.
     *          This serves as the model's name.
     **/
    const QString& getModelName(void) const;

    /**
     * \brief   Return the file name of the log database to set as a title.
     **/
    QString getLogFileName(void) const;

    /**
     * \brief   Resets the data in the model.
     *          Clears the list of log messages and resets the model.
     **/
    void dataReset(void);

    /**
     * \brief   Returns true if the model is empty, i.e. contains no log messages.
     **/
    bool isEmpty(void) const;

    /**
     * \brief   Finds the index of the specified column.
     * \param   col The column to find.
     * \return  The index of the column, or -1 if not found.
     **/
    int findColumn(LogOfflineModel::eColumn col) const;

    /**
     * \brief   Returns the list of active columns.
     *          The active columns are the ones that are currently visible in the log viewer.
     **/
    QList<LogOfflineModel::eColumn> getActiveColumns(void) const;

    /**
     * \brief   Adds a column at a given position of active columns list.
     *          If -1, adds a column before the "Log messages" column.
     * \param   col     The column to add.
     * \param   pos     The position of column to add.
     *                  If -1, adds column before "Log messages" column.
     **/
    void addColumn(LogOfflineModel::eColumn col, int pos = -1);

    /**
     * \brief   Removes specified column from the active columns list.
     * \param   col     The column to remove.
     **/
    void removeColumn(LogOfflineModel::eColumn col);

    /**
     * \brief   Sets list of active columns. If empty, it resets the default columns
     * \param   columns     The list of active columns to set.
     *                      If empty, resets the columns.
     **/
    void setActiveColumns(const QList< LogOfflineModel::eColumn>& columns);

    /**
     * \brief   Call to query and get list of names of connected instances from log database.
     * \param   names   On output, contains the list of names of connected instances.
     **/
    void getLogInstanceNames(std::vector<String>& names);
    
    /**
     * \brief   Call to query and get list of IDs of connected instances from log database
     * \param   ids     On output, contains the list of IDs of connected instances.
     **/
    void getLogInstances(std::vector<ITEM_ID>& ids);
    
    /**
     * \brief   Call to query and get list of names of threads of the connected instances from log database.
     * \param   names   On output, contains the list of all thread names that sent messages.
     **/
    void getLogThreadNames(std::vector<String>& names);
    
    /**
     * \brief   Call to query and get list of IDs of threads of the connected instances from log database.
     * \param   ids     On output, contains the list of all thread IDs that sent messages.
     **/
    void getLogThreads(std::vector<ITEM_ID>& ids);
    
    /**
     * \brief   Call to get the list of log priorities.
     * \param   names   On output, contains the names of all priorities.
     **/
    void getPriorityNames(std::vector<String>& names);
    
    /**
     * \brief   Call to query and get information of connected instances from log database.
     *          This query will receive list of all registered instances.
     * \param   infos   On output, contains the list of information of all registered instances in database.
     **/
    void getLogInstanceInfos(std::vector< NEService::sServiceConnectedInstance>& infos);
    
    /**
     * \brief   Call to query and get information of log scopes of specified instance from log database.
     *          This query will receive list of all registered scopes.
     * \param   scopes  On output, contains the list of all registered scopes in database related with the specified instance ID.
     * \param   instID  The ID of the instance.
     **/
    void getLogInstScopes(std::vector<NELogging::sScopeInfo>& scopes, ITEM_ID instId);
    
    /**
     * \brief   Call to get all log messages from log database.
     * \param   messages   On output, contains the list of all log messages.
     **/
    void getLogMessages(std::vector<SharedBuffer>& messages);
    
    /**
     * \brief   Call to get log messages of the specified instance from log database.
     *          If `instId` is `NEService::COOKIE_ANY` it receives the list of all instances
     *          similar to the call to `getLogMessages()`.
     * \param   messages    On output, contains the list of log messages of the specified instance.
     * \param   instId  The ID of the instance to get log messages.
     *                  If `NEService::COOKIE_ANY` it receives log messages of all instances.
     **/
    void getLogInstMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId = NEService::COOKIE_ANY);
    
    /**
     * \brief   Call to get log messages of the specified scope from log database.
     *          If `scopeId` is `0` it receives the list of all scopes
     *          similar to the call to `getLogMessages()`.
     * \param   messages    On output, contains the list of log messages of the specified scope.
     * \param   scopeId     The ID of the scope to get log messages.
     *                      If `0` it receives log messages of all scopes.
     **/
    void getLogScopeMessages(std::vector<SharedBuffer>& messages, uint32_t scopeId = 0);
    
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
    void getLogMessages(std::vector<SharedBuffer>& messages, ITEM_ID instId, uint32_t scopeId);

    /**
     * \brief   Returns maximum number of columns that is possible to set in the log viewer.
     **/
    int getMaxColumCount(void) const;

    /**
     * \brief   Converts the column to its index in the active columns list.
     **/
    int fromColumnToIndex(LogOfflineModel::eColumn col) const;

    /**
     * \brief   Converts the index to its column in the active columns list.
     *          Returns LogColumnInvalid if index is invalid.
     * \param   logicalIndex  The logical index of the column.
     **/
    LogOfflineModel::eColumn fromIndexToColumn(int logicalIndex) const;

    /**
     * \brief   Returns the filter proxy model.
     **/
    LogViewerFilterProxy* getFilter(void);
    
//////////////////////////////////////////////////////////////////////////
// Private helper methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Helper to get display data for a log message and column.
     **/
    QVariant _getDisplayData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get background color data for a log message and column.
     **/
    QVariant _getBackgroundData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get foreground color data for a log message and column.
     **/
    QVariant _getForegroundData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get decoration (icon) data for a log message and column.
     **/
    QVariant _getDecorationData(const NELogging::sLogMessage* logMessage, eColumn column) const;

    /**
     * \brief   Helper to get text alignment data for a column.
     **/
    QVariant _getAlignmentData(eColumn column) const;
    
//////////////////////////////////////////////////////////////////////////
// Member variable
//////////////////////////////////////////////////////////////////////////
private:
    QString             mDbPath;        //!< The path to the current database file
    bool                mDbOpen;        //!< Flag indicating if database is open
    LogSqliteDatabase*  mDatabase;      //!< The sqlite database object for offline access
    QList<eColumn>      mActiveColumns; //!< The list of active columns
    QList<SharedBuffer> mLogs;          //!< The list of log messages loaded from database
    LogViewerFilterProxy* mFilter;      //!< The log viewer filter
};

//////////////////////////////////////////////////////////////////////////
// LogOfflineModel class inline methods.
//////////////////////////////////////////////////////////////////////////

inline bool LogOfflineModel::isDatabaseOpen(void) const
{
    return mDbOpen;
}

inline const QString& LogOfflineModel::getModelName(void) const
{
    return mDbPath;
}

inline QString LogOfflineModel::getLogFileName(void) const
{
    return QString(mDbPath.isEmpty() == false ? File::getFileNameWithExtension(mDbPath.toStdString().c_str()).getString() : "");
}

inline void LogOfflineModel::dataReset(void)
{
    beginResetModel();
    mLogs.clear();
    endResetModel();
}

inline bool LogOfflineModel::isEmpty(void) const
{
    return mLogs.isEmpty();
}

inline int LogOfflineModel::findColumn(LogOfflineModel::eColumn col) const
{
    return static_cast<int>(mActiveColumns.indexOf(col));
}

inline QList<LogOfflineModel::eColumn> LogOfflineModel::getActiveColumns(void) const
{
    return mActiveColumns;
}

inline int LogOfflineModel::getMaxColumCount(void) const
{
    return static_cast<int>(eColumn::LogColumnCount);
}

inline int LogOfflineModel::fromColumnToIndex(LogOfflineModel::eColumn col) const
{
    return mActiveColumns.indexOf(col);
}

inline LogOfflineModel::eColumn LogOfflineModel::fromIndexToColumn(int logicalIndex) const
{
    return ((logicalIndex >= 0) && (logicalIndex < static_cast<int>(mActiveColumns.size())) ? mActiveColumns[logicalIndex] : LogOfflineModel::eColumn::LogColumnInvalid);
}

inline LogViewerFilterProxy* LogOfflineModel::getFilter(void)
{
    return mFilter;
}

#endif // LUSAN_MODEL_LOG_LOGOFFLINEMODEL_HPP