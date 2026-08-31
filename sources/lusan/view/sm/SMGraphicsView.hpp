#ifndef LUSAN_VIEW_SM_SMGRAPHICSVIEW_HPP
#define LUSAN_VIEW_SM_SMGRAPHICSVIEW_HPP
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
 *  \file        lusan/view/sm/SMGraphicsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas viewport.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QGraphicsView>

#include "lusan/view/sm/NESMDesign.hpp"

#include <QCursor>
#include <QPoint>

/**
 * \class   SMGraphicsView
 * \brief   The canvas viewport: rubber-band selection on empty canvas, Ctrl+wheel zoom
 *          anchored under the cursor, middle-button drag panning, and the zoom
 *          in / out / 100% / fit commands with a percent-based zoom state.
 **/
class SMGraphicsView : public QGraphicsView
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGraphicsView(QWidget* parent = nullptr);
    virtual ~SMGraphicsView() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the current zoom in percent.
     **/
    inline int getZoom() const;

    /**
     * \brief   Sets the zoom in percent, clamped to the allowed range.
     **/
    void setZoom(int percent);

    /**
     * \brief   Zooms one step in / out, resets to 100%.
     **/
    void zoomIn();
    void zoomOut();
    void zoomReset();

    /**
     * \brief   Zooms and scrolls so the level content fills the viewport;
     *          an empty level resets to 100% at the scene center.
     **/
    void zoomToFit();

    /**
     * \brief   Reflects the armed canvas tool in the cursor: a crosshair while a placement
     *          tool waits for a click, the default arrow under the Select tool (issue #541).
     *          The state is remembered so a middle-button pan restores it instead of
     *          dropping back to the arrow.
     **/
    void setToolCursor(NESMDesign::eCanvasTool tool);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when the zoom changed.
     * \param   percent The new zoom in percent.
     **/
    void signalZoomChanged(int percent);

    /**
     * \brief   Emitted on a right-click / menu-key context-menu request, carrying the
     *          viewport-relative position. A QGraphicsView routes context-menu events
     *          through contextMenuEvent(), so a Qt::CustomContextMenu policy on the
     *          viewport never fires customContextMenuRequested; this signal is the
     *          reliable hook the Design page listens on.
     * \param   viewportPos The request position in viewport coordinates.
     **/
    void signalContextMenuRequested(const QPoint& viewportPos);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Puts the remembered tool cursor on the view.
     **/
    void applyToolCursor();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    int             mZoom;          //!< The zoom in percent.
    bool            mPanning;       //!< A middle-button pan is in progress.
    QPoint          mPanStart;      //!< The last pan position in viewport coordinates.
    bool            mToolArmed;     //!< A placement tool is armed; remembered to survive a pan.
};

//////////////////////////////////////////////////////////////////////////
// SMGraphicsView inline methods
//////////////////////////////////////////////////////////////////////////

inline int SMGraphicsView::getZoom() const
{
    return mZoom;
}

#endif  // LUSAN_VIEW_SM_SMGRAPHICSVIEW_HPP
