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
 *  \file        lusan/view/si/SIAttribute.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIAttribute.hpp"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIAttributeModel.hpp"
#include "lusan/view/common/AttributeDetailsView.hpp"
#include "lusan/view/common/AttributeListView.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QToolButton>

#include "lusan/view/si/SICommon.hpp"

//////////////////////////////////////////////////////////////////////////
// SIAttributeNotifyModel class implementation
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
    , mPanels(nullptr)
{
    QVBoxLayout* root = new QVBoxLayout(this);

    QLabel* headline = new QLabel(tr("Service Interface Data Attribute Editor ..."), this);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    mPanels = new QHBoxLayout();
    root->addLayout(mPanels, 1);
}

SIAttribute::SIAttribute(SIAttributeModel& model, QWidget* parent)
    : QScrollArea   (parent)
    , mModel        (model)
    , mDetails      (new AttributeDetailsView(AttributeViewConfig{ false, true }, this))
    , mList         (new AttributeListView(AttributeViewConfig{ false, true }, this))
    , mWidget       (new SIAttributeWidget(this))
    , mTypeModel    (new DataTypesModel(model.getDataTypeData(), false))
    , mNotifyModel  (new SIAttributeNotifyModel(this))
    , mTableCell    (nullptr)
    , mCount        (0)
{
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mWidget->mPanels->addWidget(mList, 1);
    mWidget->mPanels->addWidget(mDetails, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(mWidget);

    updateWidgets();
    updateData();
    setupSignals();
    updateDetails(nullptr, true);
}

SIAttribute::~SIAttribute()
{
    mWidget->mPanels->removeWidget(mList);
    mWidget->mPanels->removeWidget(mDetails);
}

void SIAttribute::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    blockBasicSignals(true);
    mTypeModel->dataTypeConverted(oldType, newType);
    QList<uint32_t> list = mModel.replaceDataType(oldType, newType);
    if (list.isEmpty() == false)
    {
        QTreeWidget* table = mList->ctrlTableList();
        int count = table->topLevelItemCount();
        int current = table->indexOfTopLevelItem(table->currentItem());
        for (int i = 0; i < count; ++i)
        {
            AttributeEntry* entry = findAttribute(i);
            if ((entry != nullptr) && (list.contains(entry->getId())))
            {
                QTreeWidgetItem* item = table->topLevelItem(i);
                item->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase*>(newType));
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
    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    int current = table->indexOfTopLevelItem(table->currentItem());
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
    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    int current = table->indexOfTopLevelItem(table->currentItem());
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

int SIAttribute::getColumnCount() const
{
    return mList->ctrlTableList()->columnCount();
}

QString SIAttribute::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->topLevelItem(cell.row());
    return (item != nullptr ? item->text(cell.column()) : QString());
}

void SIAttribute::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == previous)
        return;

    blockBasicSignals(true);
    const int row = mList->ctrlTableList()->indexOfTopLevelItem(current);
    const AttributeEntry* entry = findAttribute(row);
    updateDetails(entry, true);
    updateToolBottons(entry != nullptr ? row : -1, mList->ctrlTableList()->topLevelItemCount());
    blockBasicSignals(false);
}

