#ifndef LUSAN_DATA_LOG_SCOPENODES_HPP
#define LUSAN_DATA_LOG_SCOPENODES_HPP
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
 *  \file        lusan/data/log/ScopeNodes.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope node classes.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/ScopeNodeBase.hpp"
#include "areg/base/SortedLinkedList.hpp"
#include "areg/component/ServiceDefs.hpp"

#include <map>

/************************************************************************
 * Declared classes
 ************************************************************************/
class ScopeLeaf;
class ScopeNode;
class ScopeRoot;

//////////////////////////////////////////////////////////////////////////
// ScopeLeaf class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The Leaf node, which has a parent, but does not have children nodes.
 **/
class ScopeLeaf : public ScopeNodeBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Crates an empty leaf with no log priority. Sets the parent object.
     * \param   parent      The parent object to set.
     **/
    ScopeLeaf( ScopeNode * parent = nullptr );

    /**
     * \brief   Creates a leaf object and sets initial parameters.
     * \param   leafName    The name of the leaf.
     * \param   prio        The log priority to set.
     * \param   parent      The pointer to the parent node object.
     **/
    ScopeLeaf(const QString leafName, uint32_t prio = static_cast<unsigned int>(areg::LogPriority::PrioNotset), ScopeNode* parent = nullptr);

    /**
     * \brief   Copies data from the ScopeNodeBase object. Should be called explicit.
     **/
    explicit ScopeLeaf( const ScopeNodeBase & base );

    /**
     * \brief   Copies or moves data from the given source
     **/
    ScopeLeaf( const ScopeLeaf & src );    
    ScopeLeaf( ScopeLeaf && src ) noexcept;

    virtual ~ScopeLeaf() = default;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Assigns data from the given source. Default implementation
     **/
    ScopeLeaf & operator = ( const ScopeLeaf & src ) = default;
    ScopeLeaf & operator = ( ScopeLeaf && src ) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
/************************************************************************
 * ScopeNodeBase override
 ************************************************************************/

    /**
     * \brief   Adds log priority bits.
     **/
    void addPriority(unsigned int prio) override;

    /**
     * \brief   Builds the full scope path for this leaf.
     *          The parent node path ends with '_'; this override replaces that
     *          trailing '_' with '.' so the path matches the new areg syntax:
     *          e.g. "areg_path_node.leaf_name".
     **/
    QString makePath() const override;

    /**
     * \brief   Sets the scope ID for the node. Valid only for leafs. Ignored in case of other nodes.
     * \param   scopeId     The scope ID to set.
     **/
    void setScopeId(uint32_t scopeId) override;

    /**
     * \brief   Returns scope ID of the leaf or 0 in case of other nodes.
     **/
    uint32_t getScopeId() const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    uint32_t    mScopeId;  //!< The scope ID of the leaf node.
};

//////////////////////////////////////////////////////////////////////////
// ScopeNode class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The scope node, which has parent, leafs and child nodes.
 **/
class ScopeNode : public ScopeNodeBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    //!< The list of the nodes.
    using NodeList = std::map<QString, ScopeNode *>;
    //!< The list of leafs.
    using LeafList = std::map<QString, ScopeLeaf *>;

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Crates an empty node with no log priority. Sets the parent object.
     * \param   parent      The parent object to set.
     **/
    ScopeNode( ScopeNode * parent = nullptr );

    /**
     * \brief   Creates a node object and sets initial parameters.
     * \param   nodeName    The name of the node.
     * \param   prio        The log priority to set.
     * \param   parent      The pointer to the parent node object.
     **/
    ScopeNode(const QString nodeName, uint32_t prio = static_cast<unsigned int>(areg::LogPriority::PrioNotset), ScopeNode* parent = nullptr);

    /**
     * \brief   Creates a node with empty list of child leafs and nodes
     *          and copies data from the base object.
     **/
    explicit ScopeNode( const ScopeNodeBase & base );

    /**
     * \brief   Copies or moves data from the given source
     **/
    ScopeNode( const ScopeNode & src );    
    ScopeNode( ScopeNode && src ) noexcept;
    
    virtual ~ScopeNode();

//////////////////////////////////////////////////////////////////////////
// Protected constructor
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Protected constructors required by root node.
     **/
    ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, unsigned int prio, ScopeRoot * parent = nullptr );
    ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, ScopeRoot * parent = nullptr );    
    ScopeNode( ScopeNodeBase::eNode nodeType, ScopeRoot * parent = nullptr );
    
