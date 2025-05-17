#ifndef LUSAN_MODEL_LOG_LOGSCOPEICONFACTORY_HPP
#define LUSAN_MODEL_LOG_LOGSCOPEICONFACTORY_HPP
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
 *  \file        lusan/model/log/LogScopeIconFactory.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes icons.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include <QIcon>

class LogScopeIconFactory
{
public:
    enum class eScopeIcon
    {
          IconNotset        = 0
        , IconScope
        , IconDebug
        , IconInfo
        , IconWarn
        , IconError
        , IconFatal
        , IconDebugInfo
        , IconDebugWarn
        , IconDebugError
        , IconDebugFatal
        , IconInfoWarn
        , IconInfoError
        , IconInfoFatal
        , IconWarnError
        , IconWarnFatal
        , IconErrorFatal
        , IconDebugInfoWarn
        , IconDebugInfoError
        , IconDebugInfoFatal
        , IconDebugWarnError
        , IconDebugWarnFatal
        , IconDebugErrorFatal
        , IconInfoWarnError
        , IconInfoWarnFatal
        , IconInfoErrorFatal
        , IconWarnErrorFatal
        , IconDebugInfoWarnError
        , IconAll
    };

    static const QIcon& getIcon(LogScopeIconFactory::eScopeIcon& scopeIcon);

    static QIcon getIcon(uint32_t scopePrio);
};

#endif  // LUSAN_MODEL_LOG_LOGSCOPEICONFACTORY_HPP
