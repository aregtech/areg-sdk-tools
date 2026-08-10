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
 *  \file        lusan/view/sm/SMInclude.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMInclude.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMIncludeModel.hpp"
#include "lusan/view/common/IncludeDetailsView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QEvent>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
    //!< The message shown when an add-time check refuses a candidate machine.
    QString refusalText(SMIncludeModel::eImportRefusal refusal, const QString& file, const QStringList& chain)
    {
        const QString name = QFileInfo(file).fileName();
        switch (refusal)
        {
        case SMIncludeModel::eImportRefusal::HostNotSaved:
            return QObject::tr("Save this machine before importing another -- import locations are stored relative to its own folder.");

        case SMIncludeModel::eImportRefusal::SelfImport:
            return QObject::tr("'%1' cannot be imported: a machine cannot import itself.").arg(name);

        case SMIncludeModel::eImportRefusal::Cycle:
            return QObject::tr("'%1' cannot be imported: it would create a cycle.\n%2")
                    .arg(name, chain.join(QStringLiteral(" -> ")));

        case SMIncludeModel::eImportRefusal::TooDeep:
            return QObject::tr("'%1' cannot be imported: its own imports already nest %2 levels deep.\n%3")
                    .arg(name).arg(SMImportResolver::MAX_IMPORT_DEPTH).arg(chain.join(QStringLiteral(" -> ")));

        default:
            return QString();
        }
    }
}

SMInclude::SMInclude(SMIncludeModel& model, QWidget* parent /*= nullptr*/)
    // A state machine is a behaviour and a service interface is an API contract, so `.siml` is
    // not offered here. A data type belongs to neither and is shared, which is why `.dtml` is.
    : IncludePage       (model
                        , IncludeTypeConfig{ QStringLiteral("fsml"), tr("State Machine"), tr("State Machines")
                                           , NELusanCommon::iconStateMachine(NELusanCommon::SizeSmall) }
                        , tr("State Machine Includes Editor ...")
                        , parent)
    , mModel            (model)
    , mMachinePage      (nullptr)
    , mMachineAlias     (nullptr)
    , mMachineLocation  (nullptr)
    , mMachineBrowse    (nullptr)
    , mMachineDescription(nullptr)
    , mMachineStatus    (nullptr)
    , mMachineUpdate    (nullptr)
{
    buildMachineDetails();
    // The base page laid its list out before the machine editor existed, so the rows are placed
    // again now that a `.fsml` selection has somewhere to go.
    refreshAll();
}

QStringList SMInclude::getMachineExtensions(void)
{
    return QStringList{ LusanApplication::buildFileFilter(tr("State Machines"), QStringList{ QStringLiteral("*.fsml") }) };
}

bool SMInclude::acceptMachine(SMIncludeModel& model, const QString& absoluteFilePath, QWidget* parent)
{
    QStringList chain;
    const SMIncludeModel::eImportRefusal refusal = model.canImport(absoluteFilePath, chain);
    if ((refusal != SMIncludeModel::eImportRefusal::None) && (refusal != SMIncludeModel::eImportRefusal::Unreadable))
    {
        QMessageBox::warning(parent, QObject::tr("Cannot Import"), refusalText(refusal, absoluteFilePath, chain));
        return false;
    }

    if (refusal == SMIncludeModel::eImportRefusal::Unreadable)
    {
        QMessageBox::warning(parent, QObject::tr("Import Added")
                            , QObject::tr("'%1' cannot be read as a state machine. It is registered anyway and is flagged by validation.")
                              .arg(QFileInfo(absoluteFilePath).fileName()));
    }

    return true;
}

QString SMInclude::browseForMachine(SMIncludeModel& model, QWidget* parent)
{
    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , SMInclude::getMachineExtensions()
                                , QObject::tr("Select State Machine to Import")
                                , parent);
    dialog.clearHistory();
    if (dialog.exec() != static_cast<int>(QDialog::DialogCode::Accepted))
    {
        return QString();
    }

    const QString picked = dialog.getSelectedFilePath();
    if (picked.isEmpty() || (acceptMachine(model, picked, parent) == false))
    {
        return QString();
    }

    return picked;
}

