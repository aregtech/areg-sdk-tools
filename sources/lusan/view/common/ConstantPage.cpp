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
 *  \file        lusan/view/common/ConstantPage.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Constants page shared by every document editor.
 *
 ************************************************************************/

#include "lusan/view/common/ConstantPage.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/model/common/ConstantModel.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/view/common/ConstantDetailsView.hpp"
#include "lusan/view/common/ConstantListView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WidgetHighlight.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStringListModel>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
    //!< Refreshes a deprecated checkbox + hint pair from an entry's current state, without
    //!< re-triggering the edit signals that would otherwise push a spurious command.
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const ConstantEntry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }

    //!< The predefined types a constant may be declared with: everything with a literal of its
    //!< own. A container has no literal, so it is not offered.
    const QList<DataTypeBase::eCategory>& literalCategories(void)
    {
        static const QList<DataTypeBase::eCategory> _categories
        {
              DataTypeBase::eCategory::Primitive
            , DataTypeBase::eCategory::PrimitiveSint
            , DataTypeBase::eCategory::PrimitiveUint
            , DataTypeBase::eCategory::PrimitiveFloat
            , DataTypeBase::eCategory::BasicObject
        };

        return _categories;
    }
}

ConstantPage::ConstantPage(ConstantModel& model, const QString& headline, QWidget* parent /*= nullptr*/)
    : QScrollArea   (parent)
    , IEditCommit   ( )
    , IETableHelper ( )
    , mModel        (model)
    , mList         (new ConstantListView(this))
    , mDetails      (new ConstantDetailsView(this))
    , mTypeNames    (new QStringListModel(this))
    , mTableCell    (nullptr)
    , mNameCounter  (0)
    , mListPending  (false)
    , mTypesPending (false)
{
    buildUi(headline);
    setupSignals();
    refreshAll();
}

ConstantListView* ConstantPage::getList(void) const
{
    return mList;
}

void ConstantPage::buildUi(const QString& headline)
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

    mDetails->setParent(content);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    columns->addWidget(mDetails, 1);

    root->addLayout(columns, 1);

    // Name, type and value are editable in the list as well as in the details panel, so a row can
    // be filled in without leaving the keyboard.
    QTreeWidget* table = mList->ctrlTableList();
    // The inline editors commit when the edit is done, not per keystroke: a commit rebuilds the
    // list, which would tear the open editor down after the first character typed.
    mTableCell = new TableCell(QList<QAbstractItemModel*>{ mTypeNames }, QList<int>{ static_cast<int>(eColumn::ColType) }, table, this, true);
    mTableCell->setColumnValidation(static_cast<int>(eColumn::ColName), TableCell::eCellValidation::Identifier);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColName) , mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColType) , mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColValue), mTableCell);

    populateTypes();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void ConstantPage::setupSignals(void)
{
    QTreeWidget* table = mList->ctrlTableList();

    connect(table                      , &QTreeWidget::currentItemChanged, this, &ConstantPage::onCurCellChanged);
    connect(mList->ctrlButtonAdd()     , &QToolButton::clicked           , this, &ConstantPage::onAddClicked);
    connect(mList->ctrlButtonInsert()  , &QToolButton::clicked           , this, &ConstantPage::onInsertClicked);
    connect(mList->ctrlButtonRemove()  , &QToolButton::clicked           , this, &ConstantPage::onRemoveClicked);
    connect(mList->ctrlButtonMoveUp()  , &QToolButton::clicked           , this, &ConstantPage::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked           , this, &ConstantPage::onMoveDownClicked);

    // List keys, each doing what the matching toolbar button does: Delete removes the selected
    // constant, Insert adds one, F2 puts the caret in its name.
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scRemove, &QShortcut::activated, this, &ConstantPage::onRemoveClicked);
    connect(scAdd   , &QShortcut::activated, this, &ConstantPage::onAddClicked);
    connect(scRename, &QShortcut::activated, this, &ConstantPage::focusNameField);
    connect(mList, &ElementListView::signalRenameRequested, this, &ConstantPage::focusNameField);

    // A commit from an inline editor rebuilds the list. Let the delegate close first.
    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &ConstantPage::onEditorDataChanged, Qt::QueuedConnection);
    connect(mTableCell, &TableCell::signalEditorTextChanged, this, &ConstantPage::onEditorTextChanged);
    connect(mTableCell, &TableCell::signalEditorClosed     , this, &ConstantPage::onEditorClosed);

    // The shared view already installs the C++ identifier validator on the Name field.
    connect(mDetails->ctrlName()   , &QLineEdit::editingFinished    , this, &ConstantPage::onNameCommitted);
    connect(mDetails                , &ConstantDetailsView::nameEdited, this, [this](const QString& text) {
        if (currentConstantId() != 0)
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(static_cast<int>(eColumn::ColName), text);
        }
    });
    connect(mDetails->ctrlTypes()   , &QComboBox::currentIndexChanged, this, &ConstantPage::onTypeChanged);
    connect(mDetails->ctrlValue()   , &QLineEdit::editingFinished    , this, &ConstantPage::onValueCommitted);
    connect(mDetails->ctrlValue()   , &QLineEdit::textChanged        , this, &ConstantPage::onValueTextChanged);
    connect(mDetails->ctrlDeprecated()   , &QCheckBox::toggled        , this, &ConstantPage::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::editingFinished, this, &ConstantPage::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    // The form carries document text. Typing in it marks the document changed at once, even
    // though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mDetails, mModel.getNotifier());

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &ConstantPage::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::Constant) onNotifierChanged(); });

    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onDataTypesChanged(); });
}

