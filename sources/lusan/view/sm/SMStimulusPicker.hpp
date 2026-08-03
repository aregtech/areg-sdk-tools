#ifndef LUSAN_VIEW_SM_SMSTIMULUSPICKER_HPP
#define LUSAN_VIEW_SM_SMSTIMULUSPICKER_HPP
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
 *  \file        lusan/view/sm/SMStimulusPicker.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM: the shared stimulus combo (fill it, read it, commit it).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMTransition.hpp"

#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QComboBox;
class StateMachineData;
class StateMachineModel;

/**
 * \namespace   SMStimulusPicker
 * \brief       One implementation of "what may fire this transition", for every surface that asks.
 *              The Properties panel's transition page and its state page `Internal` editor both
 *              show the same closed list of every trigger, event and timer; this namespace is the
 *              single place that fills it, decides what a row means, and commits a pick.
 *
 *              A row carries its (kind, name) as item DATA and is matched by that, never by its
 *              text: two registries may legally hold the same name until validation objects, and
 *              the rows carry no kind prefix to tell them apart -- the kind is a drawn mark.
 **/
namespace SMStimulusPicker
{
    //!< The stimulus kind (int of \c eStimulusKind) carried by each picker row.
    constexpr int RoleKind { Qt::UserRole };

    //!< The real registry name carried by each picker row (the label may be prefixed).
    constexpr int RoleName { Qt::UserRole + 1 };

    /**
     * \brief   The display label for a stimulus: the REGISTRY name, exactly as declared. What kind
     *          it is comes from the row's mark, never from a synthesized handler name --
     *          `on_event_<name>` / `on_timer_<name>` invented a method signature that is the code
     *          generator's to choose, not Lusan's, and it did not match what the transition then
     *          showed on the canvas (issue #543).
     **/
    QString displayLabel(SMTransitionEntry::eStimulusKind kind, const QString& name);

    /**
     * \brief   Fills \p picker with `(none)` plus every trigger, event and timer of \p data, each
     *          row marked by its kind and tipped with the kind spelled out.
     * \param   picker      The combo to fill; cleared first.
     * \param   data        The document whose registries are offered.
     * \param   currentKind The kind to preselect (int of \c eStimulusKind).
     * \param   currentName The name to preselect; empty selects `(none)`.
     * \return  The row to select, 0 (`(none)`) when the pair matches nothing.
     **/
    int fill(QComboBox& picker, const StateMachineData& data, int currentKind, const QString& currentName);

    /**
     * \brief   Applies the row \p picker currently shows to \p transitionId as one undo step.
     *          `(none)` detaches the stimulus; the picker never creates or renames a registry entry.
     *          A no-op when nothing changed, so it is safe to call on every `activated`.
     **/
    void apply(StateMachineModel& model, const QComboBox& picker, uint32_t transitionId);
}

#endif  // LUSAN_VIEW_SM_SMSTIMULUSPICKER_HPP
