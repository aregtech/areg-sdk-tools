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
 *  \file        lusan/model/common/DataTypesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Types Model.
 *
 ************************************************************************/

#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"

DataTypesModel::DataTypesModel(SIDataTypeData& dataTypeData,  QObject* parent)
    : QAbstractListModel(parent)
    , mDataTypeData (dataTypeData)
    , mExcludeList  ( )
{
}

DataTypesModel::DataTypesModel(SIDataTypeData& dataTypeData, const QStringList& excludes, QObject* parent)
    : QAbstractListModel(parent)
    , mDataTypeData (dataTypeData)
    , mExcludeList  ( )
{
    for (const QString& entry : excludes)
    {
        DataTypeBase* dataType = mDataTypeData.findDataType(entry);
        if ((dataType != nullptr) && (mExcludeList.contains(dataType) == false))
        {
            mExcludeList.append(dataType);
        }
    }
}

DataTypesModel::DataTypesModel(SIDataTypeData& dataTypeData, const QList<DataTypeBase*>& excludes, QObject* parent)
    : QAbstractListModel(parent)
    , mDataTypeData (dataTypeData)
    , mExcludeList  ( excludes )
{
}

void DataTypesModel::setFilter(const QStringList& excludes)
{
    mExcludeList.clear();
    for (const QString& entry : excludes)
    {
        DataTypeBase* dataType = mDataTypeData.findDataType(entry);
        if ((dataType != nullptr) && (mExcludeList.contains(dataType) == false))
        {
            mExcludeList.append(dataType);
        }
    }
}

void DataTypesModel::setFilter(const QList<DataTypeBase*>& excludes)
{
    mExcludeList = excludes;
}

void DataTypesModel::setFilter(const QList<DataTypeBase::eCategory>& excludes)
{
    mExcludeList.clear();
    for (DataTypeBase::eCategory category : excludes)
    {
        switch (category)
        {
        case DataTypeBase::eCategory::Primitive:
        case DataTypeBase::eCategory::PrimitiveSint:
        case DataTypeBase::eCategory::PrimitiveUint:
        case DataTypeBase::eCategory::PrimitiveFloat:
            {
                const QList<DataTypePrimitive*>& list = mDataTypeData.getPrimitiveDataTypes();
                for (DataTypePrimitive* dataType : list)
                {
                    if (dataType->getCategory() == category)
                    {
                        mExcludeList.append(dataType);
                    }
                }
            }
            break;

        case DataTypeBase::eCategory::BasicObject:
            {
                const QList<DataTypeBasicObject*>& list = mDataTypeData.getBasicDataTypes();
                uint32_t count = mExcludeList.size();
                mExcludeList.resize(count + list.size());
                for (DataTypeBasicObject* dataType : list)
                {
                    mExcludeList[count ++] = dataType;
                }
            }
            break;

        case DataTypeBase::eCategory::BasicContainer:
            {
                const QList<DataTypeBasicContainer*>& list = mDataTypeData.getContainerDatTypes();
                uint32_t count = mExcludeList.size();
                mExcludeList.resize(count + list.size());
                for (DataTypeBasicContainer* dataType : list)
                {
                    mExcludeList[count ++] = dataType;
                }
            }
            break;

        case DataTypeBase::eCategory::Enumeration:
        case DataTypeBase::eCategory::Structure:
        case DataTypeBase::eCategory::Imported:
        case DataTypeBase::eCategory::Container:
            {
                const QList<DataTypeCustom*>& list = mDataTypeData.getCustomDataTypes();
                for (DataTypeCustom* dataType : list)
                {
                    if (dataType->getCategory() == category)
                    {
                        mExcludeList.append(dataType);
                    }
                }
            }
            break;

        default:
            break;

        }
    }
}

void DataTypesModel::clearFilter()
{
    mExcludeList.clear();
}

int DataTypesModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    QList<DataTypeBase*> list;
    mDataTypeData.getDataType(list, mExcludeList, false);
    return static_cast<int>(list.size());
}

QVariant DataTypesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    
    QList<DataTypeBase*> list;
    mDataTypeData.getDataType(list, mExcludeList, false);
    DataTypeBase* dataType = (index.row() >= list.size()) ? nullptr : list[index.row()];
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    case Qt::ItemDataRole::DecorationRole:
    case Qt::ItemDataRole::EditRole:
        return QVariant(dataType != nullptr ? dataType->getName() : QString(""));
        break;
    
    case Qt::ItemDataRole::UserRole:
        return QVariant::fromValue(dataType);
    
    default:
        return QVariant();
    }
}
