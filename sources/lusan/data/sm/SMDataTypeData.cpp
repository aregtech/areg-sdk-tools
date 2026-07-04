/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMDataTypeData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM data types registry. Reuses DataTypeCustom.
 *
 ************************************************************************/

#include "lusan/data/sm/SMDataTypeData.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMDataTypeData::SMDataTypeData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<DataTypeCustom*, DocumentElem>(parent)
{
}

SMDataTypeData::~SMDataTypeData(void)
{
    removeAll();
}

bool SMDataTypeData::isValid(void) const
{
    return true;
}

bool SMDataTypeData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMDataTypeData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

const QList<DataTypeCustom*>& SMDataTypeData::getCustomDataTypes(void) const
{
    return getElements();
}

DataTypeCustom* SMDataTypeData::addCustomDataType(const QString& name, DataTypeBase::eCategory category)
{
    if (findCustomDataType(name) != nullptr)
    {
        return nullptr;
    }

    DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(category);
    if (dataType == nullptr)
    {
        return nullptr;
    }

    dataType->setParent(this);
    dataType->setId(getNextId());
    dataType->setName(name);
    addElement(dataType, true);
    return dataType;
}

DataTypeCustom* SMDataTypeData::addEnum(const QString& name)
{
    return addCustomDataType(name, DataTypeBase::eCategory::Enumeration);
}

DataTypeCustom* SMDataTypeData::addStructure(const QString& name)
{
    return addCustomDataType(name, DataTypeBase::eCategory::Structure);
}

DataTypeCustom* SMDataTypeData::addImported(const QString& name)
{
    return addCustomDataType(name, DataTypeBase::eCategory::Imported);
}

DataTypeCustom* SMDataTypeData::findCustomDataType(const QString& name) const
{
    DataTypeCustom* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

DataTypeCustom* SMDataTypeData::findCustomDataType(uint32_t id) const
{
    DataTypeCustom* const* found = findElement(id);
    return (found != nullptr) ? *found : nullptr;
}

void SMDataTypeData::removeAll(void)
{
    for (DataTypeCustom* dataType : getElements())
    {
        delete dataType;
    }

    removeAllElements();
}
