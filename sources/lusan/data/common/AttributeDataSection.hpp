#ifndef LUSAN_DATA_COMMON_ATTRIBUTEDATASECTION_HPP
#define LUSAN_DATA_COMMON_ATTRIBUTEDATASECTION_HPP
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
 *  \file        lusan/data/common/AttributeDataSection.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the `AttributeList` section shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeBase;
class DataTypeDataSection;

namespace NEAttribute
{
    //!< A service interface attribute is published to subscribers, so it says when they hear
    //!< about a change. It has no default value: the service sets it at run time.
    constexpr AttributeConfig ServiceInterface { false, true };

    //!< A state machine attribute is an internal variable, so it carries the value it starts
    //!< with. Nothing subscribes to it, so there is nothing to notify.
    constexpr AttributeConfig StateMachine { true, false };
}

/**
 * \class   AttributeDataSection
 * \brief   The `AttributeList` section: the document's named typed attributes, in document order.
 *          A row is identified by its name.
 *
 *          The section is told once what its document's attributes carry -- a default value, an
 *          update notification kind, or both -- and stamps every entry it creates or reads with
 *          it, so an entry writes the shape its document expects wherever it is serialized.
 **/
class AttributeDataSection : public TEDataContainer<AttributeEntry, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   config      What the document's attributes carry besides name, type and description.
     * \param   parent      The parent element.
     **/
    AttributeDataSection(const AttributeConfig& config, ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   config      What the document's attributes carry.
     * \param   entries     The list of attribute entries.
     * \param   parent      The parent element.
     **/
    AttributeDataSection(const AttributeConfig& config, const QList<AttributeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isValid() const override;

    /**
     * \brief   Reads the `AttributeList` element and every `Attribute` row in it.
     * \param   xml     The XML stream reader, positioned on the section element.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes the `AttributeList` element, or nothing when the section is empty.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns what this document's attributes carry.
     **/
    inline const AttributeConfig& getConfig() const;

    /**
     * \brief   Appends an attribute row.
     * \param   name    The unique name of the new attribute.
     * \return  The created entry, or nullptr when the name is already taken.
     **/
    AttributeEntry* createAttribute(const QString& name);

    /**
     * \brief   Inserts an attribute row at the given position.
     * \param   position    The position to insert at.
     * \param   name        The unique name of the new attribute.
     * \return  The created entry, or nullptr when the name is already taken.
     **/
    AttributeEntry* insertAttribute(int position, const QString& name);

    /**
     * \brief   Resolves every attribute's declared type against the document's data types, so the
     *          type-based icons and checks reflect the current registry right after a file load.
     *          A freshly parsed entry only holds the type name.
     * \param   dataTypes   The document's data type registry.
     **/
    void validate(const DataTypeDataSection& dataTypes);

    /**
     * \brief   Repoints every attribute declared with the old data type to the new one.
     * \return  The IDs of the attributes that changed.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    AttributeConfig mConfig;    //!< What this document's attributes carry.
};

//////////////////////////////////////////////////////////////////////////
// AttributeDataSection inline methods
//////////////////////////////////////////////////////////////////////////

inline const AttributeConfig& AttributeDataSection::getConfig() const
{
    return mConfig;
}

#endif  // LUSAN_DATA_COMMON_ATTRIBUTEDATASECTION_HPP
