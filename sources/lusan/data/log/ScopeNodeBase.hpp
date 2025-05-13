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

    inline ScopeNodeBase* getParent(void) const;

    inline void setParent(ScopeNodeBase* parent);

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
    virtual void setPriority(unsigned int prio);

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
     * \return  The number of nodes added.
     **/
    virtual int addChildRecursive(QString& scopePath, uint32_t prio);

    /**
     * \brief   Recursively adds a child a node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   nodeNames   The list of node names.
     *                      On output, it removes the first node name from the list.
     *                      The last name should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The number of nodes added.
     **/
    virtual int addChildRecursive(QStringList& nodeNames, uint32_t prio);

    /**
     * \brief   Adds a single child node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, it contains the next level of the path separated by '_'.
     *                      The last node should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The number of nodes added.
     **/
    virtual ScopeNodeBase* addChildNode(QString& scopePath, uint32_t prio);

    /**
     * \brief   Adds a single child node if it does not exist.
     *          Otherwise, adds a node log priority value.
     * \param   nodeNames   The list of node names.
     *                      On output, it removes the first node name from the list.
     *                      The last name should be marked as 'leaf'.
     * \param   prio        The logging priority to set.
     * \return  The number of nodes added.
     **/
    virtual ScopeNodeBase* addChildNode(QStringList& nodeNames, uint32_t prio);

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
     * \brief   Adds a child node to the parent if the node does not exist.
     *          Otherwise, it adds in the existing node the log priority of the passed node object.
     * \param   childNode   The child node to add to the parent.
     **/
    virtual void addChildNode(ScopeNodeBase* childNode);

    /**
     * \brief   Creates a list of node names from the passed scope path.
     *          The path is separated by '_'.
     * \param   scopePath   The path to the node. The path is separated by '_'.
     *                      On output, the `scopePath` is empty.
     * \return  The list of node names.
     **/
    virtual QStringList makeNodeNames(const QString& scopePath) const;

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
     **/
    virtual void resetPrioritiesRecursive(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The type of the node. Cannot be changed.
    const ScopeNodeBase::eNode  mNodeType;
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

inline bool ScopeNodeBase::hasPrioDebug( void ) const
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

inline ScopeNodeBase* ScopeNodeBase::getParent(void) const
{
    return mParent;
}

void ScopeNodeBase::setParent(ScopeNodeBase* parent)
{
    mParent = parent;
}

#endif  // LUSAN_DATA_LOG_SCOPENODEBASE_HPP