void SIAttribute::onAddClicked()
{
    QString name(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    AttributeEntry * entry = mModel.createAttribute(name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlNotification()->setEnabled(true);

        int row = table->topLevelItemCount();
        setTexts(-1, *entry);
        table->setCurrentItem(table->topLevelItem(row));
        table->scrollToBottom();
        updateDetails(entry, true);
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
        updateToolBottons(row, row + 1);
    }

    blockBasicSignals(false);
}

void SIAttribute::onRemoveClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    AttributeEntry* entry = findAttribute(row);
    AttributeEntry* nextEntry{ nullptr };
    if (entry == nullptr)
        return;

    blockBasicSignals(true);
    int count = table->topLevelItemCount();
    int nextRow = (row + 1 == count) ? row - 1 : row + 1;
    QTreeWidgetItem* next = ((nextRow >= 0) && (nextRow < count)) ? table->topLevelItem(nextRow) : nullptr;
    if (next != nullptr)
    {
        nextEntry = findAttribute(nextRow);
        table->setCurrentItem(next);
    }

    updateDetails(nextEntry, true);

    delete table->takeTopLevelItem(row);
    mModel.deleteAttribute(entry->getId());
    updateToolBottons(next != nullptr ? table->indexOfTopLevelItem(table->currentItem()) : -1, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIAttribute::onInsertClicked()
{
    QString name(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    int row = table->indexOfTopLevelItem(table->currentItem());
    row = row < 0 ? 0 : row;
    AttributeEntry* entry = mModel.insertAttribute(row, name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlNotification()->setEnabled(true);

        setTexts(row, *entry, true);
        const QList<AttributeEntry>& list = mModel.getAttributes();
        int rowCount = table->topLevelItemCount();
        Q_ASSERT(list.size() == rowCount);
        for (int i = row + 1; i < rowCount; ++i)
        {
            table->topLevelItem(i)->setData(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole, QVariant::fromValue<uint32_t>(list.at(i).getId()));
        }

        table->setCurrentItem(table->topLevelItem(row));
        updateDetails(entry, true);
        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
        updateToolBottons(row, table->topLevelItemCount());
    }

    blockBasicSignals(false);
}

void SIAttribute::onMoveUpClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row > 0)
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->topLevelItem(row)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->topLevelItem(row - 1)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapAttributes(idFirst, idSecond);
        swapAttributes(row, row - 1);
        blockBasicSignals(false);
    }
}

void SIAttribute::onMoveDownClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if ((row >= 0) && (row < (table->topLevelItemCount() - 1)))
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->topLevelItem(row)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->topLevelItem(row + 1)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapAttributes(idFirst, idSecond);
        swapAttributes(row, row + 1);
        blockBasicSignals(false);
    }
}

void SIAttribute::onNameChanged(const QString& newName)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
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
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
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
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
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
    QTreeWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->topLevelItemCount()) || (index.column() < 0))
        return;

    cellChanged(index.row(), index.column(), newValue);
}

void SIAttribute::onDeprectedChecked(bool isChecked)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row >= 0)
    {
        AttributeEntry* entry = findAttribute(row);
        Q_ASSERT(entry != nullptr);
        SICommon::checkedDeprecated<AttributeDetailsView, AttributeEntry>(mDetails, entry, isChecked);
    }
}

void SIAttribute::onDeprecateHintChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row >= 0)
    {
        AttributeEntry* entry = findAttribute(row);
        Q_ASSERT(entry != nullptr);
        SICommon::setDeprecateHint<AttributeDetailsView, AttributeEntry>(mDetails, entry, newText);
    }
}

void SIAttribute::onDescriptionChanged()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
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

    if (col == static_cast<int>(eColumn::ColName))
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
    else if (col == static_cast<int>(eColumn::ColType))
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
    else if (col == static_cast<int>(eColumn::ColNotify))
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

void SIAttribute::updateData()
{
    QTreeWidget* table = mList->ctrlTableList();
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

void SIAttribute::updateWidgets()
{
    QTreeWidget* table = mList->ctrlTableList();
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();

    mTableCell = new TableCell(QList<QAbstractItemModel*>{mTypeModel, mNotifyModel}, QList<int>{static_cast<int>(eColumn::ColType), static_cast<int>(eColumn::ColNotify)}, table, this, false);
    // Forbid invalid C++ identifier characters when editing the Name column inline, exactly
    // as the details panel's Name field does.
    mTableCell->setColumnValidation(static_cast<int>(eColumn::ColName), TableCell::eCellValidation::Identifier);
    mDetails->ctrlTypes()->setModel(mTypeModel);
    // Drive the details Notification combo from the same model as the inline column, so both
    // read/write the canonical "OnChange" / "Always" strings the entry stores.
    mDetails->ctrlNotification()->setModel(mNotifyModel);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColName)  , mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColType)  , mTableCell);
    table->setItemDelegateForColumn(static_cast<int>(eColumn::ColNotify), mTableCell);

    SICommon::enableDeprecated<AttributeDetailsView, AttributeEntry>(mDetails, nullptr, false);

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);
    mDetails->ctrlNotification()->setEnabled(false);
}

