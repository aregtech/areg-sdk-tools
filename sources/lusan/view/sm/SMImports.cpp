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
 *  \file        lusan/view/sm/SMImports.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM imported state machines section.
 *
 ************************************************************************/

#include "lusan/view/sm/SMImports.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMImportModel.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QEvent>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
    constexpr int ImportIdRole { Qt::UserRole + 1 };

    uint32_t rowId(const QTreeWidgetItem* item)
    {
        return (item != nullptr ? item->data(0, ImportIdRole).toUInt() : 0u);
    }
}

SMImports::SMImports(SMImportModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mTable        (nullptr)
    , mButtonAdd    (nullptr)
    , mButtonRemove (nullptr)
    , mButtonMoveUp (nullptr)
    , mButtonMoveDown(nullptr)
    , mButtonUpdate (nullptr)
    , mAlias        (nullptr)
    , mLocation     (nullptr)
    , mBrowse       (nullptr)
    , mDescription  (nullptr)
    , mStatus       (nullptr)
{
    buildUi();
    setupSignals();
    refreshAll();
}

void SMImports::buildUi()
{
    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* listGroup = new QGroupBox(tr("Imported State Machines:"), this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);

    QWidget* toolbar = new QWidget(listGroup);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setSpacing(5);
    toolbarLayout->setContentsMargins(2, 2, 2, 2);

    mButtonAdd    = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry add")   , tr("Register another state machine document"), QKeySequence());
    mButtonRemove = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/entry delete"), tr("Remove the selected import"), QKeySequence());

    QFrame* sep = new QFrame(toolbar);
    sep->setFrameShape(QFrame::VLine);
    sep->setMaximumSize(24, 24);

    mButtonMoveUp   = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move up")  , tr("Move selection up."), QKeySequence());
    mButtonMoveDown = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/move down"), tr("Move selection down."), QKeySequence());

    QFrame* sepUpdate = new QFrame(toolbar);
    sepUpdate->setFrameShape(QFrame::VLine);
    sepUpdate->setMaximumSize(24, 24);

    mButtonUpdate = NELusanCommon::createToolButton(toolbar, QStringLiteral(":/icons/Update Item"), tr("Pin the version currently in the imported file"), QKeySequence());

    toolbarLayout->addWidget(mButtonAdd);
    toolbarLayout->addWidget(mButtonRemove);
    toolbarLayout->addWidget(sep);
    toolbarLayout->addWidget(mButtonMoveUp);
    toolbarLayout->addWidget(mButtonMoveDown);
    toolbarLayout->addWidget(sepUpdate);
    toolbarLayout->addWidget(mButtonUpdate);
    toolbarLayout->addStretch(1);

    mTable = new QTreeWidget(listGroup);
    mTable->setCursor(Qt::PointingHandCursor);
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTable->setRootIsDecorated(false);
    mTable->setSortingEnabled(false);
    mTable->setColumnCount(4);
    mTable->setHeaderLabels(QStringList{ tr("Machine:"), tr("Location:"), tr("Pinned:"), tr("Actual:") });
    mTable->header()->setStretchLastSection(false);
    mTable->header()->setMinimumSectionSize(50);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColAlias)   , QHeaderView::ResizeToContents);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColLocation), QHeaderView::Stretch);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColPinned)  , QHeaderView::ResizeToContents);
    mTable->header()->setSectionResizeMode(static_cast<int>(eColumn::ColActual)  , QHeaderView::ResizeToContents);

    listLayout->addWidget(toolbar);
    listLayout->addWidget(mTable);

    QGroupBox* detailsGroup = new QGroupBox(tr("Import Details:"), this);
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsGroup);
    QFormLayout* form = new QFormLayout();

    mAlias = new QLineEdit(detailsGroup);
    mAlias->setMaxLength(StateMachineData::MAX_IDENTIFIER_LENGTH);
    // The alias becomes part of a generated state identity, so it obeys the same identifier
    // rule as every other declared name.
    mAlias->setValidator(new QRegularExpressionValidator(QRegularExpression(StateMachineData::identifierPattern()), mAlias));

    QWidget* locationRow = new QWidget(detailsGroup);
    QHBoxLayout* locationLayout = new QHBoxLayout(locationRow);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    mLocation = new QLineEdit(locationRow);
    mBrowse = new QPushButton(tr("Browse..."), locationRow);
    locationLayout->addWidget(mLocation, 1);
    locationLayout->addWidget(mBrowse);

    mDescription = new QPlainTextEdit(detailsGroup);
    mDescription->setPlaceholderText(tr("Description"));
    mDescription->setMaximumHeight(90);

    mStatus = new QLabel(detailsGroup);
    mStatus->setWordWrap(true);

    form->addRow(tr("Machine:"), mAlias);
    form->addRow(tr("Location:"), locationRow);
    form->addRow(tr("Description:"), mDescription);
    form->addRow(tr("Status:"), mStatus);
    detailsLayout->addLayout(form);
    detailsLayout->addStretch(1);

    // Equal-width halves: both panels ignore their content's width so neither can push the
    // other, and the page keeps the 50/50 split the sibling editor pages use.
    listGroup->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    detailsGroup->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    root->addWidget(listGroup, 1);
    root->addWidget(detailsGroup, 1);
}

