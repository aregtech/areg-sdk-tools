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
 *  \file        lusan/view/si/SIAttribute.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIAttribute.hpp"
#include "ui/ui_SIAttribute.h"

#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIAttributeModel.hpp"
#include "lusan/view/si/SIAttributeDetails.hpp"
#include "lusan/view/si/SIAttributeList.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

#include "lusan/view/si/SICommon.hpp"

//////////////////////////////////////////////////////////////////////////
// SIAttributeModel class implementation
//////////////////////////////////////////////////////////////////////////

SIAttributeNotifyModel::SIAttributeNotifyModel(QObject* parent /*= nullptr*/)
    : QAbstractListModel(parent)
{
}

int SIAttributeNotifyModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 2;
}

QVariant SIAttributeNotifyModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
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
// SIAttributeWidget class implementation
//////////////////////////////////////////////////////////////////////////

SIAttributeWidget::SIAttributeWidget(QWidget* parent)
    : QWidget{ parent }
    , ui     (new Ui::SIAttribute)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIAttribute::SIAttribute(SIAttributeModel& model, QWidget* parent)
    : QScrollArea   (parent)
    , mModel        (model)
    , mDetails      (new SIAttributeDetails(this))
    , mList         (new SIAttributeList(this))
    , mWidget       (new SIAttributeWidget(this))
    , ui            (*mWidget->ui)
    , mTypeModel    (new DataTypesModel(model.getDataTypeData(), false))
    , mNotifyModel  (new SIAttributeNotifyModel(this))
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
    updateDetails(nullptr, true);
}

SIAttribute::~SIAttribute(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIAttribute::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    blockBasicSignals(true);
    mTypeModel->dataTypeConverted(oldType, newType);
    QList<uint32_t> list = mModel.replaceDataType(oldType, newType);
    if (list.isEmpty() == false)
    {
        QTableWidget* table = mList->ctrlTableList();
        int count = table->rowCount();
        int current = table->currentRow();
        for (int i = 0; i < count; ++i)
        {
            AttributeEntry* entry = findAttribute(i);
            if ((entry != nullptr) && (list.contains(entry->getId())))
            {
                QTableWidgetItem* col1 = table->item(i, static_cast<int>(eColumn::ColType));
                col1->setData(Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase*>(newType));
                if (i == current)
                {
                    updateDetails(entry, false);
                }
            }
        }
    }

    blockBasicSignals(false);
}

void SIAttribute::dataTypeCreated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeCreated(dataType);
}

