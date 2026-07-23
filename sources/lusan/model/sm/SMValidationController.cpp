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
 *  \file        lusan/model/sm/SMValidationController.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, background scheduler for the FSM validation engine.
 *
 ************************************************************************/

#include "lusan/model/sm/SMValidationController.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"

SMValidationController::SMValidationController(const StateMachineData& data, DocModelNotifier& notifier, QObject* parent)
    : QObject   (parent)
    , mData     (&data)
    , mTimer    ( )
    , mIssues   ( )
{
    mTimer.setSingleShot(true);
    mTimer.setInterval(DEFAULT_DEBOUNCE_MS);
    connect(&mTimer, &QTimer::timeout, this, &SMValidationController::onDebounceElapsed);

    // Only structural/reference changes affect the rule set; layout and read-only do not.
    connect(&notifier, &DocModelNotifier::elementAdded,      this, &SMValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved,    this, &SMValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::elementChanged,    this, &SMValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::listReordered,     this, &SMValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::nameChanged,       this, &SMValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::documentReloaded,  this, &SMValidationController::onModelChanged);
}

void SMValidationController::validateNow()
{
    mTimer.stop();
    mIssues = (mData != nullptr ? SMValidator::validate(*mData) : QList<SMIssue>());
    emit validationUpdated(mIssues);
}

void SMValidationController::setDocument(const StateMachineData& data)
{
    mData = &data;
    validateNow();
}

void SMValidationController::scheduleValidation()
{
    mTimer.start();
}

void SMValidationController::setDebounceInterval(int milliseconds)
{
    mTimer.setInterval(milliseconds < 0 ? 0 : milliseconds);
}

void SMValidationController::onModelChanged()
{
    mTimer.start();
}

void SMValidationController::onDebounceElapsed()
{
    validateNow();
}
