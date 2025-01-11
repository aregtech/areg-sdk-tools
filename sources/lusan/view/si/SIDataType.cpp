/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIDataType.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIDataType.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataType.h"

#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/model/si/SIDataTypeModel.hpp"
#include "lusan/view/si/SIDataTypeDetails.hpp"
#include "lusan/view/si/SIDataTypeFieldDetails.hpp"
#include "lusan/view/si/SIDataTypeList.hpp"
#include "lusan/model/common/DataTypesModel.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpacerItem>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

SIDataTypeWidget::SIDataTypeWidget(QWidget* parent)
    : QWidget{ parent }
    , ui     (new Ui::SIDataType)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

const QList<DataTypeBasicContainer *> & SIDataType::_getContainerTypes(void)
{
    static QList<DataTypeBasicContainer *> _result;
    if (_result.isEmpty())
    {
        _result = DataTypeFactory::getContainerTypes();
    }
    
    return _result;
}

const QList<DataTypeBase *>& SIDataType::_getIntegerTypes(void)
{
    static QList<DataTypeBase *> _result;
    if (_result.isEmpty())
    {
        DataTypeFactory::getPredefinedTypes(_result, QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::PrimitiveSint, DataTypeBase::eCategory::PrimitiveUint});
    }
    
    return _result;
}

const QList<DataTypeBase *>& SIDataType::_getPredefinedTypes(void)
{
    static QList<DataTypeBase *> _result;
    if (_result.isEmpty())
    {
        DataTypeFactory::getPredefinedTypes(  _result
                                            , QList<DataTypeBase::eCategory>{ DataTypeBase::eCategory::Primitive
                                                                            , DataTypeBase::eCategory::PrimitiveSint
                                                                            , DataTypeBase::eCategory::PrimitiveUint
                                                                            , DataTypeBase::eCategory::PrimitiveFloat
                                                                            , DataTypeBase::eCategory::BasicObject
                                                     });
    }
    
    return _result;
}


SIDataType::SIDataType(SIDataTypeModel& model, QWidget *parent)
    : QScrollArea   (parent)
    , mDetails  (new SIDataTypeDetails(this))
    , mList     (new SIDataTypeList(this))
    , mFields   (new SIDataTypeFieldDetails(this))
    , mWidget   (new SIDataTypeWidget(this))
    , ui        (*mWidget->ui)
    , mModel    (model)

    , mCount    (0)
{
    mFields->setHidden(true);
    
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);
    ui.horizontalLayout->addWidget(mFields);
    
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);

    updateData();
    updateWidgets();
    setupSignals();
}

SIDataType::~SIDataType(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
    ui.horizontalLayout->removeWidget(mFields);
}

void SIDataType::closeEvent(QCloseEvent *event)
{
    QScrollArea::closeEvent(event);
}

void SIDataType::hideEvent(QHideEvent *event)
{
    QScrollArea::hideEvent(event);
}

void SIDataType::onCurCellChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if ((current == nullptr) || (current == previous))
        return;
    
    DataTypeCustom* oldType  = previous != nullptr ? previous->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    DataTypeCustom* dataType = current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = current->data(1, Qt::ItemDataRole::UserRole).toUInt();
    
    if (id == 0)
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
            selectedContainer(static_cast<DataTypeDefined*>(dataType));
            break;

        default:
            break;
        }
    }
    else
    {
        Q_ASSERT((dataType->getCategory() == DataTypeBase::eCategory::Structure) || (dataType->getCategory() == DataTypeBase::eCategory::Enumeration));
        ElementBase * field = mModel.findChild(dataType, id);
        Q_ASSERT(field != nullptr);

        blockBasicSignals(true);
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            selectedStructField(*static_cast<FieldEntry*>(field), static_cast<DataTypeStructure*>(dataType));
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            selectedEnumField(*static_cast<EnumEntry*>(field), static_cast<DataTypeEnum*>(dataType));
        }

        blockBasicSignals(false);
    }
}

