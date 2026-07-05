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
 *  \file        lusan/model/sm/StateMachineModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document facade.
 *
 ************************************************************************/

#include "lusan/model/sm/StateMachineModel.hpp"

#include <QUndoCommand>

namespace
{
    constexpr int AutosaveIntervalMs{ 30000 };
}

StateMachineModel::StateMachineModel(QObject* parent /*= nullptr*/)
    : QObject        (parent)
    , mData          (std::make_unique<StateMachineData>())
    , mNotifier      (this)
    , mUndoStack     (this)
    , mAutosaveTimer (this)
    , mOverviewModel (*this)
    , mOpenSuccess   (false)
{
    mUndoStack.setUndoLimit(100);
    mAutosaveTimer.setSingleShot(false);
    mAutosaveTimer.setInterval(AutosaveIntervalMs);

    connect(&mAutosaveTimer, &QTimer::timeout, this, &StateMachineModel::onAutosaveTimeout);
    connect(&mUndoStack, &QUndoStack::cleanChanged, this, &StateMachineModel::onUndoCleanChanged);
}

bool StateMachineModel::createNewDocument(const QString& machineName)
{
    mData = StateMachineData::createNewDocument(machineName);
    mOpenSuccess = (mData != nullptr);
    if (mOpenSuccess == false)
    {
        return false;
    }

    mUndoStack.clear();
    mUndoStack.setClean();
    mNotifier.notifyDocumentReloaded();
    markDirty();
    updateAutosaveTimer();
    return true;
}

bool StateMachineModel::loadFromFile(const QString& documentPath, const QString& sourcePath /*= QString()*/)
{
    mOpenSuccess = false;
    if (documentPath.isEmpty())
    {
        return false;
    }

    const QString pathToRead = sourcePath.isEmpty() ? documentPath : sourcePath;
    std::unique_ptr<StateMachineData> loaded{ std::make_unique<StateMachineData>() };
    if (loaded->readFromFile(pathToRead) == false)
    {
        return false;
    }

    loaded->setFilePath(documentPath);
    mData = std::move(loaded);
    mOpenSuccess = true;

    mUndoStack.clear();
    mUndoStack.setClean();
    mNotifier.notifyDocumentReloaded();
    if (sourcePath.isEmpty() == false)
    {
        markDirty();
    }

    updateAutosaveTimer();
    return true;
}

bool StateMachineModel::saveToFile(const QString& filePath /*= QString()*/)
{
    if (mData == nullptr)
    {
        return false;
    }

    const QString previousPath = mData->getFilePath();
    if (mData->writeToFile(filePath) == false)
    {
        return false;
    }

    mUndoStack.setClean();
    StateMachineData::removeAutosave(previousPath);
    StateMachineData::removeAutosave(mData->getFilePath());
    updateAutosaveTimer();
    return true;
}

bool StateMachineModel::writeAutosave(void)
{
    if ((mData == nullptr) || mUndoStack.isClean())
    {
        return true;
    }

    const QString documentPath = mData->getFilePath();
    if (documentPath.isEmpty())
    {
        return true;
    }

    return mData->writeToAutosaveFile(StateMachineData::autosavePathForDocument(documentPath));
}

bool StateMachineModel::removeAutosave(void)
{
    return (mData != nullptr ? StateMachineData::removeAutosave(mData->getFilePath()) : true);
}

void StateMachineModel::onAutosaveTimeout(void)
{
    writeAutosave();
}

void StateMachineModel::onUndoCleanChanged(bool clean)
{
    if (clean)
    {
        removeAutosave();
    }

    emit signalDirtyChanged(!clean);
    updateAutosaveTimer();
}

void StateMachineModel::markDirty(void)
{
    if (mUndoStack.isClean())
    {
        mUndoStack.push(new QUndoCommand(tr("Document modified")));
    }
}

void StateMachineModel::updateAutosaveTimer(void)
{
    const bool enableTimer = (mData != nullptr)
                           && (mUndoStack.isClean() == false)
                           && (mData->getFilePath().isEmpty() == false);
    if (enableTimer)
    {
        mAutosaveTimer.start();
    }
    else
    {
        mAutosaveTimer.stop();
    }
}