//////////////////////////////////////////////////////////////////////////
// Assigning operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copies data from the given source.
     **/
    ScopeNode & operator = ( const ScopeNode & src );
    ScopeNode & operator = ( ScopeNode && src ) noexcept;

//////////////////////////////////////////////////////////////////////////
// Override
//////////////////////////////////////////////////////////////////////////
public:

/************************************************************************
 * ScopeNodeBase override
 ************************************************************************/

    /**
     * \brief   Sets the node priority flag.
     **/
    void setPriority(uint32_t prio) override;

    /**
     * \brief   Adds log priority bits.
     **/
    void addPriority(unsigned int prio) override;

    /**
     * \brief   Adds log priority bits.
     **/
    void removePriority(unsigned int prio) override;

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
    ScopeNodeBase* makeChildNode(QStringList& nodeNames, uint32_t prio) override;

    /**
     * \brief   Adds a child node to the parent if the node does not exist.
     *          Otherwise, it adds in the existing node the log priority of the passed node object.
     * \param   childNode   The child node to add to the parent.
     * \return  The pointer to the added child node.
     **/
    ScopeNodeBase* addChildNode(ScopeNodeBase* childNode) override;

    /**
     * \brief   Returns child node object that contains the specified name.
     *          Returns nullptr if no child with specified name exists.
     * \param   childName   The name of the child node to find.
     **/
    ScopeNodeBase* findChild(const QString& childName) const override;

    /**
     * \brief   Returns the position of the child node in the list of child nodes.
     *          Returns NECommon::INVALID_INDEX if no child with specified name exists.
     * \param   childName   The name of the child node to find.
     **/
    int getChildPosition(const QString& childName) const override;
    
    /**
     * \brief   Returns the child node at the given position.
     * \param   pos     The position of the child.
     * \return  Valid pointer of the child node or leaf, if position is valid. Otherwise, returns nullptr.
     **/
    ScopeNodeBase* getChildAt(int pos) const override;
    
    /**
     * \brief   Returns the total number of children.
     **/
    int getChildCount() const override;

    /**
     * \brief   Returns the total number of child nodes.
     **/
    int getChildNodesCount() const override;

    /**
     * \brief   Returns the total number of child leafs.
     **/
    int getChildLeafsCount() const override;

    /**
     * \brief   Returns true if the current node has other node objects with children.
     **/
    bool hasNodes() const override;

    /**
     * \brief   Returns true if the current node has leafs.
     **/
    bool hasLeafs() const override;

    /**
     * \brief   Returns true if the node has a leaf with the specified name.
     **/
    bool containsLeaf(const QString& leafName) const override;

    /**
     * \brief   Returns true if the node has a node with the specified name.
     **/
    bool containsNode(const QString& nodeName) const override;

    /**
     * \brief   On output, the `children` parameter contains the list of child nodes.
     * \param   children    The list of child nodes to fill.
     * \return  The number of child nodes in the list.
     **/
    int getChildren(std::vector<ScopeNodeBase*>& children) const override;
    
    /**
     * \brief   Resets and invalidates the priorities of the node and all child nodes.
     * \param   skipLeafs   If true, skips resetting the priority of the leafs and resets the priority only nodes.
     *                      Otherwise, it resets the priority of the leafs and nodes.
     **/
    void resetPrioritiesRecursive(bool skipLeafs) override;

    /**
     * \brief   Refreshes the priorities by keeping the priority of leafs and refreshing the priorities of the nodes.
     **/
    void refreshPrioritiesRecursive() override;

    /**
     * \brief   Returns the list of nodes with log priority. The node should not have NotSet priority flag.
     **/
    QList<ScopeNodeBase*> getNodesWithPriority() const override;

    /**
     * \brief   Extracts nodes with log priority. The node should not have NotSet priority flag.
     * \param   list    The list to add nodes with priority.
     * \return  Returns number of new added nodes.
     **/
    int extractNodesWithPriority(QList<ScopeNodeBase*>& list) const override;

    /**
     * \brief   Extracts child nodes with log priority.
     * \param   list    The list to add nodes with priority.
     * \return  Returns number of new added nodes.
     **/
    int extractChildNodesWithPriority(QList<ScopeNodeBase*>& list) const override;

    /**
     * \brief   Retrieves all relevant leaf nodes, including leafs of the child nodes, under the current node.
     **/
    std::vector<ScopeNodeBase*> extractNodeLeafs() const override;

    /**
     * \brief   Retrieves all relevant leaf nodes, including leafs of the child nodes, under the current node.
     * \param   leafs    The vector to fill with leaf nodes.
     * \return  The number of leaf nodes found.
     **/
    uint32_t extractNodeLeafs(std::vector<ScopeNodeBase*>& leafs) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list of child nodes.
     **/
    inline const ScopeNode::NodeList & getNodes() const;

    /**
     * \brief   Returns the list of child leafs.
     **/
    inline const ScopeNode::LeafList & getLeafs() const;

    /**
     * \brief   Returns the total number of children.
     **/
    inline unsigned int childNodeCount() const;

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Resets the priority of the nodes.
     **/
    inline void resetPrioNodes();

    /**
     * \brief   Resets the priority of the leafs.
     **/
    inline void resetPrioLeafs();

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The list of child nodes.
    NodeList    mChildNodes;
    //!< The list of child leafs.
    LeafList    mChildLeafs;
};

