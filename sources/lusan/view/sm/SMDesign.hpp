#ifndef LUSAN_VIEW_SM_SMDESIGN_HPP
#define LUSAN_VIEW_SM_SMDESIGN_HPP
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
 *  \file        lusan/view/sm/SMDesign.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor Design page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QAction;
class QKeyEvent;
class SMGraphicsView;
class SMScene;
class StateMachineModel;

/**
 * \class   SMDesign
 * \brief   The Design page: the graphics viewport over the machine-level scene, plus
 *          the viewport and grid commands with their keyboard shortcuts. The commands
 *          are exposed as actions so the toolbar, Design menu, and context menus can
 *          dispatch to the same set.
 **/
class SMDesign : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMDesign(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMDesign() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the scene of the displayed machine level.
     **/
    inline SMScene& getScene() const;

    /**
     * \brief   Returns the canvas viewport.
     **/
    inline SMGraphicsView& getView() const;

    /**
     * \brief   The viewport and grid command actions.
     **/
    inline QAction* actionZoomIn() const;
    inline QAction* actionZoomOut() const;
    inline QAction* actionZoomReset() const;
    inline QAction* actionZoomFit() const;
    inline QAction* actionToggleGrid() const;
    inline QAction* actionToggleSnap() const;
    inline QAction* actionSelectAll() const;

    /**
     * \brief   The state editing actions: placement tools, delete, and rename.
     **/
    inline QAction* actionAddState() const;
    inline QAction* actionAddFinalState() const;
    inline QAction* actionAddTransition() const;
    inline QAction* actionDelete() const;
    inline QAction* actionRename() const;

    /**
     * \brief   The transition editing actions: stimulus picker and priority reorder.
     **/
    inline QAction* actionSetStimulus() const;
    inline QAction* actionRaisePriority() const;
    inline QAction* actionLowerPriority() const;

    /**
     * \brief   Deletes the current selection after confirmation: selected states (with
     *          painted substates and connected transitions), or selected transitions, as
     *          one undo step. Start states are never deleted.
     **/
    void deleteSelection();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Claims the page's shortcuts while the canvas has focus, so
     *          window-wide shortcuts of other pages cannot swallow them.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onDocumentReloaded();

private:
    void setupActions();

    /**
     * \brief   Deletes the selected transition edges after confirmation, one undo step.
     **/
    void deleteSelectedEdges();

    /**
     * \brief   Reorders the single selected transition by one step in document order
     *          (priority); \p raise moves it earlier (higher priority).
     **/
    void reorderSelectedTransition(bool raise);

    /**
     * \brief   Opens a picker over the shared registries to set the selected transition's
     *          stimulus (kind + name).
     **/
    void setStimulusOfSelection();

    /**
     * \brief   Returns the page action bound to the pressed key, or nullptr.
     **/
    QAction* matchAction(const QKeyEvent& event) const;

    /**
     * \brief   Creates the scene of the document's root level and attaches it to the view.
     **/
    void rebuildScene();

    /**
     * \brief   Applies the document's persisted grid settings to the scene.
     **/
    void applyGridSettings();

    /**
     * \brief   Fills the scene with synthetic nodes when the
     *          LUSAN_SM_CANVAS_STRESS environment variable is set (performance check).
     **/
    void populateStressContent();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    SMGraphicsView*     mView;          //!< The canvas viewport.
    SMScene*            mScene;         //!< The displayed level's scene.
    QAction*            mActZoomIn;     //!< Zoom one step in.
    QAction*            mActZoomOut;    //!< Zoom one step out.
    QAction*            mActZoomReset;  //!< Zoom to 100%.
    QAction*            mActZoomFit;    //!< Zoom to fit the level content.
    QAction*            mActToggleGrid; //!< Show / hide the grid.
    QAction*            mActToggleSnap; //!< Toggle snap-to-grid.
    QAction*            mActSelectAll;  //!< Select every element on the level.
    QAction*            mActAddState;   //!< Activate the Add State tool.
    QAction*            mActAddFinal;   //!< Activate the Add Final State tool.
    QAction*            mActAddTransition; //!< Activate the Add Transition tool.
    QAction*            mActDelete;     //!< Delete the selection with confirmation.
    QAction*            mActRename;     //!< Rename the selected state in place.
    QAction*            mActSetStimulus;//!< Set the selected transition's stimulus.
    QAction*            mActRaisePriority; //!< Raise the selected transition's priority.
    QAction*            mActLowerPriority; //!< Lower the selected transition's priority.
};

//////////////////////////////////////////////////////////////////////////
// SMDesign inline methods
//////////////////////////////////////////////////////////////////////////

inline SMScene& SMDesign::getScene() const
{
    return *mScene;
}

inline SMGraphicsView& SMDesign::getView() const
{
    return *mView;
}

inline QAction* SMDesign::actionZoomIn() const
{
    return mActZoomIn;
}

inline QAction* SMDesign::actionZoomOut() const
{
    return mActZoomOut;
}

inline QAction* SMDesign::actionZoomReset() const
{
    return mActZoomReset;
}

inline QAction* SMDesign::actionZoomFit() const
{
    return mActZoomFit;
}

inline QAction* SMDesign::actionToggleGrid() const
{
    return mActToggleGrid;
}

inline QAction* SMDesign::actionToggleSnap() const
{
    return mActToggleSnap;
}

inline QAction* SMDesign::actionSelectAll() const
{
    return mActSelectAll;
}

inline QAction* SMDesign::actionAddState() const
{
    return mActAddState;
}

inline QAction* SMDesign::actionAddFinalState() const
{
    return mActAddFinal;
}

inline QAction* SMDesign::actionAddTransition() const
{
    return mActAddTransition;
}

inline QAction* SMDesign::actionDelete() const
{
    return mActDelete;
}

inline QAction* SMDesign::actionRename() const
{
    return mActRename;
}

inline QAction* SMDesign::actionSetStimulus() const
{
    return mActSetStimulus;
}

inline QAction* SMDesign::actionRaisePriority() const
{
    return mActRaisePriority;
}

inline QAction* SMDesign::actionLowerPriority() const
{
    return mActLowerPriority;
}

#endif  // LUSAN_VIEW_SM_SMDESIGN_HPP