void SIAttribute::dataTypeDeleted(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    mTypeModel->dataTypeDeleted(dataType);
    QTableWidget* table = mList->ctrlTableList();
    int count = table->rowCount();
    int current = table->currentRow();
    for (int i = 0; i < count; ++i)
    {
        AttributeEntry* entry = findAttribute(i);
        if ((entry != nullptr) && entry->getParamType() == static_cast<DataTypeBase*>(dataType))
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

void SIAttribute::dataTypeUpdated(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    Q_ASSERT(dataType != nullptr);
    mTypeModel->dataTypeUpdated(dataType);
    QTableWidget* table = mList->ctrlTableList();
    int count = table->rowCount();
    int current = table->currentRow();
    for (int i = 0; i < count; ++i)
    {
        AttributeEntry* entry = findAttribute(i);
        if ((entry != nullptr) && entry->getParamType() == static_cast<DataTypeBase*>(dataType))
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

int SIAttribute::getColumnCount(void) const
{
    return mList->ctrlTableList()->columnCount();
}

QString SIAttribute::getCellText(const QModelIndex& cell) const
{
    QTableWidgetItem* item = mList->ctrlTableList()->item(cell.row(), cell.column());
    return (item != nullptr ? item->text() : QString());
}

void SIAttribute::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == previousRow)
        return;

    blockBasicSignals(true);
    const AttributeEntry* entry = findAttribute(currentRow);
    updateDetails(entry, true);
    updateToolBottons(entry != nullptr ? currentRow : -1, mList->ctrlTableList()->rowCount());
    blockBasicSignals(false);
}

void SIAttribute::onAddClicked(void)
{
    QString name(genName());
    QTableWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    AttributeEntry * entry = mModel.createAttribute(name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlNotification()->setEnabled(true);

        QTableWidgetItem* current = table->currentItem();
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
        updateToolBottons(row, row + 1);
    }

    blockBasicSignals(false);
}

void SIAttribute::onRemoveClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    AttributeEntry* entry = findAttribute(row);
    AttributeEntry* nextEntry{ nullptr };
    if (entry == nullptr)
        return;

    blockBasicSignals(true);
    int nextRow = row + 1 == table->rowCount() ? row - 1 : row + 1;
    QTableWidgetItem* next = (nextRow >= 0) && (nextRow < table->rowCount()) ? table->item(nextRow, static_cast<int>(eColumn::ColName)) : nullptr;
    if (next != nullptr)
    {
        nextEntry = findAttribute(nextRow);
        table->setCurrentItem(next);
        next->setSelected(true);
    }

    QTableWidgetItem* col0 = table->item(row, static_cast<int>(eColumn::ColName));
    QTableWidgetItem* col1 = table->item(row, static_cast<int>(eColumn::ColType));
    QTableWidgetItem* col2 = table->item(row, static_cast<int>(eColumn::ColNotify));
    col0->setSelected(false);
    col1->setSelected(false);
    col2->setSelected(false);

    updateDetails(nextEntry, true);

    delete col0;
    delete col1;
    delete col2;
    table->removeRow(row);
    mModel.deleteAttribute(entry->getId());
    updateToolBottons(next != nullptr ? table->indexFromItem(next).row() : -1, mList->ctrlTableList()->rowCount());
    blockBasicSignals(false);
}

void SIAttribute::onInsertClicked(void)
{
    QString name(genName());
    QTableWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    int row = table->currentRow();
    row = row < 0 ? 0 : row;
    AttributeEntry* entry = mModel.insertAttribute(row, name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlNotification()->setEnabled(true);

        QTableWidgetItem* current = table->currentItem();
        if (current != nullptr)
        {
            current->setSelected(false);
        }

        setTexts(row, *entry, true);
        const QList<AttributeEntry>& list = mModel.getAttributes();
        int rowCount = table->rowCount();
        Q_ASSERT(list.size() == rowCount);
        for (int i = row + 1; i < rowCount; ++i)
        {
            QTableWidgetItem* col0 = table->item(i, static_cast<int>(eColumn::ColName));
            col0->setData(Qt::ItemDataRole::UserRole, QVariant::fromValue<uint32_t>(list.at(i).getId()));
        }

        table->selectRow(row);
        table->showRow(row);
        updateDetails(entry, true);
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
        updateToolBottons(row, table->rowCount());
    }

    blockBasicSignals(false);
}

void SIAttribute::onMoveUpClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row > 0)
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->item(row, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->item(row - 1, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapAttributes(idFirst, idSecond);
        swapAttributes(row, row - 1);
        blockBasicSignals(false);
    }
}

void SIAttribute::onMoveDownClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if ((row >= 0) && (row < (table->rowCount() - 1)))
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->item(row, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->item(row + 1, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapAttributes(idFirst, idSecond);
        swapAttributes(row, row + 1);
        blockBasicSignals(false);
    }
}

void SIAttribute::onNameChanged(const QString& newName)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    AttributeEntry* entry = findAttribute(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);
        entry->setName(newName);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIAttribute::onTypeChanged(const QString& newType)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    AttributeEntry* entry = findAttribute(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);

        DataTypeBase* dataType = mTypeModel->findDataType(newType);
        entry->setParamType(dataType);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIAttribute::onNotificationChanged(const QString& newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    AttributeEntry* entry = findAttribute(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);
        entry->setNotification(newValue);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIAttribute::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->rowCount()) || (index.column() < 0))
        return;

    cellChanged(index.row(), index.column(), newValue);
}

