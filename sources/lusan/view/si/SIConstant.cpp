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
 *  \file        lusan/view/si/SIConstant.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIConstant.hpp"
#include "ui/ui_SIConstant.h"

#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIConstantModel.hpp"
#include "lusan/view/common/TableCell.hpp"
#include "lusan/view/si/SIConstantDetails.hpp"
#include "lusan/view/si/SIConstantList.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

#include "lusan/view/si/SICommon.hpp"

SIConstantWidget::SIConstantWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIConstant)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIConstant::SIConstant(SIConstantModel& model, QWidget* parent)
    : QScrollArea(parent)
    , mModel    (model)
    , mDetails  (new SIConstantDetails(this))
    , mList     (new SIConstantList(this))
    , mWidget   (new SIConstantWidget(this))
    , ui        (*mWidget->ui)
    , mTypeModel(new DataTypesModel(model.getDataTypeData(), false))
    , mTableCell(nullptr)
    , mCount    (0)
{
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    setWidgetResizable(true);
    setWidget(mWidget);

    updateWidgets();    
    updateData();
    setupSignals();
}

SIConstant::~SIConstant(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIConstant::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    mTypeModel->dataTypeConverted(oldType, newType);
}

void SIConstant::dataTypeCreated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeCreated(dataType);
}

void SIConstant::dataTypeDeleted(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    mTypeModel->dataTypeDeleted(dataType);
    QTableWidget* table = mList->ctrlTableList();
    int count = table->rowCount();
    int current = table->currentRow();
    for (int i = 0; i < count; ++ i)
    {
        ConstantEntry * entry = getConstant(i);
        if ((entry != nullptr) && entry->getParamType() == static_cast<DataTypeBase *>(dataType))
        {
            entry->setParamType(nullptr);
            setTexts(i, *entry);
            if (i == current)
            {
                updateDetails(entry, false);
            }
        }
    }
    
    blockBasicSignals(false);
}

void SIConstant::dataTypeUpdated(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    Q_ASSERT(dataType != nullptr);
    mTypeModel->dataTypeUpdated(dataType);
    QTableWidget* table = mList->ctrlTableList();
    int count = table->rowCount();
    int current = table->currentRow();
    for (int i = 0; i < count; ++ i)
    {
        ConstantEntry * entry = getConstant(i);
        if ((entry != nullptr) && entry->getParamType() == static_cast<DataTypeBase *>(dataType))
        {
            setTexts(i, *entry);
            if (i == current)
            {
                updateDetails(entry, false);
            }
        }
    }
    
    blockBasicSignals(false);
}


void SIConstant::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == previousRow)
        return;
    
    blockBasicSignals(true);
    QTableWidget * table = mList->ctrlTableList();
    QTableWidgetItem * col1 = currentRow >= 0 ? table->item(currentRow, static_cast<int>(eColumn::ColType)) : nullptr;
    const ConstantEntry * entry = getConstant(currentRow);
    updateDetails(entry, true);

    if (entry != nullptr)
    {
        if (currentRow == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        else if (currentRow == static_cast<int>(mList->ctrlTableList()->rowCount() - 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        
        mList->ctrlButtonRemove()->setEnabled(true);
    }
    else
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
    
    blockBasicSignals(false);
}

void SIConstant::onAddClicked(void)
{
    static const QString _defName("NewConstant");
    
    QTableWidget *table = mList->ctrlTableList();
    QString name;
    do
    {
        name =_defName + QString::number(++ mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly).isEmpty() == false);
    
    blockBasicSignals(true);
    ConstantEntry * entry = mModel.createConstant(name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlValue()->setEnabled(true);
        
        QTableWidgetItem * current = table->currentItem();
        if (current != nullptr)
        {
            current->setSelected(false);
        }
        
        int row = table->rowCount();
        setTexts(-1, *entry);
        table->selectRow(row);
        table->scrollToBottom();
        updateDetails(entry, true);
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
    
    blockBasicSignals(false);
}

void SIConstant::onRemoveClicked(void)
{
}

void SIConstant::onInsertClicked(void)
{
}

void SIConstant::onNameChanged(const QString& newName)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    ConstantEntry* entry = getConstant(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);
        entry->setName(newName);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIConstant::onTypeChanged(const QString& newType)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    ConstantEntry* entry = getConstant(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);

        DataTypeBase* dataType = mTypeModel->findDataType(newType);
        entry->setParamType(dataType);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIConstant::onValueChanged(const QString& newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    ConstantEntry* entry = getConstant(row);
    if (entry != nullptr)
    {
        entry->setValue(newValue);
        setTexts(row, *entry);
    }
}

void SIConstant::onEditorDataChanged(const QModelIndex &index, const QString &newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->rowCount()) || (index.column() < 0))
        return;
    
    cellChanged(index.row(), index.column(), newValue);
}

void SIConstant::onDeprectedChecked(bool isChecked)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        ConstantEntry* entry = getConstant(row);
        Q_ASSERT(entry != nullptr);
        SICommon::checkedDeprecated<SIConstantDetails, ConstantEntry>(mDetails, entry, isChecked);
    }
}

