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
 *  \file        lusan/data/common/WorkspaceEntry.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/data/common/WorkspaceEntry.hpp"
#include <QDir>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    //! The log window settings inside the workspace entry. They are named here rather than in
    //! the shared name list because nothing outside this file reads or writes them.
    const QString   _xmlElementLogView  { "LogView"  };
    const QString   _xmlElementColumn   { "Column"   };
    const QString   _xmlAttributeName   { "name"     };
    const QString   _xmlAttributeWidth  { "width"    };
    const QString   _xmlAttributeDb     { "database" };
    const QString   _xmlAttributeMode   { "mode"     };
    const QString   _xmlModeOffline     { "offline"  };
}

QString WorkspaceEntry::nameFromRoot(const QString& root)
{
    const QString name{ QDir(root).dirName() };
    return (name.isEmpty() ? root : name);
}

const WorkspaceEntry WorkspaceEntry::InvalidWorkspace{};

WorkspaceEntry::WorkspaceEntry()
    : mId(0)
    , mLastAccessed(0)
    , mWorkspaceRoot("")
    , mName         ("")
    , mDescription  ("")
    , mSources      ("")
    , mIncludes     ("")
    , mDelivery     ("")
    , mLogFiles     ("")
    , mLogDatabase  ("")
    , mLogColumns   ( )
{
}

WorkspaceEntry::WorkspaceEntry(const QString& root, const QString& name, const QString& description, uint32_t id /*= 0*/)
    : mId(id == 0 ? NELusanCommon::getId() : id)
    , mLastAccessed (NELusanCommon::getTimestamp())
    , mWorkspaceRoot(NELusanCommon::fixPath(root))
    , mName         (name.isEmpty() ? WorkspaceEntry::nameFromRoot(root) : name)
    , mDescription  (description)
    , mSources      ("")
    , mIncludes     ("")
    , mDelivery     ("")
    , mLogFiles     ("")
    , mLogDatabase  ("")
    , mLogColumns   ( )
{
}

WorkspaceEntry::WorkspaceEntry(QXmlStreamReader& xml)
    : mId(0)
    , mLastAccessed (0)
    , mWorkspaceRoot()
    , mName         ()
    , mDescription  ()
    , mSources      ("")
    , mIncludes     ("")
    , mDelivery     ("")
    , mLogFiles     ("")
    , mLogDatabase  ("")
    , mLogColumns   ( )
{
    readFromXml(xml);
}

WorkspaceEntry::WorkspaceEntry(const WorkspaceEntry& src)
    : mId(src.mId)
    , mLastAccessed (src.mLastAccessed)
    , mWorkspaceRoot(src.mWorkspaceRoot)
    , mName         (src.mName)
    , mDescription  (src.mDescription)
    , mSources      (src.mSources)
    , mIncludes     (src.mIncludes)
    , mDelivery     (src.mDelivery)
    , mLogFiles     (src.mLogFiles)
    , mLogDatabase  (src.mLogDatabase)
    , mLogColumns   (src.mLogColumns)
{
}

WorkspaceEntry::WorkspaceEntry(WorkspaceEntry&& src) noexcept
    : mId(src.mId)
    , mLastAccessed (src.mLastAccessed)
    , mWorkspaceRoot(std::move(src.mWorkspaceRoot))
    , mName         (std::move(src.mName))
    , mDescription  (std::move(src.mDescription))
    , mSources      (std::move(src.mSources))
    , mIncludes     (std::move(src.mIncludes))
    , mDelivery     (std::move(src.mDelivery))
    , mLogFiles     (std::move(src.mLogFiles))
    , mLogDatabase  (std::move(src.mLogDatabase))
    , mLogColumns   (std::move(src.mLogColumns))
{
}

WorkspaceEntry& WorkspaceEntry::operator = (const WorkspaceEntry& src)
{
    if (this != &src)
    {
        mId = src.mId;
        mLastAccessed   = src.mLastAccessed;
        mWorkspaceRoot  = src.mWorkspaceRoot;
        mName           = src.mName;
        mDescription    = src.mDescription;
        mSources        = src.mSources;
        mIncludes       = src.mIncludes;
        mDelivery       = src.mDelivery;
        mLogFiles       = src.mLogFiles;
        mLogDatabase    = src.mLogDatabase;
        mLogColumns     = src.mLogColumns;
    }

    return *this;
}

WorkspaceEntry& WorkspaceEntry::operator = (WorkspaceEntry&& src) noexcept
{
    if (this != &src)
    {
        mId = src.mId;
        mLastAccessed   = src.mLastAccessed;
        mWorkspaceRoot  = std::move(src.mWorkspaceRoot);
        mName           = std::move(src.mName);
        mDescription    = std::move(src.mDescription);
        mSources        = std::move(src.mSources);
        mIncludes       = std::move(src.mIncludes);
        mDelivery       = std::move(src.mDelivery);
        mLogFiles       = std::move(src.mLogFiles);
        mLogDatabase    = std::move(src.mLogDatabase);
        mLogColumns     = std::move(src.mLogColumns);
    }

    return *this;
}