void ConstantPage::commitPendingEdits(void)
{
    const uint32_t id = currentConstantId();
    if (id != 0)
    {
        mModel.setDescription(id, mDetails->ctrlDescription()->toPlainText());
    }
}

bool ConstantPage::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDetails->ctrlDescription()) && (event->type() == QEvent::FocusOut))
    {
        commitPendingEdits();
    }

    return QScrollArea::eventFilter(watched, event);
}

int ConstantPage::getColumnCount(void) const
{
    return mList->ctrlTableList()->columnCount();
}

QString ConstantPage::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->topLevelItem(cell.row());
    return (item != nullptr ? item->text(cell.column()) : QString());
}

DataTypeCustom* ConstantPage::findCustomType(const QString& typeName) const
{
    for (DataTypeCustom* type : mModel.getDocument().getCustomDataTypes())
    {
        if ((type != nullptr) && type->hasTypeName(typeName))
            return type;
    }

    return nullptr;
}

void ConstantPage::populateTypes(void)
{
    QComboBox* combo = mDetails->ctrlTypes();
    const QSignalBlocker blocker(combo);
    const QString current = combo->currentText();
    combo->clear();

    QStringList names;
    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, literalCategories());
    for (DataTypeBase* type : predefined)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
        names.append(type->getName());
    }

    for (DataTypeCustom* type : mModel.getDocument().getCustomDataTypes())
    {
        combo->addItem(type->getQualifiedName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
        names.append(type->getQualifiedName());
    }

    combo->setCurrentText(current);
    mTypeNames->setStringList(names);
}

void ConstantPage::updateValueControl(const ConstantEntry* entry)
{
    QLineEdit* value = mDetails->ctrlValue();
    const QSignalBlocker blocker(value);
    value->clear();
    mDetails->setValueChoices(QStringList());

    if (entry == nullptr)
    {
        value->setEnabled(false);
        value->setToolTip(QString());
        mDetails->showValueHint(QString());
        return;
    }

    const QString typeName = entry->getType();
    DataTypeCustom* custom = findCustomType(typeName);
    // Imported is deliberately excluded: the type is defined elsewhere and opaque to Lusan,
    // so any literal the user types is accepted as-is rather than rejected outright.
    const bool hasNoLiteral = (custom != nullptr)
        && ((custom->getCategory() == DataTypeBase::eCategory::Structure)
         || (custom->getCategory() == DataTypeBase::eCategory::Container));

    if (hasNoLiteral)
    {
        value->setEnabled(false);
        value->setToolTip(tr("'%1' has no literal value; this constant has no value.").arg(typeName));
        mDetails->showValueHint(QString());
        return;
    }

    value->setEnabled(true);
    value->setToolTip(QString());

    if ((custom != nullptr) && (custom->getCategory() == DataTypeBase::eCategory::Enumeration))
    {
        QStringList choices;
        for (const EnumEntry& field : static_cast<DataTypeEnum*>(custom)->getElements())
        {
            choices.append(field.getName());
        }

        mDetails->setValueChoices(choices);
    }

    value->setText(entry->getValue());
    updateValueValidation(typeName, entry->getValue());
}

