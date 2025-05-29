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

void ScopeLeaf::addPriority(unsigned int prio)
{
    ScopeNodeBase::setPriority(prio);
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

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, unsigned int prio, ScopeRoot * parent /*= nullptr*/ )
    : ScopeNodeBase ( nodeType, name, prio, parent )
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, ScopeRoot * parent /*= nullptr*/ )
    : ScopeNodeBase ( nodeType, name, static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset), parent )
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, ScopeRoot * parent /*= nullptr*/ )
    : ScopeNodeBase ( nodeType, parent )
    , mChildNodes   ( )
    , mChildLeafs   ( )
{
}

ScopeNode::~ScopeNode(void)
{
    for (auto node : mChildLeafs )
    {
        Q_ASSERT(node.second != nullptr);
        delete node.second;
    }
    
    for (auto node : mChildNodes)
    {
        Q_ASSERT(node.second != nullptr);
        delete node.second;
    }

    mChildLeafs.clear();
    mChildNodes.clear();
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

void ScopeNode::setPriority( uint32_t prio)
{
    ScopeNodeBase::setPriority(prio);

    for (auto & child : mChildLeafs)
    {
        child.second->setPriority(prio);
    }

    for (auto & child : mChildNodes)
    {
        child.second->setPriority(prio);
    }
}

void ScopeNode::addPriority(uint32_t prio)
{
    ScopeNodeBase::addPriority(prio);

    for (auto& child : mChildLeafs)
    {
        child.second->addPriority(prio);
    }

    for (auto& child : mChildNodes)
    {
        child.second->addPriority(prio);
    }
}

void ScopeNode::removePriority(uint32_t prio)
{
    ScopeNodeBase::removePriority(prio);

    for (auto& child : mChildLeafs)
    {
        child.second->removePriority(prio);
    }

    for (auto& child : mChildNodes)
    {
        child.second->removePriority(prio);
    }
}

ScopeNodeBase* ScopeNode::makeChildNode(QString& scopePath, uint32_t prio)
{
    ScopeNodeBase* result{ nullptr };
    if (scopePath.isEmpty() == false)
    {
        int pos = scopePath.indexOf(NELusanCommon::SCOPE_SEPRATOR);
        if (pos >= 0)
        {
            QString nodeName = scopePath.first(pos);
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

ScopeNodeBase* ScopeNode::addChildNode(ScopeNodeBase* childNode)
{
    if (childNode != nullptr)
    {
        if (childNode->isNode())
        {
            ScopeNode* existing{ containsNode(childNode->getNodeName()) ? mChildNodes[childNode->getNodeName()] : nullptr };
            if (existing != nullptr)
            {
                existing->addPriority(childNode->getPriority());
                delete childNode;
                childNode = existing;
            }
            else
            {
                mChildNodes[childNode->getNodeName()] = static_cast<ScopeNode*>(childNode);
            }
        }
        else if (childNode->isLeaf())
        {
            ScopeLeaf* existing{ containsLeaf(childNode->getNodeName()) ? mChildLeafs[childNode->getNodeName()] : nullptr };
            mChildLeafs[childNode->getNodeName()] = static_cast<ScopeLeaf*>(childNode);
            if (existing != nullptr)
            {
                delete existing;
            }
        }
        else
        {
            delete childNode;
            childNode = nullptr;
        }
    }

    return childNode;
}

ScopeNodeBase* ScopeNode::findChild(const QString& childName) const
{
    ScopeNodeBase* result{ nullptr };
    NodeList::const_iterator itNode = mChildNodes.find(childName);
    if (itNode != mChildNodes.end())
    {
        result = itNode->second;
    }
    else
    {
        LeafList::const_iterator itLeaf = mChildLeafs.find(childName);
        if (itLeaf != mChildLeafs.end())
        {
            result = itLeaf->second;
        }
    }

    return result;
}

int ScopeNode::getChildPosition(const QString& childName) const
{
    int result{ static_cast<int>(NECommon::INVALID_INDEX) };
    int pos = 0;
    for (const auto & node : mChildNodes)
    {
        if (node.second->getNodeName() == childName)
        {
            result = pos;
            break;
        }

        ++pos;
    }

    if (result == static_cast<int>(NECommon::INVALID_INDEX))
    {
        pos = 0;
        for (const auto & node : mChildLeafs)
        {
            if (node.second->getNodeName() == childName)
            {
                result = pos;
                break;
            }

            ++pos;
        }
    }

    return result;
}

ScopeNodeBase* ScopeNode::getChildAt(int pos) const
{
    ScopeNodeBase* result = nullptr;
    int cntNode = static_cast<int>(mChildNodes.size());
    int cntLeaf = static_cast<int>(mChildLeafs.size());
    
    if ((pos >= 0) && (pos < (cntNode + cntLeaf)))
    {
        if (pos < cntNode)
        {
            auto it = mChildNodes.begin();
            for (int i = 0; i < pos; ++i)
                ++it;
            Q_ASSERT(it != mChildNodes.end());
            return it->second;
        }
        else
        {
            pos -= static_cast<int>(mChildNodes.size());
            Q_ASSERT(pos >= 0);
            auto it = mChildLeafs.begin();
            for (int i = 0; i < pos; ++ i)
                ++it;
            return it->second;
        }
    }
    
    return result;
}

int ScopeNode::getChildCount(void) const
{
    return static_cast<int>(mChildNodes.size() + mChildLeafs.size());
}

int ScopeNode::getChildNodesCount(void) const
{
    return static_cast<int>(mChildNodes.size());
}

int ScopeNode::getChildLeafsCount(void) const
{
    return static_cast<int>(mChildLeafs.size());
}

bool ScopeNode::hasNodes(void) const
{
    return (mChildNodes.empty() == false);
}

bool ScopeNode::hasLeafs(void) const
{
    return (mChildLeafs.empty() == false);
}

bool ScopeNode::containsLeaf(const QString& leafName) const
{
    return (mChildLeafs.find(leafName) != mChildLeafs.end());
}

bool ScopeNode::containsNode(const QString& nodeName) const
{
    return (mChildNodes.find(nodeName) != mChildNodes.end());
}

int ScopeNode::getChildren(std::vector<ScopeNodeBase*>& children) const
{
    const auto extractValue = [](const auto &key) { return key.second; };

    std::transform(mChildNodes.cbegin(),
                   mChildNodes.cend(),
                   std::back_inserter(children),
                   extractValue);

    std::transform(mChildLeafs.cbegin(),
                   mChildLeafs.cend(),
                   std::back_inserter(children),
                   extractValue);
    return static_cast<int>(children.size());
}

void ScopeNode::resetPrioritiesRecursive(bool skipLeafs /*= false*/)
{
    ScopeNodeBase::resetPrioritiesRecursive(skipLeafs);
    for (const auto & node : mChildNodes)
    {
        node.second->resetPrioritiesRecursive(skipLeafs);
    }
    
    if (skipLeafs == false)
    {
        for (const auto & node : mChildLeafs)
        {
            node.second->resetPrioritiesRecursive(skipLeafs);
        }
    }
}

void ScopeNode::refreshPrioritiesRecursive(void)
{
    for (auto node : mChildNodes)
    {
        node.second->refreshPrioritiesRecursive();
    }

    for (auto leaf : mChildLeafs)
    {
        ScopeNodeBase* node = leaf.second;
        Q_ASSERT(node != nullptr);
        node->updateParentPrio(node->getPriority(), true);
    }
}

//////////////////////////////////////////////////////////////////////////
// ScopeRoot class declaration
//////////////////////////////////////////////////////////////////////////

ScopeRoot::ScopeRoot(void)
    : ScopeNode (ScopeNodeBase::eNode::Root, nullptr)
    , mRootId   (NEService::COOKIE_LOCAL)
{
}

ScopeRoot::ScopeRoot(ITEM_ID rootId)
    : ScopeNode (ScopeNodeBase::eNode::Root, nullptr)
    , mRootId   (rootId)
{
}

ScopeRoot::ScopeRoot(const NEService::sServiceConnectedInstance& instance)
    : ScopeNode (ScopeNodeBase::eNode::Root, QString(instance.ciInstance.c_str()), static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset), nullptr)
    , mRootId   (instance.ciCookie)
{
}

ScopeRoot::ScopeRoot(ITEM_ID rootId, const QString rootName)
    : ScopeNode (ScopeNodeBase::eNode::Root, rootName, static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset), nullptr)
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
