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
    , mDataTypeList ( )
    , mCountPredef  ( 0 )
{
}

DataTypesModel::DataTypesModel(SIDataTypeData& dataTypeData, const QStringList& excludes, QObject* parent)
    : QAbstractListModel(parent)
    , mDataTypeData (dataTypeData)
    , mExcludeList  ( )
    , mDataTypeList ( )
    , mCountPredef  ( 0 )
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
    , mDataTypeList ( )
    , mCountPredef  ( 0 )
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

void DataTypesModel::setInclusiveFilter(const QStringList& inclusive)
{
    mExcludeList.clear();
    mDataTypeData.getDataType(mExcludeList, QList<DataTypeBase*>(), false);
    for (const QString& entry : inclusive)
    {
        DataTypeBase* dataType = mDataTypeData.findDataType(entry);
        int index = mExcludeList.indexOf(dataType);
        if (index >= 0)
        {
            mExcludeList.removeAt(index);
        }
    }
}

void DataTypesModel::setInclusiveFilter(const QList<DataTypeBase*>& inclusive)
{
    mExcludeList.clear();
    mDataTypeData.getDataType(mExcludeList, QList<DataTypeBase*>(), false);
    for (DataTypeBase* dataType : inclusive)
    {
        int index = mExcludeList.indexOf(dataType);
        if (index >= 0)
        {
            mExcludeList.removeAt(index);
        }
    }
}

void DataTypesModel::setInclusiveFilter(const QList<DataTypeBase::eCategory>& inclusive)
{
    mExcludeList.clear();
    mDataTypeData.getDataType(mExcludeList, QList<DataTypeBase*>(), false);
    for (DataTypeBase::eCategory category : inclusive)
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
                int index = mExcludeList.indexOf(dataType);
                if (index >= 0)
                {
                    mExcludeList.removeAt(index);
                }
            }
        }
        break;

        case DataTypeBase::eCategory::BasicObject:
        {
            const QList<DataTypeBasicObject*>& list = mDataTypeData.getBasicDataTypes();
            for (DataTypeBasicObject* dataType : list)
            {
                int index = mExcludeList.indexOf(dataType);
                if (index >= 0)
                {
                    mExcludeList.removeAt(index);
                }
            }
        }
        break;

        case DataTypeBase::eCategory::BasicContainer:
        {
            const QList<DataTypeBasicContainer*>& list = mDataTypeData.getContainerDatTypes();
            for (DataTypeBasicContainer* dataType : list)
            {
                int index = mExcludeList.indexOf(dataType);
                if (index >= 0)
                {
                    mExcludeList.removeAt(index);
                }
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
                int index = mExcludeList.indexOf(dataType);
                if (index >= 0)
                {
                    mExcludeList.removeAt(index);
                }
            }
        }
        break;

        default:
            break;
        }
    }
}

void DataTypesModel::addToFilter(const DataTypeBase* dataType)
{
    if ((dataType != nullptr) && (mExcludeList.contains(dataType) == false))
    {
        mExcludeList.append(const_cast<DataTypeBase *>(dataType));
    }
}

void DataTypesModel::removeFromFilter(const DataTypeBase* dataType)
{
    if ((dataType != nullptr) && (mExcludeList.contains(dataType) == true))
    {
        mExcludeList.removeAll(dataType);
    }
}

void DataTypesModel::clearFilter()
{
    mExcludeList.clear();
}

int DataTypesModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(mDataTypeList.size());
}

QVariant DataTypesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.row() >= static_cast<int>(mDataTypeList.size())) )
        return QVariant();
    
    
    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    case Qt::ItemDataRole::EditRole:
    {
        DataTypeBase* dataType = mDataTypeList[index.row()];
        Q_ASSERT(dataType != nullptr);
        return QVariant(dataType->getName());
    }
    break;
    
    case Qt::ItemDataRole::UserRole:
    {
        DataTypeBase* dataType = mDataTypeList[index.row()];
        Q_ASSERT(dataType != nullptr);
        return QVariant::fromValue(dataType);
    }
    break;
    
    default:
        return QVariant();
    }
}

