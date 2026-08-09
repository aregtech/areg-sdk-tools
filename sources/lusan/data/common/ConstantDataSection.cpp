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
 *  \file        lusan/data/common/ConstantDataSection.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the constants section of a document.
 *
 ************************************************************************/

#include "lusan/data/common/ConstantDataSection.hpp"

#include "lusan/common/XmlSI.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"

ConstantDataSection::ConstantDataSection(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<ConstantEntry, DocumentElem>(parent)
{
}

ConstantDataSection::ConstantDataSection(const QList<ConstantEntry>& entries, ElementBase* parent /*= nullptr*/)
    : TEDataContainer<ConstantEntry, DocumentElem>(parent)
{
    setElements(entries);
}

bool ConstantDataSection::isValid() const
{
    return true;
}

bool ConstantDataSection::readFromXml(QXmlStreamReader& xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != XmlSI::xmlSIElementConstantList))
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementConstantList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementConstant)
        {
            ConstantEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addElement(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void ConstantDataSection::writeToXml(QXmlStreamWriter& xml) const
{
    const QList<ConstantEntry>& elements = getElements();
    if (elements.isEmpty())
        return;

    xml.writeStartElement(XmlSI::xmlSIElementConstantList);
    for (const ConstantEntry& entry : elements)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // ConstantList
}

void ConstantDataSection::validate(const QList<DataTypeCustom*>& customTypes)
{
    QList<ConstantEntry>& list = getElements();
    for (ConstantEntry& entry : list)
    {
        entry.validate(customTypes);
    }
}

ConstantEntry* ConstantDataSection::createConstant(const QString& name)
{
    if (findElement(name) != nullptr)
        return nullptr;

    ConstantEntry entry(getNextId(), name, this);
    return addElement(std::move(entry), true) ? findElement(name) : nullptr;
}

ConstantEntry* ConstantDataSection::insertConstant(int position, const QString& name)
{
    if (findElement(name) != nullptr)
        return nullptr;

    ConstantEntry entry(getNextId(), name, this);
    return insertElement(position, std::move(entry), true) ? findElement(name) : nullptr;
}

QList<uint32_t> ConstantDataSection::replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType)
{
    QList<uint32_t> result;
    QList<ConstantEntry>& list = getElements();
    for (ConstantEntry& entry : list)
    {
        if (entry.getParamType() == oldDataType)
        {
            entry.setParamType(newDataType);
            result.push_back(entry.getId());
        }
    }

    return result;
}
