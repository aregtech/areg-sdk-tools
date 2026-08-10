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
 *  \file        lusan/model/common/DocValidationController.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, background scheduler for a document's validation engine.
 *
 ************************************************************************/

#include "lusan/model/common/DocValidationController.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"

DocValidationController::DocValidationController(IEDocumentModel& document, FuncValidate checker, QObject* parent /*= nullptr*/)
    : QObject   (parent)
    , mDocument (document)
    , mChecker  (std::move(checker))
    , mTimer    ( )
    , mIssues   ( )
{
    mTimer.setSingleShot(true);
    mTimer.setInterval(DEFAULT_DEBOUNCE_MS);
    connect(&mTimer, &QTimer::timeout, this, &DocValidationController::onDebounceElapsed);

    // Only structural and reference changes affect the rule set; layout and read-only do not.
    DocModelNotifier& notifier = mDocument.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded,      this, &DocValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved,    this, &DocValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::elementChanged,    this, &DocValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::listReordered,     this, &DocValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::nameChanged,       this, &DocValidationController::onModelChanged);
    connect(&notifier, &DocModelNotifier::documentReloaded,  this, &DocValidationController::onModelChanged);
}

void DocValidationController::validateNow()
{
    mTimer.stop();
    mIssues = (mChecker ? mChecker() : QList<DocIssue>());
    emit validationUpdated(mIssues);
}

void DocValidationController::scheduleValidation()
{
    mTimer.start();
}

void DocValidationController::setDebounceInterval(int milliseconds)
{
    mTimer.setInterval(milliseconds < 0 ? 0 : milliseconds);
}

void DocValidationController::onModelChanged()
{
    mTimer.start();
}

void DocValidationController::onDebounceElapsed()
{
    validateNow();
}
