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

#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMImportResolver.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMRenameCommands.hpp"
#include "lusan/model/sm/SMValidator.hpp"

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
     * \brief   Refreshes the advisory `name` on every guard tree before a save, so the names
     *          written to the file follow renames. The name is never read back; the id binds.
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
    , mValidationController(*this, [this]() { return SMValidator::validate(getData()); }, this)
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

    mValidationController.validateNow();
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
    mData = std::move(loaded);
    mOpenSuccess = true;

    // A pin that only drifted at the PATCH level is corrected silently: nothing an import
    // provides can change at that level, so there is nothing for the author to decide. MAJOR
    // and MINOR drift are untouched and stay findings. The one-shot Info the correction leaves
    // behind (SMValidator, checkImports) tells the author it happened, once.
    bool anyPatchAutoFixed = false;
    for (IncludeEntry& entry : mData->getIncludes().getElements())
    {
        if (includeKindOf(entry.getLocation(), QStringLiteral("fsml")) != eIncludeKind::Document)
        {
            continue;
        }

        const SMImportResolver::Resolution resolution = SMImportResolver::resolve(*mData, entry);
        if (resolution.isResolved() == false)
        {
            continue;
        }

        const VersionNumber& pinned = entry.getVersion();
        const VersionNumber& actual = resolution.actualVersion;
        if ((pinned.getMajor() == actual.getMajor()) && (pinned.getMinor() == actual.getMinor())
            && (pinned.getPatch() != actual.getPatch()))
        {
            entry.setVersion(actual);
            entry.markVersionPatchAutoFixed();
            anyPatchAutoFixed = true;
        }
    }

    mValidationController.validateNow();
    mUndoStack.clear();
    mUndoStack.setClean();
    mSelectionModel.reset();
    mNotifier.notifyDocumentReloaded();
    // A renumbered element only reaches the file on the next save, so the document is dirty.
    if ((sourcePath.isEmpty() == false) || anyPatchAutoFixed || (mData->getRepairedIds().isEmpty() == false))
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

void StateMachineModel::refreshTypeReferences()
{
    if (mData == nullptr)
    {
        return;
    }

    DataTypeDataSection& types = mData->getDataTypes();
    types.refreshTypeReferences();

    // Dropped before being looked up again: validate() only fills an empty slot, so a reference
    // to a type that has been replaced would otherwise survive as it is.
    for (AttributeEntry& entry : mData->getAttributes().getElements())
    {
        entry.invalidate();
    }

    for (ConstantEntry& entry : mData->getConstants().getElements())
    {
        entry.invalidate();
    }

    for (SMEventEntry* event : mData->getEvents().getElements())
    {
        if (event != nullptr)
        {
            event->invalidate();
        }
    }

    mData->getAttributes().validate(types);
    mData->getConstants().validate(types.getResolutionTypes());
    for (SMEventEntry* event : mData->getEvents().getElements())
    {
        if (event != nullptr)
        {
            event->validate(types.getResolutionTypes());
        }
    }
}

QString StateMachineModel::getDocumentPath() const
{
    return (mData != nullptr ? mData->getFilePath() : QString());
}

const QList<DataTypeCustom*>& StateMachineModel::getCustomDataTypes() const
{
    return const_cast<StateMachineModel*>(this)->mDataTypeModel.getDataTypeData().getResolutionTypes();
}

QString StateMachineModel::describeElement(uint32_t id, eDocElementKind /*kind*/) const
{
    if ((mData == nullptr) || (id == 0))
    {
        return QString();
    }

    if (const SMStateEntry* state = mData->findStateById(id))
    {
        return tr("State '%1'").arg(state->getName());
    }

    if (const SMTransitionEntry* transition = mData->findTransitionById(id))
    {
        const SMStateEntry* target = mData->findStateById(transition->getToId());
        const QString stimulus = transition->getStimulus().isEmpty() ? tr("(initial)") : transition->getStimulus();
        return (target != nullptr)
                ? tr("Transition %1 -> %2").arg(stimulus, target->getName())
                : tr("Transition %1").arg(stimulus);
    }

    return QString();
}

QUndoCommand* StateMachineModel::createRenameSideEffects( eDocElementKind kind, uint32_t id
                                                        , const QString& oldName, const QString& newName
                                                        , QUndoCommand* parent)
{
    SMReferences::eTarget target{ SMReferences::eTarget::Constant };
    switch (kind)
    {
    case eDocElementKind::Constant:
        target = SMReferences::eTarget::Constant;
        break;

    case eDocElementKind::Attribute:
        target = SMReferences::eTarget::Attribute;
        break;

    case eDocElementKind::Event:
        target = SMReferences::eTarget::Event;
        break;

    case eDocElementKind::Timer:
        target = SMReferences::eTarget::Timer;
        break;

    case eDocElementKind::Method:
    {
        // Which references a method has depends on what kind it was declared as: a trigger is a
        // stimulus, an action is called from an operation, a condition is named by a guard.
        const MethodEntry* method = getData().getMethods().findMethod(id);
        if (method == nullptr)
            return nullptr;

        target = NESMMethod::isAction(method)      ? SMReferences::eTarget::Action
               : NESMMethod::isCondition(method)   ? SMReferences::eTarget::Condition
                                                   : SMReferences::eTarget::Trigger;
        break;
    }

    default:
        // The remaining kinds either carry no name-based reference or are renamed through their
        // own command, which already knows the target.
        return nullptr;
    }

    return new SMRewriteReferencesCommand(getData(), getNotifier(), target, id, oldName, newName, parent->text(), parent);
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
