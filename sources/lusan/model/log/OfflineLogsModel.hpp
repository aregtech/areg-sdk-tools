#ifndef LUSAN_MODEL_LOG_OFFLINELOGSMODEL_HPP
#define LUSAN_MODEL_LOG_OFFLINELOGSMODEL_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/log/OfflineLogsModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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
 *          using the LogSqliteDatabase class from the Areg Framework.
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
    void openDatabase(const QString& dbPath, bool readOnly) override;

    /**
     * \brief   Closes the currently opened database.
     **/
    void closeDatabase() override;

    /**
     * \brief   Sets up the logging query to run. By default, it reads all logs without filter.
     *          Override if need to change.
     * \param   instId  The ID of the instance to read logs. Reads logs of all instances it `areg::TARGET_ALL`.
     * \return  Number or log entries to read.
     **/
    uint32_t setupLogStatement(ITEM_ID instId = areg::TARGET_ALL) override;

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
