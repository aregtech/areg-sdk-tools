#ifndef LUSAN_DATA_LOG_NELOGOBSERVER_HPP
#define LUSAN_DATA_LOG_NELOGOBSERVER_HPP
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
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/LogObserverComp.hpp"

namespace NELogObserver
{

    constexpr const char* const LogobserverThread   { "LogObserverThread" };
    constexpr const char* const LogobserverModel    { "LogObserverModel" };
    constexpr const char* const LogObserverComponent{ "LogObserverComponent" };

    bool startLobObserver(void);

    void stopLogObserver(void);

    LogObserverComp* getLogObserver(void);
}

#endif  // LUSAN_DATA_LOG_NELOGOBSERVER_HPP
