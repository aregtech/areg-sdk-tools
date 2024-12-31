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
#include "lusan/data/common/DataTypeCustom.hpp"

SIDataTypeData::SIDataTypeData(void)
    : mDataTypes()
{
}

SIDataTypeData::SIDataTypeData(QList<DataTypeCustom *>&& entries) noexcept
    : mDataTypes(std::move(entries))
{
}

SIDataTypeData::~SIDataTypeData(void)
{
    qDeleteAll(mDataTypes);
    mDataTypes.clear();
}

const QList<DataTypeCustom *>& SIDataTypeData::getDataTypes(void) const
{
    return mDataTypes;
}

void SIDataTypeData::setDataTypes(QList<DataTypeCustom *>&& entries)
{
    mDataTypes = std::move(entries);
}

int SIDataTypeData::findDataType(const DataTypeCustom& entry) const
{
    for (int i = 0; i < mDataTypes.size(); ++i)
    {
        if (*mDataTypes[i] == entry)
        {
            return i;
        }
    }

    return -1;
}

void SIDataTypeData::addDataType(DataTypeCustom * entry)
{
    mDataTypes.append(entry);
}

bool SIDataTypeData::removeDataType(const DataTypeCustom& entry)
{
    int index = findDataType(entry);
    if (index != -1)
    {
        delete mDataTypes[index];
        mDataTypes.removeAt(index);
        return true;
    }

    return false;
}

bool SIDataTypeData::replaceDataType(const DataTypeCustom& oldEntry, DataTypeCustom * newEntry)
{
    int index = findDataType(oldEntry);
    if (index != -1)
    {
        delete mDataTypes[index];
        mDataTypes[index] = newEntry;
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
                    mDataTypes.append(dataType);
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
    for (const auto& dataType : mDataTypes)
    {
        dataType->writeToXml(xml);
    }

    xml.writeEndElement();
}

void SIDataTypeData::removeAll(void)
{
    qDeleteAll(mDataTypes);
    mDataTypes.clear();
}