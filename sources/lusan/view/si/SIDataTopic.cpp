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
 *  \file        lusan/view/si/SIDataTopic.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIDataTopic.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataTopic.h"

#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIDataTopicModel.hpp"
#include "lusan/view/si/SIDataTopicDetails.hpp"
#include "lusan/view/si/SIDataTopicList.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

//////////////////////////////////////////////////////////////////////////
// SIDataTopicModel class implementation
//////////////////////////////////////////////////////////////////////////

SITopicNotifyModel::SITopicNotifyModel(QObject* parent /*= nullptr*/)
    : QAbstractListModel(parent)
{
}

int SITopicNotifyModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 2;
}

QVariant SITopicNotifyModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    case Qt::ItemDataRole::DecorationRole:
    case Qt::ItemDataRole::EditRole:
        return QVariant(AttributeEntry::toString(NotificationList[index.row()]));
        break;

    case Qt::ItemDataRole::UserRole:
        return QVariant::fromValue(NotificationList[index.row()]);

    default:
        return QVariant();
    }
}


//////////////////////////////////////////////////////////////////////////
// SIDataTopicWidget class implementation
//////////////////////////////////////////////////////////////////////////

SIDataTopicWidget::SIDataTopicWidget(QWidget* parent)
    : QWidget{ parent }
    , ui     (new Ui::SIDataTopic)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

//////////////////////////////////////////////////////////////////////////
// SIDataTopic class implementation
//////////////////////////////////////////////////////////////////////////

SIDataTopic::SIDataTopic(SIDataTopicModel& model, QWidget* parent)
    : QScrollArea   (parent)
    , mModel        (model)
    , mDetails      (new SIDataTopicDetails(this))
    , mList         (new SIDataTopicList(this))
    , mWidget       (new SIDataTopicWidget(this))
    , ui            (*mWidget->ui)
    , mTypeModel    (new DataTypesModel(model.getDataTypeData()))
    , mNotifyModel  (new SITopicNotifyModel(this))
    , mTableCell    (nullptr)
    , mCount        (0)
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

SIDataTopic::~SIDataTopic(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIDataTopic::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    mTypeModel->dataTypeConverted(oldType, newType);
}

void SIDataTopic::dataTypeCreated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeCreated(dataType);
}

void SIDataTopic::dataTypeRemoved(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeRemoved(dataType);
}

void SIDataTopic::dataTypeUpdated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeUpdated(dataType);
}

void SIDataTopic::updateData(void)
{
    QTableWidget* table = mList->ctrlTable();
    const QList<AttributeEntry>& list = mModel.getAttributes();
    if (!list.isEmpty())
    {
        table->setRowCount(list.size());
        int row{ 0 };
        for (const AttributeEntry& entry : list)
        {
            QTableWidgetItem* col0 = new QTableWidgetItem(entry.getName());
            QTableWidgetItem* col1 = new QTableWidgetItem(entry.getType());
            QTableWidgetItem* col2 = new QTableWidgetItem(AttributeEntry::toString(entry.getNotification()));

            col0->setData(static_cast<int>(Qt::ItemDataRole::UserRole), entry.getId());
            table->setItem(row, 0, col0);
            table->setItem(row, 1, col1);
            table->setItem(row, 2, col2);

            ++row;
        }

        table->scrollToTop();
    }
}

void SIDataTopic::updateWidgets(void)
{
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();

    mTableCell = new TableCell(QList<QAbstractItemModel*>{mTypeModel, mNotifyModel}, QList<int>{1, 2}, mList->ctrlTable());
    mDetails->ctrlTypes()->setModel(mTypeModel);
    mList->ctrlTable()->setItemDelegate(mTableCell);

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);
    mDetails->ctrlNotification()->setEnabled(false);
    mDetails->ctrlDeprecated()->setEnabled(false);
    mDetails->ctrlDeprecateHint()->setEnabled(false);
    mDetails->ctrlDeprecateHint()->setText(QString());
}

