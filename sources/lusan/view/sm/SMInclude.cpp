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
#include "lusan/view/common/IncludeListView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QAction>
#include <QCheckBox>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    constexpr int PageFile    { 0 };
    constexpr int PageMachine { 1 };

    //!< Refreshes a deprecated checkbox + hint pair from an entry's current state, without
    //!< re-triggering the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const IncludeEntry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }

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
    : QScrollArea       (parent)
    , mModel            (model)
    , mList             (new IncludeListView(IncludeTypeConfig{ QStringLiteral("fsml"), tr("State Machine"), tr("State Machines")
                                                                  , NELusanCommon::iconStateMachine(NELusanCommon::SizeSmall) }, this))
    , mDetailsStack     (nullptr)
    , mDetails          (new IncludeDetailsView(this))
    , mMachinePage      (nullptr)
    , mMachineAlias     (nullptr)
    , mMachineLocation  (nullptr)
    , mMachineBrowse    (nullptr)
    , mMachineDescription(nullptr)
    , mMachineStatus    (nullptr)
    , mMachineUpdate    (nullptr)
    , mTableCell        (nullptr)
    , mNameCounter      (0)
{
    buildUi();
    setupSignals();
    refreshAll();
}

QStringList SMInclude::getSupportedExtensions()
{
    // A State Machine is a behaviour, a Service Interface is an API contract, and neither can
    // consume the other -- so `.siml` is not offered here. A data type belongs to neither and is
    // shared by both, which is why `.dtml` is.
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::buildFileFilter(tr("State Machine Files")
                                                 , QStringList{ QStringLiteral("*.fsml"), QStringLiteral("*.dtml") }));
    return exts;
}

QStringList SMInclude::getMachineExtensions()
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

int SMInclude::getColumnCount() const
{
    return mList->ctrlTableList()->columnCount();
}

QString SMInclude::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->itemAt(cell);
    return (item != nullptr ? item->text(cell.column()) : QString());
}

void SMInclude::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Includes Editor ..."), content);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    QHBoxLayout* columns = new QHBoxLayout();

    mList->setParent(content);
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mList, 1);

    // One editor slot, two contents. A machine import and a header have almost nothing in common
    // beyond a path, so the fields swap rather than half of them greying out.
    mDetailsStack = new QStackedWidget(content);
    mDetails->setParent(mDetailsStack);
    mMachinePage = buildMachineDetails();
    mDetailsStack->insertWidget(PageFile, mDetails);
    mDetailsStack->insertWidget(PageMachine, mMachinePage);
    mDetailsStack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mDetailsStack, 1);

    root->addLayout(columns, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);

    // Inline editing of the Location column: commit once on editing-finished so a single undo
    // command is pushed (matching the details field). Type, Name and Version are derived and must
    // not be edited, so the delegate refuses an editor on every other column.
    QTreeWidget* table = mList->ctrlTableList();
    mTableCell = new TableCell(QList<QAbstractItemModel*>(), QList<int>(), table, this, true);
    mTableCell->setColumnValidation(static_cast<int>(IncludeListView::eColumn::ColLocation), TableCell::eCellValidation::Path);
    mTableCell->setEditableCheck([](const QModelIndex& index) {
        return index.column() == static_cast<int>(IncludeListView::eColumn::ColLocation);
    });
    table->setItemDelegate(mTableCell);
}

QWidget* SMInclude::buildMachineDetails()
{
    QWidget* page = new QWidget(mDetailsStack);
    QVBoxLayout* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Submachine Details:"), page);
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
    return page;
}

