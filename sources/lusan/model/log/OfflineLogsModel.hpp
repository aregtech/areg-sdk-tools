#ifndef LUSAN_MODEL_LOG_OFFLINELOGSMODEL_HPP
#define LUSAN_MODEL_LOG_OFFLINELOGSMODEL_HPP
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
 *  \file        lusan/model/log/OfflineLogsModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Navigation Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LoggingModelBase.hpp"

/**
 * \brief   The offline log navigation model for reading log data from local database files.
 *          This model provides offline access to historical log data stored in database files
 *          using the LogSqliteDatabase class from the AREG Framework.
 **/
class OfflineLogsModel : public LoggingModelBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:
    static  constexpr   int DEFAULT_LOG_CHUNK   { 1000 };   // The default size of the log chunk to read in one loop from database.

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit OfflineLogsModel(QObject* parent = nullptr);
    virtual ~OfflineLogsModel();

//////////////////////////////////////////////////////////////////////////
// LoggingModelBase overrider
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Opens logging database file.
     * \param   dbPath      The path to the database.
     * \param   readOnly    If true, the database is opened in read-only mode.
     **/
    virtual void openDatabase(const QString& dbPath, bool readOnly) override;

    /**
     * \brief   Closes the currently opened database.
     **/
    virtual void closeDatabase(void) override;

    /**
     * \brief   Sets up the logging query to run. By default, it reads all logs without filter.
     *          Override if need to change.
     * \param   instId  The ID of the instance to read logs. Reads logs of all instances it `NEService::TARGET_ALL`.
     * \return  Number or log entries to read.
     **/
    virtual uint32_t setupLogStatement(ITEM_ID instId = NEService::TARGET_ALL) override;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Signal, triggered when succeeded to open log database.
     * \param   dbPath  The path to the log database file.
     **/
    void signalDatabaseIsOpened(const QString& dbPath);

    /**
     * \brief   Signal, triggered when log database file is closed.
     * \param   dbPath  The path to the log database file.
     **/
    void signalDatabaseIsClosed(const QString& dbPath);

};

#endif // LUSAN_MODEL_LOG_OFFLINELOGSMODEL_HPP
