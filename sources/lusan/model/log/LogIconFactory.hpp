#ifndef LUSAN_MODEL_LOG_LOGICONFACTORY_HPP
#define LUSAN_MODEL_LOG_LOGICONFACTORY_HPP
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
 *  \file        lusan/model/log/LogIconFactory.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes icons.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "areg/logging/areg_log.h"

#include <QColor>
#include <QIcon>

/************************************************************************
 * Class LogIconFactory
 ************************************************************************/
/**
 * \brief   The class creates icons for the log scope.
 **/ 
class LogIconFactory
{
public:

    //<! The log icons, which are used in tool buttons and menus.
    enum class eLogIcons    : uint32_t
    {
          PrioInvalid   = static_cast<uint32_t>(areg::LogPriority::PrioInvalid)
        , PrioNotset    = static_cast<uint32_t>(areg::LogPriority::PrioNotset)
        , PrioDebug     = static_cast<uint32_t>(areg::LogPriority::PrioDebug)
        , PrioInfo      = static_cast<uint32_t>(areg::LogPriority::PrioInfo)
        , PrioWarn      = static_cast<uint32_t>(areg::LogPriority::PrioWarning)
        , PrioError     = static_cast<uint32_t>(areg::LogPriority::PrioError)
        , PrioFatal     = static_cast<uint32_t>(areg::LogPriority::PrioFatal)
        , PrioScope     = static_cast<uint32_t>(areg::LogPriority::PrioScope)
        , PrioScopeEnter= static_cast<uint32_t>(areg::LogPriority::PrioScope) | 4
        , PrioScopeExit = static_cast<uint32_t>(areg::LogPriority::PrioScope) | 8
    };

    //!< The indexes of log colors, which are used in the log scope tree view and log messages.
    enum class eLogColor
    {
          ColorNotSet   = 0 //!< Not set color, used for invalid log priority
        , ColorFatal        //!< Fatal log color
        , ColorError        //!< Error log color
        , ColorWarn         //!< Warning log color
        , ColorInfo         //!< Info log color
        , ColorDebug        //!< Debug log color
        , ColorScope        //!< Scope log color, used for scope log messages
        , ColorScopeEnter   //!< Scope enter log color, used for scope enter log messages
        , ColorScopeExit    //!< Scope exit log color, used for scope exit log messages
        , ColorWithScope    //!< Scope log color, used for log messages with scope
        , ColorDefault      //!< The default color

        , ColorCount        //<!< The number of log colors
    };

    //!< Size of the icon in pixels to display in the scope navigation tree view.
    static constexpr int    IconPixels      { 16 };

    //!< Size of the icon in pixels for toolbuttons and menus.
    static constexpr int    ButtonPixels    { 42 };

    //!< Returns the icon for the log scope priority.
    //! The priority bits are combination of scope priorities.
    /**
     * \brief   Returns the icon for the log scope priority.
     * \param   scopePrio    The bits of combination of scope priorities.
     **/
    static QIcon getIcon(uint32_t scopePrio, uint32_t pixels = IconPixels);

    /**
     * \brief   Returns the icon for the log priority to display on toolbuttons and menues.
     * \param   prio        The priority of the log to display.
     * \param   active      True if the icon is active, false otherwise.
     * \param   pixels      The size of the icon in pixels.
     * \return  The icon for the log priority.
     **/
    static QIcon getLogIcon(eLogIcons prio, bool active, uint32_t pixels = ButtonPixels);

    /**
     * \brief   Returns the color for the log priority.
     * \param   logPrio      The log priority.
     **/
    static QColor getColor(areg::LogPriority logPrio);

    /**
     * \brief   Returns the color for the log priority.
     * \param   logPrio      The log priority.
     **/
    static QColor getLogColor(LogIconFactory::eLogColor logPrio);

    /**
     * \brief   Returns the color for the log message.
     * \param   logMessage   The log message to get color.
     **/
    static QColor getLogColor(const areg::LogEntry & logMessage);

    /**
     * \brief   Returns the background color for the log message based on log message priority.
     * \param   logMessage  The log message with message priority information to get background color.
     **/
    static QColor getLogBackgroundColor(const areg::LogEntry & logMessage);

    /**
     * \brief   Returns the background color for the specified log priority.
     * \param   logPrio     The log message priority.
     **/
    static QColor getLogBackgroundColor(areg::LogPriority logPrio);
};

#endif  // LUSAN_MODEL_LOG_LogIconFactory_HPP
