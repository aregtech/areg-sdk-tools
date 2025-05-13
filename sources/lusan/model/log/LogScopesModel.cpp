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
 *  \file        lusan/model/log/LogScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/model/log/LogScopesModel.hpp"
#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/data/log/LogObserver.hpp"

LogScopesModel::LogScopesModel(QObject parent)
    : QAbstractItemModel( parent )
    , mRootList         ( )
    , mRootIndex        ( )
{
}

LogScopesModel::~LogScopesModel(void)
{
    _clear();
}

bool LogScopesModel::initialize(void)
{
    _clear();
    LogObserver* log = LogObserver::getComponent();
    if ((log == nullptr) || (log->isConnected() == false))
        return false;

    connect(log, &LogObserver::signalLogInstancesConnect    , this, &LogScopesModel::slotLogInstancesConnect);
    connect(log, &LogObserver::signalLogInstancesDisconnect , this, &LogScopesModel::slotLogInstancesDisconnect);
    connect(log, &LogObserver::signalLogServiceDisconnected , this, &LogScopesModel::slotLogServiceDisconnected);
    connect(log, &LogObserver::signalLogRegisterScopes      , this, &LogScopesModel::slotLogRegisterScopes);
    connect(log, &LogObserver::signalLogUpdateScopes        , this, &LogScopesModel::slotLogUpdateScopes);
    
    mRootIndex = index(0, 0);

    return LogObserver::requestInstances();
}

void LogScopesModel::release(void)
{
    LogObserver* log = LogObserver::getComponent();
    if (log != nullptr)
    {
        disconnect(log, &LogObserver::signalLogInstancesConnect     , this, &LogScopesModel::slotLogInstancesConnect);
        disconnect(log, &LogObserver::signalLogInstancesDisconnect  , this, &LogScopesModel::slotLogInstancesDisconnect);
        disconnect(log, &LogObserver::signalLogServiceDisconnected  , this, &LogScopesModel::slotLogServiceDisconnected);
        disconnect(log, &LogObserver::signalLogRegisterScopes       , this, &LogScopesModel::slotLogRegisterScopes);
        disconnect(log, &LogObserver::signalLogUpdateScopes         , this, &LogScopesModel::slotLogUpdateScopes);
    }
}

inline void LogScopesModel::_clear(void)
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        delete root;
    }

    mRootList.clear();
}

inline bool LogScopesModel::_exists(ITEM_ID rootId) const
{
    for (auto root : mRootList)
    {
        Q_ASSERT(root != nullptr);
        if (root->getRootId() == rootId)
            return true;
    }

    return false;
}

inline bool LogScopesModel::_appendRoot(ScopeRoot* root, bool unique /*= true*/)
{
    if ((unique == false) || (_exists(root->getRootId()) == false))
    {
        mRootList.append(root);
        return true;
    }

    return false;
}

inline int LogScopesModel::_findRoot(ITEM_ID rootId) const
{
    for (int i = 0; i < mRootList.size(); ++i)
    {
        if (mRootList[i]->getRootId() == rootId)
            return i;
    }

    return static_cast<int>(NECommon::INVALID_INDEX);
}

void LogScopesModel::slotLogInstancesConnect(const QList<NEService::sServiceConnectedInstance>& instances)
{
    beginResetModel();
    for (const NEService::sServiceConnectedInstance& instance : instances)
    {
        ScopeRoot* root = new ScopeRoot(instance);
        if (_appendRoot(root, true))
        {
            LogObserver::requestScopes(root->getRootId());
        }
        else
        {
            delete root;
        }
    }

    endResetModel();
}

void LogScopesModel::slotLogInstancesDisconnect(const QList<NEService::sServiceConnectedInstance>& instances)
{
    for (const NEService::sServiceConnectedInstance& instance : instances)
    {
        int pos = _findRoot(instance.ciSource);
        if (pos != static_cast<int>(NECommon::INVALID_INDEX))
        {
            beginRemoveRows(mRootIndex, pos, pos);
            mRootList.erase(pos);
            endRemoveRows();
        }
    }
}

void LogScopesModel::slotLogServiceDisconnected(const QMap<TEM_ID, NEService::sServiceConnectedInstance>& instances)
{
    beginResetModel();
    _clear();
    endResetModel();
}

void LogScopesModel::slotLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
    int pos = scopes.isEmpty() == false ? _findRoot(cookie) : static_cast<int>(NECommon::INVALID_INDEX);
    if (pos != static_cast<int>(NECommon::INVALID_INDEX))
    {
        beginResetModel();

        ScopeRoot* root = mRootList[pos];
        Q_ASSERT(root != nullptr);
        for (int i = 0; i < static_cast<int>(scopes.size()); ++i)
        {
            const sLogScope* scope = scopes[i];
            Q_ASSERT(scope != nullptr);
            root->addChildRecursive(QString(scope->lsName), scope->lsPrio);
        }

        endResetModel();
    }
}

void LogScopesModel::slotLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes)
{
}
