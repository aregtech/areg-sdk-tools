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
 *  \file        lusan/data/si/SIConstantData.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Data.
 *
 ************************************************************************/
#include "lusan/data/si/SIConstantData.hpp"
#include "lusan/common/XmlSI.hpp"

const ConstantEntry SIConstantData::InvalidConstant = ConstantEntry();

SIConstantData::SIConstantData(void)
{
}

SIConstantData::SIConstantData(const QList<ConstantEntry>& entries)
    : mConstants(entries)
{
}

const QList<ConstantEntry>& SIConstantData::getConstants(void) const
{
    return mConstants;
}

void SIConstantData::setConstants(const QList<ConstantEntry>& entries)
{
    mConstants = entries;
}

int SIConstantData::findConstant(const ConstantEntry& entry) const
{
    return mConstants.indexOf(entry);
}

void SIConstantData::addConstant(const ConstantEntry& entry)
{
    mConstants.append(entry);
}

bool SIConstantData::removeConstant(const ConstantEntry& entry)
{
    return mConstants.removeOne(entry);
}

bool SIConstantData::replaceConstant(const ConstantEntry& oldEntry, const ConstantEntry& newEntry)
{
    int index = findConstant(oldEntry);
    if (index != -1)
    {
        mConstants[index] = newEntry;
        return true;
    }

    return false;
}

bool SIConstantData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != XmlSI::xmlSIElementConstantList)
        return false;

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSI::xmlSIElementConstantList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSI::xmlSIElementConstant)
        {
            ConstantEntry entry;
            if (entry.readFromXml(xml))
            {
                addConstant(entry);
            }
        }

        xml.readNext();
    }

    return true;
}

void SIConstantData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementConstantList);
    for (const ConstantEntry& entry : mConstants)
    {
        entry.writeToXml(xml);
    }

    xml.writeEndElement(); // ConstantList
}

int SIConstantData::findConstant(const QString& name) const
{
    for (int i = 0; i < mConstants.size(); ++i)
    {
        if (mConstants[i].getName() == name)
        {
            return i;
        }
    }
    return -1;
}

bool SIConstantData::exists(const QString& name) const
{
    return findConstant(name) != -1;
}

const ConstantEntry& SIConstantData::getConstant(const QString& name) const
{
    int index = findConstant(name);
    return (index != -1) ? mConstants[index] : InvalidConstant;
}

bool SIConstantData::removeConstant(const QString& name)
{
    int index = findConstant(name);
    if (index != -1)
    {
        mConstants.removeAt(index);
        return true;
    }

    return false;
}

void SIConstantData::addConstant(const ConstantEntry& entry, bool ascending)
{
    if (mConstants.contains(entry) == false)
    {
        mConstants.append(entry);
        sortConstants(ascending);
    }
}

void SIConstantData::sortConstants(bool ascending)
{
    std::sort(mConstants.begin(), mConstants.end(), [ascending](const ConstantEntry& left, const ConstantEntry& right) {
        return ascending ? left < right : left > right;
        });
}
