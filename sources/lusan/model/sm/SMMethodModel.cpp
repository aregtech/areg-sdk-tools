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
 *  \file        lusan/model/sm/SMMethodModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMMethodModel.hpp"

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

SMMethodModel::SMMethodModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<SMMethodEntry*>& SMMethodModel::getMethods() const
{
    return methods().getElements();
}

int SMMethodModel::getMethodCount() const
{
    return methods().getElementCount();
}

SMMethodEntry* SMMethodModel::findMethod(const QString& name) const
{
    return methods().findMethod(name);
}

SMMethodEntry* SMMethodModel::findMethod(uint32_t id) const
{
    return methods().findMethod(id);
}

int SMMethodModel::findIndex(uint32_t id) const
{
    return methods().findIndex(id);
}

int SMMethodModel::findIndex(const SMMethodEntry* method) const
{
    return (method != nullptr ? methods().findIndex(method->getId()) : -1);
}

SMDataTypeModel& SMMethodModel::getDataTypeModel() const
{
    return mFacade.getDataTypeModel();
}

DocModelNotifier& SMMethodModel::getNotifier() const
{
    return mFacade.getNotifier();
}

const StateMachineData& SMMethodModel::getData() const
{
    return mFacade.getData();
}

SMSelectionModel& SMMethodModel::getSelectionModel() const
{
    return mFacade.getSelectionModel();
}

StateMachineData::StimulusRef SMMethodModel::findStimulus(const QString& name) const
{
    return mFacade.getData().findStimulus(name);
}

const QList<MethodParameter>& SMMethodModel::getParams(const SMMethodEntry* method) const
{
    static const QList<MethodParameter> _empty;
    return (method != nullptr ? method->getElements() : _empty);
}

int SMMethodModel::getParamCount(const SMMethodEntry* method) const
{
    return (method != nullptr ? method->getElementCount() : 0);
}

MethodParameter* SMMethodModel::findParam(const SMMethodEntry* method, uint32_t paramId) const
{
    return (method != nullptr ? const_cast<SMMethodEntry*>(method)->findElement(paramId) : nullptr);
}

MethodParameter* SMMethodModel::findParam(const SMMethodEntry* method, const QString& name) const
{
    return (method != nullptr ? const_cast<SMMethodEntry*>(method)->findElement(name) : nullptr);
}

int SMMethodModel::findParamIndex(const SMMethodEntry* method, uint32_t paramId) const
{
    return (method != nullptr ? method->findIndex(paramId) : -1);
}

SMMethodEntry* SMMethodModel::createMethod(const QString& name, SMMethodEntry::eMethodType type)
{
    if (findMethod(name) != nullptr)
        return nullptr;

    SMMethodEntry* entry = new SMMethodEntry(0, name, type);
    mFacade.getUndoStack().push(new TDocAddCommand<SMMethodEntry*, DocumentElem>(getNotifier(), methods(), entry, eDocElementKind::Method, QObject::tr("Add method")));
    return findMethod(name);
}

SMMethodEntry* SMMethodModel::insertMethod(int position, const QString& name, SMMethodEntry::eMethodType type)
{
    if (findMethod(name) != nullptr)
        return nullptr;

    SMMethodEntry* entry = new SMMethodEntry(0, name, type);
    mFacade.getUndoStack().push(buildInsertCommand<SMMethodEntry*, DocumentElem>(getNotifier(), methods(), entry, position, 0u, eDocElementKind::Method, QObject::tr("Insert method")));
    return findMethod(name);
}

void SMMethodModel::deleteMethod(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<SMMethodEntry*, DocumentElem>(getNotifier(), methods(), id, eDocElementKind::Method, QObject::tr("Delete method")));
}

void SMMethodModel::swapMethods(uint32_t firstId, uint32_t secondId)
{
    const int index1 = methods().findIndex(firstId);
    const int index2 = methods().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<SMMethodEntry*, DocumentElem>(getNotifier(), methods(), index1, index2, 0u, eDocElementKind::Method, QObject::tr("Reorder methods")));
}

void SMMethodModel::renameMethod(uint32_t id, const QString& newName)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getName() : QString()); };
    auto setter = [facade, id](const QString& value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, newName, QObject::tr("Rename method")));
}

void SMMethodModel::setMethodType(uint32_t id, SMMethodEntry::eMethodType type)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (type == entry->getMethodType()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> SMMethodEntry::eMethodType { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getMethodType() : SMMethodEntry::eMethodType::Trigger); };
    auto setter = [facade, id](SMMethodEntry::eMethodType value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setMethodType(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<SMMethodEntry::eMethodType>(getNotifier(), id, eDocElementKind::Method, getter, setter, type, QObject::tr("Set method type")));
}

void SMMethodModel::setReturn(uint32_t id, const QString& typeName)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (typeName == entry->getReturn()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getReturn() : QString()); };
    auto setter = [facade, id](const QString& value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setReturn(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, typeName, QObject::tr("Set return type")));
}

void SMMethodModel::setImplement(uint32_t id, SMMethodEntry::eImplement implement)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (implement == entry->getImplement()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> SMMethodEntry::eImplement { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getImplement() : SMMethodEntry::eImplement::Handler); };
    auto setter = [facade, id](SMMethodEntry::eImplement value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setImplement(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<SMMethodEntry::eImplement>(getNotifier(), id, eDocElementKind::Method, getter, setter, implement, QObject::tr("Set implementation")));
}

void SMMethodModel::setBody(uint32_t id, const QString& body)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (body == entry->getBody()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getBody() : QString()); };
    auto setter = [facade, id](const QString& value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setBody(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, body, QObject::tr("Set method body")));
}

