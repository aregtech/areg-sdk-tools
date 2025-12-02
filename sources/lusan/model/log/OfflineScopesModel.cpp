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
    , MapScopeFilter        ( )
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
    if (mMapScopeFilter.find(instId) == mMapScopeFilter.end())
    {
        // no filter for this instance, create empty filter
        mMapScopeFilter.insert(std::make_pair(instId, ListScopeFilter()));
    }

    std::unordered_map<uint32_t, uint32_t>& filterPrio = mMapScopeFilter[instId];
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;

    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->getScopeId();
        filterPrio[scopeId] = static_cast<uint32_t>(prio);
        filter.push_back({ scopeId, filterPrio[scopeId] });
    }

    mLoggingModel->applyFilters(instId, filter);
    mLoggingModel->filterLogsAsynchronous();

    return true;
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
