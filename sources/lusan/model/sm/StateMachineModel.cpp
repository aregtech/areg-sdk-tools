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
 *  \file        lusan/model/sm/StateMachineModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document facade.
 *
 ************************************************************************/

#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"

#include <QUndoCommand>

namespace
{
    constexpr int AutosaveIntervalMs{ 30000 };

    /**
     * \brief   The legacy read-shim (driver decision): a transition still carrying the
     *          A legacy `<ConditionList>` renders to text and becomes a `<Draft>` guard --
     *          the user re-resolves it in the editor; nothing is silently dropped.
     **/
    void convertLegacyGuards(SMStateData& level)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            for (SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if ((transition != nullptr)
                    && transition->getGuard().isEmpty()
                    && (transition->getConditions().isEmpty() == false))
                {
                    transition->getGuard() = SMGuardParser::fromLegacy(transition->getConditions());
                }
            }

            if (state->hasNestedStates())
            {
                convertLegacyGuards(*state->getNestedStates());
            }
        }
    }

    /**
     * \brief   The L3 read-shim: a `DoList` whose stop condition is still the pre-L3 free-text
     *          `Until` attribute is resolved ONCE, here, into the ID-bound tree every other
     *          predicate in the format already is. Resolution needs the grammar and the document
     *          registries, neither of which the data layer knows, which is why the reader keeps
     *          the bytes and this decides what they mean.
     *
     *          `allowRaw` is on: the text was verbatim C++ and may legitimately be something the
     *          guard grammar has no node for. Whatever resolves becomes real references -- which
     *          is the point, because a rename then follows them -- and whatever does not becomes
     *          a single `Raw` node, which is exactly what the whole attribute already was. The
     *          scope is 0: a `do/` activity ticks on a timer with no stimulus in hand, so no
     *          parameter is in scope and a parameter name in the old text was never bindable.
     **/
    void convertLegacyDoUntil(const StateMachineData& data, SMStateData& level)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            const QString legacy = state->getDoUntilLegacy().trimmed();
            if ((legacy.isEmpty() == false) && state->getDoUntil().isEmpty())
            {
                SMGuard parsed = SMGuardParser::parseToGuard(data, 0u, legacy, true);
                if (parsed.isOk() == false)
                {
                    // Not parseable at all: keep it whole and verbatim rather than as a draft.
                    // A draft would say "the user is mid-edit"; this text was never edited here.
                    parsed = SMGuard();
                    parsed.setTree(SMGuardNode::makeVerbatim(SMGuardNode::eKind::Raw, legacy));
                }

                state->setDoUntil(parsed);
            }

            state->clearDoUntilLegacy();

            if (state->hasNestedStates())
            {
                convertLegacyDoUntil(data, *state->getNestedStates());
            }
        }
    }

    /**
     * \brief   Refreshes the advisory `name` (R19) on every guard tree in the document just
     *          before a save, so the human-readable names written to `.fsml` reflect the current
     *          declarations even after a rename. The name is never read back -- the id binds.
     *          A `DoList`'s `<Until>` is a guard tree too, and it gets the same pass for the same
     *          reason: it is the rename safety the whole L3 change exists to buy.
     **/
    void refreshGuardNames(const StateMachineData& data, SMStateData& level)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            for (SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                SMGuardNode* tree = (transition != nullptr) ? transition->getGuard().getTree() : nullptr;
                if (tree != nullptr)
                {
                    SMGuardRender::refreshNames(data, transition->getId(), *tree);
                }
            }

            SMGuardNode* until = state->getDoUntil().getTree();
            if (until != nullptr)
            {
                SMGuardRender::refreshNames(data, 0u, *until);
            }

            if (state->hasNestedStates())
            {
                refreshGuardNames(data, *state->getNestedStates());
            }
        }
    }
}

StateMachineModel::StateMachineModel(QObject* parent /*= nullptr*/)
    : QObject        (parent)
    , mData          (std::make_unique<StateMachineData>())
    , mNotifier      (this)
    , mUndoStack     (this)
    , mAutosaveTimer (this)
    , mOverviewModel (*this)
    , mDataTypeModel (*this)
    , mAttributeModel(*this)
    , mEventModel    (*this)
    , mTimerModel    (*this)
    , mMethodModel   (*this)
    , mConstantModel (*this)
    , mIncludeModel  (*this)
    , mSelectionModel(this)
    , mOpenSuccess   (false)
    , mReadOnlyOrigin( )
    , mValidationController(*mData, mNotifier, this)
{
    mUndoStack.setUndoLimit(100);
    mAutosaveTimer.setSingleShot(false);
    mAutosaveTimer.setInterval(AutosaveIntervalMs);

    connect(&mAutosaveTimer, &QTimer::timeout, this, &StateMachineModel::onAutosaveTimeout);
    connect(&mUndoStack, &QUndoStack::cleanChanged, this, &StateMachineModel::onUndoCleanChanged);
}

