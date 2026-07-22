#ifndef LUSAN_VIEW_SM_SMSCENEMANAGER_HPP
#define LUSAN_VIEW_SM_SMSCENEMANAGER_HPP
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
 *  \file        lusan/view/sm/SMSceneManager.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas machine-level scene manager.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>

#include <QHash>
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMScene;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMSceneManager
 * \brief   Maps machine levels to their scenes and holds the level-navigation state.
 *          Scenes are created lazily on first navigation to a level and cached for the
 *          document's lifetime, so navigation cost is bound by the level size. The
 *          manager owns the current-level path (the breadcrumb source) and drops cached
 *          scenes whose level disappears from the document, falling back to the nearest
 *          surviving ancestor. It never touches the viewport; the Design page reacts to
 *          signalLevelChanged() and swaps the view's scene.
 **/
class SMSceneManager : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMSceneManager(StateMachineModel& model, QObject* parent = nullptr);
    virtual ~SMSceneManager();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the displayed level's owner element ID (the Overview ID at root).
     **/
    uint32_t getCurrentLevel() const;

    /**
     * \brief   Returns the displayed level's scene, or nullptr before the first reset().
     **/
    SMScene* getCurrentScene() const;

    /**
     * \brief   Returns the root level's owner ID (the Overview ID).
     **/
    uint32_t getRootLevel() const;

    /**
     * \brief   Returns the current level path, root level first, current level last.
     **/
    inline const QList<uint32_t>& getCurrentPath() const;

    /**
     * \brief   Returns a level's display title: the machine name for the root level,
     *          the composite state's name otherwise.
     **/
    QString levelTitle(uint32_t levelId) const;

    /**
     * \brief   Navigates to a machine level, creating and caching its scene on first
     *          visit, and updates the selection model's active level.
     * \param   levelId The level owner's element ID.
     * \return  True when the ID names an existing level.
     **/
    bool navigateTo(uint32_t levelId);

    /**
     * \brief   Descends into a composite state's painted submachine; ignored when the
     *          state owns none.
     **/
    void enterSubmachine(uint32_t stateId);

    /**
     * \brief   Ascends one level; ignored at the root level.
     **/
    void goToParent();

    /**
     * \brief   Drops every cached scene and navigates to the root level. Called when the
     *          document content is replaced.
     **/
    void reset();

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted after the displayed level changed; the current scene is ready.
     * \param   levelId The new level owner ID.
     **/
    void signalLevelChanged(uint32_t levelId);

    /**
     * \brief   Re-emitted from any level's scene: focus a transition's Conditions tab
     *          field (edge label double-click).
     **/
    void signalGuardEditRequested(uint32_t transitionId);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);

private:
    /**
     * \brief   Returns the level's cached scene, creating and wiring it on first use.
     **/
    SMScene* sceneForLevel(uint32_t levelId);

    /**
     * \brief   Leaves a level that no longer exists (nearest surviving ancestor) and
     *          drops the cached scenes of every dead level.
     **/
    void pruneDeadLevels();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&          mModel;     //!< The document facade.
    QHash<uint32_t, SMScene*>   mScenes;    //!< Level owner ID to cached scene.
    QList<uint32_t>             mPath;      //!< The current level's ancestor chain, root first.
};

//////////////////////////////////////////////////////////////////////////
// SMSceneManager inline methods
//////////////////////////////////////////////////////////////////////////

inline const QList<uint32_t>& SMSceneManager::getCurrentPath() const
{
    return mPath;
}

#endif  // LUSAN_VIEW_SM_SMSCENEMANAGER_HPP
