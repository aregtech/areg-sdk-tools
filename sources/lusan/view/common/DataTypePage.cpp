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
 *  \file        lusan/view/common/DataTypePage.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Data Types page shared by every document editor.
 *
 ************************************************************************/

#include "lusan/view/common/DataTypePage.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/model/common/DataTypeModel.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/view/common/DataTypeDetailsView.hpp"
#include "lusan/view/common/DataTypeFieldDetailsView.hpp"
#include "lusan/view/common/DataTypeListView.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WidgetHighlight.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
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
    template<class Entry>
    void applyDeprecatedDisplay(QCheckBox* checkBox, QLineEdit* hintEdit, const Entry* entry)
    {
        const QSignalBlocker blockCheck(checkBox);
        const QSignalBlocker blockHint(hintEdit);
        const bool deprecated = (entry != nullptr) && entry->getIsDeprecated();
        checkBox->setChecked(deprecated);
        hintEdit->setEnabled(deprecated);
        hintEdit->setText(deprecated ? entry->getDeprecateHint() : QString());
    }

    //!< The predefined types a structure field or a container element may be declared with.
    const QList<DataTypeBase::eCategory>& fieldCategories(void)
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

    //!< The predefined types an enumeration may derive from.
    const QList<DataTypeBase::eCategory>& integerCategories(void)
    {
        static const QList<DataTypeBase::eCategory> _categories
        {
              DataTypeBase::eCategory::PrimitiveSint
            , DataTypeBase::eCategory::PrimitiveUint
        };

        return _categories;
    }
}

DataTypePage::DataTypePage(DataTypeModel& model, const QString& headline, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , IEditCommit       ( )
    , IETableHelper     ( )
    , mModel            (model)
    , mList             (new DataTypeListView(this))
    , mDetails          (new DataTypeDetailsView(this))
    , mFields           (new DataTypeFieldDetailsView(this))
    , mFieldTypeNames   (new QStringListModel(this))
    , mIntegerNames     (new QStringListModel(this))
    , mTableCell        (nullptr)
    , mNameCounter      (0)
    , mCurUrl           ( )
    , mCurFile          ( )
    , mListPending      (false)
{
    buildUi(headline);
    setupSignals();
    refreshAll();
}

DataTypeListView* DataTypePage::getList(void) const
{
    return mList;
}

void DataTypePage::buildUi(const QString& headline)
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

    QWidget* rightColumn = new QWidget(content);
    rightColumn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    mDetails->setParent(rightColumn);
    mFields->setParent(rightColumn);
    mFields->setHidden(true);
    rightLayout->addWidget(mDetails);
    rightLayout->addWidget(mFields);
    columns->addWidget(rightColumn, 1);

    root->addLayout(columns, 1);

    populateIntegerCombo(mDetails->ctrlEnumDerived());
    populateContainerObjectCombo(mDetails->ctrlContainerObject());

    // Name, type and value are editable in the tree as well as in the details panels, so a row
    // can be filled in without leaving the keyboard. The tree is heterogeneous, so which cells
    // open, with what editor and with what validation, is decided per cell.
    QTreeWidget* table = mList->ctrlTableList();
    mTableCell = new TableCell(table, this, true);
    mTableCell->setEditableCheck([this](const QModelIndex& idx) { return isCellEditable(idx); });
    mTableCell->setEditorModelResolver([this](const QModelIndex& idx) { return editorModelFor(idx); });
    mTableCell->setValidationResolver([this](const QModelIndex& idx) { return validationFor(idx); });
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColName) , mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColType) , mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColValue), mTableCell);

    populateInlineTypeNames();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void DataTypePage::setupSignals(void)
{
    QTreeWidget* table = mList->ctrlTableList();

    connect(table                          , &QTreeWidget::currentItemChanged , this, &DataTypePage::onCurCellChanged);
    connect(mList->ctrlButtonAdd()         , &QToolButton::clicked            , this, &DataTypePage::onAddClicked);
    connect(mList->ctrlButtonInsert()      , &QToolButton::clicked            , this, &DataTypePage::onInsertClicked);
    connect(mList->ctrlButtonRemove()      , &QToolButton::clicked            , this, &DataTypePage::onRemoveClicked);
    connect(mList->ctrlButtonAddChild()    , &QToolButton::clicked            , this, &DataTypePage::onAddFieldClicked);
    connect(mList->ctrlButtonInsertChild() , &QToolButton::clicked            , this, &DataTypePage::onInsertFieldClicked);
    connect(mList->ctrlButtonRemoveChild() , &QToolButton::clicked            , this, &DataTypePage::onRemoveFieldClicked);
    connect(mList->ctrlButtonMoveUp()      , &QToolButton::clicked            , this, &DataTypePage::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown()    , &QToolButton::clicked            , this, &DataTypePage::onMoveDownClicked);
    connect(mList->actionNewStruct()       , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Structure); });
    connect(mList->actionNewEnum()         , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Enumeration); });
    connect(mList->actionNewImport()       , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Imported); });
    connect(mList->actionNewContainer()    , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Container); });

    // Tree keys mirroring the toolbar buttons: Delete removes the selected row, Insert adds one,
    // F2 puts the caret in its name. The selected level picks which toolbar button the key follows.
    QShortcut* scRemove = new QShortcut(QKeySequence(Qt::Key_Delete), table);
    QShortcut* scAdd    = new QShortcut(QKeySequence(Qt::Key_Insert), table);
    QShortcut* scRename = new QShortcut(QKeySequence(Qt::Key_F2), table);
    scRemove->setContext(Qt::WidgetWithChildrenShortcut);
    scAdd->setContext(Qt::WidgetWithChildrenShortcut);
    scRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scRemove, &QShortcut::activated, this, [this]()
    {
        if (currentFieldId() != 0)
        {
            onRemoveFieldClicked();
        }
        else
        {
            onRemoveClicked();
        }
    });
    connect(scAdd, &QShortcut::activated, this, [this]()
    {
        if (currentFieldId() != 0)
        {
            onAddFieldClicked();
        }
        else
        {
            onAddClicked();
        }
    });
    connect(scRename, &QShortcut::activated, this, &DataTypePage::focusNameField);
    connect(mList, &ElementListView::signalRenameRequested, this, &DataTypePage::focusNameField);

    // A commit from an inline editor rebuilds the tree. Let the delegate close first.
    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &DataTypePage::onEditorDataChanged, Qt::QueuedConnection);
    connect(mTableCell, &TableCell::signalEditorTextChanged, this, &DataTypePage::onEditorTextChanged);
    connect(mTableCell, &TableCell::signalEditorClosed     , this, &DataTypePage::onEditorClosed);

    // The identifier validator is installed inside the shared DataTypeDetailsView.
    connect(mDetails->ctrlName()            , &QLineEdit::editingFinished      , this, &DataTypePage::onNameCommitted);
    // Live-preview the typed name into the Name column; the real rename commits on editingFinished.
    // Selection sets the field under a QSignalBlocker, so this fires only for genuine user edits.
    connect(mDetails->ctrlName()            , &QLineEdit::textChanged          , this, [this](const QString& text) {
        if ((currentFieldId() == 0) && (currentDataType() != nullptr))
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(static_cast<int>(eColumn::ColName), text);
        }
    });
    connect(mDetails->ctrlTypeStruct()      , &QRadioButton::clicked           , this, &DataTypePage::onStructSelected);
    connect(mDetails->ctrlTypeEnum()        , &QRadioButton::clicked           , this, &DataTypePage::onEnumSelected);
    connect(mDetails->ctrlTypeImport()      , &QRadioButton::clicked           , this, &DataTypePage::onImportSelected);
    connect(mDetails->ctrlTypeContainer()   , &QRadioButton::clicked           , this, &DataTypePage::onContainerSelected);
    connect(mDetails->ctrlEnumDerived()     , &QComboBox::currentIndexChanged  , this, &DataTypePage::onEnumDerivedChanged);
    connect(mDetails->ctrlImportLocation()  , &QLineEdit::editingFinished      , this, &DataTypePage::onImportLocationCommitted);
    connect(mDetails->ctrlImportNamespace() , &QLineEdit::editingFinished      , this, &DataTypePage::onImportNamespaceCommitted);
    connect(mDetails->ctrlImportObject()    , &QLineEdit::editingFinished      , this, &DataTypePage::onImportObjectCommitted);
    connect(mDetails->ctrlButtonBrowse()    , &QPushButton::clicked            , this, &DataTypePage::onImportBrowse);
    connect(mDetails->ctrlContainerObject() , &QComboBox::currentIndexChanged  , this, &DataTypePage::onContainerObjectChanged);
    connect(mDetails->ctrlContainerKey()    , &QComboBox::currentIndexChanged  , this, &DataTypePage::onContainerKeyChanged);
    connect(mDetails->ctrlContainerValue()  , &QComboBox::currentIndexChanged  , this, &DataTypePage::onContainerValueChanged);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled              , this, &DataTypePage::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::editingFinished      , this, &DataTypePage::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    // The identifier validator is installed inside the shared DataTypeFieldDetailsView.
    connect(mFields->ctrlName()             , &QLineEdit::editingFinished      , this, &DataTypePage::onFieldNameCommitted);
    connect(mFields->ctrlName()             , &QLineEdit::textChanged          , this, [this](const QString& text) {
        if (currentFieldId() != 0)
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(static_cast<int>(eColumn::ColName), text);
        }
    });
    connect(mFields->ctrlTypes()            , &QComboBox::currentIndexChanged  , this, &DataTypePage::onFieldTypeChanged);
    connect(mFields->ctrlValue()            , &QLineEdit::editingFinished      , this, &DataTypePage::onFieldValueCommitted);
    connect(mFields->ctrlDeprecated()       , &QCheckBox::toggled              , this, &DataTypePage::onFieldDeprecatedToggled);
    connect(mFields->ctrlDeprecateHint()    , &QLineEdit::editingFinished      , this, &DataTypePage::onFieldDeprecateHintCommitted);
    mFields->ctrlDescription()->installEventFilter(this);

    // Both forms carry document text. Typing in them marks the document changed at once, even
    // though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mDetails, mModel.getNotifier());
    PendingEditWatcher::watchField(mFields, mModel.getNotifier());

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &DataTypePage::onNotifierChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&mModel   , &DataTypeModel::importsChanged  , this, &DataTypePage::onNotifierChanged);
}

