#ifndef LUSAN_DATA_LOG_SCOPENODEBASE_HPP
#define LUSAN_DATA_LOG_SCOPENODEBASE_HPP
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
 *  \file        lusan/data/log/ScopeNodeBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope base class.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "areg/base/areg_global.h"
#include "areg/logging/areg_log.h"

#include <QList>
#include <QString>

/**
 * \brief   ScopeNodeBase is the base class for all scope nodes.
 * \see     ScopeNode, ScopeLeaf, ScopeRoot
 **/
class ScopeNodeBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   The types of the node. Set when object is created and cannot be changed.
     **/
    enum class eNode    : unsigned char
    {
          Invalid   = 0 // The node is invalid,             bits: 0000 0000
        , Leaf      = 2 // The node is root, has no parent, bits: 0000 0010
        , Node      = 4 // The node that has children,      bits: 0000 0100
        , Root      = 12// The node does not have children, bits: 0000 1100
    };

    /**
     * \brief   The state of the node. Set when object is created and cannot be changed.
     **/
    enum class eNodeState
    {
          NodeCollapsed = 0 //!< The node is collapsed, does not show children.
        , NodeExpanded  = 1 //!< The node is expanded, shows children.
    };

//////////////////////////////////////////////////////////////////////////
// Protected constructors and destructor
//////////////////////////////////////////////////////////////////////////
protected:

    //!< The default constructor. Creates an empty node with no priority.
    ScopeNodeBase();

    /**
     * \brief   Creates and empty node with no priority.
     *          The type depends on the passed parameter and cannot be changed later.
     * \param   nodeType    The type of node to set when initialized. Cannot be changed.
     **/
    explicit ScopeNodeBase(ScopeNodeBase::eNode nodeType, ScopeNodeBase * parent = nullptr);

    /**
     * \brief   Creates a node and sets the node type, node name and priority flags.
     *          The type of the node cannot be changed anymore.
     * \param   nodeType    The type of the node. Cannot be changed anymore.
     * \param   nodeName    The name of the node.
     * \param   prio        The logging priority flags set bitwise.
     **/
    ScopeNodeBase(ScopeNodeBase::eNode nodeType, const QString& nodeName, unsigned int prio = static_cast<unsigned int>(areg::LogPriority::PrioNotset), ScopeNodeBase* parent = nullptr);

    //!< Copies, moves and destroys the node object.
    ScopeNodeBase(const ScopeNodeBase& src);
    ScopeNodeBase(ScopeNodeBase&& src) noexcept;
    
public:

    virtual ~ScopeNodeBase() = default;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Assignment operators
     **/
    ScopeNodeBase& operator = (const ScopeNodeBase& src);
    ScopeNodeBase& operator = (ScopeNodeBase&& src) noexcept;

    /**
     * \brief   Compare operators.
     **/
    bool operator == (const ScopeNodeBase& other) const;
    bool operator != (const ScopeNodeBase& other) const;
    bool operator > (const ScopeNodeBase& other) const;
    bool operator < (const ScopeNodeBase& other) const;


