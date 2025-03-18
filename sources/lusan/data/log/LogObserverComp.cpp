/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/log/LogObserverComp.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer component with empty service provider.
 *
 ************************************************************************/
 /************************************************************************
  * Include files.
  ************************************************************************/
#include "lusan/data/log/LogObserverComp.hpp"


Component * LogObserverComp::CreateComponent(const NERegistry::ComponentEntry & entry, ComponentThread & owner)
{
    return DEBUG_NEW LogObserverComp( entry, owner, entry.getComponentData());
}

void LogObserverComp::DeleteComponent(Component & compObject, const NERegistry::ComponentEntry & /* entry */)
{
    delete (&compObject);
}

LogObserverComp::LogObserverComp(const NERegistry::ComponentEntry & entry, ComponentThread & ownerThread, NEMemory::uAlign OPT /* data */)
    : Component ( entry, ownerThread )
    , StubBase  ( self(), NEService::getEmptyInterface() )
    , IELogObserverEventConsumer()
    , mLogObserver()

{
}

LogObserverComp::~LogObserverComp(void)
{

}

void LogObserverComp::startupServiceInterface(Component & holder)
{
    StubBase::startupServiceInterface(holder);
}

void LogObserverComp::shutdownServiceIntrface(Component & holder)
{
    mLogObserver.loggingStop();
    mLogObserver.loggingClear();
    StubBase::shutdownServiceIntrface(holder);
}

void LogObserverComp::processEvent(const LogObserverEventData& data)
{
}
