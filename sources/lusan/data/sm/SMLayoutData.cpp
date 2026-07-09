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
 *  \file        lusan/data/sm/SMLayoutData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor-only layout data.
 *
 ************************************************************************/

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/common/XmlSM.hpp"

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

bool SMLayoutData::isValid() const
{
    return true;
}

bool SMLayoutData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementLayout)
        return false;

    QXmlStreamAttributes layoutAttrs = xml.attributes();
    if (layoutAttrs.hasAttribute(XmlSM::xmlSMAttributeGridSize))
    {
        mGridSize = layoutAttrs.value(XmlSM::xmlSMAttributeGridSize).toInt();
    }
    if (layoutAttrs.hasAttribute(XmlSM::xmlSMAttributeGridVisible))
    {
        mGridVisible = (layoutAttrs.value(XmlSM::xmlSMAttributeGridVisible).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0);
    }

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementLayout))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            const QStringView listName = xml.name();
            if (listName == XmlSM::xmlSMElementViewList)
            {
                while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementViewList))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementView)
                    {
                        QXmlStreamAttributes a = xml.attributes();
                        SMLayoutView view;
                        view.owner = a.value(XmlSM::xmlSMAttributeOwner).toUInt();
                        view.zoom  = a.value(XmlSM::xmlSMAttributeZoom).toInt();
                        view.x     = a.value(XmlSM::xmlSMAttributeX).toDouble();
                        view.y     = a.value(XmlSM::xmlSMAttributeY).toDouble();
                        mViews.append(view);
                    }

                    xml.readNext();
                }
            }
            else if (listName == XmlSM::xmlSMElementNodeList)
            {
                while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementNodeList))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementNode)
                    {
                        QXmlStreamAttributes a = xml.attributes();
                        SMLayoutNode node;
                        node.owner  = a.value(XmlSM::xmlSMAttributeOwner).toUInt();
                        node.x      = a.value(XmlSM::xmlSMAttributeX).toDouble();
                        node.y      = a.value(XmlSM::xmlSMAttributeY).toDouble();
                        node.width  = a.value(XmlSM::xmlSMAttributeWidth).toDouble();
                        node.height = a.value(XmlSM::xmlSMAttributeHeight).toDouble();
                        node.color       = a.value(XmlSM::xmlSMAttributeColor).toString();
                        node.headerColor = a.value(XmlSM::xmlSMAttributeHeaderColor).toString();
                        if (a.hasAttribute(XmlSM::xmlSMAttributeExpanded))
                        {
                            node.hasExpanded = true;
                            node.expanded = (a.value(XmlSM::xmlSMAttributeExpanded).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0);
                        }
                        mNodes.append(node);
                    }

                    xml.readNext();
                }
            }
            else if (listName == XmlSM::xmlSMElementEdgeList)
            {
                while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementEdgeList))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementEdge)
                    {
                        QXmlStreamAttributes a = xml.attributes();
                        SMLayoutEdge edge;
                        edge.owner = a.value(XmlSM::xmlSMAttributeOwner).toUInt();
                        edge.shape = (a.value(XmlSM::xmlSMAttributeShape).toString().compare(XmlSM::xmlSMShapeArc, Qt::CaseInsensitive) == 0)
                                        ? SMLayoutEdge::eShape::Arc
                                        : SMLayoutEdge::eShape::Line;
                        edge.bulge = a.value(XmlSM::xmlSMAttributeBulge).toDouble();
                        edge.color = a.value(XmlSM::xmlSMAttributeColor).toString();

                        while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementEdge))
                        {
                            if (xml.tokenType() == QXmlStreamReader::StartElement)
                            {
                                if (xml.name() == XmlSM::xmlSMElementPoint)
                                {
                                    QXmlStreamAttributes pa = xml.attributes();
                                    edge.points.append(QPointF(pa.value(XmlSM::xmlSMAttributeX).toDouble(), pa.value(XmlSM::xmlSMAttributeY).toDouble()));
                                }
                                else if (xml.name() == XmlSM::xmlSMElementLabel)
                                {
                                    QXmlStreamAttributes pa = xml.attributes();
                                    edge.hasLabel = true;
                                    edge.label = QPointF(pa.value(XmlSM::xmlSMAttributeX).toDouble(), pa.value(XmlSM::xmlSMAttributeY).toDouble());
                                }
                            }

                            xml.readNext();
                        }

                        mEdges.append(edge);
                    }

                    xml.readNext();
                }
            }
            else if (listName == XmlSM::xmlSMElementNoteList)
            {
                while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementNoteList))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementNote)
                    {
                        QXmlStreamAttributes a = xml.attributes();
                        SMLayoutNote note;
                        note.id     = a.value(XmlSM::xmlSMAttributeID).toUInt();
                        note.level  = a.value(XmlSM::xmlSMAttributeLevel).toUInt();
                        note.x      = a.value(XmlSM::xmlSMAttributeX).toDouble();
                        note.y      = a.value(XmlSM::xmlSMAttributeY).toDouble();
                        note.width  = a.value(XmlSM::xmlSMAttributeWidth).toDouble();
                        note.height = a.value(XmlSM::xmlSMAttributeHeight).toDouble();
                        note.color  = a.value(XmlSM::xmlSMAttributeColor).toString();

                        while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementNote))
                        {
                            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementText)
                            {
                                note.text = xml.readElementText();
                            }

                            xml.readNext();
                        }

                        // The note carries its own document ID; keep the counter above it.
                        setMaxId(note.id);
                        mNotes.append(note);
                    }

                    xml.readNext();
                }
            }
        }

        xml.readNext();
    }

    return true;
}