void SMImports::setupSignals()
{
    connect(mTable, &QTreeWidget::currentItemChanged, this, &SMImports::onSelectionChanged);
    connect(mButtonAdd     , &QToolButton::clicked, this, &SMImports::onAddClicked);
    connect(mButtonRemove  , &QToolButton::clicked, this, &SMImports::onRemoveClicked);
    connect(mButtonMoveUp  , &QToolButton::clicked, this, &SMImports::onMoveUpClicked);
    connect(mButtonMoveDown, &QToolButton::clicked, this, &SMImports::onMoveDownClicked);
    connect(mButtonUpdate  , &QToolButton::clicked, this, &SMImports::onUpdateClicked);
    connect(mBrowse        , &QPushButton::clicked, this, &SMImports::onBrowseClicked);
    connect(mAlias   , &QLineEdit::editingFinished, this, &SMImports::onAliasCommitted);
    connect(mLocation, &QLineEdit::editingFinished, this, &SMImports::onLocationCommitted);
    mDescription->installEventFilter(this);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMImports::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Import) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Import) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Import) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Import) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::readOnlyChanged, this, [this](bool) { updateButtons(); });
}

bool SMImports::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDescription) && (event->type() == QEvent::FocusOut))
    {
        const uint32_t id = currentImportId();
        if (id != 0)
        {
            mModel.setDescription(id, mDescription->toPlainText());
        }
    }

    return QWidget::eventFilter(watched, event);
}

uint32_t SMImports::currentImportId() const
{
    return rowId(mTable->currentItem());
}

bool SMImports::revealElement(uint32_t id)
{
    for (int i = 0; i < mTable->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = mTable->topLevelItem(i);
        if (rowId(item) == id)
        {
            mTable->setCurrentItem(item);
            mTable->scrollToItem(item);
            return true;
        }
    }

    return false;
}

void SMImports::refreshAll()
{
    const uint32_t selected = currentImportId();

    {
        const QSignalBlocker blocker(mTable);
        mTable->clear();
        for (const SMImportEntry& entry : mModel.getImports())
        {
            QTreeWidgetItem* node = new QTreeWidgetItem(mTable);
            node->setData(0, ImportIdRole, entry.getId());
            fillRow(node, entry.getId());
        }
    }

    if ((selected == 0) || (revealElement(selected) == false))
    {
        if (mTable->topLevelItemCount() > 0)
        {
            mTable->setCurrentItem(mTable->topLevelItem(0));
        }
        else
        {
            showClean();
        }
    }
    else
    {
        showDetails(selected);
    }

    updateButtons();
}

