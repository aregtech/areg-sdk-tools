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
        mNodeName = src.mNodeName;
    }

    return (*this);
}

ScopeNodeBase& ScopeNodeBase::operator = (ScopeNodeBase&& src) noexcept
{
    ASSERT(mNodeType == src.mNodeType);
    if (this != &src)
    {
        mPrioStates = src.mPrioStates;
        mNodeName = std::move(src.mNodeName);
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
    addChildNode(childNode);
    return childNode;
}

ScopeNodeBase* ScopeNodeBase::addChildNode(QStringList& nodeNames, uint32_t prio)
{
    ScopeNodeBase* childNode = makeChildNode(nodeNames, prio);
    addChildNode(childNode);
    return childNode;
}

ScopeNodeBase* ScopeNodeBase::makeChildNode(QString& scopePath, uint32_t prio)
{
    return nullptr;
}

ScopeNodeBase* ScopeNodeBase::makeChildNode(QStringList& nodeNames, uint32_t prio)
{
    return nullptr;
}

void ScopeNodeBase::addChildNode(ScopeNodeBase* childNode)
{
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

    
