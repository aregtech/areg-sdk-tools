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
 *  \file        lusan/model/sm/SMEventModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMEventModel.hpp"

#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/common/DocElementCommands.hpp"

namespace
{
    //!< The default flag + literal committed as one undo step, matching the single user
    //!< gesture (a "Default:" check-box next to the value field).
    struct ParamDefaultState
    {
        bool    hasDefault { false };
        QString value      { };
    };

    //!< Deprecation flag + hint committed as one undo step, matching the single user gesture.
    struct DeprecationState
    {
        bool    flag { false };
        QString hint { };
    };
}

SMEventModel::SMEventModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<SMEventEntry*>& SMEventModel::getEvents() const
{
    return events().getElements();
}

int SMEventModel::getEventCount() const
{
    return events().getElementCount();
}

SMEventEntry* SMEventModel::findEvent(const QString& name) const
{
    return events().findEvent(name);
}

SMEventEntry* SMEventModel::findEvent(uint32_t id) const
{
    return events().findEvent(id);
}

int SMEventModel::findIndex(uint32_t id) const
{
    return events().findIndex(id);
}

int SMEventModel::findIndex(const SMEventEntry* event) const
{
    return (event != nullptr ? events().findIndex(event->getId()) : -1);
}

SMDataTypeModel& SMEventModel::getDataTypeModel() const
{
    return mFacade.getDataTypeModel();
}

DocModelNotifier& SMEventModel::getNotifier() const
{
    return mFacade.getNotifier();
}

StateMachineData::StimulusRef SMEventModel::findStimulus(const QString& name) const
{
    return mFacade.getData().findStimulus(name);
}

const QList<MethodParameter>& SMEventModel::getParams(const SMEventEntry* event) const
{
    static const QList<MethodParameter> _empty;
    return (event != nullptr ? event->getElements() : _empty);
}

int SMEventModel::getParamCount(const SMEventEntry* event) const
{
    return (event != nullptr ? event->getElementCount() : 0);
}

MethodParameter* SMEventModel::findParam(const SMEventEntry* event, uint32_t paramId) const
{
    return (event != nullptr ? const_cast<SMEventEntry*>(event)->findElement(paramId) : nullptr);
}

MethodParameter* SMEventModel::findParam(const SMEventEntry* event, const QString& name) const
{
    return (event != nullptr ? const_cast<SMEventEntry*>(event)->findElement(name) : nullptr);
}

int SMEventModel::findParamIndex(const SMEventEntry* event, uint32_t paramId) const
{
    return (event != nullptr ? event->findIndex(paramId) : -1);
}

SMEventEntry* SMEventModel::createEvent(const QString& name)
{
    if (findEvent(name) != nullptr)
        return nullptr;

    SMEventEntry* entry = new SMEventEntry(0, name);
    mFacade.getUndoStack().push(new TDocAddCommand<SMEventEntry*, DocumentElem>(getNotifier(), events(), entry, eDocElementKind::Event, QObject::tr("Add event")));
    return findEvent(name);
}

SMEventEntry* SMEventModel::insertEvent(int position, const QString& name)
{
    if (findEvent(name) != nullptr)
        return nullptr;

    SMEventEntry* entry = new SMEventEntry(0, name);
    mFacade.getUndoStack().push(buildInsertCommand<SMEventEntry*, DocumentElem>(getNotifier(), events(), entry, position, 0u, eDocElementKind::Event, QObject::tr("Insert event")));
    return findEvent(name);
}

void SMEventModel::deleteEvent(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<SMEventEntry*, DocumentElem>(getNotifier(), events(), id, eDocElementKind::Event, QObject::tr("Delete event")));
}

void SMEventModel::swapEvents(uint32_t firstId, uint32_t secondId)
{
    const int index1 = events().findIndex(firstId);
    const int index2 = events().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<SMEventEntry*, DocumentElem>(getNotifier(), events(), index1, index2, 0u, eDocElementKind::Event, QObject::tr("Reorder events")));
}