void WorkspaceEntry::_writeLogColumns(QXmlStreamWriter& xml, const WorkspaceEntry::ListLogColumns& columns, const QString& mode) const
{
    for (const WorkspaceEntry::sLogColumn& column : columns)
    {
        xml.writeStartElement(_xmlElementColumn);
            xml.writeAttribute(_xmlAttributeName, column.key);
            xml.writeAttribute(_xmlAttributeWidth, QString::number(column.width));
            if (mode.isEmpty() == false)
            {
                xml.writeAttribute(_xmlAttributeMode, mode);
            }
        xml.writeEndElement();
    }
}

void WorkspaceEntry::_readLogView(QXmlStreamReader& xml)
{
    mLogDatabase = NELusanCommon::fixPath(xml.attributes().value(_xmlAttributeDb).toString());
    mLogColumns.clear();
    mLogColumnsFile.clear();

    QXmlStreamReader::TokenType tokenType{ xml.tokenType() };
    QStringView xmlName{ xml.name() };
    if (xmlName == _xmlElementLogView)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }

    while (!xml.atEnd() && (xmlName != _xmlElementLogView))
    {
        if ((tokenType == QXmlStreamReader::StartElement) && (xmlName == _xmlElementColumn))
        {
            WorkspaceEntry::sLogColumn column;
            column.key   = xml.attributes().value(_xmlAttributeName).toString();
            column.width = xml.attributes().value(_xmlAttributeWidth).toInt();
            if (column.key.isEmpty() == false)
            {
                // A file written before the two records existed carries no mode. Its one record
                // then stands for both, so neither window loses what it had.
                const QStringView mode{ xml.attributes().value(_xmlAttributeMode) };
                if (mode.isEmpty())
                {
                    mLogColumns.append(column);
                    mLogColumnsFile.append(column);
                }
                else if (mode == _xmlModeOffline)
                {
                    mLogColumnsFile.append(column);
                }
                else
                {
                    mLogColumns.append(column);
                }
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
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
    
    while (!xml.atEnd() && (xmlName != NELusanCommon::xmlElementWorkspace))
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementWorspaceRoot)
            {
                mWorkspaceRoot = NELusanCommon::fixPath(xml.readElementText());
            }
            else if (xmlName == NELusanCommon::xmlElementName)
            {
                mName = xml.readElementText();
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

    // Options files written before workspaces had a name carry none.
    if (mName.isEmpty())
    {
        mName = WorkspaceEntry::nameFromRoot(mWorkspaceRoot);
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
    
    while (!xml.atEnd() && (xmlName != NELusanCommon::xmlElementSettings))
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementDirectories)
            {
                _readDirectories(xml);
            }
            else if (xmlName == _xmlElementLogView)
            {
                _readLogView(xml);
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
    
    while (!xml.atEnd() && (xmlName != NELusanCommon::xmlElementDirectories))
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementSources)
            {
                mSources = NELusanCommon::fixPath(xml.readElementText());
            }
            else if (xmlName == NELusanCommon::xmlElementIncludes)
            {
                mIncludes = NELusanCommon::fixPath(xml.readElementText());
            }
            else if (xmlName == NELusanCommon::xmlElementDelivery)
            {
                mDelivery = NELusanCommon::fixPath(xml.readElementText());
            }
            else if (xmlName == NELusanCommon::xmlElementLogs)
            {
                mLogFiles = NELusanCommon::fixPath(xml.readElementText());
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
        xml.writeTextElement(NELusanCommon::xmlElementName, mName);
        xml.writeTextElement(NELusanCommon::xmlElementDescription, mDescription);
        xml.writeStartElement(NELusanCommon::xmlElementSettings);
            xml.writeStartElement(NELusanCommon::xmlElementDirectories);
                xml.writeTextElement(NELusanCommon::xmlElementSources, mSources);
                xml.writeTextElement(NELusanCommon::xmlElementIncludes, mIncludes);
                xml.writeTextElement(NELusanCommon::xmlElementDelivery, mDelivery);
                xml.writeTextElement(NELusanCommon::xmlElementLogs, mLogFiles);
            xml.writeEndElement();

            if ((mLogColumns.isEmpty() == false) || (mLogColumnsFile.isEmpty() == false) || (mLogDatabase.isEmpty() == false))
            {
                xml.writeStartElement(_xmlElementLogView);
                    xml.writeAttribute(_xmlAttributeDb, mLogDatabase);
                    _writeLogColumns(xml, mLogColumns, QString());
                    _writeLogColumns(xml, mLogColumnsFile, _xmlModeOffline);
                xml.writeEndElement();
            }
        xml.writeEndElement();
    xml.writeEndElement();

    return !xml.hasError();
}