void SMInclude::setupSignals()
{
    connect(mList->ctrlTableList()     , &QTreeWidget::currentItemChanged, this, &SMInclude::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &SMInclude::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &SMInclude::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &SMInclude::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &SMInclude::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &SMInclude::onMoveDownClicked);
    connect(mList->ctrlButtonUpdate()  , &QToolButton::clicked           , this, &SMInclude::onUpdateClicked);

    // List keys, each doing what the matching toolbar button does: Delete removes the selected
    // include, Insert adds one, F2 puts the caret in its location. A group heading carries no
    // include, so Delete and F2 do nothing there.
    QTreeWidget* table = mList->ctrlTableList();
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scRemove, &QShortcut::activated, this, &SMInclude::onRemoveClicked);
    connect(scAdd   , &QShortcut::activated, this, &SMInclude::onAddClicked);
    connect(scRename, &QShortcut::activated, this, &SMInclude::focusLocationField);

    // The include path keystroke validator lives inside the shared IncludeDetailsView.
    connect(mDetails->ctrlInclude()      , &QLineEdit::editingFinished, this, &SMInclude::onLocationCommitted);
    // Live-preview the typed path into the selected include's row (location + derived type/name
    // columns); the change commits on editingFinished. Selection sets the field under a blocker.
    connect(mDetails->ctrlInclude()      , &QLineEdit::textChanged    , this, [this](const QString& text) {
        const uint32_t id = currentIncludeId();
        QTreeWidgetItem* item = (id != 0 ? mList->ctrlTableList()->currentItem() : nullptr);
        if (item == nullptr)
        {
            return;
        }

        // The heading a row sits under is part of what the location says, so the row moves while
        // it is typed rather than at commit -- otherwise it reads "Data Type" under "Include
        // Files" until focus leaves.
        const eIncludeKind kind = mList->kindForLocation(text);
        const QSignalBlocker blocker(mList->ctrlTableList());
        mList->placeRow(item, kind, id);
        mList->refreshGroupCounts();
        item->setIcon(static_cast<int>(IncludeListView::eColumn::ColLocation), mList->iconForKind(kind));
        item->setText(static_cast<int>(IncludeListView::eColumn::ColLocation), text);
        item->setText(static_cast<int>(IncludeListView::eColumn::ColType)    , mList->typeForLocation(text));
        item->setText(static_cast<int>(IncludeListView::eColumn::ColName)    , mList->nameForLocation(text));
        mList->ctrlTableList()->setCurrentItem(item);
    });
    connect(mDetails->ctrlBrowseButton() , &QPushButton::clicked      , this, &SMInclude::onBrowseClicked);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &SMInclude::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &SMInclude::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(mMachineAlias   , &QLineEdit::editingFinished, this, &SMInclude::onAliasCommitted);
    connect(mMachineLocation, &QLineEdit::editingFinished, this, &SMInclude::onMachineLocationCommitted);
    connect(mMachineBrowse  , &QPushButton::clicked      , this, &SMInclude::onMachineBrowseClicked);
    connect(mMachineUpdate  , &QPushButton::clicked      , this, &SMInclude::onUpdateVersionClicked);
    mMachineDescription->installEventFilter(this);

    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &SMInclude::onInlineLocationEdited);

    DocModelNotifier& notifier = mModel.getNotifier();

    // Both forms carry document text. Typing in them marks the document changed at once, even
    // though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mDetails, notifier);
    PendingEditWatcher::watchField(mMachinePage, notifier);

    auto onKind = [this](uint32_t, eDocElementKind kind) {
        if ((kind == eDocElementKind::Include) || (kind == eDocElementKind::Import))
        {
            onNotifierChanged();
        }
    };
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMInclude::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, onKind);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, onKind);
    connect(&notifier, &DocModelNotifier::elementChanged, this, onKind);
    connect(&notifier, &DocModelNotifier::listReordered , this, onKind);
}

void SMInclude::commitPendingEdits(void)
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    // The selected row decides which of the two forms is on top, and only that one holds the text
    // of the current entry.
    QPlainTextEdit* description = (mDetailsStack->currentIndex() == PageMachine)
                                        ? mMachineDescription
                                        : mDetails->ctrlDescription();
    mModel.setDescription(id, description->toPlainText());
}

