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
 *  \file        lusan/data/si/SIConstantData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/

#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <memory>

SIDataTypeData::SIDataTypeData(void)
    : mCustomDataTypes      ( )
{
}

SIDataTypeData::SIDataTypeData(QList<DataTypeCustom *>&& entries) noexcept
    : mCustomDataTypes      (std::move(entries))
{
}

SIDataTypeData::~SIDataTypeData(void)
{
    qDeleteAll(mCustomDataTypes);
    mCustomDataTypes.clear();
}

int SIDataTypeData::findCustomDataType(const DataTypeCustom& entry) const
{
    for (int i = 0; i < mCustomDataTypes.size(); ++i)
    {
        if (*mCustomDataTypes[i] == entry)
        {
            return i;
        }
    }

    return -1;
}

void SIDataTypeData::addCustomDataType(DataTypeCustom * entry)
{
    mCustomDataTypes.append(entry);
}

bool SIDataTypeData::removeCustomDataType(const DataTypeCustom& entry)
{
    int index = findCustomDataType(entry);
    if (index != -1)
    {
        delete mCustomDataTypes[index];
        mCustomDataTypes.removeAt(index);
        return true;
    }

    return false;
}

bool SIDataTypeData::replaceCustomDataType(const DataTypeCustom& oldEntry, DataTypeCustom * newEntry)
{
    if (&oldEntry != newEntry)
    {
        return true;
    }

    int index = findCustomDataType(oldEntry);
    if (index != -1)
    {
        delete mCustomDataTypes[index];
        mCustomDataTypes[index] = newEntry;
        return true;
    }

    return false;
}

bool SIDataTypeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.readNextStartElement() && xml.name() == XmlSI::xmlSIElementDataTypeList)
    {
        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementDataType)
            {
                QString type = xml.attributes().value(XmlSI::xmlSIAttributeType).toString();
                DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(type);
                if (dataType && dataType->readFromXml(xml))
                {
                    mCustomDataTypes.append(dataType);
                }
                else
                {
                    xml.skipCurrentElement();
                }
            }
            else
            {
                xml.skipCurrentElement();
            }
        }

        return true;
    }

    return false;
}

void SIDataTypeData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementDataTypeList);
    for (const auto& dataType : mCustomDataTypes)
    {
        dataType->writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIDataTypeData::removeAll(void)
{
    qDeleteAll(mCustomDataTypes);
    mCustomDataTypes.clear();
}

const QList<DataTypePrimitive*>& SIDataTypeData::getPrimitiveDataTypes(void) const
{
    return DataTypeFactory::getPrimitiveTypes();
}

const QList<DataTypeBasic*>& SIDataTypeData::getBasicDataTypes(void) const
{
    return DataTypeFactory::getBasicTypes();
}

const QList<DataTypeBasic*>& SIDataTypeData::getContainerDatTypes(void) const
{
    return DataTypeFactory::getContainerTypes();
}

const QList<DataTypeCustom*>& SIDataTypeData::getCustomDataTypes(void) const
{
    return mCustomDataTypes;
}

void SIDataTypeData::setCustomDataTypes(QList<DataTypeCustom*>&& entries)
{
    mCustomDataTypes = std::move(entries);
}

void SIDataTypeData::getDataType(QList<DataTypeBase*>& out_dataTypes, const QList<DataTypeBase*>& exclude /*= QList<DataTypeCustom *>*/, bool makeSorting /*= true*/) const
{
    out_dataTypes.clear();
    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    for (DataTypePrimitive* dataType : primitives)
    {
        if (exists<DataTypeBase>(exclude, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }


    const QList<DataTypeBasic*>& basics = getBasicDataTypes();
    for (DataTypeBasic* dataType : basics)
    {
        if (exists<DataTypeBase>(exclude, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    const QList<DataTypeBasic*>& containers = getContainerDatTypes();
    for (DataTypeBasic* dataType : containers)
    {
        if (exists<DataTypeBase>(exclude, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    for (DataTypeCustom* dataType : mCustomDataTypes)
    {
        if (exists<DataTypeBase>(exclude, dataType->getName()) == false)
        {
            out_dataTypes.append(static_cast<DataTypeBase*>(dataType));
        }
    }

    if (makeSorting)
    {
        std::sort(out_dataTypes.begin(), out_dataTypes.end(), [](const DataTypeBase* lhs, const DataTypeBase* rhs) -> bool
            {
                Q_ASSERT(lhs->getName() != rhs->getName());
                return lhs->getName() < rhs->getName();
            });
    }
}

bool SIDataTypeData::existsPrimitive(const QList<DataTypePrimitive*> dataTypes, const QString& searchName) const
{
    return exists<DataTypePrimitive>(dataTypes, searchName);
}

bool SIDataTypeData::existsBasic(const QList<DataTypeBasic*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeBasic>(dataTypes, searchName);
}

bool SIDataTypeData::existsContainer(const QList<DataTypeBasic*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeBasic>(dataTypes, searchName);
}

bool SIDataTypeData::existsCustom(const QList<DataTypeCustom*> dataTypes, const QString& searchName) const
{
    return exists<DataTypeCustom>(dataTypes, searchName);
}

bool SIDataTypeData::exists(const QString& typeName) const
{
    if (existsCustom(mCustomDataTypes, typeName))
        return true;

    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    if (existsPrimitive(primitives, typeName))
        return true;

    const QList<DataTypeBasic*>& basics = getBasicDataTypes();
    if (existsBasic(basics, typeName))
        return true;

    const QList<DataTypeBasic*>& containers = getContainerDatTypes();
    if (existsContainer(containers, typeName))
        return true;

    return false;
}

DataTypeBase* SIDataTypeData::findDataType(const QString& typeName) const
{
    for (DataTypeCustom* dataType : mCustomDataTypes)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    const QList<DataTypePrimitive*>& primitives = getPrimitiveDataTypes();
    for (DataTypePrimitive* dataType : primitives)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    const QList<DataTypeBasic*>& basics = getBasicDataTypes();
    for (DataTypeBasic* dataType : basics)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    const QList<DataTypeBasic*>& containers = getContainerDatTypes();
    for (DataTypeBasic* dataType : containers)
    {
        if (dataType->getName() == typeName)
        {
            return static_cast<DataTypeBase *>(dataType);
        }
    }

    return nullptr;
}
