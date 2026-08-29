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
 *  \file        lusan/data/log/ScopeNodeBase.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope base class.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/ScopeNodeBase.hpp"

ScopeNodeBase::ScopeNodeBase()
    : mNodeType     ( ScopeNodeBase::eNode::Invalid )
    , mNodeState    ( ScopeNodeBase::eNodeState::NodeCollapsed )
    , mParent       ( nullptr )
    , mPrioStates   ( static_cast<uint32_t>(areg::LogPriority::PrioInvalid) )
    , mNodeName     ( )
    , mShown        ( true )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase::eNode nodeType, ScopeNodeBase* parent /*= nullptr*/)
    : mNodeType     ( nodeType )
    , mNodeState    ( ScopeNodeBase::eNodeState::NodeCollapsed )
    , mParent       ( parent )
    , mPrioStates   ( static_cast<uint32_t>(areg::LogPriority::PrioInvalid) )
    , mNodeName     ( )
    , mShown        ( true )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase::eNode nodeType, const QString& nodeName, unsigned int prio, ScopeNodeBase* parent)
    : mNodeType     ( nodeType )
    , mNodeState    ( ScopeNodeBase::eNodeState::NodeCollapsed )
    , mParent       ( parent )
    , mPrioStates   ( prio )
    , mNodeName     ( nodeName )
    , mShown        ( true )
{
}

ScopeNodeBase::ScopeNodeBase(const ScopeNodeBase& src)
    : mNodeType     ( src.mNodeType )
    , mNodeState    ( src.mNodeState )
    , mParent       ( src.mParent )
    , mPrioStates   ( src.mPrioStates )
    , mNodeName     ( src.mNodeName )
    , mShown        ( src.mShown )
{
}

ScopeNodeBase::ScopeNodeBase(ScopeNodeBase&& src) noexcept
    : mNodeType     ( src.mNodeType )
    , mNodeState    ( src.mNodeState )
    , mParent       ( src.mParent )
    , mPrioStates   ( src.mPrioStates )
    , mNodeName     ( std::move(src.mNodeName) )
    , mShown        ( src.mShown )
{
}

ScopeNodeBase& ScopeNodeBase::operator = (const ScopeNodeBase& src)
{
    ASSERT(mNodeType == src.mNodeType);
    if (this != &src)
    {
        mNodeState  = src.mNodeState;
        mPrioStates = src.mPrioStates;
        mNodeName   = src.mNodeName;
        mShown      = src.mShown;
    }

    return (*this);
}

