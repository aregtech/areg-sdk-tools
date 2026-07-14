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
 *  \file        lusan/model/sm/SMConstantModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMConstantModel.hpp"

#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/common/DocElementCommands.hpp"

namespace
{
    //!< Deprecation flag + hint committed as one undo step, matching the single user gesture.
    struct DeprecationState
    {
        bool    flag { false };
        QString hint { };
    };
}

SMConstantModel::SMConstantModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<ConstantEntry>& SMConstantModel::getConstants() const
{
    return constants().getElements();
}

int SMConstantModel::getConstantCount() const
{
    return constants().getElementCount();
}

ConstantEntry* SMConstantModel::findConstant(const QString& name) const
{
    return constants().findElement(name);
}

ConstantEntry* SMConstantModel::findConstant(uint32_t id) const
{
    return constants().findElement(id);
}

int SMConstantModel::findIndex(uint32_t id) const
{
    return constants().findIndex(id);
}

SMDataTypeModel& SMConstantModel::getDataTypeModel() const
{
    return mFacade.getDataTypeModel();
}

DocModelNotifier& SMConstantModel::getNotifier() const
{
    return mFacade.getNotifier();
}

ConstantEntry* SMConstantModel::createConstant(const QString& name)
{
    if (findConstant(name) != nullptr)
        return nullptr;

    ConstantEntry entry(0, name, QStringLiteral("bool"), QString());
    // Resolve the type pointer before the command's redo() fires the notifier — a page
    // rebuilds its row synchronously inside push(), before this function regains control.
    entry.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(new TDocAddCommand<ConstantEntry, DocumentElem>(getNotifier(), constants(), std::move(entry), eDocElementKind::Constant, QObject::tr("Add constant")));
    return findConstant(name);
}

ConstantEntry* SMConstantModel::insertConstant(int position, const QString& name)
{
    if (findConstant(name) != nullptr)
        return nullptr;

    ConstantEntry entry(0, name, QStringLiteral("bool"), QString());
    entry.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(buildInsertCommand<ConstantEntry, DocumentElem>(getNotifier(), constants(), std::move(entry), position, 0u, eDocElementKind::Constant, QObject::tr("Insert constant")));
    return findConstant(name);
}

void SMConstantModel::deleteConstant(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<ConstantEntry, DocumentElem>(getNotifier(), constants(), id, eDocElementKind::Constant, QObject::tr("Delete constant")));
}

void SMConstantModel::swapConstants(uint32_t firstId, uint32_t secondId)
{
    const int index1 = constants().findIndex(firstId);
    const int index2 = constants().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<ConstantEntry, DocumentElem>(getNotifier(), constants(), index1, index2, 0u, eDocElementKind::Constant, QObject::tr("Reorder constants")));
}

void SMConstantModel::renameConstant(uint32_t id, const QString& newName)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { ConstantEntry* e = facade->getData().getConstants().findElement(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [facade, id](const QString& value) { ConstantEntry* e = facade->getData().getConstants().findElement(id); if (e != nullptr) e->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, newName, QObject::tr("Rename constant")));
}

void SMConstantModel::setType(uint32_t id, const QString& typeName)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (typeName == entry->getType()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { ConstantEntry* e = facade->getData().getConstants().findElement(id); return (e != nullptr ? e->getType() : QString()); };
    auto setter = [facade, id](const QString& value)
    {
        ConstantEntry* e = facade->getData().getConstants().findElement(id);
        if (e != nullptr) { e->setType(value); e->validate(facade->getDataTypeModel().getCustomDataTypes()); }
    };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, typeName, QObject::tr("Set constant type")));
}

void SMConstantModel::setValue(uint32_t id, const QString& value)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (value == entry->getValue()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { ConstantEntry* e = facade->getData().getConstants().findElement(id); return (e != nullptr ? e->getValue() : QString()); };
    auto setter = [facade, id](const QString& val) { ConstantEntry* e = facade->getData().getConstants().findElement(id); if (e != nullptr) e->setValue(val); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, value, QObject::tr("Set constant value")));
}

void SMConstantModel::setDescription(uint32_t id, const QString& text)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { ConstantEntry* e = facade->getData().getConstants().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { ConstantEntry* e = facade->getData().getConstants().findElement(id); if (e != nullptr) e->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, text, QObject::tr("Set description")));
}

void SMConstantModel::setDeprecated(uint32_t id, bool deprecated)
{
    ConstantEntry* entry = findConstant(id);
    if (entry == nullptr)
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> DeprecationState
    {
        ConstantEntry* e = facade->getData().getConstants().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [facade, id](const DeprecationState& value)
    {
        ConstantEntry* e = facade->getData().getConstants().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Constant, getter, setter, next, QObject::tr("Set deprecated")));
}

void SMConstantModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { ConstantEntry* e = facade->getData().getConstants().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [facade, id](const QString& value) { ConstantEntry* e = facade->getData().getConstants().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

const SMConstantData& SMConstantModel::constants() const
{
    return mFacade.getData().getConstants();
}

SMConstantData& SMConstantModel::constants()
{
    return mFacade.getData().getConstants();
}
