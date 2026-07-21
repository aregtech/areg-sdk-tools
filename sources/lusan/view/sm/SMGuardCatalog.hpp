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

#include <QList>
#include <QString>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

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
    QString             name;       //!< The bare declared name inserted into the field.
    QString             glyph;      //!< The ASCII owner/kind glyph (a # K h {}).
    NEGuardStyle::eOwner owner;     //!< The owner hue.
    QString             typeText;   //!< The type shown in the catalog (return type for calls).
    QString             provenance; //!< The right-column cue (handler() / lambda / = value / empty).
    bool                isCall;     //!< True for a condition method (needs an argument list).
    QStringList         paramNames; //!< The declared parameter names (calls only).
    QStringList         paramTypes; //!< The declared parameter types, parallel to paramNames.
    uint32_t            symbolId;   //!< The document ID of the declaration (for hovers / where-used).

    //!< The catalog display label: `name(type, type)` for a call, else the bare name.
    QString display() const;
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
}

#endif  // LUSAN_VIEW_SM_SMGUARDCATALOG_HPP
