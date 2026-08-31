#ifndef LUSAN_MODEL_COMMON_DOCVALIDATIONCONTROLLER_HPP
#define LUSAN_MODEL_COMMON_DOCVALIDATIONCONTROLLER_HPP
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
 *  \file        lusan/model/common/DocValidationController.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, background scheduler for a document's validation engine.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QTimer>

#include "lusan/model/common/DocIssue.hpp"

#include <functional>

/************************************************************************
 * Dependencies
 ************************************************************************/
class IEDocumentModel;

/**
 * \class   DocValidationController
 * \brief   Runs a document's validation engine continuously without blocking editing: it listens
 *          to the document's change notifier, coalesces bursts of edits behind a short debounce
 *          timer, re-runs the engine only after the author pauses, and publishes the fresh
 *          findings by signal.
 *
 *          The engine itself is a plain callable handed over at construction, so a state machine
 *          and a service interface differ only in which checker they pass -- one controller, one
 *          schedule, one signal, whatever the document is. The callable reads the document
 *          through the facade rather than capturing its data object, so a file that is opened or
 *          reloaded is validated without re-pointing anything.
 *
 *          Owned by the document facade; the results panel consumes \ref validationUpdated.
 **/
class DocValidationController : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< A document's whole check run: everything wrong with it right now.
    using FuncValidate = std::function<QList<DocIssue>()>;

    static constexpr int DEFAULT_DEBOUNCE_MS { 300 };   //!< Idle gap before a re-validation runs.

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    DocValidationController(IEDocumentModel& document, FuncValidate checker, QObject* parent = nullptr);
    virtual ~DocValidationController() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The document under validation.
    inline IEDocumentModel& getDocument() const;

    //!< The findings of the most recent run (empty until the first run completes).
    inline const QList<DocIssue>& issues() const;

    //!< Runs the engine synchronously now and publishes the result (tests and the generator path).
    void validateNow();

    //!< Schedules a debounced re-validation (as a model change would).
    void scheduleValidation();

    //!< Overrides the debounce gap in milliseconds (0 runs on the next event-loop turn).
    void setDebounceInterval(int milliseconds);

signals:
    //!< Emitted after every completed run with the current findings.
    void validationUpdated(const QList<DocIssue>& issues);

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
    IEDocumentModel&    mDocument;  //!< The document under validation.
    FuncValidate        mChecker;   //!< The engine run.
    QTimer              mTimer;     //!< The single-shot debounce timer.
    QList<DocIssue>     mIssues;    //!< The latest findings.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline IEDocumentModel& DocValidationController::getDocument() const
{
    return mDocument;
}

inline const QList<DocIssue>& DocValidationController::issues() const
{
    return mIssues;
}

#endif  // LUSAN_MODEL_COMMON_DOCVALIDATIONCONTROLLER_HPP
