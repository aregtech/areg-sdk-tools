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
 *  \file        lusan/model/log/LogOfflineModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Navigation Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogOfflineModel.hpp"
#include "lusan/model/log/LogScopeIconFactory.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/SharedBuffer.hpp"
#include "areg/base/File.hpp"
#include "areg/logging/NELogging.hpp"

#include <QFileInfo>

LogOfflineModel::LogOfflineModel(QObject *parent)
    : LoggingModelBase  (parent)
{
}

LogOfflineModel::~LogOfflineModel()
{
    _closeDatabase();
}

void LogOfflineModel::openDatabase(const QString& filePath, bool readOnly)
{
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
        beginResetModel();
        mLogs.clear();
        getLogMessages(mLogs);        
        endResetModel();
        emit signalLogsAvailable();
    }
}

void LogOfflineModel::closeDatabase(void)
{
    _closeDatabase();
    emit signalDatabaseIsClosed(QString::fromStdString(mDatabase.getDatabasePath().getData()));
}
