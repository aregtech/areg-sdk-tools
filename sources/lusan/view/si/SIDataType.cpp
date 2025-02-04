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
#include "ui/ui_SIDataType.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIDataTypeModel.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"
#include "lusan/view/si/SIDataTypeDetails.hpp"
#include "lusan/view/si/SIDataTypeFieldDetails.hpp"
#include "lusan/view/si/SIDataTypeList.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QIcon>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "lusan/view/si/SICommon.hpp"

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
    , mTypeModel(new DataTypesModel(model.getDataTypeData(), true))
    , mValueModel(new DataTypesModel(model.getDataTypeData(), false))
    , mKeysModel(new DataTypesModel(model.getDataTypeData(), true))
    , mTableCell(nullptr)
    , mCurUrl   ( )
    , mCurFile  ( )
    , mCurView  (-1)
    , mCount    (0)
{
    mFields->setHidden(true);
    
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);
    ui.horizontalLayout->addWidget(mFields);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    setWidgetResizable(true);
    setWidget(mWidget);

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

void SIDataType::onCurCellChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if ((current == nullptr) || (current == previous))
        return;

    DataTypeCustom* oldType  = previous != nullptr ? previous->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    DataTypeCustom* dataType = current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = current->data(1, Qt::ItemDataRole::UserRole).toUInt();
    
    blockBasicSignals(true);
    if (id == 0)
    {
        switch (dataType->getCategory())
        {
        case DataTypeBase::eCategory::Structure:
            selectedStruct(oldType, static_cast<DataTypeStructure*>(dataType));
            break;

        case DataTypeBase::eCategory::Enumeration:
            selectedEnum(oldType, static_cast<DataTypeEnum*>(dataType));
            break;

        case DataTypeBase::eCategory::Imported:
            selectedImport(oldType, static_cast<DataTypeImported*>(dataType));
            break;

        case DataTypeBase::eCategory::Container:
            selectedContainer(oldType, static_cast<DataTypeContainer*>(dataType));
            break;

        default:
            break;
        }

        QTreeWidget* table = mList->ctrlTableList();
        updateToolButtons(table->indexOfTopLevelItem(current), table->topLevelItemCount());
    }
    else
    {
        Q_ASSERT((dataType->getCategory() == DataTypeBase::eCategory::Structure) || (dataType->getCategory() == DataTypeBase::eCategory::Enumeration));
        ElementBase * field = mModel.findChild(dataType, id);
        Q_ASSERT(field != nullptr);

        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            selectedStructField(oldType, *static_cast<FieldEntry*>(field), static_cast<DataTypeStructure*>(dataType));
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            selectedEnumField(oldType, *static_cast<EnumEntry*>(field), static_cast<DataTypeEnum*>(dataType));
        }

        QTreeWidgetItem* parent = current->parent();
        Q_ASSERT(parent != nullptr);
        updateToolButtons(parent->indexOfChild(current), parent->childCount());
    }
    
    blockBasicSignals(false);
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
    
    DataTypeCustom* oldType  = nullptr;    
    DataTypeCustom* dataType = mModel.createDataType(name, DataTypeBase::eCategory::Structure);
    int pos = table->topLevelItemCount();
    QTreeWidgetItem * item = createNodeStructure(static_cast<DataTypeStructure *>(dataType));
    table->insertTopLevelItem(pos, item);
    QTreeWidgetItem * cur = table->currentItem();
    if (cur != nullptr)
    {
        oldType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>();
        cur->setSelected(false);
    }
    
    item->setSelected(true);
    table->setCurrentItem(item);
    selectedStruct(oldType, static_cast<DataTypeStructure*>(dataType));
    updateToolButtons(pos, table->topLevelItemCount());
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
            selectedStructField(nullptr, *static_cast<FieldEntry *>(field), static_cast<DataTypeStructure*>(dataType));
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            updateChildNodeEnum(item, static_cast<DataTypeEnum*>(dataType), *static_cast<EnumEntry*>(field));
            selectedEnumField(nullptr, *static_cast<EnumEntry*>(field), static_cast<DataTypeEnum*>(dataType));
        }

        int pos = parent->indexOfChild(item);
        updateToolButtons(pos, parent->childCount());
        blockBasicSignals(false);
    }
}

void SIDataType::onRemoveClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;
    
    Q_ASSERT(item->data(1, Qt::ItemDataRole::UserRole).toUInt() == 0);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    item = id == 0 ? item : item->parent();

    int index = table->indexOfTopLevelItem(item);
    index = index + 1 == table->topLevelItemCount() ? index - 1 : index + 1;
    QTreeWidgetItem* next = (index >= 0) && (index < table->topLevelItemCount()) ? table->topLevelItem(index) : nullptr;
    table->setCurrentItem(next);
    item->setSelected(false);
    deleteTreeNode(item);

    int row = -1;
    int rowCount = 0;
    if (next != nullptr)
    {
        next->setSelected(true);
        row = table->indexOfTopLevelItem(item);
        rowCount = table->topLevelItemCount();
    }
    else
    {
        showClean();
    }

    updateToolButtons(row, rowCount);
}

