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
 *  \file        lusan/view/sm/SMDataType.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDataType.hpp"

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
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/view/common/DataTypeDetailsView.hpp"
#include "lusan/view/common/DataTypeFieldDetailsView.hpp"
#include "lusan/view/common/DataTypeListView.hpp"
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
#include <QSignalBlocker>
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
}

SMDataType::SMDataType(SMDataTypeModel& model, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mModel            (model)
    , mList             (new DataTypeListView(this))
    , mDetails          (new DataTypeDetailsView(this))
    , mFields           (new DataTypeFieldDetailsView(this))
    , mNameCounter      (0)
    , mCurUrl           ( )
    , mCurFile          ( )
{
    buildUi();
    setupSignals();
    refreshAll();
}

DataTypeListView* SMDataType::getList() const
{
    return mList;
}

void SMDataType::buildUi()
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(tr("State Machine Data Type Editor ..."), content);
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

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(content);
}

void SMDataType::setupSignals()
{
    connect(mList->ctrlTableList()         , &QTreeWidget::currentItemChanged , this, &SMDataType::onCurCellChanged);
    connect(mList->ctrlButtonAdd()         , &QToolButton::clicked            , this, &SMDataType::onAddClicked);
    connect(mList->ctrlButtonInsert()      , &QToolButton::clicked            , this, &SMDataType::onInsertClicked);
    connect(mList->ctrlButtonRemove()      , &QToolButton::clicked            , this, &SMDataType::onRemoveClicked);
    connect(mList->ctrlButtonAddField()    , &QToolButton::clicked            , this, &SMDataType::onAddFieldClicked);
    connect(mList->ctrlButtonInsertField() , &QToolButton::clicked            , this, &SMDataType::onInsertFieldClicked);
    connect(mList->ctrlButtonRemoveField() , &QToolButton::clicked            , this, &SMDataType::onRemoveFieldClicked);
    connect(mList->ctrlButtonMoveUp()      , &QToolButton::clicked            , this, &SMDataType::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown()    , &QToolButton::clicked            , this, &SMDataType::onMoveDownClicked);
    connect(mList->actionNewStruct()       , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Structure); });
    connect(mList->actionNewEnum()         , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Enumeration); });
    connect(mList->actionNewImport()       , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Imported); });
    connect(mList->actionNewContainer()    , &QAction::triggered              , this, [this]() { addNewType(DataTypeBase::eCategory::Container); });

    // The identifier validator is installed inside the shared DataTypeDetailsView.
    connect(mDetails->ctrlName()            , &QLineEdit::editingFinished      , this, &SMDataType::onNameCommitted);
    // Live-preview the typed name into the selected type's Name column (no command; the real
    // rename commits on editingFinished). Selection sets the field under a QSignalBlocker, so
    // this only fires for genuine user edits.
    connect(mDetails->ctrlName()            , &QLineEdit::textChanged          , this, [this](const QString& text) {
        if ((currentFieldId() == 0) && (currentDataType() != nullptr))
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(0, text);
        }
    });
    connect(mDetails->ctrlTypeStruct()      , &QRadioButton::clicked           , this, &SMDataType::onStructSelected);
    connect(mDetails->ctrlTypeEnum()        , &QRadioButton::clicked           , this, &SMDataType::onEnumSelected);
    connect(mDetails->ctrlTypeImport()      , &QRadioButton::clicked           , this, &SMDataType::onImportSelected);
    connect(mDetails->ctrlTypeContainer()   , &QRadioButton::clicked           , this, &SMDataType::onContainerSelected);
    connect(mDetails->ctrlEnumDerived()     , &QComboBox::currentIndexChanged  , this, &SMDataType::onEnumDerivedChanged);
    connect(mDetails->ctrlImportLocation()  , &QLineEdit::editingFinished      , this, &SMDataType::onImportLocationCommitted);
    connect(mDetails->ctrlImportNamespace() , &QLineEdit::editingFinished      , this, &SMDataType::onImportNamespaceCommitted);
    connect(mDetails->ctrlImportObject()    , &QLineEdit::editingFinished      , this, &SMDataType::onImportObjectCommitted);
    connect(mDetails->ctrlButtonBrowse()    , &QPushButton::clicked            , this, &SMDataType::onImportBrowse);
    connect(mDetails->ctrlContainerObject() , &QComboBox::currentIndexChanged  , this, &SMDataType::onContainerObjectChanged);
    connect(mDetails->ctrlContainerKey()    , &QComboBox::currentIndexChanged  , this, &SMDataType::onContainerKeyChanged);
    connect(mDetails->ctrlContainerValue()  , &QComboBox::currentIndexChanged  , this, &SMDataType::onContainerValueChanged);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled              , this, &SMDataType::onDeprecatedToggled);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::editingFinished      , this, &SMDataType::onDeprecateHintCommitted);
    mDetails->ctrlDescription()->installEventFilter(this);

    // The identifier validator is installed inside the shared DataTypeFieldDetailsView.
    connect(mFields->ctrlName()             , &QLineEdit::editingFinished      , this, &SMDataType::onFieldNameCommitted);
    connect(mFields->ctrlName()             , &QLineEdit::textChanged          , this, [this](const QString& text) {
        if (currentFieldId() != 0)
        {
            if (QTreeWidgetItem* item = mList->ctrlTableList()->currentItem())
                item->setText(0, text);
        }
    });
    connect(mFields->ctrlTypes()            , &QComboBox::currentIndexChanged  , this, &SMDataType::onFieldTypeChanged);
    connect(mFields->ctrlValue()            , &QLineEdit::editingFinished      , this, &SMDataType::onFieldValueCommitted);
    connect(mFields->ctrlDeprecated()       , &QCheckBox::toggled              , this, &SMDataType::onFieldDeprecatedToggled);
    connect(mFields->ctrlDeprecateHint()    , &QLineEdit::editingFinished      , this, &SMDataType::onFieldDeprecateHintCommitted);
    mFields->ctrlDescription()->installEventFilter(this);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMDataType::onNotifierChanged);
    connect(&mModel.getNotifier(), &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
    connect(&mModel.getNotifier(), &DocModelNotifier::listReordered, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) onNotifierChanged(); });
}