QString ConstantPage::valueValidationReason(const QString& typeName, const QString& value) const
{
    // The same answer the validation engine gives, so the hint under the field and the finding
    // in the results panel can never disagree.
    return DocRuleChecks::literalReason(mModel.getDocument().getDataTypeSection(), typeName, value);
}

void ConstantPage::updateValueValidation(const QString& typeName, const QString& value)
{
    mDetails->showValueHint(valueValidationReason(typeName, value));
}

void ConstantPage::setNodeText(QTreeWidgetItem* node, const ConstantEntry& entry) const
{
    node->setIcon(static_cast<int>(eColumn::ColName), entry.getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(static_cast<int>(eColumn::ColName), entry.getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(static_cast<int>(eColumn::ColType), entry.getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(static_cast<int>(eColumn::ColType), entry.getString(ElementBase::eDisplay::DisplayType));

    const QString reason = valueValidationReason(entry.getType(), entry.getValue());
    node->setIcon(static_cast<int>(eColumn::ColValue), reason.isEmpty() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
    node->setText(static_cast<int>(eColumn::ColValue), entry.getString(ElementBase::eDisplay::DisplayValue));
    node->setToolTip(static_cast<int>(eColumn::ColValue), reason);
}

void ConstantPage::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    const uint32_t id = current->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    ConstantEntry* entry = mModel.findConstant(id);
    if (entry == nullptr)
    {
        showClean();
        return;
    }

    selectedConstant(entry);
}

void ConstantPage::selectedConstant(const ConstantEntry* entry)
{
    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockType(mDetails->ctrlTypes());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
    }

    updateValueControl(entry);
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), entry);

    mDetails->ctrlName()->setEnabled(true);
    mDetails->ctrlTypes()->setEnabled(true);

    mList->ctrlButtonRemove()->setEnabled(true);
    updateMoveButtons(mModel.findIndex(entry->getId()), mModel.getConstantCount());
}

void ConstantPage::showClean(void)
{
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);
    updateValueControl(nullptr);

    const QSignalBlocker blockName(mDetails->ctrlName());
    const QSignalBlocker blockType(mDetails->ctrlTypes());
    const QSignalBlocker blockDescr(mDetails->ctrlDescription());
    mDetails->ctrlName()->clear();
    mDetails->ctrlTypes()->setCurrentIndex(-1);
    mDetails->ctrlDescription()->clear();

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

void ConstantPage::updateMoveButtons(int row, int rowCount)
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

ConstantEntry* ConstantPage::currentConstant(void) const
{
    return mModel.findConstant(currentConstantId());
}

uint32_t ConstantPage::currentConstantId(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

void ConstantPage::revealElement(uint32_t id, eIssueField field /*= eIssueField::None*/)
{
    // The list may be waiting for a rebuild the page put off while it was hidden, and what
    // is revealed has to be the row the model holds now.
    flushPendingRefresh();

    if (selectConstant(id) == false)
    {
        return;
    }

    switch (field)
    {
    case eIssueField::Name:         WidgetHighlight::reveal(mDetails->ctrlName());        break;
    case eIssueField::Type:         WidgetHighlight::reveal(mDetails->ctrlTypes());       break;
    case eIssueField::Value:        WidgetHighlight::reveal(mDetails->ctrlValue());       break;
    case eIssueField::Description:  WidgetHighlight::reveal(mDetails->ctrlDescription()); break;
    default:                                                                              break;
    }
}

void ConstantPage::refreshDataTypes(void)
{
    onDataTypesChanged();
}

void ConstantPage::replaceDataType(DataTypeBase* oldType, DataTypeBase* newType)
{
    mModel.replaceDataType(oldType, newType);
    onDataTypesChanged();
    refreshAll();
}

QString ConstantPage::genName(void)
{
    static const QString _defName("NewConstant");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findConstant(name) != nullptr);

    return name;
}

