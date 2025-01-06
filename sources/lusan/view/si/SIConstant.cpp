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

#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
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
    , mCount    (0)
{
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    
    updateDataTypes();    
    updateData();
    setupSignals();
}

SIConstant::~SIConstant(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIConstant::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == -1)
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentText("");
        mDetails->ctrlValue()->setText("");
        mDetails->ctrlDescription()->setPlainText("");
        mDetails->ctrlDeprecateHint()->setText("");
        mDetails->ctrlDepricated()->setChecked(false);
        
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
    else if (currentRow != previousRow)
    {
        QTableWidget* table = mList->ctrlTableList();
        QTableWidgetItem* name = table->item(currentRow, 0);
        QTableWidgetItem* type = table->item(currentRow, 1);
        QTableWidgetItem* value= table->item(currentRow, 2);
        
        const ConstantEntry* entry = mModel.findConstant(name->text());
        Q_ASSERT(entry != nullptr);
        mDetails->ctrlName()->setText(name->text());
        mDetails->ctrlTypes()->setCurrentText(type->text());
        mDetails->ctrlValue()->setText(value->text());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlDepricated()->setChecked(entry->isDeprecated());
        
        if (currentRow == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        else if (currentRow == static_cast<int>(table->rowCount() - 1))
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
    
    ConstantEntry elem(0, name);
    if (mModel.addContant(std::move(elem), true))
    {
        const ConstantEntry * entry = mModel.findConstant(name);
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlValue()->setText(entry->getValue());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlDepricated()->setChecked(entry->isDeprecated());
        
        int row = table->rowCount();
        table->insertRow(row);
        QTableWidgetItem* item = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::MediaRecord), entry->getName());
        item->setData(static_cast<int>(Qt::ItemDataRole::UserRole), QVariant::fromValue(*entry));
        table->setItem(row, 0, item);
        table->setItem(row, 1, new QTableWidgetItem(entry->getType()));
        table->setItem(row, 2, new QTableWidgetItem(entry->getValue()));
        table->selectRow(row);
        table->scrollToItem(item);
        
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
}

void SIConstant::onRemoveClicked(void)
{
}

void SIConstant::onInsertClicked(void)
{
}

void SIConstant::onUpdateClicked(void)
{
}

void SIConstant::onNameChanged(const QString& newName)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    QTableWidgetItem* item = row >= 0 ? table->item(row, 0) : nullptr;
    if ((item != nullptr) && mModel.updateConstantName(item->text(), newName))
    {
        item->setText(newName);
    }
}

void SIConstant::onTypeChanged(const QString& newType)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        QTableWidgetItem* item = table->item(row, 0);
        QTableWidgetItem* type = table->item(row, 1);
        type->setText(newType);
        mModel.updateConstantType(item->text(), newType);
    }
}

void SIConstant::onTypesOpened(void)
{
    updateDataTypes();
}

void SIConstant::onValueChanged(const QString& newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        QTableWidgetItem*  item = table->item(row, 0);
        QTableWidgetItem* value = table->item(row, 2);
        value->setText(newValue);
        mModel.updateConstantValue(item->text(), newValue);
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
            QTableWidgetItem * item = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::MediaRecord), entry.getName());
            item->setData(static_cast<int>(Qt::ItemDataRole::UserRole), QVariant::fromValue(entry));
            table->setItem(row, 0, item);
            table->setItem(row, 1, new QTableWidgetItem(entry.getType()));
            table->setItem(row, 2, new QTableWidgetItem(entry.getValue()));
            ++row;
        }
        
        table->scrollToTop();
    }
}

void SIConstant::updateDataTypes(void)
{
    mDetails->ctrlTypes()->setModel(mTypeModel);
}

void SIConstant::setupSignals(void)
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);
    
    connect(mList->ctrlTableList(),    &QTableWidget::currentCellChanged, this, &SIConstant::onCurCellChanged);
    connect(mList->ctrlButtonAdd(),    &QToolButton::clicked, this, &SIConstant::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked, this, &SIConstant::onRemoveClicked);
    connect(mList->ctrlButtonInsert(), &QToolButton::clicked, this, &SIConstant::onInsertClicked);
    connect(mList->ctrlButtonUpdate(), &QToolButton::clicked, this, &SIConstant::onUpdateClicked);
    connect(mDetails->ctrlName(),      &QLineEdit::textChanged, this, &SIConstant::onNameChanged);
    connect(mDetails->ctrlTypes(),     &QComboBox::editTextChanged, this, &SIConstant::onTypeChanged);
    connect(mDetails->ctrlValue(),     &QLineEdit::textChanged, this, SIConstant::onValueChanged);
}
