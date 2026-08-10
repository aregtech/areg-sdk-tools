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
 *  \file        lusan/model/common/IncludeModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Includes page model shared by every document editor.
 *
 ************************************************************************/

#include "lusan/model/common/IncludeModel.hpp"

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

IncludeModel::IncludeModel(IEDocumentModel& document)
    : mDocument (document)
{
}

const QList<IncludeEntry>& IncludeModel::getIncludes(void) const
{
    return section().getElements();
}

int IncludeModel::getIncludeCount(void) const
{
    return section().getElementCount();
}

IncludeEntry* IncludeModel::findInclude(const QString& location) const
{
    return section().findElement(location);
}

IncludeEntry* IncludeModel::findInclude(uint32_t id) const
{
    return section().findElement(id);
}

int IncludeModel::findIndex(uint32_t id) const
{
    return section().findIndex(id);
}

DocModelNotifier& IncludeModel::getNotifier(void) const
{
    return mDocument.getNotifier();
}

IEDocumentModel& IncludeModel::getDocument(void) const
{
    return mDocument;
}

bool IncludeModel::isReadOnly(void) const
{
    return false;
}

eDocElementKind IncludeModel::kindOfLocation(const QString& /*location*/) const
{
    return eDocElementKind::Include;
}

void IncludeModel::prepareNewEntry(IncludeEntry& /*entry*/) const
{
}

eDocElementKind IncludeModel::kindOfId(uint32_t id) const
{
    const IncludeEntry* entry = findInclude(id);
    return kindOfLocation(entry != nullptr ? entry->getLocation() : QString());
}

IncludeEntry* IncludeModel::createInclude(const QString& location)
{
    if (findInclude(location) != nullptr)
        return nullptr;

    IncludeEntry entry(0, location, nullptr);
    prepareNewEntry(entry);
    mDocument.getUndoStack().push(new TDocAddCommand<IncludeEntry, DocumentElem>(getNotifier(), section(), std::move(entry), kindOfLocation(location), QObject::tr("Add include")));
    return findInclude(location);
}

IncludeEntry* IncludeModel::insertInclude(int position, const QString& location)
{
    if (findInclude(location) != nullptr)
        return nullptr;

    IncludeEntry entry(0, location, nullptr);
    prepareNewEntry(entry);
    mDocument.getUndoStack().push(buildInsertCommand<IncludeEntry, DocumentElem>(getNotifier(), section(), std::move(entry), position, 0u, kindOfLocation(location), QObject::tr("Insert include")));
    return findInclude(location);
}

void IncludeModel::deleteInclude(uint32_t id)
{
    mDocument.getUndoStack().push(new TDocRemoveCommand<IncludeEntry, DocumentElem>(getNotifier(), section(), id, kindOfId(id), QObject::tr("Delete include")));
}

void IncludeModel::swapIncludes(uint32_t firstId, uint32_t secondId)
{
    const int index1 = section().findIndex(firstId);
    const int index2 = section().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mDocument.getUndoStack().push(new TDocReorderCommand<IncludeEntry, DocumentElem>(getNotifier(), section(), index1, index2, 0u, kindOfId(firstId), QObject::tr("Reorder includes")));
}

void IncludeModel::setLocation(uint32_t id, const QString& location)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (location == entry->getLocation()))
        return;

    // A location edit can move a row between groups. Announce it under the heavier of the two
    // kinds, so a listener that only follows one of them still rebuilds on the way in and out.
    const eDocElementKind before = kindOfId(id);
    const eDocElementKind after  = kindOfLocation(location);
    const eDocElementKind kind   = (before == after) ? before
                                 : ((before == eDocElementKind::Include) ? after : before);

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { IncludeEntry* e = document->getIncludeSection().findElement(id); return (e != nullptr ? e->getLocation() : QString()); };
    auto setter = [document, id](const QString& value) { IncludeEntry* e = document->getIncludeSection().findElement(id); if (e != nullptr) e->setLocation(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, kind, getter, setter, location, QObject::tr("Set include location")));
}

void IncludeModel::setDescription(uint32_t id, const QString& text)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { IncludeEntry* e = document->getIncludeSection().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [document, id](const QString& value) { IncludeEntry* e = document->getIncludeSection().findElement(id); if (e != nullptr) e->setDescription(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, kindOfId(id), getter, setter, text, QObject::tr("Set description")));
}

void IncludeModel::setDeprecated(uint32_t id, bool deprecated)
{
    IncludeEntry* entry = findInclude(id);
    if (entry == nullptr)
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> DeprecationState
    {
        IncludeEntry* e = document->getIncludeSection().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [document, id](const DeprecationState& value)
    {
        IncludeEntry* e = document->getIncludeSection().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, kindOfId(id), getter, setter, next, QObject::tr("Set deprecated")));
}

void IncludeModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document, id]() -> QString { IncludeEntry* e = document->getIncludeSection().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [document, id](const QString& value) { IncludeEntry* e = document->getIncludeSection().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, kindOfId(id), getter, setter, hint, QObject::tr("Set deprecation hint")));
}