bool SMInclude::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if ((watched == mDetails->ctrlDescription()) || (watched == mMachineDescription))
        {
            commitPendingEdits();
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

void SMInclude::setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const
{
    const QString location = entry.getLocation();
    const eIncludeKind kind = mList->kindForLocation(location);
    const bool machine = (kind == eIncludeKind::Document);
    node->setIcon(static_cast<int>(IncludeListView::eColumn::ColLocation), mList->iconForKind(kind));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColLocation), entry.getString(ElementBase::eDisplay::DisplayName));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColType)    , mList->typeForLocation(location));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColName)    , machine ? entry.getAlias() : mList->nameForLocation(location));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColVersion) , machine ? entry.getVersion().toString() : QString());
}

void SMInclude::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    IncludeEntry* entry = mModel.findInclude(IncludeListView::rowId(current));
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedInclude(entry);
}

void SMInclude::selectedInclude(const IncludeEntry* entry)
{
    const uint32_t id = entry->getId();
    const eIncludeKind kind = mList->kindForLocation(entry->getLocation());
    if (kind == eIncludeKind::Document)
    {
        mDetailsStack->setCurrentIndex(PageMachine);

        const QSignalBlocker blockAlias(mMachineAlias);
        const QSignalBlocker blockLocation(mMachineLocation);
        const QSignalBlocker blockDescr(mMachineDescription);
        mMachineAlias->setText(entry->getAlias());
        mMachineLocation->setText(entry->getLocation());
        mMachineDescription->setPlainText(entry->getDescription());

        const SMImportResolver::Resolution resolution = mModel.resolutionOf(id);
        QString status;
        switch (resolution.state)
        {
        case SMImportResolver::eState::NoLocation:
            status = tr("No file is selected.");
            break;

        case SMImportResolver::eState::NotFound:
            status = tr("The file was not found: %1").arg(entry->getLocation());
            break;

        case SMImportResolver::eState::ParseFailed:
            status = tr("The file cannot be read as a state machine: %1").arg(entry->getLocation());
            break;

        case SMImportResolver::eState::Resolved:
            status = (resolution.actualVersion == entry->getVersion())
                     ? tr("Up to date, version %1.").arg(entry->getVersion().toString())
                     : tr("Pinned to version %1, the file is now %2. Use Update to accept it.")
                        .arg(entry->getVersion().toString(), resolution.actualVersion.toString());
            break;
        }

        mMachineStatus->setText(status);
        const bool writable = (mModel.isReadOnly() == false);
        mMachineAlias->setEnabled(writable);
        mMachineLocation->setEnabled(writable);
        mMachineBrowse->setEnabled(writable);
        mMachineDescription->setEnabled(writable);
        mMachineUpdate->setEnabled(writable && resolution.isResolved());
    }
    else
    {
        mDetailsStack->setCurrentIndex(PageFile);

        {
            const QSignalBlocker blockInclude(mDetails->ctrlInclude());
            const QSignalBlocker blockDescr(mDetails->ctrlDescription());
            mDetails->ctrlInclude()->setText(entry->getLocation());
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        }
        applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), entry);

        // A data type document is not a header, so there is nothing to deprecate on it.
        mDetails->setDeprecationVisible(kind == eIncludeKind::Source);
        mDetails->ctrlInclude()->setEnabled(true);
        mDetails->ctrlBrowseButton()->setEnabled(true);
        mDetails->ctrlDescription()->setEnabled(true);
        mDetails->ctrlDeprecated()->setEnabled(kind == eIncludeKind::Source);
    }

    mList->ctrlButtonRemove()->setEnabled(true);
    updateMoveButtons(mModel.findIndex(id), mModel.getIncludeCount());
}

void SMInclude::showClean()
{
    mDetailsStack->setCurrentIndex(PageFile);
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);

    const QSignalBlocker blockInclude(mDetails->ctrlInclude());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlInclude()->clear();
    mDetails->ctrlDescription()->clear();

    mDetails->setDeprecationVisible(true);
    mDetails->ctrlInclude()->setEnabled(false);
    mDetails->ctrlBrowseButton()->setEnabled(false);
    mDetails->ctrlDescription()->setEnabled(false);
    mDetails->ctrlDeprecated()->setEnabled(false);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

