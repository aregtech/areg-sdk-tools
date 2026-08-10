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
 *  \file        lusan/model/common/MethodModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Methods page model shared by every document editor.
 *
 ************************************************************************/

#include "lusan/model/common/MethodModel.hpp"

#include "lusan/model/common/DocElementCommands.hpp"

namespace
{
    //!< The default flag and its literal committed as one undo step, matching the one user
    //!< gesture: a "Default:" check-box next to the value field.
    struct ParamDefaultState
    {
        bool    hasDefault { false };
        QString value      { };
    };

    //!< Deprecation flag and hint committed as one undo step, matching the one user gesture.
    struct DeprecationState
    {
        bool    flag { false };
        QString hint { };
    };
}

MethodModel::MethodModel(IEDocumentModel& document)
    : mDocument(document)
{
}

//////////////////////////////////////////////////////////////////////////
// Reads -- methods
//////////////////////////////////////////////////////////////////////////

const QList<MethodEntry*>& MethodModel::getMethods() const
{
    return methods().getElements();
}

int MethodModel::getMethodCount() const
{
    return methods().getElementCount();
}

MethodEntry* MethodModel::findMethod(const QString& name) const
{
    return methods().findMethod(name);
}

MethodEntry* MethodModel::findMethod(uint32_t id) const
{
    return methods().findMethod(id);
}

MethodEntry* MethodModel::findMethod(const QString& name, int kind) const
{
    return methods().findMethod(name, kind);
}

QList<MethodEntry*> MethodModel::methodsOfKind(int kind) const
{
    return methods().methodsOfKind(kind);
}

int MethodModel::findIndex(uint32_t id) const
{
    return methods().findIndex(id);
}

int MethodModel::findIndex(const MethodEntry* method) const
{
    return (method != nullptr ? methods().findIndex(method->getId()) : -1);
}

const MethodConfig& MethodModel::getConfig() const
{
    return methods().getConfig();
}

const QList<DataTypeCustom*>& MethodModel::getCustomDataTypes() const
{
    return mDocument.getCustomDataTypes();
}

DocModelNotifier& MethodModel::getNotifier() const
{
    return const_cast<IEDocumentModel&>(mDocument).getNotifier();
}

//////////////////////////////////////////////////////////////////////////
// Reads -- parameters
//////////////////////////////////////////////////////////////////////////

const QList<MethodParameter>& MethodModel::getParams(const MethodEntry* method) const
{
    static const QList<MethodParameter> _empty;
    return (method != nullptr ? method->getElements() : _empty);
}

int MethodModel::getParamCount(const MethodEntry* method) const
{
    return (method != nullptr ? method->getElementCount() : 0);
}

MethodParameter* MethodModel::findParam(const MethodEntry* method, uint32_t paramId) const
{
    return (method != nullptr ? const_cast<MethodEntry*>(method)->findElement(paramId) : nullptr);
}

MethodParameter* MethodModel::findParam(const MethodEntry* method, const QString& name) const
{
    return (method != nullptr ? const_cast<MethodEntry*>(method)->findElement(name) : nullptr);
}

int MethodModel::findParamIndex(const MethodEntry* method, uint32_t paramId) const
{
    return (method != nullptr ? method->findIndex(paramId) : -1);
}

//////////////////////////////////////////////////////////////////////////
// Mutations -- methods
//////////////////////////////////////////////////////////////////////////

MethodEntry* MethodModel::createMethod(const QString& name, int kind)
{
    if (findMethod(name, kind) != nullptr)
        return nullptr;

    MethodEntry* entry = new MethodEntry(0, name, kind, getConfig());
    mDocument.getUndoStack().push(new TDocAddCommand<MethodEntry*, DocumentElem>(getNotifier(), methods(), entry, eDocElementKind::Method, QObject::tr("Add method")));
    return findMethod(name, kind);
}

MethodEntry* MethodModel::insertMethod(int position, const QString& name, int kind)
{
    if (findMethod(name, kind) != nullptr)
        return nullptr;

    MethodEntry* entry = new MethodEntry(0, name, kind, getConfig());
    mDocument.getUndoStack().push(buildInsertCommand<MethodEntry*, DocumentElem>(getNotifier(), methods(), entry, position, 0u, eDocElementKind::Method, QObject::tr("Insert method")));
    return findMethod(name, kind);
}

void MethodModel::deleteMethod(uint32_t id)
{
    MethodEntry* entry = findMethod(id);
    if (entry == nullptr)
        return;

    // A method others are answered by takes their reply with it, in the same undo step: a name
    // that is gone must not stay written in another method's Response.
    const QString text{ QObject::tr("Delete method") };
    if (entry->isReplyKind())
    {
        DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
        clearRepliesTo(entry->getName(), composite);
        new TDocRemoveCommand<MethodEntry*, DocumentElem>(getNotifier(), methods(), id, eDocElementKind::Method, text, composite);
        mDocument.getUndoStack().push(composite);
    }
    else
    {
        mDocument.getUndoStack().push(new TDocRemoveCommand<MethodEntry*, DocumentElem>(getNotifier(), methods(), id, eDocElementKind::Method, text));
    }
}