void SIDataType::onAddClicked(void)
{
    static const QString _defName("NewDataType");

    QTreeWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (mModel.findDataType(name) != nullptr);

    blockBasicSignals(true);
    
    DataTypeCustom* dataType = mModel.createDataType(name, DataTypeBase::eCategory::Structure);
    int pos = table->topLevelItemCount();
    QTreeWidgetItem * item = createNodeStructure(static_cast<DataTypeStructure *>(dataType));
    table->insertTopLevelItem(pos, item);
    QTreeWidgetItem * cur = table->currentItem();
    if (cur != nullptr)
    {
        cur->setSelected(false);
    }
    
    item->setSelected(true);
    table->setCurrentItem(item);
    selectedStruct(static_cast<DataTypeStructure*>(dataType));
    blockBasicSignals(false);
}

void SIDataType::onAddFieldClicked(void)
{
    static const QString _defName("newField");
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* cur = table->currentItem();
    Q_ASSERT(cur != nullptr);
    QTreeWidgetItem* parent = cur->parent();
    parent = parent == nullptr ? cur : parent;
    
    DataTypeCustom* dataType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>();
    Q_ASSERT((dataType->getCategory() == DataTypeBase::eCategory::Structure) || (dataType->getCategory() == DataTypeBase::eCategory::Enumeration));
    
    uint32_t cnt {0};    
    QString name;
    do
    {
        name = _defName + QString::number(++cnt);
    } while (mModel.findChildIndex(dataType, name) != -1);

    ElementBase* field = mModel.ceateDataTypeChild(dataType, name);
    if (field != nullptr)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        
        parent->addChild(item);
        if (parent->isExpanded() == false)
        {
            parent->setExpanded(true);
        }
        
        if (cur->isSelected())
        {
            cur->setSelected(false);
        }
        
        table->setCurrentItem(item);
        item->setSelected(true);
        
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            updateChildNodeStruct(item, static_cast<DataTypeStructure*>(dataType), *static_cast<FieldEntry *>(field));
            selectedStructField(*static_cast<FieldEntry *>(field), static_cast<DataTypeStructure*>(dataType));
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            updateChildNodeEnum(item, static_cast<DataTypeEnum*>(dataType), *static_cast<EnumEntry*>(field));
            selectedEnumField(*static_cast<EnumEntry*>(field), static_cast<DataTypeEnum*>(dataType));
        }
        
        blockBasicSignals(false);
    }
}

void SIDataType::onRemoveClicked(void)
{
}

void SIDataType::onTypeNameChanged(const QString& newName)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;
    
    DataTypeCustom* dataType = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    Q_ASSERT(dataType != nullptr);
    dataType->setName(newName);
    item->setText(0, newName);
}

void SIDataType::onFieldNameChanged(const QString& newName)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;
    
    DataTypeCustom* dataType = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    ElementBase* field = mModel.findChild(dataType, id);
    Q_ASSERT(field != nullptr);
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        static_cast<FieldEntry*>(field)->setName(newName);
        item->setText(0, newName);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        static_cast<EnumEntry*>(field)->setName(newName);
        item->setText(0, newName);
    }
}

void SIDataType::onDeprectedChecked(bool isChecked)
{
}

void SIDataType::onDeprecateHintChanged(const QString& newText)
{
}

void SIDataType::updateData(void)
{
    const QList<DataTypeCustom*>& list = mModel.getDataTypes();
    QTreeWidget* table = mList->ctrlTableList();
    int count = 0;
    for (DataTypeCustom* entry : list)
    {
        QTreeWidgetItem* item{ nullptr };
        switch (entry->getCategory())
        {
        case DataTypeBase::eCategory::Structure:
            item = createNodeStructure(static_cast<DataTypeStructure*>(entry));
            break;

        case DataTypeBase::eCategory::Enumeration:
            item = createNodeEnum(static_cast<DataTypeEnum*>(entry));
            break;

        case DataTypeBase::eCategory::Imported:
            item = createNodeImported(static_cast<DataTypeImported*>(entry));
            break;

        case DataTypeBase::eCategory::Container:
            item = createNodeContainer(static_cast<DataTypeDefined*>(entry));
            break;

        default:
            break;
        }

        if (item != nullptr)
        {
            table->insertTopLevelItem(count++, item);
        }
    }
}

