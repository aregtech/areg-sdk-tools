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
 *  \file        lusan/model/sm/SMTimerModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Timers page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMTimerModel.hpp"

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

SMTimerModel::SMTimerModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<SMTimerEntry>& SMTimerModel::getTimers() const
{
    return timers().getElements();
}

int SMTimerModel::getTimerCount() const
{
    return timers().getElementCount();
}

SMTimerEntry* SMTimerModel::findTimer(const QString& name) const
{
    return timers().findElement(name);
}

SMTimerEntry* SMTimerModel::findTimer(uint32_t id) const
{
    return timers().findElement(id);
}

int SMTimerModel::findIndex(uint32_t id) const
{
    return timers().findIndex(id);
}

DocModelNotifier& SMTimerModel::getNotifier() const
{
    return mFacade.getNotifier();
}

StateMachineData::StimulusRef SMTimerModel::findStimulus(const QString& name) const
{
    return mFacade.getData().findStimulus(name);
}

SMTimerEntry* SMTimerModel::createTimer(const QString& name)
{
    if (findTimer(name) != nullptr)
        return nullptr;

    SMTimerEntry entry(0, name, 1, 1);
    mFacade.getUndoStack().push(new TDocAddCommand<SMTimerEntry, DocumentElem>(getNotifier(), timers(), std::move(entry), eDocElementKind::Timer, QObject::tr("Add timer")));
    return findTimer(name);
}

SMTimerEntry* SMTimerModel::insertTimer(int position, const QString& name)
{
    if (findTimer(name) != nullptr)
        return nullptr;

    SMTimerEntry entry(0, name, 1, 1);
    mFacade.getUndoStack().push(buildInsertCommand<SMTimerEntry, DocumentElem>(getNotifier(), timers(), std::move(entry), position, 0u, eDocElementKind::Timer, QObject::tr("Insert timer")));
    return findTimer(name);
}

void SMTimerModel::deleteTimer(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<SMTimerEntry, DocumentElem>(getNotifier(), timers(), id, eDocElementKind::Timer, QObject::tr("Delete timer")));
}

void SMTimerModel::swapTimers(uint32_t firstId, uint32_t secondId)
{
    const int index1 = timers().findIndex(firstId);
    const int index2 = timers().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<SMTimerEntry, DocumentElem>(getNotifier(), timers(), index1, index2, 0u, eDocElementKind::Timer, QObject::tr("Reorder timers")));
}

void SMTimerModel::renameTimer(uint32_t id, const QString& newName)
{
    SMTimerEntry* entry = findTimer(id);
    if ((entry == nullptr) || (newName == entry->getName()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMTimerEntry* e = facade->getData().getTimers().findElement(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [facade, id](const QString& value) { SMTimerEntry* e = facade->getData().getTimers().findElement(id); if (e != nullptr) e->setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Timer, getter, setter, newName, QObject::tr("Rename timer")));
}

void SMTimerModel::setTimeout(uint32_t id, uint32_t timeout)
{
    SMTimerEntry* entry = findTimer(id);
    if ((entry == nullptr) || (timeout == entry->getTimeout()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> uint32_t { SMTimerEntry* e = facade->getData().getTimers().findElement(id); return (e != nullptr ? e->getTimeout() : 1u); };
    auto setter = [facade, id](const uint32_t& value) { SMTimerEntry* e = facade->getData().getTimers().findElement(id); if (e != nullptr) e->setTimeout(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<uint32_t>(getNotifier(), id, eDocElementKind::Timer, getter, setter, timeout, QObject::tr("Set timeout")));
}

void SMTimerModel::setRepeat(uint32_t id, uint32_t repeat)
{
    SMTimerEntry* entry = findTimer(id);
    if ((entry == nullptr) || (repeat == entry->getRepeat()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> uint32_t { SMTimerEntry* e = facade->getData().getTimers().findElement(id); return (e != nullptr ? e->getRepeat() : 1u); };
    auto setter = [facade, id](const uint32_t& value) { SMTimerEntry* e = facade->getData().getTimers().findElement(id); if (e != nullptr) e->setRepeat(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<uint32_t>(getNotifier(), id, eDocElementKind::Timer, getter, setter, repeat, QObject::tr("Set repeat")));
}

void SMTimerModel::setDescription(uint32_t id, const QString& text)
{
    SMTimerEntry* entry = findTimer(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMTimerEntry* e = facade->getData().getTimers().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { SMTimerEntry* e = facade->getData().getTimers().findElement(id); if (e != nullptr) e->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Timer, getter, setter, text, QObject::tr("Set description")));
}

void SMTimerModel::setDeprecated(uint32_t id, bool deprecated)
{
    SMTimerEntry* entry = findTimer(id);
    if (entry == nullptr)
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> DeprecationState
    {
        SMTimerEntry* e = facade->getData().getTimers().findElement(id);
        return (e != nullptr ? DeprecationState{ e->getIsDeprecated(), e->getDeprecateHint() } : DeprecationState{});
    };
    auto setter = [facade, id](const DeprecationState& value)
    {
        SMTimerEntry* e = facade->getData().getTimers().findElement(id);
        if (e != nullptr) { e->setIsDeprecated(value.flag); e->setDeprecateHint(value.hint); }
    };
    const DeprecationState next{ deprecated, deprecated ? entry->getDeprecateHint() : QString() };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::Timer, getter, setter, next, QObject::tr("Set deprecated")));
}

void SMTimerModel::setDeprecateHint(uint32_t id, const QString& hint)
{
    SMTimerEntry* entry = findTimer(id);
    if ((entry == nullptr) || (entry->getIsDeprecated() == false) || (hint == entry->getDeprecateHint()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMTimerEntry* e = facade->getData().getTimers().findElement(id); return (e != nullptr ? e->getDeprecateHint() : QString()); };
    auto setter = [facade, id](const QString& value) { SMTimerEntry* e = facade->getData().getTimers().findElement(id); if (e != nullptr) e->setDeprecateHint(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Timer, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

const SMTimerData& SMTimerModel::timers() const
{
    return mFacade.getData().getTimers();
}

SMTimerData& SMTimerModel::timers()
{
    return mFacade.getData().getTimers();
}
