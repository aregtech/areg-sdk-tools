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

QList<DocUnknownElement> DocUnknownScan::scan(DocElementTable::eDocument doc, const QByteArray& xml)
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
        if (DocElementTable::accepts(doc, name, parent))
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
        anchorOf(stack, entry.ownerId, entry.wrappers);
        entry.text = captureElement(reader);
        found.append(entry);
    }

    return found;
}