void ConstantPage::onAddClicked(void)
{
    ConstantEntry* entry = mModel.createConstant(genName());
    if (entry != nullptr)
    {
        selectConstant(entry->getId());
        focusNameField();
    }
}

void ConstantPage::onInsertClicked(void)
{
    const uint32_t id = currentConstantId();
    const int position = (id != 0 ? mModel.findIndex(id) : 0);
    ConstantEntry* entry = mModel.insertConstant(position < 0 ? 0 : position, genName());
    if (entry != nullptr)
    {
        selectConstant(entry->getId());
        focusNameField();
    }
}

void ConstantPage::focusNameField(void)
{
    if (currentConstantId() != 0)
    {
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

bool ConstantPage::confirmRemove(uint32_t /*id*/)
{
    return true;
}

void ConstantPage::onRemoveClicked(void)
{
    const uint32_t id = currentConstantId();
    if ((id == 0) || (confirmRemove(id) == false))
        return;

    const QList<ConstantEntry>& list = mModel.getConstants();
    const int index = mModel.findIndex(id);
    uint32_t neighborId = 0;
    if (list.size() > 1)
    {
        const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
        neighborId = list.at(neighborIndex).getId();
    }

    mModel.deleteConstant(id);
    if (neighborId != 0)
    {
        selectConstant(neighborId);
    }
}

void ConstantPage::onMoveUpClicked(void)
{
    const uint32_t moved = mModel.moveConstant(currentConstantId(), -1);
    if (moved != 0)
    {
        selectConstant(moved);
    }
}

void ConstantPage::onMoveDownClicked(void)
{
    const uint32_t moved = mModel.moveConstant(currentConstantId(), +1);
    if (moved != 0)
    {
        selectConstant(moved);
    }
}

void ConstantPage::onNameCommitted(void)
{
    const uint32_t id = currentConstantId();
    if (id != 0)
    {
        mModel.renameConstant(id, mDetails->ctrlName()->text());
    }
}

void ConstantPage::onTypeChanged(int index)
{
    const uint32_t id = currentConstantId();
    if ((id == 0) || (index < 0))
        return;

    DataTypeBase* selected = mDetails->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected == nullptr)
        return;

    mModel.setType(id, selected->getName());
    updateValueControl(mModel.findConstant(id));
}

void ConstantPage::onValueCommitted(void)
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    mModel.setValue(id, mDetails->ctrlValue()->text());
    ConstantEntry* entry = mModel.findConstant(id);
    if (entry != nullptr)
    {
        updateValueValidation(entry->getType(), entry->getValue());
    }
}

void ConstantPage::onValueTextChanged(const QString& text)
{
    ConstantEntry* entry = mModel.findConstant(currentConstantId());
    if (entry == nullptr)
        return;

    const QString reason = valueValidationReason(entry->getType(), text);
    mDetails->showValueHint(reason);

    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item != nullptr)
    {
        item->setText(static_cast<int>(eColumn::ColValue), text);
        item->setIcon(static_cast<int>(eColumn::ColValue), reason.isEmpty() ? QIcon() : NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        item->setToolTip(static_cast<int>(eColumn::ColValue), reason);
    }
}

