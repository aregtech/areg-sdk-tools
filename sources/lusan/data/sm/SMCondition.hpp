#ifndef LUSAN_DATA_SM_SMCONDITION_HPP
#define LUSAN_DATA_SM_SMCONDITION_HPP
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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMCondition.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition conditions
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/sm/SMOperation.hpp"

#include <QList>
#include <QString>

class SMConditionEntry;
class SMConditionGroup;

/**
 * \class   SMConditionNode
 * \brief   Abstract common base of a node in a transition's condition tree.
 *          A node is either a leaf (\ref SMConditionEntry, a single test or a
 *          verbatim row) or a branch (\ref SMConditionGroup, an And/Or group of
 *          child nodes). Nodes are owned by pointer with stable addresses (the
 *          same discipline used for the flat condition list before nesting), so
 *          argument parent chains and command target resolution stay valid.
 **/
class SMConditionNode : public DocumentElem
{
public:
    /**
     * \enum    eNodeKind
     * \brief   Discriminates a leaf from a branch without a dynamic_cast.
     **/
    enum class eNodeKind
    {
          Leaf      //!< A single condition row (\ref SMConditionEntry).
        , Group     //!< An And/Or group of child nodes (\ref SMConditionGroup).
    };

protected:
    SMConditionNode(ElementBase* parent = nullptr);
    SMConditionNode(uint32_t id, ElementBase* parent);
    SMConditionNode(const SMConditionNode& src);
    SMConditionNode(SMConditionNode&& src) noexcept;
    SMConditionNode& operator = (const SMConditionNode& other);
    SMConditionNode& operator = (SMConditionNode&& other) noexcept;

public:
    virtual ~SMConditionNode() = default;

    /**
     * \brief   Returns the node kind (leaf or group).
     **/
    virtual eNodeKind getNodeKind() const = 0;

    /**
     * \brief   Deep-copies this node, preserving its concrete type.
     **/
    virtual SMConditionNode* clone() const = 0;

    inline bool isLeaf() const;
    inline bool isGroup() const;
};

/**
 * \class   SMConditionEntry
 * \brief   One condition row (a leaf node): a comparison (`LHS Operator RHS`), a
 *          boolean test of a single operand (no operator), an expression row
 *          (`LHSKind=Expression`, verbatim boolean C++), or a lambda row
 *          (`LHSKind=Lambda`, a verbatim multi-statement boolean body). Operand
 *          kinds reuse SMArgumentEntry::eValueSource (Param/Attribute/Constant/
 *          Condition/Value/Expression/Lambda). A condition-method LHS may carry an
 *          ArgumentList.
 **/
class SMConditionEntry : public SMConditionNode
{
public:
    using eOperandKind = SMArgumentEntry::eValueSource;

    /**
     * \enum    eOperator
     * \brief   The comparison operator. `None` marks a boolean-test or
     *          expression/lambda row that carries no operator.
     **/
    enum class eOperator
    {
          None          //!< No operator (boolean test / verbatim row).
        , Equal
        , NotEqual
        , Less
        , LessEqual
        , Greater
        , GreaterEqual
    };

    static constexpr const char* const  STR_OP_EQUAL        { "Equal"        };
    static constexpr const char* const  STR_OP_NOTEQUAL     { "NotEqual"     };
    static constexpr const char* const  STR_OP_LESS         { "Less"         };
    static constexpr const char* const  STR_OP_LESSEQUAL    { "LessEqual"    };
    static constexpr const char* const  STR_OP_GREATER      { "Greater"      };
    static constexpr const char* const  STR_OP_GREATEREQUAL { "GreaterEqual" };

    static SMConditionEntry::eOperator fromOperatorString(const QString& op);
    static const char* toString(SMConditionEntry::eOperator op);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMConditionEntry(ElementBase* parent = nullptr);
    SMConditionEntry(uint32_t id, ElementBase* parent);
    SMConditionEntry(const SMConditionEntry& src);
    SMConditionEntry(SMConditionEntry&& src) noexcept;