bool SMDataType::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        if (watched == mDetails->ctrlDescription())
        {
            if (DataTypeCustom* dataType = currentDataType())
            {
                if (currentFieldId() == 0)
                    mModel.setDescription(dataType, mDetails->ctrlDescription()->toPlainText());
            }
        }
        else if (watched == mFields->ctrlDescription())
        {
            if (DataTypeCustom* dataType = currentDataType())
            {
                const uint32_t fieldId = currentFieldId();
                if (fieldId != 0)
                    mModel.setFieldDescription(dataType, fieldId, mFields->ctrlDescription()->toPlainText());
            }
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

DataTypeCustom* SMDataType::currentDataType() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr);
}

uint32_t SMDataType::currentFieldId() const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    return (item != nullptr ? item->data(1, Qt::ItemDataRole::UserRole).toUInt() : 0u);
}

void SMDataType::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current == nullptr)
    {
        showClean();
        return;
    }

    DataTypeCustom* dataType = current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    const uint32_t fieldId = current->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if (dataType == nullptr)
    {
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
}

void SMDataType::selectedStruct(DataTypeStructure* dataType)
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
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void SMDataType::selectedEnum(DataTypeEnum* dataType)
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
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void SMDataType::selectedImport(DataTypeImported* dataType)
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
    mList->ctrlButtonAddField()->setEnabled(false);
    mList->ctrlButtonInsertField()->setEnabled(false);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void SMDataType::selectedContainer(DataTypeContainer* dataType)
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
    mList->ctrlButtonAddField()->setEnabled(false);
    mList->ctrlButtonInsertField()->setEnabled(false);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    updateMoveButtons(mModel.findIndex(dataType), mModel.getDataTypeCount());
}

