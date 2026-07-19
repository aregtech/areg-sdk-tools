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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIDataType.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIDataType.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIDataTypeModel.hpp"
#include "lusan/view/common/DataTypeDetailsView.hpp"
#include "lusan/view/common/DataTypeFieldDetailsView.hpp"
#include "lusan/view/common/DataTypeListView.hpp"
#include "lusan/view/common/TableCell.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

#include "lusan/view/si/SICommon.hpp"

SIDataTypeWidget::SIDataTypeWidget(QWidget* parent)
    : QWidget{ parent }
    , mPanels(nullptr)
{
    QVBoxLayout* root = new QVBoxLayout(this);

    QLabel* headline = new QLabel(tr("Service Interface Data Type Editor ..."), this);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    mPanels = new QHBoxLayout();
    root->addLayout(mPanels, 1);
}

const QList<DataTypeBasicContainer *> & SIDataType::_getContainerTypes()
{
    static QList<DataTypeBasicContainer *> _result;
    if (_result.isEmpty())
    {
        _result = DataTypeFactory::getContainerTypes();
    }
    
    return _result;
}

const QList<DataTypeBase *>& SIDataType::_getIntegerTypes()
{
    static QList<DataTypeBase *> _result;
    if (_result.isEmpty())
    {
        DataTypeFactory::getPredefinedTypes(_result, QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::PrimitiveSint, DataTypeBase::eCategory::PrimitiveUint});
    }
    
    return _result;
}