void SMEventModel::renameEvent(uint32_t id, const QString& newName)
{
    SMEventEntry* entry = findEvent(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMEventEntry* e = facade->getData().getEvents().findEvent(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [facade, id](const QString& value) { SMEventEntry* e = facade->getData().getEvents().findEvent(id); if (e != nullptr) e->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Event, getter, setter, newName, QObject::tr("Rename event")));
}

void SMEventModel::setDescription(uint32_t id, const QString& text)
{
    SMEventEntry* entry = findEvent(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMEventEntry* e = facade->getData().getEvents().findEvent(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { SMEventEntry* e = facade->getData().getEvents().findEvent(id); if (e != nullptr) e->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Event, getter, setter, text, QObject::tr("Set description")));
}

void SMEventModel::setDeprecated(uint32_t id, bool deprecated)
{
    SMEventEntry* entry = findEvent(id);
    if (entry == nullptr)
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> DeprecationState
    {
        SMEventEntry* e = facade->getData().getEvents().findEvent(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [facade, id](const DeprecationState& value)
    {
        SMEventEntry* e = facade->getData().getEvents().findEvent(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Event, getter, setter, next, QObject::tr("Set deprecated")));
}

void SMEventModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    SMEventEntry* entry = findEvent(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMEventEntry* e = facade->getData().getEvents().findEvent(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [facade, id](const QString& value) { SMEventEntry* e = facade->getData().getEvents().findEvent(id); if (e != nullptr) e->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Event, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

MethodParameter* SMEventModel::createParam(SMEventEntry* event, const QString& name)
{
    if ((event == nullptr) || (event->findElement(name) != nullptr))
        return nullptr;

    MethodParameter param(0, name, QStringLiteral("bool"), QString(), false, nullptr);
    param.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(new TDocAddCommand<MethodParameter, DocumentElem>(getNotifier(), *event, param, eDocElementKind::Event, QObject::tr("Add parameter")));
    return event->findElement(name);
}

MethodParameter* SMEventModel::insertParam(SMEventEntry* event, int position, const QString& name)
{
    if ((event == nullptr) || (event->findElement(name) != nullptr))
        return nullptr;

    MethodParameter param(0, name, QStringLiteral("bool"), QString(), false, nullptr);
    param.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(buildInsertCommand<MethodParameter, DocumentElem>(getNotifier(), *event, param, position, event->getId(), eDocElementKind::Event, QObject::tr("Insert parameter")));
    return event->findElement(name);
}

void SMEventModel::deleteParam(SMEventEntry* event, uint32_t paramId)
{
    if (event == nullptr)
        return;

    mFacade.getUndoStack().push(new TDocRemoveCommand<MethodParameter, DocumentElem>(getNotifier(), *event, paramId, eDocElementKind::Event, QObject::tr("Delete parameter")));
}

void SMEventModel::swapParams(SMEventEntry* event, uint32_t firstId, uint32_t secondId)
{
    if (event == nullptr)
        return;

    const int index1 = event->findIndex(firstId);
    const int index2 = event->findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<MethodParameter, DocumentElem>(getNotifier(), *event, index1, index2, event->getId(), eDocElementKind::Event, QObject::tr("Reorder parameters")));
}

void SMEventModel::setParamName(SMEventEntry* event, uint32_t paramId, const QString& name)
{
    if (event == nullptr)
        return;

    const uint32_t ownerId = event->getId();
    auto getter = [event, paramId]() -> QString { MethodParameter* p = event->findElement(paramId); return (p != nullptr ? p->getName() : QString()); };
    auto setter = [event, paramId](const QString& value) { MethodParameter* p = event->findElement(paramId); if (p != nullptr) p->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Event, getter, setter, name, QObject::tr("Rename parameter")));
}

void SMEventModel::setParamType(SMEventEntry* event, uint32_t paramId, const QString& typeName)
{
    if (event == nullptr)
        return;

    const uint32_t ownerId = event->getId();
    auto getter = [event, paramId]() -> QString { MethodParameter* p = event->findElement(paramId); return (p != nullptr ? p->getType() : QString()); };
    auto setter = [this, event, paramId](const QString& value) { MethodParameter* p = event->findElement(paramId); if (p != nullptr) { p->setType(value); p->validate(getDataTypeModel().getCustomDataTypes()); } };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Event, getter, setter, typeName, QObject::tr("Set parameter type")));
}

void SMEventModel::setParamDefault(SMEventEntry* event, uint32_t paramId, bool hasDefault, const QString& value)
{
    if (event == nullptr)
        return;

    const uint32_t ownerId = event->getId();
    auto getter = [event, paramId]() -> ParamDefaultState
    {
        MethodParameter* p = event->findElement(paramId);
        return (p != nullptr ? ParamDefaultState{ p->hasDefault(), p->getValue() } : ParamDefaultState{});
    };
    auto setter = [event, paramId](const ParamDefaultState& state)
    {
        MethodParameter* p = event->findElement(paramId);
        if (p != nullptr) { p->setDefault(state.hasDefault); p->setValue(state.hasDefault ? state.value : QString()); }
    };
    const ParamDefaultState next{ hasDefault, hasDefault ? value : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<ParamDefaultState>(getNotifier(), ownerId, eDocElementKind::Event, getter, setter, next, QObject::tr("Set parameter default")));
}

void SMEventModel::setParamDescription(SMEventEntry* event, uint32_t paramId, const QString& text)
{
    if (event == nullptr)
        return;

    const uint32_t ownerId = event->getId();
    auto getter = [event, paramId]() -> QString { MethodParameter* p = event->findElement(paramId); return (p != nullptr ? p->getDescription() : QString()); };
    auto setter = [event, paramId](const QString& value) { MethodParameter* p = event->findElement(paramId); if (p != nullptr) p->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Event, getter, setter, text, QObject::tr("Set parameter description")));
}

void SMEventModel::setParamDeprecated(SMEventEntry* event, uint32_t paramId, bool deprecated)
{
    if (event == nullptr)
        return;

    const uint32_t ownerId = event->getId();
    auto getter = [event, paramId]() -> DeprecationState
    {
        MethodParameter* p = event->findElement(paramId);
        return (p != nullptr ? DeprecationState{ p->getIsDeprecated(), p->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [event, paramId](const DeprecationState& value)
    {
        MethodParameter* p = event->findElement(paramId);
        if (p != nullptr) { p->setIsDeprecated(value.flag); p->setDeprecateHint(value.hint); }
    };
    MethodParameter* param = event->findElement(paramId);
    const DeprecationState next{ deprecated, (deprecated && (param != nullptr)) ? param->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), ownerId, eDocElementKind::Event, getter, setter, next, QObject::tr("Set parameter deprecated")));
}

void SMEventModel::setParamDeprecateHint(SMEventEntry* event, uint32_t paramId, const QString& hint)
{
    if (event == nullptr)
        return;

    MethodParameter* param = event->findElement(paramId);
    if ((param == nullptr) || (param->getIsDeprecated() == false) || (hint == param->getDeprecateHint()))
        return;

    const uint32_t ownerId = event->getId();
    auto getter = [event, paramId]() -> QString { MethodParameter* p = event->findElement(paramId); return (p != nullptr ? p->getDeprecateHint() : QString()); };
    auto setter = [event, paramId](const QString& value) { MethodParameter* p = event->findElement(paramId); if (p != nullptr) p->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Event, getter, setter, hint, QObject::tr("Set parameter deprecation hint")));
}

const SMEventData& SMEventModel::events() const
{
    return mFacade.getData().getEvents();
}

SMEventData& SMEventModel::events()
{
    return mFacade.getData().getEvents();
}
