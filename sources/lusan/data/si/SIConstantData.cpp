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
#include "lusan/data/si/SIDataTypeData.hpp"
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

bool SIConstantData::addConstant(const ConstantEntry& entry, bool unique /*= true*/)
{
    if (findConstant(entry.getName()) != -1)
        return false;

    mConstants.append(entry);
    return true;
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

void SIConstantData::sortConstants(bool ascending)
{
    std::sort(mConstants.begin(), mConstants.end(), [ascending](const ConstantEntry& left, const ConstantEntry& right) {
        return ascending ? left < right : left > right;
        });
}

void SIConstantData::removeAll(void)
{
    mConstants.clear();
}

bool SIConstantData::update(const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint)
{
    int index = findConstant(name);
    if (index != -1)
    {
        ConstantEntry newEntry(0, name, type, value, isDeprecated, description, deprecateHint);
        mConstants[index] = newEntry;
        return true;
    }

    return false;
}

bool SIConstantData::updateValue(const QString& name, const QString& value)
{
    int index = findConstant(name);
    if (index != -1)
    {
        mConstants[index].setValue(value);
        return true;
    }

    return false;
}

bool SIConstantData::updateName(const QString& oldName, const QString& newName, bool unique /*= true*/)
{
    bool result{false};
    int index = findConstant(oldName);
    if (index != -1)
    {
        if (unique)
        {
            if (findConstant(newName) == -1)
            {
                mConstants[index].setName(newName);
                result = true;
            }
        }
        else
        {
            mConstants[index].setName(newName);
            result = true;
        }
    }
    
    return result;
}

bool SIConstantData::updateType(const QString& name, const QString& type)
{
    int index = findConstant(name);
    if (index != -1)
    {
        mConstants[index].setType(type);
        return true;
    }

    return false;
}

bool SIConstantData::updateDeprecation(const QString& name, bool isDeprecated)
{
    int index = findConstant(name);
    if (index != -1)
    {
        mConstants[index].setDeprecated(isDeprecated);
        return true;
    }

    return false;
}

bool SIConstantData::updateDescription(const QString& name, const QString& description)
{
    int index = findConstant(name);
    if (index != -1)
    {
        mConstants[index].setDescription(description);
        return true;
    }

    return false;
}

bool SIConstantData::updateDeprecateHint(const QString& name, const QString& deprecateHint)
{
    int index = findConstant(name);
    if (index != -1)
    {
        mConstants[index].setDeprecateHint(deprecateHint);
        return true;
    }

    return false;
}

bool SIConstantData::validate(const SIDataTypeData& dataType) const
{
    for (const ConstantEntry& entry : mConstants)
    {
        if (dataType.findDataType(entry.getType()) == nullptr)
        {
            return false;
        }
    }

    return true;
}

ConstantEntry* SIConstantData::find(const QString& name) const
{
    int index = findConstant(name);
    return (index != -1) ? const_cast<ConstantEntry*>(&mConstants[index]) : nullptr;
}
