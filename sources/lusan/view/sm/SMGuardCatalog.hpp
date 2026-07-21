#ifndef LUSAN_VIEW_SM_SMGUARDCATALOG_HPP
#define LUSAN_VIEW_SM_SMGUARDCATALOG_HPP
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
 *  \file        lusan/view/sm/SMGuardCatalog.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard editable-surface symbol catalog (B4/B5 source).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMGuardNode;

/**
 * \struct  SMGuardSymbol
 * \brief   One entry of the guard editing surface's symbol universe (D6 bare names): a
 *          stimulus parameter, an attribute, a constant, a handler condition, or a named
 *          lambda -- everything the completion catalog (B4) offers and the signature card
 *          (B5) needs to place slots. Names, not IDs: the surface is textual, so completion
 *          inserts a bare name and the parser re-resolves it to an ID at commit (this is what
 *          keeps click-and-type paths identical).
 **/
struct SMGuardSymbol
{
    /**
     * \enum    eRefKind
     * \brief   The reference kind that becomes the `@kind:` word when the completer inserts a
     *          symbol (SM-21-03). Distinct from \ref owner (a hue) because two kinds (attr and
     *          const) share the FSM hue yet need different `@kind:` prefixes.
     **/
    enum class eRefKind
    {
          Param     //!< `@param:` -- a stimulus parameter.
        , Attr      //!< `@attr:`  -- a machine attribute.
        , Const     //!< `@const:` -- a declared constant.
        , Cond      //!< `@cond:`  -- a condition method (handler or lambda).
    };

    QString             name;       //!< The bare declared name inserted into the field.
    QString             glyph;      //!< The ASCII owner/kind glyph (a # K h {}).
    NEGuardStyle::eOwner owner;     //!< The owner hue.
    eRefKind            refkind;    //!< The `@kind:` reference kind (for canonical insertion).
    QString             typeText;   //!< The type shown in the catalog (return type for calls).
    QString             provenance; //!< The right-column cue (handler() / lambda / = value / empty).
    bool                isCall;     //!< True for a condition method (needs an argument list).
    QStringList         paramNames; //!< The declared parameter names (calls only).
    QStringList         paramTypes; //!< The declared parameter types, parallel to paramNames.
    uint32_t            symbolId;   //!< The document ID of the declaration (for hovers / where-used).

    //!< The catalog display label: `name(type, type)` for a call, else the bare name.
    QString display() const;

    //!< The `@kind:` word for this symbol ("param" | "attr" | "const" | "cond").
    QString kindWord() const;

    //!< The canonical mention the completer inserts: `@kind:name` (`@cond:name` for a call).
    QString mention() const;

    //!< The human noun for this kind ("attribute" | "constant" | "parameter" | "condition"),
    //!< used by the W1 collision advisory ("there is an attribute named 'x'").
    QString kindNoun() const;
};

/**
 * \struct  SMGuardRawCollision
 * \brief   One W1 raw-collision hit: a \ref SMGuardNode::eKind::Raw node whose whole text is a
 *          bare identifier that EXACTLY matches one or more in-scope symbol names (SM-21-08). The
 *          committed guard kept the name as raw C++ (it resolved to nothing when it was typed),
 *          but a symbol of that name now exists -- so the editor offers a one-click "bind as
 *          `@kind:name`" courtesy through the warning channel. Recomputed on every projection
 *          pass (D-SYNC): a rename that removes the collision drops the hit with no guard edit.
 **/
struct SMGuardRawCollision
{
    QList<int>              path;       //!< The child-index path to the Raw node (empty = root).
    QString                 name;       //!< The raw identifier text (a bare identifier).
    QList<SMGuardSymbol>    matches;    //!< The in-scope symbols of that name (>1 => picker).
};

/**
 * \namespace   SMGuardCatalog
 * \brief   Builds the guard symbol universe for a transition's stimulus scope from the
 *          document registries. One builder feeds both the completion catalog and the
 *          signature card so they never disagree.
 **/
namespace SMGuardCatalog
{
    //!< Every in-scope symbol for \p transitionId, owner-grouped order (stimulus, fsm, handler).
    QList<SMGuardSymbol> build(const StateMachineData& data, uint32_t transitionId);

    //!< The bare completion words (call names keep their bare form; slots are added on accept).
    QStringList completionWords(const StateMachineData& data, uint32_t transitionId);

    /**
     * \brief   The did-you-mean suggestion for an unresolved \p typed name: the nearest
     *          \p candidate by Levenshtein distance, within a small threshold (<= 2 edits, and
     *          never more than a third of the length). Empty when nothing is close enough.
     *          Model-facing and headless-testable.
     **/
    QString nearestName(const QStringList& candidates, const QString& typed);

    /**
     * \brief   The LOCAL use-count of every bound symbol in one committed guard \p tree:
     *          symbol document id -> number of BOUND references (Call / Attr / Const / Param
     *          nodes). Raw / lambda / literal text is never counted (D-HILITE / A2). Headless
     *          and testable; the catalog's `used-N` column is exactly this over the current
     *          transition's guard.
     **/
    QHash<uint32_t, int> useCounts(const SMGuardNode* tree);

    /**
     * \brief   The W1 raw-collision hits over one committed guard \p tree (SM-21-08): every
     *          \ref SMGuardNode::eKind::Raw node whose WHOLE (trimmed) text is a single bare
     *          identifier that exactly matches at least one in-scope symbol name for
     *          \p transitionId. Text that is not a bare identifier -- operators, calls, literals,
     *          member access, anything with a space or a symbol -- is never a hit (the hazard:
     *          this is a quiet courtesy, not a linter). Headless and testable; the warning
     *          channel routes each hit to a "bind as `@kind:name`" quick-fix. Recomputed on the
     *          projection pass, so a rename that removes the collision drops the hit.
     **/
    QList<SMGuardRawCollision> rawCollisions(const StateMachineData& data, uint32_t transitionId, const SMGuardNode* tree);
}

#endif  // LUSAN_VIEW_SM_SMGUARDCATALOG_HPP
