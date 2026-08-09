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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/common/DocUnknownScan.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the elements of a document the format does not define.
 *
 ************************************************************************/

#include "lusan/model/common/DocUnknownScan.hpp"

#include "lusan/common/DocElementTable.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    //!< One open element of the walk: what it is called and, when it carries one, its ID.
    struct OpenElement
    {
        QString     name;
        uint32_t    id { 0u };
        bool        hasId { false };
    };

    uint32_t elementId(const QXmlStreamReader& xml, bool& present)
    {
        const QStringView value = xml.attributes().value(XmlSM::xmlSMAttributeID);
        present = (value.isEmpty() == false);
        return present ? value.toUInt() : 0u;
    }

    //!< Copies the element the reader stands on, and everything under it, as text.
    QString captureElement(QXmlStreamReader& xml)
    {
        QString out;
        QXmlStreamWriter writer(&out);
        writer.writeStartElement(xml.name().toString());
        writer.writeAttributes(xml.attributes());

        int depth = 1;
        while ((depth > 0) && (xml.atEnd() == false) && (xml.hasError() == false))
        {
            switch (xml.readNext())
            {
            case QXmlStreamReader::StartElement:
                writer.writeStartElement(xml.name().toString());
                writer.writeAttributes(xml.attributes());
                ++depth;
                break;

            case QXmlStreamReader::EndElement:
                writer.writeEndElement();
                --depth;
                break;

            case QXmlStreamReader::Characters:
                if (xml.isCDATA())
                {
                    writer.writeCDATA(xml.text().toString());
                }
                else
                {
                    writer.writeCharacters(xml.text().toString());
                }
                break;

            case QXmlStreamReader::Comment:
                writer.writeComment(xml.text().toString());
                break;

            default:
                break;
            }
        }

        return out;
    }

    //!< Writes a kept block through \p writer, so the result is one well-formed stream rather
    //!< than text spliced into the middle of another element.
    void writeKeptBlock(QXmlStreamWriter& writer, const QString& block)
    {
        QXmlStreamReader reader(block);
        while ((reader.atEnd() == false) && (reader.hasError() == false))
        {
            switch (reader.readNext())
            {
            case QXmlStreamReader::StartElement:
                writer.writeStartElement(reader.name().toString());
                writer.writeAttributes(reader.attributes());
                break;

            case QXmlStreamReader::EndElement:
                writer.writeEndElement();
                break;

            case QXmlStreamReader::Characters:
                if (reader.isCDATA())
                {
                    writer.writeCDATA(reader.text().toString());
                }
                else
                {
                    writer.writeCharacters(reader.text().toString());
                }
                break;

            case QXmlStreamReader::Comment:
                writer.writeComment(reader.text().toString());
                break;

            default:
                break;
            }
        }
    }

    //!< The owner ID and the element names between it and the top of \p stack.
    void anchorOf(const QList<OpenElement>& stack, uint32_t& ownerId, QStringList& wrappers)
    {
        ownerId = 0u;
        wrappers.clear();
        for (int i = stack.size() - 1; i >= 0; --i)
        {
            if (stack.at(i).hasId)
            {
                ownerId = stack.at(i).id;
                break;
            }

            wrappers.prepend(stack.at(i).name);
        }
    }
}

QList<DocUnknownElement> DocUnknownScan::scan(const QByteArray& xml)
{
    QList<DocUnknownElement> found;
    QXmlStreamReader reader(xml);
    QList<OpenElement> stack;

    while (reader.atEnd() == false)
    {
        const QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::EndElement)
        {
            if (stack.isEmpty() == false)
            {
                stack.removeLast();
            }
            continue;
        }

        if (token != QXmlStreamReader::StartElement)
        {
            continue;
        }

        const QString name   = reader.name().toString();
        const QString parent = stack.isEmpty() ? QString() : stack.constLast().name;
        if (DocElementTable::accepts(name, parent))
        {
            OpenElement open;
            open.name = name;
            open.id   = elementId(reader, open.hasId);
            stack.append(open);
            continue;
        }

        DocUnknownElement entry;
        entry.name    = name;
        entry.parent  = parent;
        entry.line    = static_cast<int>(reader.lineNumber());
        const DocElementTable::Row* row = DocElementTable::find(name);
        entry.removed = (row != nullptr) && (row->status == DocElementTable::eStatus::Removed);
        anchorOf(stack, entry.ownerId, entry.wrappers);
        entry.text = captureElement(reader);
        found.append(entry);
    }

    return found;
}

QByteArray DocUnknownScan::restore(const QByteArray& written, const QList<DocUnknownElement>& unknown)
{
    if (unknown.isEmpty())
    {
        return written;
    }

    QByteArray out;
    QXmlStreamReader reader(written);
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);

    QList<OpenElement> stack;
    QList<bool> placed;
    for (int i = 0; i < unknown.size(); ++i)
    {
        placed.append(false);
    }

    while (reader.atEnd() == false)
    {
        switch (reader.readNext())
        {
        case QXmlStreamReader::StartDocument:
            writer.writeStartDocument(QStringLiteral("1.0"), true);
            break;

        case QXmlStreamReader::StartElement:
        {
            OpenElement open;
            open.name = reader.name().toString();
            open.id   = elementId(reader, open.hasId);
            stack.append(open);
            writer.writeStartElement(open.name);
            writer.writeAttributes(reader.attributes());
            break;
        }

        case QXmlStreamReader::EndElement:
        {
            // The blocks kept from the read belong inside the element they were found in, so
            // they go back just before it closes.
            uint32_t ownerId = 0u;
            QStringList wrappers;
            anchorOf(stack, ownerId, wrappers);
            const QString here = stack.isEmpty() ? QString() : stack.constLast().name;
            for (int i = 0; i < unknown.size(); ++i)
            {
                const DocUnknownElement& entry = unknown.at(i);
                // The document element keeps its own blocks, in the order they were found
                // between the sections; taking them here would write them twice.
                if (placed.at(i) || (entry.parent == XmlSM::xmlSMElementStateMachine)
                    || (entry.parent != here) || (entry.ownerId != ownerId)
                    || (entry.wrappers != wrappers))
                {
                    continue;
                }

                writeKeptBlock(writer, entry.text);
                placed[i] = true;
            }

            if (stack.isEmpty() == false)
            {
                stack.removeLast();
            }

            writer.writeEndElement();
            break;
        }

        case QXmlStreamReader::Characters:
            if (reader.isCDATA())
            {
                writer.writeCDATA(reader.text().toString());
            }
            else if (reader.isWhitespace() == false)
            {
                writer.writeCharacters(reader.text().toString());
            }
            break;

        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
            break;

        default:
            break;
        }
    }

    return reader.hasError() ? written : out;
}
