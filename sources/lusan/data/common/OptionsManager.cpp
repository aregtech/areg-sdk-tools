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
 *  \file        lusan/data/common/OptionsManager.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/data/common/OptionsManager.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

OptionsManager::OptionsManager(void)
    : mActiveKey    ( 0 )
    , mWorkspaces   ( )
    , mCurId        ( 0 )
{
}

WorkspaceEntry OptionsManager::addWorkspace(const QString& root, const QString& description)
{
    if (existsWorkspace(root))
    {
        return activateWorkspace(root, description);
    }
    else
    {
        WorkspaceEntry result(root, description, ++mCurId);
        mWorkspaces.push_back(result);
        mActiveKey = result.getKey();
        _sort();

        return result;
    }
}

void OptionsManager::addWorkspace(const WorkspaceEntry & workspace)
{
    bool found{false};
    for (auto& entry : mWorkspaces)
    {
        if (entry.getWorkspaceRoot() == workspace.getWorkspaceRoot())
        {
            uint32_t id = entry.getId();
            entry = workspace;
            entry.setId(id);
            found = true;
            break;
        }
    }
    
    if (found == false)
    {
        mWorkspaces.push_back(workspace);
        mWorkspaces[mWorkspaces.size() - 1].setId(++mCurId);
    }

    emit signalWorkspaceDirectoriesChanged(workspace);
}

WorkspaceEntry OptionsManager::removeWorkspace(uint64_t key)
{
    WorkspaceEntry result;
    if (key == mActiveKey)
        return result;

    for (auto it = mWorkspaces.begin(); it != mWorkspaces.end(); ++it)
    {
        if (it->getKey() == key)
        {
            result = *it;
            mWorkspaces.erase(it);
            _sort();
            break;
        }
    }

    return result;
}

WorkspaceEntry OptionsManager::activateWorkspace(uint64_t key, const QString& description)
{
    WorkspaceEntry result;
    for (auto it = mWorkspaces.begin(); it != mWorkspaces.end(); ++it)
    {
        if (it->getKey() == key)
        {
            result = *it;
            mWorkspaces.erase(it);
            mActiveKey = result.activate();
            result.setWorkspaceDescription(description);
            mWorkspaces.push_back(result);
            _sort();
            break;
        }
    }

    return result;
}

WorkspaceEntry OptionsManager::activateWorkspace(const QString& root, const QString& description)
{
    WorkspaceEntry result;
    for (auto it = mWorkspaces.begin(); it != mWorkspaces.end(); ++it)
    {
        if (it->getWorkspaceRoot() == root)
        {
            result = *it;
            mWorkspaces.erase(it);
            mActiveKey = result.activate();
            result.setWorkspaceDescription(description);
            mWorkspaces.push_back(result);
            _sort();
            break;
        }
    }

    return result;
}

WorkspaceEntry OptionsManager::getActiveWorkspace(void) const
{
    WorkspaceEntry result;
    if (mActiveKey == 0)
        return result;

    for (const auto & entry : mWorkspaces)
    {
        if (entry.getKey() == mActiveKey)
        {
            result = entry;
            break;
        }
    }

    return result;
}

bool OptionsManager::existsWorkspace(const QString& root) const
{
    bool result{ false };
    for (const auto& entry : mWorkspaces)
    {
        if (entry.getWorkspaceRoot() == root)
        {
            result = true;
            break;
        }
    }

    return result;
}

bool OptionsManager::readOptions(void)
{
    QString fileOptions{ NELusanCommon::getOptionsFile() };
    QFile file(fileOptions);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
        return false;

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == NELusanCommon::xmlElementOptionList)
            {
                _readOptionList(xml);
            }
        }
    }

    bool result {!xml.hasError()};
    file.close();
    
    return result;
}

void OptionsManager::writeOptions(void)
{
    QString fileOptions{ NELusanCommon::getOptionsFile() };
    QFile file(fileOptions);
    QDir dir = QFileInfo(file).absoluteDir();

    if (dir.exists() == false)
    {
        dir.mkpath(".");
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true); // Enable auto formatting for beautification
    xml.setAutoFormattingIndent(3); // Set indentation level
    xml.writeStartDocument();
        xml.writeStartElement(NELusanCommon::xmlElementOptionList);
        xml.writeAttribute(NELusanCommon::xmlAttributeVersion, NELusanCommon::xmlWorkspaceVersion);
            xml.writeStartElement(NELusanCommon::xmlElementOption);
                xml.writeStartElement(NELusanCommon::xmlElementWorkspaceList);
                for (const auto& entry : mWorkspaces)
                {
                    entry.writeToXml(xml);
                }
                xml.writeEndElement();
            xml.writeEndElement();
        xml.writeEndElement();
    xml.writeEndDocument();

    file.close();
}

void OptionsManager::_readOptionList(QXmlStreamReader& xml)
{
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementOptionList)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
    
    while (xmlName != NELusanCommon::xmlElementOptionList)
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementOption)
            {
                _readOption(xml);
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
}

void OptionsManager::_readOption(QXmlStreamReader& xml)
{
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementOption)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }

    while (xmlName != NELusanCommon::xmlElementOption)
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementWorkspaceList)
            {
                _readWorkspaceList(xml);
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
}

void OptionsManager::_readWorkspaceList(QXmlStreamReader& xml)
{
    mWorkspaces.clear();
    
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementWorkspaceList)
    {
        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
    
    while (xmlName != NELusanCommon::xmlElementWorkspaceList)
    {
        if (tokenType == QXmlStreamReader::StartElement)
        {
            if (xmlName == NELusanCommon::xmlElementWorkspace)
            {
                WorkspaceEntry workspace;
                workspace.readFromXml(xml);
                if (workspace.isValid())
                {
                    mWorkspaces.push_back(workspace);
                    mCurId = std::max(mCurId, workspace.getId());
                }
            }
        }

        xml.readNext();
        tokenType = xml.tokenType();
        xmlName = xml.name();
    }
    
    _sort();
}

WorkspaceEntry OptionsManager::_findWorkspace(const QString& root) const
{
    WorkspaceEntry result{};
    for (const auto& entry : mWorkspaces)
    {
        if (entry.getWorkspaceRoot() == root)
        {
            result = entry;
            break;
        }
    }

    return result;
}
