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
 *  \file        lusan/model/si/SIDataTypeModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type Model.
 *
 ************************************************************************/

#include "lusan/model/si/SIDataTypeModel.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"

SIDataTypeModel::SIDataTypeModel(SIDataTypeData& data, QObject* parent)
    : QAbstractTableModel(parent)
    , mData(data)
{
}

int SIDataTypeModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return mData.getCustomDataTypes().size();

    DataTypeCustom* dataType = static_cast<DataTypeCustom*>(parent.internalPointer());
    if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return static_cast<DataTypeEnum*>(dataType)->getElements().size();
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return static_cast<DataTypeStructure*>(dataType)->getElements().size();
    }

    return 0;
}

int SIDataTypeModel::columnCount(const QModelIndex& parent) const
{
    return 3;
}

QVariant SIDataTypeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    switch (role)
    {
    case static_cast<int>(Qt::ItemDataRole::DisplayRole):
    case static_cast<int>(Qt::ItemDataRole::EditRole):
    {
        DataTypeCustom* dataType = static_cast<DataTypeCustom*>(index.internalPointer());
        if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            if (col == 0)
            {
                return static_cast<DataTypeEnum*>(dataType)->getElements().at(row).getName();
            }
            else if (col == 2)
            {
                return static_cast<DataTypeEnum*>(dataType)->getElements().at(row).getValue();
            }
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            if (col == 0)
            {
                return static_cast<DataTypeStructure*>(dataType)->getElements().at(row).getName();
            }
            else if (col == 1)
            {
                return static_cast<DataTypeStructure*>(dataType)->getElements().at(row).getType();
            }
            else if (index.column() == 2)
            {
                return static_cast<DataTypeStructure*>(dataType)->getElements().at(row).getValue();
            }
        }
        else
        {
            if (col == 0)
            {
                return dataType->getName();
            }
        }
    }
    break;

    case static_cast<int>(Qt::ItemDataRole::UserRole):
    {
        DataTypeCustom* dataType = static_cast<DataTypeCustom*>(index.internalPointer());
        if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            return static_cast<DataTypeEnum*>(dataType)->getElements().at(row).getId();
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            return static_cast<DataTypeStructure*>(dataType)->getElements().at(row).getId();
        }
        else
        {
            return dataType->getId();
        }
    }
    break;

    default:
        break;
    }

    return QVariant();
}

QVariant SIDataTypeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return QString("Name:");
        case 1:
            return QString("Data Type:");
        case 2:
            return QString("Default Value:");
        }
    }

    return QVariant();
}

QModelIndex SIDataTypeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return createIndex(row, column, mData.getCustomDataTypes().at(row));
    }

    DataTypeCustom* dataType = static_cast<DataTypeCustom*>(parent.internalPointer());
    if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        return createIndex(row, column, dataType);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        return createIndex(row, column, dataType);
    }
    else
    {
        return createIndex(row, column, dataType);
    }
}

QModelIndex SIDataTypeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    int col = index.column();
    int row = index.row();
    DataTypeCustom* dataType = static_cast<DataTypeCustom*>(index.internalPointer());
    uint32_t id = data(index, static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    if (dataType->getId() != id)
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            int pos = mData.getCustomDataTypes().indexOf(dataType);
            return createIndex(pos, 0, dataType);
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            int pos = mData.getCustomDataTypes().indexOf(dataType);
            return createIndex(pos, 0, dataType);
        }
    }

    return QModelIndex();
}

DataTypeCustom* SIDataTypeModel::addDataType(const QString& name, DataTypeBase::eCategory category)
{
    const QList<DataTypeCustom*>& dataTypes = mData.getCustomDataTypes();
    beginInsertRows(QModelIndex(), dataTypes.size(), dataTypes.size());
    DataTypeCustom* dataType = mData.addCustomDataType(name, category);
    endInsertRows();
    return dataType;
}

DataTypeCustom* SIDataTypeModel::convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category)
{
    DataTypeCustom* result = mData.convertDataType(dataType, category);
    submit();
    return result;
}

DataTypeCustom* SIDataTypeModel::findDataType(const QString& name)
{
    int index = mData.findCustomDataType(name);
    return (index < 0 ? nullptr : mData.getCustomDataTypes().at(index));
}

DataTypeCustom* SIDataTypeModel::findDataType(uint32_t id)
{
    int index = mData.findCustomDataType(id);
    return (index < 0 ? nullptr : mData.getCustomDataTypes().at(index));
}