void SMInclude::updateMoveButtons(int row, int rowCount)
{
    if ((row < 0) || (row >= rowCount))
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        return;
    }

    mList->ctrlButtonMoveUp()->setEnabled(row > 0);
    mList->ctrlButtonMoveDown()->setEnabled(row < (rowCount - 1));
}

uint32_t SMInclude::currentIncludeId() const
{
    return IncludeListView::rowId(mList->ctrlTableList()->currentItem());
}

bool SMInclude::revealElement(uint32_t id)
{
    QTreeWidgetItem* item = mList->findRow(id);
    if (item == nullptr)
    {
        return false;
    }

    if (item->parent() != nullptr)
    {
        item->parent()->setExpanded(true);
    }

    mList->ctrlTableList()->setCurrentItem(item);
    mList->ctrlTableList()->scrollToItem(item);
    return true;
}

QString SMInclude::genName()
{
    static const QString _defName("NewInclude");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findInclude(name) != nullptr);

    return name;
}

bool SMInclude::acceptTypedLocation(const QString& location)
{
    if (mList->kindForLocation(location) != eIncludeKind::Document)
    {
        return true;
    }

    // Only a location that actually names a file can be checked: an empty or not-yet-created
    // path cannot close a cycle or sit too deep, and refusing it would fight a user who is
    // typing the name of a file they are about to make. Validation reports it either way.
    const QString absolute = mModel.absolutePathOf(location);
    if (absolute.isEmpty() || (QFileInfo(absolute).isFile() == false))
    {
        return true;
    }

    return acceptMachine(mModel, absolute, this);
}

void SMInclude::onAddClicked()
{
    // A new entry starts as a plain include file: its group follows its location, and it has
    // none yet. Picking or typing one moves the row.
    IncludeEntry* entry = mModel.createInclude(genName());
    if (entry != nullptr)
    {
        selectInclude(entry->getId());
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
    }
}

void SMInclude::onInsertClicked()
{
    const uint32_t id = currentIncludeId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    IncludeEntry* entry = mModel.insertInclude(position < 0 ? 0 : position, genName());
    if (entry != nullptr)
    {
        selectInclude(entry->getId());
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
    }
}

void SMInclude::focusLocationField()
{
    if (currentIncludeId() != 0)
    {
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
    }
}

void SMInclude::onRemoveClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    // Removing a hosted machine breaks its states visibly: each keeps its alias and is reported
    // as an unresolved reference, which is easier to act on than a silently emptied state.
    const IncludeEntry* entry = mModel.findInclude(id);
    if ((entry != nullptr) && (mList->kindForLocation(entry->getLocation()) == eIncludeKind::Document))
    {
        const QList<SMReferences::Use> uses = mModel.whereUsed(id);
        if (uses.isEmpty() == false)
        {
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
            if (choice != QMessageBox::Yes)
            {
                return;
            }
        }
    }

    const QList<IncludeEntry>& list = mModel.getIncludes();
    const int index = mModel.findIndex(id);
    uint32_t neighborId = 0;
    if (list.size() > 1)
    {
        const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
        neighborId = list.at(neighborIndex).getId();
    }

    mModel.deleteInclude(id);
    if (neighborId != 0)
    {
        selectInclude(neighborId);
    }
}

void SMInclude::onMoveUpClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if (index > 0)
    {
        // The swap keeps IDs attached to list position, so the moved entry's new ID is the
        // neighbor's old ID -- reselect that to keep the moved row highlighted.
        const uint32_t neighborId = mModel.getIncludes().at(index - 1).getId();
        mModel.swapIncludes(id, neighborId);
        selectInclude(neighborId);
    }
}

