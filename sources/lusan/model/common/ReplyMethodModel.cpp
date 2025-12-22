/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/common/ReplyMethodModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Reply Method Model implementation.
 *
 ************************************************************************/

#include "lusan/model/common/ReplyMethodModel.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/si/SIMethodData.hpp"
#include "lusan/data/si/SIMethodBase.hpp"
#include "lusan/data/si/SIMethodBroadcast.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/data/si/SIMethodRequest.hpp"

ReplyMethodModel::ReplyMethodModel(SIMethodData& data, QObject* parent)
    : QAbstractListModel(parent)
    , mData     (data)
    , mMethods  ( )
{
}

void ReplyMethodModel::addMethodResponse(SIMethodResponse* methodResponse)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mMethods.append(methodResponse);
    NELusanCommon::sortByName<const SIMethodResponse*>(mMethods.begin() + 1, mMethods.end(), true);
    endInsertRows();
}

void ReplyMethodModel::removeMethodResponse(SIMethodResponse* methodResponse)
{
    int index = mMethods.indexOf(methodResponse);
    if (index > 0)
    {
        beginRemoveRows(QModelIndex(), index, index);
        mMethods.removeAt(index);
        endRemoveRows();
    }
}

void ReplyMethodModel::sortByName(bool ascending)
{
    NELusanCommon::sortByName<const SIMethodResponse *>(mMethods.begin() + 1, mMethods.end(), ascending);
    emit dataChanged(index(1), index(rowCount() - 1), QList<int>{Qt::ItemDataRole::UserRole, Qt::ItemDataRole::DisplayRole});
}

void ReplyMethodModel::sortById(bool ascending)
{
    NELusanCommon::sortById<const SIMethodResponse *>(mMethods.begin() + 1, mMethods.end(), ascending);
    emit dataChanged(index(1), index(rowCount() - 1), QList<int>{Qt::ItemDataRole::UserRole, Qt::ItemDataRole::DisplayRole});
}

int ReplyMethodModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mMethods.count();
}

QVariant ReplyMethodModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= mMethods.count())
        return QVariant();

    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    case Qt::ItemDataRole::EditRole:
    {
        const SIMethodResponse* method = mMethods.at(index.row());
        return QVariant(method != nullptr ? method->getName() : QString(""));
    }
    break;
    case Qt::ItemDataRole::UserRole:
    {
        const SIMethodResponse* method = mMethods.at(index.row());
        return QVariant::fromValue(method);        
    }
    break;
    
    default:
        break;
    }

    return QVariant();
}

void ReplyMethodModel::methodCreated(SIMethodBase* method)
{
    if (method->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
    {
        addMethodResponse(static_cast<SIMethodResponse*>(method));
    }
}

void ReplyMethodModel::methodConverted(SIMethodBase* oldMethod, SIMethodBase* newMethod)
{
    if ((oldMethod != nullptr) && (oldMethod->getMethodType() == SIMethodBase::eMethodType::MethodResponse))
    {
        removeMethodResponse(static_cast<SIMethodResponse*>(oldMethod));
    }

    if ((newMethod != nullptr) && (newMethod->getMethodType() == SIMethodBase::eMethodType::MethodResponse))
    {
        addMethodResponse(static_cast<SIMethodResponse*>(newMethod));
    }
}

void ReplyMethodModel::methodRemoved(SIMethodBase* method)
{
    if ((method != nullptr) && (method->getMethodType() == SIMethodBase::eMethodType::MethodResponse))
    {
        removeMethodResponse(static_cast<SIMethodResponse*>(method));
    }
}

void ReplyMethodModel::methodUpdated(SIMethodBase* method)
{
    if ((method != nullptr) && (method->getMethodType() == SIMethodBase::eMethodType::MethodResponse))
    {
        int index = mMethods.indexOf(static_cast<SIMethodResponse*>(method));
        if (index > 0)
        {
            emit dataChanged(this->index(index), this->index(index), QList<int>{Qt::ItemDataRole::UserRole});
        }
    }
}

void ReplyMethodModel::updateList(void)
{
    const QList<SIMethodResponse*>& list{ mData.getResponses() };
    uint32_t count = list.size();
    mMethods.clear();
    beginInsertRows(QModelIndex(), 0, count + 1u);
    mMethods.append(static_cast<SIMethodResponse*>(nullptr));
    mMethods.append(list);
    NELusanCommon::sortByName<const SIMethodResponse*>(mMethods.begin() + 1, mMethods.end(), true);
    endInsertRows();
}

SIMethodResponse* ReplyMethodModel::findResponse(const QString& name) const
{
    int count = static_cast<int>(mMethods.size());
    for (int i = 1; i < count; ++i)
    {
        SIMethodResponse* method = mMethods.at(i);
        Q_ASSERT(method != nullptr);
        if (method->getName() == name)
        {
            return method;
        }
    }

    return nullptr;
}

SIMethodResponse* ReplyMethodModel::findResponse(uint32_t id) const
{
    int count = static_cast<int>(mMethods.size());
    for (int i = 1; i < count; ++i)
    {
        SIMethodResponse* method = mMethods.at(i);
        Q_ASSERT(method != nullptr);
        if (method->getId() == id)
        {
            return method;
        }
    }

    return nullptr;
}