void SIDataType::onDescriptionChanged(void)
{
}

void SIDataType::onStructSelected(bool checked)
{
    if (checked)
    {
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        onConvertDataType(item, DataTypeBase::eCategory::Structure);
    }
}

void SIDataType::onEnumSelected(bool checked)
{
    if (checked)
    {
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        onConvertDataType(item, DataTypeBase::eCategory::Enumeration);
    }
}

void SIDataType::onImportSelected(bool checked)
{
    if (checked)
    {
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        onConvertDataType(item, DataTypeBase::eCategory::Imported);
    }
}

void SIDataType::onContainerSelected(bool checked)
{
    if (checked)
    {
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        onConvertDataType(item, DataTypeBase::eCategory::Container);
    }
}

void SIDataType::onConvertDataType(QTreeWidgetItem* current, DataTypeBase::eCategory newCategory)
{
    Q_ASSERT(current != nullptr);
    Q_ASSERT(current->parent() == nullptr);

    DataTypeCustom* dataType = current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = current->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if ((id == 0) && (dataType->getCategory() != newCategory))
    {
        blockBasicSignals(true);
        current->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom*>(nullptr)));
        switch (newCategory)
        {
        case DataTypeBase::eCategory::Structure:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Structure);
            Q_ASSERT(static_cast<DataTypeStructure*>(dataType)->hasElements() == false);
            updateNodeStructure(current, static_cast<DataTypeStructure*>(dataType));
            selectedStruct(static_cast<DataTypeStructure*>(dataType));
            break;

        case DataTypeBase::eCategory::Enumeration:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Enumeration);
            Q_ASSERT(static_cast<DataTypeEnum*>(dataType)->hasElements() == false);
            updateNodeEnum(current, static_cast<DataTypeEnum*>(dataType));
            selectedEnum(static_cast<DataTypeEnum*>(dataType));
            break;

        case DataTypeBase::eCategory::Imported:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Imported);
            updateNodeImported(current, static_cast<DataTypeImported*>(dataType));
            selectedImport(static_cast<DataTypeImported*>(dataType));
            break;

        case DataTypeBase::eCategory::Container:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Container);
            updateNodeContainer(current, static_cast<DataTypeDefined*>(dataType));
            selectedContainer(static_cast<DataTypeDefined*>(dataType));
            break;

        default:
            break;
        }
        
        blockBasicSignals(false);
    }
}

void SIDataType::onContainerObjectChanged(int index)
{
    QComboBox* container = mDetails->ctrlContainerObject();
    if ((index < 0) || (index >= container->count()))
    {
        return;
    }
    
    DataTypeBasicContainer*  dataType = static_cast<DataTypeBasicContainer *>(container->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>());
    Q_ASSERT(dataType != nullptr);
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeDefined* typeContainer = static_cast<DataTypeDefined*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>());
    Q_ASSERT(typeContainer->getCategory() == DataTypeBase::eCategory::Container);
    typeContainer->setContainer(dataType->getName());
    mDetails->ctrlContainerKey()->setEnabled(dataType->hasKey());
}

void SIDataType::onContainerKeyChanged(int index)
{
    QComboBox* key = mDetails->ctrlContainerKey();
    if ((index < 0) || (index >= key->count()))
    {
        return;
    }
    
    DataTypeBase*  dataType = key->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    Q_ASSERT(dataType != nullptr);
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeDefined* typeContainer = static_cast<DataTypeDefined*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>());
    Q_ASSERT(typeContainer->getCategory() == DataTypeBase::eCategory::Container);
    typeContainer->setKey(dataType->getName());
}

