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

#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
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