bool DataTypesModel::dataTypeCreated(DataTypeCustom* dataType)
{
    const int count{ mCountPredef };
    Q_ASSERT(mDataTypeList.indexOf(dataType) == -1);
    if (mExcludeList.indexOf(dataType) == -1)
    {
        beginInsertRows(QModelIndex(), static_cast<int>(mDataTypeList.size()), static_cast<int>(mDataTypeList.size()));
        mDataTypeList.append(dataType);
        endInsertRows();
        _sort(false);

        return true;
    }

    return false;
}

bool DataTypesModel::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    const int count{ mCountPredef };
    Q_ASSERT(mDataTypeList.indexOf(newType) == -1);
    int index = mDataTypeList.indexOf(oldType);
    if (index >= 0)
    {
        mDataTypeList[index] = newType;
        _sort(false);
        return true;
    }

    return false;
}

bool DataTypesModel::dataTypeRemoved(DataTypeCustom* dataType)
{
    const int count{ mCountPredef };
    int index = mDataTypeList.indexOf(dataType);
    if (index >= 0)
    {
        beginRemoveRows(QModelIndex(), index, index);
        mDataTypeList.removeAt(index);
        endRemoveRows();

        return true;
    }

    return false;
}

bool DataTypesModel::dataTypeUpdated(DataTypeCustom* dataType)
{
    return (mDataTypeList.indexOf(dataType) != -1);
}

void DataTypesModel::updateDataTypeLists(void)
{
    mDataTypeList.clear();
    mCountPredef = 0;
    mDataTypeData.getDataType(mDataTypeList, mExcludeList, true);
    for (DataTypeBase* dataType : mDataTypeList)
    {
        if (dataType->isPredefined())
        {
            ++mCountPredef;
        }
    }
}

bool DataTypesModel::removeDataType(DataTypeCustom* dataType)
{
    int index = mDataTypeList.indexOf(dataType);
    if (index >= 0)
    {
        Q_ASSERT(mExcludeList.indexOf(dataType) == -1);
        mExcludeList.append(dataType);
        beginRemoveRows(QModelIndex(), index, index);
        mDataTypeList.removeAt(index);
        endRemoveRows();

        return true;
    }
    else if (mExcludeList.indexOf(dataType) == -1)
    {
        mExcludeList.append(dataType);
        return false;
    }

    return false;
}

bool DataTypesModel::addDataType(DataTypeCustom* dataType)
{
    if (mDataTypeList.indexOf(dataType) < 0)
    {
        int index = mExcludeList.indexOf(dataType);
        Q_ASSERT(index != -1);
        if (index >= 0)
        {
            mExcludeList.removeAt(index);
        }
        
        beginInsertRows(QModelIndex(), static_cast<int>(mDataTypeList.size()), static_cast<int>(mDataTypeList.size()));
        mDataTypeList.append(dataType);
        endInsertRows();
        _sort(false);

        return true;
    }

    return false;
}

inline void DataTypesModel::_sort(bool sortPredefined /*= true*/)
{
    const int count{ mCountPredef };
    if (count > 0)
    {
        if (sortPredefined)
        {
            std::sort(mDataTypeList.begin() + count, mDataTypeList.begin() + count - 1, [](const DataTypeBase* lhs, const DataTypeBase* rhs) -> bool
                {
                    return lhs->getId() < rhs->getId();
                });
        }

        std::sort(mDataTypeList.begin() + count, mDataTypeList.end(), [](const DataTypeBase* lhs, const DataTypeBase* rhs) -> bool
            {
                return lhs->getName().compare(rhs->getName(), Qt::CaseSensitivity::CaseInsensitive) < 0;
            });
    }
}