    SMConditionEntry& operator = (const SMConditionEntry& other);
    SMConditionEntry& operator = (SMConditionEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eOperandKind getLhsKind() const;
    inline void setLhsKind(eOperandKind kind);
    inline const QString& getLhs() const;
    inline void setLhs(const QString& lhs);

    inline eOperator getOperator() const;
    inline void setOperator(eOperator op);
    inline bool hasOperator() const;

    inline eOperandKind getRhsKind() const;
    inline void setRhsKind(eOperandKind kind);
    inline const QString& getRhs() const;
    inline void setRhs(const QString& rhs);

    inline bool isNegated() const;
    inline void setNegated(bool negate);

    inline const QString& getExpression() const;
    inline void setExpression(const QString& expression);

    inline const QString& getBody() const;
    inline void setBody(const QString& body);

    inline const QList<SMArgumentEntry>& getArguments() const;
    inline QList<SMArgumentEntry>& getArguments();

    /**
     * \brief   Appends an argument (for a condition-method LHS), allocating its ID.
     **/
    SMArgumentEntry* addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value);

    /**
     * \brief   True if this is an expression row (LHSKind = Expression): a verbatim
     *          single boolean expression stored in the `Expression` CDATA child.
     **/
    inline bool isExpressionRow() const;

    /**
     * \brief   True if this is a lambda row (LHSKind = Lambda): a verbatim
     *          multi-statement boolean body stored in the `Body` CDATA child.
     **/
    inline bool isLambdaRow() const;

    /**
     * \brief   True if this is a verbatim (opaque) row: an expression or lambda row.
     **/
    inline bool isVerbatimRow() const;

    /**
     * \brief   Copies every operand field (kind/ref/operator/negate/arguments/verbatim
     *          body) from \p src into this leaf, keeping this leaf's own ID and parent.
     *          Used by the leaf-set command so a picked or typed leaf replaces the row's
     *          content without changing its identity.
     **/
    void assignContent(const SMConditionEntry& src);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The row's display name -- its LHS reference; required by the container.
     **/
    inline const QString& getName() const;

    eNodeKind getNodeKind() const override;
    SMConditionNode* clone() const override;

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eOperandKind            mLhsKind;       //!< The LHS operand kind.
    QString                 mLhs;           //!< The LHS reference (name/literal), or empty.
    eOperator               mOperator;      //!< The comparison operator (None for tests/verbatim).
    eOperandKind            mRhsKind;       //!< The RHS operand kind (comparison only).
    QString                 mRhs;           //!< The RHS reference (name/literal).
    bool                    mNegate;        //!< Whether the row result is inverted.
    QString                 mExpression;    //!< Verbatim boolean C++ (expression row only).
    QString                 mBody;          //!< Verbatim multi-statement body (lambda row only).
    QList<SMArgumentEntry>  mArguments;     //!< Args for a condition-method LHS.
};

//////////////////////////////////////////////////////////////////////////
// SMConditionGroup class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMConditionGroup
 * \brief   A branch node in the condition tree: an ordered set of child nodes
 *          (leaves and sub-groups, mixed) combined with `And` (default) or `Or`,
 *          with an optional group-level `Negate`. Children are owned by pointer;
 *          the group deep-copies its children on copy and deletes them on
 *          destruction. One combinator per group (mixing `And`/`Or` at one level
 *          requires an explicit sub-group) is a UI-level rule; the data model
 *          simply stores the single Combine value.
 **/
class SMConditionGroup : public SMConditionNode
{
public:
    /**
     * \enum    eCombine
     * \brief   How the group's child nodes combine.
     **/
    enum class eCombine
    {
          And   //!< All children must hold (default).
        , Or    //!< Any child holds.
    };

    static constexpr const char* const  STR_COMBINE_AND { "And" };
    static constexpr const char* const  STR_COMBINE_OR  { "Or"  };

    static SMConditionGroup::eCombine fromCombineString(const QString& combine);
    static const char* toString(SMConditionGroup::eCombine combine);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMConditionGroup(ElementBase* parent = nullptr);
    SMConditionGroup(uint32_t id, ElementBase* parent);
    SMConditionGroup(const SMConditionGroup& src);
    SMConditionGroup(SMConditionGroup&& src) noexcept;
    virtual ~SMConditionGroup();

