#ifndef LUSAN_VIEW_SM_NESMDESIGN_HPP
#define LUSAN_VIEW_SM_NESMDESIGN_HPP
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
 *  \file        lusan/view/sm/NESMDesign.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas rendering constants and helpers.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QColor>
#include <QPointF>
#include <QRectF>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QPainter;
class QPalette;

/**
 * \namespace   NESMDesign
 * \brief       View-layer constants and drawing helpers of the FSM design canvas.
 *              All values derive from the active application palette, so the canvas
 *              follows the selected theme; nothing here is persisted.
 **/
namespace NESMDesign
{
    /**
     * \brief   The canvas tool modes. One strategy object per mode handles the
     *          scene's mouse and key events while the mode is active.
     **/
    enum class eCanvasTool
    {
          Select        //!< Default pointer: select, multi-select, move.
        , AddState      //!< Place a normal state.
        , AddFinalState //!< Place a final state.
        , AddTransition //!< Draw a transition between states.
        , Waypoint      //!< Insert / remove edge waypoints.
        , AddNote       //!< Place a text annotation.
        , ColorApply    //!< Apply a color swatch to the selection.
    };

    //!< The default grid cell size in scene units (pixels at 100% zoom).
    constexpr int       GridSizeDefault { 16 };
    //!< The smallest allowed grid cell size.
    constexpr int       GridSizeMin     { 4 };

    //!< The zoom range in percent.
    constexpr int       ZoomMin         { 10 };
    constexpr int       ZoomMax         { 800 };
    //!< The default zoom in percent.
    constexpr int       ZoomDefault     { 100 };
    //!< The multiplier applied by one zoom-in / zoom-out step.
    constexpr double    ZoomStepFactor  { 1.25 };
    //!< The margin in scene units kept around the content by zoom-to-fit.
    constexpr double    ZoomFitMargin   { 48.0 };

    //!< The grid is hidden when one cell is smaller than this on screen (device pixels).
    constexpr double    GridHidePixels  { 4.0 };
    //!< The grid reaches full opacity when one cell is at least this large on screen.
    constexpr double    GridFullPixels  { 12.0 };

    //!< The fixed canvas working area of one machine level.
    constexpr double    SceneExtent     { 10000.0 };

    /**
     * \brief   Returns the canvas background color of the given palette.
     **/
    QColor canvasBackground(const QPalette& palette);

    /**
     * \brief   Returns the grid line color of the given palette.
     * \param   palette The active palette.
     * \param   opacity The grid opacity factor in range [0.0, 1.0].
     **/
    QColor gridColor(const QPalette& palette, double opacity = 1.0);

    /**
     * \brief   Returns the selection highlight color of the given palette.
     **/
    QColor selectionColor(const QPalette& palette);

    /**
     * \brief   Snaps a single coordinate to the nearest grid line.
     **/
    qreal snapValue(qreal value, int gridSize);

    /**
     * \brief   Snaps a point to the nearest grid crossing.
     **/
    QPointF snapPoint(const QPointF& point, int gridSize);

    /**
     * \brief   Paints the selection frame around an item's bounds: a highlight-colored
     *          dashed rectangle, solid when the item also has keyboard focus.
     * \param   painter     The painter, already in item coordinates.
     * \param   bounds      The item bounds to frame.
     * \param   palette     The active palette.
     * \param   hasFocus    True to paint the stronger focus variant.
     **/
    void paintSelectionFrame(QPainter* painter, const QRectF& bounds, const QPalette& palette, bool hasFocus);
}

#endif  // LUSAN_VIEW_SM_NESMDESIGN_HPP