//////////////////////////////////////////////////////////////////////////
// ScopeRoot class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The root node, which is a top level of node, has no parent, leafs and child nodes.
 **/
class ScopeRoot : public ScopeNode
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Default constructor. Creates an empty root node.
     **/
    ScopeRoot();

    /**
     * \brief   Creates a root node and sets the root ID, which is unique within the system.
     * \param   rootId  The ID of the root node.
     **/
    explicit ScopeRoot( ITEM_ID rootId );

    /**
     * \brief   Creates a root node and sets the unique root ID and root name.
     * \param   instance    The structure of connected instance, which contains root ID and name.
     **/
    explicit ScopeRoot(const areg::ConnectedInstance& instance);

    /**
     * \brief   Create a root node with the given name and ID.
     * \param   rootId      The ID of the root to set.
     * @param   rootName    The name of root node to set.
     */
    ScopeRoot(ITEM_ID rootId, const QString rootName );

    /**
     * \brief   Copies or moves data from the given source
     **/
    ScopeRoot( const ScopeRoot & src );
    ScopeRoot( ScopeRoot && src ) noexcept;
    
    virtual ~ScopeRoot() = default;

//////////////////////////////////////////////////////////////////////////
// Assigning operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copies data from the given source.
     **/
    ScopeRoot & operator = ( const ScopeRoot & src );
    ScopeRoot & operator = ( ScopeRoot && src ) noexcept;

//////////////////////////////////////////////////////////////////////////
// Override
//////////////////////////////////////////////////////////////////////////
public:

/************************************************************************
 * ScopeNodeBase override
 ************************************************************************/
    
    /**
     * \brief   Returns the string used to create the path. Root nodes should return empty string.
     **/
    QString getPathString() const override;
    
    /**
     * \brief   Returns the string to display on screen.
     **/
    QString getDisplayName() const override;
    
//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the ID of the root.
     **/
    inline ITEM_ID getRootId() const;

    /**
     * \brief   Sets root ID
     **/
    inline void setRootId(const ITEM_ID rootId);

    /**
     * \brief   Returns the root node name.
     **/
    inline const QString & getRootName() const;

    /**
     * \brief   Sets root node name
     * \param   newRoot     New name of the root to set.
     **/
    inline void setRootName(const QString& newRoot);

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
private:
    //!< The ID of the root.
    ITEM_ID     mRootId;
};

//////////////////////////////////////////////////////////////////////////
// ScopeNode class inline methods
//////////////////////////////////////////////////////////////////////////

inline const ScopeNode::NodeList& ScopeNode::getNodes() const
{
    return mChildNodes;
}

inline const ScopeNode::LeafList& ScopeNode::getLeafs() const
{
    return mChildLeafs;
}

inline unsigned int ScopeNode::childNodeCount() const
{
    return (mChildLeafs.size() + mChildNodes.size());
}

inline void ScopeNode::resetPrioNodes()
{
    resetPriority();
    for (const auto& node : mChildNodes)
    {
        node.second->resetPrioNodes();
    }
}

inline void ScopeNode::resetPrioLeafs()
{
    resetPriority();
    for (const auto& leaf : mChildLeafs)
    {
        leaf.second->resetPriority();
    }
}

//////////////////////////////////////////////////////////////////////////
// ScopeRoot class inline methods
//////////////////////////////////////////////////////////////////////////

inline ITEM_ID ScopeRoot::getRootId() const
{
    return mRootId;
}

inline void ScopeRoot::setRootId(const ITEM_ID rootId)
{
    mRootId = rootId;
}

inline const QString& ScopeRoot::getRootName() const
{
    return mNodeName;
}

inline void ScopeRoot::setRootName(const QString& newRoot)
{
    mNodeName = newRoot;
}

#endif  // LUSAN_DATA_LOG_SCOPENODES_HPP