void MethodModel::swapMethods(uint32_t firstId, uint32_t secondId)
{
    const int index1 = methods().findIndex(firstId);
    const int index2 = methods().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mDocument.getUndoStack().push(new TDocReorderCommand<MethodEntry*, DocumentElem>(getNotifier(), methods(), index1, index2, 0u, eDocElementKind::Method, QObject::tr("Reorder methods")));
}

void MethodModel::renameMethod(uint32_t id, const QString& newName)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    const QString oldName{ entry->getName() };
    const bool isReplyKind{ entry->isReplyKind() };
    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getName() : QString()); };
    auto setter = [document, id](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setName(value); };

    const QString text{ QObject::tr("Rename method") };
    DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
    new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, newName, text, composite);

    // A method others are answered by carries its name in theirs, so they follow it here.
    if (isReplyKind)
    {
        for (MethodEntry* other : getMethods())
        {
            if ((other == nullptr) || (other->hasReply() == false) || (other->getReply() != oldName))
                continue;

            const uint32_t otherId = other->getId();
            auto replyGet = [document, otherId]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(otherId); return (m != nullptr ? m->getReply() : QString()); };
            auto replySet = [document, otherId](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(otherId); if (m != nullptr) m->setReply(value); };
            new TDocSetPropertyCommand<QString>(getNotifier(), otherId, eDocElementKind::Method, replyGet, replySet, newName, text, composite);
        }
    }

    // A document that reaches its methods by name repairs those references in the same step.
    mDocument.createRenameSideEffects(eDocElementKind::Method, id, oldName, newName, composite);
    mDocument.getUndoStack().push(composite);
}

void MethodModel::clearRepliesTo(const QString& replyName, QUndoCommand* parent)
{
    if ((replyName.isEmpty()) || (parent == nullptr))
        return;

    IEDocumentModel* document = &mDocument;
    for (MethodEntry* other : getMethods())
    {
        if ((other == nullptr) || (other->hasReply() == false) || (other->getReply() != replyName))
            continue;

        const uint32_t otherId = other->getId();
        auto getter = [document, otherId]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(otherId); return (m != nullptr ? m->getReply() : QString()); };
        auto setter = [document, otherId](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(otherId); if (m != nullptr) m->setReply(value); };
        new TDocSetPropertyCommand<QString>(getNotifier(), otherId, eDocElementKind::Method, getter, setter, QString(), parent->text(), parent);
    }
}

void MethodModel::setKind(uint32_t id, int kind)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (kind == entry->getKind()))
        return;

    // The kind decides what the method carries, so a method that stops being an answer takes
    // the replies that named it with it, in the same undo step.
    const QString text{ QObject::tr("Set method type") };
    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> int { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getKind() : 0); };
    auto setter = [document, id](int value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setKind(value); };

    if (entry->isReplyKind())
    {
        DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
        clearRepliesTo(entry->getName(), composite);
        new TDocSetPropertyCommand<int>(getNotifier(), id, eDocElementKind::Method, getter, setter, kind, text, composite);
        mDocument.getUndoStack().push(composite);
    }
    else
    {
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<int>(getNotifier(), id, eDocElementKind::Method, getter, setter, kind, text));
    }
}

void MethodModel::setReply(uint32_t id, const QString& replyName)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (replyName == entry->getReply()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getReply() : QString()); };
    auto setter = [document, id](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setReply(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, replyName, QObject::tr("Set connected response")));
}

void MethodModel::setReturn(uint32_t id, const QString& typeName)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (typeName == entry->getReturn()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getReturn() : QString()); };
    auto setter = [document, id](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setReturn(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, typeName, QObject::tr("Set return type")));
}

void MethodModel::setImplement(uint32_t id, MethodEntry::eImplement implement)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (implement == entry->getImplement()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> MethodEntry::eImplement { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getImplement() : MethodEntry::eImplement::Handler); };
    auto setter = [document, id](MethodEntry::eImplement value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setImplement(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<MethodEntry::eImplement>(getNotifier(), id, eDocElementKind::Method, getter, setter, implement, QObject::tr("Set implementation")));
}

void MethodModel::setBody(uint32_t id, const QString& body)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (body == entry->getBody()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getBody() : QString()); };
    auto setter = [document, id](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setBody(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, body, QObject::tr("Set method body")));
}

void MethodModel::setDescription(uint32_t id, const QString& text)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getDescription() : QString()); };
    auto setter = [document, id](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setDescription(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, text, QObject::tr("Set description")));
}