void SMInclude::buildMachineDetails(void)
{
    QStackedWidget* stack = getDetailsStack();
    mMachinePage = new QWidget(stack);
    QVBoxLayout* root = new QVBoxLayout(mMachinePage);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Submachine Details:"), mMachinePage);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    QFormLayout* form = new QFormLayout();

    mMachineAlias = new QLineEdit(group);
    mMachineAlias->setMaxLength(StateMachineData::MAX_IDENTIFIER_LENGTH);
    // The alias becomes part of a generated state identity, so it obeys the same identifier rule
    // as every other declared name.
    mMachineAlias->setValidator(new QRegularExpressionValidator(QRegularExpression(StateMachineData::identifierPattern()), mMachineAlias));

    QWidget* locationRow = new QWidget(group);
    QHBoxLayout* locationLayout = new QHBoxLayout(locationRow);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    mMachineLocation = new QLineEdit(locationRow);
    mMachineBrowse = new QPushButton(tr("Browse..."), locationRow);
    locationLayout->addWidget(mMachineLocation, 1);
    locationLayout->addWidget(mMachineBrowse);

    mMachineDescription = new QPlainTextEdit(group);
    mMachineDescription->setPlaceholderText(tr("Description"));
    mMachineDescription->setMaximumHeight(90);

    mMachineStatus = new QLabel(group);
    mMachineStatus->setWordWrap(true);

    QWidget* statusRow = new QWidget(group);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusRow);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    mMachineUpdate = new QPushButton(tr("Update"), statusRow);
    mMachineUpdate->setToolTip(tr("Pin the version currently in the imported file"));
    statusLayout->addWidget(mMachineStatus, 1);
    statusLayout->addWidget(mMachineUpdate, 0, Qt::AlignTop);

    form->addRow(tr("Machine:"), mMachineAlias);
    form->addRow(tr("Location:"), locationRow);
    form->addRow(tr("Description:"), mMachineDescription);
    form->addRow(tr("Status:"), statusRow);
    groupLayout->addLayout(form);
    groupLayout->addStretch(1);
    root->addWidget(group);
    stack->addWidget(mMachinePage);

    connect(mMachineAlias   , &QLineEdit::editingFinished, this, &SMInclude::onAliasCommitted);
    connect(mMachineLocation, &QLineEdit::editingFinished, this, &SMInclude::onMachineLocationCommitted);
    connect(mMachineBrowse  , &QPushButton::clicked      , this, &SMInclude::onMachineBrowseClicked);
    connect(mMachineUpdate  , &QPushButton::clicked      , this, &SMInclude::onUpdateVersionClicked);
    mMachineDescription->installEventFilter(this);

    // The form carries document text, so typing in it marks the document changed at once, even
    // though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mMachinePage, mModel.getNotifier());
}

void SMInclude::commitPendingEdits(void)
{
    // The selected row decides which of the two forms is on top, and only that one holds the text
    // of the current entry.
    if (getDetailsStack()->currentWidget() != mMachinePage)
    {
        IncludePage::commitPendingEdits();
        return;
    }

    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setDescription(id, mMachineDescription->toPlainText());
    }
}

bool SMInclude::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::FocusOut) && (watched == mMachineDescription))
    {
        commitPendingEdits();
    }

    return IncludePage::eventFilter(watched, event);
}

QString SMInclude::machineStatusText(const IncludeEntry& entry) const
{
    const SMImportResolver::Resolution resolution = mModel.resolutionOf(entry.getId());
    switch (resolution.state)
    {
    case SMImportResolver::eState::NoLocation:
        return tr("No file is selected.");

    case SMImportResolver::eState::NotFound:
        return tr("The file was not found: %1").arg(entry.getLocation());

    case SMImportResolver::eState::ParseFailed:
        return tr("The file cannot be read as a state machine: %1").arg(entry.getLocation());

    default:
        return (resolution.actualVersion == entry.getVersion())
               ? tr("Up to date, version %1.").arg(entry.getVersion().toString())
               : tr("Pinned to version %1, the file is now %2. Use Update to accept it.")
                  .arg(entry.getVersion().toString(), resolution.actualVersion.toString());
    }
}