void SMImports::fillRow(QTreeWidgetItem* node, uint32_t id)
{
    const SMImportEntry* entry = mModel.findImport(id);
    if ((node == nullptr) || (entry == nullptr))
    {
        return;
    }

    const SMImportResolver::Resolution resolution = mModel.resolutionOf(id);
    QString actual;
    switch (resolution.state)
    {
    case SMImportResolver::eState::Resolved:    actual = resolution.actualVersion.toString();   break;
    case SMImportResolver::eState::NoLocation:  actual = tr("no file");                         break;
    case SMImportResolver::eState::NotFound:    actual = tr("not found");                       break;
    case SMImportResolver::eState::ParseFailed: actual = tr("unreadable");                      break;
    }

    node->setText(static_cast<int>(eColumn::ColAlias), entry->getName());
    node->setText(static_cast<int>(eColumn::ColLocation), entry->getLocation());
    node->setText(static_cast<int>(eColumn::ColPinned), entry->getVersion().toString());
    node->setText(static_cast<int>(eColumn::ColActual), actual);
}

void SMImports::showDetails(uint32_t id)
{
    const SMImportEntry* entry = mModel.findImport(id);
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    const QSignalBlocker blockAlias(mAlias);
    const QSignalBlocker blockLocation(mLocation);
    const QSignalBlocker blockDescription(mDescription);

    mAlias->setText(entry->getName());
    mLocation->setText(entry->getLocation());
    mDescription->setPlainText(entry->getDescription());

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
        if (resolution.actualVersion == entry->getVersion())
        {
            status = tr("Up to date, version %1.").arg(entry->getVersion().toString());
        }
        else
        {
            status = tr("Pinned to version %1, the file is now %2. Use Update to accept it.")
                        .arg(entry->getVersion().toString(), resolution.actualVersion.toString());
        }
        break;
    }

    mStatus->setText(status);
    updateButtons();
}

void SMImports::showClean()
{
    const QSignalBlocker blockAlias(mAlias);
    const QSignalBlocker blockLocation(mLocation);
    const QSignalBlocker blockDescription(mDescription);

    mAlias->clear();
    mLocation->clear();
    mDescription->clear();
    mStatus->clear();
    updateButtons();
}

void SMImports::updateButtons()
{
    const uint32_t id = currentImportId();
    const bool writable = (mModel.isReadOnly() == false);
    const bool hasRow   = (id != 0);
    const int  index    = hasRow ? mModel.findIndex(id) : -1;
    const int  count    = mModel.getImportCount();

    mButtonAdd->setEnabled(writable);
    mButtonRemove->setEnabled(writable && hasRow);
    mButtonMoveUp->setEnabled(writable && hasRow && (index > 0));
    mButtonMoveDown->setEnabled(writable && hasRow && (index >= 0) && (index + 1 < count));
    mButtonUpdate->setEnabled(writable && hasRow && mModel.resolutionOf(id).isResolved());

    mAlias->setReadOnly(writable == false);
    mLocation->setReadOnly(writable == false);
    mDescription->setReadOnly(writable == false);
    mAlias->setEnabled(hasRow);
    mLocation->setEnabled(hasRow);
    mBrowse->setEnabled(writable && hasRow);
    mDescription->setEnabled(hasRow);
}

QString SMImports::uniqueAlias(const QString& baseName) const
{
    QString seed;
    for (const QChar& ch : baseName)
    {
        seed.append(ch.isLetterOrNumber() || (ch == QLatin1Char('_')) ? ch : QLatin1Char('_'));
    }

    if (seed.isEmpty() || seed.at(0).isDigit())
    {
        seed.prepend(QStringLiteral("Machine"));
    }

    QString candidate = seed;
    for (int suffix = 2; mModel.findImport(candidate) != nullptr; ++suffix)
    {
        candidate = seed + QString::number(suffix);
    }

    return candidate;
}