void MethodModel::setDeprecated(uint32_t id, bool deprecated)
{
    MethodEntry* entry = findMethod(id);
    if (entry == nullptr)
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> DeprecationState
    {
        MethodEntry* m = document->getMethodSection().findMethod(id);
        return (m != nullptr ? DeprecationState{ m->getIsDeprecated(), m->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [document, id](const DeprecationState& value)
    {
        MethodEntry* m = document->getMethodSection().findMethod(id);
        if (m != nullptr) { m->setIsDeprecated(value.flag); m->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Method, getter, setter, next, QObject::tr("Set deprecated")));
}

void MethodModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    MethodEntry* entry = findMethod(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { MethodEntry* m = document->getMethodSection().findMethod(id); return (m != nullptr ? m->getDeprecateHint() : QString()); };
    auto setter = [document, id](const QString& value) { MethodEntry* m = document->getMethodSection().findMethod(id); if (m != nullptr) m->setDeprecateHint(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Method, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

//////////////////////////////////////////////////////////////////////////
// Mutations -- parameters
//////////////////////////////////////////////////////////////////////////

MethodParameter* MethodModel::createParam(MethodEntry* method, const QString& name)
{
    if ((method == nullptr) || (method->findElement(name) != nullptr))
        return nullptr;

    MethodParameter param(0, name, QStringLiteral("bool"), QString(), false, nullptr);
    param.validate(getCustomDataTypes());
    mDocument.getUndoStack().push(new TDocAddCommand<MethodParameter, DocumentElem>(getNotifier(), *method, param, eDocElementKind::Method, QObject::tr("Add parameter")));
    return method->findElement(name);
}

MethodParameter* MethodModel::insertParam(MethodEntry* method, int position, const QString& name)
{
    if ((method == nullptr) || (method->findElement(name) != nullptr))
        return nullptr;

    MethodParameter param(0, name, QStringLiteral("bool"), QString(), false, nullptr);
    param.validate(getCustomDataTypes());
    mDocument.getUndoStack().push(buildInsertCommand<MethodParameter, DocumentElem>(getNotifier(), *method, param, position, method->getId(), eDocElementKind::Method, QObject::tr("Insert parameter")));
    return method->findElement(name);
}

void MethodModel::deleteParam(MethodEntry* method, uint32_t paramId)
{
    if (method == nullptr)
        return;

    mDocument.getUndoStack().push(new TDocRemoveCommand<MethodParameter, DocumentElem>(getNotifier(), *method, paramId, eDocElementKind::Method, QObject::tr("Delete parameter")));
}

void MethodModel::swapParams(MethodEntry* method, uint32_t firstId, uint32_t secondId)
{
    if (method == nullptr)
        return;

    const int index1 = method->findIndex(firstId);
    const int index2 = method->findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mDocument.getUndoStack().push(new TDocReorderCommand<MethodParameter, DocumentElem>(getNotifier(), *method, index1, index2, method->getId(), eDocElementKind::Method, QObject::tr("Reorder parameters")));
}

void MethodModel::setParamName(MethodEntry* method, uint32_t paramId, const QString& name)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getName() : QString()); };
    auto setter = [method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) p->setName(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, name, QObject::tr("Rename parameter")));
}

void MethodModel::setParamType(MethodEntry* method, uint32_t paramId, const QString& typeName)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getType() : QString()); };
    auto setter = [this, method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) { p->setType(value); p->validate(getCustomDataTypes()); } };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, typeName, QObject::tr("Set parameter type")));
}

void MethodModel::setParamDefault(MethodEntry* method, uint32_t paramId, bool hasDefault, const QString& value)
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
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<ParamDefaultState>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, next, QObject::tr("Set parameter default")));
}

void MethodModel::setParamDescription(MethodEntry* method, uint32_t paramId, const QString& text)
{
    if (method == nullptr)
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getDescription() : QString()); };
    auto setter = [method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) p->setDescription(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, text, QObject::tr("Set parameter description")));
}

void MethodModel::setParamDeprecated(MethodEntry* method, uint32_t paramId, bool deprecated)
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
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, next, QObject::tr("Set parameter deprecated")));
}

void MethodModel::setParamDeprecateHint(MethodEntry* method, uint32_t paramId, const QString& hint)
{
    if (method == nullptr)
        return;

    MethodParameter* param = method->findElement(paramId);
    if ((param == nullptr) || (param->getIsDeprecated() == false) || (hint == param->getDeprecateHint()))
        return;

    const uint32_t ownerId = method->getId();
    auto getter = [method, paramId]() -> QString { MethodParameter* p = method->findElement(paramId); return (p != nullptr ? p->getDeprecateHint() : QString()); };
    auto setter = [method, paramId](const QString& value) { MethodParameter* p = method->findElement(paramId); if (p != nullptr) p->setDeprecateHint(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::Method, getter, setter, hint, QObject::tr("Set parameter deprecation hint")));
}

void MethodModel::resolveDeclaredTypes()
{
    const QList<DataTypeCustom*>& customTypes = getCustomDataTypes();
    for (MethodEntry* entry : getMethods())
    {
        if (entry != nullptr)
        {
            entry->validate(customTypes);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////

const MethodDataSection& MethodModel::methods() const
{
    return const_cast<IEDocumentModel&>(mDocument).getMethodSection();
}

MethodDataSection& MethodModel::methods()
{
    return mDocument.getMethodSection();
}
