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
    , mIcon         ( )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase::eNode nodeType, ScopeNodeBase* parent /*= nullptr*/)
    : mNodeType     ( nodeType )
    , mParent       ( parent )
    , mPrioStates   ( static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid) )
    , mNodeName     ( )
    , mIcon         ( nodeType != ScopeNodeBase::eNode::Leaf ? QIcon::fromTheme(QIcon::ThemeIcon::ListAdd) : QIcon())
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase::eNode nodeType, const QString& nodeName, unsigned int prio, ScopeNodeBase* parent)
    : mNodeType     ( nodeType )
    , mParent       ( parent )
    , mPrioStates   ( prio )
    , mNodeName     ( nodeName )
    , mIcon         ( nodeType != ScopeNodeBase::eNode::Leaf ? QIcon::fromTheme(QIcon::ThemeIcon::ListAdd) : QIcon())
{
}

ScopeNodeBase::ScopeNodeBase(const ScopeNodeBase& src)
    : mNodeType     ( src.mNodeType )
    , mParent       ( src.mParent )
    , mPrioStates   ( src.mPrioStates )
    , mNodeName     ( src.mNodeName )
    , mIcon         ( src.mIcon)
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase&& src) noexcept
    : mNodeType     ( src.mNodeType )
    , mParent       ( src.mParent )
    , mPrioStates   ( src.mPrioStates )
    , mNodeName     ( std::move(src.mNodeName) )
    , mIcon         ( std::move(src.mIcon) )
{
}

ScopeNodeBase& ScopeNodeBase::operator = (const ScopeNodeBase& src)
{
    ASSERT(mNodeType == src.mNodeType);
    if (this != &src)
    {
        mPrioStates = src.mPrioStates;
        mNodeName   = src.mNodeName;
        mIcon       = src.mIcon;
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
        mIcon       = std::move(src.mIcon);
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

void ScopeNodeBase::setPriority( unsigned int prio )
{
    mPrioStates = prio;
}

void ScopeNodeBase::addPriority( unsigned int prio )
{
    mPrioStates |= prio;
}

void ScopeNodeBase::removePriority(unsigned int prio)
{
    mPrioStates &= ~prio;
}

int ScopeNodeBase::addChildRecursive(QString& scopePath, uint32_t prio)
{
    ScopeNodeBase* node = addChildNode(scopePath, prio);
    return ((node != nullptr) && (node->isValid()) ? 1 + node->addChildRecursive(scopePath, prio) : 0);
}

int ScopeNodeBase::addChildRecursive(QStringList& scopeNodes, uint32_t prio)
{
    ScopeNodeBase* node = addChildNode(scopeNodes, prio);
    return ((node != nullptr) && (node->isValid()) ? 1 + node->addChildRecursive(scopeNodes, prio) : 0);
}

ScopeNodeBase* ScopeNodeBase::addChildNode(QString& scopePath, uint32_t prio)
{
    ScopeNodeBase* childNode = makeChildNode(scopePath, prio);
    return addChildNode(childNode);
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
    ASSERT(scopePath.isEmpty());
    return nullptr;
}

ScopeNodeBase* ScopeNodeBase::makeChildNode(QStringList& nodeNames, uint32_t prio)
{
    Q_ASSERT(nodeNames.isEmpty());
    return nullptr;
}

QStringList ScopeNodeBase::makeNodeNames(const QString& scopePath) const
{
    return scopePath.split(NELusanCommon::SCOPE_SEPRATOR);
}

QString ScopeNodeBase::makePath(void) const
{
    QString result(mParent != nullptr ? mParent->makePath() : getPathString());
    if (result.isEmpty() == false)
    {
        result += NELusanCommon::SCOPE_SEPRATOR + getPathString();
    }
    else if (isRoot() == false)
    {
        result = getPathString();
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
            addPriority(child->getPriority());
        }
    }
    else
    {
        setPriority(prio);
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
    if ((skipLeafs == false) || (isLeaf() == false))
        resetPriority();
}

void ScopeNodeBase::refreshPrioritiesRecursive(void)
{
}