void DataTypePage::commitPendingEdits(void)
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    // The selection decides which of the two boxes is the live one: a field row is edited in the
    // field form, the type itself in the type form.
    const uint32_t fieldId = currentFieldId();
    if (fieldId == 0)
        mModel.setDescription(dataType, mDetails->ctrlDescription()->toPlainText());
    else
        mModel.setFieldDescription(dataType, fieldId, mFields->ctrlDescription()->toPlainText());
}

bool DataTypePage::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if ((watched == mDetails->ctrlDescription()) || (watched == mFields->ctrlDescription()))
        {
            commitPendingEdits();
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

int DataTypePage::getColumnCount(void) const
{
    return mList->ctrlTableList()->columnCount();
}

QString DataTypePage::getCellText(const QModelIndex& cell) const
{
    // The display text already stored on the tree cell seeds the inline editor.
    return (cell.isValid() ? cell.data(Qt::DisplayRole).toString() : QString());
}

DataTypeCustom* DataTypePage::currentDataType(void) const
{
    // The type this page may edit. A type read out of an included data type document belongs to
    // that document, so it answers nothing here and every editing path stops at the first line.
    DataTypeCustom* dataType = selectedDataType();
    return (((dataType != nullptr) && dataType->isDocumentImport()) ? nullptr : dataType);
}

DataTypeCustom* DataTypePage::selectedDataType(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr);
}

uint32_t DataTypePage::currentFieldId(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

void DataTypePage::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    DataTypeCustom* dataType = current->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    const uint32_t fieldId = current->data(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole).toUInt();
    if (dataType == nullptr)
    {
        // The heading of an imported group: it names a document, not a declaration.
        showClean();
        return;
    }

    if (fieldId == 0)
    {
        switch (dataType->getCategory())
        {
        case DataTypeBase::eCategory::Structure:
            selectedStruct(static_cast<DataTypeStructure*>(dataType));
            break;
        case DataTypeBase::eCategory::Enumeration:
            selectedEnum(static_cast<DataTypeEnum*>(dataType));
            break;
        case DataTypeBase::eCategory::Imported:
            selectedImport(static_cast<DataTypeImported*>(dataType));
            break;
        case DataTypeBase::eCategory::Container:
            selectedContainer(static_cast<DataTypeContainer*>(dataType));
            break;
        default:
            break;
        }
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        selectedStructField(static_cast<DataTypeStructure*>(dataType), fieldId);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        selectedEnumField(static_cast<DataTypeEnum*>(dataType), fieldId);
    }

    // Last, because the calls above enable the row tools for the category they filled in. The
    // forms stay filled so an imported declaration can be read; what changes is that nothing in
    // them, and no tool beside them, can be touched.
    lockDetails(dataType->isDocumentImport());
}

