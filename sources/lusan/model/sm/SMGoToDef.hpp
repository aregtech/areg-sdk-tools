#ifndef LUSAN_MODEL_SM_SMGOTODEF_HPP
#define LUSAN_MODEL_SM_SMGOTODEF_HPP
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
 *  \file        lusan/model/sm/SMGoToDef.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM go-to-declaration: the declarations a canvas element uses.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMReferences.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \namespace   SMGoToDef
 * \brief   Resolves the registry declarations a single canvas element (a state or a transition)
 *          references, each to its document ID and current name, so the editor can jump from a
 *          use on the canvas to its declaration page. This is the inverse of where-used and its
 *          model-layer twin: it unions the name-based references (SMReferences, the headless
 *          walker) with the ID-bound guard symbols (condition / attribute / constant), exactly as
 *          SMWhereUsed does, so the two navigation directions never disagree.
 **/
namespace SMGoToDef
{
    /**
     * \struct  Target
     * \brief   One declaration referenced by the queried element, ready to navigate to.
     **/
    struct Target
    {
        SMReferences::eTarget   kind;       //!< The kind of the referenced declaration.
        uint32_t                declId;     //!< The declaration's resolved document ID.
        QString                 name;       //!< The declaration's current name (for the menu label).
    };

    /**
     * \brief   Every registry declaration referenced from the element \p elementId (\p isState
     *          selects a state vs a transition), resolved to ID and name and de-duplicated. A
     *          dangling reference (a name that resolves to no declaration) is skipped, since it
     *          has nowhere to navigate.
     * \param   data        The document to inspect.
     * \param   elementId   The state or transition the user acted on.
     * \param   isState     True when \p elementId is a state, false for a transition.
     * \return  The navigable declarations, in document order, without duplicates.
     **/
    QList<Target> collect(const StateMachineData& data, uint32_t elementId, bool isState);

    /**
     * \brief   The single stimulus declaration a transition fires on (its trigger, event, or timer),
     *          resolved to ID and name. Unlike collect(), this isolates the stimulus from the
     *          transition's operations (which can reference the same event/timer), so a Ctrl+Shift
     *          click on the stimulus part of an edge label jumps to exactly that declaration.
     * \param   data            The document to inspect.
     * \param   transitionId    The transition whose stimulus to resolve.
     * \return  The stimulus target; declId is 0 when the transition has no stimulus or it is dangling.
     **/
    Target stimulusOf(const StateMachineData& data, uint32_t transitionId);

    /**
     * \brief   Resolves a set of name-based references (e.g. the operations of one state-body row)
     *          to navigable declarations: each to its document ID and current name, dangling names
     *          dropped and duplicates removed. Mirrors the resolution collect() applies, so a row
     *          click and a whole-element go-to-declaration agree on where a reference points.
     * \param   data    The document to inspect.
     * \param   refs    The references to resolve (kind + name).
     * \return  The navigable declarations, without danglers or duplicates.
     **/
    QList<Target> resolveRefs(const StateMachineData& data, const QList<SMReferences::Ref>& refs);

    /**
     * \brief   A short, user-facing word for a target kind ("trigger", "condition", "timer", ...),
     *          used to disambiguate same-named declarations in the go-to-declaration menu.
     **/
    QString kindWord(SMReferences::eTarget kind);
}

#endif  // LUSAN_MODEL_SM_SMGOTODEF_HPP
