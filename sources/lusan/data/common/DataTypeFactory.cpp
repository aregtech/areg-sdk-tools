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

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"

#include <QFile>
#include <QXmlStreamReader>

QList<DataTypePrimitive*>       DataTypeFactory::mPredefinePrimitiveTypes;    //!< The list of primitive data types.
QList<DataTypeBasicObject*>     DataTypeFactory::mPredefinedBasicTypes;       //!< The list of basic data types.
QList<DataTypeBasicContainer*>  DataTypeFactory::mPredefinedContainerTypes;   //!< The list of container data types.


DataTypeBase::eCategory DataTypeFactory::fromString(const QString& dataType)
{
    if (dataType == XmlSI::xmlSIValueBool)
        return DataTypeBase::eCategory::Primitive;
    else if (dataType == XmlSI::xmlSIValueChar)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == XmlSI::xmlSIValueUint8)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == XmlSI::xmlSIValueInt16)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == XmlSI::xmlSIValueUint16)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == XmlSI::xmlSIValueInt32)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == XmlSI::xmlSIValueUint32)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == XmlSI::xmlSIValueInt64)
        return DataTypeBase::eCategory::PrimitiveSint;
    else if (dataType == XmlSI::xmlSIValueUint64)
        return DataTypeBase::eCategory::PrimitiveUint;
    else if (dataType == XmlSI::xmlSIValueFloat)
        return DataTypeBase::eCategory::PrimitiveFloat;
    else if (dataType == XmlSI::xmlSIValueDouble)
        return DataTypeBase::eCategory::PrimitiveFloat;
    else if (dataType == XmlSI::xmlSIValueString)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == XmlSI::xmlSIValueBinary)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == XmlSI::xmlSIValueDateTime)
        return DataTypeBase::eCategory::BasicObject;
    else if (dataType == XmlSI::xmlSIValueArray)
        return DataTypeBase::eCategory::BasicContainer;
    else if (dataType == XmlSI::xmlSIValueLinkedList)
        return DataTypeBase::eCategory::BasicContainer;
    else if (dataType == XmlSI::xmlSIValueHashMap)
        return DataTypeBase::eCategory::BasicContainer;
    else if (dataType == XmlSI::xmlSIValueMap)
        return DataTypeBase::eCategory::BasicContainer;
    else if (dataType == XmlSI::xmlSIValuePair)
        return DataTypeBase::eCategory::BasicContainer;
    else if (dataType == XmlSI::xmlSIValueNewType)
        return DataTypeBase::eCategory::BasicContainer;
    else if (dataType == XmlSI::xmlSIValueEnumeration)
        return DataTypeBase::eCategory::Enumeration;
    else if (dataType == XmlSI::xmlSIValueStructure)
        return DataTypeBase::eCategory::Structure;
    else if (dataType == XmlSI::xmlSIValueImported)
        return DataTypeBase::eCategory::Imported;
    else if (dataType == XmlSI::xmlSIValueContainer)
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
        return new DataTypeBasicObject(dataType);

    case DataTypeBase::eCategory::BasicContainer:
        return new DataTypeBasicContainer(dataType);

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
        return new DataTypeContainer();
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

const QList<DataTypeBasicObject*>& DataTypeFactory::getBasicTypes(void)
{
    if (mPredefinedBasicTypes.isEmpty())
        _initPredefined();
    return mPredefinedBasicTypes;
}

const QList<DataTypeBasicContainer*>& DataTypeFactory::getContainerTypes(void)
{
    if (mPredefinedContainerTypes.isEmpty())
        _initPredefined();
    return mPredefinedContainerTypes;
}

int DataTypeFactory::getPredefinedTypes(QList<DataTypeBase *>& result, const QList<DataTypeBase::eCategory> & categories)
{
    int initCount = static_cast<int>(result.size());
    for (auto category : categories)
    {
        switch (category)
        {
        case DataTypeBase::eCategory::Primitive:
        case DataTypeBase::eCategory::PrimitiveSint:
        case DataTypeBase::eCategory::PrimitiveUint:
        case DataTypeBase::eCategory::PrimitiveFloat:
        {
            const QList<DataTypePrimitive*>& types { DataTypeFactory::getPrimitiveTypes() };
            for (auto dataType : types)
            {
                if (dataType->getCategory() == category)
                {
                    result.append(dataType);
                }
            }
        }
        break;
        
        case DataTypeBase::eCategory::BasicObject:
        {
            const QList<DataTypeBasicObject*>& types {DataTypeFactory::getBasicTypes()};
            uint32_t count = result.size();
            result.resize(count + types.size());
            for (auto dataType : types)
            {
                result[count++] = dataType;
            }
        }
        break;
            
        case DataTypeBase::eCategory::BasicContainer:
        {
            const QList<DataTypeBasicContainer*>& types {DataTypeFactory::getContainerTypes()};
            uint32_t count = result.size();
            result.resize(count + types.size());
            for (auto dataType : types)
            {
                result[count++] = dataType;
            }
        }
        break;
        
        default:
            break;
        }
    }

    NELusanCommon::sortById<const DataTypeBase*>(result.begin() + initCount, result.end(), true);
    return static_cast<int>(result.size()) - initCount;
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
            uint32_t id = attributes.value(XmlSI::xmlSIAttributeID).toUInt();
            QString typeName = attributes.value(XmlSI::xmlSIAttributeType).toString();
            QString name = attributes.value(XmlSI::xmlSIAttributeName).toString();
            bool hasKey = attributes.hasAttribute(XmlSI::xmlSIAttributeHasKey) ? attributes.value(XmlSI::xmlSIAttributeHasKey).toString() == XmlSI::xmlSIValueTrue : false;
            bool hasValue = attributes.hasAttribute(XmlSI::xmlSIAttributeHasValue) ? attributes.value(XmlSI::xmlSIAttributeHasValue).toString() == XmlSI::xmlSIValueTrue : false;
            DataTypeBase* dataType = DataTypeFactory::createDataType(name);
            if (dataType != nullptr)
            {
                dataType->setId(id);
                dataType->setParent(static_cast<ElementBase *>(nullptr));

                if (typeName == XmlSI::xmlSIValuePrimitive)
                {
                    mPredefinePrimitiveTypes.append(static_cast<DataTypePrimitive*>(dataType));
                }
                else if (typeName == XmlSI::xmlSIValueBasicObject)
                {
                    mPredefinedBasicTypes.append(static_cast<DataTypeBasicObject*>(dataType));
                }
                else if (typeName == XmlSI::xmlSIValueBasicContainer)
                {
                    Q_ASSERT(hasValue);
                    DataTypeBasicContainer* container = static_cast<DataTypeBasicContainer*>(dataType);
                    container->setKey(hasKey);
                    mPredefinedContainerTypes.append(container);
                }
            }
        }
    }
}
