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
 *  \file        lusan/model/sm/SMAttributeModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Attributes page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMAttributeModel.hpp"

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

SMAttributeModel::SMAttributeModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<SMAttributeEntry>& SMAttributeModel::getAttributes() const
{
    return attributes().getElements();
}

int SMAttributeModel::getAttributeCount() const
{
    return attributes().getElementCount();
}

SMAttributeEntry* SMAttributeModel::findAttribute(const QString& name) const
{
    return attributes().findElement(name);
}

SMAttributeEntry* SMAttributeModel::findAttribute(uint32_t id) const
{
    return attributes().findElement(id);
}

int SMAttributeModel::findIndex(uint32_t id) const
{
    return attributes().findIndex(id);
}

SMDataTypeModel& SMAttributeModel::getDataTypeModel() const
{
    return mFacade.getDataTypeModel();
}

DocModelNotifier& SMAttributeModel::getNotifier() const
{
    return mFacade.getNotifier();
}

SMAttributeEntry* SMAttributeModel::createAttribute(const QString& name)
{
    if (findAttribute(name) != nullptr)
        return nullptr;

    SMAttributeEntry entry(0, name, QStringLiteral("bool"), QString());
    // Resolve the type pointer before the command's redo() fires the notifier — a page
    // rebuilds its row synchronously inside push(), before this function regains control.
    entry.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(new TDocAddCommand<SMAttributeEntry, DocumentElem>(getNotifier(), attributes(), std::move(entry), eDocElementKind::Attribute, QObject::tr("Add attribute")));
    return findAttribute(name);
}

SMAttributeEntry* SMAttributeModel::insertAttribute(int position, const QString& name)
{
    if (findAttribute(name) != nullptr)
        return nullptr;

    SMAttributeEntry entry(0, name, QStringLiteral("bool"), QString());
    entry.validate(getDataTypeModel().getCustomDataTypes());
    mFacade.getUndoStack().push(buildInsertCommand<SMAttributeEntry, DocumentElem>(getNotifier(), attributes(), std::move(entry), position, 0u, eDocElementKind::Attribute, QObject::tr("Insert attribute")));
    return findAttribute(name);
}

void SMAttributeModel::deleteAttribute(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<SMAttributeEntry, DocumentElem>(getNotifier(), attributes(), id, eDocElementKind::Attribute, QObject::tr("Delete attribute")));
}

void SMAttributeModel::swapAttributes(uint32_t firstId, uint32_t secondId)
{
    const int index1 = attributes().findIndex(firstId);
    const int index2 = attributes().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<SMAttributeEntry, DocumentElem>(getNotifier(), attributes(), index1, index2, 0u, eDocElementKind::Attribute, QObject::tr("Reorder attributes")));
}

void SMAttributeModel::renameAttribute(uint32_t id, const QString& newName)
{
    SMAttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [facade, id](const QString& value) { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); if (e != nullptr) e->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, newName, QObject::tr("Rename attribute")));
}

void SMAttributeModel::setType(uint32_t id, const QString& typeName)
{
    SMAttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (typeName == entry->getType()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); return (e != nullptr ? e->getType() : QString()); };
    auto setter = [facade, id](const QString& value)
    {
        SMAttributeEntry* e = facade->getData().getAttributes().findElement(id);
        if (e != nullptr) { e->setType(value); e->validate(facade->getDataTypeModel().getCustomDataTypes()); }
    };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, typeName, QObject::tr("Set attribute type")));
}

void SMAttributeModel::setValue(uint32_t id, const QString& value)
{
    SMAttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (value == entry->getValue()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); return (e != nullptr ? e->getValue() : QString()); };
    auto setter = [facade, id](const QString& val) { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); if (e != nullptr) e->setValue(val); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, value, QObject::tr("Set attribute value")));
}

void SMAttributeModel::setDescription(uint32_t id, const QString& text)
{
    SMAttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); if (e != nullptr) e->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, text, QObject::tr("Set description")));
}

void SMAttributeModel::setDeprecated(uint32_t id, bool deprecated)
{
    SMAttributeEntry* entry = findAttribute(id);
    if (entry == nullptr)
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> DeprecationState
    {
        SMAttributeEntry* e = facade->getData().getAttributes().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [facade, id](const DeprecationState& value)
    {
        SMAttributeEntry* e = facade->getData().getAttributes().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, next, QObject::tr("Set deprecated")));
}

void SMAttributeModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    SMAttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [facade, id](const QString& value) { SMAttributeEntry* e = facade->getData().getAttributes().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

const SMAttributeData& SMAttributeModel::attributes() const
{
    return mFacade.getData().getAttributes();
}

SMAttributeData& SMAttributeModel::attributes()
{
    return mFacade.getData().getAttributes();
}
