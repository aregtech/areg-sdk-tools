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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/NESMDesign.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas rendering constants and helpers.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"

#include <QColor>
#include <QFont>
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

    /**
     * \brief   The grid rendering styles: full cell lines or dots at the crossings.
     *          An application-level display preference; the document persists only
     *          the grid size and visibility.
     **/
    enum class eGridStyle
    {
          Lines //!< Grid lines forming cell squares (default).
        , Dots  //!< A dot at each grid crossing.
    };

    //!< The default grid cell size in scene units (pixels at 100% zoom).
    constexpr int       GridSizeDefault { 16 };
    //!< The smallest allowed grid cell size.
    constexpr int       GridSizeMin     { 4 };

    //!< The dot diameter of the dotted grid style, in device pixels. Configurable per the
    //!< user's display preference; a small default that still reads clearly on the canvas.
    constexpr int       GridDotSizeDefault { 3 };
    //!< The allowed dot-diameter range (device pixels).
    constexpr int       GridDotSizeMin     { 1 };
    constexpr int       GridDotSizeMax     { 10 };

    //!< The zoom range in percent.
    constexpr int       ZoomMin         { 10 };
    constexpr int       ZoomMax         { 800 };
    //!< The default zoom in percent.
    constexpr int       ZoomDefault     { 100 };
    //!< The multiplier applied by one zoom-in / zoom-out step.
    constexpr double    ZoomStepFactor  { 1.25 };
    //!< The margin in scene units kept around the content by zoom-to-fit.
    constexpr double    ZoomFitMargin   { 48.0 };

    //!< The text size of the zoom box in device-independent pixels.
    constexpr int       ZoomTextSize    { 9 };
    //!< The gap between the canvas edge and the zoom box.
    constexpr int       ZoomBoxIndent   { 2 };
    //!< The characters the closed zoom box makes room for.
    constexpr int       ZoomBoxChars    { 5 };

    //!< The grid is hidden when one cell is smaller than this on screen (device pixels).
    constexpr double    GridHidePixels  { 4.0 };
    //!< The grid reaches full opacity when one cell is at least this large on screen.
    constexpr double    GridFullPixels  { 12.0 };

    /**
     * \brief   True for the tools that AIM: the ones that drop a brand-new element exactly where
     *          the user clicks, and therefore swap the pointer for a crosshair. Every other mode
     *          keeps the ordinary pointer, because it acts on what is already on the canvas
     *          (Select, Waypoint, ColorApply) or, like AddNote, is placed loosely enough that a
     *          crosshair only makes the canvas feel stuck in a mode (issue #543).
     *
     *          This is the whole list -- edit it to change which tools aim.
     **/
    constexpr bool toolAims(eCanvasTool tool)
    {
        return (tool == eCanvasTool::AddState)
            || (tool == eCanvasTool::AddFinalState)
            || (tool == eCanvasTool::AddTransition);
    }

    //!< The full span of the crosshair cursor shown while an aiming tool is armed, in
    //!< device-independent pixels. Kept odd so the hot spot lands on a single center pixel.
    //!< The system Qt::CrossCursor is about twice this and reads as heavy on a dense canvas;
    //!< a value of 0 or less falls back to it.
    constexpr int       ToolCursorSize      { 15 };
    //!< The stroke width of the crosshair, in device-independent pixels.
    constexpr int       ToolCursorWidth     { 1 };

    //!< The narrowest the Design page's Properties panel ever asks to be, in device-independent
    //!< pixels. It is a CEILING on the panel's minimum width, not a starting size: nothing the
    //!< panel shows may widen it, so the dock separator keeps its full travel and the width stays
    //!< the user's to set. Raise it if the panel's controls become unusable when squeezed.
    constexpr int       PanelMinWidth   { 180 };

    //!< The shortest the Design page's Properties panel ever asks to be, in device-independent
    //!< pixels. It is a CEILING on the panel's minimum height: the panel scrolls its content
    //!< instead of growing, so a tall selection cannot push the Design page past the height the
    //!< editor window has, which would carry the page tabs out of sight.
    constexpr int       PanelMinHeight  { 120 };

    //!< The width the Design page's Properties dock is given when the page is built. A dock that
    //!< was never assigned a width follows its widget's size hint, so the first selection that
    //!< fills the panel would push the canvas edge aside; handing it this width once makes the
    //!< width the user's from the start. Shares NELusanCommon::MIN_NAVI_WIDTH with the Navigation
    //!< Window so the Design page opens with the left and right docks the same width.
    constexpr int       PanelDefaultWidth   { static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH) };

    //!< The fixed canvas working area of one machine level.
    constexpr double    SceneExtent     { 10000.0 };

    //!< The default box size of a newly placed state.
    constexpr double    StateDefaultWidth   { 160.0 };
    constexpr double    StateDefaultHeight  { 96.0 };
    //!< The default box size of a Start / Final marker state (compact pill box):
    //!< 4 x 2 default grid cells (issue #514), so the marker spans whole grid squares.
    constexpr double    MarkerStateWidth    { 64.0 };
    constexpr double    MarkerStateHeight   { 32.0 };
    //!< The smallest box a marker (Start / Final) can be resized to (a marker pill is
    //!< intentionally compact, so it may go smaller than a normal state box).
    constexpr double    MarkerStateMinWidth { 52.0 };
    constexpr double    MarkerStateMinHeight{ 26.0 };
    //!< The smallest box a state can be resized to.
    constexpr double    StateMinWidth       { 64.0 };
    constexpr double    StateMinHeight      { 48.0 };
    //!< The state box corner radius.
    constexpr double    StateCornerRadius   { 8.0 };
    //!< The header band height (name and badges).
    constexpr double    StateHeaderHeight   { 24.0 };
    //!< One behavior row's height in the state body. Kept tight so the action, event and timer rows
    //!< of one Enter/Internal/Exit group read as a single compact cluster (the zone glyph and continuation
    //!< cue still separate one group from the next); the extra slack falls between the zone bands.
    constexpr double    StateRowHeight      { 14.0 };
    //!< The horizontal text padding inside the state box.
    constexpr double    StatePadding        { 8.0 };
    //!< The body text size relative to the state name. The name is the box's one bold, full-size
    //!< label; the behavior rows below it are running text and step down from it.
    constexpr double    StateBodyFontScale  { 0.85 };
    /**
     * \enum    eBandStyle
     * \brief   How the state body says WHEN a group of operations runs.
     **/
    enum class eBandStyle
    {
          Glyph //!< The drawn marks `->|` and `<-|`: one 12 px column, and the same in every language.
        , Word  //!< The plain words `entry` and `exit`: no mark to learn, about 32 px more per row.
    };

    //!< THE switch between the two band encodings. Both are built and kept working; change this one
    //!< line to try the other on a real diagram. The words are also used on their own whenever the
    //!< kind marks are in their word style, because then nothing is drawn at all.
    constexpr eBandStyle StateBandStyle     { eBandStyle::Glyph };

    //!< How present the `entry` / `exit` words are beside the action they label, and the overflow
    //!< `...` marker with them. Structure, not content: a step below the action names, and still
    //!< above the contrast floor for small text on the body fill.
    constexpr double    StateBandWordAlpha  { 0.68 };
    //!< The gap between the band column and the action column beside it.
    constexpr double    StateBandGap        { 5.0 };
    //!< The narrowest the action column may be indented. It keeps an internal transition's
    //!< operations sitting under their `on ...` header even on a box with no entry or exit band
    //!< to give the column its width.
    constexpr double    StateBodyIndentMin  { 10.0 };
    //!< The side length of a selection resize handle.
    constexpr double    HandleSize          { 7.0 };
    //!< The darkening factor applied to the body color to derive the header shade.
    constexpr int       HeaderShadeFactor   { 145 };

    //!< The radius of the begin dot drawn on the source state border.
    constexpr double    EdgeBeginDotRadius  { 3.0 };
    //!< The arrowhead length and half-width at the target border.
    constexpr double    EdgeArrowLength     { 10.0 };
    constexpr double    EdgeArrowHalfWidth  { 4.0 };
    //!< The drawn width of an edge line (a little narrower than a full 2px stroke so the
    //!< transition reads as a fine line, not a rule).
    constexpr double    EdgeLineWidth       { 1.0 };
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
    //!< The bulge a transition takes when it is first curved: enough to read as a curve at a
    //!< glance, shallow enough not to collide with what the straight line already cleared.
    constexpr double    EdgeArcBulgeDefault { 0.35 };
    //!< The bulge limit in either direction. Past a half-circle (1.0) the arc doubles back on
    //!< its own chord and its endpoints stop describing where the transition runs.
    constexpr double    EdgeArcBulgeMax     { 1.0 };
    //!< The bulge limit of a SELF-loop. Both of its anchors sit on the same box, only a short
    //!< chord apart, so a loop that reads at all must double back over that chord -- the very
    //!< thing \ref EdgeArcBulgeMax rules out for a transition running between two boxes.
    constexpr double    EdgeArcSelfBulgeMax { 3.0 };
    //!< How far a self-loop stands off the border of its own box: the depth of the default
    //!< polyline loop's corners, and the apex height an arc self-loop is seeded with, so
    //!< switching a loop between the two shapes keeps it the same size.
    constexpr double    EdgeSelfLoopStandoff{ 44.0 };
    //!< Half the span between the two anchors of a default self-loop on its box border.
    constexpr double    EdgeSelfLoopHalfSpan{ 22.0 };
    //!< How far off horizontal/vertical a transition's first or last leg may sit and still be
    //!< read as "meant to be straight", and squared to the border. Beyond it the angle is the
    //!< user's and the anchor stays where they put it.
    constexpr double    EdgeSquareToleranceDeg { 8.0 };
    //!< How far outside a state box a press still counts as landing on it when starting or
    //!< finishing a transition. Aiming at a 1px border is not a reasonable thing to ask.
    constexpr double    StatePickMargin     { 3.0 };
    //!< The gap kept between the label and the edge line.
    constexpr double    EdgeLabelGap        { 4.0 };
    //!< The band around a state box border from which a transition drag can be started.
    constexpr double    EdgeBorderDragMargin{ 6.0 };
    //!< The font scale of on-canvas edge labels (trigger signature, guard, operation summary),
    //!< relative to the application font: a little smaller so the labels stay compact.
    constexpr double    EdgeLabelFontScale  { 0.85 };
    //!< The font scale of a state-body behavior row, relative to the application font.
    constexpr double    StateRowFontScale   { 0.80 };
    //!< The largest distance an edge's movable label block may sit from the nearest point of the
    //!< transition line. The label follows mouse or arrow keys but is clamped to this radius so it
    //!< always reads as belonging to its edge (issue #532 -- movable trigger/operation text).
    constexpr double    EdgeLabelMaxOffset  { 30.0 };

    //!< The largest size of the submachine miniature hint in a composite state's body.
    constexpr double    MiniatureMaxWidth   { 46.0 };
    constexpr double    MiniatureMaxHeight  { 30.0 };
    //!< The gap between the miniature and the state box border.
    constexpr double    MiniaturePadding    { 5.0 };

    //!< The default box size of a newly placed note.
    constexpr double    NoteDefaultWidth    { 140.0 };
    constexpr double    NoteDefaultHeight   { 90.0 };
    //!< The smallest box a note can be resized to.
    constexpr double    NoteMinWidth        { 48.0 };
    constexpr double    NoteMinHeight       { 32.0 };
    //!< The note box corner radius and text padding.
    constexpr double    NoteCornerRadius    { 4.0 };
    constexpr double    NotePadding         { 6.0 };
    //!< The side length of the single bottom-right resize handle.
    constexpr double    NoteHandleSize      { 9.0 };

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
     * \brief   Returns the point on a rounded rectangle's border along the ray from its
     *          center toward \p towards. With \p radius 0 this equals borderPoint().
     **/
    QPointF borderPoint(const QRectF& rect, double radius, const QPointF& towards);

    /**
     * \brief   Returns the point on a rounded rectangle's border closest to \p point.
     *          Works for points inside and outside the rectangle; used to glue a
     *          user-placed edge endpoint to the state box border.
     **/
    QPointF nearestBorderPoint(const QRectF& rect, double radius, const QPointF& point);

    /**
     * \brief   Returns a border point that both sticks to the box border AND aligns to the grid:
     *          the nearest border point, then slid along its edge to the nearest half-grid step so
     *          it lands exactly on a grid crossing or midway between two crossings (issue #532 --
     *          the endpoint no longer jitters as the mouse or connected box moves). The tangential
     *          coordinate is clamped to the straight span so it never rides onto a rounded corner.
     **/
    QPointF gridAlignedBorderPoint(const QRectF& rect, double radius, const QPointF& point, int gridSize);

    /**
     * \brief   Keeps \p border on the side it already sits on, but slides it along that side to
     *          follow \p pointer. Only the coordinate that runs along the chosen edge moves (x on a
     *          top/bottom edge, y on a left/right edge); the pinned coordinate stays on the border.
     *          The side is taken from \p border (typically a center-facing borderPoint()), so a
     *          transition endpoint follows where the user pressed/released along the facing side
     *          instead of hopping to whichever side is merely nearest the pointer. With \p snap the
     *          sliding coordinate rounds to the nearest half-grid step; the span is clamped so the
     *          endpoint never rides onto a rounded corner.
     * \param   rect        The state box geometry in scene coordinates.
     * \param   radius      The box corner radius.
     * \param   border      A point already on the box border; its edge selects the slide axis.
     * \param   pointer     The pointer position whose along-edge coordinate the result follows.
     * \param   gridSize    The grid cell size; the slide snaps to half of it.
     * \param   snap        When false the sliding coordinate follows the pointer exactly (no grid).
     **/
    QPointF slideBorderPoint(const QRectF& rect, double radius, const QPointF& border
                           , const QPointF& pointer, int gridSize, bool snap);

    /**
     * \brief   Returns the outward unit normal of the box side \p border sits on (the side it is
     *          nearest to). Used to push a self-loop's corner straight away from the box, so the
     *          loop stands off whichever border its anchors ended up on.
     **/
    QPointF borderOutwardNormal(const QRectF& rect, const QPointF& border);

    /**
     * \brief   True when \p direction lies within \ref EdgeSquareToleranceDeg of horizontal or
     *          vertical -- i.e. the user was drawing a straight leg and missed by a hair.
     **/
    bool isNearAxis(const QPointF& direction);

    /**
     * \brief   Returns the endpoint anchor of a polyline edge whose adjacent corner is \p neighbour,
     *          for an edge that has no anchor of its own to honour (a reloaded or auto-placed one).
     *          The facing side is chosen center-to-neighbour; the anchor then slides along that side
     *          to line up with the neighbour ONLY when that leg is already within
     *          \ref EdgeSquareToleranceDeg of an axis, so a near-straight leg is tidied to exactly
     *          straight while a deliberate diagonal is left alone.
     *          No grid rounding: the waypoint is already snapped when snap-to-grid is on, and
     *          re-rounding here would break the very alignment the anchor is following.
     * \param   rect        The state box geometry in scene coordinates.
     * \param   radius      The box corner radius.
     * \param   neighbour   The waypoint next to this endpoint.
     **/
    QPointF polylineBorderPoint(const QRectF& rect, double radius, const QPointF& neighbour);

    /**
     * \brief   Returns the endpoint anchor of a polyline leg the user just drew. The anchor keeps
     *          the border position they pressed on (\p chosen) and is squared to \p neighbour only
     *          when the resulting leg is within \ref EdgeSquareToleranceDeg of an axis. This is what
     *          keeps a deliberately angled first leg from snapping to the waypoint's row or column.
     * \param   rect        The state box geometry in scene coordinates.
     * \param   radius      The box corner radius.
     * \param   chosen      Where the user pressed / released, in scene coordinates.
     * \param   neighbour   The waypoint next to this endpoint.
     * \param   gridSize    The grid step used when \p snap is set.
     * \param   snap        Whether snap-to-grid is on.
     **/
    QPointF polylineAnchorPoint(  const QRectF& rect, double radius, const QPointF& chosen
                                , const QPointF& neighbour, int gridSize, bool snap);

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
     * \brief   Returns the fill color of a Start state's marker box ("go" green).
     **/
    QColor startStateColor(const QPalette& palette);

    /**
     * \brief   Returns the fill color of a Final state's marker box ("stop" red).
     **/
    QColor finalStateColor(const QPalette& palette);

    /**
     * \brief   Returns the state border color of the given palette.
     **/
    QColor stateBorderColor(const QPalette& palette);

    /**
     * \brief   Returns the default note fill color of the given palette (a soft, sticky-note
     *          tint distinct from the state body color).
     **/
    QColor noteColor(const QPalette& palette);

    /**
     * \brief   Derives the header shade of a body color (always darker).
     **/
    QColor deriveHeaderShade(const QColor& bodyColor);

    /**
     * \brief   Returns a readable text color (light or dark) for the given fill.
     **/
    QColor contrastTextColor(const QColor& fill);

    /**
     * \brief   Returns the palette every canvas colour is taken from.
     * \note    A widget palette is not usable here. A style sheet writes its own colours into
     *          the palette of the widgets it matches, and the sheet of the next theme does not
     *          always overwrite them, so a widget can still carry a colour of the theme before
     *          the last one. The application palette is replaced whole on every theme.
     **/
    QPalette canvasPalette(void);

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
     * \brief   Returns the grid dot color of the given palette. Stronger than the line
     *          color (dots are sparse, so they need more contrast to read at a small size).
     * \param   palette The active palette.
     * \param   opacity The grid opacity factor in range [0.0, 1.0].
     **/
    QColor gridDotColor(const QPalette& palette, double opacity = 1.0);

    /**
     * \brief   Returns the selection highlight color of the given palette.
     **/
    QColor selectionColor(const QPalette& palette);

    /**
     * \brief   Returns \p base scaled by \p scale, preserving whichever of point size or pixel
     *          size the base font uses (so it works whether the app font is point- or pixel-sized).
     **/
    QFont scaledFont(const QFont& base, double scale);

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
