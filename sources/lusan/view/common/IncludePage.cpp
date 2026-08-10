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
 *  \file        lusan/view/common/IncludePage.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Includes page shared by every document editor.
 *
 ************************************************************************/

#include "lusan/view/common/IncludePage.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/IncludeModel.hpp"
#include "lusan/view/common/IncludeDetailsView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WidgetHighlight.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QCheckBox>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
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
}

IncludePage::IncludePage(IncludeModel& model, const IncludeTypeConfig& config, const QString& headline, QWidget* parent /*= nullptr*/)
    : QScrollArea   (parent)
    , mModel        (model)
    , mList         (new IncludeListView(config, this))
    , mDetailsStack (nullptr)
    , mDetails      (new IncludeDetailsView(this))
    , mTableCell    (nullptr)
    , mNameCounter  (0)
{
    buildUi(headline);
    setupSignals();
    // A subclass that adds an editor of its own is not built yet, so this first pass shows the
    // page as the base knows it and the subclass refreshes again once its editor exists.
    refreshAll();
}

int IncludePage::getColumnCount(void) const
{
    return mList->ctrlTableList()->columnCount();
}

QString IncludePage::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->itemAt(cell);
    return (item != nullptr ? item->text(cell.column()) : QString());
}

QStringList IncludePage::getSupportedExtensions(void) const
{
    // A data type document belongs to no editor in particular and is shared, so `.dtml` is
    // offered wherever the host takes one. Whether a document of the host's own kind may be
    // included is the other configured difference between the editors.
    const IncludeTypeConfig& config = mList->getConfig();
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    if (config.hasDocuments())
    {
        QStringList docExts{ QStringLiteral("*.") + config.docExtension };
        if (config.hasDataTypes())
        {
            docExts.append(QStringLiteral("*.dtml"));
        }

        exts.append(LusanApplication::buildFileFilter(tr("%1 Files").arg(config.docTypeLabel), docExts));
    }
    else if (config.hasDataTypes())
    {
        exts.append(LusanApplication::buildFileFilter(tr("Data Type Files"), QStringList{ QStringLiteral("*.dtml") }));
    }

    return exts;
}

void IncludePage::buildUi(const QString& headline)
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* title = new QLabel(headline, content);
    QFont titleFont{ title->font() };
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleFont.setItalic(true);
    title->setFont(titleFont);
    root->addWidget(title);

    QHBoxLayout* columns = new QHBoxLayout();

    mList->setParent(content);
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mList, 1);

    // One editor slot. A document that gives some row kind an editor of its own adds it to the
    // stack, so the fields swap rather than half of them greying out.
    mDetailsStack = new QStackedWidget(content);
    mDetails->setParent(mDetailsStack);
    mDetailsStack->addWidget(mDetails);
    mDetailsStack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mDetailsStack, 1);

    root->addLayout(columns, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);

    // Inline editing of the Location column, committed once on editing-finished so one undo command
    // is pushed. Type, Name and Version are derived, so the delegate refuses an editor there.
    QTreeWidget* table = mList->ctrlTableList();
    mTableCell = new TableCell(QList<QAbstractItemModel*>(), QList<int>(), table, this, true);
    mTableCell->setColumnValidation(static_cast<int>(IncludeListView::eColumn::ColLocation), TableCell::eCellValidation::Path);
    mTableCell->setEditableCheck([](const QModelIndex& index) {
        return index.column() == static_cast<int>(IncludeListView::eColumn::ColLocation);
    });
    table->setItemDelegate(mTableCell);
}

void IncludePage::setupSignals(void)
{
    connect(mList->ctrlTableList()     , &QTreeWidget::currentItemChanged, this, &IncludePage::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &IncludePage::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &IncludePage::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &IncludePage::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &IncludePage::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &IncludePage::onMoveDownClicked);
    connect(mList->ctrlButtonUpdate()  , &QToolButton::clicked           , this, &IncludePage::onUpdateClicked);

    // List keys mirroring the toolbar buttons: Delete removes the selected include, Insert adds
    // one, F2 puts the caret in its location. A group heading carries none, so those do nothing.
    QTreeWidget* table = mList->ctrlTableList();
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scRemove, &QShortcut::activated, this, &IncludePage::onRemoveClicked);
    connect(scAdd   , &QShortcut::activated, this, &IncludePage::onAddClicked);
    connect(scRename, &QShortcut::activated, this, &IncludePage::focusLocationField);

    // The include path keystroke validator lives inside the shared IncludeDetailsView.
    connect(mDetails->ctrlInclude()      , &QLineEdit::editingFinished, this, &IncludePage::onLocationCommitted);
    connect(mDetails->ctrlInclude()      , &QLineEdit::textChanged    , this, &IncludePage::onLocationTextChanged);
    connect(mDetails->ctrlBrowseButton() , &QPushButton::clicked      , this, &IncludePage::onBrowseClicked);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &IncludePage::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &IncludePage::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &IncludePage::onInlineLocationEdited);

    DocModelNotifier& notifier = mModel.getNotifier();

    // The form carries document text. Typing in it marks the document changed at once, even
    // though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mDetails, notifier);

    auto onKind = [this](uint32_t, eDocElementKind kind) {
        if ((kind == eDocElementKind::Include) || (kind == eDocElementKind::Import))
        {
            onNotifierChanged();
        }
    };
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &IncludePage::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, onKind);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, onKind);
    connect(&notifier, &DocModelNotifier::elementChanged, this, onKind);
    connect(&notifier, &DocModelNotifier::listReordered , this, onKind);
}