void SIConstant::onDeprecateHintChanged(const QString& newText)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        ConstantEntry* entry = getConstant(row);
        Q_ASSERT(entry != nullptr);
        SICommon::setDeprecateHint<SIConstantDetails, ConstantEntry>(mDetails, entry, newText);
    }
}

void SIConstant::onDescriptionChanged(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        ConstantEntry* entry = getConstant(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIConstant::cellChanged(int row, int col, const QString& newValue)
{
    ConstantEntry* entry = getConstant(row);
    Q_ASSERT(entry != nullptr);
    
    if (col == 0)
    {
        if (mDetails->ctrlName()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setName(newValue);
            setTexts(row, *entry);
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
    else if (col == 1)
    {
        if (mDetails->ctrlTypes()->currentText() != newValue)
        {
            blockBasicSignals(true);
            DataTypeBase* dataType = mTypeModel->findDataType(newValue);
            entry->setParamType(dataType);
            setTexts(row, *entry);
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
    else if (col == 2)
    {
        if (mDetails->ctrlValue()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setValue(newValue);
            setTexts(row, *entry);
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
}
    
void SIConstant::updateData(void)
{
    QTableWidget* table = mList->ctrlTableList();
    const QList<ConstantEntry>& list = mModel.getConstants();
    if (list.isEmpty() == false)
    {
        for (const ConstantEntry & entry : list)
        {
            setTexts(-1, entry);
        }
        
        table->scrollToTop();
    }
}

void SIConstant::updateWidgets(void)
{
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();
    
    mTableCell = new TableCell(QList<QAbstractItemModel *>{mTypeModel}, QList<int>{1}, mList->ctrlTableList());
    mDetails->ctrlTypes()->setModel(mTypeModel);
    mList->ctrlTableList()->setItemDelegateForColumn(0, mTableCell);
    mList->ctrlTableList()->setItemDelegateForColumn(1, mTableCell);
    mList->ctrlTableList()->setItemDelegateForColumn(2, mTableCell);

    SICommon::enableDeprecated<SIConstantDetails, ConstantEntry>(mDetails, nullptr, false);

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);
    mDetails->ctrlValue()->setEnabled(false);
}

void SIConstant::setupSignals(void)
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);
    
    connect(mList->ctrlTableList(),    &QTableWidget::currentCellChanged, this, &SIConstant::onCurCellChanged);
    connect(mList->ctrlButtonAdd(),    &QToolButton::clicked        , this, &SIConstant::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked        , this, &SIConstant::onRemoveClicked);
    connect(mList->ctrlButtonInsert(), &QToolButton::clicked        , this, &SIConstant::onInsertClicked);
    connect(mDetails->ctrlName(),      &QLineEdit::textChanged      , this, &SIConstant::onNameChanged);
    connect(mDetails->ctrlTypes(),     &QComboBox::currentTextChanged, this, &SIConstant::onTypeChanged);
    connect(mDetails->ctrlValue(),     &QLineEdit::textChanged      , this, &SIConstant::onValueChanged);
    connect(mDetails->ctrlDeprecated(),&QCheckBox::toggled          , this, &SIConstant::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint(),&QLineEdit::textEdited    , this, &SIConstant::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription(),&QPlainTextEdit::textChanged, this, &SIConstant::onDescriptionChanged);
    connect(mTableCell                , &TableCell::editorDataChanged,this, &SIConstant::onEditorDataChanged);
}

void SIConstant::blockBasicSignals(bool doBlock)
{
    mList->ctrlTableList()->blockSignals(doBlock);
    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlTypes()->blockSignals(doBlock);
    mDetails->ctrlValue()->blockSignals(doBlock);
}

inline void SIConstant::setTexts(int row, const ConstantEntry & entry)
{
    QTableWidget * table = mList->ctrlTableList();
    if (row < 0)
    {
        row = table->rowCount();
        QTableWidgetItem * col0 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayName), entry.getString(ElementBase::eDisplay::DisplayName));
        QTableWidgetItem * col1 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayType), entry.getString(ElementBase::eDisplay::DisplayType));
        QTableWidgetItem * col2 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayValue), entry.getString(ElementBase::eDisplay::DisplayValue));
        col0->setData(Qt::ItemDataRole::UserRole, entry.getId());
        col1->setData(Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase *>(entry.getParamType()));
        table->setRowCount(row + 1);
        table->setItem(row, static_cast<int>(eColumn::ColName), col0);
        table->setItem(row, static_cast<int>(eColumn::ColType), col1);
        table->setItem(row, static_cast<int>(eColumn::ColValue), col2);
    }
    else
    {
        QTableWidgetItem * col0 = table->item(row, static_cast<int>(eColumn::ColName));
        QTableWidgetItem * col1 = table->item(row, static_cast<int>(eColumn::ColType));
        QTableWidgetItem * col2 = table->item(row, static_cast<int>(eColumn::ColValue));
        
        Q_ASSERT(col0->data(Qt::ItemDataRole::UserRole).toUInt() == entry.getId());
        col1->setData(Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase*>(entry.getParamType()));

        col0->setIcon(entry.getIcon(ElementBase::eDisplay::DisplayName));
        col1->setIcon(entry.getIcon(ElementBase::eDisplay::DisplayType));
        col2->setIcon(entry.getIcon(ElementBase::eDisplay::DisplayValue));
        
        col0->setText(entry.getString(ElementBase::eDisplay::DisplayName));
        col1->setText(entry.getString(ElementBase::eDisplay::DisplayType));
        col2->setText(entry.getString(ElementBase::eDisplay::DisplayValue));
    }
}

