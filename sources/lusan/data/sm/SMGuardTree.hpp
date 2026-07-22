#ifndef LUSAN_DATA_SM_SMGUARDTREE_HPP
#define LUSAN_DATA_SM_SMGUARDTREE_HPP
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
 *  \file        lusan/data/sm/SMGuardTree.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition guard: the ID-bound expression tree (AST)
 *               and the guard state holder that persists it in `.fsml`.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QString>
#include <cstdint>

class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * \namespace   NEGuardText
 * \brief       The lexical constants of the guard editing surface. The reference sigil is
 *              declared here -- the single point of change -- because the parser (grammar),
 *              the renderer (canonical text) and the field/catalog (insertion, completer)
 *              must all agree on the very same character.
 **/
namespace NEGuardText
{
    //!< The character that opens a typed reference mention `#kind:name` in the guard text.
    constexpr QChar     RefSigil    { QLatin1Char('#') };

    //!< The separator between the kind word and the symbol name in `#kind:name`.
    constexpr QChar     KindSep     { QLatin1Char(':') };

    //!< Builds the canonical `#kind:` prefix of a reference mention.
    inline QString refPrefix(const QString& kindWord)
    {
        return QString(RefSigil) + kindWord + QString(KindSep);
    }
}

/**
 * \class   SMGuardNode
 * \brief   One node of a transition guard's resolved expression tree. The tree is the
 *          deterministic storage: reference nodes carry a symbol's
 *          document ID (never a name -- names live at declarations), verbatim nodes carry
 *          their bytes. A single tagged class (not a hierarchy) so the XML mapping stays
 *          "element name = node kind" and a walker can switch on the kind. Children are
 *          owned by pointer; the node deep-copies on clone and frees them on destruction.
 **/
class SMGuardNode
{
public:
    /**
     * \enum    eKind
     * \brief   The node kind. Also the XML element name of the node.
     **/
    enum class eKind
    {
          And       //!< n-ary conjunction (2+ children).
        , Or        //!< n-ary disjunction (2+ children).
        , Not       //!< logical negation (exactly 1 child).
        , Cmp       //!< comparison (exactly 2 children), operator in \ref eCmpOp.
        , Call      //!< a condition-method / named-lambda call (symbol ID + ordered args).
        , Attr      //!< a machine attribute reference (symbol ID).
        , Const     //!< a declared constant reference (symbol ID).
        , Param     //!< an in-scope stimulus parameter reference (symbol ID).
        , Lit       //!< a verbatim literal token (number / string / bool / enum member).
        , Lambda    //!< a verbatim anonymous-lambda boolean body.
, Raw       //!< a verbatim raw-C++ fragment (the explicit escape hatch).
    };

    /**
     * \enum    eCmpOp
     * \brief   The comparison operator of a \ref eKind::Cmp node.
     **/
    enum class eCmpOp
    {
          Eq        //!< ==
        , Ne        //!< !=
        , Lt        //!< <
        , Le        //!< <=
        , Gt        //!< >
        , Ge        //!< >=
    };

    static QLatin1StringView toString(eKind kind);
    static const char* toString(eCmpOp op);
    static bool fromKindString(const QString& text, eKind& kind);
    static bool fromOpString(const QString& text, eCmpOp& op);

//////////////////////////////////////////////////////////////////////////
// Factories
//////////////////////////////////////////////////////////////////////////
public:
    //!< A logical And/Or over the (2+) children; takes ownership of the children.
    static SMGuardNode* makeGroup(eKind kind, const QList<SMGuardNode*>& children);
    //!< A Not over the single child; takes ownership.
    static SMGuardNode* makeNot(SMGuardNode* child);
    //!< A comparison lhs <op> rhs; takes ownership of lhs and rhs.
    static SMGuardNode* makeCmp(eCmpOp op, SMGuardNode* lhs, SMGuardNode* rhs);
    //!< A call of the symbol \p symbolId with the ordered \p args; takes ownership of args.
    static SMGuardNode* makeCall(uint32_t symbolId, const QList<SMGuardNode*>& args);
    //!< A reference leaf (Attr / Const / Param) to the symbol \p symbolId.
    static SMGuardNode* makeRef(eKind kind, uint32_t symbolId);
    //!< A verbatim leaf (Lit / Lambda / Raw) carrying \p text.
    static SMGuardNode* makeVerbatim(eKind kind, const QString& text);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardNode(eKind kind);
    SMGuardNode(const SMGuardNode& src);
    ~SMGuardNode();

    SMGuardNode& operator = (const SMGuardNode& other);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eKind getKind() const;
    //!< Retags a group node (And <-> Or); the guard edit commands use it to flip a combinator.
    inline void setKind(eKind kind);

    inline eCmpOp getOp() const;
    inline void setOp(eCmpOp op);

    inline uint32_t getSymbolId() const;
    inline void setSymbolId(uint32_t id);

