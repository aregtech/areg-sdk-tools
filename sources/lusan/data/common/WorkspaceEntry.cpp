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
 *  \file        lusan/data/common/WorkspaceEntry.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/data/common/WorkspaceEntry.hpp"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

WorkspaceEntry::WorkspaceEntry(void)
    : mId(0)
    , mLastAccessed(0)
    , mWorkspaceRoot()
    , mDescription()
    , mSources()
    , mIncludes()
    , mDelivery()
    , mLogFiles()
{
}

WorkspaceEntry::WorkspaceEntry(const QString& root, const QString& description, uint32_t id /*= 0*/)
    : mId(id == 0 ? NELusanCommon::getId() : id)
    , mLastAccessed(NELusanCommon::getTimestamp())
    , mWorkspaceRoot(root)
    , mDescription(description)
    , mSources()
    , mIncludes()
    , mDelivery()
    , mLogFiles()
{
}

WorkspaceEntry::WorkspaceEntry(QXmlStreamReader& xml)
    : mId(0)
    , mLastAccessed(0)
    , mWorkspaceRoot()
    , mDescription()
    , mSources()
    , mIncludes()
    , mDelivery()
    , mLogFiles()
{
    readFromXml(xml);
}

WorkspaceEntry::WorkspaceEntry(const WorkspaceEntry& src)
    : mId(src.mId)
    , mLastAccessed(src.mLastAccessed)
    , mWorkspaceRoot(src.mWorkspaceRoot)
    , mDescription(src.mDescription)
    , mSources(src.mSources)
    , mIncludes(src.mIncludes)
    , mDelivery(src.mDelivery)
    , mLogFiles(src.mLogFiles)
{
}

WorkspaceEntry::WorkspaceEntry(WorkspaceEntry&& src) noexcept
    : mId(src.mId)
    , mLastAccessed(src.mLastAccessed)
    , mWorkspaceRoot(std::move(src.mWorkspaceRoot))
    , mDescription(std::move(src.mDescription))
    , mSources(std::move(src.mSources))
    , mIncludes(std::move(src.mIncludes))
    , mDelivery(std::move(src.mDelivery))
    , mLogFiles(std::move(src.mLogFiles))
{
}

WorkspaceEntry& WorkspaceEntry::operator = (const WorkspaceEntry& src)
{
    if (this != &src)
    {
        mId = src.mId;
        mLastAccessed = src.mLastAccessed;
        mWorkspaceRoot = src.mWorkspaceRoot;
        mDescription = src.mDescription;
        mSources = src.mSources;
        mIncludes = src.mIncludes;
        mDelivery = src.mDelivery;
        mLogFiles = src.mLogFiles;
    }
    return *this;
}

WorkspaceEntry& WorkspaceEntry::operator = (WorkspaceEntry&& src) noexcept
{
    if (this != &src)
    {
        mId = src.mId;
        mLastAccessed = src.mLastAccessed;
        mWorkspaceRoot = std::move(src.mWorkspaceRoot);
        mDescription = std::move(src.mDescription);
        mSources = std::move(src.mSources);
        mIncludes = std::move(src.mIncludes);
        mDelivery = std::move(src.mDelivery);
        mLogFiles = std::move(src.mLogFiles);
    }
    return *this;
}

bool WorkspaceEntry::operator==(const WorkspaceEntry& other) const
{
    return (mId == other.mId);
}

bool WorkspaceEntry::operator > (const WorkspaceEntry& other) const
{
    return (mLastAccessed > other.mLastAccessed);
}

bool WorkspaceEntry::operator < (const WorkspaceEntry& other) const
{
    return (mLastAccessed < other.mLastAccessed);
}

bool WorkspaceEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != NELusanCommon::xmlElementWorkspace)
    {
        return false;
    }

    mId = xml.attributes().value(NELusanCommon::xmlAttributeId).toUInt();
    mLastAccessed = xml.attributes().value(NELusanCommon::xmlAttributeLastAccessed).toULongLong();
    
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementWorkspace)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
    
    while (xmlName != NELusanCommon::xmlElementWorkspace)
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementWorspaceRoot)
            {
                mWorkspaceRoot = xml.readElementText();
            }
            else if (xmlName == NELusanCommon::xmlElementDescription)
            {
                mDescription = xml.readElementText();
            }
            else if (xmlName == NELusanCommon::xmlElementSettings)
            {
                _readSettings(xml);
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }

    return (mId != 0);
}

inline void WorkspaceEntry::_readSettings(QXmlStreamReader& xml)
{
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementSettings)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
    
    while (xmlName != NELusanCommon::xmlElementSettings)
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementDirectories)
            {
                _readDirectories(xml);
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
}

void WorkspaceEntry::_readDirectories(QXmlStreamReader& xml)
{
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementDirectories)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
    
    while (xmlName != NELusanCommon::xmlElementDirectories)
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementSources)
            {
                mSources = xml.readElementText();
            }
            else if (xmlName == NELusanCommon::xmlElementIncludes)
            {
                mIncludes = xml.readElementText();
            }
            else if (xmlName == NELusanCommon::xmlElementDelivery)
            {
                mDelivery = xml.readElementText();
            }
            else if (xmlName == NELusanCommon::xmlElementLogs)
            {
                mLogFiles = xml.readElementText();
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
}

bool WorkspaceEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(NELusanCommon::xmlElementWorkspace);

        xml.writeAttribute(NELusanCommon::xmlAttributeId, QString::number(mId));
        xml.writeAttribute(NELusanCommon::xmlAttributeLastAccessed, QString::number(mLastAccessed));

        xml.writeTextElement(NELusanCommon::xmlElementWorspaceRoot, mWorkspaceRoot);
        xml.writeTextElement(NELusanCommon::xmlElementDescription, mDescription);
        xml.writeStartElement(NELusanCommon::xmlElementSettings);
            xml.writeStartElement(NELusanCommon::xmlElementDirectories);
                xml.writeTextElement(NELusanCommon::xmlElementSources, mSources);
                xml.writeTextElement(NELusanCommon::xmlElementIncludes, mIncludes);
                xml.writeTextElement(NELusanCommon::xmlElementDelivery, mDelivery);
                xml.writeTextElement(NELusanCommon::xmlElementLogs, mLogFiles);
            xml.writeEndElement();
        xml.writeEndElement();
    xml.writeEndElement();

    return !xml.hasError();
}
