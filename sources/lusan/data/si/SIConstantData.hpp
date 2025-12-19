#ifndef LUSAN_DATA_SI_SICONSTANTDATA_HPP
#define LUSAN_DATA_SI_SICONSTANTDATA_HPP
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
 *  \file        lusan/data/si/SIConstantData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SIDataTypeData;
class DataTypeBase;

/**
 * \class   SIConstantData
 * \brief   Manages constant data for service interfaces.
 **/
class SIConstantData    : public TEDataContainer<ConstantEntry, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    SIConstantData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of constants.
     * \param   parent      The parent element.
     **/
    SIConstantData(const QList<ConstantEntry>& entries, ElementBase* parent = nullptr);

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
     * \brief   Reads constant data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the constant data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes constant data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Validates the constant data.
     * \param   dataTypes   The data type data to validate the constants.
     **/
    void validate(const SIDataTypeData& dataTypes);

    /**
     * \brief   Creates new ConstantEntry at the end of list of SIConstantData.
     * \param   name    The name of the new constant to create.
     * \return  Valid pointer to the new created constant element. Otherwise, returns nullptr.
     **/
    ConstantEntry* createConstant(const QString& name);

    /**
     * \brief   Replaces the data of constants in the list of constant entries.
     * \param   oldDataType     The old data type to replace.
     * \param   newDataType     The new data type to set.
     * \return  Returns the list IDs of constant entries, which.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

    /**
     * \brief   Inserts new ConstantEntry at the given position in SIConstantData.
     * \param   name    The name of the constant.
     * \return  Valid pointer to the new created constant element. Otherwise, returns nullptr.
     **/
    ConstantEntry* insertConstant(int position, const QString& name);
};

#endif  // LUSAN_DATA_SI_SICONSTANTDATA_HPP

