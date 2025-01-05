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
 *  \file        lusan/data/common/DataTypeFactory.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type factory.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeFactory.hpp"

#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"

#include <QFile>
#include <QXmlStreamReader>

QList<DataTypePrimitive*>   DataTypeFactory::mPredefinePrimitiveTypes;    //!< The list of primitive data types.
QList<DataTypeBasic*>       DataTypeFactory::mPredefinedBasicTypes;       //!< The list of basic data types.
QList<DataTypeBasic*>       DataTypeFactory::mPredefinedContainerTypes;   //!< The list of container data types.


DataTypeBase::eCategory DataTypeFactory::fromString(const QString& dataType)
{
    if (dataType == TypeNameBool)
        return DataTypeBase::eCategory::Primitive;
    else if (dataType == TypeNameChar)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == TypeNameUint8)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == TypeNameInt16)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == TypeNameUint16)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == TypeNameInt32)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == TypeNameUint32)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == TypeNameInt64)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == TypeNameUint64)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == TypeNameFloat)
        return DataTypeBase::eCategory::PrimitiveFloat;
    else if (dataType == TypeNameDouble)
        return DataTypeBase::eCategory::PrimitiveFloat;
    else if (dataType == TypeNameString)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameBinary)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameDateTime)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameArray)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameLinkedList)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameHashMap)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameMap)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNamePair)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameNewType)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == TypeNameEnumerate)
        return DataTypeBase::eCategory::Enumeration;
    else if (dataType == TypeNameStructure)
        return DataTypeBase::eCategory::Structure;
    else if (dataType == TypeNameImported)
        return DataTypeBase::eCategory::Imported;
    else if (dataType == TypeNameDefined)
        return DataTypeBase::eCategory::Container;
    else
        return DataTypeBase::eCategory::Undefined;
}

DataTypeBase* DataTypeFactory::createDataType(const QString& dataType)
{
    DataTypeBase::eCategory category{ DataTypeFactory::fromString(dataType) };

    switch (category)
    {
    case DataTypeBase::eCategory::Primitive:
        return new DataTypePrimitive(dataType);

    case DataTypeBase::eCategory::PrimitiveSint:
        return new DataTypePrimitiveInt(dataType);

    case DataTypeBase::eCategory::PrimitiveUint:
        return new DataTypePrimitiveUint(dataType);

    case DataTypeBase::eCategory::PrimitiveFloat:
        return new DataTypePrimitiveFloat(dataType);

    case DataTypeBase::eCategory::BasicObject:
        return new DataTypeBasic(dataType);

    case DataTypeBase::eCategory::Enumeration:
    case DataTypeBase::eCategory::Structure:
    case DataTypeBase::eCategory::Imported:
    case DataTypeBase::eCategory::Container:
        return DataTypeFactory::createCustomDataType(category);

    default:
        return nullptr;
    }
}



DataTypeCustom* DataTypeFactory::createCustomDataType(const QString& type)
{
    return createCustomDataType(DataTypeCustom::fromTypeString(type));
}

DataTypeCustom* DataTypeFactory::createCustomDataType(DataTypeBase::eCategory category)
{
    switch (category)
    {
    case DataTypeBase::eCategory::Enumeration:
        return new DataTypeEnum();
    case DataTypeBase::eCategory::Structure:
        return new DataTypeStructure();
    case DataTypeBase::eCategory::Imported:
        return new DataTypeImported();
    case DataTypeBase::eCategory::Container:
        return new DataTypeDefined();
    default:
        return nullptr;
    }
}

const QList<DataTypePrimitive*>& DataTypeFactory::getPrimitiveTypes(void)
{
    if (mPredefinePrimitiveTypes.isEmpty())
        _initPredefined();
    return mPredefinePrimitiveTypes;
}

const QList<DataTypeBasic*>& DataTypeFactory::getBasicTypes(void)
{
    if (mPredefinedBasicTypes.isEmpty())
        _initPredefined();
    return mPredefinedBasicTypes;
}

const QList<DataTypeBasic*>& DataTypeFactory::getContainerTypes(void)
{
    if (mPredefinedContainerTypes.isEmpty())
        _initPredefined();
    return mPredefinedContainerTypes;
}

void DataTypeFactory::_initPredefined(void)
{
    Q_ASSERT(mPredefinePrimitiveTypes.isEmpty());
    Q_ASSERT(mPredefinedBasicTypes.isEmpty());
    Q_ASSERT(mPredefinedContainerTypes.isEmpty());

    // Open the resource file
    QFile file(":/data/Predefined Types");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return;

    // Create an XML stream reader
    QXmlStreamReader xml(&file);
    // Read the XML content
    while (!xml.atEnd() && !xml.hasError())
    {
        // If the token is a start element, process it
        if ((xml.readNext() == QXmlStreamReader::StartElement) && (xml.name() == XmlSI::xmlSIElementDataType))
        {
            QXmlStreamAttributes attributes = xml.attributes();
            QString typeName = attributes.value(XmlSI::xmlSIAttributeType).toString();
            QString name = attributes.value(XmlSI::xmlSIAttributeName).toString();
            bool hasKey = attributes.hasAttribute(XmlSI::xmlSIAttributeHasKey) ? attributes.value(XmlSI::xmlSIAttributeHasKey).toString() == XmlSI::xmlSIValueTrue : false;
            bool hasValue = attributes.hasAttribute(XmlSI::xmlSIAttributeHasValue) ? attributes.value(XmlSI::xmlSIAttributeHasValue).toString() == XmlSI::xmlSIValueTrue : false;
            DataTypeBase* dataType = DataTypeFactory::createDataType(name);
            if (dataType != nullptr)
            {
                if (typeName == XmlSI::xmlSIValuePrimitive)
                {
                    mPredefinePrimitiveTypes.append(static_cast<DataTypePrimitive*>(dataType));
                }
                else if (typeName == XmlSI::xmlSIValueBasic)
                {
                    mPredefinedBasicTypes.append(static_cast<DataTypeBasic*>(dataType));
                }
                else if (typeName == XmlSI::xmlSIValueContainer)
                {
                    Q_ASSERT(hasValue);
                    DataTypeBasic* container = static_cast<DataTypeBasic*>(dataType);
                    container->setDataContainer(hasValue, hasKey);
                    mPredefinedContainerTypes.append(container);
                }
            }
        }
    }
}