ScopeNodeBase& ScopeNodeBase::operator = (ScopeNodeBase&& src) noexcept
{
    ASSERT(mNodeType == src.mNodeType);
    if (this != &src)
    {
        mNodeState  = src.mNodeState;
        mPrioStates = src.mPrioStates;
        mNodeName   = std::move(src.mNodeName);
        mShown      = src.mShown;
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

unsigned int ScopeNodeBase::getPriority() const
{
    return mPrioStates;
}

inline bool ScopeNodeBase::_isExactPrio(uint32_t prio) const
{
    switch (prio)
    {
    case static_cast<uint32_t>(areg::LogPriority::PrioDebug):
    case static_cast<uint32_t>(areg::LogPriority::PrioInfo):
    case static_cast<uint32_t>(areg::LogPriority::PrioWarning):
    case static_cast<uint32_t>(areg::LogPriority::PrioError):
    case static_cast<uint32_t>(areg::LogPriority::PrioFatal):
        return true;

    default:
        return false;
    }
}

void ScopeNodeBase::setPriority( uint32_t prio)
{
    if ((hasPrioValid() == false) || hasPrioNotset())
    {
        mPrioStates = prio;
    }
    else if (hasScopeEntries() && (prio != static_cast<uint32_t>(areg::LogPriority::PrioNotset)) && _isExactPrio(prio))
    {
        mPrioStates = prio | static_cast<uint32_t>(areg::LogPriority::PrioScope);
    }
    else if (prio == static_cast<uint32_t>(areg::LogPriority::PrioScope))
    {
        mPrioStates |= prio;
    }
    else
    {
        mPrioStates = prio;
    }
}

void ScopeNodeBase::addPriority( unsigned int prio )
{
    if ((hasPrioValid() == false) || isLeaf())
    {
        ScopeNodeBase::setPriority(prio);
    }
    else if (hasPrioNotset())
    {
        mPrioStates = prio;
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
        mPrioStates = static_cast<uint32_t>(areg::LogPriority::PrioNotset);
    }
}

int ScopeNodeBase::addChildRecursive(const areg::ScopeEntry& scope)
{
    QString name(QString::fromStdString(scope.scopeName.data()));
    return addChildRecursive(name, scope.scopePrio, scope.scopeId);
}

int ScopeNodeBase::addChildRecursive(QString& scopePath, uint32_t prio, uint32_t scopeId)
{
    QStringList scopeNodes;
    return ((splitScopePath(scopePath, scopeNodes) != 0) ? addChildRecursive(scopeNodes, prio, scopeId) : 0);
}

int ScopeNodeBase::addChildRecursive(QStringList& scopeNodes, uint32_t prio, uint32_t scopeId)
{
    ScopeNodeBase* node = addChildNode(scopeNodes, prio);
    if ((node == nullptr) || (node->isValid() == false))
        return 0;

    node->setScopeId(scopeId);
    return (1 + node->addChildRecursive(scopeNodes, prio, scopeId));
}

ScopeNodeBase* ScopeNodeBase::addChildNode(QString& scopePath, uint32_t prio)
{
    QStringList scopeNodes;
    return ((splitScopePath(scopePath, scopeNodes) != 0) ? addChildNode(scopeNodes, prio) : nullptr);
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
    return ((splitScopePath(scopePath, scopeNodes) != 0) ? makeChildNode(scopeNodes, prio) : nullptr);
}

ScopeNodeBase* ScopeNodeBase::makeChildNode(QStringList& nodeNames, uint32_t prio)
{
    Q_ASSERT(nodeNames.isEmpty());
    return nullptr;
}

QString ScopeNodeBase::makePath() const
{
    QString result(mParent != nullptr ? mParent->makePath() : getPathString());

    if (isRoot() == false)
        result += getPathString();

    // Nodes end with '_' so children append their name with the right separator. ScopeLeaf
    // swaps that trailing '_' for '.'.
    if (isNode())
        result += NELusanCommon::SCOPE_SEPRATOR;

    return result;
}

QString ScopeNodeBase::getPathString() const
{
    return mNodeName;
}

ScopeNodeBase* ScopeNodeBase::findChild(const QString& childName) const
{
    return nullptr;
}

ScopeNodeBase* ScopeNodeBase::findChildByPath(const QString& childPath) const
{
    QStringList nameList;
    QString pathCopy(childPath);
    splitScopePath(pathCopy, nameList);
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
    return static_cast<int>(areg::INVALID_INDEX);
}

ScopeNodeBase* ScopeNodeBase::getChildAt(int pos) const
{
    return nullptr;
}

int ScopeNodeBase::getChildCount() const
{
    return 0;
}

int ScopeNodeBase::getChildNodesCount() const
{
    return 0;
}

int ScopeNodeBase::getChildLeafsCount() const
{
    return 0;
}

void ScopeNodeBase::addChildPriorityRecursive(QString& nodePath, uint32_t prio)
{
    QStringList nameList;
    splitScopePath(nodePath, nameList);
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
    QStringList nameList;
    splitScopePath(nodePath, nameList);
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
            child->removeChildPriorityRecursive(pathList, prio);
            removePriority(prio);
        }
    }
    else
    {
        removePriority(prio);
    }
}

bool ScopeNodeBase::hasNodes() const
{
    return false;
}

bool ScopeNodeBase::hasLeafs() const
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

void ScopeNodeBase::refreshPrioritiesRecursive()
{
}

