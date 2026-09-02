#ifndef LUSAN_MODEL_COMMON_CONSTANTMODEL_HPP
#define LUSAN_MODEL_COMMON_CONSTANTMODEL_HPP
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
 *  \file        lusan/model/common/ConstantModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Constants page model shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/ConstantDataSection.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeBase;

/**
 * \class   ConstantModel
 * \brief   The Constants page model. Reads the document's `ConstantList` section and routes
 *          every edit through an undo command, so a page never mutates a `ConstantEntry`
 *          directly. An entry is stored by value in its section, so the mutators identify it
 *          by ID and re-resolve it inside the command's getter and setter rather than
 *          capturing a pointer that a sibling insert or remove could invalidate.
 *
 *          The model knows only the document interface, so the same class serves the service
 *          interface, the state machine and a standalone data type document. The section itself
 *          is asked for on every access: a document that opens or reloads a file swaps the data
 *          object underneath, and a section reference kept from construction would dangle.
 **/
class ConstantModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit ConstantModel(IEDocumentModel& document);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<ConstantEntry>& getConstants() const;
    int getConstantCount() const;

    ConstantEntry* findConstant(const QString& name) const;
    ConstantEntry* findConstant(uint32_t id) const;
    int findIndex(uint32_t id) const;

    DocModelNotifier& getNotifier() const;
    IEDocumentModel& getDocument() const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    ConstantEntry* createConstant(const QString& name);
    ConstantEntry* insertConstant(int position, const QString& name);
    void deleteConstant(uint32_t id);
    /**
     * rief   Moves the constant one position up or down and answers the ID it carries
     *          afterwards. A reorder leaves the moved element with the ID of the element it
     *          passed, so a caller keeps the selection on what moved by taking this answer.
     * \param   id      The ID of the constant to move.
     * \param   delta   -1 to move one position up, +1 to move one position down.
     * eturn  The ID the moved constant carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveConstant(uint32_t id, int delta);

    void renameConstant(uint32_t id, const QString& newName);

    /**
     * \brief   Sets the constant's declared type by name. The name, not the resolved pointer,
     *          is what the command records, so undo and redo survive a data type being
     *          replaced or reloaded underneath.
     **/
    void setType(uint32_t id, const QString& typeName);
    void setValue(uint32_t id, const QString& value);
    void setDescription(uint32_t id, const QString& text);
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

    /**
     * \brief   Repoints every constant that uses the old data type to the new one.
     *          Called when a data type is converted or removed, not by a page edit, so it
     *          writes through the section and reports what changed.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    inline ConstantDataSection& section() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;
};

//////////////////////////////////////////////////////////////////////////
// ConstantModel inline methods
//////////////////////////////////////////////////////////////////////////

inline ConstantDataSection& ConstantModel::section() const
{
    return mDocument.getConstantSection();
}

#endif  // LUSAN_MODEL_COMMON_CONSTANTMODEL_HPP
