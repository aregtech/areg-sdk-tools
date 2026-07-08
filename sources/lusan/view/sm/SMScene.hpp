#ifndef LUSAN_VIEW_SM_SMSCENE_HPP
#define LUSAN_VIEW_SM_SMSCENE_HPP
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
 *  \file        lusan/view/sm/SMScene.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas scene of one machine level.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QGraphicsScene>

#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMCanvasTool.hpp"

#include <QHash>
#include <QList>
#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMCanvasItem;
class StateMachineModel;

/**
 * \class   SMScene
 * \brief   The graphics scene of one machine level. It renders the grid, owns the
 *          per-level item lookup (element ID to graphics item), runs the mode-based
 *          tool controller, applies grid snapping to interactive moves, and keeps the
 *          scene selection and the document selection model in sync.
 **/
class SMScene : public QGraphicsScene
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the scene of one machine level.
     * \param   model   The document facade.
     * \param   levelId The level owner's element ID (the Overview ID for the root level).
     * \param   parent  The owning object.
     **/
    SMScene(StateMachineModel& model, uint32_t levelId, QObject* parent = nullptr);
    virtual ~SMScene();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the level owner's element ID this scene displays.
     **/
    inline uint32_t getLevelId() const;

    /**
     * \brief   Returns the document facade.
     **/
    inline StateMachineModel& getModel() const;

    /**
     * \brief   The grid cell size in scene units; clamped to the allowed minimum.
     **/
    inline int getGridSize() const;
    void setGridSize(int gridSize);

    /**
     * \brief   The grid visibility.
     **/
    inline bool isGridVisible() const;
    void setGridVisible(bool visible);

    /**
     * \brief   The snap-to-grid mode applied to interactive moves and resizes.
     **/
    inline bool isSnapToGrid() const;
    void setSnapToGrid(bool snap);

    /**
     * \brief   Returns true while a mouse drag is in progress and snapping is on;
     *          items consult this to snap their interactive position changes.
     **/
    inline bool isInteractiveSnap() const;

    /**
     * \brief   Snaps a point to the grid when snapping is enabled; identity otherwise.
     **/
    QPointF snappedPosition(const QPointF& position) const;

    /**
     * \brief   Returns the active tool mode.
     **/
    inline NESMDesign::eCanvasTool getActiveTool() const;

    /**
     * \brief   Activates a tool mode.
     * \param   tool    The tool mode; modes without an implementation fall back to Select.
     * \param   sticky  True keeps the tool active after a finished gesture,
     *                  false reverts to Select (single-shot).
     **/
    void setActiveTool(NESMDesign::eCanvasTool tool, bool sticky = false);

    /**
     * \brief   Cancels the in-progress gesture and returns to the Select tool (Esc).
     **/
    void cancelActiveGesture();

    /**
     * \brief   Called by the active tool when its gesture completed; single-shot
     *          tools revert to Select.
     **/
    void finishToolGesture();

    /**
     * \brief   Returns the graphics item of an element, or nullptr.
     **/
    inline SMCanvasItem* findCanvasItem(uint32_t elementId) const;

    /**
     * \brief   Returns the bounding rectangle of the level content (all items).
     **/
    QRectF contentBounds() const;

    /**
     * \brief   Selects every element on this level.
     **/
    void selectAll();

//////////////////////////////////////////////////////////////////////////
// Internal: item registry (called by SMCanvasItem on scene changes)
//////////////////////////////////////////////////////////////////////////
public:
    void registerCanvasItem(SMCanvasItem& item);
    void unregisterCanvasItem(SMCanvasItem& item);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when the grid size, visibility, or snapping changed.
     **/
    void signalGridChanged();

    /**
     * \brief   Emitted when the active tool mode changed.
     **/
    void signalToolChanged(NESMDesign::eCanvasTool tool);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onSceneSelectionChanged();
    void onModelSelectionChanged(const QList<uint32_t>& selected);

private:
    /**
     * \brief   Moves the selected top-level items by one step (keyboard nudge).
     * \return  True when a selection was moved.
     **/
    bool nudgeSelection(int dx, int dy, bool pixelWise);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&              mModel;         //!< The document facade.
    const uint32_t                  mLevelId;       //!< The displayed level's owner element ID.
    QHash<uint32_t, SMCanvasItem*>  mItems;         //!< Element ID to graphics item.
    std::unique_ptr<SMCanvasTool>   mTool;          //!< The active tool strategy.
    bool                            mToolSticky;    //!< Keep the tool after a finished gesture.
    int                             mGridSize;      //!< The grid cell size in scene units.
    bool                            mGridVisible;   //!< The grid visibility.
    bool                            mSnapToGrid;    //!< Snap interactive moves to the grid.
    bool                            mMouseDrag;     //!< A mouse drag is in progress.
    bool                            mSyncSelection; //!< Guards the two-way selection sync.
};

//////////////////////////////////////////////////////////////////////////
// SMScene inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMScene::getLevelId() const
{
    return mLevelId;
}

inline StateMachineModel& SMScene::getModel() const
{
    return mModel;
}

inline int SMScene::getGridSize() const
{
    return mGridSize;
}

inline bool SMScene::isGridVisible() const
{
    return mGridVisible;
}

inline bool SMScene::isSnapToGrid() const
{
    return mSnapToGrid;
}

inline bool SMScene::isInteractiveSnap() const
{
    return mSnapToGrid && mMouseDrag;
}

inline NESMDesign::eCanvasTool SMScene::getActiveTool() const
{
    return (mTool != nullptr ? mTool->getKind() : NESMDesign::eCanvasTool::Select);
}

inline SMCanvasItem* SMScene::findCanvasItem(uint32_t elementId) const
{
    return mItems.value(elementId, nullptr);
}

#endif  // LUSAN_VIEW_SM_SMSCENE_HPP