void DataTypePage::selectedStruct(DataTypeStructure* dataType)
{
    activateFields(false);
    mDetails->setEnumRowVisible(false);
    mDetails->setImportRowVisible(false);
    mDetails->setContainerRowVisible(false);

    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockStruct(mDetails->ctrlTypeStruct());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        mDetails->ctrlName()->setText(dataType->getName());
        mDetails->ctrlTypeStruct()->setChecked(true);
        mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), static_cast<DataTypeCustom*>(dataType));

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddChild()->setEnabled(true);
    mList->ctrlButtonInsertChild()->setEnabled(true);
    mList->ctrlButtonRemoveChild()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void DataTypePage::selectedEnum(DataTypeEnum* dataType)
{
    activateFields(false);
    mDetails->setEnumRowVisible(true);
    mDetails->setImportRowVisible(false);
    mDetails->setContainerRowVisible(false);

    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockEnum(mDetails->ctrlTypeEnum());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        const QSignalBlocker blockDerived(mDetails->ctrlEnumDerived());
        mDetails->ctrlName()->setText(dataType->getName());
        mDetails->ctrlTypeEnum()->setChecked(true);
        mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
        mDetails->ctrlEnumDerived()->setCurrentText(dataType->getDerived());
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), static_cast<DataTypeCustom*>(dataType));

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddChild()->setEnabled(true);
    mList->ctrlButtonInsertChild()->setEnabled(true);
    mList->ctrlButtonRemoveChild()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void DataTypePage::selectedImport(DataTypeImported* dataType)
{
    activateFields(false);
    mDetails->setEnumRowVisible(false);
    mDetails->setImportRowVisible(true);
    mDetails->setContainerRowVisible(false);

    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockImport(mDetails->ctrlTypeImport());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        const QSignalBlocker blockLoc(mDetails->ctrlImportLocation());
        const QSignalBlocker blockNs(mDetails->ctrlImportNamespace());
        const QSignalBlocker blockObj(mDetails->ctrlImportObject());
        mDetails->ctrlName()->setText(dataType->getName());
        mDetails->ctrlTypeImport()->setChecked(true);
        mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
        mDetails->ctrlImportLocation()->setText(dataType->getLocation());
        mDetails->ctrlImportNamespace()->setText(dataType->getNamespace());
        mDetails->ctrlImportObject()->setText(dataType->getObject());
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), static_cast<DataTypeCustom*>(dataType));

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddChild()->setEnabled(false);
    mList->ctrlButtonInsertChild()->setEnabled(false);
    mList->ctrlButtonRemoveChild()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void DataTypePage::selectedContainer(DataTypeContainer* dataType)
{
    activateFields(false);
    mDetails->setEnumRowVisible(false);
    mDetails->setImportRowVisible(false);
    mDetails->setContainerRowVisible(true);

    {
        const QSignalBlocker blockName(mDetails->ctrlName());
        const QSignalBlocker blockContainer(mDetails->ctrlTypeContainer());
        const QSignalBlocker blockDescr(mDetails->ctrlDescription());
        const QSignalBlocker blockObject(mDetails->ctrlContainerObject());
        const QSignalBlocker blockKey(mDetails->ctrlContainerKey());
        const QSignalBlocker blockValue(mDetails->ctrlContainerValue());

        // Self-excluding: neither key nor value may reference the container's own type
        populateTypeCombo(mDetails->ctrlContainerKey(), dataType);
        populateTypeCombo(mDetails->ctrlContainerValue(), dataType);

        mDetails->ctrlName()->setText(dataType->getName());
        mDetails->ctrlTypeContainer()->setChecked(true);
        mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
        mDetails->ctrlContainerObject()->setCurrentText(dataType->getContainer());
        mDetails->ctrlContainerValue()->setCurrentText(dataType->getValue());
        mDetails->ctrlContainerKey()->setEnabled(dataType->canHaveKey());
        if (dataType->canHaveKey())
        {
            mDetails->ctrlContainerKey()->setCurrentText(dataType->getKey());
        }
        else
        {
            mDetails->ctrlContainerKey()->setCurrentIndex(-1);
        }
    }
    applyDeprecatedDisplay(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), static_cast<DataTypeCustom*>(dataType));

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddChild()->setEnabled(false);
    mList->ctrlButtonInsertChild()->setEnabled(false);
    mList->ctrlButtonRemoveChild()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void DataTypePage::selectedStructField(DataTypeStructure* parent, uint32_t fieldId)
{
    FieldEntry* field = parent->findElement(fieldId);
    if (field == nullptr)
        return;

    activateFields(true);
    mFields->setTypeRowVisible(true);

    {
        const QSignalBlocker blockName(mFields->ctrlName());
        const QSignalBlocker blockType(mFields->ctrlTypes());
        const QSignalBlocker blockValue(mFields->ctrlValue());
        const QSignalBlocker blockDescr(mFields->ctrlDescription());
        // Must run under the signal blockers: clear() and addItem() fire currentIndexChanged, which
        // would re-enter the model mid-selection and rebuild the tree recursively.
        populateTypeCombo(mFields->ctrlTypes(), parent);
        mFields->ctrlName()->setText(field->getName());
        mFields->ctrlTypes()->setCurrentText(field->getType());
        mFields->ctrlValue()->setText(field->getValue());
        mFields->ctrlDescription()->setPlainText(field->getDescription());
    }
    mFields->showValueHint(validateFieldValue(field->getType(), field->getValue()));
    applyDeprecatedDisplay(mFields->ctrlDeprecated(), mFields->ctrlDeprecateHint(), field);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddChild()->setEnabled(true);
    mList->ctrlButtonInsertChild()->setEnabled(true);
    mList->ctrlButtonRemoveChild()->setEnabled(true);
    updateMoveButtons(parent->findIndex(fieldId), parent->getElementCount());
}