void SIDataTopic::setupSignals(void)
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);

    connect(mList->ctrlTable()          , &QTableWidget::currentCellChanged, this, &SIDataTopic::onCurCellChanged);
    connect(mList->ctrlButtonAdd()      , &QToolButton::clicked         , this, &SIDataTopic::onAddClicked);
    connect(mList->ctrlButtonRemove()   , &QToolButton::clicked         , this, &SIDataTopic::onRemoveClicked);
    connect(mList->ctrlButtonInsert()   , &QToolButton::clicked         , this, &SIDataTopic::onInsertClicked);
    connect(mDetails->ctrlName()        , &QLineEdit::textChanged       , this, &SIDataTopic::onNameChanged);
    connect(mDetails->ctrlTypes()       , &QComboBox::currentTextChanged, this, &SIDataTopic::onTypeChanged);
    connect(mDetails->ctrlNotification(), &QComboBox::currentTextChanged, this, &SIDataTopic::onNotificationChanged);
    connect(mDetails->ctrlDeprecated()  , &QCheckBox::toggled           , this, &SIDataTopic::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint(),&QLineEdit::textEdited        , this, &SIDataTopic::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription() , &QPlainTextEdit::textChanged  , this, &SIDataTopic::onDescriptionChanged);
    connect(mTableCell                  , &TableCell::editorDataChanged , this, &SIDataTopic::onEditorDataChanged);
}

void SIDataTopic::blockBasicSignals(bool doBlock)
{
    mList->ctrlTable()->blockSignals(doBlock);
    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlTypes()->blockSignals(doBlock);
    mDetails->ctrlNotification()->blockSignals(doBlock);
}

void SIDataTopic::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    blockBasicSignals(true);
    if (currentRow == -1)
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentIndex(0);
        mDetails->ctrlNotification()->setCurrentIndex(0);
        mDetails->ctrlDescription()->setPlainText(QString());
        mDetails->ctrlDeprecateHint()->setText(QString());
        mDetails->ctrlDeprecateHint()->setEnabled(false);
        mDetails->ctrlDeprecated()->setChecked(false);
        mDetails->ctrlDeprecated()->setEnabled(false);

        mDetails->ctrlName()->setEnabled(false);
        mDetails->ctrlTypes()->setEnabled(false);
        mDetails->ctrlNotification()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
    else if (currentRow != previousRow)
    {
        const AttributeEntry* entry = _findAttribute(currentRow);
        Q_ASSERT(entry != nullptr);
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlNotification()->setCurrentText(AttributeEntry::toString(entry->getNotification()));
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());

        mDetails->ctrlDeprecated()->setEnabled(true);
        mDetails->ctrlDeprecated()->setChecked(entry->getIsDeprecated());
        if (entry->getIsDeprecated())
        {
            mDetails->ctrlDeprecateHint()->setEnabled(true);
            mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        }
        else
        {
            mDetails->ctrlDeprecateHint()->setEnabled(false);
            mDetails->ctrlDeprecateHint()->setText(QString());
        }

        if (currentRow == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        else if (currentRow == static_cast<int>(mList->ctrlTable()->rowCount() - 1))
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

void SIDataTopic::onAddClicked(void)
{
    static const QString _defName("NewAttribute");

    QTableWidget* table = mList->ctrlTable();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly).isEmpty() == false);

    uint32_t id = mModel.createAttribute(name);
    if (id != 0)
    {
        blockBasicSignals(true);

        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlNotification()->setEnabled(true);

        mDetails->ctrlTypes()->setCurrentIndex(0);
        AttributeEntry* entry = mModel.findAttribute(id);
        entry->setType(mDetails->ctrlTypes()->currentText());

        QTableWidgetItem* current = table->currentItem();
        if (current != nullptr)
        {
            current->setSelected(false);
        }

        int row = table->rowCount();
        table->setRowCount(row + 1);
        QTableWidgetItem* col0 = new QTableWidgetItem(entry->getName());
        QTableWidgetItem* col1 = new QTableWidgetItem(entry->getType());
        QTableWidgetItem* col2 = new QTableWidgetItem(AttributeEntry::toString(entry->getNotification()));
        col0->setData(static_cast<int>(Qt::ItemDataRole::UserRole), entry->getId());

        table->setItem(row, 0, col0);
        table->setItem(row, 1, col1);
        table->setItem(row, 2, col2);
        table->selectRow(row);
        table->scrollToItem(col0);

        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlTypes()->setCurrentText(entry->getType());
        mDetails->ctrlNotification()->setCurrentText(AttributeEntry::toString(entry->getNotification()));
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlDeprecated()->setChecked(entry->getIsDeprecated());
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();

        blockBasicSignals(false);
    }
}

