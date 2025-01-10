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

#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/model/si/SIDataTypeModel.hpp"
#include "lusan/view/si/SIDataTypeDetails.hpp"
#include "lusan/view/si/SIDataTypeFieldDetails.hpp"
#include "lusan/view/si/SIDataTypeList.hpp"

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
    
    // setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    
    updateWidgets();
    setupSignals();
}

SIDataType::~SIDataType(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
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

void SIDataType::onNameChanged(const QString& newName)
{
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
    connect(mList->ctrlToolRemove()         , &QToolButton::clicked             , this, &SIDataType::onRemoveClicked);
    connect(mDetails->ctrlName()            , &QLineEdit::textChanged           , this, &SIDataType::onNameChanged);
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
    if (mDetails->isHidden())
    {
        mFields->hide();
        mDetails->show();
    }
    
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
    if (mDetails->isHidden())
    {
        mFields->hide();
        mDetails->show();
    }
    
    showEnumDetails(false);
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
    if (mDetails->isHidden())
    {
        mFields->hide();
        mDetails->show();
    }
    
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
    if (mDetails->isHidden())
    {
        mFields->hide();
        mDetails->show();
    }
    
    showEnumDetails(false);
    showImportDetails(true);
    showContainerDetails(false);

    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeContainer()->setChecked(true);
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
    mDetails->ctrlDeprecated()->setChecked(dataType->getIsDeprecated());
    mDetails->ctrlDeprecateHint()->setText(dataType->getDeprecateHint());

    mDetails->ctrlTypeContainer()->setText(dataType->getContainer());
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
    if (mFields->isHidden())
    {
        mDetails->hide();
        mFields->show();
    }
    
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
    if (mFields->isHidden())
    {
        mDetails->hide();
        mFields->show();
    }

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
    else
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