void DataTypePage::selectedEnumField(DataTypeEnum* parent, uint32_t fieldId)
{
    EnumEntry* field = parent->findElement(fieldId);
    if (field == nullptr)
        return;

    activateFields(true);
    mFields->setTypeRowVisible(false);

    {
        const QSignalBlocker blockName(mFields->ctrlName());
        const QSignalBlocker blockValue(mFields->ctrlValue());
        const QSignalBlocker blockDescr(mFields->ctrlDescription());
        mFields->ctrlName()->setText(field->getName());
        mFields->ctrlValue()->setText(field->getValue());
        mFields->ctrlDescription()->setPlainText(field->getDescription());
    }
    // The enumerator's own literal (its ordinal/derived-type value) is a different concern
    // from a structure field's typed default - not validated here; just clear any stale hint.
    mFields->showValueHint(QString());
    applyDeprecatedDisplay(mFields->ctrlDeprecated(), mFields->ctrlDeprecateHint(), field);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddChild()->setEnabled(true);
    mList->ctrlButtonInsertChild()->setEnabled(true);
    mList->ctrlButtonRemoveChild()->setEnabled(true);
    updateMoveButtons(parent->findIndex(fieldId), parent->getElementCount());
}

void DataTypePage::activateFields(bool activate)
{
    if (activate)
    {
        if (mFields->isHidden())
        {
            mDetails->hide();
            mFields->show();
        }
    }
    else
    {
        if (mDetails->isHidden())
        {
            mFields->hide();
            mDetails->show();
        }
    }
}

void DataTypePage::updateMoveButtons(int row, int rowCount)
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

void DataTypePage::showClean(void)
{
    activateFields(false);
    mDetails->setEnumRowVisible(false);
    mDetails->setImportRowVisible(false);
    mDetails->setContainerRowVisible(false);
    applyDeprecatedDisplay<DataTypeCustom>(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);

    const QSignalBlocker blockName(mDetails->ctrlName());
    mDetails->ctrlName()->clear();

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddChild()->setEnabled(false);
    mList->ctrlButtonInsertChild()->setEnabled(false);
    mList->ctrlButtonRemoveChild()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

void DataTypePage::lockDetails(bool locked)
{
    // Greying the two forms out is the whole read-only presentation: the text stays legible, and
    // no widget in them can take a keystroke.
    mDetails->setEnabled(locked == false);
    mFields->setEnabled(locked == false);
    if (locked)
    {
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonAddChild()->setEnabled(false);
        mList->ctrlButtonInsertChild()->setEnabled(false);
        mList->ctrlButtonRemoveChild()->setEnabled(false);
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

QTreeWidgetItem* DataTypePage::createImportNode(const DataTypeDataSection::ImportedTypes& group) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setIcon(static_cast<int>(eColumn::ColName), NELusanCommon::iconLocked(NELusanCommon::SizeSmall));
    item->setText(static_cast<int>(eColumn::ColName), group.space);
    item->setText(static_cast<int>(eColumn::ColType), tr("Imported data types"));
    item->setText(static_cast<int>(eColumn::ColValue), group.location);
    item->setToolTip(static_cast<int>(eColumn::ColName)
                    , tr("Declared in '%1'. Write '%2::' before a name to declare with one of these.")
                        .arg(group.location, group.space));
    item->setData(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole
                 , QVariant::fromValue(static_cast<DataTypeCustom*>(nullptr)));
    item->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, 0u);

    for (DataTypeCustom* type : group.types)
    {
        if (type == nullptr)
            continue;

        QTreeWidgetItem* child = createNode(type);
        child->setIcon(static_cast<int>(eColumn::ColName), NELusanCommon::iconLocked(NELusanCommon::SizeSmall));
        // The spelling the author has to write, which is the one thing a borrowed type has to say
        // for itself that its own document does not.
        child->setText(static_cast<int>(eColumn::ColValue), type->getQualifiedName());
        child->setToolTip(static_cast<int>(eColumn::ColValue), tr("Declared in '%1' and read only here").arg(group.location));
        item->addChild(child);
    }

    return item;
}

QTreeWidgetItem* DataTypePage::createNode(DataTypeCustom* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    setNodeText(item, dataType);
    item->setData(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole, QVariant::fromValue(dataType));
    item->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, 0u);
    item->setToolTip(static_cast<int>(eColumn::ColType), validateDeclaredType(dataType));

    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        for (const FieldEntry& field : static_cast<DataTypeStructure*>(dataType)->getElements())
        {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            setNodeText(child, &field);
            const QString reason = validateFieldValue(field.getType(), field.getValue());
            if (reason.isEmpty() == false)
            {
                child->setIcon(static_cast<int>(eColumn::ColValue), NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
                child->setToolTip(static_cast<int>(eColumn::ColValue), reason);
            }
            child->setToolTip(static_cast<int>(eColumn::ColType), validateDeclaredType(field));
            child->setData(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole, QVariant::fromValue(dataType));
            child->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, field.getId());
            item->addChild(child);
        }
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        for (const EnumEntry& field : static_cast<DataTypeEnum*>(dataType)->getElements())
        {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            setNodeText(child, &field);
            child->setData(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole, QVariant::fromValue(dataType));
            child->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, field.getId());
            item->addChild(child);
        }
    }

    return item;
}

QString DataTypePage::validateFieldValue(const QString& typeName, const QString& value) const
{
    // The same answer the validation engine gives, so the marker on the row and the finding in
    // the results panel can never disagree.
    return DocRuleChecks::literalReason(mModel.getDocument().getDataTypeSection(), typeName, value);
}

QString DataTypePage::validateDeclaredType(const FieldEntry& field)
{
    return (field.getParamType() != nullptr) ? QString() : unknownTypeHint(field.getType());
}

QString DataTypePage::validateDeclaredType(const DataTypeCustom* dataType)
{
    // Asks the same question the row's warning marker answers, so the marker and the tooltip can
    // never disagree: a declared type is either known to the document or it is not.
    if (dataType->getCategory() != DataTypeBase::eCategory::Container)
        return QString();

    const DataTypeContainer* container = static_cast<const DataTypeContainer*>(dataType);
    if (container->canHaveKey() && (container->getKeyDataType() == nullptr))
        return unknownTypeHint(container->getKey());
    if (container->getValueDataType() == nullptr)
        return unknownTypeHint(container->getValue());

    return QString();
}

QString DataTypePage::unknownTypeHint(const QString& typeName)
{
    return typeName.isEmpty()
            ? QObject::tr("No type is chosen yet.")
            : QObject::tr("Type '%1' is not declared in this document.").arg(typeName);
}

