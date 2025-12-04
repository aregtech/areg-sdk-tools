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
 *  \file        lusan/model/log/OfflineScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Scopes model.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/

#include "lusan/model/log/OfflineScopesModel.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/OfflineLogsModel.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "areg/base/NECommon.hpp"

#include <QIcon>

OfflineScopesModel::OfflineScopesModel(QObject* parent)
    : LoggingScopesModelBase(parent)
    , mMapScopeFilter       ( )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

OfflineScopesModel::~OfflineScopesModel(void)
{
}

bool OfflineScopesModel::setLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if ((root == nullptr) || (mLoggingModel == nullptr) || (mLoggingModel->isOperable() == false))
        return false;

    ITEM_ID instId{ static_cast<ScopeRoot*>(root)->getRootId() };
    auto pos = mMapScopeFilter.addIfUnique(instId, ScopeFilters{}, false).first;
    ASSERT(mMapScopeFilter.isValidPosition(pos));
    ScopeFilters& filterPrio = mMapScopeFilter.valueAtPosition(pos);
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;

    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->s();
        uint32_t scopePrio = _logFilterPrio(prio);
        filterPrio[scopeId] = scopePrio;
        filter.add({ scopeId, scopePrio });
    }

    mLoggingModel->applyFilters(instId, filter.getData());
    mLoggingModel->filterLogsAsynchronous();

    return true;
}

bool OfflineScopesModel::addLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if ((root == nullptr) || (mLoggingModel == nullptr) || (mLoggingModel->isOperable() == false))
        return false;
    
    ITEM_ID instId{ static_cast<ScopeRoot*>(root)->getRootId() };
    auto pos = mMapScopeFilter.addIfUnique(instId, ScopeFilters{}, false).first;
    ASSERT(mMapScopeFilter.isValidPosition(pos));
    ScopeFilters& filterPrio = mMapScopeFilter.valueAtPosition(pos);
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;
    
    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->getPriority();
        uint32_t scopePrio = filterPrio.contains(scopeId) ? filterPrio[scopeId] | static_cast<uint32_t>(prio) : static_cast<uint32_t>(NELogging);
        filterPrio[scopeId] = scopePrio;
        filter.add({ scopeId, scopePrio });
    }
    
    mLoggingModel->applyFilters(instId, filter.getData());
    mLoggingModel->filterLogsAsynchronous();
    
    return true;
}

bool OfflineScopesModel::removeLogPriority(const QModelIndex& index, NELogging::eLogPriority prio)
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if ((root == nullptr) || (mLoggingModel == nullptr) || (mLoggingModel->isOperable() == false))
        return false;
    
    ITEM_ID instId{ static_cast<ScopeRoot*>(root)->getRootId() };
    auto pos = mMapScopeFilter.addIfUnique(instId, ScopeFilters{}, false).first;
    ASSERT(mMapScopeFilter.isValidPosition(pos));
    ScopeFilters& filterPrio = mMapScopeFilter.valueAtPosition(pos);
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;
    
    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->getPriority();
        uint32_t scopePrio = filterPrio.contains(scopeId) ? filterPrio[scopeId] | static_cast<uint32_t>(prio) : static_cast<uint32_t>(prio);
        filterPrio[scopeId] = scopePrio;
        filter.add({ scopeId, scopePrio });
    }
    
    mLoggingModel->applyFilters(instId, filter.getData());
    mLoggingModel->filterLogsAsynchronous();
    
    return true;
}

bool OfflineScopesModel::saveLogScopePriority(const QModelIndex& target) const
{
    return false;
}

void OfflineScopesModel::setLoggingModel(LoggingModelBase* model)
{
    LoggingScopesModelBase::setLoggingModel(model);
    if ((model != nullptr) && model->isOperable())
    {
        if (model->rootCount() == 0)
        {
            const std::vector<NEService::sServiceConnectedInstance>& instances = model->getLogInstances();
            slotInstancesAvailable(instances);
            for (const auto& inst : instances)
            {
                const std::vector<NELogging::sScopeInfo>& scopes = model->getLogInstScopes(inst.ciCookie);
                slotScopesAvailable(inst.ciCookie, scopes);
            }
        }
        else
        {
            beginResetModel();
            endResetModel();
        }
    }
    else
    {
        beginResetModel();
        endResetModel();
    }
}

//////////////////////////////////////////////////////////////////////////
// Private helper methods
//////////////////////////////////////////////////////////////////////////

void OfflineScopesModel::_buildScopeTree(void)
{
    if (mLoggingModel == nullptr)
        return;

    beginResetModel();
    const std::vector<NEService::sServiceConnectedInstance>& instances = mLoggingModel->getLogInstances();
    slotInstancesAvailable(instances);

    for (const auto& inst : instances)
    {
        const std::vector<NELogging::sScopeInfo> & scopes = mLoggingModel->getLogInstScopes(inst.ciCookie);
        slotScopesAvailable(inst.ciCookie, scopes);
    }

    endResetModel();
}

constexpr uint32_t OfflineScopesModel::_logFilterPrio(NELogging::eLogPriority prio) const
{
    uint32_t result{ NELogging::eLogPriority::PrioUndefined };
    switch (prio)
    {
    case NELogging::eLogPriority::PrioScope:
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioScope);
        break;

    case NELogging::eLogPriority::PrioDebug:
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug); // fall through
    case NELogging::eLogPriority::PrioInfo:
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo); // fall through
    case NELogging::eLogPriority::PrioWarning:
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning); // fall through
    case NELogging::eLogPriority::PrioError:
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioError); // fall through
    case NELogging::eLogPriority::PrioFatal:
        result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal); // fall through
        break;

    case NELogging::eLogPriority::PrioNotset:
    case NELogging::eLogPriority::PrioUndefined:
    default:
        break;
    }

    return result;
}