void SIDataType::onRemoveFieldClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;
    
    Q_ASSERT(item->data(1, Qt::ItemDataRole::UserRole).toUInt() != 0);
    QTreeWidgetItem* parent = item->parent();
    int index = parent->indexOfChild(item);
    index = index + 1 == parent->childCount() ? index - 1 : index;
    QTreeWidgetItem* next = nullptr;
    item->setSelected(false);

    deleteTreeNode(item);
    int row = 0;
    int rowCount = 0;
    if ((index >= 0) && (index < parent->childCount()))
    {
        next = parent->child(index);
        row = index;
        rowCount = parent->childCount();
    }
    else
    {
        next = parent;
        row = table->indexOfTopLevelItem(next);
        rowCount = table->topLevelItemCount();
    }

    if (next != nullptr)
    {
        next->setSelected(true);
        table->setCurrentItem(next);
    }
    
    updateToolButtons(row, rowCount);
}

void SIDataType::onMoveUpClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;

    blockBasicSignals(true);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if (id == 0)
    {
        moveDataTypeUp(item);
    }
    else
    {
        moveDataTypeParamUp(item);
    }

    blockBasicSignals(false);
}

void SIDataType::onMoveDownClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;

    blockBasicSignals(true);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if (id == 0)
    {
        moveDataTypeDown(item);
    }
    else
    {
        moveDataTypeParamDown(item);
    }

    blockBasicSignals(false);
}

void SIDataType::onTypeNameChanged(const QString& newName)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;
    
    DataTypeCustom* dataType = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    mModel.updateDataType(dataType, newName);
    setNodeText(item, dataType);
}

void SIDataType::onDeprectedChecked(bool isChecked)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;

    DataTypeCustom* entry = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    Q_ASSERT(entry != nullptr);
    SICommon::checkedDeprecated<SIDataTypeDetails, DataTypeCustom>(mDetails, entry, isChecked);
}

void SIDataType::onDeprecateHintChanged(const QString& newText)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;

    DataTypeCustom* entry = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    Q_ASSERT(entry != nullptr);
    SICommon::setDeprecateHint<SIDataTypeDetails, DataTypeCustom>(mDetails, entry, newText);
}

void SIDataType::onDescriptionChanged(void)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;

    DataTypeCustom* dataType = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    Q_ASSERT(dataType != nullptr);
    dataType->setDescription(mDetails->ctrlDescription()->toPlainText());
}

void SIDataType::onStructSelected(bool checked)
{
    if (checked)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        convertDataType(item, DataTypeBase::eCategory::Structure);
        blockBasicSignals(false);
    }
}

void SIDataType::onEnumSelected(bool checked)
{
    if (checked)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        convertDataType(item, DataTypeBase::eCategory::Enumeration);
        blockBasicSignals(false);
    }
}

void SIDataType::onImportSelected(bool checked)
{
    if (checked)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        convertDataType(item, DataTypeBase::eCategory::Imported);
        blockBasicSignals(false);
    }
}

void SIDataType::onContainerSelected(bool checked)
{
    if (checked)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
        convertDataType(item, DataTypeBase::eCategory::Container);
        blockBasicSignals(false);
    }
}

void SIDataType::dataTypeCreated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeCreated(dataType);
    mValueModel->dataTypeCreated(dataType);
    mKeysModel->dataTypeCreated(dataType);
}

void SIDataType::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    mTypeModel->dataTypeConverted(oldType, newType);
    mValueModel->dataTypeConverted(oldType, newType);
    mKeysModel->dataTypeConverted(oldType, newType);
}

void SIDataType::dataTypeDeleted(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeDeleted(dataType);
    mValueModel->dataTypeDeleted(dataType);
    mKeysModel->dataTypeDeleted(dataType);

    QTreeWidget* table = mList->ctrlTableList();
    int cntTop = table->topLevelItemCount();
    for (int i = 0; i < cntTop; ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        Q_ASSERT(top != nullptr);
        DataTypeCustom* topType = top->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        if (topType == dataType)
            continue;

        if (topType->isStructure())
        {
            int cntFields = top->childCount();
            for (int j = 0; j < cntFields; ++j)
            {
                QTreeWidgetItem* child = top->child(j);
                Q_ASSERT(child != nullptr);
                uint32_t id = child->data(1, Qt::ItemDataRole::UserRole).toUInt();
                FieldEntry* field = static_cast<DataTypeStructure*>(topType)->findElement(id);
                if ((field != nullptr) && (field->getParamType() == dataType))
                {
                    field->setParamType(nullptr);
                }
                
                setNodeText(child, field);
            }
        }
        else if (topType->isContainer())
        {
            DataTypeContainer* container = static_cast<DataTypeContainer*>(topType);
            if (container->getKeyDataType() == dataType)
            {
                container->setKeyDataType(nullptr);
            }
            if (container->getValueDataType() == dataType)
            {
                container->setValueDataType(nullptr);
            }
            
            setNodeText(top, topType);
        }
    }
}

void SIDataType::dataTypeUpdated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeUpdated(dataType);
    mValueModel->dataTypeUpdated(dataType);
    mKeysModel->dataTypeUpdated(dataType);

    QTreeWidget* table = mList->ctrlTableList();
    int cntTop = table->topLevelItemCount();
    for (int i = 0; i < cntTop; ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        Q_ASSERT(top != nullptr);
        DataTypeCustom * topType = top->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>();
        if (topType == dataType)
            continue;
        
        if (topType->isStructure())
        {
            int cntFields = top->childCount();
            for (int j = 0; j < cntFields; ++j)
            {
                QTreeWidgetItem* child = top->child(j);
                Q_ASSERT(child != nullptr);
                uint32_t id = child->data(1, Qt::ItemDataRole::UserRole).toUInt();
                FieldEntry* field = static_cast<DataTypeStructure *>(topType)->findElement(id);
                if ((field != nullptr) && (field->getParamType() == dataType))
                {
                    setNodeText(child, field);
                }
            }
        }
        else if (topType->isContainer())
        {
            setNodeText(top, topType);
        }
    }
}

