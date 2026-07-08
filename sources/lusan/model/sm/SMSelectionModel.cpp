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
 *  \file        lusan/model/sm/SMSelectionModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document-wide selection model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMSelectionModel.hpp"

SMSelectionModel::SMSelectionModel(QObject* parent /*= nullptr*/)
    : QObject       (parent)
    , mSelection    ( )
    , mActiveLevel  (0)
{
}

void SMSelectionModel::setSelection(const QList<uint32_t>& ids)
{
    QList<uint32_t> unique;
    unique.reserve(ids.size());
    for (uint32_t id : ids)
    {
        if (unique.contains(id) == false)
        {
            unique.append(id);
        }
    }

    if (unique != mSelection)
    {
        mSelection = std::move(unique);
        emit signalSelectionChanged(mSelection);
    }
}

void SMSelectionModel::select(uint32_t id)
{
    if (mSelection.contains(id) == false)
    {
        mSelection.append(id);
        emit signalSelectionChanged(mSelection);
    }
}

void SMSelectionModel::deselect(uint32_t id)
{
    if (mSelection.removeAll(id) > 0)
    {
        emit signalSelectionChanged(mSelection);
    }
}

void SMSelectionModel::toggle(uint32_t id)
{
    if (mSelection.removeAll(id) == 0)
    {
        mSelection.append(id);
    }

    emit signalSelectionChanged(mSelection);
}

void SMSelectionModel::clearSelection()
{
    if (mSelection.isEmpty() == false)
    {
        mSelection.clear();
        emit signalSelectionChanged(mSelection);
    }
}

void SMSelectionModel::setActiveLevel(uint32_t levelId)
{
    if (levelId != mActiveLevel)
    {
        clearSelection();
        mActiveLevel = levelId;
        emit signalActiveLevelChanged(mActiveLevel);
    }
}

void SMSelectionModel::reset()
{
    clearSelection();
    if (mActiveLevel != 0)
    {
        mActiveLevel = 0;
        emit signalActiveLevelChanged(mActiveLevel);
    }
}