inline void SIConstant::updateDetails(const ConstantEntry* entry, bool updateAll /*= false*/)
{
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlValue()->setText(entry->getValue());
        if (entry->isValid())
        {
            mDetails->ctrlTypes()->setCurrentText(entry->getType());
        }
        else
        {
            mDetails->ctrlTypes()->setCurrentIndex(0);
        }

        if (updateAll)
        {
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
            SICommon::enableDeprecated<SIConstantDetails, ConstantEntry>(mDetails, entry, true);
        }
    }
    else
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentText("");
        mDetails->ctrlValue()->setText("");
        mDetails->ctrlDescription()->setPlainText("");

        SICommon::enableDeprecated<SIConstantDetails, ConstantEntry>(mDetails, nullptr, false);

        mDetails->ctrlName()->setEnabled(false);
        mDetails->ctrlTypes()->setEnabled(false);
        mDetails->ctrlValue()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline const ConstantEntry* SIConstant::getConstant(int row) const
{
    QTableWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;
    
    QTableWidgetItem* col0 = table->item(row, static_cast<int>(eColumn::ColName));
    uint32_t id = col0->data(Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findConstant(id);
}

inline ConstantEntry* SIConstant::getConstant(int row)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;
    
    QTableWidgetItem* col0 = table->item(row, static_cast<int>(eColumn::ColName));
    uint32_t id = col0->data(Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findConstant(id);
}
