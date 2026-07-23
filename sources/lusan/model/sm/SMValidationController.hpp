#ifndef LUSAN_MODEL_SM_SMVALIDATIONCONTROLLER_HPP
#define LUSAN_MODEL_SM_SMVALIDATIONCONTROLLER_HPP
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
 *  \file        lusan/model/sm/SMValidationController.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, background scheduler for the FSM validation engine.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QTimer>

#include "lusan/model/sm/SMValidator.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class DocModelNotifier;

/**
 * \class   SMValidationController
 * \brief   Drives \ref SMValidator continuously without blocking editing: it listens to the
 *          document's change notifier, coalesces bursts of edits behind a short debounce
 *          timer, and re-runs the headless engine only after the user pauses, publishing the
 *          fresh findings by signal. Owned by the model facade; a results panel and the
 *          where-used and navigation surfaces consume `validationUpdated`.
 **/
class SMValidationController : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    static constexpr int DEFAULT_DEBOUNCE_MS { 300 };   //!< Idle gap before a re-validation runs.

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMValidationController(const StateMachineData& data, DocModelNotifier& notifier, QObject* parent = nullptr);
    virtual ~SMValidationController() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The findings of the most recent run (empty until the first run completes).
    inline const QList<SMIssue>& issues() const;

    //!< Runs the engine synchronously now and publishes the result (tests and the generator path).
    void validateNow();

    //!< Re-points at the current document after the facade swaps its data object, then
    //!< re-validates. The controller instance stays stable so consumers keep their connection.
    void setDocument(const StateMachineData& data);

    //!< Schedules a debounced re-validation (as a model change would).
    void scheduleValidation();

    //!< Overrides the debounce gap in milliseconds (0 runs on the next event-loop turn).
    void setDebounceInterval(int milliseconds);

signals:
    //!< Emitted after every completed run with the current findings.
    void validationUpdated(const QList<SMIssue>& issues);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onModelChanged();
    void onDebounceElapsed();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const StateMachineData* mData;      //!< The document under validation (re-pointed on facade swap).
    QTimer                  mTimer;     //!< The single-shot debounce timer.
    QList<SMIssue>          mIssues;    //!< The latest findings.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline const QList<SMIssue>& SMValidationController::issues() const
{
    return mIssues;
}

#endif  // LUSAN_MODEL_SM_SMVALIDATIONCONTROLLER_HPP
