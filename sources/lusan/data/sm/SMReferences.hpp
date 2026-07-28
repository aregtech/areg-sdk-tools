#ifndef LUSAN_DATA_SM_SMREFERENCES_HPP
#define LUSAN_DATA_SM_SMREFERENCES_HPP
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
 *  \file        lusan/data/sm/SMReferences.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM name-reference traversal: the single walker that
 *               answers "where is X used" and rewrites references on rename.
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
class SMOperationBase;

/**
 * \namespace   SMReferences
 * \brief   The one traversal of the recursive state tree that resolves every reference to
 *          a state or a registry entry. It backs where-used, atomic rename, and delete
 *          confirmation from a single definition of "reference" so the four features can
 *          never disagree (sm-arch-data.md section 2).
 *
 *          Two reference mechanisms coexist and are both covered here:
 *          - Name-based fields (transition `Stimulus`, `ActionCall@Action`,
 *            `AttributeSet@Attribute`, `TimerStart/Stop@Timer`, `EventSend@Event`,
 *            `State@OnFinal`, argument `Value` when its source is a registry entry). These
 *            are what rename rewrites.
 *          - ID-based references: a transition `To` names its target state by ID, and guard
 *            trees bind method/attribute/constant symbols by ID. These auto-reflect a rename
 *            and are never rewritten; where-used still reports them (transition targets here,
 *            guard uses delegated to SMGuardWhereUsed so guard knowledge stays in one place).
 **/
namespace SMReferences
{
    /**
     * \enum    eTarget
     * \brief   What a reference points at. Methods split into Trigger/Action/Condition
     *          because a method's kind decides which field references it; the other targets
     *          map one-to-one to a registry (or to a state).
     **/
    enum class eTarget
    {
          State         //!< A state, referenced by a transition `To` (by ID).
        , Trigger       //!< A trigger method (transition `Stimulus`, kind Trigger).
        , Action        //!< An action method (`ActionCall@Action`).
        , Condition     //!< A condition method (guard symbol; argument `Value`, source Condition).
        , Event         //!< An event (transition `Stimulus` kind Event; `EventSend`; `OnFinal`).
        , Timer         //!< A timer (transition `Stimulus` kind Timer; `TimerStart`/`TimerStop`).
        , Attribute     //!< An attribute (`AttributeSet`; argument/value `Value`, source Attribute; guard).
        , Constant      //!< A constant (argument/value `Value`, source Constant; guard).
        , Import        //!< A submachine import (`State@Submachine`).
    };

    /**
     * \struct  Use
     * \brief   One place that references the queried element, navigable in the editor.
     **/
    struct Use
    {
        uint32_t    navId;      //!< The element to select on navigation (a transition or a state).
        bool        isState;    //!< True when \a navId is a state, false when it is a transition.
        QString     location;   //!< Human-readable place, e.g. "Idle : Trigger start -> Running".
    };

    /**
     * \struct  Ref
     * \brief   One declaration referenced *by* a queried element -- the inverse of a Use. Backs
     *          go-to-declaration: the kind of registry element and the referenced name.
     **/
    struct Ref
    {
        eTarget target;     //!< The kind of the referenced declaration.
        QString name;       //!< The referenced declaration's name.
    };

    /**
     * \brief   Every place that references the element identified by \p target and, for
     *          name-based targets, \p name; for a State target, \p targetId (the state ID).
     *          For Condition/Attribute/Constant the ID-bound guard uses are appended.
     * \param   data        The document to walk.
     * \param   target      The kind of element being looked up.
     * \param   name        The element's current name (ignored for a State target).
     * \param   targetId    The element's ID (used for a State target and for guard uses).
     * \return  The referencing places, in document order.
     **/
    QList<Use> whereUsed(const StateMachineData& data, eTarget target, const QString& name, uint32_t targetId);

    /**
     * \brief   Every name-based declaration referenced from the element identified by
     *          \p elementId (\p isState selects a state vs a transition). This is the inverse of
     *          whereUsed: where whereUsed lists the sites that reference one declaration, this
     *          lists the declarations one element references -- a state's entry/exit/do operations
     *          and OnFinal, or a transition's stimulus and operations. ID-based references (a
     *          transition's target state, guard symbols) are not included here; the guard union is
     *          added one layer up (model), mirroring the where-used split so the two agree.
     * \param   data        The document to walk.
     * \param   elementId   The state or transition whose references are wanted.
     * \param   isState     True when \p elementId is a state, false for a transition.
     * \return  The referenced declarations, in document order (duplicates possible).
     **/
    QList<Ref> definitionsOf(const StateMachineData& data, uint32_t elementId, bool isState);

    /**
     * \brief   The registry declaration(s) one operation references directly -- its own action,
     *          attribute, timer, or event. Used to make a single state-body operation row
     *          navigable: a Ctrl+Shift click on the row jumps to what the operation runs. Argument
     *          references are intentionally excluded, so a row targets the operation itself.
     * \param   op  The operation to inspect.
     * \return  The referenced declaration(s); empty for inline code.
     **/
    QList<Ref> operationRefs(const SMOperationBase& op);

    /**
     * \brief   Rewrites every name-based reference of (\p target, \p oldName) to \p newName
     *          in place. ID-based references (transition targets, guard symbols) are left
     *          untouched -- they already reflect the new name. This is the model mutation the
     *          atomic-rename command wraps; it does not emit notifications and does not touch
     *          the primary element's own name.
     * \return  The number of reference sites rewritten.
     **/
    int rewriteReferences(StateMachineData& data, eTarget target, const QString& oldName, const QString& newName);
}

#endif  // LUSAN_DATA_SM_SMREFERENCES_HPP