void SIAttribute::setupSignals()
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);

    connect(mList->ctrlTableList(),    &QTreeWidget::currentItemChanged, this, &SIAttribute::onCurCellChanged);
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
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || insert)
    {
        row = row < 0 ? table->topLevelItemCount() : row;
        QTreeWidgetItem* item = new QTreeWidgetItem();
        // Editable flag lets the TableCell delegate open an inline editor on double-click.
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setIcon(static_cast<int>(eColumn::ColName)  , entry.getIcon(ElementBase::eDisplay::DisplayName));
        item->setText(static_cast<int>(eColumn::ColName)  , entry.getString(ElementBase::eDisplay::DisplayName));
        item->setIcon(static_cast<int>(eColumn::ColType)  , entry.getIcon(ElementBase::eDisplay::DisplayType));
        item->setText(static_cast<int>(eColumn::ColType)  , entry.getString(ElementBase::eDisplay::DisplayType));
        item->setIcon(static_cast<int>(eColumn::ColNotify), entry.getIcon(ElementBase::eDisplay::DisplayValue));
        item->setText(static_cast<int>(eColumn::ColNotify), entry.getString(ElementBase::eDisplay::DisplayValue));
        item->setData(static_cast<int>(eColumn::ColName)  , Qt::ItemDataRole::UserRole, entry.getId());
        item->setData(static_cast<int>(eColumn::ColType)  , Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase*>(entry.getParamType()));
        table->insertTopLevelItem(row, item);
    }
    else
    {
        QTreeWidgetItem* item = table->topLevelItem(row);

        Q_ASSERT(item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt() == entry.getId());
        item->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase*>(entry.getParamType()));

        item->setIcon(static_cast<int>(eColumn::ColName)  , entry.getIcon(ElementBase::eDisplay::DisplayName));
        item->setIcon(static_cast<int>(eColumn::ColType)  , entry.getIcon(ElementBase::eDisplay::DisplayType));
        item->setIcon(static_cast<int>(eColumn::ColNotify), entry.getIcon(ElementBase::eDisplay::DisplayValue));

        item->setText(static_cast<int>(eColumn::ColName)  , entry.getString(ElementBase::eDisplay::DisplayName));
        item->setText(static_cast<int>(eColumn::ColType)  , entry.getString(ElementBase::eDisplay::DisplayType));
        item->setText(static_cast<int>(eColumn::ColNotify), entry.getString(ElementBase::eDisplay::DisplayValue));
    }
}

inline void SIAttribute::updateDetails(const AttributeEntry* entry, bool updateAll /*= false*/)
{
    QTreeWidget* table = mList->ctrlTableList();
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

        if (table->indexOfTopLevelItem(table->currentItem()) >= 0)
        {
            mList->ctrlButtonRemove()->setEnabled(true);
        }

        if (updateAll)
        {
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
            SICommon::enableDeprecated<AttributeDetailsView, AttributeEntry>(mDetails, entry, true);
        }
    }
    else
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentText("");
        mDetails->ctrlNotification()->setCurrentIndex(0);
        mDetails->ctrlDescription()->setPlainText("");

        SICommon::enableDeprecated<AttributeDetailsView, AttributeEntry>(mDetails, nullptr, false);

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
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->topLevelItemCount()))
        return nullptr;

    QTreeWidgetItem* item = table->topLevelItem(row);
    uint32_t id = item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findAttribute(id);
}

inline const AttributeEntry* SIAttribute::findAttribute(int row) const
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->topLevelItemCount()))
        return nullptr;

    QTreeWidgetItem* item = table->topLevelItem(row);
    uint32_t id = item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findAttribute(id);
}

inline void SIAttribute::swapAttributes(int firstRow, int secondRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    Q_ASSERT(firstRow >= 0 && firstRow < table->topLevelItemCount());
    Q_ASSERT(secondRow >= 0 && secondRow < table->topLevelItemCount());

    const AttributeEntry* first = findAttribute(firstRow);
    const AttributeEntry* second = findAttribute(secondRow);

    Q_ASSERT((first != nullptr) && (second != nullptr));
    setTexts(firstRow, *first);
    setTexts(secondRow, *second);
    table->setCurrentItem(table->topLevelItem(secondRow));
    updateToolBottons(secondRow, table->topLevelItemCount());
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

inline QString SIAttribute::genName()
{
    static const QString _defName("NewAttribute");

    QTreeWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly, static_cast<int>(eColumn::ColName)).isEmpty() == false);

    return name;
}
