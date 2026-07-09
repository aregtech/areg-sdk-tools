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
#include <QList>
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

    //!< The default box size of a newly placed state.
    constexpr double    StateDefaultWidth   { 160.0 };
    constexpr double    StateDefaultHeight  { 96.0 };
    //!< The smallest box a state can be resized to.
    constexpr double    StateMinWidth       { 64.0 };
    constexpr double    StateMinHeight      { 48.0 };
    //!< The state box corner radius.
    constexpr double    StateCornerRadius   { 8.0 };
    //!< The header band height (name and badges).
    constexpr double    StateHeaderHeight   { 24.0 };
    //!< One behavior row's height in the state body.
    constexpr double    StateRowHeight      { 16.0 };
    //!< The horizontal text padding inside the state box.
    constexpr double    StatePadding        { 8.0 };
    //!< The side length of a selection resize handle.
    constexpr double    HandleSize          { 7.0 };
    //!< The darkening factor applied to the body color to derive the header shade.
    constexpr int       HeaderShadeFactor   { 145 };

    //!< The radius of the begin dot drawn on the source state border.
    constexpr double    EdgeBeginDotRadius  { 3.5 };
    //!< The arrowhead length and half-width at the target border.
    constexpr double    EdgeArrowLength     { 12.0 };
    constexpr double    EdgeArrowHalfWidth  { 5.0 };
    //!< The drawn width of an edge line.
    constexpr double    EdgeLineWidth       { 1.6 };
    //!< The side length of a waypoint handle drawn on a selected edge.
    constexpr double    WaypointHandleSize  { 7.0 };
    //!< The pick radius for grabbing an endpoint or waypoint with the mouse.
    constexpr double    EndpointPickRadius  { 8.0 };
    //!< The pick radius within which a double-click merges (removes) a waypoint.
    constexpr double    WaypointMergeRadius { 7.0 };
    //!< The maximum distance from a segment at which a double-click inserts a waypoint.
    constexpr double    SegmentPickTolerance{ 6.0 };
    //!< The number of straight sections an arc edge is sampled into for drawing and hit test.
    constexpr int       EdgeArcSamples      { 28 };
    //!< The gap kept between the label and the edge line.
    constexpr double    EdgeLabelGap        { 4.0 };
    //!< The band around a state box border from which a transition drag can be started.
    constexpr double    EdgeBorderDragMargin{ 6.0 };

    //!< The largest size of the submachine miniature hint in a composite state's body.
    constexpr double    MiniatureMaxWidth   { 46.0 };
    constexpr double    MiniatureMaxHeight  { 30.0 };
    //!< The gap between the miniature and the state box border.
    constexpr double    MiniaturePadding    { 5.0 };

    /**
     * \brief   Returns the default transition edge color of the given palette.
     **/
    QColor edgeColor(const QPalette& palette);

    /**
     * \brief   Returns the highlight color for an incoming edge (enters a selected state).
     **/
    QColor edgeIncomingColor(const QPalette& palette);

    /**
     * \brief   Returns the highlight color for an outgoing edge (leaves a selected state).
     **/
    QColor edgeOutgoingColor(const QPalette& palette);

    /**
     * \brief   Returns the point on a rectangle's border along the ray from its center
     *          toward \p towards. Used to anchor an edge endpoint on a state box border.
     **/
    QPointF borderPoint(const QRectF& rect, const QPointF& towards);

    /**
     * \brief   Samples a circular arc through \p begin and \p end with the given signed
     *          bulge (arc height over half chord; sign picks the bowing side, 1 = semicircle)
     *          into a polyline. Returns the plain chord when the bulge is ~0 or degenerate.
     **/
    QList<QPointF> arcPolyline(const QPointF& begin, const QPointF& end, double bulge, int samples);

    /**
     * \brief   Returns the default state body fill color of the given palette.
     **/
    QColor stateBodyColor(const QPalette& palette);

    /**
     * \brief   Returns the state border color of the given palette.
     **/
    QColor stateBorderColor(const QPalette& palette);

    /**
     * \brief   Derives the header shade of a body color (always darker, spec-fixed).
     **/
    QColor deriveHeaderShade(const QColor& bodyColor);

    /**
     * \brief   Returns a readable text color (light or dark) for the given fill.
     **/
    QColor contrastTextColor(const QColor& fill);

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
