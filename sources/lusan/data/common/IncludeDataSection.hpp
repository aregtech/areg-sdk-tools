#ifndef LUSAN_DATA_COMMON_INCLUDEDATASECTION_HPP
#define LUSAN_DATA_COMMON_INCLUDEDATASECTION_HPP
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
 *  \file        lusan/data/common/IncludeDataSection.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the `IncludeList` section shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \class   IncludeDataSection
 * \brief   The `IncludeList` section: everything a document pulls in from outside, in document
 *          order. A row is identified by its location, which is also its unique name.
 *
 *          The section stores rows and reads and writes them; what a location points at, and
 *          whether that file resolves, belongs to the document that owns the section.
 **/
class IncludeDataSection : public TEDataContainer<IncludeEntry, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    IncludeDataSection(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of include entries.
     * \param   parent      The parent element.
     **/
    IncludeDataSection(const QList<IncludeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isValid() const override;

    /**
     * \brief   Reads the `IncludeList` element and every `Location` row in it.
     * \param   xml     The XML stream reader, positioned on the section element.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes the `IncludeList` element, or nothing when the section is empty.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Appends an include row.
     * \param   location    The file the document pulls in; its unique name.
     * \return  The created entry, or nullptr when the location is already registered.
     **/
    IncludeEntry* createInclude(const QString& location);

    /**
     * \brief   Inserts an include row at the given position.
     * \param   position    The position to insert at.
     * \param   location    The file the document pulls in; its unique name.
     * \return  The created entry, or nullptr when the location is already registered.
     **/
    IncludeEntry* insertInclude(int position, const QString& location);
};

#endif  // LUSAN_DATA_COMMON_INCLUDEDATASECTION_HPP
