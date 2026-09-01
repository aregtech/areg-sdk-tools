#ifndef LUSAN_MODEL_COMMON_METHODMODEL_HPP
#define LUSAN_MODEL_COMMON_METHODMODEL_HPP
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
 *  \file        lusan/model/common/MethodModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Methods page model shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/MethodDataSection.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeCustom;

/**
 * \class   MethodModel
 * \brief   The Methods page model. Reads the live `MethodList` section through the document
 *          facade and routes every edit through an undo command, so the page never mutates a
 *          \ref MethodEntry or a \ref MethodParameter itself.
 *
 *          An entry is stored by owning pointer in its section, so its address survives the
 *          container growing and a method-level command may capture the pointer. Its parameters
 *          are stored by value inside the method, so a parameter-level command identifies the
 *          parameter by ID and re-resolves it inside its own getter and setter.
 *
 *          The section is asked for on every access rather than held: a document that opens or
 *          reloads a file replaces the data object the section lives in, and a reference taken
 *          at construction time would outlive it.
 **/
class MethodModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit MethodModel(IEDocumentModel& document);

//////////////////////////////////////////////////////////////////////////
// Reads -- methods
//////////////////////////////////////////////////////////////////////////
public:
    const QList<MethodEntry*>& getMethods() const;
    int getMethodCount() const;

    MethodEntry* findMethod(const QString& name) const;
    MethodEntry* findMethod(uint32_t id) const;
    MethodEntry* findMethod(const QString& name, int kind) const;
    QList<MethodEntry*> methodsOfKind(int kind) const;

    int findIndex(uint32_t id) const;
    int findIndex(const MethodEntry* method) const;

    //!< What this document's methods are: the kinds it offers and what each of them carries.
    const MethodConfig& getConfig() const;

    //!< The custom data types a declared parameter or return type resolves against.
    const QList<DataTypeCustom*>& getCustomDataTypes() const;

    DocModelNotifier& getNotifier() const;

    //!< The document this model edits, for a page that needs more of it than this model offers.
    inline IEDocumentModel& getDocument() const;

//////////////////////////////////////////////////////////////////////////
// Reads -- parameters
//////////////////////////////////////////////////////////////////////////
public:
    const QList<MethodParameter>& getParams(const MethodEntry* method) const;
    int getParamCount(const MethodEntry* method) const;
    MethodParameter* findParam(const MethodEntry* method, uint32_t paramId) const;
    MethodParameter* findParam(const MethodEntry* method, const QString& name) const;
    int findParamIndex(const MethodEntry* method, uint32_t paramId) const;

//////////////////////////////////////////////////////////////////////////
// Mutations -- methods
//////////////////////////////////////////////////////////////////////////
public:
    MethodEntry* createMethod(const QString& name, int kind);
    MethodEntry* insertMethod(int position, const QString& name, int kind);
    void deleteMethod(uint32_t id);
    /**
     * rief   Moves the method one position up or down and answers the ID it carries
     *          afterwards. A reorder leaves the moved element with the ID of the element it
     *          passed, so a caller keeps the selection on what moved by taking this answer.
     * \param   id      The ID of the method to move.
     * \param   delta   -1 to move one position up, +1 to move one position down.
     * eturn  The ID the moved method carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveMethod(uint32_t id, int delta);

    void renameMethod(uint32_t id, const QString& newName);
    void setKind(uint32_t id, int kind);
    void setReply(uint32_t id, const QString& replyName);
    void setReturn(uint32_t id, const QString& typeName);
    void setImplement(uint32_t id, MethodEntry::eImplement implement);
    void setBody(uint32_t id, const QString& body);
    void setDescription(uint32_t id, const QString& text);
    //!< Sets the deprecated flag (and clears the hint when cleared) as one undo step.
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

    /**
     * \brief   Clears the reply of every method that names the given one. A method that answers
     *          others is deleted or renamed through this, so no request is left pointing at a
     *          name that is gone.
     * \param   replyName   The name that is going away.
     * \param   parent      The composite command to attach the clearing to.
     **/
    void clearRepliesTo(const QString& replyName, QUndoCommand* parent);

//////////////////////////////////////////////////////////////////////////
// Mutations -- parameters
//////////////////////////////////////////////////////////////////////////
public:
    MethodParameter* createParam(MethodEntry* method, const QString& name);
    MethodParameter* insertParam(MethodEntry* method, int position, const QString& name);
    void deleteParam(MethodEntry* method, uint32_t paramId);
    /**
     * \brief   Moves a parameter of the given method one position up or down and answers the ID
     *          it carries afterwards. Refuses a move that would put a parameter carrying a
     *          default value in front of one that carries none, the way C++ does.
     * \param   method  The method owning the parameter.
     * \param   paramId The ID of the parameter to move.
     * \param   delta   -1 to move one position up, +1 to move one position down.
     * \return  The ID the moved parameter carries afterwards, or 0 when nothing moved.
     **/
    uint32_t moveParam(MethodEntry* method, uint32_t paramId, int delta);

    void setParamName(MethodEntry* method, uint32_t paramId, const QString& name);
    //!< Sets the parameter's declared type by name, never by the resolved pointer: the pointer
    //!< a command captured may be gone by the time the command is redone.
    void setParamType(MethodEntry* method, uint32_t paramId, const QString& typeName);
    //!< Sets the default flag and its literal as one undo step, matching the one user gesture.
    void setParamDefault(MethodEntry* method, uint32_t paramId, bool hasDefault, const QString& value);
    void setParamDescription(MethodEntry* method, uint32_t paramId, const QString& text);
    //!< Sets the parameter deprecated flag (and clears the hint when cleared) as one undo step.
    void setParamDeprecated(MethodEntry* method, uint32_t paramId, bool deprecated);
    void setParamDeprecateHint(MethodEntry* method, uint32_t paramId, const QString& hint);

    /**
     * \brief   Re-resolves every parameter's declared type against what the document holds now.
     *          A declared type is a name plus a lazily resolved pointer, and the pointer a
     *          category conversion left behind is not the object that came back.
     **/
    void resolveDeclaredTypes();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const MethodDataSection& methods() const;
    MethodDataSection& methods();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IEDocumentModel&    mDocument;  //!< The document being edited.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    MethodModel(const MethodModel& /*src*/) = delete;
    MethodModel& operator = (const MethodModel& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// MethodModel inline methods
//////////////////////////////////////////////////////////////////////////

inline IEDocumentModel& MethodModel::getDocument() const
{
    return mDocument;
}

#endif  // LUSAN_MODEL_COMMON_METHODMODEL_HPP