void DataTypePage::setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const
{
    // Tree items are not editable by default, unlike table items, so the flag is set for the
    // shared delegate. Which columns actually open is decided per cell by isCellEditable().
    node->setFlags(node->flags() | Qt::ItemIsEditable);

    node->setIcon(static_cast<int>(eColumn::ColName), elem->getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(static_cast<int>(eColumn::ColName), elem->getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(static_cast<int>(eColumn::ColType), elem->getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(static_cast<int>(eColumn::ColType), elem->getString(ElementBase::eDisplay::DisplayType));
    node->setIcon(static_cast<int>(eColumn::ColValue), elem->getIcon(ElementBase::eDisplay::DisplayValue));
    node->setText(static_cast<int>(eColumn::ColValue), elem->getString(ElementBase::eDisplay::DisplayValue));
}

void DataTypePage::refreshAll(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    uint32_t selType = 0;
    uint32_t selField = 0;
    QString  selImported;
    if (QTreeWidgetItem* cur = table->currentItem())
    {
        DataTypeCustom* dataType = cur->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        selType = (dataType != nullptr ? dataType->getId() : 0u);
        selField = cur->data(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole).toUInt();
        // An imported type carries the ID it has in its own document, which says nothing here and
        // may well be a type of this one. Its qualified name is what identifies it across a rebuild.
        if ((dataType != nullptr) && dataType->isDocumentImport())
        {
            selImported = dataType->getQualifiedName();
            selType = 0;
        }
    }

    // A row the author opened stays open across the rebuild, otherwise every edit folds the
    // fields away under the type they belong to.
    QList<uint32_t> openTypes;
    QStringList openImports;
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        if (top->isExpanded() == false)
            continue;

        DataTypeCustom* entry = top->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        if (entry != nullptr)
            openTypes.append(entry->getId());
        else
            openImports.append(top->text(static_cast<int>(eColumn::ColName)));
    }

    {
        const QSignalBlocker blocker(table);
        table->clear();

        // One insert for the whole list. Handed the rows one at a time, the view answers each of
        // them on its own, and the answer gets longer as the list grows.
        QList<QTreeWidgetItem*> rows;
        const QList<DataTypeCustom*>& entries = mModel.getCustomDataTypes();
        rows.reserve(entries.size());
        for (DataTypeCustom* entry : entries)
        {
            rows.append(createNode(entry));
        }

        // Only the documents that resolved: one that does not is reported on the Includes page,
        // where the row the author has to repair lives.
        for (const DataTypeDataSection::ImportedTypes& group : mModel.getImports())
        {
            if (group.isResolved())
            {
                rows.append(createImportNode(group));
            }
        }

        table->addTopLevelItems(rows);

        for (QTreeWidgetItem* row : rows)
        {
            DataTypeCustom* entry = row->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
            row->setExpanded((entry != nullptr) ? openTypes.contains(entry->getId())
                                                : openImports.contains(row->text(static_cast<int>(eColumn::ColName))));
        }
    }

    populateInlineTypeNames();

    if (selImported.isEmpty() == false)
    {
        if (selectImportedType(selImported) == false)
        {
            showClean();
        }
    }
    else if ((selType == 0) || (selectDataType(selType, selField) == false))
    {
        showClean();
    }
}

bool DataTypePage::selectImportedType(const QString& qualifiedName)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        for (int j = 0; j < top->childCount(); ++j)
        {
            QTreeWidgetItem* child = top->child(j);
            DataTypeCustom* dataType = child->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
            if ((dataType != nullptr) && dataType->isDocumentImport() && (dataType->getQualifiedName() == qualifiedName))
            {
                top->setExpanded(true);
                table->setCurrentItem(child);
                return true;
            }
        }
    }

    return false;
}

bool DataTypePage::selectDataType(uint32_t typeId, uint32_t fieldId /*= 0*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        DataTypeCustom* dataType = top->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        if ((dataType == nullptr) || (dataType->getId() != typeId))
            continue;

        if (fieldId == 0)
        {
            table->setCurrentItem(top);
            return true;
        }

        for (int j = 0; j < top->childCount(); ++j)
        {
            QTreeWidgetItem* child = top->child(j);
            if (child->data(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole).toUInt() == fieldId)
            {
                table->setCurrentItem(child);
                return true;
            }
        }

        table->setCurrentItem(top);
        return true;
    }

    return false;
}

void DataTypePage::revealElement(uint32_t id, eIssueField field /*= eIssueField::None*/)
{
    // The list may be waiting for a rebuild the page put off while it was hidden, and what
    // is revealed has to be the row the model holds now.
    flushPendingRefresh();

    // A finding about a structure field or an enumeration entry carries the field's own id, so
    // a type lookup that only tries the top-level rows would come back empty and reveal nothing.
    uint32_t typeId = id;
    uint32_t fieldId = 0;
    if (mModel.findDataType(id) == nullptr)
    {
        for (DataTypeCustom* type : mModel.getCustomDataTypes())
        {
            if ((type != nullptr) && (mModel.findChild(type, id) != nullptr))
            {
                typeId = type->getId();
                fieldId = id;
                break;
            }
        }
    }

    if (selectDataType(typeId, fieldId) == false)
    {
        return;
    }

    if (fieldId != 0)
    {
        switch (field)
        {
        case eIssueField::Name:         WidgetHighlight::reveal(mFields->ctrlName());        break;
        case eIssueField::Type:         WidgetHighlight::reveal(mFields->ctrlTypes());       break;
        case eIssueField::Value:        WidgetHighlight::reveal(mFields->ctrlValue());       break;
        case eIssueField::Description:  WidgetHighlight::reveal(mFields->ctrlDescription()); break;
        default:                                                                             break;
        }
    }
    else
    {
        switch (field)
        {
        case eIssueField::Name:         WidgetHighlight::reveal(mDetails->ctrlName());        break;
        case eIssueField::Description:  WidgetHighlight::reveal(mDetails->ctrlDescription()); break;
        default:                                                                              break;
        }
    }
}

void DataTypePage::populateTypeCombo(QComboBox* combo, const DataTypeCustom* exclude) const
{
    combo->clear();

    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, fieldCategories());
    for (DataTypeBase* type : predefined)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
    }

    for (DataTypeCustom* type : mModel.getCustomDataTypes())
    {
        if (type != exclude)
        {
            combo->addItem(type->getName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
        }
    }

    for (DataTypeCustom* type : mModel.getDataTypeData().getImportedTypes())
    {
        if (type != exclude)
        {
            combo->addItem(type->getQualifiedName(), QVariant::fromValue(static_cast<DataTypeBase*>(type)));
        }
    }
}

void DataTypePage::populateIntegerCombo(QComboBox* combo) const
{
    combo->clear();
    combo->addItem(QString(), QVariant::fromValue(static_cast<DataTypeBase*>(nullptr)));

    QList<DataTypeBase*> integers;
    DataTypeFactory::getPredefinedTypes(integers, integerCategories());
    for (DataTypeBase* type : integers)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
    }
}

