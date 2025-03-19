#ifndef LUSAN_DATA_LOG_LOGOBSERVERCOMP_HPP
#define LUSAN_DATA_LOG_LOGOBSERVERCOMP_HPP
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
 *  \file        lusan/data/log/LogObserverComp.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer component with empty service provider.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "areg/base/GEGlobal.h"

#include "lusan/data/log/LogObserverEvent.hpp"
#include "lusan/data/log/LogObserver.hpp"
#include "areg/component/Component.hpp"
#include "areg/component/StubBase.hpp"

//! \brief   An empty servicing component to support multithreading.
class LogObserverComp       : public    Component
                            , protected StubBase
                            , protected IELogObserverEventConsumer
{
//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
private:

//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Called by system to instantiate the component.
     * \param   entry   The entry of registry, which describes the component.
     * \param   owner   The component owning thread.
     * \return  Returns instantiated component to run in the system
     **/
    static Component * CreateComponent( const NERegistry::ComponentEntry & entry, ComponentThread & owner );

    /**
     * \brief   Called by system to delete component and free resources.
     * \param   compObject  The instance of component previously created by CreateComponent method.
     * \param   entry   The entry of registry, which describes the component.
     **/
    static void DeleteComponent( Component & compObject, const NERegistry::ComponentEntry & entry );

    QString getConnectedAddress(void) const;

    uint32_t getConnectedPort(void) const;

    bool isObserverConnected(void) const;

signals:
    void signalLogServiceConnected(const char* address, unsigned short port);

    void signalLogServiceDisconnected(void);

    void signalLogingStarted(void);

    void signalLogingStopped(void);

    void signalConnectedInstances(const sLogInstance* instances, uint32_t count);

    void signalDisconnectedInstances(const ITEM_ID* instances, uint32_t count);

    void signalScopesRegistered(ITEM_ID cookie, const sLogScope* scopes, uint32_t count);

    void signalScopesUpdated(ITEM_ID target, const sLogScope* scopes, uint32_t count);

    void signalLogMessageEx(const sLogMessage * message);

protected:

    /**
     * \brief   Instantiates the component object.
     * \param   entry   The entry of registry, which describes the component.
     * \param   ownerThread The instance of component owner thread.
     * \param   data        The optional component data set in system. Can be empty / no data.
     **/
    LogObserverComp(const NERegistry::ComponentEntry & entry, ComponentThread & ownerThread, NEMemory::uAlign OPT data);

    virtual ~LogObserverComp(void);

/************************************************************************/
// IELogObserverEventConsumer overrides
/************************************************************************/

    virtual void processEvent(const LogObserverEventData & data) override;

/************************************************************************/
// StubBase overrides. Triggered by Component on startup.
/************************************************************************/

    /**
     * \brief   This function is triggered by Component when starts up.
     * \param   holder  The holder component of service interface of Stub.
     **/
    virtual void startupServiceInterface( Component & holder ) override;

    /**
     * \brief   This function is triggered by Component when shuts down.
     * \param   holder  The holder component of service interface of Stub.
     **/
    virtual void shutdownServiceIntrface ( Component & holder ) override;

//////////////////////////////////////////////////////////////////////////
// These methods must exist, but can have empty body
//////////////////////////////////////////////////////////////////////////
protected:
/************************************************************************/
// StubBase overrides. Public pure virtual methods
/************************************************************************/

    /**
     * \brief   Sends update notification message to all clients.
     **/
    virtual void sendNotification( unsigned int /*msgId*/ ) override
    {
    }

    /**
     * \brief   Sends error message to clients.
     **/
    virtual void errorRequest( unsigned int /*msgId*/, bool /*msgCancel*/ ) override
    {
    }

/************************************************************************/
// IEStubEventConsumer interface overrides.
/************************************************************************/

    /**
     * \brief   Triggered to process service request event.
     **/
    virtual void processRequestEvent( ServiceRequestEvent & /*eventElem*/ ) override
    {
    }

    /**
     * \brief   Triggered to process attribute update notification event.
     **/
    virtual void processAttributeEvent( ServiceRequestEvent & /*eventElem*/ ) override
    {
    }

private:
    inline LogObserverComp & self( void )
    {
        return (*this);
    }

    void connectedInstances(const sLogInstance* instances, uint32_t count);

    void disconnectedInstances(const ITEM_ID* instances, uint32_t count);

    void logScopesRegistered(ITEM_ID target, const sLogScope* scopes, uint32_t count);

    void logScopesUpdated(ITEM_ID target, const sLogScope* scopes, uint32_t count);

    void logMessageEx(SharedBuffer & message);

    void requestChangeScopePrio(ITEM_ID target, const sLogScope* scopes, uint32_t count);

private:
    LogObserver     mLogObserver;   //!< The log observer object.
    String          mConfigFile;    //!< The path to config file.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
    LogObserverComp( void ) = delete;
    DECLARE_NOCOPY_NOMOVE( LogObserverComp );
};

#endif  // LUSAN_DATA_LOG_LOGOBSERVERCOMP_HPP