const QList<DataTypeBase *>& SIDataType::_getPredefinedTypes()
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
    , mDetails  (new DataTypeDetailsView(this))
    , mList     (new DataTypeListView(this))
    , mFields   (new DataTypeFieldDetailsView(this))
    , mRightPanel(new QWidget(this))
    , mWidget   (new SIDataTypeWidget(this))
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

    QVBoxLayout* rightLayout = new QVBoxLayout(mRightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(mDetails);
    rightLayout->addWidget(mFields);

    // Ignored so the row splits by stretch alone, not by each child's size hint —
    // see .claude/memory/equal-split-two-panels.md.
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mRightPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mWidget->mPanels->addWidget(mList, 1);
    mWidget->mPanels->addWidget(mRightPanel, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(mWidget);

    updateData();
    updateWidgets();
    setupSignals();
}

SIDataType::~SIDataType()
{
    mWidget->mPanels->removeWidget(mList);
    mWidget->mPanels->removeWidget(mRightPanel);
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

void SIDataType::onAddClicked()
{
    addNewType(DataTypeBase::eCategory::Structure);
}

void SIDataType::addNewType(DataTypeBase::eCategory category)
{
    QString name(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);

    DataTypeCustom* oldType  = nullptr;
    DataTypeCustom* dataType = mModel.createDataType(name, category);
    Q_ASSERT(dataType != nullptr);

    int pos = table->topLevelItemCount();
    QTreeWidgetItem* item{ nullptr };
    switch (category)
    {
    case DataTypeBase::eCategory::Structure:
        item = createNodeStructure(static_cast<DataTypeStructure*>(dataType));
        break;
    case DataTypeBase::eCategory::Enumeration:
        item = createNodeEnum(static_cast<DataTypeEnum*>(dataType));
        break;
    case DataTypeBase::eCategory::Imported:
        item = createNodeImported(static_cast<DataTypeImported*>(dataType));
        break;
    case DataTypeBase::eCategory::Container:
        item = createNodeContainer(static_cast<DataTypeContainer*>(dataType));
        break;
    default:
        break;
    }
    Q_ASSERT(item != nullptr);

    table->addTopLevelItem(item);
    QTreeWidgetItem* cur = table->currentItem();
    if (cur != nullptr)
    {
        oldType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        cur->setSelected(false);
    }

    item->setSelected(true);
    table->setCurrentItem(item);

    switch (category)
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

    updateToolButtons(pos, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIDataType::onInsertClicked()
{
    QString name(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    
    QTreeWidgetItem* cur = table->currentItem();
    uint32_t id = cur != nullptr ? cur->data(1, Qt::ItemDataRole::UserRole).toUInt() : 0;
    QTreeWidgetItem* top = id == 0 ? cur : cur->parent();
    int row = top != nullptr ? table->indexOfTopLevelItem(top) : 0;
    row = row < 0 ? 0 : row;
    DataTypeCustom* oldType = nullptr;
    DataTypeCustom* dataType = mModel.insertDataType(row, name, DataTypeBase::eCategory::Structure);
    QTreeWidgetItem* item = createNodeStructure(static_cast<DataTypeStructure*>(dataType));
    table->insertTopLevelItem(row, item);
    if (cur != nullptr)
    {
        oldType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
        cur->setSelected(false);
    }

    item->setSelected(true);
    table->setCurrentItem(item);
    selectedStruct(oldType, static_cast<DataTypeStructure*>(dataType));
    updateToolButtons(row, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIDataType::onAddFieldClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* cur = table->currentItem();
    
    Q_ASSERT(cur != nullptr);
    QTreeWidgetItem* parent = cur->parent();
    parent = parent == nullptr ? cur : parent;
    
    DataTypeCustom* dataType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom *>();
    Q_ASSERT((dataType->getCategory() == DataTypeBase::eCategory::Structure) || (dataType->getCategory() == DataTypeBase::eCategory::Enumeration));
    QString name(genName(dataType));
    ElementBase* field = mModel.ceateDataTypeChild(dataType, name);
    if (field != nullptr)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = new QTreeWidgetItem();

        int pos = parent->childCount();
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

        updateToolButtons(pos, parent->childCount());
        blockBasicSignals(false);
    }
}

void SIDataType::onInsertFieldClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* cur = table->currentItem();

    Q_ASSERT(cur != nullptr);
    QTreeWidgetItem* parent = cur->parent();
    QTreeWidgetItem* child = parent != nullptr ? cur : nullptr;
    parent = parent == nullptr ? cur : parent;

    DataTypeCustom* dataType = cur->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    Q_ASSERT((dataType->getCategory() == DataTypeBase::eCategory::Structure) || (dataType->getCategory() == DataTypeBase::eCategory::Enumeration));
    QString name(genName(dataType));
    int row = child != nullptr ? parent->indexOfChild(child) : 0;
    row = row < 0 ? 0 : row;
    ElementBase* field = mModel.insertDataTypeChild(row, dataType, name);
    if (field != nullptr)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = new QTreeWidgetItem();

        parent->insertChild(row, item);
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
        const int count {parent->childCount()};

        if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        {
            updateChildNodeStruct(item, static_cast<DataTypeStructure*>(dataType), *static_cast<FieldEntry*>(field));
            selectedStructField(nullptr, *static_cast<FieldEntry*>(field), static_cast<DataTypeStructure*>(dataType));
            const QList<FieldEntry>& list = static_cast<DataTypeStructure*>(dataType)->getElements();
            Q_ASSERT(count == list.size());
            for (int i = row + 1; i < count; ++i)
            {
                QTreeWidgetItem* temp = parent->child(i);
                Q_ASSERT(temp != nullptr);
                temp->setData(1, Qt::ItemDataRole::UserRole, list[i].getId());
            }
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            updateChildNodeEnum(item, static_cast<DataTypeEnum*>(dataType), *static_cast<EnumEntry*>(field));
            selectedEnumField(nullptr, *static_cast<EnumEntry*>(field), static_cast<DataTypeEnum*>(dataType));
            const QList<EnumEntry>& list = static_cast<DataTypeEnum*>(dataType)->getElements();
            Q_ASSERT(count == list.size());
            for (int i = row + 1; i < count; ++i)
            {
                QTreeWidgetItem* temp = parent->child(i);
                Q_ASSERT(temp != nullptr);
                temp->setData(1, Qt::ItemDataRole::UserRole, list[i].getId());
            }
        }
        else
        {
            Q_ASSERT(false); // this should never happen
        }

        updateToolButtons(row, count);
        blockBasicSignals(false);
    }
}

void SIDataType::onRemoveClicked()
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
        row = table->indexOfTopLevelItem(next);
        rowCount = table->topLevelItemCount();
    }
    else
    {
        showClean();
    }

    updateToolButtons(row, rowCount);
}

void SIDataType::onRemoveFieldClicked()
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
    int row = -1;
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

void SIDataType::onMoveUpClicked()
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

void SIDataType::onMoveDownClicked()
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
    SICommon::checkedDeprecated<DataTypeDetailsView, DataTypeCustom>(mDetails, entry, isChecked);
}