void SIDataType::onContainerValueChanged(int index)
{
    QComboBox* value = mDetails->ctrlContainerValue();
    if ((index < 0) || (index >= value->count()))
    {
        return;
    }
    
    DataTypeBase*  dataType = value->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    Q_ASSERT(dataType != nullptr);
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeDefined* typeContainer = static_cast<DataTypeDefined*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>());
    Q_ASSERT(typeContainer->getCategory() == DataTypeBase::eCategory::Container);
    typeContainer->setKey(dataType->getName());
}

void SIDataType::showEnumDetails(bool show)
{
    static constexpr int _space{180};
    SIDataTypeDetails::CtrlGroup item{mDetails->ctrlDetailsEnum()};
    mDetails->changeSpace((show ? -1 : 1) * _space);
    item.first->setHidden(!show);
    item.second->setHidden(!show);
}

void SIDataType::showImportDetails(bool show)
{
    static constexpr int _space{120};
    SIDataTypeDetails::CtrlGroup item{mDetails->ctrlDetailsImport()};
    mDetails->changeSpace((show ? -1 : 1) * _space);
    item.first->setHidden(!show);
    item.second->setHidden(!show);
}

void SIDataType::showContainerDetails(bool show)
{
    static constexpr int _space{60};
    SIDataTypeDetails::CtrlGroup item{mDetails->ctrlDetailsContainer()};
    mDetails->changeSpace((show ? -1 : 1) * _space);
    item.first->setHidden(!show);
    item.second->setHidden(!show);
}

void SIDataType::updateWidgets(void)
{
    // mDetails->ctrlContainerKey()->setModel(mModelKey);
    // mDetails->ctrlContainerValue()->setModel(mModelValue);
    // mFields->ctrlTypes()->setModel(mModelFields);
    
    QComboBox* container = mDetails->ctrlContainerObject();
    const QList<DataTypeBasicContainer*>& containers {_getContainerTypes()};
    for (auto dataType : containers)
    {
        container->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
    }
    
    QComboBox* enumDerive = mDetails->ctrlEnumDerived();
    const QList<DataTypeBase*>& integers{_getIntegerTypes()};
    for (auto dataType : integers)
    {
        enumDerive->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
    }
    
    const QList<DataTypeBase *> predefined {_getPredefinedTypes()};
    QComboBox* types = mFields->ctrlTypes();
    for (auto dataType : predefined)
    {
        types->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
    }
    
    QComboBox* keys = mDetails->ctrlContainerKey();
    QComboBox* values = mDetails->ctrlContainerValue();
    for (auto dataType : predefined)
    {
        keys->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
        values->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
    }
    
    showEnumDetails(false);
    showContainerDetails(false);
    showImportDetails(false);
    
    mList->ctrlToolRemove()->setEnabled(false);
    mList->ctrlToolAddField()->setEnabled(false);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(false);
    mList->ctrlToolMoveUp()->setEnabled(false);
    mList->ctrlToolMoveDown()->setEnabled(false);
}

