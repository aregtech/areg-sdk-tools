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

const ConstantEntry SIAttributeData::InvalidAttribute = ConstantEntry();

SIAttributeData::SIAttributeData(void)
{
}

SIAttributeData::SIAttributeData(const QList<ConstantEntry>& entries)
    : mAttributes(entries)
{
}

const QList<ConstantEntry>& SIAttributeData::getAttributes(void) const
{
    return mAttributes;
}

void SIAttributeData::setAttributes(const QList<ConstantEntry>& entries)
{
    mAttributes = entries;
}

int SIAttributeData::findAttribute(const ConstantEntry& entry) const
{
    return mAttributes.indexOf(entry);
}

void SIAttributeData::addAttribute(const ConstantEntry& entry)
{
    mAttributes.append(entry);
}

bool SIAttributeData::removeAttribute(const ConstantEntry& entry)
{
    return mAttributes.removeOne(entry);
}

bool SIAttributeData::replaceAttribute(const ConstantEntry& oldEntry, const ConstantEntry& newEntry)
{
    int index = findAttribute(oldEntry);
    if (index != -1)
    {
        mAttributes[index] = newEntry;
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
            ConstantEntry entry;
            if (entry.readFromXml(xml))
            {
                addAttribute(entry);
            }
        }

        xml.readNext();
    }

    return true;
}

void SIAttributeData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementAttributeList);
    for (const ConstantEntry& entry : mAttributes)
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

const ConstantEntry& SIAttributeData::getAttribute(const QString& name) const
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

void SIAttributeData::addAttribute(const ConstantEntry& entry, bool ascending)
{
    if (mAttributes.contains(entry) == false)
    {
        mAttributes.append(entry);
        sortAttributes(ascending);
    }
}

void SIAttributeData::sortAttributes(bool ascending)
{
    std::sort(mAttributes.begin(), mAttributes.end(), [ascending](const ConstantEntry& left, const ConstantEntry& right) {
        return ascending ? left < right : left > right;
    });
}
