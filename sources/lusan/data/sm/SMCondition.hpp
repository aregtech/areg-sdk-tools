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
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMCondition.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition conditions (spec 6.7).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/sm/SMOperation.hpp"

#include <QList>
#include <QString>

/**
 * \class   SMConditionEntry
 * \brief   One condition row: a comparison (`LHS Operator RHS`), a boolean
 *          test of a single operand (no operator), or an expression row
 *          (`LHSKind=Expression`, verbatim boolean C++). Operand kinds reuse
 *          SMArgumentEntry::eValueSource (Param/Attribute/Constant/Condition/Value/
 *          Expression). A condition-method LHS may carry an ArgumentList.
 **/
class SMConditionEntry : public DocumentElem
{
public:
    using eOperandKind = SMArgumentEntry::eValueSource;

    /**
     * \enum    eOperator
     * \brief   The comparison operator (spec 6.7). `None` marks a boolean-test or
     *          expression row that carries no operator.
     **/
    enum class eOperator
    {
          None          //!< No operator (boolean test / expression row).
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
    inline eOperandKind getLhsKind(void) const;
    inline void setLhsKind(eOperandKind kind);
    inline const QString& getLhs(void) const;
    inline void setLhs(const QString& lhs);

    inline eOperator getOperator(void) const;
    inline void setOperator(eOperator op);
    inline bool hasOperator(void) const;

    inline eOperandKind getRhsKind(void) const;
    inline void setRhsKind(eOperandKind kind);
    inline const QString& getRhs(void) const;
    inline void setRhs(const QString& rhs);

    inline bool isNegated(void) const;
    inline void setNegated(bool negate);

    inline const QString& getExpression(void) const;
    inline void setExpression(const QString& expression);

    inline const QList<SMArgumentEntry>& getArguments(void) const;
    inline QList<SMArgumentEntry>& getArguments(void);

    /**
     * \brief   Appends an argument (for a condition-method LHS), allocating its ID.
     **/
    SMArgumentEntry* addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value);

    /**
     * \brief   True if this is an expression row (LHSKind = Expression).
     **/
    inline bool isExpressionRow(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The row's display name — its LHS reference; required by the container.
     **/
    inline const QString& getName(void) const;

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eOperandKind            mLhsKind;       //!< The LHS operand kind.
    QString                 mLhs;           //!< The LHS reference (name/literal), or empty.
    eOperator               mOperator;      //!< The comparison operator (None for tests/expr).
    eOperandKind            mRhsKind;       //!< The RHS operand kind (comparison only).
    QString                 mRhs;           //!< The RHS reference (name/literal).
    bool                    mNegate;        //!< Whether the row result is inverted.
    QString                 mExpression;    //!< Verbatim boolean C++ (expression row only).
    QList<SMArgumentEntry>  mArguments;     //!< Args for a condition-method LHS.
};

//////////////////////////////////////////////////////////////////////////
// SMConditionList class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMConditionList
 * \brief   A transition's `ConditionList`: an ordered set of condition rows
 *          combined with `And` (default) or `Or`, evaluated in document order. Rows are
 *          owned by pointer (stable addresses for their argument parent chains); the
 *          list deep-copies its rows on copy and deletes them on destruction.
 **/
class SMConditionList : public TEDataContainer<SMConditionEntry*, DocumentElem>
{
public:
    /**
     * \enum    eCombine
     * \brief   How the condition rows combine.
     **/
    enum class eCombine
    {
          And   //!< All rows must hold (default).
        , Or    //!< Any row holds.
    };

    static constexpr const char* const  STR_COMBINE_AND { "And" };
    static constexpr const char* const  STR_COMBINE_OR  { "Or"  };

    static SMConditionList::eCombine fromCombineString(const QString& combine);
    static const char* toString(SMConditionList::eCombine combine);

public:
    SMConditionList(ElementBase* parent = nullptr);
    SMConditionList(const SMConditionList& src);
    SMConditionList(SMConditionList&& src) noexcept;
    virtual ~SMConditionList(void);

    SMConditionList& operator = (const SMConditionList& other);
    SMConditionList& operator = (SMConditionList&& other) noexcept;

    inline eCombine getCombine(void) const;
    inline void setCombine(eCombine combine);

    /**
     * \brief   Creates a new empty condition row appended at the end.
     **/
    SMConditionEntry* addCondition(void);

    /**
     * \brief   Deletes and removes all condition rows.
     **/
    void removeAll(void);

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

private:
    //!< Deep-copies the rows of \p src into this (empty) list, re-parenting them.
    void cloneFrom(const SMConditionList& src);

    eCombine    mCombine;   //!< The combine mode.
};

//////////////////////////////////////////////////////////////////////////
// SMConditionEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline SMConditionEntry::eOperandKind SMConditionEntry::getLhsKind(void) const
{
    return mLhsKind;
}

inline void SMConditionEntry::setLhsKind(eOperandKind kind)
{
    mLhsKind = kind;
}

inline const QString& SMConditionEntry::getLhs(void) const
{
    return mLhs;
}

inline void SMConditionEntry::setLhs(const QString& lhs)
{
    mLhs = lhs;
}

inline SMConditionEntry::eOperator SMConditionEntry::getOperator(void) const
{
    return mOperator;
}

inline void SMConditionEntry::setOperator(eOperator op)
{
    mOperator = op;
}

inline bool SMConditionEntry::hasOperator(void) const
{
    return (mOperator != eOperator::None);
}

inline SMConditionEntry::eOperandKind SMConditionEntry::getRhsKind(void) const
{
    return mRhsKind;
}

inline void SMConditionEntry::setRhsKind(eOperandKind kind)
{
    mRhsKind = kind;
}

inline const QString& SMConditionEntry::getRhs(void) const
{
    return mRhs;
}

inline void SMConditionEntry::setRhs(const QString& rhs)
{
    mRhs = rhs;
}

inline bool SMConditionEntry::isNegated(void) const
{
    return mNegate;
}

inline void SMConditionEntry::setNegated(bool negate)
{
    mNegate = negate;
}

inline const QString& SMConditionEntry::getExpression(void) const
{
    return mExpression;
}

inline void SMConditionEntry::setExpression(const QString& expression)
{
    mExpression = expression;
}

inline const QList<SMArgumentEntry>& SMConditionEntry::getArguments(void) const
{
    return mArguments;
}

inline QList<SMArgumentEntry>& SMConditionEntry::getArguments(void)
{
    return mArguments;
}

inline bool SMConditionEntry::isExpressionRow(void) const
{
    return (mLhsKind == eOperandKind::Expression);
}

inline const QString& SMConditionEntry::getName(void) const
{
    return mLhs;
}

inline SMConditionList::eCombine SMConditionList::getCombine(void) const
{
    return mCombine;
}

inline void SMConditionList::setCombine(eCombine combine)
{
    mCombine = combine;
}

#endif  // LUSAN_DATA_SM_SMCONDITION_HPP