void SIDataType::setupSignals(void)
{
    connect(mList->ctrlTableList()          , &QTreeWidget::currentItemChanged  , this, &SIDataType::onCurCellChanged);
    connect(mList->ctrlToolAdd()            , &QToolButton::clicked             , this, &SIDataType::onAddClicked);
    connect(mList->ctrlToolAddField()       , &QToolButton::clicked             , this, &SIDataType::onAddFieldClicked);
    connect(mDetails->ctrlName()            , &QLineEdit::textChanged           , this, &SIDataType::onTypeNameChanged);
    connect(mFields->ctrlName()             , &QLineEdit::textChanged           , this, &SIDataType::onFieldNameChanged);
    connect(mDetails->ctrlTypeStruct()      , &QRadioButton::clicked            , this, &SIDataType::onStructSelected);
    connect(mDetails->ctrlTypeEnum()        , &QRadioButton::clicked            , this, &SIDataType::onEnumSelected);
    connect(mDetails->ctrlTypeImport()      , &QRadioButton::clicked            , this, &SIDataType::onImportSelected);
    connect(mDetails->ctrlTypeContainer()   , &QRadioButton::clicked            , this, &SIDataType::onContainerSelected);
    connect(mDetails->ctrlContainerObject() , &QComboBox::currentIndexChanged   , this, &SIDataType::onContainerObjectChanged);
    connect(mDetails->ctrlContainerKey()    , &QComboBox::currentIndexChanged   , this, &SIDataType::onContainerKeyChanged);
    connect(mDetails->ctrlContainerValue()  , &QComboBox::currentIndexChanged   , this, &SIDataType::onContainerValueChanged);
    
    connect(mList->ctrlToolRemove()         , &QToolButton::clicked             , this, &SIDataType::onRemoveClicked);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled               , this, &SIDataType::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::textEdited            , this, &SIDataType::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription()     , &QPlainTextEdit::textChanged      , this, &SIDataType::onDescriptionChanged);
    // connect(mTableCell                , &TableCell::editorDataChanged,this, &SIConstant::onEditorDataChanged);
}

void SIDataType::blockBasicSignals(bool doBlock)
{
    if (mDetails->isHidden() == false)
    {
        mDetails->ctrlName()->blockSignals(doBlock);
        mDetails->ctrlDeprecated()->blockSignals(doBlock);
        mDetails->ctrlDeprecateHint()->blockSignals(doBlock);
        mDetails->ctrlDescription()->blockSignals(doBlock);

        mDetails->ctrlTypeStruct()->blockSignals(doBlock);
        mDetails->ctrlTypeEnum()->blockSignals(doBlock);
        mDetails->ctrlTypeImport()->blockSignals(doBlock);
        mDetails->ctrlTypeContainer()->blockSignals(doBlock);
        
        mDetails->ctrlContainerObject()->blockSignals(doBlock);
        mDetails->ctrlContainerKey()->blockSignals(doBlock);
        mDetails->ctrlContainerValue()->blockSignals(doBlock);
    }

    if (mFields->isHidden() == false)
    {
        mFields->ctrlName()->blockSignals(doBlock);
        mFields->ctrlTypes()->blockSignals(doBlock);
        mFields->ctrlValue()->blockSignals(doBlock);
        mFields->ctrlDescription()->blockSignals(doBlock);
        mFields->ctrlDeprecated()->blockSignals(doBlock);
    }

    mList->ctrlTableList()->blockSignals(doBlock);
}