void SIDataType::convertDataType(QTreeWidgetItem* current, DataTypeBase::eCategory newCategory)
{
    Q_ASSERT(current != nullptr);
    Q_ASSERT(current->parent() == nullptr);

    DataTypeCustom* dataType = current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = current->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if ((id == 0) && (dataType->getCategory() != newCategory))
    {
        current->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom*>(nullptr)));
        DataTypeCustom* oldType = dataType;
        DataTypeCustom* newType = nullptr;
        
        QList<QTreeWidgetItem*> children {current->takeChildren()};
        for (auto child : children)
        {
            current->removeChild(child);
            delete child;
        }
        
        switch (newCategory)
        {
        case DataTypeBase::eCategory::Structure:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Structure);
            newType = dataType;
            Q_ASSERT(static_cast<DataTypeStructure*>(dataType)->hasElements() == false);
            updateNodeStructure(current, static_cast<DataTypeStructure*>(dataType));
            selectedStruct(nullptr, static_cast<DataTypeStructure*>(dataType));
            break;

        case DataTypeBase::eCategory::Enumeration:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Enumeration);
            newType = dataType;
            Q_ASSERT(static_cast<DataTypeEnum*>(dataType)->hasElements() == false);
            updateNodeEnum(current, static_cast<DataTypeEnum*>(dataType));
            selectedEnum(nullptr, static_cast<DataTypeEnum*>(dataType));
            break;

        case DataTypeBase::eCategory::Imported:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Imported);
            newType = dataType;
            updateNodeImported(current, static_cast<DataTypeImported*>(dataType));
            selectedImport(nullptr, static_cast<DataTypeImported*>(dataType));
            break;

        case DataTypeBase::eCategory::Container:
            dataType = mModel.convertDataType(dataType, DataTypeBase::eCategory::Container);
            newType = dataType;
            static_cast<DataTypeContainer *>(dataType)->setContainer(mDetails->ctrlContainerObject()->itemText(0));
            static_cast<DataTypeContainer *>(dataType)->setValue(mDetails->ctrlContainerValue()->itemText(0));
            if (static_cast<DataTypeContainer *>(dataType)->canHaveKey())
            {
                mKeysModel->removeEmptyEntry();
                mDetails->ctrlContainerKey()->setCurrentIndex(0);
                static_cast<DataTypeContainer *>(dataType)->setKey(mDetails->ctrlContainerKey()->itemText(0));
            }
            else
            {
                mKeysModel->addEmptyEntry();
                mDetails->ctrlContainerKey()->setCurrentIndex(0);
                static_cast<DataTypeContainer *>(dataType)->setKey(QString());
            }
            
            updateNodeContainer(current, static_cast<DataTypeContainer*>(dataType));
            selectedContainer(nullptr, static_cast<DataTypeContainer*>(dataType));
            break;

        default:
            break;
        }

        if (newType != nullptr)
        {
            delete oldType;
        }
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
    DataTypeContainer* typeContainer = static_cast<DataTypeContainer*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>());
    Q_ASSERT(typeContainer->getCategory() == DataTypeBase::eCategory::Container);
    typeContainer->setContainer(dataType->getName());
    
    blockBasicSignals(true);
    if (dataType->hasKey())
    {
        mKeysModel->removeEmptyEntry();
        mDetails->ctrlContainerKey()->setCurrentIndex(0);
        mDetails->ctrlContainerKey()->setEnabled(true);
        typeContainer->setKey(mDetails->ctrlContainerKey()->currentText());
    }
    else
    {
        mKeysModel->addEmptyEntry();
        mDetails->ctrlContainerKey()->setCurrentIndex(0);
        mDetails->ctrlContainerKey()->setEnabled(false);
        typeContainer->setKey(QString());
    }

    mModel.normalizeDataType(typeContainer);
    blockBasicSignals(false);

    updateContainerNames(current, typeContainer);
}

void SIDataType::onContainerKeyChanged(int index)
{
    if ((index < 0) || (index >= mDetails->ctrlContainerKey()->count()))
    {
        return;
    }
    
    DataTypeBase*  dataType = mDetails->ctrlContainerKey()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    Q_ASSERT(dataType != nullptr);
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeContainer* typeContainer = static_cast<DataTypeContainer*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>());
    Q_ASSERT(typeContainer->getCategory() == DataTypeBase::eCategory::Container);
    typeContainer->setKeyDataType(dataType);
    updateContainerNames(current, typeContainer);
}

void SIDataType::onContainerValueChanged(int index)
{
    if ((index < 0) || (index >= mDetails->ctrlContainerValue()->count()))
    {
        return;
    }
    
    DataTypeBase*  dataType = mDetails->ctrlContainerValue()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
    Q_ASSERT(dataType != nullptr);
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeContainer* typeContainer = static_cast<DataTypeContainer*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>());
    Q_ASSERT(typeContainer->getCategory() == DataTypeBase::eCategory::Container);
    typeContainer->setValueDataType(dataType);
    updateContainerNames(current, typeContainer);
}