void SIDataType::onDeprecateHintChanged(const QString& newText)
{
    QTreeWidgetItem * item = mList->ctrlTableList()->currentItem();
    if (item == nullptr)
        return;

    DataTypeCustom* entry = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    Q_ASSERT(entry != nullptr);
    SICommon::setDeprecateHint<DataTypeDetailsView, DataTypeCustom>(mDetails, entry, newText);
}

void SIDataType::onDescriptionChanged()
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

void SIDataType::onImportLocationBrowse()
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

void SIDataType::onFieldDescriptionChanged()
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
            SICommon::checkedDeprecated<DataTypeFieldDetailsView, FieldEntry>(mFields, static_cast<FieldEntry*>(entry), isChecked);
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            SICommon::checkedDeprecated<DataTypeFieldDetailsView, EnumEntry>(mFields, static_cast<EnumEntry*>(entry), isChecked);
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
            SICommon::setDeprecateHint<DataTypeFieldDetailsView, FieldEntry>(mFields, static_cast<FieldEntry*>(field), newText);
        }
        else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        {
            SICommon::setDeprecateHint<DataTypeFieldDetailsView, EnumEntry>(mFields, static_cast<EnumEntry*>(field), newText);
        }
        else
        {
            Q_ASSERT(false);
        }
    }
}

void SIDataType::showEnumDetails(bool show)
{
    mDetails->setEnumRowVisible(show);
}

void SIDataType::showImportDetails(bool show)
{
    mDetails->setImportRowVisible(show);
}

void SIDataType::showContainerDetails(bool show)
{
    mDetails->setContainerRowVisible(show);
}

void SIDataType::updateData()
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

