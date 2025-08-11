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

#include "areg/base/NECommon.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <algorithm>

OptionsManager::OptionsManager(void)
    : mActiveKey    ( 0 )
    , mDefWorkspace ( 0 )
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
    auto workspaceIt{
                     std::find_if(std::begin(mWorkspaces), std::end(mWorkspaces),
                                  [workspace](WorkspaceEntry const& w) { return w.getId() == workspace.getId(); }) };
    if (std::end(mWorkspaces) == workspaceIt)
    {
        mWorkspaces.push_back(workspace);
        mWorkspaces[mWorkspaces.size() - 1].setId(++mCurId);
        workspaceIt = mWorkspaces.begin() + (mWorkspaces.size() - 1);
    }
    else
    {
        *workspaceIt = workspace;
    }
    
    emit signalWorkspaceDirectoriesChanged(*workspaceIt, workspaceIt->getKey() == mActiveKey);
}

bool OptionsManager::updateWorkspace(const WorkspaceEntry & workspace)
{
    uint32_t const id = workspace.getId();

    auto workspaceIt{
        std::find_if(std::begin(mWorkspaces), std::end(mWorkspaces),
            [id](WorkspaceEntry const& w) { return w.getId() == id; }) };

    if (std::end(mWorkspaces) == workspaceIt)
        return false;

    *workspaceIt = workspace;
    emit signalWorkspaceDirectoriesChanged(*workspaceIt, workspaceIt->getKey() == mActiveKey);

    return true;
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
                if (hasDefaultWorkspace())
                {
                    xml.writeAttribute(NELusanCommon::xmlAttributeDefault, QString::number(mDefWorkspace));
                }
                
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

bool OptionsManager::hasDefaultWorkspace(void) const
{
    return ((mDefWorkspace != 0) && _existWorkspaceId(mDefWorkspace));
}

bool OptionsManager::isDefaultWorkspace(uint64_t workspaceId) const
{
    return ((mDefWorkspace == workspaceId) && hasDefaultWorkspace());
}

bool OptionsManager::isDefaultWorkspace(const QString& workspaceRoot) const
{
    return (workspaceRoot.isEmpty() == false) && (workspaceRoot == getDefaultWorkspaceRoot());
}

uint64_t OptionsManager::getDefaultWorkspaceId(void) const
{
    return getDefaultWorkspace().getId();
}

const QString& OptionsManager::getDefaultWorkspaceRoot(void) const
{
    return getDefaultWorkspace().getWorkspaceRoot();
}

const WorkspaceEntry& OptionsManager::getDefaultWorkspace(void) const
{
    return (mDefWorkspace != 0 ? _findWorkspace(mDefWorkspace) : WorkspaceEntry::InvalidWorkspace);
}

uint64_t OptionsManager::activateDefaultWorkspace(void)
{
    uint32_t id     = mDefWorkspace;
    mActiveKey      = 0u;
    mDefWorkspace   = 0u;
    for (auto& entry : mWorkspaces)
    {
        if (entry.getId() == id)
        {
            mDefWorkspace   = id;
            mActiveKey      = entry.activate();
            break;
        }
    }
    
    return mActiveKey;
}

bool OptionsManager::setDefaultWorkspace(uint64_t defWorkspaceId)
{
    mDefWorkspace = 0;
    if ((defWorkspaceId != 0) && _existWorkspaceId(defWorkspaceId))
        mDefWorkspace = defWorkspaceId;
    
    return (mDefWorkspace != 0u);
}

bool OptionsManager::setDefaultWorkspace(const QString defWorkspaceRoot)
{
    mDefWorkspace = 0;
    if (defWorkspaceRoot.isEmpty())
        return false;

    const WorkspaceEntry& workspace = _findWorkspace(defWorkspaceRoot);
    if (workspace.isValid())
        mDefWorkspace = workspace.getId();
    
    return (mDefWorkspace != 0u);
}

bool OptionsManager::isActiveWorkspace(uint32_t id) const
{
    bool result{ false };
    if (mActiveKey == 0)
        return result;
    
    for (const auto & entry : mWorkspaces)
    {
        if (entry.getKey() == mActiveKey)
        {
            result = (entry.getId() == id);
            break;
        }
    }
    
    return result;
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
    mDefWorkspace = 0u;
    mActiveKey = 0u;
    mCurId = 0u;
    
    QXmlStreamReader::TokenType tokenType{xml.tokenType()};
    QStringView xmlName {xml.name()};
    if (xmlName == NELusanCommon::xmlElementWorkspaceList)
    {
        QXmlStreamAttributes attrs = xml.attributes();
        mDefWorkspace = attrs.hasAttribute(NELusanCommon::xmlAttributeDefault) ? attrs.value(NELusanCommon::xmlAttributeDefault).toUInt() : 0;
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

const WorkspaceEntry& OptionsManager::_findWorkspace(const QString& root) const
{
    for (const auto& entry : mWorkspaces)
    {
        if (entry.getWorkspaceRoot() == root)
        {
            return entry;
        }
    }

    return WorkspaceEntry::InvalidWorkspace;
}

const WorkspaceEntry& OptionsManager::_findWorkspace(uint64_t id) const
{
    for (const auto& entry : mWorkspaces)
    {
        if (entry.getId() == id)
        {
            return entry;
        }
    }

    return WorkspaceEntry::InvalidWorkspace;
}

inline bool OptionsManager::_existWorkspaceId(uint64_t workspaceId) const
{
    for (const auto& entry : mWorkspaces)
    {
        if (entry.getId() == workspaceId)
        {
            return true;
        }
    }

    return false;
}
