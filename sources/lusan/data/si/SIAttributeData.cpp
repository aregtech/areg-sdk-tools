/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIAttributeData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Attribute Data.
 *
 ************************************************************************/
#include "lusan/data/si/SIAttributeData.hpp"
#include "lusan/common/XmlSI.hpp"

const AttributeEntry SIAttributeData::InvalidAttribute = AttributeEntry();

SIAttributeData::SIAttributeData(ElementBase* parent /*= nullptr*/)
    : ElementBase(parent)
{
}

SIAttributeData::SIAttributeData(const QList<AttributeEntry>& entries, ElementBase* parent /*= nullptr*/)
    : mAttributes(entries)
{
    for (AttributeEntry& entry : mAttributes)
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            entry.setId(getNextId());
        }
    }
}

const QList<AttributeEntry>& SIAttributeData::getAttributes(void) const
{
    return mAttributes;
}

void SIAttributeData::setAttributes(const QList<AttributeEntry>& entries)
{
    mAttributes = entries;
    for (AttributeEntry& entry : mAttributes)
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            entry.setId(getNextId());
        }
    }
}

int SIAttributeData::findAttribute(const AttributeEntry& entry) const
{
    return mAttributes.indexOf(entry);
}

bool SIAttributeData::removeAttribute(const AttributeEntry& entry)
{
    return mAttributes.removeOne(entry);
}

bool SIAttributeData::replaceAttribute(const AttributeEntry& oldEntry, AttributeEntry&& newEntry)
{
    int index = findAttribute(oldEntry);
    if (index != -1)
    {
        if (newEntry.getParent() != this)
        {
            newEntry.setParent(this);
            newEntry.setId(getNextId());
        }

        mAttributes[index] = std::move(newEntry);
        return true;
    }

    return false;
}

bool SIAttributeData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementAttributeList)
        return false;

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementAttributeList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementAttribute)
        {
            AttributeEntry entry(this);
            if (entry.readFromXml(xml))
            {
                addAttribute(std::move(entry), true);
            }
        }

        xml.readNext();
    }

    return true;
}

void SIAttributeData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementAttributeList);
    for (const AttributeEntry& entry : mAttributes)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // AttributeList
}

int SIAttributeData::findAttribute(const QString& name) const
{
    for (int i = 0; i < mAttributes.size(); ++i)
    {
        if (mAttributes[i].getName() == name)
        {
            return i;
        }
    }

    return -1;
}

bool SIAttributeData::exists(const QString& name) const
{
    return findAttribute(name) != -1;
}

const AttributeEntry& SIAttributeData::getAttribute(const QString& name) const
{
    int index = findAttribute(name);
    return (index != -1) ? mAttributes[index] : InvalidAttribute;
}

bool SIAttributeData::removeAttribute(const QString& name)
{
    int index = findAttribute(name);
    if (index != -1)
    {
        mAttributes.removeAt(index);
        return true;
    }

    return false;
}

bool SIAttributeData::addAttribute(AttributeEntry&& entry, bool unique)
{
    if ((unique == false) || (mAttributes.contains(entry) == false))
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            entry.setId(getNextId());
        }

        mAttributes.append(std::move(entry));
        return true;
    }

    return false;
}

void SIAttributeData::sortAttributes(bool ascending)
{
    std::sort(mAttributes.begin(), mAttributes.end(), [ascending](const AttributeEntry& left, const AttributeEntry& right) {
        return ascending ? left < right : left > right;
    });
}

void SIAttributeData::removeAll(void)
{
    mAttributes.clear();
}