void SMMethodModel::setDescription(uint32_t id, const QString& text)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, text, QObject::tr("Set description")));
}

void SMMethodModel::setDeprecated(uint32_t id, bool deprecated)
{
    SMMethodEntry* entry = findMethod(id);
    if (entry == nullptr)
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> DeprecationState
    {
        SMMethodEntry* m = facade->getData().getMethods().findMethod(id);
        return (m != nullptr ? DeprecationState{ m->getIsDeprecated(), m->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [facade, id](const DeprecationState& value)
    {
        SMMethodEntry* m = facade->getData().getMethods().findMethod(id);
        if (m != nullptr) { m->setIsDeprecated(value.flag); m->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Method, getter, setter, next, QObject::tr("Set deprecated")));
}

void SMMethodModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    SMMethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); return (m != nullptr ? m->getDeprecateHint() : QString()); };
    auto setter = [facade, id](const QString& value) { SMMethodEntry* m = facade->getData().getMethods().findMethod(id); if (m != nullptr) m->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

MethodParameter* SMMethodModel::createParam(SMMethodEntry* method, const QString& name)
{
    if ((method == nullptr) || (method->findElement(name) != nullptr))
        return nullptr;

    MethodParameter param(0, name, QStringLiteral("bool"), QString(), false, nullptr);
    param.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(new TDocAddCommand<MethodParameter, DocumentElem>(getNotifier(), *method, param, eDocElementKind::Method, QObject::tr("Add parameter")));
    return method->findElement(name);
}

MethodParameter* SMMethodModel::insertParam(SMMethodEntry* method, int position, const QString& name)
{
    if ((method == nullptr) || (method->findElement(name) != nullptr))
        return nullptr;

    MethodParameter param(0, name, QStringLiteral("bool"), QString(), false, nullptr);
    param.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(buildInsertCommand<MethodParameter, DocumentElem>(getNotifier(), *method, param, position, method->getId(), eDocElementKind::Method, QObject::tr("Insert parameter")));
    return method->findElement(name);
}

void SMMethodModel::deleteParam(SMMethodEntry* method, uint32_t paramId)
{
    if (method == nullptr)
        return;

    mFacade.getUndoStack().push(new TDocRemoveCommand<MethodParameter, DocumentElem>(getNotifier(), *method, paramId, eDocElementKind::Method, QObject::tr("Delete parameter")));
}

void SMMethodModel::swapParams(SMMethodEntry* method, uint32_t firstId, uint32_t secondId)
{
    if (method == nullptr)
        return;

    const int index1 = method->findIndex(firstId);
    const int index2 = method->findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<MethodParameter, DocumentElem>(getNotifier(), *method, index1, index2, method->getId(), eDocElementKind::Method, QObject::tr("Reorder parameters")));
}

void SMMethodModel::setParamName(SMMethodEntry* method, uint32_t paramId, const QString& name)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getName() : QString()); };
    auto setter = [method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) p->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, name, QObject::tr("Rename parameter")));
}

void SMMethodModel::setParamType(SMMethodEntry* method, uint32_t paramId, const QString& typeName)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getType() : QString()); };
    auto setter = [this, method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) { p->setType(value); p->validate(getDataTypeModel().getCustomDataTypes()); } };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, typeName, QObject::tr("Set parameter type")));
}

void SMMethodModel::setParamDefault(SMMethodEntry* method, uint32_t paramId, bool hasDefault, const QString& value)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> ParamDefaultState
    {
        MethodParameter* p = method->findElement(paramId);
        return (p != nullptr ? ParamDefaultState{ p->hasDefault(), p->getValue() } : ParamDefaultState{});
    };
    auto setter = [method, paramId](const ParamDefaultState& state)
    {
        MethodParameter* p = method->findElement(paramId);
        if (p != nullptr) { p->setDefault(state.hasDefault); p->setValue(state.hasDefault ? state.value : QString()); }
    };
    const ParamDefaultState next{ hasDefault, hasDefault ? value : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<ParamDefaultState>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, next, QObject::tr("Set parameter default")));
}

void SMMethodModel::setParamDescription(SMMethodEntry* method, uint32_t paramId, const QString& text)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getDescription() : QString()); };
    auto setter = [method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) p->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, text, QObject::tr("Set parameter description")));
}

void SMMethodModel::setParamDeprecated(SMMethodEntry* method, uint32_t paramId, bool deprecated)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> DeprecationState
    {
        MethodParameter* p = method->findElement(paramId);
        return (p != nullptr ? DeprecationState{ p->getIsDeprecated(), p->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [method, paramId](const DeprecationState& value)
    {
        MethodParameter* p = method->findElement(paramId);
        if (p != nullptr) { p->setIsDeprecated(value.flag); p->setDeprecateHint(value.hint); }
    };
    MethodParameter* param = method->findElement(paramId);
    const DeprecationState next{ deprecated, (deprecated && (param != nullptr)) ? param->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, next, QObject::tr("Set parameter deprecated")));
}

void SMMethodModel::setParamDeprecateHint(SMMethodEntry* method, uint32_t paramId, const QString& hint)
{
    if (method == nullptr)
        return;

    MethodParameter* param = method->findElement(paramId);
    if ((param == nullptr) || (param->getIsDeprecated() == false) || (hint == param->getDeprecateHint()))
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getDeprecateHint() : QString()); };
    auto setter = [method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) p->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, hint, QObject::tr("Set parameter deprecation hint")));
}

const SMMethodData& SMMethodModel::methods() const
{
    return mFacade.getData().getMethods();
}

SMMethodData& SMMethodModel::methods()
{
    return mFacade.getData().getMethods();
}
