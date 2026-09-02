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
 *  \file        lusan/model/common/AttributeModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Attributes page model shared by every document editor.
 *
 ************************************************************************/

#include "lusan/model/common/AttributeModel.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/model/common/DocCommand.hpp"
#include "lusan/model/common/DocElementCommands.hpp"

#include <QObject>

namespace
{
    //!< Deprecation flag and hint committed as one undo step, matching the single user gesture.
    struct DeprecationState
    {
        bool    flag { false };
        QString hint { };
    };
}

AttributeModel::AttributeModel(IEDocumentModel& document)
    : mDocument (document)
{
}

const QList<AttributeEntry>& AttributeModel::getAttributes() const
{
    return section().getElements();
}

int AttributeModel::getAttributeCount() const
{
    return section().getElementCount();
}

AttributeEntry* AttributeModel::findAttribute(const QString& name) const
{
    return section().findElement(name);
}

AttributeEntry* AttributeModel::findAttribute(uint32_t id) const
{
    return section().findElement(id);
}

int AttributeModel::findIndex(uint32_t id) const
{
    return section().findIndex(id);
}

DocModelNotifier& AttributeModel::getNotifier() const
{
    return mDocument.getNotifier();
}

IEDocumentModel& AttributeModel::getDocument() const
{
    return mDocument;
}

const AttributeConfig& AttributeModel::getConfig() const
{
    return section().getConfig();
}

AttributeEntry* AttributeModel::createAttribute(const QString& name)
{
    if (findAttribute(name) != nullptr)
        return nullptr;

    AttributeEntry entry(0, name, AttributeEntry::eNotification::NotifyOnChange, nullptr);
    entry.setConfig(section().getConfig());
    // Resolve the type pointer before the command's redo() fires the notifier: a page rebuilds
    // its row synchronously inside push(), before this function regains control.
    entry.validate(mDocument.getCustomDataTypes());
    mDocument.getUndoStack().push(new TDocAddCommand<AttributeEntry, DocumentElem>(getNotifier(), section(), std::move(entry), eDocElementKind::Attribute, QObject::tr("Add attribute")));
    return findAttribute(name);
}

AttributeEntry* AttributeModel::insertAttribute(int position, const QString& name)
{
    if (findAttribute(name) != nullptr)
        return nullptr;

    AttributeEntry entry(0, name, AttributeEntry::eNotification::NotifyOnChange, nullptr);
    entry.setConfig(section().getConfig());
    entry.validate(mDocument.getCustomDataTypes());
    mDocument.getUndoStack().push(buildInsertCommand<AttributeEntry, DocumentElem>(getNotifier(), section(), std::move(entry), position, 0u, eDocElementKind::Attribute, QObject::tr("Insert attribute")));
    return findAttribute(name);
}

void AttributeModel::deleteAttribute(uint32_t id)
{
    mDocument.getUndoStack().push(new TDocRemoveCommand<AttributeEntry, DocumentElem>(getNotifier(), section(), id, eDocElementKind::Attribute, QObject::tr("Delete attribute")));
}

uint32_t AttributeModel::moveAttribute(uint32_t id, int delta)
{
    return docMoveElement<AttributeEntry, DocumentElem>(mDocument.getUndoStack(), getNotifier(), section(), id, delta, 0u, eDocElementKind::Attribute, QObject::tr("Reorder attributes"));
}

void AttributeModel::renameAttribute(uint32_t id, const QString& newName)
{
    AttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    const QString oldName{ entry->getName() };
    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { AttributeEntry* e = document->getAttributeSection().findElement(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [document, id](const QString& value) { AttributeEntry* e = document->getAttributeSection().findElement(id); if (e != nullptr) e->setName(value); };

    const QString text{ QObject::tr("Rename attribute") };
    DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
    new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, newName, text, composite);
    // A document that reaches its attributes by name repairs those references in the same step.
    mDocument.createRenameSideEffects(eDocElementKind::Attribute, id, oldName, newName, composite);
    mDocument.getUndoStack().push(composite);
}

void AttributeModel::setType(uint32_t id, const QString& typeName)
{
    AttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (typeName == entry->getType()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { AttributeEntry* e = document->getAttributeSection().findElement(id); return (e != nullptr ? e->getType() : QString()); };
    auto setter = [document, id](const QString& value)
    {
        AttributeEntry* e = document->getAttributeSection().findElement(id);
        if (e != nullptr) { e->setType(value); e->validate(document->getCustomDataTypes()); }
    };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, typeName, QObject::tr("Set attribute type")));
}

void AttributeModel::setValue(uint32_t id, const QString& value)
{
    AttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (value == entry->getValue()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { AttributeEntry* e = document->getAttributeSection().findElement(id); return (e != nullptr ? e->getValue() : QString()); };
    auto setter = [document, id](const QString& val) { AttributeEntry* e = document->getAttributeSection().findElement(id); if (e != nullptr) e->setValue(val); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, value, QObject::tr("Set attribute value")));
}

void AttributeModel::setNotification(uint32_t id, AttributeEntry::eNotification notification)
{
    AttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (notification == entry->getNotification()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> AttributeEntry::eNotification
    {
        AttributeEntry* e = document->getAttributeSection().findElement(id);
        return (e != nullptr ? e->getNotification() : AttributeEntry::eNotification::NotifyOnChange);
    };
    auto setter = [document, id](AttributeEntry::eNotification value)
    {
        AttributeEntry* e = document->getAttributeSection().findElement(id);
        if (e != nullptr) e->setNotification(value);
    };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<AttributeEntry::eNotification>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, notification, QObject::tr("Set attribute notification")));
}

void AttributeModel::setDescription(uint32_t id, const QString& text)
{
    AttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { AttributeEntry* e = document->getAttributeSection().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [document, id](const QString& value) { AttributeEntry* e = document->getAttributeSection().findElement(id); if (e != nullptr) e->setDescription(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, text, QObject::tr("Set description")));
}

void AttributeModel::setDeprecated(uint32_t id, bool deprecated)
{
    AttributeEntry* entry = findAttribute(id);
    if (entry == nullptr)
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> DeprecationState
    {
        AttributeEntry* e = document->getAttributeSection().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [document, id](const DeprecationState& value)
    {
        AttributeEntry* e = document->getAttributeSection().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, next, QObject::tr("Set deprecated")));
}

void AttributeModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    AttributeEntry* entry = findAttribute(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { AttributeEntry* e = document->getAttributeSection().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [document, id](const QString& value) { AttributeEntry* e = document->getAttributeSection().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Attribute, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

QList<uint32_t> AttributeModel::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    return section().replaceDataType(oldDataType, newDataType);
}

void AttributeModel::resolveDeclaredTypes()
{
    const QList<DataTypeCustom*>& customTypes = mDocument.getCustomDataTypes();
    for (AttributeEntry& entry : section().getElements())
    {
        entry.invalidate();
        entry.validate(customTypes);
    }
}
