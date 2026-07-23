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
 *  \file        lusan/view/sm/SMSceneManager.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas machine-level scene manager.
 *
 ************************************************************************/

#include "lusan/view/sm/SMSceneManager.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMScene.hpp"

SMSceneManager::SMSceneManager(StateMachineModel& model, QObject* parent /*= nullptr*/)
    : QObject   (parent)
    , mModel    (model)
    , mScenes   ( )
    , mPath     ( )
{
    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMSceneManager::onElementRemoved);
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMSceneManager::onElementChanged);
}

SMSceneManager::~SMSceneManager()
{
    qDeleteAll(mScenes);
    mScenes.clear();
}

uint32_t SMSceneManager::getCurrentLevel() const
{
    return (mPath.isEmpty() ? getRootLevel() : mPath.last());
}

SMScene* SMSceneManager::getCurrentScene() const
{
    return mScenes.value(getCurrentLevel(), nullptr);
}

uint32_t SMSceneManager::getRootLevel() const
{
    return mModel.getData().getOverview().getId();
}

QString SMSceneManager::levelTitle(uint32_t levelId) const
{
    const StateMachineData& data = mModel.getData();
    if (levelId == data.getOverview().getId())
    {
        const QString& name = data.getOverview().getName();
        return (name.isEmpty() ? tr("State Machine") : name);
    }

    const SMStateEntry* state = data.findStateById(levelId);
    return (state != nullptr ? state->getName() : QString());
}

bool SMSceneManager::navigateTo(uint32_t levelId)
{
    if (mModel.getData().findLevel(levelId) == nullptr)
    {
        return false;
    }

    if ((getCurrentLevel() == levelId) && mScenes.contains(levelId))
    {
        return true;
    }

    SMScene* scene = sceneForLevel(levelId);
    if (scene == nullptr)
    {
        return false;
    }

    mPath = mModel.getData().getLevelPath(levelId);
    mModel.getSelectionModel().setActiveLevel(levelId);
    emit signalLevelChanged(levelId);
    return true;
}

void SMSceneManager::enterSubmachine(uint32_t stateId)
{
    const SMStateEntry* state = mModel.getData().findStateById(stateId);
    if ((state != nullptr) && state->hasNestedStates())
    {
        navigateTo(stateId);
    }
}

void SMSceneManager::goToParent()
{
    if (mPath.size() >= 2)
    {
        navigateTo(mPath.at(mPath.size() - 2));
    }
}

void SMSceneManager::reset()
{
    const QList<SMScene*> stale{ mScenes.values() };
    mScenes.clear();
    mPath.clear();

    navigateTo(getRootLevel());

    // The view was switched to the fresh root scene by the level change; the old scenes
    // may still be inside their own signal handlers, so they die on the event loop.
    for (SMScene* scene : stale)
    {
        scene->deleteLater();
    }
}

void SMSceneManager::onElementRemoved(uint32_t /*id*/, eDocElementKind kind)
{
    if (kind == eDocElementKind::State)
    {
        pruneDeadLevels();
    }
}

void SMSceneManager::onElementChanged(uint32_t /*id*/, eDocElementKind kind)
{
    // A composite can revert to a plain state (undo of the conversion): its level dies.
    if (kind == eDocElementKind::State)
    {
        pruneDeadLevels();
    }
}

SMScene* SMSceneManager::sceneForLevel(uint32_t levelId)
{
    SMScene* scene = mScenes.value(levelId, nullptr);
    if (scene == nullptr)
    {
        scene = new SMScene(mModel, levelId, this);
        mScenes.insert(levelId, scene);
        connect(scene, &SMScene::signalEnterSubmachine, this, &SMSceneManager::enterSubmachine);
        connect(scene, &SMScene::signalGoToParent, this, &SMSceneManager::goToParent);
        connect(scene, &SMScene::signalGuardEditRequested, this, &SMSceneManager::signalGuardEditRequested);
        connect(scene, &SMScene::signalRequestSubstate, this, &SMSceneManager::signalRequestSubstate);
    }

    return scene;
}

void SMSceneManager::pruneDeadLevels()
{
    const StateMachineData& data = mModel.getData();
    if (data.findLevel(getCurrentLevel()) == nullptr)
    {
        // The displayed level died; the last-known path leads to the nearest survivor.
        uint32_t target = getRootLevel();
        for (int i = mPath.size() - 2; i >= 0; --i)
        {
            if (data.findLevel(mPath.at(i)) != nullptr)
            {
                target = mPath.at(i);
                break;
            }
        }

        navigateTo(target);
    }

    for (auto it = mScenes.begin(); it != mScenes.end(); )
    {
        if (data.findLevel(it.key()) == nullptr)
        {
            it.value()->deleteLater();
            it = mScenes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
