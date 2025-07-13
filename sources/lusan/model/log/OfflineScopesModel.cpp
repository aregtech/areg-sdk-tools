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
{
    mRootIndex = createIndex(0, 0, nullptr);
}

OfflineScopesModel::~OfflineScopesModel(void)
{
}

void OfflineScopesModel::setLoggingModel(LoggingModelBase* model)
{
    LoggingScopesModelBase::setLoggingModel(model);
    if ((model != nullptr) && model->isOperable())
    {
        mRootList.clear();
        const std::vector<NEService::sServiceConnectedInstance>& instances = model->getLogInstances();
        slotInstancesAvailable(instances);
        
        for (const auto& inst : instances)
        {
            const std::vector<NELogging::sScopeInfo>& scopes = model->getLogInstScopes(inst.ciCookie);
            slotScopesAvailable(inst.ciCookie, scopes);
        }
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