//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns node name.
     **/
    inline const QString& getNodeName() const;

    /**
     * \brief   Sets node name.
     **/
    inline void setNodeName(const QString& newName);

    /**
     * \brief   Returns true if the object is the root. The root does not have a parent
     **/
    inline bool isRoot() const;

    /**
     * \brief   Returns true if the object is a node. Nodes have a parent, it may have child leafs and child nodes.
     **/
    inline bool isNode() const;

    /**
     * \brief   Returns true if the object is a leaf. Leafs have parent, but cannot have children nodes.
     **/
    inline bool isLeaf() const;

    /**
     * \brief   Returns true if the node is valid.
     **/
    inline bool isValid() const;

    /**
     * \brief   Returns true if contains a bit indicating `PrioNotset` priority.
     **/
    inline bool hasPrioNotset() const;

    /**
     * \brief   Returns true if the logging priority has debug priority bit set.
     **/
    inline bool hasPrioDebug() const;

    /**
     * \brief   Returns true if the logging priority has info priority bit set.
     **/
    inline bool hasPrioInfo() const;

    /**
     * \brief   Returns true if the logging priority has warning priority bit set.
     **/
    inline bool hasPrioWarning() const;

    /**
     * \brief   Returns true if the logging priority has error priority bit set.
     **/
    inline bool hasPrioError() const;

    /**
     * \brief   Returns true if the logging priority has fatal error priority bit set.
     **/
    inline bool hasPrioFatal() const;

    /**
     * \brief   Returns true if any logging priority bit is set.
     **/
    inline bool hasLogsEneabled() const;

    /**
     * \brief   Returns true if the logging scopes priority bit set.
     **/
    inline bool hasScopeEntries() const;

    /**
     * \brief   Returns true if the node has any valid priority bits set.
     **/
    inline bool hasPrioValid() const;

    /**
     * \brief   Returns true if the node has multiple priorities.
     * \param   prioIgnore  The priority bits to ignore.
     **/
    inline bool hasMultiPrio(uint32_t prioIgnore) const;

    /**
     * \brief   Returns the pointer to parent node. The root nodes have no parent.
     **/
    inline ScopeNodeBase* getParent() const;

    /**
     * \brief   Sets the pointer to the parent node. All parents except root should have a parent.
     * \param   parent  The pointer to the parent object.
     **/
    inline void setParent(ScopeNodeBase* parent);

    /**
     * \brief   Resets the priority bits of the node.
     **/
    inline void resetPriority();

    /**
     * \brief   Updates priority bits of the parent node.
     * \param   prio        The priority bits to set.
     * \param   recursive   If true, the parent node is updated recursively until root node.
     **/
    inline void updateParentPrio(uint32_t prio, bool recursive);

    /**
     * \brief   Returns true if node has children.
     **/
    inline bool hasChildren() const;

    /**
     * \brief   Returns the root of the tree, where this node is located.
     *          If the node is root, returns itself.
     **/
    inline ScopeNodeBase* getTreeRoot() const;

    /**
     * \brief   Checks and returns true if can add the specified prio to the node.
     *          The prio can be added to the node if the node or child nodes do not have the specified prio set.
     **/
    inline bool canAddPriority(unsigned int prio) const;

    /**
     * \brief   Checks and returns true if can remove the specified prio from the node.
     *          The prio can be removed from the node if the node or child nodes have the specified prio set.
     **/
    inline bool canRemovePriority(unsigned int prio) const;

    /**
     * \brief   Sets node state expanded or collapsed.
     * \param   isExpanded  If true, sets the node state to expanded. Otherwise, sets to collapsed.
     **/
    inline void setNodeState(bool isExpanded);

    /**
     * \brief   Returns the node's expanded or collapsed state.
     **/
    inline ScopeNodeBase::eNodeState getNodeState() const;

    /**
     * \brief   Returns true if the node is expanded.
     **/
    inline bool isNodeExpanded() const;
    
    /**
     * \brief   Sets the node and all child nodes tree in the expanded state.
     **/
    inline void setNodeTreeExpanded();
    
    /**
     * \brief   Sets the node and all child nodes tree in the collapsed state.
     **/
    inline void setNodeTreeCollapsed();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the node priority flag.
     **/
    virtual unsigned int getPriority() const;

    /**
     * \brief   Sets the node priority flag.
     **/
    virtual void setPriority(uint32_t prio);

    /**
     * \brief   Adds log priority bits.
     **/
    virtual void addPriority(unsigned int prio);

    /**
     * \brief   Adds log priority bits.
     **/
    virtual void removePriority(unsigned int prio);

    /**
     * \brief   Recursively adds a child a node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   scopePath   The path to the node. Nodes are separated by '_', the leaf by '.'.
     *                      On output, it contains the next level of the path.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \param   scopeId     The scope ID to set for the leaf node.
     * \return  The number of nodes added.
     **/
    virtual int addChildRecursive(QString& scopePath, uint32_t prio, uint32_t scopeId);

    /**
     * \brief   Recursively adds a child a node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   scope       The scope information to add.
     * \return  The number of nodes added.
     **/
    virtual int addChildRecursive(const areg::ScopeEntry & scope);

    /**
     * \brief   Recursively adds a child a node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   nodeNames   The list of node names.
     *                      On output, it removes the first node name from the list.
     *                      The last name should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \param   scopeId     The scope ID to set for the leaf node.
     * \return  The number of nodes added.
     **/
    virtual int addChildRecursive(QStringList& nodeNames, uint32_t prio, uint32_t scopeId);

    /**
     * \brief   Adds a single child node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   scopePath   The path to the node. Nodes are separated by '_', the leaf by '.'.
     *                      On output, it contains the next level of the path.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The pointer to the added child node.
     **/
    virtual ScopeNodeBase* addChildNode(QString& scopePath, uint32_t prio);

    /**
     * \brief   Adds a single child node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   nodeNames   The list of node names.
     *                      On output, it removes the first node name from the list.
     *                      The last name should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The pointer to the added child node.
     **/
    virtual ScopeNodeBase* addChildNode(QStringList& nodeNames, uint32_t prio);

    /**
     * \brief   Adds a child node to the parent if the node does not exist.
     *          Otherwise, it adds in the existing node the log priority of the passed node object.
     * \param   childNode   The child node to add to the parent.
     * \return  The pointer to the added child node.
     **/
    virtual ScopeNodeBase* addChildNode(ScopeNodeBase* childNode);

    /**
     * \brief   Creates a child node. The child node is not added to the parent.
     *          Nodes are separated by '_'; if the path contains '.', the part after
     *          the dot is created as a leaf (verbatim, may contain '_').
     *          If path is empty, returns nullptr.
     * \param   scopePath   The path to the node. Nodes separated by '_', leaf by '.'.
     *                      On output, it contains the next level of the path.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The node object created.
     **/
    virtual ScopeNodeBase* makeChildNode(QString& scopePath, uint32_t prio);

    /**
     * \brief   Creates a child node. The child node is not added to the parent.
     *          Each child node is listed in the `nodeNames`. If the list contains last entry, it is created as a 'leaf'.
     *          If the passed list empty, returns nullptr.
     * \param   nodeNames   The list of node names.
     *                      On output, it removes the first node name from the list.
     *                      The last name should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The node object created.
     **/
    virtual ScopeNodeBase* makeChildNode(QStringList& nodeNames, uint32_t prio);

    /**
     * \brief   Creates and returns the path of the node, where each name of the node is separated by `_`.
     **/
    virtual QString makePath() const;

    /**
     * \brief   Returns the string used to create the path. Root nodes should return empty string.
     **/
    virtual QString getPathString() const;

    /**
     * \brief   Returns child node object that contains the specified name.
     *          Returns nullptr if no child with specified name exists.
     * \param   childName   The name of the child node to find.
     **/
    virtual ScopeNodeBase* findChild(const QString& childName) const;

    /**
     * \brief   Returns child node object that contains the specified path.
     *          Returns nullptr if no child with specified path exists.
     *          It splits the specified path and searcher for the child in the tree.
     * \param   childPath   The path of the child node to find.
     **/
    virtual ScopeNodeBase* findChildByPath(const QString& childPath) const;

    /**
     * \brief   Returns the position of the child node in the list of child nodes.
     *          Returns NECommon::INVALID_INDEX if no child with specified name exists.
     * \param   childName   The name of the child node to find.
     **/
    virtual int getChildPosition(const QString& childName) const;
    
    /**
     * \brief   Returns the child node at the given position.
     * \param   pos     The position of the child.
     * \return  Valid pointer of the child node or leaf, if position is valid. Otherwise, returns nullptr.
     **/
    virtual ScopeNodeBase* getChildAt(int pos) const;

    /**
     * \brief   Returns the total number of children.
     **/
    virtual int getChildCount() const;

    /**
     * \brief   Returns the total number of child nodes.
     **/
    virtual int getChildNodesCount() const;

    /**
     * \brief   Returns the total number of child leafs.
     **/
    virtual int getChildLeafsCount() const;

    /**
     * \brief   Adds the priority recursively to the child nodes.
     *          On returns, the `nodePath` returns empty string if the complete path was updated.
     * \param   nodePath    The path to the node. The path is separated by '_'.
     * \param   prio        The logging priority to set.
     **/
    virtual void addChildPriorityRecursive(QString& nodePath, uint32_t prio);

    /**
     * \brief   Adds the priority recursively to the child nodes.
     *          On returns, the `pathList` returns empty string if the complete path was updated.
     * \param   pathList    The list of node names.
     * \param   prio        The logging priority to set.
     **/
    virtual void addChildPriorityRecursive(QStringList& pathList, uint32_t prio);

    /**
     * \brief   Removes the priority recursively from the child nodes.
     *          On returns, the `nodePath` returns empty string if the complete path was updated.
     * \param   nodePath    The path to the node. The path is separated by '_'.
     * \param   prio        The logging priority to set.
     **/
    virtual void removeChildPriorityRecursive(QString& nodePath, uint32_t prio);

    /**
     * \brief   Removes the priority recursively from the child nodes.
     *          On returns, the `pathList` returns empty string if the complete path was updated.
     * \param   pathList    The list of node names.
     * \param   prio        The logging priority to set.
     **/
    virtual void removeChildPriorityRecursive(QStringList& pathList, uint32_t prio);

    /**
     * \brief   Returns true if the current node has other node objects with children.
     **/
    virtual bool hasNodes() const;

    /**
     * \brief   Returns true if the current node has leafs.
     **/
    virtual bool hasLeafs() const;

    /**
     * \brief   Returns true if the node has a leaf with the specified name.
     **/
    virtual bool containsLeaf(const QString& leafName) const;

    /**
     * \brief   Returns true if the node has a node with the specified name.
     **/
    virtual bool containsNode(const QString& nodeName) const;

    /**
     * \brief   On output, the `children` parameter contains the list of child nodes.
     * \param   children    The list of child nodes to fill.
     * \return  The number of child nodes in the list.
     **/
    virtual int getChildren(std::vector<ScopeNodeBase*> & children) const;

    /**
     * \brief   Resets and invalidates the priorities of the node and all child nodes.
     * \param   skipLeafs   If true, skips resetting the priority of the leafs and resets the priority only nodes.
     *                      Otherwise, it resets the priority of the leafs and nodes.
     **/
    virtual void resetPrioritiesRecursive(bool skipLeafs);

    /**
     * \brief   Refreshes the priorities by keeping the priority of leafs and refreshing the priorities of the nodes.
     **/
    virtual void refreshPrioritiesRecursive();

    /**
     * \brief   Returns the list of nodes with log priority. The node should not have NotSet priority flag.
     **/
    virtual QList<ScopeNodeBase*> getNodesWithPriority() const;

    /**
     * \brief   Extracts nodes with log priority.
     * \param   list    The list to add nodes with priority.
     * \return  Returns number of new added nodes.
     **/
    virtual int extractNodesWithPriority(QList<ScopeNodeBase*>& list) const;

    /**
     * \brief   Extracts child nodes with log priority.
     * \param   list    The list to add nodes with priority.
     * \return  Returns number of new added nodes.
     **/
    virtual int extractChildNodesWithPriority(QList<ScopeNodeBase*>& list) const;
    
    /**
     * \brief   Creates a list of node names from the passed scope path.
     *          Nodes are separated by '_'; a '.' separates the last node from
     *          its leaf name, which is taken verbatim (may contain '_' for snake_case).
     *          If the path contains no '.', the last '_'-delimited token becomes the leaf.
     *          Consecutive underscores ('__') encode a literal '_' in a node name.
     * \param   scopePath   The scope path to split. Not modified.
     * \param   nodeNames   The list of names produced (nodes followed by the leaf name).
     * \return  The number of names in the list.
     **/
    virtual int splitScopePath(QString& scopePath, QStringList& nodeNames) const;

    /**
     * \brief   Returns the string to display on screen.
     **/
    virtual QString getDisplayName() const;

    /**
     * \brief   Retrieves all relevant leaf nodes, including leafs of the child nodes, under the current node.
     **/
    virtual std::vector<ScopeNodeBase*> extractNodeLeafs() const;

    /**
     * \brief   Retrieves all relevant leaf nodes, including leafs of the child nodes, under the current node.
     * \param   leafs    The vector to fill with leaf nodes.
     * \return  The number of leaf nodes found.
     **/
    virtual uint32_t extractNodeLeafs(std::vector<ScopeNodeBase*>& leafs) const;

    /**
     * \brief   Sets the scope ID for the node. Valid only for leafs. Ignored in case of other nodes.
     * \param   scopeId     The scope ID to set.
     **/
    virtual void setScopeId(uint32_t scopeId);

    /**
     * \brief   Returns scope ID of the leaf or 0 in case of other nodes.
     **/
    virtual uint32_t getScopeId() const;