void SMInclude::onMoveDownClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    const int index = mModel.findIndex(id);
    if ((index >= 0) && (index < (mModel.getIncludeCount() - 1)))
    {
        const uint32_t neighborId = mModel.getIncludes().at(index + 1).getId();
        mModel.swapIncludes(id, neighborId);
        selectInclude(neighborId);
    }
}

void SMInclude::onLocationCommitted()
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        // Skip a no-op command: after an Escape-cancel the field already holds the model value, so
        // a spurious editingFinished must not push an empty undo step.
        const IncludeEntry* entry = mModel.findInclude(id);
        const QString text = mDetails->ctrlInclude()->text();
        if ((entry != nullptr) && (entry->getLocation() != text) && acceptTypedLocation(text))
        {
            mModel.setLocation(id, text);
        }
    }
}

void SMInclude::onInlineLocationEdited(const QModelIndex& index, const QString& newValue)
{
    if (index.column() != static_cast<int>(IncludeListView::eColumn::ColLocation))
        return;

    const uint32_t id = IncludeListView::rowId(mList->itemAt(index));
    const IncludeEntry* entry = mModel.findInclude(id);
    if ((entry != nullptr) && (entry->getLocation() != newValue) && acceptTypedLocation(newValue))
    {
        // The undo command fires an include notifier, which rebuilds the list with the re-derived
        // Type/Name columns and moves the row if its kind changed.
        mModel.setLocation(id, newValue);
    }
}

void SMInclude::onBrowseClicked()
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , SMInclude::getSupportedExtensions()
                                , tr("Select Include File")
                                , this);
    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        const QString absolute = dialog.getSelectedFilePath();
        if (mList->kindForLocation(absolute) == eIncludeKind::Document)
        {
            // Changing a path to a `.fsml` is a registration too, so it passes the same add-time
            // checks -- otherwise the safe route is only the slow one.
            if (acceptMachine(mModel, absolute, this) == false)
            {
                return;
            }

            mModel.setLocation(id, mModel.storableLocation(absolute));
        }
        else
        {
            mModel.setLocation(id, dialog.getSelectedFileRelativePath());
        }
    }
}

void SMInclude::onDeprecatedToggled(bool checked)
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    mModel.setDeprecated(id, checked);
    IncludeEntry* entry = mModel.findInclude(id);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void SMInclude::onDeprecateHintCommitted()
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void SMInclude::onAliasCommitted()
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

void SMInclude::onMachineLocationCommitted()
{
    const uint32_t id = currentIncludeId();
    const IncludeEntry* entry = mModel.findInclude(id);
    const QString text = mMachineLocation->text();
    if ((entry != nullptr) && (entry->getLocation() != text) && acceptTypedLocation(text))
    {
        mModel.setLocation(id, text);
    }
}

void SMInclude::onMachineBrowseClicked()
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

void SMInclude::onUpdateVersionClicked()
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

void SMInclude::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();
    const uint32_t selId = currentIncludeId();

    {
        const QSignalBlocker blocker(table);
        mList->clearRows();
        int counts[3]{ 0, 0, 0 };
        for (const IncludeEntry& entry : mModel.getIncludes())
        {
            const eIncludeKind kind = mList->kindForLocation(entry.getLocation());
            QTreeWidgetItem* item = mList->placeRow(nullptr, kind, entry.getId());
            setNodeText(item, entry);
            ++counts[static_cast<int>(kind)];
        }

        mList->updateGroupCounts(counts[static_cast<int>(eIncludeKind::Source)]
                                 , counts[static_cast<int>(eIncludeKind::DataType)]
                                 , counts[static_cast<int>(eIncludeKind::Document)]);
    }

    if ((selId == 0) || (selectInclude(selId) == false))
    {
        showClean();
    }
}

bool SMInclude::selectInclude(uint32_t id)
{
    return revealElement(id);
}

void SMInclude::onUpdateClicked()
{
    // Re-derives the Type/Name/Version columns and the resolution status from the current
    // locations: the imported files live outside the document and change under the editor.
    refreshAll();
}

void SMInclude::onNotifierChanged()
{
    refreshAll();
}
