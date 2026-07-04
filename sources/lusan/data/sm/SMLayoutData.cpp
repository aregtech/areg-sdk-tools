/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMLayoutData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor-only layout data.
 *
 ************************************************************************/

#include "lusan/data/sm/SMLayoutData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMLayoutData::SMLayoutData(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mGridSize     (16)
    , mGridVisible  (true)
    , mViews        ( )
    , mNodes        ( )
    , mEdges        ( )
    , mNotes        ( )
{
}

bool SMLayoutData::isValid(void) const
{
    return true;
}

bool SMLayoutData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMLayoutData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

SMLayoutView& SMLayoutData::addView(uint32_t owner)
{
    SMLayoutView view;
    view.owner = owner;
    mViews.append(view);
    return mViews.last();
}

SMLayoutNode& SMLayoutData::addNode(uint32_t owner)
{
    SMLayoutNode node;
    node.owner = owner;
    mNodes.append(node);
    return mNodes.last();
}

SMLayoutEdge& SMLayoutData::addEdge(uint32_t owner)
{
    SMLayoutEdge edge;
    edge.owner = owner;
    mEdges.append(edge);
    return mEdges.last();
}

SMLayoutNote& SMLayoutData::addNote(uint32_t level)
{
    SMLayoutNote note;
    note.id    = getNextId();
    note.level = level;
    mNotes.append(note);
    return mNotes.last();
}

SMLayoutView* SMLayoutData::findView(uint32_t owner)
{
    for (SMLayoutView& view : mViews)
    {
        if (view.owner == owner)
        {
            return &view;
        }
    }

    return nullptr;
}

SMLayoutNode* SMLayoutData::findNode(uint32_t owner)
{
    for (SMLayoutNode& node : mNodes)
    {
        if (node.owner == owner)
        {
            return &node;
        }
    }

    return nullptr;
}

SMLayoutEdge* SMLayoutData::findEdge(uint32_t owner)
{
    for (SMLayoutEdge& edge : mEdges)
    {
        if (edge.owner == owner)
        {
            return &edge;
        }
    }

    return nullptr;
}

SMLayoutNote* SMLayoutData::findNote(uint32_t id)
{
    for (SMLayoutNote& note : mNotes)
    {
        if (note.id == id)
        {
            return &note;
        }
    }

    return nullptr;
}

int SMLayoutData::removeOwned(const QList<uint32_t>& ownerIds)
{
    int removed = 0;

    for (int i = mViews.size() - 1; i >= 0; --i)
    {
        if (ownerIds.contains(mViews.at(i).owner))
        {
            mViews.removeAt(i);
            ++removed;
        }
    }

    for (int i = mNodes.size() - 1; i >= 0; --i)
    {
        if (ownerIds.contains(mNodes.at(i).owner))
        {
            mNodes.removeAt(i);
            ++removed;
        }
    }

    for (int i = mEdges.size() - 1; i >= 0; --i)
    {
        if (ownerIds.contains(mEdges.at(i).owner))
        {
            mEdges.removeAt(i);
            ++removed;
        }
    }

    return removed;
}
