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
 *  \file        lusan/data/log/NELogObserver.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log observer common namespace.
 *
 ************************************************************************/

/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/NELogObserver.hpp"

#include "lusan/data/log/LogObserverComp.hpp"
#include "areg/component/ComponentLoader.hpp"


BEGIN_MODEL(NELogObserver::LogobserverModel)

    BEGIN_REGISTER_THREAD(NELogObserver::LogobserverThread, NECommon::WATCHDOG_IGNORE)
        BEGIN_REGISTER_COMPONENT(NELogObserver::LogObserverComponent, LogObserverComp )
            REGISTER_IMPLEMENT_SERVICE( NEService::EmptyServiceName, NEService::EmptyServiceVersion )
        END_REGISTER_COMPONENT(NELogObserver::LogObserverComponent)
    END_REGISTER_THREAD(NELogObserver::LogobserverThread)

END_MODEL(NELogObserver::LogobserverModel)

bool NELogObserver::startLobObserver(void)
{
    return ComponentLoader::loadComponentModel(NELogObserver::LogobserverModel);
}

void NELogObserver::stopLogObserver(void)
{
    ComponentLoader::unloadComponentModel(true, NELogObserver::LogobserverModel);
}

LogObserverComp* NELogObserver::getLogObserver(void)
{
    Component* result = Component::findComponentByName(NELogObserver::LogObserverComponent);
    return static_cast<LogObserverComp*>(result);
}