private:
    //!< Return true if the prio is an exact match
    inline bool _isExactPrio(uint32_t prio) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The type of the node. Set when object is created and cannot be changed.
    const ScopeNodeBase::eNode  mNodeType;
    //!< The state of the node.
    ScopeNodeBase::eNodeState   mNodeState;
    //!< The parent of the node. Can be null if the node is root.   
    ScopeNodeBase*              mParent;
    //!< The priority flags set bitwise.
    unsigned int                mPrioStates;
    //!< The name of the node.
    QString                     mNodeName;
};

//////////////////////////////////////////////////////////////////////////
// ScopeNodeBase class inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString & ScopeNodeBase::getNodeName() const
{
    return mNodeName;
}

inline void ScopeNodeBase::setNodeName( const QString& newName )
{
    mNodeName = newName;
}

inline bool ScopeNodeBase::isRoot() const
{
    return (mNodeType == ScopeNodeBase::eNode::Root);
}

inline bool ScopeNodeBase::isNode() const
{
    return (mNodeType == ScopeNodeBase::eNode::Node);
}

inline bool ScopeNodeBase::isLeaf() const
{
    return (mNodeType == ScopeNodeBase::eNode::Leaf);
}

inline bool ScopeNodeBase::isValid() const
{
    return (mNodeType != ScopeNodeBase::eNode::Invalid);
}

