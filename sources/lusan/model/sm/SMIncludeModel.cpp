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
 *  \file        lusan/model/sm/SMIncludeModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMIncludeModel.hpp"

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

SMIncludeModel::SMIncludeModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<IncludeEntry>& SMIncludeModel::getIncludes() const
{
    return includes().getElements();
}

int SMIncludeModel::getIncludeCount() const
{
    return includes().getElementCount();
}

IncludeEntry* SMIncludeModel::findInclude(const QString& location) const
{
    return includes().findElement(location);
}

IncludeEntry* SMIncludeModel::findInclude(uint32_t id) const
{
    return includes().findElement(id);
}

int SMIncludeModel::findIndex(uint32_t id) const
{
    return includes().findIndex(id);
}

DocModelNotifier& SMIncludeModel::getNotifier() const
{
    return mFacade.getNotifier();
}

IncludeEntry* SMIncludeModel::createInclude(const QString& location)
{
    if (findInclude(location) != nullptr)
        return nullptr;

    IncludeEntry entry(0, location, nullptr);
    mFacade.getUndoStack().push(new TDocAddCommand<IncludeEntry, DocumentElem>(getNotifier(), includes(), std::move(entry), eDocElementKind::Include, QObject::tr("Add include")));
    return findInclude(location);
}

IncludeEntry* SMIncludeModel::insertInclude(int position, const QString& location)
{
    if (findInclude(location) != nullptr)
        return nullptr;

    IncludeEntry entry(0, location, nullptr);
    mFacade.getUndoStack().push(buildInsertCommand<IncludeEntry, DocumentElem>(getNotifier(), includes(), std::move(entry), position, 0u, eDocElementKind::Include, QObject::tr("Insert include")));
    return findInclude(location);
}

void SMIncludeModel::deleteInclude(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<IncludeEntry, DocumentElem>(getNotifier(), includes(), id, eDocElementKind::Include, QObject::tr("Delete include")));
}

void SMIncludeModel::swapIncludes(uint32_t firstId, uint32_t secondId)
{
    const int index1 = includes().findIndex(firstId);
    const int index2 = includes().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<IncludeEntry, DocumentElem>(getNotifier(), includes(), index1, index2, 0u, eDocElementKind::Include, QObject::tr("Reorder includes")));
}

void SMIncludeModel::setLocation(uint32_t id, const QString& location)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (location == entry->getLocation()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { IncludeEntry* e = facade->getData().getIncludes().findElement(id); return (e != nullptr ? e->getLocation() : QString()); };
    auto setter = [facade, id](const QString& value) { IncludeEntry* e = facade->getData().getIncludes().findElement(id); if (e != nullptr) e->setLocation(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Include, getter, setter, location, QObject::tr("Set include location")));
}

void SMIncludeModel::setDescription(uint32_t id, const QString& text)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { IncludeEntry* e = facade->getData().getIncludes().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { IncludeEntry* e = facade->getData().getIncludes().findElement(id); if (e != nullptr) e->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Include, getter, setter, text, QObject::tr("Set description")));
}

void SMIncludeModel::setDeprecated(uint32_t id, bool deprecated)
{
    IncludeEntry* entry = findInclude(id);
    if (entry == nullptr)
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> DeprecationState
    {
        IncludeEntry* e = facade->getData().getIncludes().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [facade, id](const DeprecationState& value)
    {
        IncludeEntry* e = facade->getData().getIncludes().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Include, getter, setter, next, QObject::tr("Set deprecated")));
}

void SMIncludeModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { IncludeEntry* e = facade->getData().getIncludes().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [facade, id](const QString& value) { IncludeEntry* e = facade->getData().getIncludes().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Include, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

const SMIncludeData& SMIncludeModel::includes() const
{
    return mFacade.getData().getIncludes();
}

SMIncludeData& SMIncludeModel::includes()
{
    return mFacade.getData().getIncludes();
}
