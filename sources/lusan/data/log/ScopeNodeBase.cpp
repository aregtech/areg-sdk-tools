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
 *  \file        lusan/data/log/ScopeNodeBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope base class.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/ScopeNodeBase.hpp"

ScopeNodeBase::ScopeNodeBase(void)
    : mNodeType     ( ScopeNodeBase::eNode::Invalid )
    , mParent       ( nullptr )
    , mPrioStates   ( static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid) )
    , mNodeName     ( )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase::eNode nodeType, ScopeNodeBase* parent /*= nullptr*/)
    : mNodeType     ( nodeType )
    , mParent       ( parent )
    , mPrioStates   ( static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid) )
    , mNodeName     ( )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase::eNode nodeType, const QString& nodeName, unsigned int prio, ScopeNodeBase* parent)
    : mNodeType     ( nodeType )
    , mParent       ( parent )
    , mPrioStates   ( prio )
    , mNodeName     ( nodeName )
{
}

ScopeNodeBase::ScopeNodeBase(const ScopeNodeBase& src)
    : mNodeType     ( src.mNodeType )
    , mParent       ( src.mParent )
    , mPrioStates   ( src.mPrioStates )
    , mNodeName     ( src.mNodeName )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase&& src) noexcept
    : mNodeType     ( src.mNodeType )
    , mParent       ( src.mParent )
    , mPrioStates   ( src.mPrioStates )
    , mNodeName     ( std::move(src.mNodeName) )
{
}

ScopeNodeBase& ScopeNodeBase::operator = (const ScopeNodeBase& src)
{
    ASSERT(mNodeType == src.mNodeType);
    if (this != &src)
    {
        mPrioStates = src.mPrioStates;
        mNodeName   = src.mNodeName;
    }

    return (*this);
}

ScopeNodeBase& ScopeNodeBase::operator = (ScopeNodeBase&& src) noexcept
{
    ASSERT(mNodeType == src.mNodeType);
    if (this != &src)
    {
        mPrioStates = src.mPrioStates;
        mNodeName   = std::move(src.mNodeName);
    }

    return (*this);
}

bool ScopeNodeBase::operator == (const ScopeNodeBase& other) const
{
    return (mNodeType == other.mNodeType) && (mNodeName == other.mNodeName);
}

bool ScopeNodeBase::operator != (const ScopeNodeBase& other) const
{
    return (mNodeType != other.mNodeType) || (mNodeName != other.mNodeName);
}

bool ScopeNodeBase::operator > (const ScopeNodeBase& other) const
{
    return (mNodeType == other.mNodeType ? (mNodeName > other.mNodeName) : (mNodeType > other.mNodeType));
}

bool ScopeNodeBase::operator < (const ScopeNodeBase& other) const
{
    return (mNodeType == other.mNodeType ? (mNodeName < other.mNodeName) : (mNodeType < other.mNodeType));
}

unsigned int ScopeNodeBase::getPriority( void ) const
{
    return mPrioStates;
}

void ScopeNodeBase::setPriority( uint32_t prio)
{
    mPrioStates = (hasLogScopes() ? (prio | static_cast<uint32_t>(NELogging::eLogPriority::PrioScope)) : prio);
}

void ScopeNodeBase::addPriority( unsigned int prio )
{
    if ((hasPrioValid() == false) || isLeaf())
    {
        ScopeNodeBase::setPriority(prio);
    }
    else
    {
        mPrioStates |= prio;
    }
}

void ScopeNodeBase::removePriority(unsigned int prio)
{
    mPrioStates &= ~prio;
    if (hasPrioValid() == false)
    {
        mPrioStates = static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset);
    }
}

int ScopeNodeBase::addChildRecursive(QString& scopePath, uint32_t prio)
{
    QStringList scopeNodes;
    if (splitScopePath(scopePath, scopeNodes) != 0)
    {
        return addChildRecursive(scopeNodes, prio);
    }
    else
    {
        return 0;
    }
}

int ScopeNodeBase::addChildRecursive(QStringList& scopeNodes, uint32_t prio)
{
    ScopeNodeBase* node = addChildNode(scopeNodes, prio);
    return ((node != nullptr) && (node->isValid()) ? 1 + node->addChildRecursive(scopeNodes, prio) : 0);
}

ScopeNodeBase* ScopeNodeBase::addChildNode(QString& scopePath, uint32_t prio)
{
    QStringList scopeNodes;
    if (splitScopePath(scopePath, scopeNodes) != 0)
    {
        return addChildNode(scopeNodes, prio);
    }
    else
    {
        return nullptr;
    }
}

ScopeNodeBase* ScopeNodeBase::addChildNode(QStringList& nodeNames, uint32_t prio)
{
    ScopeNodeBase* childNode = makeChildNode(nodeNames, prio);
    return addChildNode(childNode);
}

ScopeNodeBase* ScopeNodeBase::addChildNode(ScopeNodeBase* childNode)
{
    delete childNode;
    return nullptr;
}

ScopeNodeBase* ScopeNodeBase::makeChildNode(QString& scopePath, uint32_t prio)
{
    QStringList scopeNodes;
    if (splitScopePath(scopePath, scopeNodes) != 0)
    {
        return makeChildNode(scopeNodes, prio);
    }
    else
    {
        return nullptr;
    }
}

ScopeNodeBase* ScopeNodeBase::makeChildNode(QStringList& nodeNames, uint32_t prio)
{
    Q_ASSERT(nodeNames.isEmpty());
    return nullptr;
}