void SMDataType::selectedStructField(DataTypeStructure* parent, uint32_t fieldId)
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
        // Must run under the signal blockers: combo->clear()/addItem() fire live
        // currentIndexChanged signals that would otherwise re-enter the model mid-selection
        // (onFieldTypeChanged -> a command -> notifier -> refreshAll() rebuilding the
        // QTreeWidget while this very selection is in progress, i.e. unbounded recursion).
        populateTypeCombo(mFields->ctrlTypes(), parent);
        mFields->ctrlName()->setText(field->getName());
        mFields->ctrlTypes()->setCurrentText(field->getType());
        mFields->ctrlValue()->setText(field->getValue());
        mFields->ctrlDescription()->setPlainText(field->getDescription());
    }
    mFields->showValueHint(validateFieldValue(field->getType(), field->getValue()));
    applyDeprecatedDisplay(mFields->ctrlDeprecated(), mFields->ctrlDeprecateHint(), field);

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);
    mList->ctrlButtonRemoveField()->setEnabled(true);
    updateMoveButtons(parent->findIndex(fieldId), parent->getElementCount());
}

void SMDataType::selectedEnumField(DataTypeEnum* parent, uint32_t fieldId)
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
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);
    mList->ctrlButtonRemoveField()->setEnabled(true);
    updateMoveButtons(parent->findIndex(fieldId), parent->getElementCount());
}

void SMDataType::activateFields(bool activate)
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

void SMDataType::updateMoveButtons(int row, int rowCount)
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

void SMDataType::showClean()
{
    activateFields(false);
    mDetails->setEnumRowVisible(false);
    mDetails->setImportRowVisible(false);
    mDetails->setContainerRowVisible(false);
    applyDeprecatedDisplay<DataTypeCustom>(mDetails->ctrlDeprecated(), mDetails->ctrlDeprecateHint(), nullptr);

    const QSignalBlocker blockName(mDetails->ctrlName());
    mDetails->ctrlName()->clear();

    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddField()->setEnabled(false);
    mList->ctrlButtonInsertField()->setEnabled(false);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
}

QTreeWidgetItem* SMDataType::createNode(DataTypeCustom* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    setNodeText(item, dataType);
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(dataType));
    item->setData(1, Qt::ItemDataRole::UserRole, 0u);

    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        for (const FieldEntry& field : static_cast<DataTypeStructure*>(dataType)->getElements())
        {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            setNodeText(child, &field);
            const QString reason = validateFieldValue(field.getType(), field.getValue());
            if (reason.isEmpty() == false)
            {
                child->setIcon(2, NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
                child->setToolTip(2, reason);
            }
            child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(dataType));
            child->setData(1, Qt::ItemDataRole::UserRole, field.getId());
            item->addChild(child);
        }
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        for (const EnumEntry& field : static_cast<DataTypeEnum*>(dataType)->getElements())
        {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            setNodeText(child, &field);
            child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(dataType));
            child->setData(1, Qt::ItemDataRole::UserRole, field.getId());
            item->addChild(child);
        }
    }

    return item;
}

QString SMDataType::validateFieldValue(const QString& typeName, const QString& value) const
{
    // No default is always valid, and a declared (enum/structure/container/imported) type is
    // ignored - its literal form, if any, is not this validator's concern.
    if (value.isEmpty() || (mModel.findDataType(typeName) != nullptr))
        return QString();

    return SMLiteralValidator::validate(typeName, value);
}

