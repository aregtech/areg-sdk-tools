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
 *  \file        lusan/model/log/OfflineLogsModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Navigation Model.
 *
 ************************************************************************/

#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/File.hpp"
#include "areg/logging/areg_log.h"

#include <QFileInfo>

OfflineLogsModel::OfflineLogsModel(QObject *parent)
    : LoggingModelBase(LoggingModelBase::eLogging::LoggingOffline, parent)
{
}

OfflineLogsModel::~OfflineLogsModel()
{
    _closeDatabase();
}

void OfflineLogsModel::openDatabase(const QString& filePath, bool readOnly)
{
    if (mDatabase.is_operable() && (mLogs.empty() == false) && (mDatabase.database_path() == filePath))
        return;

    _closeDatabase(); // Close any existing database    
    if (filePath.isEmpty())
        return;
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return;
    
    if (mDatabase.connect(filePath.toStdString(), readOnly))
    {
        emit signalDatabaseIsOpened(QString::fromStdString(mDatabase.database_path().data()));
        // Load initial log messages for display
        
        mInstances.clear();
        mDatabase.log_instance_infos(mInstances);
        emit signalInstanceAvailable(mInstances);
        
        mScopes.clear();
        for (const auto & inst : mInstances)
        {
            std::vector<areg::ScopeEntry> scopes;
            mDatabase.log_inst_scopes(scopes, inst.ciCookie);
            mScopes[inst.ciCookie] = scopes;
            emit signalScopesAvailable(inst.ciCookie, scopes);
        }
        
        mDatabase.setup_filter_logs(areg::TARGET_ALL, areg::ArrayList<areg::ext::LogSqliteDatabase::ScopeFilter>{});
        readLogsAsynchronous(OfflineLogsModel::DEFAULT_LOG_CHUNK);
    }
}

uint32_t OfflineLogsModel::setupLogStatement(ITEM_ID instId /*= areg::TARGET_ALL*/, int32_t limit /*= -1*/, uint32_t offset /*= 0u*/)
{
    return mDatabase.setup_statement_read_filter_logs(mStatement, instId);
}

void OfflineLogsModel::closeDatabase()
{
    _closeDatabase();
    emit signalDatabaseIsClosed(QString::fromStdString(mDatabase.database_path().data()));
}