inline bool ScopeNodeBase::hasPrioNotset() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioNotset)) != 0;
}

inline bool ScopeNodeBase::hasPrioDebug() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioDebug)) != 0;
}

inline bool ScopeNodeBase::hasPrioInfo() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioInfo)) != 0;
}

inline bool ScopeNodeBase::hasPrioWarning() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioWarning)) != 0;
}

inline bool ScopeNodeBase::hasPrioError() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioError)) != 0;
}

inline bool ScopeNodeBase::hasPrioFatal() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioFatal)) != 0;
}

inline bool ScopeNodeBase::hasLogsEneabled() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioLogs)) != 0;
}

inline bool ScopeNodeBase::hasScopeEntries() const
{
    return (mPrioStates & static_cast<uint32_t>(areg::LogPriority::PrioScope)) != 0;
}

inline bool ScopeNodeBase::hasPrioValid() const
{
    return (mPrioStates != static_cast<uint32_t>(areg::LogPriority::PrioInvalid));
}

inline bool ScopeNodeBase::hasMultiPrio(uint32_t prioIgnore) const
{
    uint32_t prio = mPrioStates & (~prioIgnore);
    switch (prio)
    {
    case static_cast<uint32_t>(areg::LogPriority::PrioInvalid):
        return false;

    case static_cast<uint32_t>(areg::LogPriority::PrioDebug):
    case static_cast<uint32_t>(areg::LogPriority::PrioInfo):
    case static_cast<uint32_t>(areg::LogPriority::PrioWarning):
    case static_cast<uint32_t>(areg::LogPriority::PrioError):
    case static_cast<uint32_t>(areg::LogPriority::PrioFatal):
    case static_cast<uint32_t>(areg::LogPriority::PrioNotset):
        return true;
        
    default:
        return true;
    }
}