void SIDataType::updateWidgets()
{
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();
    mValueModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mValueModel->updateDataTypeLists();
    mKeysModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mKeysModel->updateDataTypeLists();
    
    // Inline (in-table) editing: the shared TableCell delegate opens an editor per cell. The tree
    // is heterogeneous (four data type categories plus struct/enum fields), so which cells are
    // editable, whether a cell shows a combo or a text editor, and how it is validated are all
    // resolved per cell by the controller. See cellChanged() for the commit routing.
    QTreeWidget* treeList = mList->ctrlTableList();
    mTableCell = new TableCell(treeList, this, false);
    mTableCell->setEditableCheck([this](const QModelIndex& idx) { return isCellEditable(idx); });
    mTableCell->setEditorModelResolver([this](const QModelIndex& idx) { return editorModelFor(idx); });
    mTableCell->setValidationResolver([this](const QModelIndex& idx) { return validationFor(idx); });
    treeList->setItemDelegateForColumn(0, mTableCell);
    treeList->setItemDelegateForColumn(1, mTableCell);
    treeList->setItemDelegateForColumn(2, mTableCell);

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

void SIDataType::setupSignals()
{
    connect(mList->ctrlTableList()          , &QTreeWidget::currentItemChanged  , this, &SIDataType::onCurCellChanged);
    connect(mList->ctrlButtonAdd()          , &QToolButton::clicked             , this, &SIDataType::onAddClicked);
    connect(mList->ctrlButtonAddField()     , &QToolButton::clicked             , this, &SIDataType::onAddFieldClicked);
    connect(mList->ctrlButtonInsert()       , &QToolButton::clicked             , this, &SIDataType::onInsertClicked);
    connect(mList->ctrlButtonInsertField()  , &QToolButton::clicked             , this, &SIDataType::onInsertFieldClicked);    
    connect(mList->ctrlButtonRemove()       , &QToolButton::clicked             , this, &SIDataType::onRemoveClicked);
    connect(mList->ctrlButtonRemoveField()  , &QToolButton::clicked             , this, &SIDataType::onRemoveFieldClicked);
    connect(mList->ctrlButtonMoveUp()       , &QToolButton::clicked             , this, &SIDataType::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown()     , &QToolButton::clicked             , this, &SIDataType::onMoveDownClicked);
    connect(mList->actionNewStruct()        , &QAction::triggered               , this, [this]() { addNewType(DataTypeBase::eCategory::Structure); });
    connect(mList->actionNewEnum()          , &QAction::triggered               , this, [this]() { addNewType(DataTypeBase::eCategory::Enumeration); });
    connect(mList->actionNewImport()        , &QAction::triggered               , this, [this]() { addNewType(DataTypeBase::eCategory::Imported); });
    connect(mList->actionNewContainer()     , &QAction::triggered               , this, [this]() { addNewType(DataTypeBase::eCategory::Container); });

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

    connect(mTableCell                      , &TableCell::signalEditorDataChanged, this, &SIDataType::onEditorDataChanged);
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

    SICommon::enableDeprecated<DataTypeDetailsView, DataTypeCustom>(mDetails, dataType, true);

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);    
    mList->ctrlButtonRemoveField()->setEnabled(false);

    int index = mModel.findIndex(dataType->getId());
    mList->ctrlButtonMoveUp()->setEnabled(index > 0);
    mList->ctrlButtonMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
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
    
    SICommon::enableDeprecated<DataTypeDetailsView, DataTypeCustom>(mDetails, dataType, true);
    
    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);    
    mList->ctrlButtonRemoveField()->setEnabled(false);
    
    int index = mModel.findIndex(dataType->getId());
    mList->ctrlButtonMoveUp()->setEnabled(index > 0);
    mList->ctrlButtonMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
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
    
    SICommon::enableDeprecated<DataTypeDetailsView, DataTypeCustom>(mDetails, dataType, true);

    mDetails->ctrlImportLocation()->setText(dataType->getLocation());
    mDetails->ctrlImportNamespace()->setText(dataType->getNamespace());
    mDetails->ctrlImportObject()->setText(dataType->getObject());
    mDetails->ctrlButtonBrowse()->setEnabled(true);

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddField()->setEnabled(false);
    mList->ctrlButtonInsertField()->setEnabled(false);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    
    int index = mModel.findIndex(dataType->getId());
    mList->ctrlButtonMoveUp()->setEnabled(index > 0);
    mList->ctrlButtonMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
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
    
    SICommon::enableDeprecated<DataTypeDetailsView, DataTypeCustom>(mDetails, dataType, true);

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

    mList->ctrlButtonRemove()->setEnabled(true);
    mList->ctrlButtonAddField()->setEnabled(false);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    mList->ctrlButtonInsertField()->setEnabled(false);

    int index = mModel.findIndex(dataType->getId());
    mList->ctrlButtonMoveUp()->setEnabled(index > 0);
    mList->ctrlButtonMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
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

    SICommon::enableDeprecated<DataTypeFieldDetailsView, FieldEntry>(mFields, &field, true);
    
    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);
    mList->ctrlButtonRemoveField()->setEnabled(true);
    
    int index = parent->findIndex(field.getId());    
    mList->ctrlButtonMoveUp()->setEnabled(index > 0);
    mList->ctrlButtonMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
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
    
    SICommon::enableDeprecated<DataTypeFieldDetailsView, EnumEntry>(mFields, &field, true);
    
    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddField()->setEnabled(true);
    mList->ctrlButtonInsertField()->setEnabled(true);
    mList->ctrlButtonRemoveField()->setEnabled(true);
    
    int index = parent->findIndex(field.getId());    
    mList->ctrlButtonMoveUp()->setEnabled(index > 0);
    mList->ctrlButtonMoveDown()->setEnabled((index >= 0) && (index < (mModel.getDataTypeCount() - 1)));
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

inline ElementBase* SIDataType::getSelectedField() const
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

    // Tree items are not editable by default (unlike table items); set the flag so the shared
    // TableCell delegate can open an inline editor. Which columns actually open is decided
    // per-cell by isCellEditable().
    node->setFlags(node->flags() | Qt::ItemIsEditable);

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
        node->setIcon(0, NELusanCommon::iconWarning(NELusanCommon::SizeSmall));
        node->setText(0, "<invalid>");
    }
}

