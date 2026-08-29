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
 *  \file        lusan/data/log/ScopeNodes.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope node classes.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/ScopeNodes.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "areg/component/ServiceDefs.hpp"

#include <algorithm>

namespace
{
    //! Folds one child roll-up into the roll-up of its parent. The first child sets it.
    inline void _mergeRollup(ScopeNodeBase::sPrioRollup & into, const ScopeNodeBase::sPrioRollup & from, bool & empty)
    {
        if (empty)
        {
            into  = from;
            empty = false;
        }
        else
        {
            into.levelLow  = std::min(into.levelLow , from.levelLow );
            into.levelHigh = std::max(into.levelHigh, from.levelHigh);
            into.linesSome = into.linesSome || from.linesSome;
            into.linesAll  = into.linesAll  && from.linesAll;
            into.prioSome  = into.prioSome  || from.prioSome;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// ScopeLeaf class implementation
//////////////////////////////////////////////////////////////////////////

ScopeLeaf::ScopeLeaf(ScopeNode* parent)
    : ScopeNodeBase ( ScopeNodeBase::eNode::Leaf, parent )
    , mScopeId      ( areg::LOG_SCOPE_ID_NONE )
{
}

ScopeLeaf::ScopeLeaf(const QString& leafName, uint32_t prio, ScopeNode* parent)
    : ScopeNodeBase ( ScopeNodeBase::eNode::Leaf, leafName, prio, parent )
    , mScopeId      ( areg::LOG_SCOPE_ID_NONE )
{
}

ScopeLeaf::ScopeLeaf(const ScopeNodeBase& base)
    : ScopeNodeBase(ScopeNodeBase::eNode::Leaf, base.getNodeName(), base.getPriority(), base.getParent())
    , mScopeId      ( areg::LOG_SCOPE_ID_NONE )
{
}

ScopeLeaf::ScopeLeaf( const ScopeLeaf & src )
    : ScopeNodeBase ( static_cast<const ScopeNodeBase &>(src) )
    , mScopeId      ( areg::LOG_SCOPE_ID_NONE )
{
}

ScopeLeaf::ScopeLeaf( ScopeLeaf && src ) noexcept
    : ScopeNodeBase ( std::move(static_cast<ScopeNodeBase &>(src)) )
    , mScopeId      ( areg::LOG_SCOPE_ID_NONE )
{
}

void ScopeLeaf::addPriority(unsigned int prio)
{
    ScopeNodeBase::setPriority(prio);
}

QString ScopeLeaf::makePath() const
{
    // The parent path ends with '_'. Swap it for the leaf separator '.'.
    QString parentPath = mParent != nullptr ? mParent->makePath() : QString();
    if (parentPath.isEmpty() == false && parentPath.back() == NELusanCommon::SCOPE_SEPRATOR)
    {
        parentPath.chop(1);
        return parentPath + NELusanCommon::SCOPE_LEAF_SEPRATOR + mNodeName;
    }

    return parentPath + mNodeName;
}

void ScopeLeaf::setScopeId(uint32_t scopeId)
{
    mScopeId = scopeId;
}

uint32_t ScopeLeaf::getScopeId() const
{
    return mScopeId;
}


//////////////////////////////////////////////////////////////////////////
// ScopeNode class implementation
//////////////////////////////////////////////////////////////////////////

ScopeNode::ScopeNode(ScopeNode* parent)
    : ScopeNodeBase (ScopeNodeBase::eNode::Node, parent)
    , mChildNodes   ( )
    , mChildLeafs   ( )
    , mShownState   ( Qt::CheckState::Checked )
    , mPrioRollup   ( ScopeNodeBase::priorityRollup() )
{
}

ScopeNode::ScopeNode(const QString& nodeName, uint32_t prio, ScopeNode* parent)
    : ScopeNodeBase (ScopeNodeBase::eNode::Node, nodeName, prio, parent)
    , mChildNodes   ( )
    , mChildLeafs   ( )
    , mShownState   ( Qt::CheckState::Checked )
    , mPrioRollup   ( ScopeNodeBase::priorityRollup() )
{
}

ScopeNode::ScopeNode(const ScopeNodeBase& base)
    : ScopeNodeBase (ScopeNodeBase::eNode::Node, base.getNodeName(), base.getPriority(), base.getParent())
    , mChildNodes   ( )
    , mChildLeafs   ( )
    , mShownState   ( Qt::CheckState::Checked )
    , mPrioRollup   ( ScopeNodeBase::priorityRollup() )
{
}

ScopeNode::ScopeNode(const ScopeNode& src)
    : ScopeNodeBase ( static_cast<const ScopeNodeBase &>(src) )
    , mChildNodes   ( src.mChildNodes )
    , mChildLeafs   ( src.mChildLeafs )
    , mShownState   ( src.mShownState )
    , mPrioRollup   ( src.mPrioRollup )
{
}

ScopeNode::ScopeNode( ScopeNode && src ) noexcept
    : ScopeNodeBase ( static_cast<const ScopeNodeBase &>(src) )
    , mChildNodes   ( std::move(src.mChildNodes) )
    , mChildLeafs   ( std::move(src.mChildLeafs) )
    , mShownState   ( src.mShownState )
    , mPrioRollup   ( src.mPrioRollup )
{
}

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, unsigned int prio, ScopeRoot * parent /*= nullptr*/ )
    : ScopeNodeBase ( nodeType, name, prio, parent )
    , mChildNodes   ( )
    , mChildLeafs   ( )
    , mShownState   ( Qt::CheckState::Checked )
    , mPrioRollup   ( ScopeNodeBase::priorityRollup() )
{
}

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, ScopeRoot * parent /*= nullptr*/ )
    : ScopeNodeBase ( nodeType, name, static_cast<uint32_t>(areg::LogPriority::PrioNotset), parent )
    , mChildNodes   ( )
    , mChildLeafs   ( )
    , mShownState   ( Qt::CheckState::Checked )
    , mPrioRollup   ( ScopeNodeBase::priorityRollup() )
{
}

ScopeNode::ScopeNode( ScopeNodeBase::eNode nodeType, ScopeRoot * parent /*= nullptr*/ )
    : ScopeNodeBase ( nodeType, parent )
    , mChildNodes   ( )
    , mChildLeafs   ( )
    , mShownState   ( Qt::CheckState::Checked )
    , mPrioRollup   ( ScopeNodeBase::priorityRollup() )
{
}

ScopeNode::~ScopeNode()
{
    for (const auto& node : mChildLeafs )
    {
        Q_ASSERT(node.second != nullptr);
        delete node.second;
    }
    
    for (const auto& node : mChildNodes)
    {
        Q_ASSERT(node.second != nullptr);
        delete node.second;
    }

    mChildLeafs.clear();
    mChildNodes.clear();
}

void ScopeNode::setShownRecursive(bool shown)
{
    ScopeNodeBase::setShownRecursive(shown);
    mShownState = shown ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    for (const auto & leaf : mChildLeafs)
    {
        leaf.second->setShownRecursive(shown);
    }

    for (const auto & node : mChildNodes)
    {
        node.second->setShownRecursive(shown);
    }
}

Qt::CheckState ScopeNode::shownState() const
{
    return (childNodeCount() == 0) ? ScopeNodeBase::shownState() : mShownState;
}

void ScopeNode::refreshShownState()
{
    if (childNodeCount() == 0)
    {
        mShownState = ScopeNodeBase::shownState();
        return;
    }

    int shown{ 0 };
    int total{ 0 };
    for (const auto & leaf : mChildLeafs)
    {
        ++total;
        shown += (leaf.second->shownState() == Qt::CheckState::Checked) ? 1 : 0;
    }

    for (const auto & node : mChildNodes)
    {
        const Qt::CheckState state{ node.second->shownState() };
        if (state == Qt::CheckState::PartiallyChecked)
        {
            mShownState = Qt::CheckState::PartiallyChecked;
            return;
        }

        ++total;
        shown += (state == Qt::CheckState::Checked) ? 1 : 0;
    }

    if (shown == 0)
        mShownState = Qt::CheckState::Unchecked;
    else
        mShownState = (shown == total) ? Qt::CheckState::Checked : Qt::CheckState::PartiallyChecked;
}

int ScopeNode::collectHiddenScopes(QSet<uint32_t> & scopeIds) const
{
    int result{ 0 };
    for (const auto & leaf : mChildLeafs)
    {
        result += leaf.second->collectHiddenScopes(scopeIds);
    }

    for (const auto & node : mChildNodes)
    {
        result += node.second->collectHiddenScopes(scopeIds);
    }

    return result;
}

void ScopeNode::removeChildren(void)
{
    for (const auto & leaf : mChildLeafs)
    {
        delete leaf.second;
    }

    for (const auto & node : mChildNodes)
    {
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
        mShownState = src.mShownState;
        mPrioRollup = src.mPrioRollup;
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
        mShownState = src.mShownState;
        mPrioRollup = src.mPrioRollup;
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
                if (existing->hasPrioValid() == false)
                {
                    existing->mPrioStates = childNode->getPriority();
                }
                else
                {
                    existing->mPrioStates |= childNode->getPriority();
                }
                
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
    int result{ static_cast<int>(areg::INVALID_INDEX) };
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

    if (result == static_cast<int>(areg::INVALID_INDEX))
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

int ScopeNode::getChildCount() const
{
    return static_cast<int>(mChildNodes.size() + mChildLeafs.size());
}

int ScopeNode::getChildNodesCount() const
{
    return static_cast<int>(mChildNodes.size());
}

int ScopeNode::getChildLeafsCount() const
{
    return static_cast<int>(mChildLeafs.size());
}

bool ScopeNode::hasNodes() const
{
    return (mChildNodes.empty() == false);
}

bool ScopeNode::hasLeafs() const
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

void ScopeNode::refreshPrioritiesRecursive()
{
    uint32_t states{ static_cast<uint32_t>(areg::LogPriority::PrioInvalid) };
    ScopeNodeBase::sPrioRollup rollup{ };
    bool empty{ true };

    for (const auto& node : mChildNodes)
    {
        ScopeNode* child = node.second;
        Q_ASSERT(child != nullptr);
        child->refreshPrioritiesRecursive();
        states |= child->getPriority();
        _mergeRollup(rollup, child->priorityRollup(), empty);
    }

    for (const auto& leaf : mChildLeafs)
    {
        ScopeNodeBase* child = leaf.second;
        Q_ASSERT(child != nullptr);
        states |= child->getPriority();
        _mergeRollup(rollup, child->priorityRollup(), empty);
    }

    if (empty == false)
    {
        mPrioStates = states;
        mPrioRollup = rollup;
    }
    else
    {
        mPrioRollup = ScopeNodeBase::priorityRollup();
    }
}

ScopeNodeBase::sPrioRollup ScopeNode::priorityRollup() const
{
    return (childNodeCount() == 0) ? ScopeNodeBase::priorityRollup() : mPrioRollup;
}

QList<ScopeNodeBase*> ScopeNode::getNodesWithPriority() const
{
    QList<ScopeNodeBase*> result = ScopeNodeBase::getNodesWithPriority();
    if (result.isEmpty())
    {
        for (const auto& node : mChildNodes)
        {
            QList<ScopeNodeBase*> list = node.second->getNodesWithPriority();
            if (list.isEmpty() == false)
                result.append(list);
        }
        
        for (const auto& node : mChildLeafs)
        {
            QList<ScopeNodeBase*> list = node.second->getNodesWithPriority();
            if (list.isEmpty() == false)
                result.append(list);
        }
    }
    
    return result;
}

int ScopeNode::extractNodesWithPriority(QList<ScopeNodeBase*>& list) const
{
    int result{ ScopeNodeBase::extractNodesWithPriority(list) };
    if (result == 0)
    {
        for (const auto& node : mChildNodes)
        {
            result += node.second->extractNodesWithPriority(list);
        }

        for (const auto& node : mChildLeafs)
        {
            result += node.second->extractNodesWithPriority(list);
        }
    }

    return result;
}

int ScopeNode::extractChildNodesWithPriority(QList<ScopeNodeBase*>& list) const
{
    int result{ 0 };
    for (const auto& node : mChildNodes)
    {
        result += node.second->extractNodesWithPriority(list);
    }

    for (const auto& node : mChildLeafs)
    {
        result += node.second->extractNodesWithPriority(list);
    }

    return result;
}

std::vector<ScopeNodeBase*> ScopeNode::extractNodeLeafs() const
{
    std::vector<ScopeNodeBase*> leafs;
    extractNodeLeafs(leafs);
    return leafs;
}

uint32_t ScopeNode::extractNodeLeafs(std::vector<ScopeNodeBase*>& leafs) const
{
    for (auto node : mChildLeafs)
    {
        leafs.push_back(node.second);
    }

    for (const auto& node : mChildNodes)
    {
        node.second->extractNodeLeafs(leafs);
    }

    return static_cast<uint32_t>(leafs.size());
}

//////////////////////////////////////////////////////////////////////////
// ScopeRoot class declaration
//////////////////////////////////////////////////////////////////////////

ScopeRoot::ScopeRoot()
    : ScopeNode (ScopeNodeBase::eNode::Root, nullptr)
    , mRootId   (areg::COOKIE_LOCAL)
    , mInstance ()
    , mLocation ()
    , mConnected(true)
    , mSavedPrio()
    , mTargetState(ScopeRoot::eTargetState::TargetApplied)
    , mTargetAge  (0)
    , mTargetFade (0.0)
{
}

ScopeRoot::ScopeRoot(ITEM_ID rootId)
    : ScopeNode (ScopeNodeBase::eNode::Root, nullptr)
    , mRootId   (rootId)
    , mInstance ()
    , mLocation ()
    , mConnected(true)
    , mSavedPrio()
    , mTargetState(ScopeRoot::eTargetState::TargetApplied)
    , mTargetAge  (0)
    , mTargetFade (0.0)
{
}

ScopeRoot::ScopeRoot(const areg::ConnectedInstance& instance)
    : ScopeNode (ScopeNodeBase::eNode::Root, QString(instance.ciInstance.c_str()), static_cast<uint32_t>(areg::LogPriority::PrioNotset), nullptr)
    , mRootId   (instance.ciCookie)
    , mInstance (QString::fromStdString(instance.ciInstance))
    , mLocation (QString::fromStdString(instance.ciLocation))
    , mConnected(true)
    , mSavedPrio()
    , mTargetState(ScopeRoot::eTargetState::TargetApplied)
    , mTargetAge  (0)
    , mTargetFade (0.0)
{
}

ScopeRoot::ScopeRoot(ITEM_ID rootId, const QString& rootName)
    : ScopeNode (ScopeNodeBase::eNode::Root, rootName, static_cast<uint32_t>(areg::LogPriority::PrioNotset), nullptr)
    , mRootId   (rootId)
    , mInstance (rootName)
    , mLocation ()
    , mConnected(true)
    , mSavedPrio()
    , mTargetState(ScopeRoot::eTargetState::TargetApplied)
    , mTargetAge  (0)
    , mTargetFade (0.0)
{
}

ScopeRoot::ScopeRoot(const ScopeRoot& src)
    : ScopeNode ( static_cast<const ScopeNode &>(src) )
    , mRootId   ( src.mRootId )
    , mInstance ( src.mInstance )
    , mLocation ( src.mLocation )
    , mConnected( src.mConnected )
    , mSavedPrio( src.mSavedPrio )
    , mTargetState( src.mTargetState )
    , mTargetAge  ( src.mTargetAge )
    , mTargetFade ( src.mTargetFade )
{
}

ScopeRoot::ScopeRoot(ScopeRoot&& src) noexcept
    : ScopeNode ( std::move(static_cast<ScopeNode &&>(src)) )
    , mRootId   ( src.mRootId )
    , mInstance ( std::move(src.mInstance) )
    , mLocation ( std::move(src.mLocation) )
    , mConnected( src.mConnected )
    , mSavedPrio( std::move(src.mSavedPrio) )
    , mTargetState( src.mTargetState )
    , mTargetAge  ( src.mTargetAge )
    , mTargetFade ( src.mTargetFade )
{
}

ScopeRoot& ScopeRoot::operator = (const ScopeRoot& src)
{
    ScopeNode::operator = (static_cast<const ScopeNode&>(src));
    mRootId    = src.mRootId;
    mInstance  = src.mInstance;
    mLocation  = src.mLocation;
    mConnected = src.mConnected;
    mSavedPrio = src.mSavedPrio;
    mTargetState = src.mTargetState;
    mTargetAge = src.mTargetAge;
    mTargetFade = src.mTargetFade;
    return (*this);
}

ScopeRoot& ScopeRoot::operator = (ScopeRoot&& src) noexcept
{
    ScopeNode::operator = (std::move(static_cast<ScopeNode&&>(src)));
    mRootId    = src.mRootId;
    mInstance  = std::move(src.mInstance);
    mLocation  = std::move(src.mLocation);
    mConnected = src.mConnected;
    mSavedPrio = std::move(src.mSavedPrio);
    mTargetState = src.mTargetState;
    mTargetAge = src.mTargetAge;
    mTargetFade = src.mTargetFade;
    return (*this);
}

QString ScopeRoot::getPathString() const
{
    return QString();
}

QString ScopeRoot::getDisplayName() const
{
    QString result {getNodeName() + " (" + QString::number(mRootId) + ")"};
    return result;
}

bool ScopeRoot::isSameInstance(const areg::ConnectedInstance & instance) const
{
    return ( (mInstance == QString::fromStdString(instance.ciInstance))
          && (mLocation == QString::fromStdString(instance.ciLocation)) );
}

void ScopeRoot::setTargetState(ScopeRoot::eTargetState state)
{
    mTargetState = state;
    mTargetAge = 0;
    mTargetFade = (state == ScopeRoot::eTargetState::TargetApplied) ? 0.0 : 1.0;
}

bool ScopeRoot::ageTargetState(int elapsedMs)
{
    if (isTargetAgeing() == false)
        return false;

    mTargetAge += elapsedMs;

    if (mTargetState == ScopeRoot::eTargetState::TargetSent)
    {
        if (mTargetAge < ScopeRoot::TargetWaitMs)
            return false;

        setTargetState(ScopeRoot::eTargetState::TargetPending);
        return true;
    }

    mTargetFade = 1.0 - (static_cast<qreal>(mTargetAge) / static_cast<qreal>(ScopeRoot::TargetFadeMs));
    if (mTargetFade <= 0.0)
    {
        setTargetState(ScopeRoot::eTargetState::TargetApplied);
    }

    return true;
}

void ScopeRoot::savePriorities(void)
{
    mSavedPrio.clear();
    QList<ScopeNodeBase *> nodes;
    extractChildNodesWithPriority(nodes);
    for (const ScopeNodeBase * node : nodes)
    {
        Q_ASSERT(node != nullptr);
        mSavedPrio.insert(node->makePath(), node->getPriority());
    }
}

int ScopeRoot::restorePriorities(void)
{
    int result{ 0 };
    for (auto it = mSavedPrio.constBegin(); it != mSavedPrio.constEnd(); ++it)
    {
        ScopeNodeBase * node = findChildByPath(it.key());
        if (node != nullptr)
        {
            node->setPriority(it.value());
            ++result;
        }
    }

    if (result != 0)
    {
        resetPrioritiesRecursive(true);
        refreshPrioritiesRecursive();
    }

    return result;
}
