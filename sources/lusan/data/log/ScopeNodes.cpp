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
 *  \file        lusan/data/log/ScopeNodes.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope node classes.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/ScopeNodes.hpp"

#include "areg/component/NEService.hpp"

//////////////////////////////////////////////////////////////////////////
// ScopeLeaf class implementation
//////////////////////////////////////////////////////////////////////////

ScopeLeaf::ScopeLeaf(ScopeNode* parent)
    : ScopeNodeBase (ScopeNodeBase::eNode::Leaf, parent)
{
}

ScopeLeaf::ScopeLeaf(const QString leafName, uint32_t prio, ScopeNode* parent)
    : ScopeNodeBase(ScopeNodeBase::eNode::Leaf, leafName, prio, parent)
{
}

ScopeLeaf::ScopeLeaf(const ScopeNodeBase& base)
    : ScopeNodeBase(ScopeNodeBase::eNode::Leaf, base.getNodeName(), base.getPriority(), base.getParent())
{
}

ScopeLeaf::ScopeLeaf( const ScopeLeaf & src )
    : ScopeNodeBase ( static_cast<const ScopeNodeBase &>(src) )
{
}

ScopeLeaf::ScopeLeaf( ScopeLeaf && src ) noexcept
    : ScopeNodeBase( std::move(static_cast<ScopeNodeBase &>(src)) )
{
}

ScopeNodeBase* ScopeLeaf::makeChildNode(QString& scopePath, uint32_t prio)
{
    ASSERT(scopePath.isEmpty());
    return nullptr;
}

ScopeNodeBase* ScopeLeaf::makeChildNode(QStringList& nodeNames, uint32_t prio)
{
    Q_ASSERT(nodeNames.isEmpty());
    return nullptr;
}

void ScopeLeaf::addChildNode(ScopeNodeBase* childNode)
{
}

//////////////////////////////////////////////////////////////////////////
// ScopeNode class implementation
//////////////////////////////////////////////////////////////////////////

ScopeNode::ScopeNode(ScopeNode* parent)
    : ScopeNodeBase (ScopeNodeBase::eNode::Node, parent)
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::ScopeNode(const QString nodeName, uint32_t prio, ScopeNode* parent)
    : ScopeNodeBase (ScopeNodeBase::eNode::Node, nodeName, prio, parent)
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::ScopeNode(const ScopeNodeBase& base)
    : ScopeNodeBase (ScopeNodeBase::eNode::Node, base.getNodeName(), base.getPriority(), base.getParent())
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::ScopeNode(const ScopeNode& src)
    : ScopeNodeBase ( static_cast<const ScopeNodeBase &>(src) )
    , mChildNodes   ( src.mChildNodes )
    , mChildLeafs   ( src.mChildLeafs )
{
}

ScopeNode::ScopeNode( ScopeNode && src ) noexcept
    : ScopeNodeBase ( static_cast<const ScopeNodeBase &>(src) )
    , mChildNodes   ( std::move(src.mChildNodes) )
    , mChildLeafs   ( std::move(src.mChildLeafs) )
{
}

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, unsigned int prio )
    : ScopeNodeBase ( nodeType, name, prio )
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::~ScopeNode(void)
{
    for (auto node : mChildLeafs )
    {
        Q_ASSERT(node != nullptr);
        delete node;
    }
    
    for (auto node : mChildNodes)
    {
        Q_ASSERT(node != nullptr);
        delete node;
    }
}

ScopeNode & ScopeNode::operator = ( const ScopeNode & src )
{
    ScopeNodeBase::operator = ( static_cast<const ScopeNodeBase &>(src) );
    if ( this != &src )
    {
        mChildNodes = src.mChildNodes;
        mChildLeafs = src.mChildLeafs;
    }

    return (*this);
}

ScopeNode & ScopeNode::operator = ( ScopeNode && src ) noexcept
{
    ScopeNodeBase::operator = ( std::move(static_cast<ScopeNodeBase &&>(src)) );
    if ( this != &src )
    {
        mChildNodes = std::move(src.mChildNodes);
        mChildLeafs = std::move(src.mChildLeafs);
    }

    return (*this);
}