void SMInclude::selectedInclude(const IncludeEntry* entry)
{
    if (getList()->kindForLocation(entry->getLocation()) != eIncludeKind::Document)
    {
        IncludePage::selectedInclude(entry);
        return;
    }

    getDetailsStack()->setCurrentWidget(mMachinePage);

    const QSignalBlocker blockAlias(mMachineAlias);
    const QSignalBlocker blockLocation(mMachineLocation);
    const QSignalBlocker blockDescr(mMachineDescription);
    mMachineAlias->setText(entry->getAlias());
    mMachineLocation->setText(entry->getLocation());
    mMachineDescription->setPlainText(entry->getDescription());
    mMachineStatus->setText(machineStatusText(*entry));

    const bool writable = (mModel.isReadOnly() == false);
    mMachineAlias->setEnabled(writable);
    mMachineLocation->setEnabled(writable);
    mMachineBrowse->setEnabled(writable);
    mMachineDescription->setEnabled(writable);
    mMachineUpdate->setEnabled(writable && mModel.resolutionOf(entry->getId()).isResolved());

    getList()->ctrlButtonRemove()->setEnabled(writable);
    updateMoveButtons(mModel.findIndex(entry->getId()), mModel.getIncludeCount());
}

bool SMInclude::confirmRemove(uint32_t id)
{
    // Removing a hosted machine breaks its states visibly: each keeps its alias and is reported
    // as an unresolved reference, which is easier to act on than a silently emptied state.
    const IncludeEntry* entry = mModel.findInclude(id);
    if ((entry == nullptr) || (getList()->kindForLocation(entry->getLocation()) != eIncludeKind::Document))
    {
        return true;
    }

    const QList<SMReferences::Use> uses = mModel.whereUsed(id);
    if (uses.isEmpty())
    {
        return true;
    }

    QStringList places;
    for (const SMReferences::Use& use : uses)
    {
        places.append(QStringLiteral("  - ") + use.location);
    }

    const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Submachine is used")
                        , tr("'%1' is hosted by %2 state%3:\n%4\n\nRemove anyway? Those states lose their machine and are listed by validation.")
                          .arg(entry->getAlias())
                          .arg(uses.size())
                          .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                          .arg(places.join(QLatin1Char('\n')))
                        , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    return (choice == QMessageBox::Yes);
}

bool SMInclude::acceptLocation(const QString& location)
{
    if (getList()->kindForLocation(location) != eIncludeKind::Document)
    {
        return true;
    }

    // Only a location that names a file can be checked: an empty or not-yet-created path cannot
    // close a cycle, and refusing it would fight a user typing the name of a file still to come.
    const QString absolute = mModel.absolutePathOf(location);
    if (absolute.isEmpty() || (QFileInfo(absolute).isFile() == false))
    {
        return true;
    }

    return acceptMachine(mModel, absolute, this);
}

void SMInclude::commitBrowsedLocation(uint32_t id, const QString& absolutePath, const QString& relativePath)
{
    if (getList()->kindForLocation(absolutePath) != eIncludeKind::Document)
    {
        IncludePage::commitBrowsedLocation(id, absolutePath, relativePath);
        return;
    }

    // Changing a path to a `.fsml` is a registration too, so it passes the same add-time checks
    // -- otherwise the safe route is only the slow one.
    if (acceptMachine(mModel, absolutePath, this))
    {
        mModel.setLocation(id, mModel.storableLocation(absolutePath));
    }
}

void SMInclude::onAliasCommitted(void)
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setAlias(id, mMachineAlias->text());
        IncludeEntry* entry = mModel.findInclude(id);
        if (entry != nullptr)
        {
            selectedInclude(entry);
        }
    }
}

void SMInclude::onMachineLocationCommitted(void)
{
    const uint32_t id = currentIncludeId();
    const IncludeEntry* entry = mModel.findInclude(id);
    const QString text = mMachineLocation->text();
    if ((entry != nullptr) && (entry->getLocation() != text) && acceptLocation(text))
    {
        mModel.setLocation(id, text);
    }
}

void SMInclude::onMachineBrowseClicked(void)
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    const QString picked = browseForMachine(mModel, this);
    if (picked.isEmpty() == false)
    {
        mModel.setLocation(id, mModel.storableLocation(picked));
    }
}

void SMInclude::onUpdateVersionClicked(void)
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    if (mModel.updateVersion(id) == false)
    {
        // Nothing to re-pin: say so rather than leaving the click unanswered.
        QMessageBox::information(this, tr("Import Version")
                                , tr("The pinned version already matches the imported file."));
    }
}
