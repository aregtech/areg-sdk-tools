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
 *  \file        lusan/model/log/OfflineLogsModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Navigation Model.
 *
 ************************************************************************/

#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/File.hpp"
#include "areg/logging/NELogging.hpp"

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
    if (mDatabase.isOperable() && (mLogs.empty() == false) && (mDatabase.getDatabasePath() == filePath))
        return;

    _closeDatabase(); // Close any existing database    
    if (filePath.isEmpty())
        return;
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return;
    
    if (mDatabase.connect(filePath.toStdString(), readOnly))
    {
        emit signalDatabaseIsOpened(QString::fromStdString(mDatabase.getDatabasePath().getData()));
        // Load initial log messages for display
        
        mInstances.clear();
        mDatabase.getLogInstanceInfos(mInstances);
        emit signalInstanceAvailable(mInstances);
        
        mScopes.clear();
        for (const auto & inst : mInstances)
        {
            std::vector<NELogging::sScopeInfo> scopes;
            mDatabase.getLogInstScopes(scopes, inst.ciCookie);
            mScopes[inst.ciCookie] = scopes;
            emit signalScopesAvailable(inst.ciCookie, scopes);
        }
        
        readLogsAsynchronous(OfflineLogsModel::DEFAULT_LOG_CHUNK);
    }
}

void OfflineLogsModel::closeDatabase(void)
{
    _closeDatabase();
    emit signalDatabaseIsClosed(QString::fromStdString(mDatabase.getDatabasePath().getData()));
}
