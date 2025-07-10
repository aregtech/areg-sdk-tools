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
 *  \file        lusan/model/log/LiveScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/model/log/LiveScopesModel.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/data/log/LogObserver.hpp"

LiveScopesModel::LiveScopesModel(QObject* parent)
    : LoggingScopesModelBase( parent )
{
}

LiveScopesModel::~LiveScopesModel(void)
{
}

void LiveScopesModel::setLoggingModel(LoggingModelBase* model)
{
    LoggingScopesModelBase::setLoggingModel(model);
}

bool LiveScopesModel::setLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    if (node->getPriority() != static_cast<uint32_t>(prio))
    {
        node->setPriority(static_cast<uint32_t>(prio));
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        sLogScope scope;
        scope.lsId = 0;
        scope.lsPrio = node->getPriority();
        QString path = node->makePath();
        if (node->isLeaf() == false)
        {
            path += NELusanCommon::SCOPE_ALL;
        }
        
        NEString::copyString(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str());
        result = LogObserver::requestChangeScopePrio(static_cast<ScopeRoot *>(root)->getRootId(), &scope, 1);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LiveScopesModel::addLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    // if (((node->getPriority() & static_cast<uint32_t>(prio)) == 0) || node->hasMultiPrio(static_cast<uint32_t>(prio)))
    if (node->canAddPriority(static_cast<uint32_t>(prio)))
    {
        node->addPriority(static_cast<uint32_t>(prio));        
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        
        result = _requestNodePriority(static_cast<const ScopeRoot &>(*root), *node);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LiveScopesModel::removeLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    bool result{ false };
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if (root == nullptr)
        return result;
    
    // if ((node->getPriority() & static_cast<uint32_t>(prio)) != 0)
    if (node->canRemovePriority(static_cast<uint32_t>(prio)))
    {
        node->removePriority(static_cast<uint32_t>(prio));
        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        result = _requestNodePriority(static_cast<const ScopeRoot &>(*root), *node);
    }
    else
    {
        result = true;
    }

    return result;
}

bool LiveScopesModel::saveLogScopePriority(const QModelIndex& target /*= QModelIndex()*/) const
{
    if (target.isValid())
    {
        ScopeNodeBase* node = static_cast<ScopeNodeBase *>(target.internalPointer());
        Q_ASSERT(node != nullptr);
        ScopeRoot* root = static_cast<ScopeRoot *>(node->getTreeRoot());
        return LogObserver::requestSaveConfig(root->getRootId());
    }
    else
    {
        return LogObserver::requestSaveConfig(NEService::TARGET_ALL);
    }
}

bool LiveScopesModel::slotInstancesAvailable(const std::vector<NEService::sServiceConnectedInstance> & instances)
{
    if (LoggingScopesModelBase::slotInstancesAvailable(instances))
    {
        for (const auto & entry : instances)
        {
            LogObserver::requestScopes(entry.ciCookie);
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

bool LiveScopesModel::_requestNodePriority(const ScopeRoot& root, const ScopeNodeBase& node)
{
    bool result{false};
    QList<ScopeNodeBase*> nodes;
    int count = node.extractChildNodesWithPriority(nodes);
    count += count == 0 ? 1 : 0;
    sLogScope* scopes = new sLogScope[count];
    if (scopes != nullptr)
    {
        int pos = 0;
        if (nodes.isEmpty())
        {
            // Q_ASSERT(node.hasPrioNotset());
            sLogScope& scope = scopes[0];
            scope.lsId = 0;
            scope.lsPrio = node.getPriority();
            QString path = node.makePath();
            if (node.isLeaf() == false)
            {
                path += NELusanCommon::SCOPE_ALL;
            }

            NEMemory::memCopy(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str(), path.length() + 1);
            ++pos;
        }

        for (; pos < count; ++pos)
        {
            ScopeNodeBase* nodeBase = nodes[pos];
            sLogScope& scope = scopes[pos];
            scope.lsId = 0;
            scope.lsPrio = nodeBase->getPriority();
            QString path = nodeBase->makePath();
            if (nodeBase->isLeaf() == false)
            {
                path += NELusanCommon::SCOPE_ALL;
            }

            NEMemory::memCopy(scope.lsName, LENGTH_SCOPE, path.toStdString().c_str(), path.length() + 1);
        }

        result = LogObserver::requestChangeScopePrio(root.getRootId(), scopes, count);
        delete[] scopes;
    }
    
    return result;
}