void SMDataType::setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const
{
    node->setIcon(0, elem->getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, elem->getString(ElementBase::eDisplay::DisplayName));
    node->setIcon(1, elem->getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(1, elem->getString(ElementBase::eDisplay::DisplayType));
    node->setIcon(2, elem->getIcon(ElementBase::eDisplay::DisplayValue));
    node->setText(2, elem->getString(ElementBase::eDisplay::DisplayValue));
}

void SMDataType::refreshAll()
{
    QTreeWidget* table = mList->ctrlTableList();
    uint32_t selType = 0;
    uint32_t selField = 0;
    if (QTreeWidgetItem* cur = table->currentItem())
    {
        DataTypeCustom* dataType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        selType = (dataType != nullptr ? dataType->getId() : 0u);
        selField = cur->data(1, Qt::ItemDataRole::UserRole).toUInt();
    }

    {
        const QSignalBlocker blocker(table);
        table->clear();
        for (DataTypeCustom* entry : mModel.getCustomDataTypes())
        {
            table->addTopLevelItem(createNode(entry));
        }
    }

    if ((selType == 0) || (selectDataType(selType, selField) == false))
    {
        showClean();
    }
}

bool SMDataType::selectDataType(uint32_t typeId, uint32_t fieldId /*= 0*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    for (int i = 0; i < table->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        DataTypeCustom* dataType = top->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
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
            if (child->data(1, Qt::ItemDataRole::UserRole).toUInt() == fieldId)
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

void SMDataType::populateTypeCombo(QComboBox* combo, const DataTypeCustom* exclude) const
{
    combo->clear();

    QList<DataTypeBase*> predefined;
    DataTypeFactory::getPredefinedTypes(predefined, QList<DataTypeBase::eCategory>{
          DataTypeBase::eCategory::Primitive
        , DataTypeBase::eCategory::PrimitiveSint
        , DataTypeBase::eCategory::PrimitiveUint
        , DataTypeBase::eCategory::PrimitiveFloat
        , DataTypeBase::eCategory::BasicObject
    });

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
}

void SMDataType::populateIntegerCombo(QComboBox* combo) const
{
    combo->clear();
    combo->addItem(QString(), QVariant::fromValue(static_cast<DataTypeBase*>(nullptr)));

    QList<DataTypeBase*> integers;
    DataTypeFactory::getPredefinedTypes(integers, QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::PrimitiveSint, DataTypeBase::eCategory::PrimitiveUint});
    for (DataTypeBase* type : integers)
    {
        combo->addItem(type->getName(), QVariant::fromValue(type));
    }
}

void SMDataType::populateContainerObjectCombo(QComboBox* combo) const
{
    combo->clear();
    for (DataTypeBasicContainer* basic : DataTypeFactory::getContainerTypes())
    {
        combo->addItem(basic->getName(), QVariant::fromValue(static_cast<DataTypeBase*>(basic)));
    }
}

QString SMDataType::genTypeName()
{
    static const QString _defName("NewDataType");
    QString name;
    do
    {
        name = _defName + QString::number(++mNameCounter);
    } while (mModel.findDataType(name) != nullptr);

    return name;
}

QString SMDataType::genFieldName(const DataTypeCustom* dataType) const
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

void SMDataType::onAddClicked()
{
    addNewType(DataTypeBase::eCategory::Structure);
}

void SMDataType::addNewType(DataTypeBase::eCategory category)
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

void SMDataType::onInsertClicked()
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

void SMDataType::onRemoveClicked()
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

void SMDataType::onAddFieldClicked()
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

void SMDataType::onInsertFieldClicked()
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

void SMDataType::onRemoveFieldClicked()
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

void SMDataType::onMoveUpClicked()
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const uint32_t fieldId = currentFieldId();
    if (fieldId == 0)
    {
        const int index = mModel.findIndex(dataType);
        if (index > 0)
        {
            const uint32_t neighborId = mModel.getCustomDataTypes().at(index - 1)->getId();
            mModel.swapDataTypes(dataType->getId(), neighborId);
            selectDataType(dataType->getId());
        }
    }
    else
    {
        const int index = mModel.findChildIndex(dataType, fieldId);
        if (index > 0)
        {
            uint32_t neighborId = 0;
            if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
                neighborId = mModel.getStructChildren(static_cast<DataTypeStructure*>(dataType)).at(index - 1).getId();
            else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
                neighborId = mModel.getEnumChildren(static_cast<DataTypeEnum*>(dataType)).at(index - 1).getId();

            mModel.swapFields(dataType, fieldId, neighborId);
            selectDataType(dataType->getId(), fieldId);
        }
    }
}

void SMDataType::onMoveDownClicked()
{
    DataTypeCustom* dataType = currentDataType();
    if (dataType == nullptr)
        return;

    const uint32_t fieldId = currentFieldId();
    if (fieldId == 0)
    {
        const int index = mModel.findIndex(dataType);
        if ((index >= 0) && (index < (mModel.getDataTypeCount() - 1)))
        {
            const uint32_t neighborId = mModel.getCustomDataTypes().at(index + 1)->getId();
            mModel.swapDataTypes(dataType->getId(), neighborId);
            selectDataType(dataType->getId());
        }
    }
    else
    {
        const int index = mModel.findChildIndex(dataType, fieldId);
        const int count = mModel.getChildCount(dataType);
        if ((index >= 0) && (index < (count - 1)))
        {
            uint32_t neighborId = 0;
            if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
                neighborId = mModel.getStructChildren(static_cast<DataTypeStructure*>(dataType)).at(index + 1).getId();
            else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
                neighborId = mModel.getEnumChildren(static_cast<DataTypeEnum*>(dataType)).at(index + 1).getId();

            mModel.swapFields(dataType, fieldId, neighborId);
            selectDataType(dataType->getId(), fieldId);
        }
    }
}

void SMDataType::onNameCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (currentFieldId() == 0))
    {
        mModel.renameDataType(dataType, mDetails->ctrlName()->text());
    }
}

void SMDataType::onStructSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Structure);
        selectDataType(converted->getId());
    }
}

void SMDataType::onEnumSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Enumeration);
        selectDataType(converted->getId());
    }
}

void SMDataType::onImportSelected(bool checked)
{
    if (!checked)
        return;

    if (DataTypeCustom* dataType = currentDataType())
    {
        DataTypeCustom* converted = mModel.convertDataType(dataType, DataTypeBase::eCategory::Imported);
        selectDataType(converted->getId());
    }
}

void SMDataType::onContainerSelected(bool checked)
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

void SMDataType::onEnumDerivedChanged(int index)
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType == nullptr) || (dataType->getCategory() != DataTypeBase::eCategory::Enumeration))
        return;

    DataTypeBase* selected = (index >= 0 ? mDetails->ctrlEnumDerived()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>() : nullptr);
    mModel.setEnumDerived(static_cast<DataTypeEnum*>(dataType), selected != nullptr ? selected->getName() : QString());
}

void SMDataType::onImportLocationCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
    {
        mModel.setImportLocation(static_cast<DataTypeImported*>(dataType), mDetails->ctrlImportLocation()->text());
    }
}

void SMDataType::onImportNamespaceCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
    {
        mModel.setImportNamespace(static_cast<DataTypeImported*>(dataType), mDetails->ctrlImportNamespace()->text());
    }
}

void SMDataType::onImportObjectCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
    {
        mModel.setImportObject(static_cast<DataTypeImported*>(dataType), mDetails->ctrlImportObject()->text());
    }
}

void SMDataType::onImportBrowse()
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

        DataTypeCustom* dataType = currentDataType();
        if ((dataType != nullptr) && (dataType->getCategory() == DataTypeBase::eCategory::Imported))
        {
            mModel.setImportLocation(static_cast<DataTypeImported*>(dataType), dialog.getSelectedFileRelativePath());
            const QSignalBlocker blockLoc(mDetails->ctrlImportLocation());
            mDetails->ctrlImportLocation()->setText(dialog.getSelectedFileRelativePath());
        }
    }
}

void SMDataType::onContainerObjectChanged(int index)
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

void SMDataType::onContainerKeyChanged(int index)
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

void SMDataType::onContainerValueChanged(int index)
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

void SMDataType::onDeprecatedToggled(bool checked)
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

void SMDataType::onDeprecateHintCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    if ((dataType != nullptr) && (currentFieldId() == 0))
    {
        mModel.setDeprecateHint(dataType, mDetails->ctrlDeprecateHint()->text());
    }
}

void SMDataType::onFieldNameCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldName(dataType, fieldId, mFields->ctrlName()->text());
    }
}

void SMDataType::onFieldTypeChanged(int index)
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

void SMDataType::onFieldValueCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldValue(dataType, fieldId, mFields->ctrlValue()->text());
    }
}

void SMDataType::onFieldDeprecatedToggled(bool checked)
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

void SMDataType::onFieldDeprecateHintCommitted()
{
    DataTypeCustom* dataType = currentDataType();
    const uint32_t fieldId = currentFieldId();
    if ((dataType != nullptr) && (fieldId != 0))
    {
        mModel.setFieldDeprecateHint(dataType, fieldId, mFields->ctrlDeprecateHint()->text());
    }
}

void SMDataType::onNotifierChanged()
{
    refreshAll();
}