void SIDataType::onEnumDerivedChanged(int index)
{
    if (index >= 0)
    {
        blockBasicSignals(true);
        DataTypeBase* dataType = mDetails->ctrlEnumDerived()->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>();
        QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
        QString derived{dataType != nullptr ? dataType->getName() : QString("")};
        DataTypeEnum* typeEnum = static_cast<DataTypeEnum*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>());
        Q_ASSERT((typeEnum != nullptr) && (typeEnum->getCategory() == DataTypeBase::eCategory::Enumeration));
        typeEnum->setDerived(derived);
        setNodeText(current, typeEnum);
        blockBasicSignals(false);
    }
}

void SIDataType::onImportLocationChanged(const QString& newText)
{
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeImported* dataType = static_cast<DataTypeImported*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>());
    dataType->setLocation(newText);
}

void SIDataType::onImportNamespaceChanged(const QString& newText)
{
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeImported* dataType = static_cast<DataTypeImported*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>());
    dataType->setNamespace(newText);
    blockBasicSignals(true);
    updateImportNames(current, dataType);
    blockBasicSignals(false);
}

void SIDataType::onImportObjectChanged(const QString& newText)
{
    QTreeWidgetItem* current = mList->ctrlTableList()->currentItem();
    DataTypeImported* dataType = static_cast<DataTypeImported*>(current->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>());
    dataType->setObject(newText);
    blockBasicSignals(true);
    updateImportNames(current, dataType);
    blockBasicSignals(false);
}

void SIDataType::onImportLocationBrowse(void)
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

    QString curFile  = mDetails->ctrlImportLocation()->text();
    curFile = curFile.isEmpty() ? mCurFile : curFile;

    dialog.setDirectoryUrl(QUrl::fromLocalFile(mCurUrl));
    dialog.setDirectory(mCurUrl);

    if (curFile.isEmpty() == false)
    {
        QFileInfo info(curFile);
        dialog.setDirectory(info.absoluteDir());
        dialog.selectFile(curFile);
    }

    if (mCurView != -1)
    {
        dialog.setViewMode(static_cast<QFileDialog::ViewMode>(mCurView));
    }

    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        mCurUrl = dialog.directoryUrl().path();
        mCurFile = dialog.getSelectedFilePath();
        mCurView = static_cast<int>(dialog.viewMode());

        QString location = dialog.getSelectedFileRelativePath();
        mDetails->ctrlImportLocation()->setText(location);
    }
}

void SIDataType::onFieldNameChanged(const QString& newName)
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    DataTypeCustom* dataType = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    ElementBase* field = getSelectedField();
    if ((dataType != nullptr) && (field != nullptr))
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            static_cast<FieldEntry*>(field)->setName(newName);
            setNodeText(item, static_cast<FieldEntry*>(field));
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            static_cast<EnumEntry*>(field)->setName(newName);
            setNodeText(item, static_cast<EnumEntry*>(field));
        }
    }
}

void SIDataType::onFieldTypeChanged(int index)
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    DataTypeCustom* dataType = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    ElementBase* field = getSelectedField();
    QComboBox* types = mFields->ctrlTypes();
    DataTypeBase* selType = index >= 0 ? types->itemData(index, Qt::ItemDataRole::UserRole).value<DataTypeBase*>() : nullptr;

    if ((dataType != nullptr) && (field != nullptr) && (selType != nullptr))
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            static_cast<FieldEntry *>(field)->setParamType(selType);
            setNodeText(item, static_cast<FieldEntry *>(field));
        }
    }
}

void SIDataType::onFieldValueChanged(const QString& newValue)
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    DataTypeCustom* dataType = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    ElementBase* field = getSelectedField();
    if ((dataType != nullptr) && (field != nullptr))
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            static_cast<FieldEntry*>(field)->setValue(newValue);
            setNodeText(item, static_cast<FieldEntry *>(field));
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            static_cast<EnumEntry*>(field)->setValue(newValue);
            setNodeText(item, static_cast<EnumEntry *>(field));
        }
    }
}

void SIDataType::onFieldDescriptionChanged(void)
{
    QString descr{mFields->ctrlDescription()->toPlainText()};
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    DataTypeCustom* dataType = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    ElementBase* field = getSelectedField();
    if ((dataType != nullptr) && (field != nullptr))
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            static_cast<FieldEntry*>(field)->setDescription(descr);
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            static_cast<EnumEntry*>(field)->setDescription(descr);
        }
    }
}

void SIDataType::onFieldDeprecatedChecked(bool isChecked)
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    DataTypeCustom* dataType = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    ElementBase* entry = getSelectedField();
    if ((dataType != nullptr) && (entry != nullptr))
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            SICommon::checkedDeprecated<SIDataTypeFieldDetails, FieldEntry>(mFields, static_cast<FieldEntry*>(entry), isChecked);
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            SICommon::checkedDeprecated<SIDataTypeFieldDetails, EnumEntry>(mFields, static_cast<EnumEntry*>(entry), isChecked);
        }
        else
        {
            Q_ASSERT(false);
        }
    }
}

void SIDataType::onFieldDeprecateHint(const QString& newText)
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    DataTypeCustom* dataType = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>() : nullptr;
    ElementBase* field = getSelectedField();
    if ((dataType != nullptr) && (field != nullptr))
    {
        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            SICommon::setDeprecateHint<SIDataTypeFieldDetails, FieldEntry>(mFields, static_cast<FieldEntry*>(field), newText);
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            SICommon::setDeprecateHint<SIDataTypeFieldDetails, EnumEntry>(mFields, static_cast<EnumEntry*>(field), newText);
        }
        else
        {
            Q_ASSERT(false);
        }
    }
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
            item = createNodeContainer(static_cast<DataTypeContainer*>(entry));
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

