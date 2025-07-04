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
 *  \file        lusan/model/log/LogViewerModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogViewerModel.hpp"
#include "lusan/data/log/LogObserver.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"

#include "areg/base/DateTime.hpp"
#include "areg/base/File.hpp"


QString LogViewerModel::generateFileName(void)
{
    QString result{ "log_%time%.sqlog" };

    LogObserver::Component* logObserver = LogObserver::getComponent();
    if (logObserver != nullptr)
    {
        QString fileName = LogObserver::getConfigDatabaseName();
        if (fileName.isEmpty() == false)
        {
            result = fileName;
        }
    }

    return QString(File::normalizePath(result.toStdString().c_str()).getString());
}

QString LogViewerModel::newFileName(void)
{
    QString result;
    
    WorkspaceEntry workspace = LusanApplication::getActiveWorkspace();
    QString dir = workspace.getDirLogs();
    if (dir.isEmpty())
    {
        dir = LogObserver::getConfigDatabaseLocation();
    }
    
    if (dir.isEmpty() == false)
    {
        dir = File::normalizePath(dir.toStdString().c_str()).getString();
        QString fileName = generateFileName();
        
        std::filesystem::path fPath(dir.toStdString().c_str());
        fPath /= fileName.toStdString().c_str();
        result = QString(fPath.c_str());
    }
    
    return result;
}

LogViewerModel::LogViewerModel(QObject *parent)
    : LoggingModelBase(parent)

    , mIsConnected(false)
    , mAddress()
    , mPort(NESocket::InvalidPort)
    , mConLogger            ( )
    , mConLogs              ( )
    , mConInstancesConnect  ( )
    , mConInstancesDisconnect( )
    , mConServiceDisconnected( )
    , mConRegisterScopes    ( )
    , mConUpdateScopes      ( )
{
}

bool LogViewerModel::connectService(const QString& hostName /*= ""*/, unsigned short portNr /*= 0u*/)
{
    return false;
}

void LogViewerModel::disconnectService(void)
{
}

void LogViewerModel::serviceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    mIsConnected = isConnected;
    mAddress     = address;
    mPort        = port;
    
    openDatabase(dbPath, false);

    disconnect(mConLogger);
    disconnect(mConLogs);
    disconnect(mConInstancesConnect);
    disconnect(mConInstancesDisconnect);
    disconnect(mConServiceDisconnected);
    disconnect(mConRegisterScopes);
    disconnect(mConUpdateScopes);
    
    if (isConnected)
    {
        LogObserver* log = LogObserver::getComponent();
        Q_ASSERT(log != nullptr);
        mConLogger             = connect(log, &LogObserver::signalLogMessage         , this, &LogViewerModel::slotLogMessage);
        mConLogs               = connect(log, &LogObserver::signalLogServiceConnected, this, &LogViewerModel::slotLogServiceConnected);
        mConInstancesConnect   = connect(log, &LogObserver::signalLogInstancesConnect, this, &LogViewerModel::slotLogInstancesConnect);
        mConInstancesDisconnect= connect(log, &LogObserver::signalLogInstancesDisconnect, this, &LogViewerModel::slotLogInstancesDisconnect);
        mConServiceDisconnected= connect(log, &LogObserver::signalLogServiceDisconnected, this, &LogViewerModel::slotLogServiceDisconnected);
        mConRegisterScopes     = connect(log, &LogObserver::signalLogRegisterScopes, this, &LogViewerModel::slotLogRegisterScopes);
        mConUpdateScopes       = connect(log, &LogObserver::signalLogUpdateScopes, this, &LogViewerModel::slotLogUpdateScopes);
    }
}

void LogViewerModel::pauseLogging(void)
{
    LogObserver::pause();
}

void LogViewerModel::resumeLogging(void)
{
    LogObserver::resume();
}

void LogViewerModel::stopLogging(void)
{
    LogObserver::stop();
}

void LogViewerModel::restartLogging(const QString& dbName /*= QString()*/)
{
    beginResetModel();
    mLogs.clear();
    LogObserver::restart(dbName);
    endResetModel();
}

void LogViewerModel::slotLogServiceConnected(bool isConnected, const QString& address, uint16_t port)
{
    if (isConnected == false)
    {
        mIsConnected = false;
        disconnect(mConLogger);
        disconnect(mConLogs);
        disconnect(mConInstancesConnect);
        disconnect(mConInstancesDisconnect);
        disconnect(mConServiceDisconnected);
        disconnect(mConRegisterScopes);
        disconnect(mConUpdateScopes);
    }
}

void LogViewerModel::slotLogMessage(const SharedBuffer& logMessage)
{
    if (logMessage.isEmpty() == false)
    {
        beginInsertRows(QModelIndex(), static_cast<int>(mLogs.size()), static_cast<int>(mLogs.size()));
        mLogs.push_back(logMessage);
        endInsertRows();
    }
}

void LogViewerModel::slotLogInstancesConnect(const QList<NEService::sServiceConnectedInstance>& instances)
{
    // Forward the signal to any listening objects (like LogScopesModel)
    emit signalLogInstancesConnect(instances);
}

void LogViewerModel::slotLogInstancesDisconnect(const QList<NEService::sServiceConnectedInstance>& instances)
{
    // Forward the signal to any listening objects (like LogScopesModel)
    emit signalLogInstancesDisconnect(instances);
}

void LogViewerModel::slotLogServiceDisconnected(const QMap<ITEM_ID, NEService::sServiceConnectedInstance>& instances)
{
    // Forward the signal to any listening objects (like LogScopesModel)
    emit signalLogServiceDisconnected(instances);
}

void LogViewerModel::slotLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
    // Forward the signal to any listening objects (like LogScopesModel)
    emit signalLogRegisterScopes(cookie, scopes);
}

void LogViewerModel::slotLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
    // Forward the signal to any listening objects (like LogScopesModel)
    emit signalLogUpdateScopes(cookie, scopes);
}