    /**
     * \brief   The document id of the FORMAL parameter this node is bound to -- meaningful
     *          ONLY when the node is a direct argument child of a \ref eKind::Call node.
     *          0 means "not an argument" or a legacy positional arg (no id was stored),
     *          which the projection matches back to a formal by position.
     *          Keying an arg on the formal's id (never its position) is what lets a signature
     *          insert/rename never re-bind, and what makes ghost and orphan rows representable.
     **/
    inline uint32_t getArgFormalId() const;
    inline void setArgFormalId(uint32_t id);

    /**
     * \brief   Layout hint: true when the user placed a line break before this operand. It is
     *          meaningful only for a node rendered in an operand sequence (a group child or a
     *          call argument). User-owned surface layout -- the renderer re-applies it and the
     *          code generator ignores it, so it never changes generated C++. It is NOT
     *          part of \ref equals (structural identity), which stays layout-independent.
     **/
    inline bool isBreakBefore() const;
    inline void setBreakBefore(bool breakBefore);

    //!< Layout hint: the leading spaces the renderer restores on the line this operand opens (R18).
    inline int getIndent() const;
    inline void setIndent(int indent);

    inline const QString& getText() const;
    inline void setText(const QString& text);

    inline const QList<SMGuardNode*>& getChildren() const;
    inline QList<SMGuardNode*>& getChildren();
    inline int getCount() const;
    inline SMGuardNode* childAt(int index) const;

    //!< Appends a child, taking ownership; returns the same pointer.
    SMGuardNode* addChild(SMGuardNode* child);

    inline bool isGroup() const;    //!< And or Or.
    inline bool isLeaf() const;     //!< Attr/Const/Param/Lit/Lambda/Raw (no children).
    inline bool isReference() const;//!< Attr/Const/Param (carries a symbol ID).
    inline bool isVerbatim() const; //!< Lit/Lambda/Raw (carries verbatim text).

    /**
     * \brief   Deep structural equality (kind, operator, symbol IDs, verbatim text, and
     *          the ordered children), ignoring nothing. Used by the round-trip test.
     **/
    bool equals(const SMGuardNode& other) const;

    //!< A deep copy of this node and its whole sub-tree.
    SMGuardNode* clone() const;

//////////////////////////////////////////////////////////////////////////
// XML persistence
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Writes this node as its kind element. A \ref eKind::Call wraps each child in
     *          an `<Arg>` element (declared-parameter order); all other kinds write their
     *          children directly.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Reads one node from the current start element (the reader must be positioned
     *          on a node start element). Returns the parsed node (caller owns it) or nullptr
     *          when the element is not a recognized guard node kind.
     **/
    static SMGuardNode* readFromXml(QXmlStreamReader& xml);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eKind                   mKind;      //!< The node kind.
    eCmpOp                  mOp;        //!< The comparison operator (Cmp only).
    uint32_t                mSymbolId;  //!< The referenced symbol's document ID (Call/Attr/Const/Param).
    uint32_t                mArgFormalId;//!< The bound formal parameter's ID (a Call's arg child only; 0 = positional).
    bool                    mBreakBefore;//!< Layout: a user line break precedes this operand (R18).
    int                     mIndent;    //!< Layout: leading spaces on this operand's line (R18).
    QString                 mText;      //!< The verbatim bytes (Lit/Lambda/Raw).
    QList<SMGuardNode*>     mChildren;  //!< The owned child nodes, in operand order.
};

//////////////////////////////////////////////////////////////////////////
// SMGuard class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMGuard
 * \brief   A transition's guard state: `empty` (no guard, the transition always fires),
 *          `ok` (a fully resolved expression tree), or `draft` (the user's raw text kept
 *          losslessly, plus an optional last-good tree). Persisted under `<Guard>`; the
 *          absence of the element means an empty guard. `<Rendered>` is a human-readable
 *          cache written on save and ignored (semantically) on load -- the tree is the truth.
 **/
class SMGuard
{
public:
    /**
     * \enum    eState
     * \brief   The guard state.
     **/
    enum class eState
    {
          Empty     //!< No guard.
        , Ok        //!< A fully resolved tree.
        , Draft     //!< Unresolved raw text (+ optional last-good tree).
    };

    static const char* toString(eState state);
    static SMGuard::eState fromStateString(const QString& text);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMGuard();
    SMGuard(const SMGuard& src);
    SMGuard(SMGuard&& src) noexcept;
    ~SMGuard();