void SIDataType::updateWidgets(void)
{
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();
    mValueModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mValueModel->updateDataTypeLists();
    mKeysModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mKeysModel->updateDataTypeLists();
    
    // mTableCell = new TableCell(QList<QAbstractItemModel*>{mTypeModel}, QList<int>{1}, mList->ctrlTableList());
    
    QComboBox* container = mDetails->ctrlContainerObject();
    const QList<DataTypeBasicContainer*>& containers {_getContainerTypes()};
    for (auto dataType : containers)
    {
        container->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
    }
    
    QComboBox* enumDerive = mDetails->ctrlEnumDerived();
    const QList<DataTypeBase*>& integers{_getIntegerTypes()};
    enumDerive->addItem(QString(), QVariant::fromValue(static_cast<DataTypeBase*>(nullptr)));
    for (auto dataType : integers)
    {
        enumDerive->addItem(dataType->getName(), QVariant::fromValue(static_cast<DataTypeBase *>(dataType)));
    }
    
    mFields->ctrlTypes()->setModel(mTypeModel);
    mDetails->ctrlContainerValue()->setModel(mValueModel);
    mDetails->ctrlContainerKey()->setModel(mKeysModel);
    
    showClean();
}

void SIDataType::setupSignals(void)
{
    connect(mList->ctrlTableList()          , &QTreeWidget::currentItemChanged  , this, &SIDataType::onCurCellChanged);
    connect(mList->ctrlToolAdd()            , &QToolButton::clicked             , this, &SIDataType::onAddClicked);
    connect(mList->ctrlToolAddField()       , &QToolButton::clicked             , this, &SIDataType::onAddFieldClicked);
    connect(mList->ctrlToolRemove()         , &QToolButton::clicked             , this, &SIDataType::onRemoveClicked);
    connect(mList->ctrlToolRemoveField()    , &QToolButton::clicked             , this, &SIDataType::onRemoveFieldClicked);
    connect(mList->ctrlToolMoveUp()         , &QToolButton::clicked             , this, &SIDataType::onMoveUpClicked);
    connect(mList->ctrlToolMoveDown()       , &QToolButton::clicked             , this, &SIDataType::onMoveDownClicked);

    connect(mDetails->ctrlName()            , &QLineEdit::textChanged           , this, &SIDataType::onTypeNameChanged);
    connect(mDetails->ctrlTypeStruct()      , &QRadioButton::clicked            , this, &SIDataType::onStructSelected);
    connect(mDetails->ctrlTypeEnum()        , &QRadioButton::clicked            , this, &SIDataType::onEnumSelected);
    connect(mDetails->ctrlTypeImport()      , &QRadioButton::clicked            , this, &SIDataType::onImportSelected);
    connect(mDetails->ctrlTypeContainer()   , &QRadioButton::clicked            , this, &SIDataType::onContainerSelected);
    connect(mDetails->ctrlContainerObject() , &QComboBox::currentIndexChanged   , this, &SIDataType::onContainerObjectChanged);
    connect(mDetails->ctrlContainerKey()    , &QComboBox::currentIndexChanged   , this, &SIDataType::onContainerKeyChanged);
    connect(mDetails->ctrlContainerValue()  , &QComboBox::currentIndexChanged   , this, &SIDataType::onContainerValueChanged);
    connect(mDetails->ctrlEnumDerived()     , &QComboBox::currentIndexChanged   , this, &SIDataType::onEnumDerivedChanged);
    connect(mDetails->ctrlImportLocation()  , &QLineEdit::textChanged           , this, &SIDataType::onImportLocationChanged);
    connect(mDetails->ctrlImportNamespace() , &QLineEdit::textChanged           , this, &SIDataType::onImportNamespaceChanged);
    connect(mDetails->ctrlImportObject()    , &QLineEdit::textChanged           , this, &SIDataType::onImportObjectChanged);
    connect(mDetails->ctrlButtonBrowse()    , &QPushButton::pressed             , this, &SIDataType::onImportLocationBrowse);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled               , this, &SIDataType::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::textEdited            , this, &SIDataType::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription()     , &QPlainTextEdit::textChanged      , this, &SIDataType::onDescriptionChanged);

    connect(mFields->ctrlName()             , &QLineEdit::textChanged           , this, &SIDataType::onFieldNameChanged);
    connect(mFields->ctrlTypes()            , &QComboBox::currentIndexChanged   , this, &SIDataType::onFieldTypeChanged);
    connect(mFields->ctrlValue()            , &QLineEdit::textChanged           , this, &SIDataType::onFieldValueChanged);
    connect(mFields->ctrlDescription()      , &QPlainTextEdit::textChanged      , this, &SIDataType::onFieldDescriptionChanged);
    connect(mFields->ctrlDeprecated()       , &QCheckBox::toggled               , this, &SIDataType::onFieldDeprecatedChecked);
    connect(mFields->ctrlDeprecateHint()    , &QLineEdit::textChanged           , this, &SIDataType::onFieldDeprecateHint);

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
        mFields->ctrlDeprecateHint()->blockSignals(doBlock);
    }

    mList->ctrlTableList()->blockSignals(doBlock);
}