void DataTypePage::populateContainerObjectCombo(QComboBox* combo) const
{
    combo->clear();
    for (DataTypeBasicContainer* basic : DataTypeFactory::getContainerTypes())
    {
        combo->addItem(basic->getName(), QVariant::fromValue(static_cast<DataTypeBase*>(basic)));
    }
}

void DataTypePage::populateInlineTypeNames(void)
{
    QStringList fieldNames;
    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, fieldCategories());
    for (DataTypeBase* type : predefined)
    {
        fieldNames.append(type->getName());
    }

    for (DataTypeCustom* type : mModel.getCustomDataTypes())
    {
        fieldNames.append(type->getName());
    }

    for (DataTypeCustom* type : mModel.getDataTypeData().getImportedTypes())
    {
        fieldNames.append(type->getQualifiedName());
    }

    mFieldTypeNames->setStringList(fieldNames);

    QStringList integerNames;
    integerNames.append(QString());
    QList<DataTypeBase*> integers;
    DataTypeFactory::getPredefinedTypes(integers, integerCategories());
    for (DataTypeBase* type : integers)
    {
        integerNames.append(type->getName());
    }

    mIntegerNames->setStringList(integerNames);
}

QString DataTypePage::genTypeName(void)
{
    static const QString _defName("NewDataType");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findDataType(name) != nullptr);

    return name;
}

QString DataTypePage::genFieldName(const DataTypeCustom* dataType) const
{
    static const QString _defName("newField");
    uint32_t count{ 0 };
    QString name;
    do
    {
        name = _defName + QString::number(++count);
    } while (mModel.findChildIndex(dataType, name) != -1);

    return name;
}

void DataTypePage::onAddClicked(void)
{
    addNewType(DataTypeBase::eCategory::Structure);
}

void DataTypePage::addNewType(DataTypeBase::eCategory category)
{
    const QString name = genTypeName();
    DataTypeCustom* dataType = mModel.createDataType(name, category);
    if (dataType != nullptr)
    {
        selectDataType(dataType->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void DataTypePage::onInsertClicked(void)
{
    DataTypeCustom* current = currentDataType();
    const int position = (current != nullptr ? mModel.findIndex(current) : 0);
    const QString name = genTypeName();
    DataTypeCustom* dataType = mModel.insertDataType(position < 0 ? 0 : position, name, DataTypeBase::eCategory::Structure);
    if (dataType != nullptr)
    {
        selectDataType(dataType->getId());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void DataTypePage::focusNameField(void)
{
    QLineEdit* name = (currentFieldId() != 0) ? mFields->ctrlName() : mDetails->ctrlName();
    if (currentDataType() != nullptr)
    {
        name->setFocus();
        name->selectAll();
    }
}

void DataTypePage::onRemoveClicked(void)
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const QList<DataTypeCustom*>& list = mModel.getCustomDataTypes();
    const int index = mModel.findIndex(dataType);
    uint32_t neighborId = 0;
    if (list.size() > 1)
    {
        const int neighborIndex = ((index + 1) < list.size()) ? (index + 1) : (index - 1);
        neighborId = list.at(neighborIndex)->getId();
    }

    mModel.deleteDataType(dataType);
    if (neighborId != 0)
    {
        selectDataType(neighborId);
    }
}

void DataTypePage::onAddFieldClicked(void)
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const QString name = genFieldName(dataType);
    ElementBase* field = mModel.createField(dataType, name);
    if (field != nullptr)
    {
        selectDataType(dataType->getId(), field->getId());
        mFields->ctrlName()->setFocus();
        mFields->ctrlName()->selectAll();
    }
}

void DataTypePage::onInsertFieldClicked(void)
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const uint32_t curFieldId = currentFieldId();
    const int position = (curFieldId != 0 ? mModel.findChildIndex(dataType, curFieldId) : 0);
    const QString name = genFieldName(dataType);
    ElementBase* field = mModel.insertField(dataType, position < 0 ? 0 : position, name);
    if (field != nullptr)
    {
        selectDataType(dataType->getId(), field->getId());
        mFields->ctrlName()->setFocus();
        mFields->ctrlName()->selectAll();
    }
}

void DataTypePage::onRemoveFieldClicked(void)
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType == nullptr) || (fieldId == 0))
        return;

    const int index = mModel.findChildIndex(dataType, fieldId);
    const int count = mModel.getChildCount(dataType);
    uint32_t neighborId = 0;
    if (count > 1)
    {
        const int neighborIndex = ((index + 1) < count) ? (index + 1) : (index - 1);
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            neighborId = mModel.getStructChildren(static_cast<DataTypeStructure*>(dataType)).at(neighborIndex).getId();
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            neighborId = mModel.getEnumChildren(static_cast<DataTypeEnum*>(dataType)).at(neighborIndex).getId();
        }
    }

    mModel.deleteField(dataType, fieldId);
    if (neighborId != 0)
    {
        selectDataType(dataType->getId(), neighborId);
    }
    else
    {
        selectDataType(dataType->getId());
    }
}

void DataTypePage::moveSelection(int delta)
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const uint32_t fieldId = currentFieldId();
    if (fieldId == 0)
    {
        const uint32_t moved = mModel.moveDataType(dataType->getId(), delta);
        if (moved != 0)
        {
            selectDataType(moved);
        }
    }
    else
    {
        const uint32_t moved = mModel.moveField(dataType, fieldId, delta);
        if (moved != 0)
        {
            selectDataType(dataType->getId(), moved);
        }
    }
}

void DataTypePage::onMoveUpClicked(void)
{
    moveSelection(-1);
}

void DataTypePage::onMoveDownClicked(void)
{
    moveSelection(+1);
}

void DataTypePage::onNameCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (currentFieldId() == 0))
    {
        mModel.renameDataType(dataType, mDetails->ctrlName()->text());
    }
}

void DataTypePage::onStructSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Structure);
        selectDataType(converted->getId());
    }
}

void DataTypePage::onEnumSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Enumeration);
        selectDataType(converted->getId());
    }
}

void DataTypePage::onImportSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Imported);
        selectDataType(converted->getId());
    }
}

void DataTypePage::onContainerSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        // DataTypeContainer's default constructor already seeds "Array"/"bool",
        // no extra seeding is needed after conversion
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Container);
        selectDataType(converted->getId());
    }
}

