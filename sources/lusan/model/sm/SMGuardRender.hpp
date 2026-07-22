#ifndef LUSAN_MODEL_SM_SMGUARDRENDER_HPP
#define LUSAN_MODEL_SM_SMGUARDRENDER_HPP
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
 *  \file        lusan/model/sm/SMGuardRender.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard renderer: AST -> canonical text + span roles.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMGuardNode;
class SMGuard;

/**
 * \class   SMGuardRender
 * \brief   Renders a resolved guard tree back to its canonical, human-readable surface text
 *          (bare declared names, minimal precedence parentheses) plus a span list that the
 *          highlighter colors by owner. The tree is the truth: `render(parse(text))` is the
 *          canonical form of \p text, and `parse(render(tree))` reproduces \p tree. Because
 *          names are looked up by ID at render time, a rename re-renders every guard with the
 *          new name and no guard edit.
 **/
class SMGuardRender
{
public:
    /**
     * \enum    eRole
     * \brief   The owner/kind role of a text span, mapped to a visual token by the
     *          highlighter. Roles, never literal colors.
     **/
    enum class eRole
    {
          Stim      //!< A stimulus parameter reference.
        , Fsm       //!< An attribute, constant, or named-lambda reference.
        , Handler   //!< A handler-condition call.
        , Literal   //!< A verbatim literal.
        , Raw       //!< A raw-C++ fragment.
        , Lambda    //!< An anonymous-lambda body token.
        , Operator  //!< A logical/comparison operator.
        , Punct     //!< Parentheses, commas.
    };

    /**
     * \struct  Span
     * \brief   One colored span of the rendered text.
     **/
    struct Span
    {
        int     start;      //!< The 0-based start offset in the rendered text.
        int     length;     //!< The span length.
        eRole   role;       //!< The owner/kind role.
    };

    /**
     * \struct  Chip
     * \brief   One committed reference occurrence in the rendered text, addressed by its span
     *          so the field can fold it into a compact chip. Chips appear in text
     *          order, which is pre-order over the reference nodes (the nth chip is the nth
     *          reference node) -- the same INDEX identity the island tokens use. The
     *          view maps \ref role to an owner hue/glyph; \ref kind is the `#kind:` word.
     **/
    struct Chip
    {
        int     start;      //!< The 0-based start offset of the reference in the rendered text.
        int     length;     //!< The span length (including any disambiguating `#kind:` prefix).
        eRole   role;       //!< The owner role (maps to a hue / glyph in the view).
        QString kind;       //!< The reference kind word: "param" | "attr" | "const" | "cond".
        QString name;       //!< The bare display name shown inside the chip.
        bool    reveal;     //!< Keep the `#kind:` prefix visible (a same-name collision).
    };

    /**
     * \struct  Rendered
     * \brief   The rendered text, its span list, and the reference chips it contains.
     **/
    struct Rendered
    {
        QString         text;
        QList<Span>     spans;
        QList<Chip>     chips;
    };

    /**
     * \struct  NodeSpan
     * \brief   The full text range one tree node occupies in the rendered text (including
     *          its wrapping parentheses), addressed by its child-index path from the root.
     *          The structure lens and mapping grid use it to map a node to a caret span --
     *          from the same render walk, so the two views can never disagree on offsets.
     **/
    struct NodeSpan
    {
        QList<int>  path;       //!< The child-index path from the root (empty = the root).
        int         start;      //!< The 0-based start offset in the rendered text.
        int         length;     //!< The span length.
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The canonical text of a node sub-tree. With \p layout set, user line breaks and
     *          indent are restored (R18); the default single-line form is unchanged, so every
     *          caller that wants inline text (caches, argument projections) is byte-stable.
     **/
    static QString text(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node, bool layout = false);

    //!< The canonical text plus span list of a node sub-tree; \p layout restores line breaks (R18).
    static Rendered render(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node, bool layout = false);

    //!< The text range of every node of the sub-tree; \p layout must match the displayed text (R18).
    static QList<NodeSpan> nodeSpans(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node, bool layout = false);

    /**
     * \brief   The display text of a whole guard: the rendered tree (ok), the raw draft text
     *          (draft), or an empty string (empty). \p layout restores multi-line guards (R18).
     **/
    static QString guardText(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard, bool layout = false);

    /**
     * rief   A SHORT, structural summary of \p guard for the FSM canvas label, where the full
     *          expression does not fit and a wall of C++ tells the reader nothing.
     *          Operators and structure are kept; the bulky parts collapse:
     *          - a condition call keeps its name, its arguments collapse: `HasWaiting(...)`;
     *          - a lambda-implemented (Embedded) condition captures its scope, so it takes no
     *            arguments and reads `name()`;
     *          - an inline `{ ... }` block is an anonymous `[this]() -> bool` lambda and reads
     *            `this -> bool`;
     *          - a raw C++ fragment reads `{...}`.
     *          References and literals are already short and are kept verbatim, so a plain guard
     *          such as `count > 3 && ready` summarises to itself.
     *          An unresolved draft has no tree to walk and returns its raw text unchanged.
     **/
    static QString canvasSummary(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard);
};

#endif  // LUSAN_MODEL_SM_SMGUARDRENDER_HPP
