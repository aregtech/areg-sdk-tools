#ifndef LUSAN_MODEL_COMMON_INCLUDEMODEL_HPP
#define LUSAN_MODEL_COMMON_INCLUDEMODEL_HPP
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
 *  \file        lusan/model/common/IncludeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Includes page model shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/IncludeDataSection.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/**
 * \class   IncludeModel
 * \brief   The Includes page model. Reads the document's `IncludeList` section and routes every
 *          edit through an undo command, so a page never mutates an `IncludeEntry` directly. An
 *          entry is stored by value in its section, so the mutators identify it by ID and
 *          re-resolve it inside the command's getter and setter rather than capturing a pointer
 *          that a sibling insert or remove could invalidate.
 *
 *          An include is identified by its location, which is also its unique name.
 *
 *          The model knows only the document interface, so the same class serves the service
 *          interface, the state machine and a standalone data type document. The section itself
 *          is asked for on every access: a document that opens or reloads a file swaps the data
 *          object underneath, and a section reference kept from construction would dangle.
 *
 *          A document that gives a row extra meaning subclasses this: see \ref kindOfLocation
 *          and \ref prepareNewEntry.
 **/
class IncludeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit IncludeModel(IEDocumentModel& document);

    virtual ~IncludeModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<IncludeEntry>& getIncludes(void) const;
    int getIncludeCount(void) const;

    IncludeEntry* findInclude(const QString& location) const;
    IncludeEntry* findInclude(uint32_t id) const;
    int findIndex(uint32_t id) const;

    DocModelNotifier& getNotifier(void) const;
    IEDocumentModel& getDocument(void) const;

    /**
     * \brief   True while the document refuses every change. The base document is always
     *          writable; a read-only view says otherwise.
     **/
    virtual bool isReadOnly(void) const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Registers an include at the end of the list.
     * \param   location    The file the document pulls in; its unique name.
     * \return  The created entry, or nullptr when the location is already registered.
     **/
    IncludeEntry* createInclude(const QString& location);

    /**
     * \brief   Registers an include at the given position.
     * \param   position    The position to insert at.
     * \param   location    The file the document pulls in; its unique name.
     * \return  The created entry, or nullptr when the location is already registered.
     **/
    IncludeEntry* insertInclude(int position, const QString& location);

    void deleteInclude(uint32_t id);
    /**
     * rief   Moves the include one position up or down and answers the ID it carries
     *          afterwards. A reorder leaves the moved element with the ID of the element it
     *          passed, so a caller keeps the selection on what moved by taking this answer.
     * \param   id      The ID of the include to move.
     * \param   delta   -1 to move one position up, +1 to move one position down.
     * eturn  The ID the moved include carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveInclude(uint32_t id, int delta);

    //!< The location is the include's unique name, so this is both the rename and the path edit.
    void setLocation(uint32_t id, const QString& location);
    void setDescription(uint32_t id, const QString& text);
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   The notifier kind a row of the given location announces itself under. Everything
     *          is an ordinary include unless the document says the file means more to it.
     **/
    virtual eDocElementKind kindOfLocation(const QString& location) const;

    /**
     * \brief   Last chance to fill in what the document derives from a new row's location,
     *          before the add command takes the entry over.
     **/
    virtual void prepareNewEntry(IncludeEntry& entry) const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The notifier kind of the row with the given ID.
    eDocElementKind kindOfId(uint32_t id) const;

    inline IncludeDataSection& section(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;
};

//////////////////////////////////////////////////////////////////////////
// IncludeModel inline methods
//////////////////////////////////////////////////////////////////////////

inline IncludeDataSection& IncludeModel::section(void) const
{
    return mDocument.getIncludeSection();
}

#endif  // LUSAN_MODEL_COMMON_INCLUDEMODEL_HPP
