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
 *  \file        lusan/model/sm/SMOverviewModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMOverviewModel.hpp"

#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/model/common/DocElementCommands.hpp"

SMOverviewModel::SMOverviewModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QString& SMOverviewModel::getName() const
{
    return overview().getName();
}

const VersionNumber& SMOverviewModel::getVersion() const
{
    return overview().getVersion();
}

SMOverviewData::eThreading SMOverviewModel::getThreading() const
{
    return overview().getThreading();
}

const QString& SMOverviewModel::getDescription() const
{
    return overview().getDescription();
}

void SMOverviewModel::setName(const QString& name)
{
    if (name == getName())
        return;

    StateMachineModel* facade = &mFacade;
    const uint32_t id = getOverviewId();
    auto getter = [facade]() -> QString { return facade->getData().getOverview().getName(); };
    auto setter = [facade](const QString& value) { facade->getData().getOverview().setName(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Overview, getter, setter, name, QObject::tr("Set machine name")));
}

void SMOverviewModel::setVersion(const VersionNumber& version)
{
    if (version == getVersion())
        return;

    StateMachineModel* facade = &mFacade;
    const uint32_t id = getOverviewId();
    auto getter = [facade]() -> VersionNumber { return facade->getData().getOverview().getVersion(); };
    auto setter = [facade](const VersionNumber& value) { facade->getData().getOverview().setVersion(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<VersionNumber>(getNotifier(), id, eDocElementKind::Overview, getter, setter, version, QObject::tr("Set version")));
}

void SMOverviewModel::setThreading(SMOverviewData::eThreading threading)
{
    if (threading == getThreading())
        return;

    StateMachineModel* facade = &mFacade;
    const uint32_t id = getOverviewId();
    auto getter = [facade]() -> SMOverviewData::eThreading { return facade->getData().getOverview().getThreading(); };
    auto setter = [facade](const SMOverviewData::eThreading& value) { facade->getData().getOverview().setThreading(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<SMOverviewData::eThreading>(getNotifier(), id, eDocElementKind::Overview, getter, setter, threading, QObject::tr("Set threading mode")));
}

void SMOverviewModel::setDescription(const QString& description)
{
    if (description == getDescription())
        return;

    StateMachineModel* facade = &mFacade;
    const uint32_t id = getOverviewId();
    auto getter = [facade]() -> QString { return facade->getData().getOverview().getDescription(); };
    auto setter = [facade](const QString& value) { facade->getData().getOverview().setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Overview, getter, setter, description, QObject::tr("Set description")));
}

DocModelNotifier& SMOverviewModel::getNotifier() const
{
    return mFacade.getNotifier();
}

uint32_t SMOverviewModel::getOverviewId() const
{
    return overview().getId();
}

const SMOverviewData& SMOverviewModel::overview() const
{
    return mFacade.getData().getOverview();
}

SMOverviewData& SMOverviewModel::overview()
{
    return mFacade.getData().getOverview();
}
