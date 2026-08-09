#ifndef LUSAN_DATA_COMMON_CONSTANTDATASECTION_HPP
#define LUSAN_DATA_COMMON_CONSTANTDATASECTION_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/ConstantDataSection.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the constants section of a document.
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
class DataTypeBase;
class DataTypeCustom;

/**
 * \class   ConstantDataSection
 * \brief   The `ConstantList` section of a document: an ordered container of ConstantEntry.
 *          A constant carries the same name, type, value and description in every document
 *          type, so one section serves the service interface, the state machine and a
 *          standalone data type document alike.
 **/
class ConstantDataSection    : public TEDataContainer<ConstantEntry, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    ConstantDataSection(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of constants.
     * \param   parent      The parent element.
     **/
    ConstantDataSection(const QList<ConstantEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks if the section is valid.
     **/
    bool isValid() const override;

    /**
     * \brief   Reads the constants from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the constants were read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes the constants to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Resolves every constant's declared type against the given custom types.
     * \param   customTypes     The document's custom data types.
     **/
    void validate(const QList<DataTypeCustom*>& customTypes);

    /**
     * \brief   Creates a new constant appended at the end of the list.
     * \param   name    The unique name of the new constant.
     * \return  Pointer to the created constant, or nullptr if the name is already taken.
     **/
    ConstantEntry* createConstant(const QString& name);

    /**
     * \brief   Inserts a new constant at the given position.
     * \param   position    The position to insert at.
     * \param   name        The unique name of the new constant.
     * \return  Pointer to the created constant, or nullptr if the name is already taken.
     **/
    ConstantEntry* insertConstant(int position, const QString& name);

    /**
     * \brief   Repoints every constant that uses the old data type to the new one.
     * \param   oldDataType     The data type to replace.
     * \param   newDataType     The data type to set.
     * \return  The IDs of the constants that changed.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);
};

#endif  // LUSAN_DATA_COMMON_CONSTANTDATASECTION_HPP