void SMLayoutData::writeToXml(QXmlStreamWriter& xml) const
{
    // The Layout section is editor state only; omit it when there is nothing to persist.
    if (mViews.isEmpty() && mNodes.isEmpty() && mEdges.isEmpty() && mNotes.isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementLayout);
    xml.writeAttribute(XmlSM::xmlSMAttributeGridSize, QString::number(mGridSize));
    xml.writeAttribute(XmlSM::xmlSMAttributeGridVisible, mGridVisible ? XmlSM::xmlSMValueTrue : XmlSM::xmlSMValueFalse);

    if (mViews.isEmpty() == false)
    {
        xml.writeStartElement(XmlSM::xmlSMElementViewList);
        for (const SMLayoutView& view : mViews)
        {
            xml.writeStartElement(XmlSM::xmlSMElementView);
            xml.writeAttribute(XmlSM::xmlSMAttributeOwner, QString::number(view.owner));
            xml.writeAttribute(XmlSM::xmlSMAttributeZoom, QString::number(view.zoom));
            xml.writeAttribute(XmlSM::xmlSMAttributeX, QString::number(view.x));
            xml.writeAttribute(XmlSM::xmlSMAttributeY, QString::number(view.y));
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    if (mNodes.isEmpty() == false)
    {
        xml.writeStartElement(XmlSM::xmlSMElementNodeList);
        for (const SMLayoutNode& node : mNodes)
        {
            xml.writeStartElement(XmlSM::xmlSMElementNode);
            xml.writeAttribute(XmlSM::xmlSMAttributeOwner, QString::number(node.owner));
            xml.writeAttribute(XmlSM::xmlSMAttributeX, QString::number(node.x));
            xml.writeAttribute(XmlSM::xmlSMAttributeY, QString::number(node.y));
            xml.writeAttribute(XmlSM::xmlSMAttributeWidth, QString::number(node.width));
            xml.writeAttribute(XmlSM::xmlSMAttributeHeight, QString::number(node.height));
            if (node.color.isEmpty() == false)
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeColor, node.color);
            }
            if (node.headerColor.isEmpty() == false)
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeHeaderColor, node.headerColor);
            }
            if (node.hasExpanded)
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeExpanded, node.expanded ? XmlSM::xmlSMValueTrue : XmlSM::xmlSMValueFalse);
            }
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    if (mEdges.isEmpty() == false)
    {
        xml.writeStartElement(XmlSM::xmlSMElementEdgeList);
        for (const SMLayoutEdge& edge : mEdges)
        {
            xml.writeStartElement(XmlSM::xmlSMElementEdge);
            xml.writeAttribute(XmlSM::xmlSMAttributeOwner, QString::number(edge.owner));
            if (edge.shape == SMLayoutEdge::eShape::Arc)
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeShape, XmlSM::xmlSMShapeArc);
                xml.writeAttribute(XmlSM::xmlSMAttributeBulge, QString::number(edge.bulge));
            }
            if (edge.color.isEmpty() == false)
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeColor, edge.color);
            }
            for (const QPointF& point : edge.points)
            {
                xml.writeStartElement(XmlSM::xmlSMElementPoint);
                xml.writeAttribute(XmlSM::xmlSMAttributeX, QString::number(point.x()));
                xml.writeAttribute(XmlSM::xmlSMAttributeY, QString::number(point.y()));
                xml.writeEndElement();
            }
            if (edge.hasLabel)
            {
                xml.writeStartElement(XmlSM::xmlSMElementLabel);
                xml.writeAttribute(XmlSM::xmlSMAttributeX, QString::number(edge.label.x()));
                xml.writeAttribute(XmlSM::xmlSMAttributeY, QString::number(edge.label.y()));
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    if (mNotes.isEmpty() == false)
    {
        xml.writeStartElement(XmlSM::xmlSMElementNoteList);
        for (const SMLayoutNote& note : mNotes)
        {
            xml.writeStartElement(XmlSM::xmlSMElementNote);
            xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(note.id));
            xml.writeAttribute(XmlSM::xmlSMAttributeLevel, QString::number(note.level));
            xml.writeAttribute(XmlSM::xmlSMAttributeX, QString::number(note.x));
            xml.writeAttribute(XmlSM::xmlSMAttributeY, QString::number(note.y));
            xml.writeAttribute(XmlSM::xmlSMAttributeWidth, QString::number(note.width));
            xml.writeAttribute(XmlSM::xmlSMAttributeHeight, QString::number(note.height));
            if (note.color.isEmpty() == false)
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeColor, note.color);
            }
            xml.writeTextElement(XmlSM::xmlSMElementText, note.text);
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
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

bool SMLayoutData::removeView(uint32_t owner)
{
    for (int i = 0; i < mViews.size(); ++i)
    {
        if (mViews.at(i).owner == owner)
        {
            mViews.removeAt(i);
            return true;
        }
    }

    return false;
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
