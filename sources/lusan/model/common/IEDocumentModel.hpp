#ifndef LUSAN_MODEL_COMMON_IEDOCUMENTMODEL_HPP
#define LUSAN_MODEL_COMMON_IEDOCUMENTMODEL_HPP
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
 *  \file        lusan/model/common/IEDocumentModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the document surface the shared page models edit through.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocUndoStack.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class AttributeDataSection;
class ConstantDataSection;
class DataTypeCustom;
class DataTypeDataSection;
class DocValidationController;
class IncludeDataSection;
class MethodDataSection;
class QUndoCommand;

/**
 * \class   IEDocumentModel
 * \brief   Everything a shared page model needs from the document that owns it, and nothing
 *          more: where to push a command, where change notifications come from, and which
 *          custom data types a declared type name may resolve against.
 *
 *          A page model holds this interface plus a reference to its own section container, so
 *          it never learns whether it is editing a service interface, a state machine or a
 *          standalone data type document.
 **/
class IEDocumentModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    IEDocumentModel() = default;

public:
    virtual ~IEDocumentModel() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The document's change notifier. Commands are its only emitters.
     **/
    virtual DocModelNotifier& getNotifier() = 0;

    /**
     * \brief   The document's undo stack. Every mutation reaches the data through it.
     **/
    virtual DocUndoStack& getUndoStack() = 0;

    /**
     * \brief   The custom data types a declared type name resolves against.
     **/
    virtual const QList<DataTypeCustom*>& getCustomDataTypes() const = 0;

    /**
     * \brief   The document's `ConstantList` section. Answered on every call rather than handed
     *          out once, because a document that opens or reloads a file replaces the data object
     *          the section lives in, and a reference taken earlier would outlive it.
     **/
    virtual ConstantDataSection& getConstantSection() = 0;

    /**
     * \brief   The document's `DataTypeList` section. Answered on every call, for the same
     *          reason as the constants section above.
     **/
    virtual DataTypeDataSection& getDataTypeSection() = 0;

    /**
     * \brief   The document's `IncludeList` section. Answered on every call, for the same
     *          reason as the constants section above.
     **/
    virtual IncludeDataSection& getIncludeSection() = 0;

    /**
     * \brief   The document's `AttributeList` section. Answered on every call, for the same
     *          reason as the constants section above.
     **/
    virtual AttributeDataSection& getAttributeSection() = 0;

    /**
     * \brief   The document's `MethodList` section. Answered on every call, for the same
     *          reason as the constants section above.
     **/
    virtual MethodDataSection& getMethodSection() = 0;

    /**
     * \brief   The document's validation scheduler, which owns its findings and re-runs the
     *          checks in the background as the document changes.
     **/
    virtual DocValidationController& getValidationController() = 0;

    /**
     * \brief   Names the element a finding blames, in the document's own words, so a results row
     *          says where before it says what. Answer an empty string when the element's kind is
     *          the whole answer -- a registry entry whose message already quotes its name needs
     *          nothing finer, while a state or a transition is worth naming.
     * \param   id      The unique ID of the element.
     * \param   kind    The kind of the element.
     **/
    virtual QString describeElement(uint32_t /*id*/, eDocElementKind /*kind*/) const
    {
        return QString();
    }

    /**
     * \brief   Builds the command that repairs whatever else in the document refers to a renamed
     *          element by name, to be run inside the same undo step as the rename itself.
     *          A document with no name-based references returns nullptr, which is the default.
     * \param   kind        The kind of element being renamed.
     * \param   id          The unique ID of the renamed element.
     * \param   oldName     The name before the rename.
     * \param   newName     The name after the rename.
     * \param   parent      The composite command the result must attach to.
     * \return  The side-effect command, or nullptr when the document keeps no such references.
     **/
    virtual QUndoCommand* createRenameSideEffects( eDocElementKind /*kind*/, uint32_t /*id*/
                                                 , const QString& /*oldName*/, const QString& /*newName*/
                                                 , QUndoCommand* /*parent*/)
    {
        return nullptr;
    }
};

#endif  // LUSAN_MODEL_COMMON_IEDOCUMENTMODEL_HPP
