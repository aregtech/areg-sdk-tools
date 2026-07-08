#ifndef LUSAN_MODEL_SM_SMTIMERMODEL_HPP
#define LUSAN_MODEL_SM_SMTIMERMODEL_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMTimerModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Timers page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class DocModelNotifier;

/**
 * \class   SMTimerModel
 * \brief   The Timers page model. Reads the live `TimerList` section through the facade and
 *          routes every edit through an undo command, so the page never mutates
 *          `SMTimerEntry` directly. `SMTimerEntry` is stored by value in its section, so
 *          mutators identify an entry by ID and re-resolve it inside the command's
 *          getter/setter rather than capturing a pointer that a sibling insert/remove could
 *          invalidate.
 **/
class SMTimerModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMTimerModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<SMTimerEntry>& getTimers() const;
    int getTimerCount() const;

    SMTimerEntry* findTimer(const QString& name) const;
    SMTimerEntry* findTimer(uint32_t id) const;
    int findIndex(uint32_t id) const;

    DocModelNotifier& getNotifier() const;

    //!< Resolves a name in the document-wide stimulus name space (spec 6.10) so the page
    //!< can flag a collision as the user types, without waiting for commit.
    StateMachineData::StimulusRef findStimulus(const QString& name) const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    SMTimerEntry* createTimer(const QString& name);
    SMTimerEntry* insertTimer(int position, const QString& name);
    void deleteTimer(uint32_t id);
    void swapTimers(uint32_t firstId, uint32_t secondId);

    void renameTimer(uint32_t id, const QString& newName);
    //!< Timeout in milliseconds; minimum 1 (spec 6.10 — a zero timeout is invalid).
    void setTimeout(uint32_t id, uint32_t timeout);
    //!< Repeat count; 0 (or 0xFFFFFFFF) means continuous.
    void setRepeat(uint32_t id, uint32_t repeat);
    void setDescription(uint32_t id, const QString& text);
    //!< Sets the deprecated flag (and clears the hint when cleared) as one undo step.
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMTimerData& timers() const;
    SMTimerData& timers();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMTIMERMODEL_HPP