QString ScopeNodeBase::makePath(void) const
{
    QString result(mParent != nullptr ? mParent->makePath() : getPathString());
    
    if (isRoot() == false)
    {
        result += getPathString();
    }
    
    if (isNode())
    {
        result += NELusanCommon::SCOPE_SEPRATOR;
    }
    
    return result;
}

QString ScopeNodeBase::getPathString(void) const
{
    return mNodeName;
}

ScopeNodeBase* ScopeNodeBase::findChild(const QString& childName) const
{
    return nullptr;
}

ScopeNodeBase* ScopeNodeBase::findChildByPath(const QString& childPath) const
{
    QStringList nameList = childPath.split(NELusanCommon::SCOPE_SEPRATOR);
    const ScopeNodeBase* node{ nameList.isEmpty() ? nullptr : this };
    while ((nameList.isEmpty() == false) && (node != nullptr))
    {
        QString nodeName{ nameList.front() };
        nameList.pop_front();
        node = node->findChild(nodeName);
    }

    return const_cast<ScopeNodeBase *>(node);
}

int ScopeNodeBase::getChildPosition(const QString& childName) const
{
    return static_cast<int>(NECommon::INVALID_INDEX);
}

ScopeNodeBase* ScopeNodeBase::getChildAt(int pos) const
{
    return nullptr;
}

int ScopeNodeBase::getChildCount(void) const
{
    return 0;
}

int ScopeNodeBase::getChildNodesCount(void) const
{
    return 0;
}

int ScopeNodeBase::getChildLeafsCount(void) const
{
    return 0;
}

void ScopeNodeBase::addChildPriorityRecursive(QString& nodePath, uint32_t prio)
{
    QStringList nameList = nodePath.split(NELusanCommon::SCOPE_SEPRATOR);
    addChildPriorityRecursive(nameList, prio);
}

void ScopeNodeBase::addChildPriorityRecursive(QStringList& pathList, uint32_t prio)
{
    if (pathList.isEmpty() == false)
    {
        QString nodeName{ pathList.front() };
        pathList.pop_front();
        ScopeNodeBase* child = findChild(nodeName);
        if (child != nullptr)
        {
            child->addChildPriorityRecursive(pathList, prio);
            prio = child->getPriority();
        }
    }
    
    if (hasPrioValid() && (isLeaf() == false))
    {
        mPrioStates |= prio;
    }
    else
    {
        ScopeNodeBase::setPriority(prio);
    }
}

void ScopeNodeBase::removeChildPriorityRecursive(QString& nodePath, uint32_t prio)
{
    QStringList nameList = nodePath.split(NELusanCommon::SCOPE_SEPRATOR);
    removeChildPriorityRecursive(nameList, prio);
}

void ScopeNodeBase::removeChildPriorityRecursive(QStringList& pathList, uint32_t prio)
{
    if (pathList.isEmpty() == false)
    {
        QString nodeName{ pathList.front() };
        pathList.pop_front();
        ScopeNodeBase* child = findChild(nodeName);
        if (child != nullptr)
        {
            child->addChildPriorityRecursive(pathList, prio);
            removePriority(prio);
        }
    }
    else
    {
        removePriority(prio);
    }
}

bool ScopeNodeBase::hasNodes(void) const
{
    return false;
}

bool ScopeNodeBase::hasLeafs(void) const
{
    return false;
}

bool ScopeNodeBase::containsLeaf(const QString& leafName) const
{
    return false;
}

bool ScopeNodeBase::containsNode(const QString& nodeName) const
{
    return false;
}

int ScopeNodeBase::getChildren(std::vector<ScopeNodeBase*>& children) const
{
    return 0;
}

void ScopeNodeBase::resetPrioritiesRecursive(bool skipLeafs)
{
    if ((isLeaf() == false) || (skipLeafs == false))
        resetPriority();
}

void ScopeNodeBase::refreshPrioritiesRecursive(void)
{
}

QList<ScopeNodeBase*> ScopeNodeBase::getNodesWithPriority(void) const
{
    QList<ScopeNodeBase*> result;
    if (hasPrioValid() && (hasPrioNotset() == false))
        result.push_back(const_cast<ScopeNodeBase *>(this));
    
    return result;
}

int ScopeNodeBase::extractNodesWithPriority(QList<ScopeNodeBase*>& list) const
{
    int result{ 0 };
    if (hasPrioValid() && (hasMultiPrio(static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)) == false))
    {
        list.push_back(const_cast<ScopeNodeBase *>(this));
        result = 1;
    }

    return result;
}

int ScopeNodeBase::splitScopePath(QString& scopePath, QStringList& nodeNames) const
{
    QStringList nodes = scopePath.split(NELusanCommon::SCOPE_SEPRATOR, Qt::KeepEmptyParts);
    QString prefix, postfix;
    
    for (int i = 0; i < static_cast<int>(nodes.size()); ++ i)
    {
        const QString & name = nodes[i];
        if (name.isEmpty())
        {
            if ((i == (static_cast<int>(nodes.size()) - 1)) && (nodeNames.isEmpty() == false))
            {
                postfix += NELusanCommon::SCOPE_SEPRATOR;
                nodeNames[nodeNames.size() - 1] = prefix + nodeNames[nodeNames.size() - 1] + postfix;
                prefix.clear();
                postfix.clear();
            }
            else
            {
                prefix += NELusanCommon::SCOPE_SEPRATOR;
            }
        }
        else
        {
            nodeNames.push_back(prefix + name + postfix);
            prefix.clear();
            postfix.clear();
        }
    }
    
    return static_cast<int>(nodeNames.size());
}