ScopeNodeBase* ScopeNode::makeChildNode(QString& scopePath, uint32_t prio)
{
    ScopeNodeBase* result{ nullptr };
    if (scopePath.isEmpty() == false)
    {
        int pos = scopePath.indexOf(NELusanCommon::SCOPE_SEPRATOR);
        if (pos >= 0)
        {
            QString nodeName = scopePath.first(pos + 1);
            scopePath = scopePath.mid(pos + 1, -1);
            result = new ScopeNode(nodeName, prio, this);
        }
        else
        {
            result = new ScopeLeaf(scopePath, prio, this);
            scopePath.clear();
        }
    }
    
    return result;
}

ScopeNodeBase* ScopeNode::makeChildNode(QStringList& nodeNames, uint32_t prio)
{
    ScopeNodeBase * result { nullptr };
    if (nodeNames.isEmpty() == false)
    {
        QString nodeName = nodeNames.front();
        nodeNames.pop_front();
        if (nodeNames.isEmpty())
        {
            result = new ScopeLeaf(nodeName, prio, this);
        }
        else
        {
            result = new ScopeNode(nodeName, prio, this);
        }
    }
    
    return result;
}

void ScopeNode::addChildNode(ScopeNodeBase* childNode)
{
    if (childNode != nullptr)
    {
        if (childNode->isNode())
        {
            if (mChildNodes.contains(childNode->getNodeName()))
            {
                mChildNodes[childNode->getNodeName()]->addPriority(childNode->getPriority());
                delete childNode;
            }
            else
            {
                mChildNodes[childNode->getNodeName()] = static_cast<ScopeNode *>(childNode);
            }
        }
        else if (childNode->isLeaf())
        {
            Q_ASSERT(mChildLeafs.contains(childNode->getNodeName()) == false);
            mChildLeafs[childNode->getNodeName()] = static_cast<ScopeLeaf *>(childNode);
        }
        else
        {
            delete childNode;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// ScopeRoot class declaration
//////////////////////////////////////////////////////////////////////////

ScopeRoot::ScopeRoot(void)
    : ScopeNode (nullptr)
    , mRootId   (NEService::COOKIE_LOCAL)
{
}

ScopeRoot::ScopeRoot(ITEM_ID rootId)
    : ScopeNode (nullptr)
    , mRootId   (rootId)
{
}

ScopeRoot::ScopeRoot(ITEM_ID rootId, const QString rootName)
    : ScopeNode (rootName, static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset), nullptr)
    , mRootId   (rootId)
{
}

ScopeRoot::ScopeRoot(const ScopeRoot& src)
    : ScopeNode ( static_cast<const ScopeNode &>(src) )
    , mRootId   ( src.mRootId )
{
}

ScopeRoot::ScopeRoot(ScopeRoot&& src) noexcept
    : ScopeNode ( std::move(static_cast<ScopeNode &&>(src)) )
    , mRootId   ( src.mRootId )
{
}

ScopeRoot& ScopeRoot::operator = (const ScopeRoot& src)
{
    ScopeNode::operator = (static_cast<const ScopeNode&>(src));
    mRootId = src.mRootId;
    return (*this);
}

ScopeRoot& ScopeRoot::operator = (ScopeRoot&& src) noexcept
{
    ScopeNode::operator = (std::move(static_cast<ScopeNode&&>(src)));
    mRootId = src.mRootId;
    return (*this);
}

int ScopeRoot::addChildRecursive(QString& scopePath, uint32_t prio)
{
    addPriority(prio);
    return ScopeNode::addChildRecursive(scopePath, prio);
}

int ScopeRoot::addChildRecursive(QStringList& nodeNames, uint32_t prio)
{
    addPriority(prio);
    return ScopeNode::addChildRecursive(nodeNames, prio);
}

QString ScopeRoot::getPathString(void) const
{
    return QString();
}