inline ScopeNodeBase* ScopeNodeBase::getParent() const
{
    return mParent;
}

inline void ScopeNodeBase::setParent(ScopeNodeBase* parent)
{
    mParent = parent;
}

inline void ScopeNodeBase::resetPriority()
{
    mPrioStates = static_cast<uint32_t>(areg::LogPriority::PrioInvalid);
}

inline void ScopeNodeBase::updateParentPrio(uint32_t prio, bool recursive)
{
    if (mParent != nullptr)
    {
        mParent->mPrioStates |= prio;
        if (recursive)
        {
            mParent->updateParentPrio(prio, recursive);
        }
    }
}

inline bool ScopeNodeBase::hasChildren() const
{
    return (getChildCount() != 0);
}

inline ScopeNodeBase* ScopeNodeBase::getTreeRoot() const
{
    if (isRoot())
    {
        Q_ASSERT(mParent == nullptr);
        return const_cast<ScopeNodeBase*>(this);
    }
    else if (mParent != nullptr)
    {
        return mParent->getTreeRoot();
    }
    else
    {
        Q_ASSERT(false && "ScopeNodeBase::getTreeRoot() called on invalid node without parent.");
        return nullptr;
    }
}

inline bool ScopeNodeBase::canAddPriority(unsigned int prio) const
{
    if (hasPrioValid() == false)
    {
        return true;
    }
    else if ((mPrioStates & prio) == 0)
    {
        return true;
    }
    else if (hasMultiPrio(prio))
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool ScopeNodeBase::canRemovePriority(unsigned int prio) const
{
    if (mPrioStates == static_cast<uint32_t>(areg::LogPriority::PrioInvalid))
    {
        return false;
    }
    else if ((mPrioStates & prio) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

inline void ScopeNodeBase::setNodeState(bool isExpanded)
{
    if (isExpanded)
    {
        mNodeState = ScopeNodeBase::eNodeState::NodeExpanded;
        Q_ASSERT((mParent == nullptr) || mParent->isNodeExpanded());
    }
    else
    {
        mNodeState = ScopeNodeBase::eNodeState::NodeCollapsed;
        std::vector<ScopeNodeBase*> children;
        if (getChildren(children) > 0)
        {
            for (ScopeNodeBase* child : children)
            {
                Q_ASSERT(child != nullptr);
                child->setNodeState(false);
            }
        }
    }
}

inline ScopeNodeBase::eNodeState ScopeNodeBase::getNodeState() const
{
    return mNodeState;
}

inline bool ScopeNodeBase::isNodeExpanded() const
{
    return (mNodeState == ScopeNodeBase::eNodeState::NodeExpanded);
}

inline void ScopeNodeBase::setNodeTreeExpanded()
{
    mNodeState = ScopeNodeBase::eNodeState::NodeExpanded;
    std::vector<ScopeNodeBase*> children;
    if (getChildren(children) > 0)
    {
        for (ScopeNodeBase* child : children)
        {
            Q_ASSERT(child != nullptr);
            child->setNodeTreeExpanded();
        }
    }
}

inline void ScopeNodeBase::setNodeTreeCollapsed()
{
    mNodeState = ScopeNodeBase::eNodeState::NodeCollapsed;
    std::vector<ScopeNodeBase*> children;
    if (getChildren(children) > 0)
    {
        for (ScopeNodeBase* child : children)
        {
            Q_ASSERT(child != nullptr);
            child->setNodeTreeCollapsed();
        }
    }
}

#endif  // LUSAN_DATA_LOG_SCOPENODEBASE_HPP