void SIAttribute::onDeprectedChecked(bool isChecked)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = findAttribute(row);
        Q_ASSERT(entry != nullptr);
        SICommon::checkedDeprecated<SIAttributeDetails, AttributeEntry>(mDetails, entry, isChecked);
    }
}

void SIAttribute::onDeprecateHintChanged(const QString& newText)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = findAttribute(row);
        Q_ASSERT(entry != nullptr);
        SICommon::setDeprecateHint<SIAttributeDetails, AttributeEntry>(mDetails, entry, newText);
    }
}

void SIAttribute::onDescriptionChanged(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        AttributeEntry* entry = findAttribute(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIAttribute::cellChanged(int row, int col, const QString& newValue)
{
    AttributeEntry* entry = findAttribute(row);
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
        if (mDetails->ctrlNotification()->currentText() != newValue)
        {
            blockBasicSignals(true);
            entry->setNotification(newValue);
            setTexts(row, *entry);
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
}

void SIAttribute::updateData(void)
{
    QTableWidget* table = mList->ctrlTableList();
    const QList<AttributeEntry>& list = mModel.getAttributes();
    if (list.isEmpty() == false)
    {
        for (const AttributeEntry& entry : list)
        {
            setTexts(-1, entry);
        }

        table->scrollToTop();
    }
}

void SIAttribute::updateWidgets(void)
{
    QTableWidget* table = mList->ctrlTableList();
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();

    mTableCell = new TableCell(QList<QAbstractItemModel*>{mTypeModel, mNotifyModel}, QList<int>{1, 2}, mList->ctrlTableList(), this, false);
    mDetails->ctrlTypes()->setModel(mTypeModel);
    table->setItemDelegateForColumn(0, mTableCell);
    table->setItemDelegateForColumn(1, mTableCell);
    table->setItemDelegateForColumn(2, mTableCell);

    SICommon::enableDeprecated<SIAttributeDetails, AttributeEntry>(mDetails, nullptr, false);

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);
    mDetails->ctrlNotification()->setEnabled(false);
}

void SIAttribute::setupSignals(void)
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);

    connect(mList->ctrlTableList(),    &QTableWidget::currentCellChanged, this, &SIAttribute::onCurCellChanged);
    connect(mList->ctrlButtonAdd(),    &QToolButton::clicked, this, &SIAttribute::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked, this, &SIAttribute::onRemoveClicked);
    connect(mList->ctrlButtonInsert(), &QToolButton::clicked, this, &SIAttribute::onInsertClicked);
    connect(mList->ctrlButtonMoveUp(), &QToolButton::clicked, this, &SIAttribute::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked, this, &SIAttribute::onMoveDownClicked);

    connect(mDetails->ctrlName(), &QLineEdit::textChanged, this, &SIAttribute::onNameChanged);
    connect(mDetails->ctrlTypes(), &QComboBox::currentTextChanged, this, &SIAttribute::onTypeChanged);
    connect(mDetails->ctrlNotification(), &QComboBox::currentTextChanged, this, &SIAttribute::onNotificationChanged);
    connect(mDetails->ctrlDeprecated(), &QCheckBox::toggled, this, &SIAttribute::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::textEdited, this, &SIAttribute::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription(), &QPlainTextEdit::textChanged, this, &SIAttribute::onDescriptionChanged);

    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &SIAttribute::onEditorDataChanged);
}

void SIAttribute::blockBasicSignals(bool doBlock)
{
    mList->ctrlTableList()->blockSignals(doBlock);

    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlTypes()->blockSignals(doBlock);
    mDetails->ctrlNotification()->blockSignals(doBlock);
    mDetails->ctrlDescription()->blockSignals(doBlock);
    mDetails->ctrlDeprecated()->blockSignals(doBlock);
    mDetails->ctrlDeprecateHint()->blockSignals(doBlock);
}

