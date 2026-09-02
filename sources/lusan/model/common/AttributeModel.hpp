#ifndef LUSAN_MODEL_COMMON_ATTRIBUTEMODEL_HPP
#define LUSAN_MODEL_COMMON_ATTRIBUTEMODEL_HPP
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
 *  \file        lusan/model/common/AttributeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Attributes page model shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/AttributeDataSection.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeBase;

/**
 * \class   AttributeModel
 * \brief   The Attributes page model. Reads the document's `AttributeList` section and routes
 *          every edit through an undo command, so a page never mutates an `AttributeEntry`
 *          directly. An entry is stored by value in its section, so the mutators identify it by
 *          ID and re-resolve it inside the command's getter and setter rather than capturing a
 *          pointer that a sibling insert or remove could invalidate.
 *
 *          The model knows only the document interface, so the same class serves the service
 *          interface, the state machine and a standalone data type document. The section itself
 *          is asked for on every access: a document that opens or reloads a file swaps the data
 *          object underneath, and a section reference kept from construction would dangle.
 **/
class AttributeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit AttributeModel(IEDocumentModel& document);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<AttributeEntry>& getAttributes() const;
    int getAttributeCount() const;

    AttributeEntry* findAttribute(const QString& name) const;
    AttributeEntry* findAttribute(uint32_t id) const;
    int findIndex(uint32_t id) const;

    DocModelNotifier& getNotifier() const;
    IEDocumentModel& getDocument() const;

    //!< What this document's attributes carry besides name, type and description.
    const AttributeConfig& getConfig() const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    AttributeEntry* createAttribute(const QString& name);
    AttributeEntry* insertAttribute(int position, const QString& name);
    void deleteAttribute(uint32_t id);
    /**
     * rief   Moves the attribute one position up or down and answers the ID it carries
     *          afterwards. A reorder leaves the moved element with the ID of the element it
     *          passed, so a caller keeps the selection on what moved by taking this answer.
     * \param   id      The ID of the attribute to move.
     * \param   delta   -1 to move one position up, +1 to move one position down.
     * eturn  The ID the moved attribute carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveAttribute(uint32_t id, int delta);

    void renameAttribute(uint32_t id, const QString& newName);

    /**
     * \brief   Sets the attribute's declared type by name. The name, not the resolved pointer, is
     *          what the command records, so undo and redo survive a data type being replaced or
     *          reloaded underneath.
     **/
    void setType(uint32_t id, const QString& typeName);
    void setValue(uint32_t id, const QString& value);
    void setNotification(uint32_t id, AttributeEntry::eNotification notification);
    void setDescription(uint32_t id, const QString& text);
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

    /**
     * \brief   Repoints every attribute that uses the old data type to the new one. Called when a
     *          data type is converted or removed, not by a page edit, so it writes through the
     *          section and reports what changed.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

    /**
     * \brief   Drops every attribute's resolved type pointer and looks it up again by name against
     *          the document's current data types. A type that was removed, renamed or converted
     *          leaves that pointer wrong; a name with nothing behind it stays as typed. Not an
     *          edit, so it pushes no command.
     **/
    void resolveDeclaredTypes();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    inline AttributeDataSection& section() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;
};

//////////////////////////////////////////////////////////////////////////
// AttributeModel inline methods
//////////////////////////////////////////////////////////////////////////

inline AttributeDataSection& AttributeModel::section() const
{
    return mDocument.getAttributeSection();
}

#endif  // LUSAN_MODEL_COMMON_ATTRIBUTEMODEL_HPP
