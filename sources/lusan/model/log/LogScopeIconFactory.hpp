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

/************************************************************************
 * Class LogScopeIconFactory
 ************************************************************************/
/**
 * \brief   The class creates icons for the log scope.
 **/ 
class LogScopeIconFactory
{
public:
    
    //!< Size of the icon in pixels
    static constexpr int    IconPixels      { 16 };

    //!< Returns the icon for the log scope priority.
    //! The priority bits are combination of scope priorities.
    /**
     * \brief   Returns the icon for the log scope priority.
     * \param   scopePrio    The bits of combination of scope priorities.
     **/
    static QIcon getIcon(uint32_t scopePrio);
};

#endif  // LUSAN_MODEL_LOG_LOGSCOPEICONFACTORY_HPP
