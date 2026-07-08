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
 *  \file        lusan/model/log/LiveLogsModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Model.
 *
 ************************************************************************/

#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/data/log/LogObserver.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "areg/base/File.hpp"


QString LiveLogsModel::generateFileName()
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

    return QString::fromStdString(areg::File::normalize_path(result.toStdString()).as_string());
}

QString LiveLogsModel::newFileName()
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
        dir = QString::fromStdString(areg::File::normalize_path(dir.toStdString()).as_string());
        QString fileName = generateFileName();
        
        std::filesystem::path fPath(dir.toStdString().c_str());
        fPath /= fileName.toStdString().c_str();
        result = QString(fPath.c_str());
    }
    
    return result;
}

LiveLogsModel::LiveLogsModel(QObject *parent)
    : LoggingModelBase(LoggingModelBase::eLogging::LoggingLive, parent)

    , mIsConnected              (false)
    , mAddress                  ( )
    , mPort                     (areg::InvalidPort)
    , mSignalsSetup             (false)
    , mConLogger                ( )
    , mConLogs                  ( )
    , mConInstancesConnect      ( )
    , mConInstancesDisconnect   ( )
    , mConServiceDisconnected   ( )
    , mConRegisterScopes        ( )
    , mConUpdateScopes          ( )
{
}

LiveLogsModel::~LiveLogsModel()
{
    _setupSignals(false);
}

bool LiveLogsModel::connectService(const QString& hostName /*= ""*/, unsigned short portNr /*= 0u*/)
{
    return false;
}

void LiveLogsModel::disconnectService()
{
}

void LiveLogsModel::setupModel()
{
    _setupSignals(true);
}

void LiveLogsModel::releaseModel()
{
    _setupSignals(false);
}

void LiveLogsModel::serviceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    mIsConnected = isConnected;
    mAddress     = address;
    mPort        = port;
    
    if (isConnected == false)
    {
        _setupSignals(false);        
    }
    
    openDatabase(dbPath, true);
}

void LiveLogsModel::pauseLogging()
{
    LogObserver::pause();
}

void LiveLogsModel::resumeLogging()
{
    LogObserver::resume();
}

void LiveLogsModel::stopLogging()
{
    LogObserver::stop();
}

void LiveLogsModel::restartLogging(const QString& dbName /*= QString()*/)
{
    beginResetModel();
    cleanLogs();
    LogObserver::restart(dbName);
    endResetModel();
}

void LiveLogsModel::_setupSignals(bool doSetup)
{
    if (doSetup)
    {
        if (mSignalsSetup)
            return;
        
        mSignalsSetup = true;
        LogObserver* log = LogObserver::getComponent();
        Q_ASSERT(log != nullptr);
        
        mConLogs                = connect(log, &LogObserver::signalLogMessage            , this, &LiveLogsModel::slotLogMessage);
        mConInstancesDisconnect = connect(log, &LogObserver::signalLogInstancesDisconnect, this, &LiveLogsModel::slotLogInstancesDisconnect);
        
        mConLogger              = connect(log, &LogObserver::signalLogServiceConnected   , this, [this]() {
        });
        mConInstancesConnect    = connect(log, &LogObserver::signalLogInstancesConnect   , this, [this](const std::vector<areg::ConnectedInstance>& instances) {
            addInstances(instances, true);
            emit signalInstanceAvailable(instances);
        });
        mConServiceDisconnected = connect(log, &LogObserver::signalLogServiceDisconnected, this, [this] {
            emit signalLogServiceDisconnected();
        });
        mConRegisterScopes      = connect(log, &LogObserver::signalLogRegisterScopes     , this, [this](ITEM_ID cookie, const std::vector<areg::ScopeEntry>& scopes) {
            mScopes[cookie] = scopes;
            emit signalScopesAvailable(cookie, scopes);
        });
        mConUpdateScopes        = connect(log, &LogObserver::signalLogUpdateScopes       , this, [this](ITEM_ID cookie, const std::vector<areg::ScopeEntry>& scopes) {
            mScopes[cookie] = scopes;
            emit signalScopesUpdated(cookie, scopes);
        });
    }
    else if (mSignalsSetup)
    {
        disconnect(mConLogger);
        disconnect(mConLogs);
        disconnect(mConInstancesConnect);
        disconnect(mConInstancesDisconnect);
        disconnect(mConServiceDisconnected);
        disconnect(mConRegisterScopes);
        disconnect(mConUpdateScopes);
        mSignalsSetup = false;
    }
}
    
void LiveLogsModel::slotLogMessage(const areg::SharedBuffer& logMessage)
{
    if (logMessage.is_empty() == false)
    {
        int count {static_cast<int>(mLogs.size())};
        beginInsertRows(QModelIndex(), count, count);
        mLogs.push_back(logMessage);
        ++ mLogCount;
        endInsertRows();
    }
}

void LiveLogsModel::slotLogInstancesDisconnect(const std::vector<areg::ConnectedInstance>& instances)
{
    removeInstances(instances);
    std::vector<ITEM_ID> ids(instances.size());
    for (int i = 0; i < static_cast<int>(instances.size()); ++i)
    {
        ids[i] = instances[i].ciCookie;
    }

    emit signalInstanceUnavailable(ids);
}