void SIDataType::selectedStruct(DataTypeStructure* dataType)
{
    activateFields(false);
    showEnumDetails(false);
    showImportDetails(false);
    showContainerDetails(false);

    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeStruct()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    mDetails->ctrlDeprecated()->setChecked(dataType->getIsDeprecated());
    mDetails->ctrlDeprecateHint()->setText(dataType->getDeprecateHint());

    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(true);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(true);

    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedEnum(DataTypeEnum* dataType)
{
    activateFields(false);
    showEnumDetails(true);
    showImportDetails(false);
    showContainerDetails(false);

    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeEnum()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    mDetails->ctrlDeprecated()->setChecked(dataType->getIsDeprecated());
    mDetails->ctrlDeprecateHint()->setText(dataType->getDeprecateHint());
    mDetails->ctrlEnumDerived()->setCurrentText(dataType->getDerived());

    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(true);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(true);
    
    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedImport(DataTypeImported* dataType)
{
    activateFields(false);
    showEnumDetails(false);
    showImportDetails(true);
    showContainerDetails(false);

    QString name;
    if (dataType->getNamespace().isEmpty())
    {
        name = dataType->getName();
    }
    else
    {
        name = dataType->getNamespace() + "::" + dataType->getName();
    }

    mDetails->ctrlName()->setText(name);
    mDetails->ctrlTypeImport()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    mDetails->ctrlDeprecated()->setChecked(dataType->getIsDeprecated());
    mDetails->ctrlDeprecateHint()->setText(dataType->getDeprecateHint());

    mDetails->ctrlImportLocation()->setText(dataType->getLocation());
    mDetails->ctrlImportNamespace()->setText(dataType->getNamespace());
    mDetails->ctrlButtonBrowse()->setEnabled(true);

    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(false);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(false);
    
    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedContainer(DataTypeDefined* dataType)
{
    activateFields(false);
    showEnumDetails(false);
    showImportDetails(false);
    showContainerDetails(true);
    
    const QList<DataTypeBase *> predefined {_getPredefinedTypes()};
    const QList<DataTypeCustom *>& customs {mModel.getCustomDataTypes()};
    QComboBox* keys = mDetails->ctrlContainerKey();
    QComboBox* values = mDetails->ctrlContainerValue();
    Q_ASSERT(keys->count() == values->count());
    
    const int countPredefined    {static_cast<int>(predefined.size())};
    while (keys->count() > countPredefined)
    {
        keys->removeItem(keys->count() - 1);
        values->removeItem(values->count() - 1);
    }
    
    if (customs.size() > 1)
    {
        keys->insertSeparator(keys->count());
        values->insertSeparator(values->count());
        
        for (auto type : customs)
        {
            if (type->getId() != dataType->getId())
            {
                keys->addItem(type->getName(), QVariant::fromValue(type));
                values->addItem(type->getName(), QVariant::fromValue(type));
            }
        }
    }
    
    keys->setEnabled(dataType->canHaveKey());
    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeContainer()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    mDetails->ctrlDeprecated()->setChecked(dataType->getIsDeprecated());
    mDetails->ctrlDeprecateHint()->setText(dataType->getDeprecateHint());

    mDetails->ctrlContainerValue()->setCurrentText(dataType->getValue());
    mDetails->ctrlContainerKey()->setCurrentText(dataType->getKey());
    mDetails->ctrlContainerKey()->setEnabled(dataType->canHaveKey());

    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(false);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(false);

    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedStructField(const FieldEntry& field, DataTypeStructure* parent)
{
    activateFields(true);
    
    const QList<DataTypeBase *>& predefined {_getPredefinedTypes()};
    const QList<DataTypeCustom *>& customs {mModel.getCustomDataTypes()};
    QComboBox* types = mFields->ctrlTypes();
    
    const int countPredefined    {static_cast<int>(predefined.size())};
    while (types->count() > countPredefined)
    {
        types->removeItem(types->count() - 1);
    }
    
    if (customs.size() > 1)
    {
        types->insertSeparator(customs.size());
        for (auto type : customs)
        {
            if (type->getId() != parent->getId())
            {
                types->addItem(type->getName(), QVariant::fromValue(types));
            }
        }
    }        
    
    types->setEnabled(true);
    mFields->ctrlName()->setText(field.getName());
    mFields->ctrlTypes()->setCurrentText(field.getType());
    mFields->ctrlValue()->setText(field.getValue());
    mFields->ctrlDescription()->setPlainText(field.getDescription());
    mFields->ctrlDeprecated()->setChecked(field.getIsDeprecated());
    mFields->ctrlDeprecateHint()->setText(field.getDeprecateHint());
    
    int index = parent->findIndex(field.getId());    
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedEnumField(const EnumEntry& field, DataTypeEnum* parent)
{
    activateFields(true);
    mFields->ctrlTypes()->setEnabled(false);
    mFields->ctrlName()->setText(field.getName());
    mFields->ctrlValue()->setText(field.getValue());
    mFields->ctrlDescription()->setPlainText(field.getDescription());
    mFields->ctrlDeprecated()->setChecked(field.getIsDeprecated());
    mFields->ctrlDeprecateHint()->setText(field.getDeprecateHint());
    
    int index = parent->findIndex(field.getId());    
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

QTreeWidgetItem* SIDataType::createNodeStructure(DataTypeStructure* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    updateNodeStructure(item, dataType);
    return item;
}

QTreeWidgetItem* SIDataType::createNodeEnum(DataTypeEnum* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    updateNodeEnum(item, dataType);
    return item;
}

QTreeWidgetItem* SIDataType::createNodeImported(DataTypeImported* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    updateNodeImported(item, dataType);
    return item;
}

QTreeWidgetItem* SIDataType::createNodeContainer(DataTypeDefined* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    updateNodeContainer(item, dataType);
    return item;
}

void SIDataType::updateNodeStructure(QTreeWidgetItem* node, DataTypeStructure* dataType) const
{
    node->setIcon(0, QIcon::fromTheme(QIcon::ThemeIcon::WeatherStorm));
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);

    node->setText(0, dataType->getName());
    node->setText(1, QString());
    const QList<FieldEntry>& fields = dataType->getElements();
    for (const FieldEntry& field : fields)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        updateChildNodeStruct(child, dataType, field);
        node->addChild(child);
    }

}

void SIDataType::updateNodeEnum(QTreeWidgetItem* node, DataTypeEnum* dataType) const
{
    node->setIcon(0, QIcon::fromTheme(QIcon::ThemeIcon::WeatherStorm));
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);

    node->setText(0, dataType->getName());
    node->setText(1, QString());
    const QList<EnumEntry>& fields = dataType->getElements();
    for (const EnumEntry& field : fields)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        node->addChild(child);
    }
}

void SIDataType::updateNodeImported(QTreeWidgetItem* node, DataTypeImported* dataType) const
{
    node->setIcon(0, QIcon::fromTheme(QIcon::ThemeIcon::WeatherStorm));
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);
    QString name;
    if (static_cast<DataTypeImported*>(dataType)->getNamespace().isEmpty())
    {
        name = dataType->getName();
    }
    else
    {
        name = static_cast<DataTypeImported*>(dataType)->getNamespace() + "::" + dataType->getName();
    }

    node->setText(0, dataType->getName());
    node->setText(1, QString());
}

void SIDataType::updateNodeContainer(QTreeWidgetItem* node, DataTypeDefined* dataType) const
{
    node->setIcon(0, QIcon::fromTheme(QIcon::ThemeIcon::WeatherStorm));
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);
    QString typeName;
    if (dataType->hasKey())
    {
        typeName = QString("%1<%2, %3>").arg(dataType->getContainer(), dataType->getKey(), dataType->getValue());
    }
    else if (dataType->getContainer().isEmpty() == false)
    {
        typeName = QString("%1<%2>").arg(dataType->getContainer(), dataType->getValue());
    }

    node->setText(0, dataType->getName());
    node->setText(1, typeName);
}