inline void SIAttribute::setTexts(int row, const AttributeEntry& entry, bool insert /*= false*/)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((row < 0) || insert)
    {
        row = row < 0 ? table->rowCount() : row;
        table->insertRow(row);
        QTableWidgetItem * col0 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayName), entry.getString(ElementBase::eDisplay::DisplayName));
        QTableWidgetItem * col1 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayType), entry.getString(ElementBase::eDisplay::DisplayType));
        QTableWidgetItem * col2 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayValue), entry.getString(ElementBase::eDisplay::DisplayValue));
        col0->setData(Qt::ItemDataRole::UserRole, entry.getId());
        col1->setData(Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase *>(entry.getParamType()));
        table->setItem(row, static_cast<int>(eColumn::ColName)  , col0);
        table->setItem(row, static_cast<int>(eColumn::ColType)  , col1);
        table->setItem(row, static_cast<int>(eColumn::ColNotify), col2);
    }
    else
    {
        QTableWidgetItem * col0 = table->item(row, static_cast<int>(eColumn::ColName));
        QTableWidgetItem * col1 = table->item(row, static_cast<int>(eColumn::ColType));
        QTableWidgetItem * col2 = table->item(row, static_cast<int>(eColumn::ColNotify));
        
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

inline void SIAttribute::updateDetails(const AttributeEntry* entry, bool updateAll /*= false*/)
{
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlNotification()->setEnabled(true);
        mDetails->ctrlName()->setText(entry->getName());
        mDetails->ctrlNotification()->setCurrentText(AttributeEntry::toString(entry->getNotification()));
        if (entry->isValid())
        {
            mDetails->ctrlTypes()->setCurrentText(entry->getType());
        }
        else
        {
            mDetails->ctrlTypes()->setCurrentIndex(0);
        }

        if (mList->ctrlTableList()->currentRow() >= 0)
        {
            mList->ctrlButtonRemove()->setEnabled(true);
        }

        if (updateAll)
        {
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
            SICommon::enableDeprecated<SIAttributeDetails, AttributeEntry>(mDetails, entry, true);
        }
    }
    else
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentText("");
        mDetails->ctrlNotification()->setCurrentIndex(0);
        mDetails->ctrlDescription()->setPlainText("");

        SICommon::enableDeprecated<SIAttributeDetails, AttributeEntry>(mDetails, nullptr, false);

        mDetails->ctrlName()->setEnabled(false);
        mDetails->ctrlTypes()->setEnabled(false);
        mDetails->ctrlNotification()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline AttributeEntry* SIAttribute::findAttribute(int row)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findAttribute(id);
}

inline const AttributeEntry* SIAttribute::findAttribute(int row) const
{
    QTableWidget* table = mList->ctrlTableList();
    if (row < 0 || row > table->rowCount())
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findAttribute(id);
}

inline void SIAttribute::swapAttributes(int firstRow, int secondRow)
{
    QTableWidget* table = mList->ctrlTableList();
    Q_ASSERT(firstRow >= 0 && firstRow < table->rowCount());
    Q_ASSERT(secondRow >= 0 && secondRow < table->rowCount());

    const AttributeEntry* first = findAttribute(firstRow);
    const AttributeEntry* second = findAttribute(secondRow);

    Q_ASSERT((first != nullptr) && (second != nullptr));
    setTexts(firstRow, *first);
    setTexts(secondRow, *second);
    table->item(firstRow, 0)->setSelected(false);
    table->setCurrentItem(table->item(secondRow, 0));
    table->selectRow(secondRow);
    updateToolBottons(secondRow, mList->ctrlTableList()->rowCount());
}

inline void SIAttribute::updateToolBottons(int row, int rowCount)
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

        mList->ctrlButtonRemove()->setEnabled(true);
    }
    else
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline QString SIAttribute::genName(void)
{
    static const QString _defName("NewAttribute");

    QTableWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly).isEmpty() == false);
    
    return name;
}
