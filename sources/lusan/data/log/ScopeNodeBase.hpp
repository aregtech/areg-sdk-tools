#ifndef LUSAN_DATA_LOG_SCOPENODEBASE_HPP
#define LUSAN_DATA_LOG_SCOPENODEBASE_HPP
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
 *  \file        lusan/data/log/ScopeNodeBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope base class.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "areg/base/GEGlobal.h"
#include "areg/logging/NELogging.hpp"

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
    ScopeNodeBase(void);

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
    ScopeNodeBase(ScopeNodeBase::eNode nodeType, const QString& nodeName, unsigned int prio = static_cast<unsigned int>(NELogging::eLogPriority::PrioNotset), ScopeNodeBase* parent = nullptr);

    //!< Copies, moves and destroys the node object.
    ScopeNodeBase(const ScopeNodeBase& src);
    ScopeNodeBase(ScopeNodeBase&& src) noexcept;
    
public:

    virtual ~ScopeNodeBase(void) = default;

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
    inline const QString& getNodeName(void) const;

    /**
     * \brief   Sets node name.
     **/
    inline void setNodeName(const QString newName);

    /**
     * \brief   Returns true if the object is the root. The root does not have a parent
     **/
    inline bool isRoot(void) const;

    /**
     * \brief   Returns true if the object is a node. Nodes have a parent, it may have child leafs and child nodes.
     **/
    inline bool isNode(void) const;

    /**
     * \brief   Returns true if the object is a leaf. Leafs have parent, but cannot have children nodes.
     **/
    inline bool isLeaf(void) const;

    /**
     * \brief   Returns true if the node is valid.
     **/
    inline bool isValid(void) const;

    /**
     * \brief   Returns true if contains a bit indicating `PrioNotset` priority.
     **/
    inline bool hasPrioNotset(void) const;

    /**
     * \brief   Returns true if the logging priority has debug priority bit set.
     **/
    inline bool hasPrioDebug(void) const;

    /**
     * \brief   Returns true if the logging priority has info priority bit set.
     **/
    inline bool hasPrioInfo(void) const;

    /**
     * \brief   Returns true if the logging priority has warning priority bit set.
     **/
    inline bool hasPrioWarning(void) const;

    /**
     * \brief   Returns true if the logging priority has error priority bit set.
     **/
    inline bool hasPrioError(void) const;

    /**
     * \brief   Returns true if the logging priority has fatal error priority bit set.
     **/
    inline bool hasPrioFatal(void) const;

    /**
     * \brief   Returns true if any logging priority bit is set.
     **/
    inline bool hasLogsEneabled(void) const;

    /**
     * \brief   Returns true if the logging scopes priority bit set.
     **/
    inline bool hasLogScopes(void) const;

    /**
     * \brief   Returns true if the node has any valid priority bits set.
     **/
    inline bool hasPrioValid(void) const;

    /**
     * \brief   Returns true if the node has multiple priorities.
     * \param   prioIgnore  The priority bits to ignore.
     **/
    inline bool hasMultiPrio(uint32_t prioIgnore) const;

    /**
     * \brief   Returns the pointer to parent node. The root nodes have no parent.
     **/
    inline ScopeNodeBase* getParent(void) const;

    /**
     * \brief   Sets the pointer to the parent node. All parents except root should have a parent.
     * \param   parent  The pointer to the parent object.
     **/
    inline void setParent(ScopeNodeBase* parent);

    /**
     * \brief   Resets the priority bits of the node.
     **/
    inline void resetPriority(void);

    /**
     * \brief   Updates priority bits of the parent node.
     * \param   prio        The priority bits to set.
     * \param   recursive   If true, the parent node is updated recursively until root node.
     **/
    inline void updateParentPrio(uint32_t prio, bool recursive);

    /**
     * \brief   Returns true if node has children.
     **/
    inline bool hasChildren(void) const;

    /**
     * \brief   Returns the root of the tree, where this node is located.
     *          If the node is root, returns itself.
     **/
    inline ScopeNodeBase* getTreeRoot(void) const;

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
    inline ScopeNodeBase::eNodeState getNodeState(void) const;

    /**
     * \brief   Returns true if the node is expanded.
     **/
    inline bool isNodeExpanded(void) const;
    
    /**
     * \brief   Sets the node and all child nodes tree in the expanded state.
     **/
    inline void setNodeTreeExpanded(void);
    
    /**
     * \brief   Sets the node and all child nodes tree in the collapsed state.
     **/
    inline void setNodeTreeCollapsed(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the node priority flag.
     **/
    virtual unsigned int getPriority(void) const;

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
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
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
    virtual int addChildRecursive(const NELogging::sScopeInfo & scope);

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
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
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
     *          Each child node is separated by '_'. If the path does not contain '_', it is created as a 'leaf'.
     *          If path is empty, returns nullptr.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
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
    virtual QString makePath(void) const;

    /**
     * \brief   Returns the string used to create the path. Root nodes should return empty string.
     **/
    virtual QString getPathString(void) const;

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
    virtual int getChildCount(void) const;

    /**
     * \brief   Returns the total number of child nodes.
     **/
    virtual int getChildNodesCount(void) const;

    /**
     * \brief   Returns the total number of child leafs.
     **/
    virtual int getChildLeafsCount(void) const;

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
    virtual bool hasNodes(void) const;

    /**
     * \brief   Returns true if the current node has leafs.
     **/
    virtual bool hasLeafs(void) const;

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
    virtual void refreshPrioritiesRecursive(void);

    /**
     * \brief   Returns the list of nodes with log priority. The node should not have NotSet priority flag.
     **/
    virtual QList<ScopeNodeBase*> getNodesWithPriority(void) const;

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
     *          The path is separated by '_'. If path contains "__", only one
     *          symbol is removed and the is used as a prefix (or postfix) in the name.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, the `scopePath` is empty.
     * \param   nodeNames   The list names to create nodes.
     * \return  The number of names in the list.
     **/
    virtual int splitScopePath(QString& scopePath, QStringList& nodeNames) const;

    /**
     * \brief   Returns the string to display on screen.
     **/
    virtual QString getDisplayName(void) const;

    /**
     * \brief   Retrieves all relevant leaf nodes, including leafs of the child nodes, under the current node.
     **/
    virtual std::vector<ScopeNodeBase*> extractNodeLeafs(void) const;

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
    virtual uint32_t getScopeId(void) const;

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

inline const QString & ScopeNodeBase::getNodeName( void ) const
{
    return mNodeName;
}

inline void ScopeNodeBase::setNodeName( const QString newName )
{
    mNodeName = newName;
}

inline bool ScopeNodeBase::isRoot( void ) const
{
    return (mNodeType == ScopeNodeBase::eNode::Root);
}

inline bool ScopeNodeBase::isNode( void ) const
{
    return (mNodeType == ScopeNodeBase::eNode::Node);
}

inline bool ScopeNodeBase::isLeaf( void ) const
{
    return (mNodeType == ScopeNodeBase::eNode::Leaf);
}

inline bool ScopeNodeBase::isValid( void ) const
{
    return (mNodeType != ScopeNodeBase::eNode::Invalid);
}

inline bool ScopeNodeBase::hasPrioNotset( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset)) != 0;
}