    SMConditionGroup& operator = (const SMConditionGroup& other);
    SMConditionGroup& operator = (SMConditionGroup&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eCombine getCombine() const;
    inline void setCombine(eCombine combine);

    inline bool isNegated() const;
    inline void setNegated(bool negate);

    inline const QList<SMConditionNode*>& getChildren() const;
    inline QList<SMConditionNode*>& getChildren();
    inline int getCount() const;
    inline bool isEmpty() const;

    /**
     * \brief   Creates a new empty leaf row appended at the end and returns it.
     **/
    SMConditionEntry* addCondition();

    /**
     * \brief   Creates a new empty sub-group appended at the end and returns it.
     **/
    SMConditionGroup* addGroup();

    /**
     * \brief   Appends an already-created node, taking ownership and re-parenting it.
     * \return  The same pointer that was passed in.
     **/
    SMConditionNode* addChild(SMConditionNode* node);

    /**
     * \brief   Detaches and deletes \p node if it is a direct child. Returns true on success.
     **/
    bool removeChild(SMConditionNode* node);

    /**
     * \brief   Deletes and removes all direct children (recursively freeing sub-trees).
     **/
    void removeAll();

    /**
     * \brief   Collects every leaf row in the sub-tree, in document order. Used by
     *          reference collection, rename remap and ID reallocation, which act on
     *          leaves regardless of nesting depth.
     **/
    QList<SMConditionEntry*> collectLeaves() const;

    /**
     * \brief   Collects every descendant sub-group (excluding this node), in
     *          document order.
     **/
    QList<SMConditionGroup*> collectGroups() const;

    /**
     * \brief   Finds the node with the given ID in this sub-tree (this group included),
     *          or nullptr. Commands resolve their target node by ID each redo/undo, so the
     *          tree may be rebuilt between steps without dangling a captured pointer.
     **/
    SMConditionNode* findNode(uint32_t id);

    /**
     * \brief   Finds the group (this one or a descendant) with the given ID, or nullptr.
     **/
    SMConditionGroup* findGroup(uint32_t id);

    /**
     * \brief   Finds the leaf (a descendant \ref SMConditionEntry) with the given ID, or nullptr.
     **/
    SMConditionEntry* findLeaf(uint32_t id);

    /**
     * \brief   The index of \p node among this group's direct children, or -1 if not a
     *          direct child.
     **/
    int indexOfChild(const SMConditionNode* node) const;

    /**
     * \brief   Detaches a direct child WITHOUT deleting it, transferring ownership to the
     *          caller (the capture-on-remove half of an undoable edit). Returns the node,
     *          or nullptr when it is not a direct child.
     **/
    SMConditionNode* detachChild(SMConditionNode* node);

    /**
     * \brief   Inserts \p node at \p index (clamped), re-parenting it to this group and
     *          taking ownership. The undo half of a remove / the re-insert of an add.
     **/
    void insertChild(int index, SMConditionNode* node);

    /**
     * \brief   Swaps the two direct children at \p index1 and \p index2 (document order =
     *          evaluation order); each swap is its own inverse.
     **/
    void swapChildren(int index1, int index2);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    eNodeKind getNodeKind() const override;
    SMConditionNode* clone() const override;

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Writes the group's `Combine`/`Negate` attributes (omitting defaults)
     *          followed by every child element. The caller writes the enclosing
     *          start/end element and any ID attribute.
     **/
    void writeGroupBody(QXmlStreamWriter& xml) const;

    /**
     * \brief   Reads the `Combine`/`Negate` attributes from the current start element,
     *          then reads child `Condition` / `ConditionGroup` elements until the
     *          matching \p endElem end element.
     **/
    void readGroupBody(QXmlStreamReader& xml, QLatin1StringView endElem);

    /**
     * \brief   Deep-copies the children of \p src into this (already empty) group.
     **/
    void cloneChildrenFrom(const SMConditionGroup& src);

    /**
     * \brief   Re-parents every direct child to this group (after a move).
     **/
    void reparentChildren();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    eCombine                    mCombine;   //!< The combine mode.
    bool                        mNegate;    //!< Whether the group result is inverted.
    QList<SMConditionNode*>     mChildren;  //!< The owned child nodes, in document order.
};

//////////////////////////////////////////////////////////////////////////
// SMConditionList class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMConditionList
 * \brief   A transition's `ConditionList`: the root of its condition tree. It is a
 *          root \ref SMConditionGroup persisted under the `<ConditionList>` element
 *          for backward compatibility. A depth-1, non-negated tree serializes as
 *          the legacy flat `<ConditionList><Condition/>...` form (byte-stable with
 *          existing files); a real sub-group or a group-negate serializes with the
 *          added optional `Negate` attribute and nested `<ConditionGroup>` children.
 **/
class SMConditionList : public SMConditionGroup
{
public:
    SMConditionList(ElementBase* parent = nullptr);
    SMConditionList(const SMConditionList& src);
    SMConditionList(SMConditionList&& src) noexcept;
    virtual ~SMConditionList() = default;