uint8_t ScopeNodeBase::priorityLevel(uint32_t prio)
{
    const uint32_t bits{ prio & static_cast<uint32_t>(areg::LogPriority::PrioValidLogs) };

    if ((bits & static_cast<uint32_t>(areg::LogPriority::PrioDebug)) != 0)
        return 4u;
    else if ((bits & static_cast<uint32_t>(areg::LogPriority::PrioInfo)) != 0)
        return 3u;
    else if ((bits & static_cast<uint32_t>(areg::LogPriority::PrioWarning)) != 0)
        return 2u;
    else if ((bits & (static_cast<uint32_t>(areg::LogPriority::PrioError) | static_cast<uint32_t>(areg::LogPriority::PrioFatal))) != 0)
        return 1u;
    else
        return 0u;
}

ScopeNodeBase::sPrioRollup ScopeNodeBase::priorityRollup() const
{
    const uint8_t level{ ScopeNodeBase::priorityLevel(mPrioStates) };
    const bool lines{ hasScopeEntries() };
    const bool prio { hasPrioValid() && (hasPrioNotset() == false) };
    return ScopeNodeBase::sPrioRollup{ level, level, lines, lines, prio };
}

QList<ScopeNodeBase*> ScopeNodeBase::getNodesWithPriority() const
{
    QList<ScopeNodeBase*> result;
    if (hasPrioValid() && (hasPrioNotset() == false))
        result.push_back(const_cast<ScopeNodeBase *>(this));
    
    return result;
}

int ScopeNodeBase::extractNodesWithPriority(QList<ScopeNodeBase*>& list) const
{
    int result{ 0 };
    if (hasPrioValid() && (hasMultiPrio(static_cast<uint32_t>(areg::LogPriority::PrioScope)) == false))
    {
        list.push_back(const_cast<ScopeNodeBase *>(this));
        result = 1;
    }

    return result;
}

int ScopeNodeBase::extractChildNodesWithPriority(QList<ScopeNodeBase*>& list) const
{
    return ScopeNodeBase::extractNodesWithPriority(list);
}

int ScopeNodeBase::splitScopePath(QString& scopePath, QStringList& nodeNames) const
{
    // '_' separates nodes, '.' separates the last node from its leaf name. Without a '.' the
    // last '_'-delimited token is the leaf, and '__' encodes a literal '_' prefix.

    int dotPos = scopePath.indexOf(NELusanCommon::SCOPE_LEAF_SEPRATOR);
    QString nodePart = (dotPos >= 0) ? scopePath.left(dotPos) : scopePath;
    QString leafName = (dotPos >= 0) ? scopePath.mid(dotPos + 1) : QString();

    // Split the node part by '_', preserving double-underscore sequences as literal '_' prefixes.
    if (nodePart.isEmpty() == false)
    {
        QStringList nodes = nodePart.split(NELusanCommon::SCOPE_SEPRATOR, Qt::KeepEmptyParts);
        QString prefix, postfix;

        for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
        {
            const QString& name = nodes[i];
            if (name.isEmpty())
            {
                if ((i == static_cast<int>(nodes.size()) - 1) && (nodeNames.isEmpty() == false))
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
    }

    // Append the leaf name verbatim (may contain '_' for snake_case leaf names).
    if (leafName.isEmpty() == false)
    {
        nodeNames.push_back(leafName);
    }

    return static_cast<int>(nodeNames.size());
}

QString ScopeNodeBase::getDisplayName() const
{
    return getNodeName();
}

std::vector<ScopeNodeBase*> ScopeNodeBase::extractNodeLeafs() const
{
    return (std::vector<ScopeNodeBase*>());
}

uint32_t ScopeNodeBase::extractNodeLeafs(std::vector<ScopeNodeBase*>& leafs) const
{
    return 0u;
}

void ScopeNodeBase::setScopeId(uint32_t /*scopeId*/)
{
}

uint32_t ScopeNodeBase::getScopeId() const
{
    return areg::LOG_SCOPE_ID_NONE;
}

void ScopeNodeBase::setShownRecursive(bool shown)
{
    mShown = shown;
}

Qt::CheckState ScopeNodeBase::shownState() const
{
    return (mShown ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

int ScopeNodeBase::collectHiddenScopes(QSet<uint32_t> & scopeIds) const
{
    const uint32_t scopeId{ getScopeId() };
    if (mShown || (scopeId == areg::LOG_SCOPE_ID_NONE))
        return 0;

    scopeIds.insert(scopeId);
    return 1;
}