inline bool ScopeNodeBase::hasPrioDebug(void) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug)) != 0;
}

inline bool ScopeNodeBase::hasPrioInfo( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo)) != 0;
}

inline bool ScopeNodeBase::hasPrioWarning( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning)) != 0;
}

inline bool ScopeNodeBase::hasPrioError( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioError)) != 0;
}

inline bool ScopeNodeBase::hasPrioFatal( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal)) != 0;
}

inline bool ScopeNodeBase::hasLogsEneabled( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioLogs)) != 0;
}

inline bool ScopeNodeBase::hasLogScopes( void ) const
{
    return (mPrioStates & static_cast<uint32_t>(NELogging::eLogPriority::PrioScope)) != 0;
}

inline bool ScopeNodeBase::hasPrioValid( void ) const
{
    return (mPrioStates != static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid));
}

inline bool ScopeNodeBase::hasMultiPrio(uint32_t prioIgnore) const
{
    uint32_t prio = mPrioStates & (~prioIgnore);
    switch (prio)
    {
    case static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid):
        return false;

    case static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug):
    case static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo):
    case static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning):
    case static_cast<uint32_t>(NELogging::eLogPriority::PrioError):
    case static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal):
    case static_cast<uint32_t>(NELogging::eLogPriority::PrioNotset):
        return true;
        
    default:
        return true;
    }
}

inline ScopeNodeBase* ScopeNodeBase::getParent(void) const
{
    return mParent;
}

inline void ScopeNodeBase::setParent(ScopeNodeBase* parent)
{
    mParent = parent;
}

inline void ScopeNodeBase::resetPriority(void)
{
    mPrioStates = static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid);
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

inline bool ScopeNodeBase::hasChildren(void) const
{
    return (getChildCount() != 0);
}

inline ScopeNodeBase* ScopeNodeBase::getTreeRoot(void) const
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
    if (mPrioStates == static_cast<uint32_t>(NELogging::eLogPriority::PrioInvalid))
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

inline ScopeNodeBase::eNodeState ScopeNodeBase::getNodeState(void) const
{
    return mNodeState;
}

inline bool ScopeNodeBase::isNodeExpanded(void) const
{
    return (mNodeState == ScopeNodeBase::eNodeState::NodeExpanded);
}

inline void ScopeNodeBase::setNodeTreeExpanded(void)
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

inline void ScopeNodeBase::setNodeTreeCollapsed(void)
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
