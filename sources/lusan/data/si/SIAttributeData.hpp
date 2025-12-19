#ifndef LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
#define LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
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
 *  \file        lusan/data/si/SIAttributeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Attribute Data.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include "lusan/data/common/AttributeEntry.hpp"
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class SIDataTypeData;
class DataTypeBase;

/**
 * \class   SIAttributeData
 * \brief   Manages attribute data for service interfaces.
 **/
class SIAttributeData   : public TEDataContainer< AttributeEntry, DocumentElem >
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIAttributeData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of attributes.
     **/
    SIAttributeData(const QList<AttributeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Checks if the parameter is valid.
     * \return  True if the parameter is valid, false otherwise.
     **/
    virtual bool isValid() const override;

    /**
     * \brief   Reads attribute data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the attribute data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes attribute data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Validates the attribute data.
     * \param   dataTypes   The data type data to validate the attributes.
     **/
    void validate(const SIDataTypeData& dataTypes);

    /**
     * \brief   Creates a AttributeEntry and sets it in SIAttributeData.
     * \param   name    The name of the new attribute to create.
     * \param   notification    The notification type of the attribute.
     * \return  Valid pointer to the new created attribute element. Otherwise, returns nullptr.
     **/
    AttributeEntry* createAttribute(const QString& name, AttributeEntry::eNotification notification = AttributeEntry::eNotification::NotifyOnChange);

    /**
     * \brief   Replaces the data of attributes in the list of attribute entries.
     * \param   oldDataType     The old data type to replace.
     * \param   newDataType     The new data type to set.
     * \return  Returns the list IDs of attribute entries, which.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

    /**
     * \brief   Inserts new AttributeEntry at the given position in SIAttributeData.
     * \param   name    The name of the data attribute.
     * \return  Valid pointer to the new created attribute element. Otherwise, returns nullptr.
     **/
    AttributeEntry* insertAttribute(int position, const QString& name, AttributeEntry::eNotification notification = AttributeEntry::eNotification::NotifyOnChange);
};

#endif  // LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