void SMImports::onSelectionChanged()
{
    const uint32_t id = currentImportId();
    if (id != 0)
    {
        showDetails(id);
    }
    else
    {
        showClean();
    }
}

void SMImports::onAddClicked()
{
    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , QStringList{ QStringLiteral("*.fsml") }
                                , tr("Select State Machine to Import")
                                , this);
    dialog.clearHistory();
    if (dialog.exec() != static_cast<int>(QDialog::DialogCode::Accepted))
    {
        return;
    }

    const QString picked = dialog.getSelectedFilePath();
    if (picked.isEmpty())
    {
        return;
    }

    const QString alias = uniqueAlias(QFileInfo(picked).completeBaseName());
    if (mModel.createImport(alias, mModel.storableLocation(picked)) != nullptr)
    {
        revealElement(mModel.findImport(alias)->getId());
        mAlias->setFocus();
        mAlias->selectAll();
    }
}

void SMImports::onRemoveClicked()
{
    const uint32_t id = currentImportId();
    if (id == 0)
    {
        return;
    }

    // Removing a hosted import breaks its states visibly: each keeps its alias and is reported
    // as an unresolved reference, which is easier to act on than a silently emptied state.
    const SMImportEntry* entry = mModel.findImport(id);
    const QList<SMReferences::Use> uses = mModel.whereUsed(id);
    if (uses.isEmpty() == false)
    {
        QStringList places;
        for (const SMReferences::Use& use : uses)
        {
            places.append(QStringLiteral("  - ") + use.location);
        }

        const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Import is used")
                            , tr("'%1' is hosted by %2 state%3:\n%4\n\nRemove anyway? Those states lose their machine and are listed by validation.")
                              .arg(entry != nullptr ? entry->getName() : QString())
                              .arg(uses.size())
                              .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                              .arg(places.join(QLatin1Char('\n')))
                            , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        if (choice != QMessageBox::Yes)
        {
            return;
        }
    }

    mModel.deleteImport(id);
}

void SMImports::onMoveUpClicked()
{
    const uint32_t id = currentImportId();
    const int index = (id != 0) ? mModel.findIndex(id) : -1;
    if (index > 0)
    {
        mModel.swapImports(id, mModel.getImports().at(index - 1).getId());
        revealElement(id);
    }
}

void SMImports::onMoveDownClicked()
{
    const uint32_t id = currentImportId();
    const int index = (id != 0) ? mModel.findIndex(id) : -1;
    if ((index >= 0) && ((index + 1) < mModel.getImportCount()))
    {
        mModel.swapImports(id, mModel.getImports().at(index + 1).getId());
        revealElement(id);
    }
}

void SMImports::onUpdateClicked()
{
    const uint32_t id = currentImportId();
    if (id == 0)
    {
        return;
    }

    if (mModel.updateVersion(id) == false)
    {
        // Nothing to re-pin: say so rather than leaving the click unanswered.
        showDetails(id);
        QMessageBox::information(this, tr("Import Version")
                                , tr("The pinned version already matches the imported file."));
    }
}

void SMImports::onBrowseClicked()
{
    const uint32_t id = currentImportId();
    if (id == 0)
    {
        return;
    }

    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , QStringList{ QStringLiteral("*.fsml") }
                                , tr("Select State Machine to Import")
                                , this);
    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        const QString picked = dialog.getSelectedFilePath();
        if (picked.isEmpty() == false)
        {
            mModel.setLocation(id, mModel.storableLocation(picked));
        }
    }
}

void SMImports::onAliasCommitted()
{
    const uint32_t id = currentImportId();
    if (id != 0)
    {
        mModel.setName(id, mAlias->text());
        showDetails(id);
    }
}

void SMImports::onLocationCommitted()
{
    const uint32_t id = currentImportId();
    if (id != 0)
    {
        mModel.setLocation(id, mLocation->text());
    }
}

void SMImports::onNotifierChanged()
{
    refreshAll();
}
