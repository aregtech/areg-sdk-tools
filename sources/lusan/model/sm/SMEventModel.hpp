#ifndef LUSAN_MODEL_SM_SMEVENTMODEL_HPP
#define LUSAN_MODEL_SM_SMEVENTMODEL_HPP
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
 *  \file        lusan/model/sm/SMEventModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMEventData.hpp"
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
 * \class   SMEventModel
 * \brief   The Events page model. Reads the live `EventList` section through the facade and
 *          routes every edit through an undo command, so the page never mutates
 *          `SMEventEntry`/`MethodParameter` directly.
 *
 *          `SMEventEntry` is stored by owning pointer in its section (address stable across
 *          container growth), so event-level commands may capture the `SMEventEntry*`
 *          directly. Its payload parameters (`MethodParameter`) are stored by value inside
 *          the event, so parameter-level commands identify a parameter by ID and re-resolve
 *          it inside the command's getter/setter — the same rule
 *          `SMDataTypeModel` follows for structure fields.
 **/
class SMEventModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMEventModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads — events
//////////////////////////////////////////////////////////////////////////
public:
    const QList<SMEventEntry*>& getEvents() const;
    int getEventCount() const;

    SMEventEntry* findEvent(const QString& name) const;
    SMEventEntry* findEvent(uint32_t id) const;
    int findIndex(uint32_t id) const;
    int findIndex(const SMEventEntry* event) const;

    //!< The data types page model, so the parameter type editor can offer declared types.
    SMDataTypeModel& getDataTypeModel() const;
    DocModelNotifier& getNotifier() const;

    //!< Resolves a name in the document-wide stimulus name space (spec 6.10) so the page
    //!< can flag a collision as the user types, without waiting for commit.
    StateMachineData::StimulusRef findStimulus(const QString& name) const;

//////////////////////////////////////////////////////////////////////////
// Reads — payload parameters
//////////////////////////////////////////////////////////////////////////
public:
    const QList<MethodParameter>& getParams(const SMEventEntry* event) const;
    int getParamCount(const SMEventEntry* event) const;
    MethodParameter* findParam(const SMEventEntry* event, uint32_t paramId) const;
    MethodParameter* findParam(const SMEventEntry* event, const QString& name) const;
    int findParamIndex(const SMEventEntry* event, uint32_t paramId) const;

//////////////////////////////////////////////////////////////////////////
// Mutations — events
//////////////////////////////////////////////////////////////////////////
public:
    SMEventEntry* createEvent(const QString& name);
    SMEventEntry* insertEvent(int position, const QString& name);
    void deleteEvent(uint32_t id);
    void swapEvents(uint32_t firstId, uint32_t secondId);

    void renameEvent(uint32_t id, const QString& newName);
    void setDescription(uint32_t id, const QString& text);

//////////////////////////////////////////////////////////////////////////
// Mutations — payload parameters
//////////////////////////////////////////////////////////////////////////
public:
    MethodParameter* createParam(SMEventEntry* event, const QString& name);
    MethodParameter* insertParam(SMEventEntry* event, int position, const QString& name);
    void deleteParam(SMEventEntry* event, uint32_t paramId);
    void swapParams(SMEventEntry* event, uint32_t firstId, uint32_t secondId);

    void setParamName(SMEventEntry* event, uint32_t paramId, const QString& name);
    //!< Sets the parameter's declared type by name — see SMDataTypeModel::setFieldType for
    //!< why name, not the resolved DataTypeBase*.
    void setParamType(SMEventEntry* event, uint32_t paramId, const QString& typeName);
    //!< Sets the optional default flag and its literal as one undo step (one user gesture).
    void setParamDefault(SMEventEntry* event, uint32_t paramId, bool hasDefault, const QString& value);
    void setParamDescription(SMEventEntry* event, uint32_t paramId, const QString& text);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMEventData& events() const;
    SMEventData& events();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMEVENTMODEL_HPP