void IncludePage::commitPendingEdits(void)
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setDescription(id, mDetails->ctrlDescription()->toPlainText());
    }
}

bool IncludePage::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::FocusOut) && (watched == mDetails->ctrlDescription()))
    {
        commitPendingEdits();
    }

    return QScrollArea::eventFilter(watched, event);
}

void IncludePage::setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const
{
    if (node == nullptr)
        return;

    const QString location = entry.getLocation();
    const eIncludeKind kind = mList->kindForLocation(location);
    const bool document = (kind == eIncludeKind::Document);
    node->setIcon(static_cast<int>(IncludeListView::eColumn::ColLocation), mList->iconForKind(kind));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColLocation), entry.getString(ElementBase::eDisplay::DisplayName));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColType)    , mList->typeForLocation(location));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColName)    , document ? entry.getAlias() : mList->nameForLocation(location));
    node->setText(static_cast<int>(IncludeListView::eColumn::ColVersion) , document ? entry.getVersion().toString() : QString());
}

void IncludePage::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    IncludeEntry* entry = mModel.findInclude(IncludeListView::rowId(current));
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedInclude(entry);
}

void IncludePage::selectedInclude(const IncludeEntry* entry)
{
    const uint32_t id = entry->getId();
    const eIncludeKind kind = mList->kindForLocation(entry->getLocation());
    mDetailsStack->setCurrentWidget(mDetails);

    {
        const QSignalBlocker blockInclude(mDetails->ctrlInclude());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlInclude()->setText(entry->getLocation());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), entry);

    // A data type document is not a header, so there is nothing to deprecate on it.
    const bool writable = (mModel.isReadOnly() == false);
    mDetails->setDeprecationVisible(kind == eIncludeKind::Source);
    mDetails->ctrlInclude()->setEnabled(writable);
    mDetails->ctrlBrowseButton()->setEnabled(writable);
    mDetails->ctrlDescription()->setEnabled(writable);
    mDetails->ctrlDeprecated()->setEnabled(writable && (kind == eIncludeKind::Source));

    mList->ctrlButtonRemove()->setEnabled(writable);
    updateMoveButtons(mModel.findIndex(id), mModel.getIncludeCount());
}

void IncludePage::showClean(void)
{
    mDetailsStack->setCurrentWidget(mDetails);
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

bool IncludePage::confirmRemove(uint32_t /*id*/)
{
    return true;
}

bool IncludePage::acceptLocation(const QString& /*location*/)
{
    return true;
}

void IncludePage::commitBrowsedLocation(uint32_t id, const QString& /*absolutePath*/, const QString& relativePath)
{
    if (acceptLocation(relativePath))
    {
        mModel.setLocation(id, relativePath);
    }
}

void IncludePage::updateMoveButtons(int row, int rowCount)
{
    if ((row < 0) || (row >= rowCount) || mModel.isReadOnly())
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        return;
    }

    mList->ctrlButtonMoveUp()->setEnabled(row > 0);
    mList->ctrlButtonMoveDown()->setEnabled(row < (rowCount - 1));
}

uint32_t IncludePage::currentIncludeId(void) const
{
    return IncludeListView::rowId(mList->ctrlTableList()->currentItem());
}

bool IncludePage::revealElement(uint32_t id, eIssueField field /*= eIssueField::None*/)
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

    switch (field)
    {
    case eIssueField::Name:         WidgetHighlight::reveal(mDetails->ctrlInclude());     break;
    case eIssueField::Description:  WidgetHighlight::reveal(mDetails->ctrlDescription()); break;
    default:                                                                              break;
    }

    return true;
}