void SIDataType::selectedStruct(DataTypeCustom* oldType, DataTypeStructure* dataType)
{
    enableTypeSelection(true);
    Q_ASSERT(dataType != nullptr);
    mTypeModel->removeDataType(dataType);
    mValueModel->removeDataType(dataType);
    mKeysModel->removeDataType(dataType);
    if ((oldType != nullptr) && (oldType != dataType))
    {
        mTypeModel->addDataType(oldType);
        mValueModel->addDataType(oldType);
        mKeysModel->addDataType(oldType);
    }    
    
    activateFields(false);
    showEnumDetails(false);
    showImportDetails(false);
    showContainerDetails(false);

    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeStruct()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());

    SICommon::enableDeprecated<SIDataTypeDetails, DataTypeCustom>(mDetails, dataType, true);

    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(true);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(true);

    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedEnum(DataTypeCustom* oldType, DataTypeEnum* dataType)
{
    enableTypeSelection(true);
    Q_ASSERT(dataType != nullptr);
    mTypeModel->removeDataType(dataType);
    mValueModel->removeDataType(dataType);
    mKeysModel->removeDataType(dataType);
    if ((oldType != nullptr) && (oldType != dataType))
    {
        mTypeModel->addDataType(oldType);
        mValueModel->addDataType(oldType);
        mKeysModel->addDataType(oldType);
    }

    activateFields(false);
    showEnumDetails(true);
    showImportDetails(false);
    showContainerDetails(false);

    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeEnum()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    mDetails->ctrlEnumDerived()->setCurrentText(dataType->getDerived());
    
    SICommon::enableDeprecated<SIDataTypeDetails, DataTypeCustom>(mDetails, dataType, true);
    
    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(true);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(true);
    
    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedImport(DataTypeCustom* oldType, DataTypeImported* dataType)
{
    enableTypeSelection(true);
    if ((oldType != nullptr) && (oldType != dataType))
    {
        mTypeModel->addDataType(oldType);
        mValueModel->addDataType(oldType);
        mKeysModel->addDataType(oldType);
    }
    
    activateFields(false);
    showEnumDetails(false);
    showImportDetails(true);
    showContainerDetails(false);

    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeImport()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    
    SICommon::enableDeprecated<SIDataTypeDetails, DataTypeCustom>(mDetails, dataType, true);

    mDetails->ctrlImportLocation()->setText(dataType->getLocation());
    mDetails->ctrlImportNamespace()->setText(dataType->getNamespace());
    mDetails->ctrlImportObject()->setText(dataType->getObject());
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

void SIDataType::selectedContainer(DataTypeCustom* oldType, DataTypeContainer* dataType)
{
    enableTypeSelection(true);
    Q_ASSERT(dataType != nullptr);
    mTypeModel->removeDataType(dataType);
    mValueModel->removeDataType(dataType);
    mKeysModel->removeDataType(dataType);
    if ((oldType != nullptr) && (oldType != dataType))
    {
        mTypeModel->addDataType(oldType);
        mValueModel->addDataType(oldType);
        mKeysModel->addDataType(oldType);
    }
    
    activateFields(false);
    showEnumDetails(false);
    showImportDetails(false);
    showContainerDetails(true);
    
    mDetails->ctrlContainerKey()->setEnabled(dataType->canHaveKey());
    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeContainer()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    
    SICommon::enableDeprecated<SIDataTypeDetails, DataTypeCustom>(mDetails, dataType, true);

    mDetails->ctrlContainerObject()->setCurrentText(dataType->getContainer());
    mDetails->ctrlContainerValue()->setCurrentText(dataType->getValue());
    if (dataType->canHaveKey())
    {
        mKeysModel->removeEmptyEntry();
        mDetails->ctrlContainerKey()->setEnabled(true);
        mDetails->ctrlContainerKey()->setCurrentText(dataType->getKey());
    }
    else
    {
        mKeysModel->addEmptyEntry();
        mDetails->ctrlContainerKey()->setCurrentIndex(0);
        mDetails->ctrlContainerKey()->setEnabled(false);
    }

    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(true);
    mList->ctrlToolAddField()->setEnabled(false);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(false);

    int index = mModel.findIndex(dataType->getId());
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedStructField(DataTypeCustom* oldType, const FieldEntry& field, DataTypeStructure* parent)
{
    Q_ASSERT(parent != nullptr);
    mTypeModel->removeEmptyEntry();
    mTypeModel->removeDataType(parent);
    mValueModel->removeDataType(parent);
    mKeysModel->removeDataType(parent);
    if ((oldType != nullptr) && (oldType != static_cast<DataTypeCustom *>(parent)))
    {
        mTypeModel->addDataType(oldType);
        mValueModel->addDataType(oldType);
        mKeysModel->addDataType(oldType);
    }
    
    activateFields(true);
    
    
    mFields->ctrlTypes()->setEnabled(true);
    mFields->ctrlName()->setText(field.getName());
    mFields->ctrlTypes()->setCurrentText(field.getType());
    mFields->ctrlValue()->setText(field.getValue());
    mFields->ctrlDescription()->setPlainText(field.getDescription());

    SICommon::enableDeprecated<SIDataTypeFieldDetails, FieldEntry>(mFields, &field, true);
    
    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(false);
    mList->ctrlToolAddField()->setEnabled(true);
    mList->ctrlToolInsertField()->setEnabled(true);
    mList->ctrlToolRemoveField()->setEnabled(true);
    
    int index = parent->findIndex(field.getId());    
    mList->ctrlToolMoveUp()->setEnabled(index > 0);
    mList->ctrlToolMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
}

void SIDataType::selectedEnumField(DataTypeCustom* oldType, const EnumEntry& field, DataTypeEnum* parent)
{
    Q_ASSERT(parent != nullptr);
    mTypeModel->addEmptyEntry();
    mTypeModel->removeDataType(parent);
    mValueModel->removeDataType(parent);
    mKeysModel->removeDataType(parent);
    if ((oldType != nullptr) && (oldType != static_cast<DataTypeCustom *>(parent)))
    {
        mTypeModel->addDataType(oldType);
        mValueModel->addDataType(oldType);
        mKeysModel->addDataType(oldType);
    }
    
    activateFields(true);
    mFields->ctrlTypes()->setCurrentIndex(0);
    mFields->ctrlTypes()->setEnabled(false);
    mFields->ctrlName()->setText(field.getName());
    mFields->ctrlValue()->setText(field.getValue());
    mFields->ctrlDescription()->setPlainText(field.getDescription());
    
    SICommon::enableDeprecated<SIDataTypeFieldDetails, EnumEntry>(mFields, &field, true);
    
    mList->ctrlToolAdd()->setEnabled(true);
    mList->ctrlToolRemove()->setEnabled(false);
    mList->ctrlToolAddField()->setEnabled(true);
    mList->ctrlToolInsertField()->setEnabled(true);
    mList->ctrlToolRemoveField()->setEnabled(true);
    
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

QTreeWidgetItem* SIDataType::createNodeContainer(DataTypeContainer* dataType) const
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    updateNodeContainer(item, dataType);
    return item;
}

void SIDataType::updateNodeStructure(QTreeWidgetItem* node, DataTypeStructure* dataType) const
{
    setNodeText(node, dataType);
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);
    QList<FieldEntry>& fields = dataType->getElements();
    for (FieldEntry& field : fields)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        updateChildNodeStruct(child, dataType, field);
        node->addChild(child);
    }
}

void SIDataType::updateNodeEnum(QTreeWidgetItem* node, DataTypeEnum* dataType) const
{
    setNodeText(node, dataType);
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);
    QList<EnumEntry>& fields = dataType->getElements();
    for (EnumEntry& field : fields)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        updateChildNodeEnum(child, dataType, field);
        node->addChild(child);
    }
}

