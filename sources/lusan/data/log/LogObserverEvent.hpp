#ifndef LUSAN_DATA_LOG_LOGOBSERVEREVENT_HPP
#define LUSAN_DATA_LOG_LOGOBSERVEREVENT_HPP
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
 *  \file        lusan/data/log/LogObserverEvent.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer event.
 *
 ************************************************************************/

/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"

#include "areg/base/areg_global.h"
#include "areg/component/EventTemplate.hpp"
#include "areg/base/SharedBuffer.hpp"

class LogObserverEventData
{
//////////////////////////////////////////////////////////////////////////
// LogObserverEventData Types and constants
//////////////////////////////////////////////////////////////////////////
public:
    enum class LogObserverCommand : unsigned short
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

public:
    inline LogObserverEventData(void);
    inline LogObserverEventData(LogObserverCommand event);
    inline LogObserverEventData(LogObserverCommand event, const areg::SharedBuffer& data);
    inline LogObserverEventData(const LogObserverEventData& src);
    inline LogObserverEventData(LogObserverEventData && src);

    inline LogObserverEventData::LogObserverCommand getEvent(void) const;

    inline areg::SharedBuffer& getBuffer(void);

    inline const areg::SharedBuffer& getBuffer(void) const;

    inline operator const areg::InStream& (void) const;

    inline operator areg::OutStream& (void);

private:
    LogObserverCommand  mEvent;     //!< The event type
    areg::SharedBuffer  mBuffer;    //!< The buffer to store event data
};

//////////////////////////////////////////////////////////////////////////
// Define custom event and the event consumer.
//////////////////////////////////////////////////////////////////////////
AREG_DECLARE_EVENT(LogObserverEventData, LogObserverEvent, LogObserverEventConsumer);


//////////////////////////////////////////////////////////////////////////
// LogObserverEventData inline methods.
//////////////////////////////////////////////////////////////////////////
LogObserverEventData::LogObserverEventData(void)
    : mEvent(LogObserverEventData::LogObserverCommand::CMD_Unknown)
    , mBuffer()
{
}

inline LogObserverEventData::LogObserverEventData(LogObserverCommand event)
    : mEvent(event)
    , mBuffer()
{
}

inline LogObserverEventData::LogObserverEventData(LogObserverCommand event, const areg::SharedBuffer& data)
    : mEvent(event)
    , mBuffer(data)
{
}

inline LogObserverEventData::LogObserverEventData(const LogObserverEventData& src)
    : mEvent(src.mEvent)
    , mBuffer(src.mBuffer)
{
}

inline LogObserverEventData::LogObserverEventData(LogObserverEventData&& src)
    : mEvent(src.mEvent)
    , mBuffer(std::move(src.mBuffer))
{
    src.mEvent = LogObserverCommand::CMD_Unknown;
}

inline LogObserverEventData::LogObserverCommand LogObserverEventData::getEvent(void) const
{
    return mEvent;
}

inline areg::SharedBuffer& LogObserverEventData::getBuffer(void)
{
    return mBuffer;
}

inline const areg::SharedBuffer& LogObserverEventData::getBuffer(void) const
{
    return mBuffer;
}

inline LogObserverEventData::operator const areg::InStream& (void) const
{
    return static_cast<const areg::InStream&>(mBuffer);
}

inline LogObserverEventData::operator areg::OutStream& (void)
{
    return static_cast<areg::OutStream&>(mBuffer);
}

#endif  // LUSAN_DATA_LOG_LOGOBSERVEREVENT_HPP