    SMGuard& operator = (const SMGuard& other);
    SMGuard& operator = (SMGuard&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eState getState() const;
    inline bool isEmpty() const;
    inline bool isOk() const;
    inline bool isDraft() const;

    //!< The resolved tree (Ok), the last-good tree (Draft, may be nullptr), or nullptr (Empty).
    inline const SMGuardNode* getTree() const;
    inline SMGuardNode* getTree();

    inline const QString& getDraftText() const;

    //!< The cached rendered display text (best-effort; ignored on load).
    inline const QString& getRendered() const;
    inline void setRendered(const QString& rendered);

    /**
     * \brief   Sets the guard to `ok` with the given resolved \p tree; takes ownership of
     *          \p tree and frees any previous tree.
     **/
    void setTree(SMGuardNode* tree);

    /**
     * \brief   Sets the guard to `draft` with the losslessly-kept \p text and an optional
     *          last-good \p lastGood tree; takes ownership of \p lastGood (may be nullptr).
     **/
    void setDraft(const QString& text, SMGuardNode* lastGood = nullptr);

    //!< Clears to the empty state, freeing any tree.
    void clear();

//////////////////////////////////////////////////////////////////////////
// XML persistence
//////////////////////////////////////////////////////////////////////////
public:
    //!< True when a `<Guard>` element is written (i.e. the guard is not empty).
    inline bool hasContent() const;

    /**
     * \brief   Writes the `<Guard>` element (nothing when empty): `state`, the `<Draft>`
     *          text (draft only), the `<Expr>` tree (present when a tree exists) and the
     *          `<Rendered>` cache (when set).
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Reads a `<Guard>` element (the reader must be positioned on its start
     *          element). Replaces this guard's content.
     **/
    bool readFromXml(QXmlStreamReader& xml);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eState          mState;     //!< The guard state.
    SMGuardNode*    mTree;       //!< The owned tree (Ok tree / Draft last-good / nullptr).
    QString         mDraftText;  //!< The lossless raw text (Draft only).
    QString         mRendered;   //!< The cached display text.
};

//////////////////////////////////////////////////////////////////////////
// SMGuardNode inline methods
//////////////////////////////////////////////////////////////////////////

inline SMGuardNode::eKind SMGuardNode::getKind() const
{
    return mKind;
}

inline void SMGuardNode::setKind(eKind kind)
{
    mKind = kind;
}

inline SMGuardNode::eCmpOp SMGuardNode::getOp() const
{
    return mOp;
}

inline void SMGuardNode::setOp(eCmpOp op)
{
    mOp = op;
}

inline uint32_t SMGuardNode::getSymbolId() const
{
    return mSymbolId;
}

inline void SMGuardNode::setSymbolId(uint32_t id)
{
    mSymbolId = id;
}

inline uint32_t SMGuardNode::getArgFormalId() const
{
    return mArgFormalId;
}

inline void SMGuardNode::setArgFormalId(uint32_t id)
{
    mArgFormalId = id;
}

inline bool SMGuardNode::isBreakBefore() const
{
    return mBreakBefore;
}

inline void SMGuardNode::setBreakBefore(bool breakBefore)
{
    mBreakBefore = breakBefore;
}

inline int SMGuardNode::getIndent() const
{
    return mIndent;
}

inline void SMGuardNode::setIndent(int indent)
{
    mIndent = indent;
}

inline const QString& SMGuardNode::getText() const
{
    return mText;
}

inline void SMGuardNode::setText(const QString& text)
{
    mText = text;
}

inline const QList<SMGuardNode*>& SMGuardNode::getChildren() const
{
    return mChildren;
}

inline QList<SMGuardNode*>& SMGuardNode::getChildren()
{
    return mChildren;
}

inline int SMGuardNode::getCount() const
{
    return static_cast<int>(mChildren.size());
}

inline SMGuardNode* SMGuardNode::childAt(int index) const
{
    return ((index >= 0) && (index < mChildren.size())) ? mChildren.at(index) : nullptr;
}

inline bool SMGuardNode::isGroup() const
{
    return ((mKind == eKind::And) || (mKind == eKind::Or));
}

inline bool SMGuardNode::isReference() const
{
    return ((mKind == eKind::Attr) || (mKind == eKind::Const) || (mKind == eKind::Param));
}

inline bool SMGuardNode::isVerbatim() const
{
    return ((mKind == eKind::Lit) || (mKind == eKind::Lambda) || (mKind == eKind::Raw));
}

inline bool SMGuardNode::isLeaf() const
{
    return (isReference() || isVerbatim());
}

//////////////////////////////////////////////////////////////////////////
// SMGuard inline methods
//////////////////////////////////////////////////////////////////////////

inline SMGuard::eState SMGuard::getState() const
{
    return mState;
}

inline bool SMGuard::isEmpty() const
{
    return (mState == eState::Empty);
}

inline bool SMGuard::isOk() const
{
    return (mState == eState::Ok);
}

inline bool SMGuard::isDraft() const
{
    return (mState == eState::Draft);
}

inline const SMGuardNode* SMGuard::getTree() const
{
    return mTree;
}

inline SMGuardNode* SMGuard::getTree()
{
    return mTree;
}

inline const QString& SMGuard::getDraftText() const
{
    return mDraftText;
}

inline const QString& SMGuard::getRendered() const
{
    return mRendered;
}

inline void SMGuard::setRendered(const QString& rendered)
{
    mRendered = rendered;
}

inline bool SMGuard::hasContent() const
{
    return (mState != eState::Empty);
}

#endif  // LUSAN_DATA_SM_SMGUARDTREE_HPP