void SIDataType::updateNodeImported(QTreeWidgetItem* node, DataTypeImported* dataType) const
{
    setNodeText(node, dataType);
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);
}

void SIDataType::updateNodeContainer(QTreeWidgetItem* node, DataTypeContainer* dataType) const
{
    setNodeText(node, dataType);
    node->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    node->setData(1, Qt::ItemDataRole::UserRole, 0);
}

void SIDataType::updateChildNodeStruct(QTreeWidgetItem* child, DataTypeStructure* dataType, FieldEntry& field) const
{
    if (field.getParamType() == nullptr)
    {
        field.validate(mModel.getDataTypes());
    }
    
    setNodeText(child, &field);
    child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(static_cast<DataTypeCustom *>(dataType)));
    child->setData(1, Qt::ItemDataRole::UserRole, field.getId());
}

void SIDataType::updateChildNodeEnum(QTreeWidgetItem* child, DataTypeEnum* dataType, EnumEntry& field) const
{
    setNodeText(child, &field);
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

void SIDataType::updateContainerNames(QTreeWidgetItem* node, DataTypeContainer* dataType) const
{
    setNodeText(node, dataType);
}

void SIDataType::updateImportNames(QTreeWidgetItem* node, DataTypeImported* dataType) const
{
    setNodeText(node, dataType);
}

inline ElementBase* SIDataType::getSelectedField(void) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return nullptr;
    
    DataTypeCustom* dataType = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = dataType != nullptr ? item->data(1, Qt::ItemDataRole::UserRole).toUInt() : 0;
    return (id != 0 ? mModel.findChild(dataType, id) : nullptr);
}

inline void SIDataType::enableTypeSelection(bool enable)
{
    mDetails->ctrlTypeStruct()->setEnabled(enable);
    mDetails->ctrlTypeEnum()->setEnabled(enable);
    mDetails->ctrlTypeImport()->setEnabled(enable);
    mDetails->ctrlTypeContainer()->setEnabled(enable);
}

inline void SIDataType::deleteTreeNode(QTreeWidgetItem* node)
{
    if (node == nullptr)
        return;

    DataTypeCustom* dataType = node->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    uint32_t id = node->data(1, Qt::ItemDataRole::UserRole).toUInt();
    QTreeWidgetItem* parent = node->parent();
    QList<QTreeWidgetItem*> children {node->takeChildren()};
    for (auto child : children)
    {
        node->removeChild(child);
        delete child;
    }
    
    if (parent != nullptr)
    {
        parent->removeChild(node);
    }
    else
    {
        int index = mList->ctrlTableList()->indexOfTopLevelItem(node);
        QTreeWidgetItem * item = mList->ctrlTableList()->takeTopLevelItem(index);
        Q_ASSERT(item == node);
    }
    
    if (id == 0)
    {
        mModel.deleteDataType(dataType);
    }
    else
    {
        mModel.deleteDataTypeChild(dataType, id);
    }

    delete node;
}