    SMConditionList& operator = (const SMConditionList& other);
    SMConditionList& operator = (SMConditionList&& other) noexcept;

    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;
};

//////////////////////////////////////////////////////////////////////////
// SMConditionNode inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMConditionNode::isLeaf() const
{
    return (getNodeKind() == eNodeKind::Leaf);
}

inline bool SMConditionNode::isGroup() const
{
    return (getNodeKind() == eNodeKind::Group);
}

//////////////////////////////////////////////////////////////////////////
// SMConditionEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline SMConditionEntry::eOperandKind SMConditionEntry::getLhsKind() const
{
    return mLhsKind;
}

inline void SMConditionEntry::setLhsKind(eOperandKind kind)
{
    mLhsKind = kind;
}

inline const QString& SMConditionEntry::getLhs() const
{
    return mLhs;
}

inline void SMConditionEntry::setLhs(const QString& lhs)
{
    mLhs = lhs;
}

inline SMConditionEntry::eOperator SMConditionEntry::getOperator() const
{
    return mOperator;
}

inline void SMConditionEntry::setOperator(eOperator op)
{
    mOperator = op;
}

inline bool SMConditionEntry::hasOperator() const
{
    return (mOperator != eOperator::None);
}

inline SMConditionEntry::eOperandKind SMConditionEntry::getRhsKind() const
{
    return mRhsKind;
}

inline void SMConditionEntry::setRhsKind(eOperandKind kind)
{
    mRhsKind = kind;
}

inline const QString& SMConditionEntry::getRhs() const
{
    return mRhs;
}

inline void SMConditionEntry::setRhs(const QString& rhs)
{
    mRhs = rhs;
}

inline bool SMConditionEntry::isNegated() const
{
    return mNegate;
}

inline void SMConditionEntry::setNegated(bool negate)
{
    mNegate = negate;
}

inline const QString& SMConditionEntry::getExpression() const
{
    return mExpression;
}

inline void SMConditionEntry::setExpression(const QString& expression)
{
    mExpression = expression;
}

inline const QString& SMConditionEntry::getBody() const
{
    return mBody;
}

inline void SMConditionEntry::setBody(const QString& body)
{
    mBody = body;
}

inline const QList<SMArgumentEntry>& SMConditionEntry::getArguments() const
{
    return mArguments;
}

inline QList<SMArgumentEntry>& SMConditionEntry::getArguments()
{
    return mArguments;
}

inline bool SMConditionEntry::isExpressionRow() const
{
    return (mLhsKind == eOperandKind::Expression);
}

inline bool SMConditionEntry::isLambdaRow() const
{
    return (mLhsKind == eOperandKind::Lambda);
}

inline bool SMConditionEntry::isVerbatimRow() const
{
    return (isExpressionRow() || isLambdaRow());
}

inline const QString& SMConditionEntry::getName() const
{
    return mLhs;
}

//////////////////////////////////////////////////////////////////////////
// SMConditionGroup inline methods
//////////////////////////////////////////////////////////////////////////

inline SMConditionGroup::eCombine SMConditionGroup::getCombine() const
{
    return mCombine;
}

inline void SMConditionGroup::setCombine(eCombine combine)
{
    mCombine = combine;
}

inline bool SMConditionGroup::isNegated() const
{
    return mNegate;
}

inline void SMConditionGroup::setNegated(bool negate)
{
    mNegate = negate;
}

inline const QList<SMConditionNode*>& SMConditionGroup::getChildren() const
{
    return mChildren;
}

inline QList<SMConditionNode*>& SMConditionGroup::getChildren()
{
    return mChildren;
}

inline int SMConditionGroup::getCount() const
{
    return static_cast<int>(mChildren.size());
}

inline bool SMConditionGroup::isEmpty() const
{
    return mChildren.isEmpty();
}

#endif  // LUSAN_DATA_SM_SMCONDITION_HPP
