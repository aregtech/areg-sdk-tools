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
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIConstant.h"

#include "lusan/view/si/SIConstantDetails.hpp"
#include "lusan/view/si/SIConstantList.hpp"
#include "lusan/model/si/SIConstantModel.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

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
    , mTypeModel(new DataTypesModel(model.getDataTypeData()))
    , mTableCell(nullptr)
    , mCount    (0)
{
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
        
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

void SIConstant::dataTypeRemoved(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeRemoved(dataType);
}

void SIConstant::dataTypeUpdated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeUpdated(dataType);
}


inline ConstantEntry* SIConstant::_findConstant(int row)
{
    QTableWidget* table = mList->ctrlTableList();
    if (row < 0 || row > table->rowCount())
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findConstant(id);
}

inline const ConstantEntry* SIConstant::_findConstant(int row) const
{
    QTableWidget* table = mList->ctrlTableList();
    if (row < 0 || row > table->rowCount())
        return nullptr;
    
    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findConstant(id);
}

void SIConstant::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    blockBasicSignals(true);
    if (currentRow == -1)
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentText("");
        mDetails->ctrlValue()->setText("");
        mDetails->ctrlDescription()->setPlainText("");
        mDetails->ctrlDeprecateHint()->setText("");
        mDetails->ctrlDeprecated()->setChecked(false);

        mDetails->ctrlName()->setEnabled(false);
        mDetails->ctrlTypes()->setEnabled(false);
        mDetails->ctrlValue()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
    else if (currentRow != previousRow)
    {
        const ConstantEntry* entry = _findConstant(currentRow);
        Q_ASSERT(entry != nullptr);
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlValue()->setText(entry->getValue());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlDeprecated()->setChecked(entry->getIsDeprecated());
        
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
    
    uint32_t id = mModel.createConstant(name);
    if (id != 0)
    {
        blockBasicSignals(true);
        
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlValue()->setEnabled(true);

        mDetails->ctrlTypes()->setCurrentIndex(0);
        ConstantEntry * entry = mModel.findConstant(id);
        entry->setType(mDetails->ctrlTypes()->currentText());
        
        QTableWidgetItem * current = table->currentItem();
        if (current != nullptr)
        {
            current->setSelected(false);
        }
        
        int row = table->rowCount();
        table->setRowCount(row + 1);
        QTableWidgetItem* col0 = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::MediaRecord), entry->getName());
        QTableWidgetItem* col2 = new QTableWidgetItem(entry->getValue());
        QTableWidgetItem* col1 = new QTableWidgetItem(entry->getType());
        col0->setData(static_cast<int>(Qt::ItemDataRole::UserRole)   , entry->getId());

        table->setItem(row, 0, col0);
        table->setItem(row, 1, col1);
        table->setItem(row, 2, col2);
        table->selectRow(row);
        table->scrollToItem(col0);
        
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlValue()->setText(entry->getValue());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlDeprecated()->setChecked(entry->getIsDeprecated());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
        
        blockBasicSignals(false);
    }
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
    if (row >= 0)
    {
        blockBasicSignals(true);
        ConstantEntry* entry = _findConstant(row);
        Q_ASSERT(entry != nullptr);
        QTableWidgetItem* col0 = table->item(row, 0);
        entry->setName(newName);
        col0->setText(newName);
        blockBasicSignals(false);
    }
}

void SIConstant::onTypeChanged(const QString& newType)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        blockBasicSignals(true);
        ConstantEntry* entry = _findConstant(row);
        Q_ASSERT(entry != nullptr);
        QTableWidgetItem* col1 = table->item(row, 1);
        entry->setType(newType);
        col1->setText(newType);
        blockBasicSignals(false);
    }
}

void SIConstant::onValueChanged(const QString& newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        ConstantEntry* entry = _findConstant(row);
        Q_ASSERT(entry != nullptr);
        QTableWidgetItem* col2 = table->item(row, 2);
        entry->setValue(newValue);
        col2->setText(newValue);
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
        ConstantEntry* entry = _findConstant(row);
        Q_ASSERT(entry != nullptr);
        entry->setIsDeprecated(isChecked);
        mDetails->ctrlDeprecateHint()->setEnabled(isChecked);
    }
}

void SIConstant::onDeprecateHintChanged(const QString& newText)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        ConstantEntry* entry = _findConstant(row);
        Q_ASSERT(entry != nullptr);
        entry->setDeprecateHint(newText);
    }
}

void SIConstant::onDescriptionChanged(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        ConstantEntry* entry = _findConstant(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIConstant::cellChanged(int row, int col, const QString& newValue)
{
    ConstantEntry* entry = _findConstant(row);
    Q_ASSERT(entry != nullptr);
    
    if (col == 0)
    {
        if (mDetails->ctrlName()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setName(newValue);
            mDetails->ctrlName()->setText(newValue);
            blockBasicSignals(false);            
        }
    }
    else if (col == 1)
    {
        if (mDetails->ctrlTypes()->currentText() != newValue)
        {
            blockBasicSignals(true);
            entry->setType(newValue);
            mDetails->ctrlTypes()->setCurrentText(newValue);
            blockBasicSignals(false);
        }
    }
    else if (col == 2)
    {
        if (mDetails->ctrlValue()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setValue(newValue);
            mDetails->ctrlValue()->setText(newValue);
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
        table->setRowCount(list.size());
        int row{0};
        for (const ConstantEntry & entry : list)
        {
            QTableWidgetItem* col0 = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::MediaRecord), entry.getName());
            QTableWidgetItem* col2 = new QTableWidgetItem(entry.getValue());
            QTableWidgetItem* col1 = new QTableWidgetItem(entry.getType());
            
            col0->setData(static_cast<int>(Qt::ItemDataRole::UserRole)   , entry.getId());
            table->setItem(row, 0, col0);
            table->setItem(row, 1, col1);
            table->setItem(row, 2, col2);
            
            ++row;
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