inline void SIDataType::setNodeText(QTreeWidgetItem* node, DocumentElem * elem) const
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(elem != nullptr);
    
    if (elem != nullptr)
    {
        node->setIcon(0, elem->getIcon(ElementBase::eDisplay::DisplayName));
        node->setText(0, elem->getString(ElementBase::eDisplay::DisplayName));
        node->setIcon(1, elem->getIcon(ElementBase::eDisplay::DisplayType));
        node->setText(1, elem->getString(ElementBase::eDisplay::DisplayType));
        node->setIcon(2, elem->getIcon(ElementBase::eDisplay::DisplayValue));
        node->setText(2, elem->getString(ElementBase::eDisplay::DisplayValue));    
    }
    else
    {
        node->setIcon(0, QIcon::fromTheme("dialog-warning"));
        node->setText(0, "<invalid>");
    }
}

inline void SIDataType::showClean(void)
{
    SICommon::enableDeprecated<SIDataTypeDetails, DataTypeCustom>(mDetails, nullptr, false);
    SICommon::enableDeprecated<SIDataTypeFieldDetails, FieldEntry>(mFields, nullptr, false);
    
    mDetails->ctrlName()->clear();
    showEnumDetails(false);
    showContainerDetails(false);
    showImportDetails(false);
    
    mList->ctrlToolRemove()->setEnabled(false);
    mList->ctrlToolAddField()->setEnabled(false);
    mList->ctrlToolRemoveField()->setEnabled(false);
    mList->ctrlToolInsertField()->setEnabled(false);
    mList->ctrlToolMoveUp()->setEnabled(false);
    mList->ctrlToolMoveDown()->setEnabled(false);
    
    enableTypeSelection(false);
}

inline void SIDataType::moveDataTypeUp(QTreeWidgetItem* node)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(node);
    if (row > 0)
    {
        bool isExpanded = node->isExpanded();
        swapDataTypes(node, row, row - 1);
        node->setExpanded(isExpanded);
    }
}

inline void SIDataType::moveDataTypeParamUp(QTreeWidgetItem* node)
{
    QTreeWidgetItem* parent = node->parent();
    Q_ASSERT(parent != nullptr);
    int row = parent->indexOfChild(node);
    if (row > 0)
    {
        swapDataTypeFields(node, parent, row, row - 1);
    }
}

inline void SIDataType::moveDataTypeDown(QTreeWidgetItem* node)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(node);
    if ((row >= 0) && (row < (table->topLevelItemCount() - 1)))
    {
        swapDataTypes(node, row, row + 1);
    }
}

inline void SIDataType::moveDataTypeParamDown(QTreeWidgetItem* node)
{
    QTreeWidgetItem* parent = node->parent();
    Q_ASSERT(parent != nullptr);
    int row = parent->indexOfChild(node);
    if ((row >= 0) && (row < (parent->childCount() - 1)))
    {
        swapDataTypeFields(node, parent, row, row + 1);
    }
}

inline void SIDataType::swapDataTypes(QTreeWidgetItem* node, int row, int moveRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* nodeSecond = table->topLevelItem(moveRow);
    Q_ASSERT(nodeSecond != nullptr);
    DataTypeCustom* first = node->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    DataTypeCustom* second = nodeSecond->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    mModel.swapDataTypes(*first, *second);

    table->takeTopLevelItem(row);
    table->insertTopLevelItem(moveRow, node);
    table->setCurrentItem(node);
    nodeSecond->setSelected(false);
    node->setSelected(true);
    updateToolButtons(moveRow, table->topLevelItemCount());
}

inline void SIDataType::swapDataTypeFields(QTreeWidgetItem* node, QTreeWidgetItem* parent, int row, int moveRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* nodeSecond = parent->child(moveRow);
    uint32_t firstId = node->data(1, Qt::ItemDataRole::UserRole).toUInt();
    uint32_t secondId = nodeSecond->data(1, Qt::ItemDataRole::UserRole).toUInt();
    DataTypeCustom* dataType = parent->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        mModel.swapStructureFields(*static_cast<DataTypeStructure*>(dataType), firstId, secondId);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        mModel.swapEnumFields(*static_cast<DataTypeEnum*>(dataType), firstId, secondId);
    }
    else
    {
        return;
    }

    node->setData(1, Qt::ItemDataRole::UserRole, secondId);
    nodeSecond->setData(1, Qt::ItemDataRole::UserRole, firstId);
    parent->takeChild(row);
    parent->insertChild(moveRow, node);
    table->setCurrentItem(node);
    nodeSecond->setSelected(false);
    node->setSelected(true);
    updateToolButtons(moveRow, parent->childCount());
}

inline void SIDataType::updateToolButtons(int row, int rowCount)
{
    if ((row >= 0) && (row < rowCount))
    {
        if ((row == 0) && (rowCount == 1))
        {
            mList->ctrlToolMoveUp()->setEnabled(false);
            mList->ctrlToolMoveDown()->setEnabled(false);
        }
        else if (row == 0)
        {
            mList->ctrlToolMoveUp()->setEnabled(false);
            mList->ctrlToolMoveDown()->setEnabled(true);
        }
        else if (row == (rowCount - 1))
        {
            mList->ctrlToolMoveUp()->setEnabled(true);
            mList->ctrlToolMoveDown()->setEnabled(false);
        }
        else
        {
            mList->ctrlToolMoveUp()->setEnabled(true);
            mList->ctrlToolMoveDown()->setEnabled(true);
        }

        // mList->ctrlToolRemove()->setEnabled(true);
    }
    else
    {
        mList->ctrlToolMoveUp()->setEnabled(false);
        mList->ctrlToolMoveDown()->setEnabled(false);
        // mList->ctrlToolRemove()->setEnabled(false);
    }
}