void SIDataTopic::onRemoveClicked(void)
{
    // Implementation for removing an attribute
}

void SIDataTopic::onInsertClicked(void)
{
    // Implementation for inserting an attribute
}

void SIDataTopic::onNameChanged(const QString& newName)
{
    QTableWidget* table = mList->ctrlTable();
    int row = table->currentRow();
    if (row >= 0)
    {
        blockBasicSignals(true);
        AttributeEntry* entry = _findAttribute(row);
        Q_ASSERT(entry != nullptr);
        QTableWidgetItem* col0 = table->item(row, 0);
        entry->setName(newName);
        col0->setText(newName);
        blockBasicSignals(false);
    }
}

void SIDataTopic::onTypeChanged(const QString& newType)
{
    QTableWidget* table = mList->ctrlTable();
    int row = table->currentRow();
    if (row >= 0)
    {
        blockBasicSignals(true);
        AttributeEntry* entry = _findAttribute(row);
        Q_ASSERT(entry != nullptr);
        QTableWidgetItem* col1 = table->item(row, 1);
        entry->setType(newType);
        col1->setText(newType);
        blockBasicSignals(false);
    }
}

void SIDataTopic::onNotificationChanged(const QString& newValue)
{
    QTableWidget* table = mList->ctrlTable();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = _findAttribute(row);
        Q_ASSERT(entry != nullptr);
        QTableWidgetItem* col2 = table->item(row, 2);
        entry->setNotification(newValue);
        col2->setText(newValue);
    }
}

void SIDataTopic::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    QTableWidget* table = mList->ctrlTable();
    if ((index.row() < 0) || (index.row() >= table->rowCount()) || (index.column() < 0))
        return;

    cellChanged(index.row(), index.column(), newValue);
}

void SIDataTopic::onDeprectedChecked(bool isChecked)
{
    QTableWidget* table = mList->ctrlTable();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = _findAttribute(row);
        Q_ASSERT(entry != nullptr);
        entry->setIsDeprecated(isChecked);
        mDetails->ctrlDeprecateHint()->setEnabled(isChecked);
        if (isChecked == false)
        {
            entry->setDeprecateHint(QString());
            mDetails->ctrlDeprecateHint()->setText(QString());
        }
    }
}

void SIDataTopic::onDeprecateHintChanged(const QString& newText)
{
    QTableWidget* table = mList->ctrlTable();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = _findAttribute(row);
        Q_ASSERT(entry != nullptr);
        if (entry->getIsDeprecated())
        {
            entry->setDeprecateHint(newText);
        }
    }
}

void SIDataTopic::onDescriptionChanged(void)
{
    QTableWidget* table = mList->ctrlTable();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = _findAttribute(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIDataTopic::cellChanged(int row, int col, const QString& newValue)
{
    AttributeEntry* entry = _findAttribute(row);
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
        if (mDetails->ctrlNotification()->currentText() != newValue)
        {
            blockBasicSignals(true);
            entry->setNotification(newValue);
            mDetails->ctrlNotification()->setCurrentText(newValue);
            blockBasicSignals(false);
        }
    }
}

inline AttributeEntry* SIDataTopic::_findAttribute(int row)
{
    QTableWidget* table = mList->ctrlTable();
    if (row < 0 || row > table->rowCount())
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findAttribute(id);
}

inline const AttributeEntry* SIDataTopic::_findAttribute(int row) const
{
    QTableWidget* table = mList->ctrlTable();
    if (row < 0 || row > table->rowCount())
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findAttribute(id);
}