void ConstantPage::onDeprecatedToggled(bool checked)
{
    const uint32_t id = currentConstantId();
    if (id == 0)
        return;

    mModel.setDeprecated(id, checked);
    ConstantEntry* entry = mModel.findConstant(id);
    const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
    mDetails->ctrlDeprecateHint()->setEnabled(checked);
    mDetails->ctrlDeprecateHint()->setText((checked && (entry != nullptr)) ? entry->getDeprecateHint() : QString());
    if (checked)
    {
        mDetails->ctrlDeprecateHint()->setFocus();
    }
}

void ConstantPage::onDeprecateHintCommitted(void)
{
    const uint32_t id = currentConstantId();
    if (id != 0)
    {
        mModel.setDeprecateHint(id, mDetails->ctrlDeprecateHint()->text());
    }
}

void ConstantPage::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->topLevelItemCount()))
        return;

    QTreeWidgetItem* item = table->topLevelItem(index.row());
    const uint32_t id = item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    if (mModel.findConstant(id) == nullptr)
        return;

    switch (index.column())
    {
    case static_cast<int>(eColumn::ColName):
        mModel.renameConstant(id, newValue);
        break;

    case static_cast<int>(eColumn::ColType):
        mModel.setType(id, newValue);
        break;

    case static_cast<int>(eColumn::ColValue):
        mModel.setValue(id, newValue);
        break;

    default:
        break;
    }

    // The commit above rebuilt the list; put the details panel back on the edited row.
    selectConstant(id);
}

void ConstantPage::onEditorTextChanged(const QModelIndex& index, const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->topLevelItem(index.row());
    if ((item == nullptr) || (item != table->currentItem()))
        return;

    if (index.column() == static_cast<int>(eColumn::ColName))
    {
        // Blocked: the field re-emits its text as nameEdited, which writes it back into the row.
        const QSignalBlocker blockName(mDetails->ctrlName());
        mDetails->ctrlName()->setText(newText);
    }
    else if (index.column() == static_cast<int>(eColumn::ColValue))
    {
        const ConstantEntry* entry = mModel.findConstant(currentConstantId());
        if (entry == nullptr)
            return;

        const QSignalBlocker blockValue(mDetails->ctrlValue());
        mDetails->ctrlValue()->setText(newText);
        updateValueValidation(entry->getType(), newText);
    }
}

void ConstantPage::onEditorClosed(void)
{
    onCurCellChanged(mList->ctrlTableList()->currentItem(), nullptr);
}

void ConstantPage::refreshAll(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    const uint32_t selId = currentConstantId();

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (const ConstantEntry& entry : mModel.getConstants())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            // Editable lets the delegate open an inline editor on double-click.
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            setNodeText(item, entry);
            item->setData(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole, entry.getId());
            table->addTopLevelItem(item);
        }
    }

    if ((selId == 0) || (selectConstant(selId) == false))
    {
        showClean();
    }
}

bool ConstantPage::selectConstant(uint32_t id)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = table->topLevelItem(i);
        if (item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt() == id)
        {
            table->setCurrentItem(item);
            return true;
        }
    }

    return false;
}

void ConstantPage::onNotifierChanged(void)
{
    // One edit reaches every page of the document. A page the user is not looking at
    // rebuilds its list when it comes forward, so the cost of an edit follows what the
    // user sees and not how many pages the document has.
    if (isVisible() == false)
    {
        mListPending = true;
        return;
    }

    mListPending = false;
    refreshAll();
}

void ConstantPage::flushPendingRefresh(void)
{
    if (mTypesPending)
    {
        mTypesPending = false;
        applyDataTypesChanged();
    }

    if (mListPending)
    {
        mListPending = false;
        refreshAll();
    }
}

void ConstantPage::showEvent(QShowEvent* event)
{
    QScrollArea::showEvent(event);
    flushPendingRefresh();
}

void ConstantPage::onDataTypesChanged(void)
{
    if (isVisible() == false)
    {
        mTypesPending = true;
        return;
    }

    mTypesPending = false;
    applyDataTypesChanged();
}

void ConstantPage::applyDataTypesChanged(void)
{
    populateTypes();
    updateValueControl(mModel.findConstant(currentConstantId()));
}
