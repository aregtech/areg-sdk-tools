#ifndef LUSAN_VIEW_SM_SMCANVASTOOL_HPP
#define LUSAN_VIEW_SM_SMCANVASTOOL_HPP
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
 *  \file        lusan/view/sm/SMCanvasTool.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas tool strategies.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/NESMDesign.hpp"

#include <QList>
#include <QPointF>
#include <cstdint>
#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QGraphicsPathItem;
class QGraphicsSceneMouseEvent;
class QKeyEvent;
class SMScene;

/**
 * \class   SMCanvasTool
 * \brief   One tool strategy per canvas mode. The scene forwards its mouse and key
 *          events to the active tool; a handler returns true to consume the event and
 *          false to fall through to the scene's default behavior. Each gesture's state
 *          stays local to its tool; a tool reports a finished gesture through
 *          SMScene::finishToolGesture() and abandons it in cancelGesture().
 **/
class SMCanvasTool
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    explicit SMCanvasTool(SMScene& scene);

public:
    virtual ~SMCanvasTool() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the mode this tool implements.
     **/
    virtual NESMDesign::eCanvasTool getKind() const = 0;

    /**
     * \brief   Called when the tool becomes active.
     **/
    virtual void activate();

    /**
     * \brief   Called when the tool is deactivated or Esc is pressed:
     *          abandons the in-progress gesture without side effects.
     **/
    virtual void cancelGesture();

    /**
     * \brief   Event hooks; return true when the event is consumed.
     **/
    virtual bool mousePress(QGraphicsSceneMouseEvent* event);
    virtual bool mouseMove(QGraphicsSceneMouseEvent* event);
    virtual bool mouseRelease(QGraphicsSceneMouseEvent* event);
    virtual bool mouseDoubleClick(QGraphicsSceneMouseEvent* event);
    virtual bool keyPress(QKeyEvent* event);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
protected:
    inline SMScene& getScene() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMScene&    mScene; //!< The scene the tool operates on.
};

/**
 * \class   SMSelectTool
 * \brief   The default pointer: selection, multi-select, rubber-band, and item moves
 *          are the scene / view built-in behavior, so every hook falls through.
 **/
class SMSelectTool : public SMCanvasTool
{
public:
    explicit SMSelectTool(SMScene& scene);
    virtual ~SMSelectTool() = default;

    virtual NESMDesign::eCanvasTool getKind() const override;
};

/**
 * \class   SMPlaceStateTool
 * \brief   The Add State / Add Final State tool: a click places a state of the tool's
 *          kind at the (snapped) click position as one undo step and opens the in-place
 *          name editor. The scene's single-shot/sticky policy decides whether the tool
 *          stays active afterwards.
 **/
class SMPlaceStateTool : public SMCanvasTool
{
public:
    /**
     * \brief   Creates the placement tool.
     * \param   scene       The scene the tool operates on.
     * \param   finalState  True places Final states, false places Normal states.
     **/
    SMPlaceStateTool(SMScene& scene, bool finalState);
    virtual ~SMPlaceStateTool() = default;

    virtual NESMDesign::eCanvasTool getKind() const override;
    virtual bool mousePress(QGraphicsSceneMouseEvent* event) override;
    virtual bool mouseRelease(QGraphicsSceneMouseEvent* event) override;
    virtual bool mouseDoubleClick(QGraphicsSceneMouseEvent* event) override;

private:
    /**
     * \brief   Places the state centered at the given scene position.
     **/
    void placeState(const QPointF& scenePos);

private:
    const bool  mFinal; //!< True to place Final states.
};

/**
 * \class   SMTransitionTool
 * \brief   The Add Transition tool: click the source state then the target state to create
 *          an external transition (same state = self-transition; Enter on the source = an
 *          internal transition; Esc cancels). It also supports the drag gesture: press on a
 *          source, paint the edge live to the cursor, drop a waypoint on each intermediate
 *          click, and release on the target; a release on empty canvas offers an internal
 *          transition. The same drag can be started from a state border with the Select tool
 *          via beginDragFrom().
 **/
class SMTransitionTool : public SMCanvasTool
{
public:
    explicit SMTransitionTool(SMScene& scene);
    virtual ~SMTransitionTool();

    virtual NESMDesign::eCanvasTool getKind() const override;
    virtual void activate() override;
    virtual void cancelGesture() override;
    virtual bool mousePress(QGraphicsSceneMouseEvent* event) override;
    virtual bool mouseMove(QGraphicsSceneMouseEvent* event) override;
    virtual bool mouseRelease(QGraphicsSceneMouseEvent* event) override;
    virtual bool mouseDoubleClick(QGraphicsSceneMouseEvent* event) override;
    virtual bool keyPress(QKeyEvent* event) override;

    /**
     * \brief   Starts the drag gesture from a state border (Select-tool border drag).
     * \param   sourceId    The source state's element ID.
     * \param   scenePos    The press position in scene coordinates.
     **/
    void beginDragFrom(uint32_t sourceId, const QPointF& scenePos);

private:
    void createPreview();
    void clearPreview();
    void updatePreview(const QPointF& cursor);
    void resetGesture();

    /**
     * \brief   Completes the gesture as an external/self transition to the target state.
     **/
    void completeExternal(uint32_t targetId);

    /**
     * \brief   Completes the gesture as an internal transition on the source state.
     **/
    void completeInternal();

    /**
     * \brief   Offers an internal transition when the drag is released on empty canvas.
     **/
    void offerInternalOnEmpty();

    /**
     * \brief   The source state's current box geometry in scene coordinates.
     **/
    QRectF sourceRect() const;

private:
    bool                mArmed;         //!< A source has been picked.
    bool                mDragging;      //!< The gesture is a drag (past the move threshold).
    bool                mFromBorder;    //!< The gesture was started from a state border (Select tool).
    uint32_t            mSourceId;      //!< The picked source state ID.
    QPointF             mPressPos;      //!< The initial press position.
    QList<QPointF>      mWaypoints;     //!< The dropped intermediate waypoints.
    QGraphicsPathItem*  mPreview;       //!< The live edge preview, or nullptr.
};

/**
 * \brief   Creates the strategy object of the given tool mode.
 * \param   tool    The requested tool mode.
 * \param   scene   The scene the tool operates on.
 * \return  The tool object, or nullptr when the mode has no implementation yet.
 **/
std::unique_ptr<SMCanvasTool> createCanvasTool(NESMDesign::eCanvasTool tool, SMScene& scene);

//////////////////////////////////////////////////////////////////////////
// SMCanvasTool inline methods
//////////////////////////////////////////////////////////////////////////

inline SMScene& SMCanvasTool::getScene() const
{
    return mScene;
}

#endif  // LUSAN_VIEW_SM_SMCANVASTOOL_HPP