inline void SIDataType::showClean()
{
    SICommon::enableDeprecated<DataTypeDetailsView, DataTypeCustom>(mDetails, nullptr, false);
    SICommon::enableDeprecated<DataTypeFieldDetailsView, FieldEntry>(mFields, nullptr, false);
    
    mDetails->ctrlName()->clear();
    showEnumDetails(false);
    showContainerDetails(false);
    showImportDetails(false);
    
    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonAddField()->setEnabled(false);
    mList->ctrlButtonRemoveField()->setEnabled(false);
    mList->ctrlButtonInsertField()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
    
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
        bool isExpanded = node->isExpanded();
        swapDataTypes(node, row, row + 1);
        node->setExpanded(isExpanded);
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
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else if (row == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        else if (row == (rowCount - 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
    }
    else
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

inline QString SIDataType::genName()
{
    static const QString _defName("NewDataType");

    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (mModel.findDataType(name) != nullptr);

    return name;
}

inline QString SIDataType::genName(DataTypeCustom* dataType)
{
    static const QString _defName("newField");

    Q_ASSERT(dataType != nullptr);
    uint32_t cnt{ 0 };
    QString name;
    do
    {
        name = _defName + QString::number(++cnt);
    } while (mModel.findChildIndex(dataType, name) != -1);

    return name;
}

int SIDataType::getColumnCount() const
{
    return mList->ctrlTableList()->columnCount();
}

QString SIDataType::getCellText(const QModelIndex& cell) const
{
    // The display text already stored on the tree cell seeds the inline editor.
    return cell.isValid() ? cell.data(Qt::DisplayRole).toString() : QString();
}

bool SIDataType::isCellEditable(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return false;

    DataTypeCustom* dataType = index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType == nullptr)
        return false;

    const uint32_t id = index.sibling(index.row(), 1).data(Qt::ItemDataRole::UserRole).toUInt();
    const int col = index.column();
    if (id == 0)
    {
        // Top-level data type node: which columns are editable depends on the category.
        switch (dataType->getCategory())
        {
        case DataTypeBase::eCategory::Structure:    return (col == 0);                   // only the name
        case DataTypeBase::eCategory::Enumeration:  return (col == 0) || (col == 1);     // name + derived type
        case DataTypeBase::eCategory::Imported:     return (col == 0) || (col == 1);     // name + namespace::object
        case DataTypeBase::eCategory::Container:    return (col == 0);                   // name only (type in details)
        default:                                    return false;
        }
    }

    // Field node: structure fields edit name/type/value; enumeration entries edit name/value.
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        return (col == 0) || (col == 1) || (col == 2);
    if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
        return (col == 0) || (col == 2);

    return false;
}

QAbstractItemModel* SIDataType::editorModelFor(const QModelIndex& index) const
{
    // The Data Type column is a combo in two cases: an enumeration's derived integer type (top
    // node) and a structure field's type. Imported type and enumeration values use a line editor.
    if ((index.isValid() == false) || (index.column() != 1))
        return nullptr;

    DataTypeCustom* dataType = index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType == nullptr)
        return nullptr;

    const uint32_t id = index.sibling(index.row(), 1).data(Qt::ItemDataRole::UserRole).toUInt();
    if (id == 0)
    {
        // Enumeration top node: pick the derived integer type from the SAME model that backs the
        // details Derived combo, so the inline list and the Derived list are identical and stay
        // in sync by construction (both edit the enum's derived type).
        if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
            return mDetails->ctrlEnumDerived()->model();

        return nullptr;
    }

    // Structure field: the type combo is backed by the shared type model.
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
        return mTypeModel;

    return nullptr;
}

TableCell::eCellValidation SIDataType::validationFor(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return TableCell::eCellValidation::NoValidation;

    const int col = index.column();
    if (col == 0)
        return TableCell::eCellValidation::Identifier;

    DataTypeCustom* dataType = index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType == nullptr)
        return TableCell::eCellValidation::NoValidation;

    const uint32_t id = index.sibling(index.row(), 1).data(Qt::ItemDataRole::UserRole).toUInt();
    if (col == 1)
    {
        if (id == 0)
        {
            if (dataType->getCategory() == DataTypeBase::eCategory::Imported)
                return TableCell::eCellValidation::QualifiedName;   // namespace::object
            if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
                return TableCell::eCellValidation::Identifier;      // a derived integer type name
        }
        return TableCell::eCellValidation::NoValidation;            // a structure field type is a combo
    }

    // Column 2 (value): an enumeration value is restricted to C++ value characters; a structure
    // default value stays unrestricted (it may be a number, string literal, expression, etc.).
    if ((id != 0) && (dataType->getCategory() == DataTypeBase::eCategory::Enumeration))
        return TableCell::eCellValidation::Value;

    return TableCell::eCellValidation::NoValidation;
}

