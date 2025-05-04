#ifndef LUSAN_DATA_LOG_SCOPENODES_HPP
#define LUSAN_DATA_LOG_SCOPENODES_HPP
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
 *  \file        lusan/data/log/ScopeNodes.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Scope node classes.
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/ScopeNodeBase.hpp"
#include "areg/base/TESortedLinkedList.hpp"

#include <QMap>

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
    ScopeLeaf(const QString leafName, uint32_t prio = static_cast<unsigned int>(NELogging::eLogPriority::PrioNotset), ScopeNode* parent = nullptr);

    /**
     * \brief   Copies data from the ScopeNodeBase object. Should be called explicit.
     **/
    explicit ScopeLeaf( const ScopeNodeBase & base );

    /**
     * \brief   Copies or moves data from the given source
     **/
    ScopeLeaf( const ScopeLeaf & src );    
    ScopeLeaf( ScopeLeaf && src ) noexcept;

    virtual ~ScopeLeaf( void ) = default;

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
     * \brief   Creates a child node. The child node is not added to the parent.
     *          Each child node is separated by '_'. If the path does not contain '_', it is created as a 'leaf'.
     *          If path is empty, returns nullptr.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The node object created.
     **/
    virtual ScopeNodeBase* makeChildNode(QString& scopePath, uint32_t prio) override;

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
    virtual ScopeNodeBase* makeChildNode(QStringList& nodeNames, uint32_t prio) override;

    /**
     * \brief   Adds a child node to the parent if the node does not exist.
     *          Otherwise, it adds in the existing node the log priority of the passed node object.
     * \param   childNode   The child node to add to the parent.
     **/
    virtual void addChildNode(ScopeNodeBase* childNode) override;
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
    using NodeList = QMap<QString, ScopeNode *>;
    //!< The list of leafs.
    using LeafList = QMap<QString, ScopeLeaf *>;

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
    ScopeNode(const QString nodeName, uint32_t prio = static_cast<unsigned int>(NELogging::eLogPriority::PrioNotset), ScopeNode* parent = nullptr);

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
    
    virtual ~ScopeNode( void );

//////////////////////////////////////////////////////////////////////////
// Protected constructor
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Protected constructor required by root node.
     **/
    ScopeNode( ScopeNodeBase::eNode nodeType, const QString & name, unsigned int prio );

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
     * \brief   Creates a child node. The child node is not added to the parent.
     *          Each child node is separated by '_'. If the path does not contain '_', it is created as a 'leaf'.
     *          If path is empty, returns nullptr.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The node object created.
     **/
    virtual ScopeNodeBase* makeChildNode(QString& scopePath, uint32_t prio) override;

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
    virtual ScopeNodeBase* makeChildNode(QStringList& nodeNames, uint32_t prio) override;

    /**
     * \brief   Adds a child node to the parent if the node does not exist.
     *          Otherwise, it adds in the existing node the log priority of the passed node object.
     * \param   childNode   The child node to add to the parent.
     **/
    virtual void addChildNode(ScopeNodeBase* childNode) override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list of child nodes.
     **/
    inline const ScopeNode::NodeList & getNodes( void ) const;

    /**
     * \brief   Returns the list of child leafs.
     **/
    inline const ScopeNode::LeafList & getLeafs( void ) const;

    /**
     * \brief   Returns the total number of children.
     **/
    inline unsigned int childNodeCount( void ) const;

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
    ScopeRoot(void);

    /**
     * \brief   Creates a root node and sets the root ID, which is unique within the system.
     * \param   rootId  The ID of the root node.
     **/
    explicit ScopeRoot( ITEM_ID rootId );

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
    
    virtual ~ScopeRoot( void ) = default;

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
     * \brief   Creates a child node. The child node is not added to the parent.
     *          Each child node is separated by '_'. If the path does not contain '_', it is created as a 'leaf'.
     *          If path is empty, returns nullptr.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The node object created.
     **/
    virtual int addChildRecursive(QString& scopePath, uint32_t prio) override;
    
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
    virtual int addChildRecursive(QStringList& nodeNames, uint32_t prio) override;
    
    /**
     * \brief   Returns the string used to create the path. Root nodes should return empty string.
     **/
    virtual QString getPathString(void) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the ID of the root.
     **/
    inline ITEM_ID getNodeId( void ) const;

    /**
     * \brief   Sets root ID
     **/
    inline void setNodeId(const ITEM_ID rootId);

    /**
     * \brief   Returns the root node name.
     **/
    inline const QString & getRootName(void) const;

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

inline const ScopeNode::NodeList& ScopeNode::getNodes(void) const
{
    return mChildNodes;
}

inline const ScopeNode::LeafList& ScopeNode::getLeafs(void) const
{
    return mChildLeafs;
}

inline unsigned int ScopeNode::childNodeCount(void) const
{
    return (mChildLeafs.size() + mChildNodes.size());
}

//////////////////////////////////////////////////////////////////////////
// ScopeRoot class inline methods
//////////////////////////////////////////////////////////////////////////

inline ITEM_ID ScopeRoot::getNodeId(void) const
{
    return mRootId;
}

inline void ScopeRoot::setNodeId(const ITEM_ID rootId)
{
    mRootId = rootId;
}

inline const QString& ScopeRoot::getRootName(void) const
{
    return mNodeName;
}

inline void ScopeRoot::setRootName(const QString& newRoot)
{
    mNodeName = newRoot;
}

#endif  // LUSAN_DATA_LOG_SCOPENODES_HPP
