#ifndef LUSAN_MODEL_SM_SMMETHODMODEL_HPP
#define LUSAN_MODEL_SM_SMMETHODMODEL_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMMethodModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class SMDataTypeModel;
class DocModelNotifier;

/**
 * \class   SMMethodModel
 * \brief   The Methods page model. Reads the live `MethodList` section through the facade and
 *          routes every edit through an undo command, so the page never mutates
 *          `SMMethodEntry`/`MethodParameter` directly.
 *
 *          `SMMethodEntry` is stored by owning pointer in its section (address stable across
 *          container growth), so method-level commands may capture the `SMMethodEntry*`
 *          directly. Its parameters (`MethodParameter`) are stored by value inside the
 *          method, so parameter-level commands identify a parameter by ID and re-resolve it
 *          inside the command's getter/setter, the same rule the Events and Data Types page
 *          models follow.
 **/
class SMMethodModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMMethodModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads — methods
//////////////////////////////////////////////////////////////////////////
public:
    const QList<SMMethodEntry*>& getMethods() const;
    int getMethodCount() const;

    SMMethodEntry* findMethod(const QString& name) const;
    SMMethodEntry* findMethod(uint32_t id) const;
    int findIndex(uint32_t id) const;
    int findIndex(const SMMethodEntry* method) const;

    //!< The data types page model, so the parameter/return type editors can offer declared types.
    SMDataTypeModel& getDataTypeModel() const;
    DocModelNotifier& getNotifier() const;

    //!< Resolves a name in the document-wide stimulus name space (spec 6.10) so the page can
    //!< flag a trigger-method collision as the user types, without waiting for commit.
    StateMachineData::StimulusRef findStimulus(const QString& name) const;

//////////////////////////////////////////////////////////////////////////
// Reads — parameters
//////////////////////////////////////////////////////////////////////////
public:
    const QList<MethodParameter>& getParams(const SMMethodEntry* method) const;
    int getParamCount(const SMMethodEntry* method) const;
    MethodParameter* findParam(const SMMethodEntry* method, uint32_t paramId) const;
    MethodParameter* findParam(const SMMethodEntry* method, const QString& name) const;
    int findParamIndex(const SMMethodEntry* method, uint32_t paramId) const;

//////////////////////////////////////////////////////////////////////////
// Mutations — methods
//////////////////////////////////////////////////////////////////////////
public:
    SMMethodEntry* createMethod(const QString& name, SMMethodEntry::eMethodType type);
    SMMethodEntry* insertMethod(int position, const QString& name, SMMethodEntry::eMethodType type);
    void deleteMethod(uint32_t id);
    void swapMethods(uint32_t firstId, uint32_t secondId);

    void renameMethod(uint32_t id, const QString& newName);
    void setMethodType(uint32_t id, SMMethodEntry::eMethodType type);
    void setReturn(uint32_t id, const QString& typeName);
    void setImplement(uint32_t id, SMMethodEntry::eImplement implement);
    void setBody(uint32_t id, const QString& body);
    void setDescription(uint32_t id, const QString& text);
    //!< Sets the deprecated flag (and clears the hint when cleared) as one undo step.
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Mutations — parameters
//////////////////////////////////////////////////////////////////////////
public:
    MethodParameter* createParam(SMMethodEntry* method, const QString& name);
    MethodParameter* insertParam(SMMethodEntry* method, int position, const QString& name);
    void deleteParam(SMMethodEntry* method, uint32_t paramId);
    void swapParams(SMMethodEntry* method, uint32_t firstId, uint32_t secondId);

    void setParamName(SMMethodEntry* method, uint32_t paramId, const QString& name);
    //!< Sets the parameter's declared type by name — see SMDataTypeModel::setFieldType for
    //!< why name, not the resolved DataTypeBase*.
    void setParamType(SMMethodEntry* method, uint32_t paramId, const QString& typeName);
    //!< Sets the optional default flag and its literal as one undo step (one user gesture).
    void setParamDefault(SMMethodEntry* method, uint32_t paramId, bool hasDefault, const QString& value);
    void setParamDescription(SMMethodEntry* method, uint32_t paramId, const QString& text);
    //!< Sets the parameter deprecated flag (and clears the hint when cleared) as one undo step.
    void setParamDeprecated(SMMethodEntry* method, uint32_t paramId, bool deprecated);
    void setParamDeprecateHint(SMMethodEntry* method, uint32_t paramId, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMMethodData& methods() const;
    SMMethodData& methods();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMMETHODMODEL_HPP