bool StateMachineModel::createNewDocument(const QString& machineName)
{
    mData = StateMachineData::createNewDocument(machineName);
    mOpenSuccess = (mData != nullptr);
    if (mOpenSuccess == false)
    {
        return false;
    }

    mValidationController.setDocument(*mData);
    mUndoStack.clear();
    mUndoStack.setClean();
    mSelectionModel.reset();
    mNotifier.notifyDocumentReloaded();
    markDirty();
    updateAutosaveTimer();
    return true;
}

bool StateMachineModel::loadFromFile(const QString& documentPath, const QString& sourcePath /*= QString()*/)
{
    mOpenSuccess = false;
    if (documentPath.isEmpty())
    {
        return false;
    }

    const QString pathToRead = sourcePath.isEmpty() ? documentPath : sourcePath;
    std::unique_ptr<StateMachineData> loaded{ std::make_unique<StateMachineData>() };
    if (loaded->readFromFile(pathToRead) == false)
    {
        return false;
    }

    loaded->setFilePath(documentPath);
    convertLegacyGuards(loaded->getStates());
    convertLegacyDoUntil(*loaded, loaded->getStates());
    mData = std::move(loaded);
    mOpenSuccess = true;

    mValidationController.setDocument(*mData);
    mUndoStack.clear();
    mUndoStack.setClean();
    mSelectionModel.reset();
    mNotifier.notifyDocumentReloaded();
    if (sourcePath.isEmpty() == false)
    {
        markDirty();
    }

    updateAutosaveTimer();
    return true;
}

void StateMachineModel::setReadOnly(bool readOnly, const QString& origin /*= QString()*/)
{
    mReadOnlyOrigin = readOnly ? origin : QString();
    mUndoStack.setReadOnly(readOnly);
    if (readOnly)
    {
        // Nothing can change, so there is nothing to autosave and no history worth keeping.
        mAutosaveTimer.stop();
        mUndoStack.clear();
        mUndoStack.setClean();
    }

    mNotifier.notifyReadOnlyChanged(readOnly);
}

bool StateMachineModel::saveToFile(const QString& filePath /*= QString()*/)
{
    if ((mData == nullptr) || isReadOnly())
    {
        return false;
    }

    refreshGuardNames(*mData, mData->getStates());

    const QString previousPath = mData->getFilePath();
    if (mData->writeToFile(filePath) == false)
    {
        return false;
    }

    mUndoStack.setClean();
    StateMachineData::removeAutosave(previousPath);
    StateMachineData::removeAutosave(mData->getFilePath());
    updateAutosaveTimer();
    return true;
}

bool StateMachineModel::writeAutosave()
{
    if ((mData == nullptr) || isReadOnly() || mUndoStack.isClean())
    {
        return true;
    }

    const QString documentPath = mData->getFilePath();
    if (documentPath.isEmpty())
    {
        return true;
    }

    refreshGuardNames(*mData, mData->getStates());
    return mData->writeToAutosaveFile(StateMachineData::autosavePathForDocument(documentPath));
}

bool StateMachineModel::removeAutosave()
{
    return (mData != nullptr ? StateMachineData::removeAutosave(mData->getFilePath()) : true);
}

void StateMachineModel::publishStateNamePreview(uint32_t stateId, const QString& text)
{
    emit signalStateNamePreview(stateId, text);
}

void StateMachineModel::onAutosaveTimeout()
{
    writeAutosave();
}

void StateMachineModel::onUndoCleanChanged(bool clean)
{
    if (clean)
    {
        removeAutosave();
    }

    emit signalDirtyChanged(!clean);
    updateAutosaveTimer();
}

void StateMachineModel::markDirty()
{
    if (mUndoStack.isClean())
    {
        mUndoStack.push(new QUndoCommand(tr("Document modified")));
    }
}

void StateMachineModel::updateAutosaveTimer()
{
    const bool enableTimer = (mData != nullptr)
                           && (mUndoStack.isClean() == false)
                           && (mData->getFilePath().isEmpty() == false);
    if (enableTimer)
    {
        mAutosaveTimer.start();
    }
    else
    {
        mAutosaveTimer.stop();
    }
}