void DataTypePage::onEnumDerivedChanged(int index)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType == nullptr) || (dataType->getCategory() != DataTypeBase::eCategory::Enumeration))
        return;

    DataTypeBase* selected = (index >= 0 ? mDetails->ctrlEnumDerived()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>() : nullptr);
    mModel.setEnumDerived(static_cast<DataTypeEnum*>(dataType), selected != nullptr ? selected->getName() : QString());
}

void DataTypePage::onImportLocationCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
    {
        mModel.setImportLocation(static_cast<DataTypeImported*>(dataType), mDetails->ctrlImportLocation()->text());
    }
}

void DataTypePage::onImportNamespaceCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
    {
        mModel.setImportNamespace(static_cast<DataTypeImported*>(dataType), mDetails->ctrlImportNamespace()->text());
    }
}

void DataTypePage::onImportObjectCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
    {
        mModel.setImportObject(static_cast<DataTypeImported*>(dataType), mDetails->ctrlImportObject()->text());
    }
}

void DataTypePage::onImportBrowse(void)
{
    WorkspaceFileDialog dialog(  true
                               , false
                               , LusanApplication::getWorkspaceDirectories()
                               , LusanApplication::getExternalFileExtensions()
                               , tr("Select Imported File")
                               , this);

    if (mCurUrl.isEmpty())
    {
        mCurUrl = LusanApplication::getWorkspaceDirectories().at(0);
    }

    QString curFile = mDetails->ctrlImportLocation()->text();
    curFile = curFile.isEmpty() ? mCurFile : curFile;

    dialog.setDirectoryUrl(QUrl::fromLocalFile(mCurUrl));
    dialog.setDirectory(mCurUrl);
    if (curFile.isEmpty() == false)
    {
        QFileInfo info(curFile);
        dialog.setDirectory(info.absoluteDir());
        dialog.selectFile(curFile);
    }

    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        mCurUrl = dialog.directoryUrl().path();
        mCurFile = dialog.getSelectedFilePath();

        const QString location{ dialog.getSelectedFileRelativePath() };
        DataTypeCustom* dataType = currentDataType();
        if ((location.isEmpty() == false) && (dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
        {
            mModel.setImportLocation(static_cast<DataTypeImported*>(dataType), location);
            const QSignalBlocker blockLoc(mDetails->ctrlImportLocation());
            mDetails->ctrlImportLocation()->setText(location);
        }
    }
}

void DataTypePage::onContainerObjectChanged(int index)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType == nullptr) || (dataType->getCategory() != DataTypeBase::eCategory::Container) || (index < 0))
        return;

    DataTypeBase* basic = mDetails->ctrlContainerObject()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (basic == nullptr)
        return;

    DataTypeContainer* container = static_cast<DataTypeContainer*>(dataType);
    mModel.setContainerObject(container, basic->getName());

    const QSignalBlocker blockKey(mDetails->ctrlContainerKey());
    mDetails->ctrlContainerKey()->setEnabled(container->canHaveKey());
    if (container->canHaveKey())
    {
        mDetails->ctrlContainerKey()->setCurrentText(container->getKey());
    }
    else
    {
        mDetails->ctrlContainerKey()->setCurrentIndex(-1);
    }
}

void DataTypePage::onContainerKeyChanged(int index)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType == nullptr) || (dataType->getCategory() != DataTypeBase::eCategory::Container) || (index < 0))
        return;

    DataTypeBase* selected = mDetails->ctrlContainerKey()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected != nullptr)
    {
        mModel.setContainerKey(static_cast<DataTypeContainer*>(dataType), selected->getName());
    }
}

void DataTypePage::onContainerValueChanged(int index)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType == nullptr) || (dataType->getCategory() != DataTypeBase::eCategory::Container) || (index < 0))
        return;

    DataTypeBase* selected = mDetails->ctrlContainerValue()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected != nullptr)
    {
        mModel.setContainerValue(static_cast<DataTypeContainer*>(dataType), selected->getName());
    }
}

void DataTypePage::onDeprecatedToggled(bool checked)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (currentFieldId() == 0))
    {
        mModel.setDeprecated(dataType, checked);
        const QSignalBlocker blockHint(mDetails->ctrlDeprecateHint());
        mDetails->ctrlDeprecateHint()->setEnabled(checked);
        mDetails->ctrlDeprecateHint()->setText(checked ? dataType->getDeprecateHint() : QString());
        if (checked)
        {
            mDetails->ctrlDeprecateHint()->setFocus();
        }
    }
}

void DataTypePage::onDeprecateHintCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (currentFieldId() == 0))
    {
        mModel.setDeprecateHint(dataType, mDetails->ctrlDeprecateHint()->text());
    }
}

void DataTypePage::onFieldNameCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldName(dataType, fieldId, mFields->ctrlName()->text());
    }
}

void DataTypePage::onFieldTypeChanged(int index)
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType == nullptr) || (fieldId == 0) || (dataType->getCategory() != DataTypeBase::eCategory::Structure) || (index < 0))
        return;

    DataTypeBase* selected = mFields->ctrlTypes()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    if (selected != nullptr)
    {
        mModel.setFieldType(static_cast<DataTypeStructure*>(dataType), fieldId, selected->getName());
    }
}

void DataTypePage::onFieldValueCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldValue(dataType, fieldId, mFields->ctrlValue()->text());
    }
}

void DataTypePage::onFieldDeprecatedToggled(bool checked)
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldDeprecated(dataType, fieldId, checked);
        ElementBase* field = mModel.findChild(dataType, fieldId);
        QString hint;
        if (checked && (field != nullptr))
        {
            hint = (dataType->getCategory() == DataTypeBase::eCategory::Structure)
                 ? static_cast<FieldEntry*>(field)->getDeprecateHint()
                 : static_cast<EnumEntry*>(field)->getDeprecateHint();
        }

        const QSignalBlocker blockHint(mFields->ctrlDeprecateHint());
        mFields->ctrlDeprecateHint()->setEnabled(checked);
        mFields->ctrlDeprecateHint()->setText(hint);
        if (checked)
        {
            mFields->ctrlDeprecateHint()->setFocus();
        }
    }
}

void DataTypePage::onFieldDeprecateHintCommitted(void)
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldDeprecateHint(dataType, fieldId, mFields->ctrlDeprecateHint()->text());
    }
}

