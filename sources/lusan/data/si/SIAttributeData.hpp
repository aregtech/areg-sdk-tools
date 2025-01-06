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
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIAttributeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Attribute Data.
 *
 ************************************************************************/

#include "lusan/common/ElementBase.hpp"

#include "lusan/data/common/AttributeEntry.hpp"
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \class   SIAttributeData
 * \brief   Manages attribute data for service interfaces.
 **/
class SIAttributeData   : public ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Local types and static members
//////////////////////////////////////////////////////////////////////////
private:
    static const AttributeEntry InvalidAttribute; //!< Invalid attribute object

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
     * \brief   Gets the list of attributes.
     * \return  The list of attributes.
     **/
    const QList<AttributeEntry>& getAttributes(void) const;

    /**
     * \brief   Sets the list of attributes.
     * \param   entries     The list of attributes.
     **/
    void setAttributes(const QList<AttributeEntry>& entries);

    /**
     * \brief   Searches for an attribute in the list.
     * \param   entry   The attribute to search for.
     * \return  The index of the attribute, or -1 if not found.
     **/
    int findAttribute(const AttributeEntry& entry) const;

    /**
     * \brief   Removes an attribute from the list.
     * \param   entry   The attribute to remove.
     * \return  True if the attribute was removed, false otherwise.
     **/
    bool removeAttribute(const AttributeEntry& entry);

    /**
     * \brief   Replaces an attribute in the list.
     * \param   oldEntry    The attribute to replace.
     * \param   newEntry    The new attribute.
     * \return  True if the attribute was replaced, false otherwise.
     **/
    bool replaceAttribute(const AttributeEntry& oldEntry, AttributeEntry&& newEntry);

    /**
     * \brief   Reads attribute data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the attribute data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes attribute data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Finds an attribute by name.
     * \param   name    The name to search for.
     * \return  The index of the attribute, or -1 if not found.
     **/
    int findAttribute(const QString& name) const;

    /**
     * \brief   Checks if an attribute exists by name.
     * \param   name    The name to check.
     * \return  True if the attribute exists, false otherwise.
     **/
    bool exists(const QString& name) const;

    /**
     * \brief   Gets an attribute by name.
     * \param   name    The name to search for.
     * \return  The attribute if found, otherwise an invalid attribute.
     **/
    const AttributeEntry& getAttribute(const QString& name) const;

    /**
     * \brief   Removes an attribute by name.
     * \param   name    The name to remove.
     * \return  True if the attribute was removed, false otherwise.
     **/
    bool removeAttribute(const QString& name);

    /**
     * \brief   Adds an attribute to the list and sorts the list.
     * \param   entry   The attribute to add.
     * \param   unique  If true, the entry is unique.
     **/
    bool addAttribute(AttributeEntry&& entry, bool unique);

    /**
     * \brief   Sorts the list of attributes.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortAttributes(bool ascending = true);

    /**
     * \brief   Clears the list of attributes.
     **/
    void removeAll(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QList<AttributeEntry> mAttributes; //!< The list of attributes.
};

#endif  // LUSAN_DATA_SI_SIATTRIBUTEDATA_HPP