void SIDataType::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if (index.isValid() == false)
        return;

    // The edited node is the current item (inline editing starts on the current index).
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item != nullptr)
    {
        cellChanged(item, index.column(), newValue);
    }
}

void SIDataType::cellChanged(QTreeWidgetItem* item, int column, const QString& newValue)
{
    Q_ASSERT(item != nullptr);
    DataTypeCustom* dataType = item->data(0, Qt::ItemDataRole::UserRole).value<DataTypeCustom*>();
    if (dataType == nullptr)
        return;

    const uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    blockBasicSignals(true);

    if (id == 0)
    {
        // Top-level data type node.
        if (column == 0)
        {
            if (dataType->getName() != newValue)
            {
                mModel.updateDataType(dataType, newValue);
                setNodeText(item, dataType);
                mDetails->ctrlName()->setText(dataType->getName());
            }
        }
        else if (column == 1)
        {
            if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
            {
                DataTypeEnum* typeEnum = static_cast<DataTypeEnum*>(dataType);
                typeEnum->setDerived(newValue);
                setNodeText(item, typeEnum);
                mDetails->ctrlEnumDerived()->setCurrentText(newValue);
            }
            else if (dataType->getCategory() == DataTypeBase::eCategory::Imported)
            {
                DataTypeImported* typeImport = static_cast<DataTypeImported*>(dataType);
                // Parse the "<namespace::>object" pattern typed into the Data Type column and
                // mirror both halves into the details Namespace/Object fields.
                QString nameSpace;
                QString object;
                const int pos = newValue.lastIndexOf(QStringLiteral("::"));
                if (pos >= 0)
                {
                    nameSpace = newValue.left(pos);
                    object = newValue.mid(pos + 2);
                }
                else
                {
                    object = newValue;
                }

                typeImport->setNamespace(nameSpace);
                typeImport->setObject(object);
                setNodeText(item, typeImport);
                mDetails->ctrlImportNamespace()->setText(nameSpace);
                mDetails->ctrlImportObject()->setText(object);
            }
        }
    }
    else
    {
        // Field node: a structure field or an enumeration entry.
        ElementBase* field = mModel.findChild(dataType, id);
        if (field != nullptr)
        {
            if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
            {
                FieldEntry* entry = static_cast<FieldEntry*>(field);
                if (column == 0)
                {
                    entry->setName(newValue);
                    mFields->ctrlName()->setText(newValue);
                }
                else if (column == 1)
                {
                    entry->setParamType(mTypeModel->findDataType(newValue));
                    mFields->ctrlTypes()->setCurrentText(newValue);
                }
                else if (column == 2)
                {
                    entry->setValue(newValue);
                    mFields->ctrlValue()->setText(newValue);
                }

                setNodeText(item, entry);
            }
            else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
            {
                EnumEntry* entry = static_cast<EnumEntry*>(field);
                if (column == 0)
                {
                    entry->setName(newValue);
                    mFields->ctrlName()->setText(newValue);
                }
                else if (column == 2)
                {
                    entry->setValue(newValue);
                    mFields->ctrlValue()->setText(newValue);
                }

                setNodeText(item, entry);
            }
        }
    }

    blockBasicSignals(false);
}