bool DataTypePage::isCellEditable(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return false;

    DataTypeCustom* dataType = index.sibling(index.row(), static_cast<int>(eColumn::ColName)).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if ((dataType == nullptr) || dataType->isDocumentImport())
        return false;

    const uint32_t fieldId = index.sibling(index.row(), static_cast<int>(eColumn::ColType)).data(Qt::ItemDataRole::UserRole).toUInt();
    const int col = index.column();
    if (fieldId == 0)
    {
        // Top-level data type node: which columns are editable depends on the category.
        switch (dataType->getCategory())
        {
        case DataTypeBase::eCategory::Structure:    return (col == static_cast<int>(eColumn::ColName));
        case DataTypeBase::eCategory::Enumeration:  return (col == static_cast<int>(eColumn::ColName)) || (col == static_cast<int>(eColumn::ColType));
        case DataTypeBase::eCategory::Imported:     return (col == static_cast<int>(eColumn::ColName)) || (col == static_cast<int>(eColumn::ColType));
        case DataTypeBase::eCategory::Container:    return (col == static_cast<int>(eColumn::ColName));
        default:                                    return false;
        }
    }

    // Field node: structure fields edit name/type/value; enumeration entries edit name/value.
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        return true;
    if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        return (col == static_cast<int>(eColumn::ColName)) || (col == static_cast<int>(eColumn::ColValue));

    return false;
}

QAbstractItemModel* DataTypePage::editorModelFor(const QModelIndex& index) const
{
    // The Data Type column is a picker in two cases: an enumeration's derived integer type (top
    // node) and a structure field's type. An imported type is typed in, as a qualified name.
    if ((index.isValid() == false) || (index.column() != static_cast<int>(eColumn::ColType)))
        return nullptr;

    DataTypeCustom* dataType = index.sibling(index.row(), static_cast<int>(eColumn::ColName)).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType == nullptr)
        return nullptr;

    const uint32_t fieldId = index.sibling(index.row(), static_cast<int>(eColumn::ColType)).data(Qt::ItemDataRole::UserRole).toUInt();
    if (fieldId == 0)
    {
        return (dataType->getCategory() == DataTypeBase::eCategory::Enumeration ? mIntegerNames : nullptr);
    }

    return (dataType->getCategory() == DataTypeBase::eCategory::Structure ? mFieldTypeNames : nullptr);
}

TableCell::eCellValidation DataTypePage::validationFor(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return TableCell::eCellValidation::NoValidation;

    const int col = index.column();
    if (col == static_cast<int>(eColumn::ColName))
        return TableCell::eCellValidation::Identifier;

    DataTypeCustom* dataType = index.sibling(index.row(), static_cast<int>(eColumn::ColName)).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType == nullptr)
        return TableCell::eCellValidation::NoValidation;

    const uint32_t fieldId = index.sibling(index.row(), static_cast<int>(eColumn::ColType)).data(Qt::ItemDataRole::UserRole).toUInt();
    if (col == static_cast<int>(eColumn::ColType))
    {
        if ((fieldId == 0) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
            return TableCell::eCellValidation::QualifiedName;

        return TableCell::eCellValidation::NoValidation;
    }

    // The value column: an enumerator is restricted to C++ value characters; a structure field's
    // default stays unrestricted, it may be a number, a string literal or an expression.
    if ((fieldId != 0) && (dataType->getCategory() == DataTypeBase::eCategory::Enumeration))
        return TableCell::eCellValidation::Value;

    return TableCell::eCellValidation::NoValidation;
}

void DataTypePage::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if (index.isValid() == false)
        return;

    DataTypeCustom* dataType = index.sibling(index.row(), static_cast<int>(eColumn::ColName)).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if ((dataType == nullptr) || (mModel.findDataType(dataType->getId()) != dataType))
        return;

    const uint32_t fieldId = index.sibling(index.row(), static_cast<int>(eColumn::ColType)).data(Qt::ItemDataRole::UserRole).toUInt();
    applyCellEdit(dataType, fieldId, index.column(), newValue);

    // The commit above rebuilt the tree; put the details panel back on the edited row.
    selectDataType(dataType->getId(), fieldId);
}

void DataTypePage::onEditorTextChanged(const QModelIndex& index, const QString& newText)
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const uint32_t fieldId = currentFieldId();
    const int column = index.column();
    if (fieldId == 0)
    {
        if (column == static_cast<int>(eColumn::ColName))
        {
            // Blocked: the field mirrors its text back into the row whose editor is open.
            const QSignalBlocker blockName(mDetails->ctrlName());
            mDetails->ctrlName()->setText(newText);
        }
    }
    else if (column == static_cast<int>(eColumn::ColName))
    {
        const QSignalBlocker blockName(mFields->ctrlName());
        mFields->ctrlName()->setText(newText);
    }
    else if (column == static_cast<int>(eColumn::ColValue))
    {
        const QSignalBlocker blockValue(mFields->ctrlValue());
        mFields->ctrlValue()->setText(newText);

        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            const FieldEntry* field = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId);
            if (field != nullptr)
            {
                mFields->showValueHint(validateFieldValue(field->getType(), newText));
            }
        }
    }
}

void DataTypePage::onEditorClosed(void)
{
    onCurCellChanged(mList->ctrlTableList()->currentItem(), nullptr);
}

void DataTypePage::applyCellEdit(DataTypeCustom* dataType, uint32_t fieldId, int column, const QString& newValue)
{
    if (fieldId == 0)
    {
        if (column == static_cast<int>(eColumn::ColName))
        {
            mModel.renameDataType(dataType, newValue);
        }
        else if (column == static_cast<int>(eColumn::ColType))
        {
            if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
            {
                mModel.setEnumDerived(static_cast<DataTypeEnum*>(dataType), newValue);
            }
            else if (dataType->getCategory() == DataTypeBase::eCategory::Imported)
            {
                mModel.setImportQualifiedName(static_cast<DataTypeImported*>(dataType), newValue);
            }
        }

        return;
    }

    switch (column)
    {
    case static_cast<int>(eColumn::ColName):
        mModel.setFieldName(dataType, fieldId, newValue);
        break;

    case static_cast<int>(eColumn::ColType):
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            mModel.setFieldType(static_cast<DataTypeStructure*>(dataType), fieldId, newValue);
        }
        break;

    case static_cast<int>(eColumn::ColValue):
        mModel.setFieldValue(dataType, fieldId, newValue);
        break;

    default:
        break;
    }
}

void DataTypePage::onNotifierChanged(void)
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

void DataTypePage::flushPendingRefresh(void)
{
    if (mListPending)
    {
        mListPending = false;
        refreshAll();
    }
}

void DataTypePage::showEvent(QShowEvent* event)
{
    QScrollArea::showEvent(event);
    flushPendingRefresh();
}
