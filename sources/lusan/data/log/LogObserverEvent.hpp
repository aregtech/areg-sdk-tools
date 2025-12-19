#ifndef LUSAN_DATA_LOG_LOGOBSERVEREVENT_HPP
#define LUSAN_DATA_LOG_LOGOBSERVEREVENT_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/log/LogObserverEvent.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer event.
 *
 ************************************************************************/

/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"

#include "areg/base/GEGlobal.h"
#include "areg/component/TEEvent.hpp"
#include "areg/base/SharedBuffer.hpp"

class LogObserverEventData
{
//////////////////////////////////////////////////////////////////////////
// LogObserverEventData Types and constants
//////////////////////////////////////////////////////////////////////////
public:
    enum class eLogObserverEvent : unsigned short
    {
          CMD_Unknown           //!< Invalid event
        , CMD_Configured        //!< Initialize and connect the log observer
        , CMD_DbConfigured      //!< Disconnect log observer.
        , CMD_Connected         //!< The log observer is connected to the log collector service and ready to operate.
        , CMD_Started           //!< The log observer is started and can communicate with the log observer.
        , CMD_DbCreated         //!< The logging database is created by log observer.
        , CMD_MessageFailed     //!< Logging message failed.
        , CMD_InstConnected     //!< The log observer received list of connected instances.
        , CMD_InstDisconnected  //!< The log observer received list of disconnected instances.
        , CMD_ServiceDisconnect //!< The log observer disconnected service.
        , CMD_ScopesRegistered  //!< The log observer received list of registered scopes.
        , CMD_ScopesUpdated     //!< The log observer received list of updated scopes.
        , CMD_LogMessage        //!< The log observer received log message.
    };

    DECLARE_STREAMABLE(LogObserverEventData::eLogObserverEvent);

public:
    inline LogObserverEventData(void);
    inline LogObserverEventData(eLogObserverEvent event);
    inline LogObserverEventData(eLogObserverEvent event, const SharedBuffer& data);
    inline LogObserverEventData(const LogObserverEventData& src);

    inline LogObserverEventData::eLogObserverEvent getEvent(void) const;

    inline SharedBuffer& getBuffer(void);

    inline const SharedBuffer& getBuffer(void) const;

    inline operator const IEInStream& (void) const;

    inline operator IEOutStream& (void);

private:
    eLogObserverEvent   mEvent;     //!< The event type
    SharedBuffer        mBuffer;    //!< The buffer to store event data
};

//////////////////////////////////////////////////////////////////////////
// Implement PatientInfoEventData::eUpdateCommands streamable.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_STREAMABLE(LogObserverEventData::eLogObserverEvent);

//////////////////////////////////////////////////////////////////////////
// Define custom event and the event consumer.
//////////////////////////////////////////////////////////////////////////
DECLARE_EVENT(LogObserverEventData, LogObserverEvent, IELogObserverEventConsumer);


//////////////////////////////////////////////////////////////////////////
// LogObserverEventData inline methods.
//////////////////////////////////////////////////////////////////////////
LogObserverEventData::LogObserverEventData(void)
    : mEvent(LogObserverEventData::eLogObserverEvent::CMD_Unknown)
    , mBuffer()
{
}

inline LogObserverEventData::LogObserverEventData(eLogObserverEvent event)
    : mEvent(event)
    , mBuffer()
{
}

inline LogObserverEventData::LogObserverEventData(eLogObserverEvent event, const SharedBuffer& data)
    : mEvent(event)
    , mBuffer(data)
{
}

inline LogObserverEventData::LogObserverEventData(const LogObserverEventData& src)
    : mEvent(src.mEvent)
    , mBuffer(src.mBuffer)
{
}

inline LogObserverEventData::eLogObserverEvent LogObserverEventData::getEvent(void) const
{
    return mEvent;
}

inline SharedBuffer& LogObserverEventData::getBuffer(void)
{
    return mBuffer;
}

inline const SharedBuffer& LogObserverEventData::getBuffer(void) const
{
    return mBuffer;
}

inline LogObserverEventData::operator const IEInStream& (void) const
{
    return static_cast<const IEInStream&>(mBuffer);
}

inline LogObserverEventData::operator IEOutStream& (void)
{
    return static_cast<IEOutStream&>(mBuffer);
}

#endif  // LUSAN_DATA_LOG_LOGOBSERVEREVENT_HPP
