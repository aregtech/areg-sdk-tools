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
#include "lusan/model/log/LoggingModelBase.hpp"

/**
 * \brief   The offline log navigation model for reading log data from local database files.
 *          This model provides offline access to historical log data stored in database files
 *          using the LogSqliteDatabase class from the AREG Framework.
 **/
class LogOfflineModel : public LoggingModelBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogOfflineModel(QObject* parent = nullptr);
    virtual ~LogOfflineModel();


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

    /**
     * \brief   Signal, triggered when read the logging data from logging file.
     **/
    void signalLogsAvailable(void);
};

#endif // LUSAN_MODEL_LOG_LOGOFFLINEMODEL_HPP