void SIDataType::updateChildNodeStruct(QTreeWidgetItem* child, DataTypeStructure* dataType, const FieldEntry& field) const
{
    child->setText(0, field.getName());
    child->setText(1, field.getType());
    child->setText(2, field.getValue());
    child->setIcon(0, QIcon::fromTheme(QIcon::ThemeIcon::WeatherSnow));
    child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    child->setData(1, Qt::ItemDataRole::UserRole, field.getId());
}

void SIDataType::updateChildNodeEnum(QTreeWidgetItem* child, DataTypeEnum* dataType, const EnumEntry& field) const
{
    child->setText(0, field.getName());
    child->setText(2, field.getValue());
    child->setIcon(0, QIcon::fromTheme(QIcon::ThemeIcon::WeatherSnow));
    child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    child->setData(1, Qt::ItemDataRole::UserRole, field.getId());
}

void SIDataType::activateFields(bool activate)
{
    if (activate)
    {
        if (mFields->isHidden())
        {
            mDetails->hide();
            mFields->show();
            // mFields->ctrlDescription()->move(NELusanCommon::DescPos);
        }
    }
    else
    {
        if (mDetails->isHidden())
        {
            mFields->hide();
            mDetails->show();
            // mDetails->ctrlDescription()->move(NELusanCommon::DescPos);
        }
    }
}
