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
 *  \file        lusan/model/common/ConstantModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Constants page model shared by every document editor.
 *
 ************************************************************************/

#include "lusan/model/common/ConstantModel.hpp"

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

ConstantModel::ConstantModel(IEDocumentModel& document)
    : mDocument (document)
{
}

const QList<ConstantEntry>& ConstantModel::getConstants() const
{
    return section().getElements();
}

int ConstantModel::getConstantCount() const
{
    return section().getElementCount();
}

ConstantEntry* ConstantModel::findConstant(const QString& name) const
{
    return section().findElement(name);
}

ConstantEntry* ConstantModel::findConstant(uint32_t id) const
{
    return section().findElement(id);
}

int ConstantModel::findIndex(uint32_t id) const
{
    return section().findIndex(id);
}

DocModelNotifier& ConstantModel::getNotifier() const
{
    return mDocument.getNotifier();
}

IEDocumentModel& ConstantModel::getDocument() const
{
    return mDocument;
}

ConstantEntry* ConstantModel::createConstant(const QString& name)
{
    if (findConstant(name) != nullptr)
        return nullptr;

    ConstantEntry entry(0, name, QStringLiteral("bool"), QString());
    // Resolve the type pointer before the command's redo() fires the notifier -- a page
    // rebuilds its row synchronously inside push(), before this function regains control.
    entry.validate(mDocument.getCustomDataTypes());
    mDocument.getUndoStack().push(new TDocAddCommand<ConstantEntry, DocumentElem>(getNotifier(), section(), std::move(entry), eDocElementKind::Constant, QObject::tr("Add constant")));
    return findConstant(name);
}

ConstantEntry* ConstantModel::insertConstant(int position, const QString& name)
{
    if (findConstant(name) != nullptr)
        return nullptr;

    ConstantEntry entry(0, name, QStringLiteral("bool"), QString());
    entry.validate(mDocument.getCustomDataTypes());
    mDocument.getUndoStack().push(buildInsertCommand<ConstantEntry, DocumentElem>(getNotifier(), section(), std::move(entry), position, 0u, eDocElementKind::Constant, QObject::tr("Insert constant")));
    return findConstant(name);
}

void ConstantModel::deleteConstant(uint32_t id)
{
    mDocument.getUndoStack().push(new TDocRemoveCommand<ConstantEntry, DocumentElem>(getNotifier(), section(), id, eDocElementKind::Constant, QObject::tr("Delete constant")));
}

uint32_t ConstantModel::moveConstant(uint32_t id, int delta)
{
    return docMoveElement<ConstantEntry, DocumentElem>(mDocument.getUndoStack(), getNotifier(), section(), id, delta, 0u, eDocElementKind::Constant, QObject::tr("Reorder constants"));
}

void ConstantModel::renameConstant(uint32_t id, const QString& newName)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    const QString oldName{ entry->getName() };
    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { ConstantEntry* e = document->getConstantSection().findElement(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [document, id](const QString& value) { ConstantEntry* e = document->getConstantSection().findElement(id); if (e != nullptr) e->setName(value); };

    const QString text{ QObject::tr("Rename constant") };
    DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
    new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, newName, text, composite);
    // A document that reaches its constants by name repairs those references in the same step.
    mDocument.createRenameSideEffects(eDocElementKind::Constant, id, oldName, newName, composite);
    mDocument.getUndoStack().push(composite);
}

void ConstantModel::setType(uint32_t id, const QString& typeName)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (typeName == entry->getType()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { ConstantEntry* e = document->getConstantSection().findElement(id); return (e != nullptr ? e->getType() : QString()); };
    auto setter = [document, id](const QString& value)
    {
        ConstantEntry* e = document->getConstantSection().findElement(id);
        if (e != nullptr) { e->setType(value); e->validate(document->getCustomDataTypes()); }
    };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, typeName, QObject::tr("Set constant type")));
}

void ConstantModel::setValue(uint32_t id, const QString& value)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (value == entry->getValue()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { ConstantEntry* e = document->getConstantSection().findElement(id); return (e != nullptr ? e->getValue() : QString()); };
    auto setter = [document, id](const QString& val) { ConstantEntry* e = document->getConstantSection().findElement(id); if (e != nullptr) e->setValue(val); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, value, QObject::tr("Set constant value")));
}

void ConstantModel::setDescription(uint32_t id, const QString& text)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { ConstantEntry* e = document->getConstantSection().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [document, id](const QString& value) { ConstantEntry* e = document->getConstantSection().findElement(id); if (e != nullptr) e->setDescription(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, text, QObject::tr("Set description")));
}

void ConstantModel::setDeprecated(uint32_t id, bool deprecated)
{
    ConstantEntry* entry = findConstant(id);
    if (entry == nullptr)
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> DeprecationState
    {
        ConstantEntry* e = document->getConstantSection().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [document, id](const DeprecationState& value)
    {
        ConstantEntry* e = document->getConstantSection().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Constant, getter, setter, next, QObject::tr("Set deprecated")));
}

void ConstantModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    ConstantEntry* entry = findConstant(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { ConstantEntry* e = document->getConstantSection().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [document, id](const QString& value) { ConstantEntry* e = document->getConstantSection().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Constant, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

QList<uint32_t> ConstantModel::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    return section().replaceDataType(oldDataType, newDataType);
}
