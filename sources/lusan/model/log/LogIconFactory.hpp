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
#include <QRectF>

class QPainter;

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

    //!< The way the scope lines of the charger are drawn.
    enum class eScopeLines : uint8_t
    {
          LinesOff      = 0 //!< Scope lines are off everywhere below: corners only
        , LinesOn           //!< Scope lines are on everywhere below: the full bracket pair
        , LinesPartial      //!< Scope lines are on for some scopes below: the pair, interrupted
    };

    /**
     * \brief   What the charger of one tree node shows.
     *
     *          The charger reads as a range: the cells fill from the bottom up to the
     *          chosen level, so everything up to that level is generated. The brackets
     *          around the cells carry the scope lines.
     **/
    struct sCharger
    {
        uint8_t     level;  //!< 0 nothing, 1 Error, 2 Warning, 3 Information, 4 Debug
        uint8_t     mixed;  //!< One bit per cell, bit 0 Error to bit 3 Debug, set when the scopes below disagree
        eScopeLines lines;  //!< The state of the scope lines
        bool        frozen; //!< The process is gone, so the whole charger greys
    };

    //!< The number of cells the charger has, one per severity.
    static constexpr int    ChargerCells    { 4 };

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
     * \brief   Draws the charger inside the given box.
     *          The drawing is centred and keeps its proportions, so the box may be any size.
     * \param   painter The painter to draw with. Its state is restored on return.
     * \param   box     The rectangle to draw into.
     * \param   charger What the charger shows.
     **/
    static void paintCharger(QPainter & painter, const QRectF & box, const sCharger & charger);

    /**
     * \brief   Returns the charger as an icon of the given size. The icons are cached,
     *          and the cache is dropped when the theme changes.
     * \param   charger The charger to draw.
     * \param   pixels  The extent of the square icon.
     **/
    static QIcon chargerIcon(const sCharger & charger, uint32_t pixels = IconPixels);

    /**
     * \brief   Returns the charger that stands for the given combination of priorities.
     *          The scopes below are taken to agree, so no cell is drawn hollow.
     * \param   scopePrio   The bits of combination of scope priorities.
     **/
    static sCharger chargerOf(uint32_t scopePrio);
};

#endif  // LUSAN_MODEL_LOG_LogIconFactory_HPP