QString IncludePage::genName(void)
{
    static const QString _defName("NewInclude");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findInclude(name) != nullptr);

    return name;
}

void IncludePage::onAddClicked(void)
{
    // A new entry starts as a plain include file: its group follows its location, and it has
    // none yet. Picking or typing one moves the row.
    IncludeEntry* entry = mModel.createInclude(genName());
    if (entry != nullptr)
    {
        selectInclude(entry->getId());
        focusLocationField();
    }
}

void IncludePage::onInsertClicked(void)
{
    const uint32_t id = currentIncludeId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    IncludeEntry* entry = mModel.insertInclude(position < 0 ? 0 : position, genName());
    if (entry != nullptr)
    {
        selectInclude(entry->getId());
        focusLocationField();
    }
}

void IncludePage::focusLocationField(void)
{
    if (currentIncludeId() != 0)
    {
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
    }
}

void IncludePage::onRemoveClicked(void)
{
    const uint32_t id = currentIncludeId();
    if ((id == 0) || (confirmRemove(id) == false))
        return;

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

void IncludePage::onMoveUpClicked(void)
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

void IncludePage::onMoveDownClicked(void)
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

void IncludePage::onLocationCommitted(void)
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        // Skip a no-op command: after an Escape-cancel the field already holds the model value, so
        // a spurious editingFinished must not push an empty undo step.
        const IncludeEntry* entry = mModel.findInclude(id);
        const QString text = mDetails->ctrlInclude()->text();
        if ((entry != nullptr) && (entry->getLocation() != text) && acceptLocation(text))
        {
            mModel.setLocation(id, text);
        }
    }
}

void IncludePage::onLocationTextChanged(const QString& text)
{
    // Live-preview the typed path into the selected include's row (location + derived type/name
    // columns); the change commits on editingFinished. Selection sets the field under a blocker.
    const uint32_t id = currentIncludeId();
    QTreeWidgetItem* item = (id != 0 ? mList->ctrlTableList()->currentItem() : nullptr);
    if (item == nullptr)
    {
        return;
    }

    // The heading a row sits under is part of what the location says, so the row moves while it
    // is typed rather than at commit.
    const eIncludeKind kind = mList->kindForLocation(text);
    const QSignalBlocker blocker(mList->ctrlTableList());
    mList->placeRow(item, kind, id);
    mList->refreshGroupCounts();
    item->setIcon(static_cast<int>(IncludeListView::eColumn::ColLocation), mList->iconForKind(kind));
    item->setText(static_cast<int>(IncludeListView::eColumn::ColLocation), text);
    item->setText(static_cast<int>(IncludeListView::eColumn::ColType)    , mList->typeForLocation(text));
    item->setText(static_cast<int>(IncludeListView::eColumn::ColName)    , mList->nameForLocation(text));
    mList->ctrlTableList()->setCurrentItem(item);
}

void IncludePage::onInlineLocationEdited(const QModelIndex& index, const QString& newValue)
{
    if (index.column() != static_cast<int>(IncludeListView::eColumn::ColLocation))
        return;

    const uint32_t id = IncludeListView::rowId(mList->itemAt(index));
    const IncludeEntry* entry = mModel.findInclude(id);
    if ((entry != nullptr) && (entry->getLocation() != newValue) && acceptLocation(newValue))
    {
        // The undo command fires an include notifier, which rebuilds the list with the re-derived
        // Type/Name columns and moves the row if its kind changed.
        mModel.setLocation(id, newValue);
    }
}

void IncludePage::onBrowseClicked(void)
{
    const uint32_t id = currentIncludeId();
    if (id == 0)
        return;

    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , getSupportedExtensions()
                                , tr("Select Include File")
                                , this);
    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        commitBrowsedLocation(id, dialog.getSelectedFilePath(), dialog.getSelectedFileRelativePath());
    }
}

void IncludePage::onDeprecatedToggled(bool checked)
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

void IncludePage::onDeprecateHintCommitted(void)
{
    const uint32_t id = currentIncludeId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void IncludePage::refreshAll(void)
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
            setNodeText(mList->placeRow(nullptr, kind, entry.getId()), entry);
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

bool IncludePage::selectInclude(uint32_t id)
{
    return revealElement(id);
}

void IncludePage::onUpdateClicked(void)
{
    // Re-derives the Type/Name/Version columns and the resolution status from the current
    // locations: the included files live outside the document and change under the editor.
    refreshAll();
}

void IncludePage::onNotifierChanged(void)
{
    refreshAll();
}
