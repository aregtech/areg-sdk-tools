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
 *  \file        lusan/view/si/SIConstant.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIConstant.hpp"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/si/SIConstantModel.hpp"
#include "lusan/view/common/ConstantDetailsView.hpp"
#include "lusan/view/common/ConstantListView.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QToolButton>

#include "lusan/view/si/SICommon.hpp"

SIConstantWidget::SIConstantWidget(QWidget* parent)
    : QWidget{ parent }
    , mPanels(nullptr)
{
    QVBoxLayout* root = new QVBoxLayout(this);

    QLabel* headline = new QLabel(tr("Service Interface Constants Editor ..."), this);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    mPanels = new QHBoxLayout();
    root->addLayout(mPanels, 1);
}

SIConstant::SIConstant(SIConstantModel& model, QWidget* parent)
    : QScrollArea(parent)
    , mModel    (model)
    , mDetails  (new ConstantDetailsView(this))
    , mList     (new ConstantListView(this))
    , mWidget   (new SIConstantWidget(this))
    , mTypeModel(new DataTypesModel(model.getDataTypeData(), false))
    , mTableCell(nullptr)
    , mCount    (0)
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

SIConstant::~SIConstant()
{
    mWidget->mPanels->removeWidget(mList);
    mWidget->mPanels->removeWidget(mDetails);
}

void SIConstant::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
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
            ConstantEntry* entry = findConstant(i);
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

void SIConstant::dataTypeCreated(DataTypeCustom* dataType)
{
    mTypeModel->dataTypeCreated(dataType);
}

void SIConstant::dataTypeDeleted(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    mTypeModel->dataTypeDeleted(dataType);
    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    int current = table->indexOfTopLevelItem(table->currentItem());
    for (int i = 0; i < count; ++ i)
    {
        ConstantEntry * entry = findConstant(i);
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

void SIConstant::dataTypeUpdated(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    Q_ASSERT(dataType != nullptr);
    mTypeModel->dataTypeUpdated(dataType);
    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    int current = table->indexOfTopLevelItem(table->currentItem());
    for (int i = 0; i < count; ++ i)
    {
        ConstantEntry * entry = findConstant(i);
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

int SIConstant::getColumnCount() const
{
    return mList->ctrlTableList()->columnCount();
}

QString SIConstant::getCellText(const QModelIndex& cell) const
{
    QTreeWidgetItem* item = mList->ctrlTableList()->topLevelItem(cell.row());
    return (item != nullptr ? item->text(cell.column()) : QString());
}

void SIConstant::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == previous)
        return;

    blockBasicSignals(true);
    const int row = mList->ctrlTableList()->indexOfTopLevelItem(current);
    const ConstantEntry * entry = findConstant(row);
    updateDetails(entry, true);
    updateToolBottons(entry != nullptr ? row : -1, mList->ctrlTableList()->topLevelItemCount());
    blockBasicSignals(false);
}

void SIConstant::onAddClicked()
{
    QString name(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    ConstantEntry * entry = mModel.createConstant(name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlValue()->setEnabled(true);

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

void SIConstant::onRemoveClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    ConstantEntry* entry = findConstant(row);
    ConstantEntry* nextEntry{ nullptr };
    if (entry == nullptr)
        return;

    blockBasicSignals(true);
    int count = table->topLevelItemCount();
    int nextRow = (row + 1 == count) ? row - 1 : row + 1;
    QTreeWidgetItem* next = ((nextRow >= 0) && (nextRow < count)) ? table->topLevelItem(nextRow) : nullptr;
    if (next != nullptr)
    {
        nextEntry = findConstant(nextRow);
        table->setCurrentItem(next);
    }

    updateDetails(nextEntry, true);

    delete table->takeTopLevelItem(row);
    mModel.deleteConstant(entry->getId());
    updateToolBottons(next != nullptr ? table->indexOfTopLevelItem(table->currentItem()) : -1, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIConstant::onInsertClicked()
{
    QString name(genName());
    QTreeWidget* table = mList->ctrlTableList();

    blockBasicSignals(true);
    int row = table->indexOfTopLevelItem(table->currentItem());
    row = row < 0 ? 0 : row;
    ConstantEntry* entry = mModel.insertConstant(row, name);
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlValue()->setEnabled(true);

        setTexts(row, *entry, true);
        const QList<ConstantEntry>& list = mModel.getConstants();
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

void SIConstant::onMoveUpClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row > 0)
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->topLevelItem(row)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->topLevelItem(row - 1)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapConstants(idFirst, idSecond);
        swapConstants(row, row - 1);
        blockBasicSignals(false);
    }
}

void SIConstant::onMoveDownClicked()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if ((row >= 0) && (row < (table->topLevelItemCount() - 1)))
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->topLevelItem(row)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->topLevelItem(row + 1)->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapConstants(idFirst, idSecond);
        swapConstants(row, row + 1);
        blockBasicSignals(false);
    }
}

void SIConstant::onNameChanged(const QString& newName)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    ConstantEntry* entry = findConstant(row);
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
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    ConstantEntry* entry = findConstant(row);
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
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    ConstantEntry* entry = findConstant(row);
    if (entry != nullptr)
    {
        entry->setValue(newValue);
        setTexts(row, *entry);
    }
}

void SIConstant::onEditorDataChanged(const QModelIndex &index, const QString &newValue)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->topLevelItemCount()) || (index.column() < 0))
        return;

    cellChanged(index.row(), index.column(), newValue);
}

void SIConstant::onDeprectedChecked(bool isChecked)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row >= 0)
    {
        ConstantEntry* entry = findConstant(row);
        Q_ASSERT(entry != nullptr);
        SICommon::checkedDeprecated<ConstantDetailsView, ConstantEntry>(mDetails, entry, isChecked);
    }
}

void SIConstant::onDeprecateHintChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row >= 0)
    {
        ConstantEntry* entry = findConstant(row);
        Q_ASSERT(entry != nullptr);
        SICommon::setDeprecateHint<ConstantDetailsView, ConstantEntry>(mDetails, entry, newText);
    }
}

void SIConstant::onDescriptionChanged()
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(table->currentItem());
    if (row >= 0)
    {
        ConstantEntry* entry = findConstant(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIConstant::cellChanged(int row, int col, const QString& newValue)
{
    ConstantEntry* entry = findConstant(row);
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

void SIConstant::updateData()
{
    QTreeWidget* table = mList->ctrlTableList();
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

void SIConstant::updateWidgets()
{
    QTreeWidget* table = mList->ctrlTableList();
    mTypeModel->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mTypeModel->updateDataTypeLists();

    mTableCell = new TableCell(QList<QAbstractItemModel*>{mTypeModel}, QList<int>{1}, table, this, false);
    // Forbid invalid C++ identifier characters when editing the Name column inline, exactly
    // as the details panel's Name field does.
    mTableCell->setColumnValidation(static_cast<int>(eColumn::ColName), TableCell::eCellValidation::Identifier);
    mDetails->ctrlTypes()->setModel(mTypeModel);
    table->setItemDelegateForColumn(0, mTableCell);
    table->setItemDelegateForColumn(1, mTableCell);
    table->setItemDelegateForColumn(2, mTableCell);

    SICommon::enableDeprecated<ConstantDetailsView, ConstantEntry>(mDetails, nullptr, false);

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlTypes()->setEnabled(false);
    mDetails->ctrlValue()->setEnabled(false);
}

void SIConstant::setupSignals()
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);

    connect(mList->ctrlTableList(),    &QTreeWidget::currentItemChanged, this, &SIConstant::onCurCellChanged);
    connect(mList->ctrlButtonAdd(),    &QToolButton::clicked        , this, &SIConstant::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked        , this, &SIConstant::onRemoveClicked);
    connect(mList->ctrlButtonInsert(), &QToolButton::clicked        , this, &SIConstant::onInsertClicked);
    connect(mList->ctrlButtonMoveUp(), &QToolButton::clicked        , this, &SIConstant::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked      , this, &SIConstant::onMoveDownClicked);

    connect(mDetails->ctrlName(),      &QLineEdit::textChanged      , this, &SIConstant::onNameChanged);
    connect(mDetails->ctrlTypes(),     &QComboBox::currentTextChanged, this,&SIConstant::onTypeChanged);
    connect(mDetails->ctrlValue(),     &QLineEdit::textChanged      , this, &SIConstant::onValueChanged);
    connect(mDetails->ctrlDeprecated(),&QCheckBox::toggled          , this, &SIConstant::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint(),&QLineEdit::textEdited    , this, &SIConstant::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription(),&QPlainTextEdit::textChanged, this, &SIConstant::onDescriptionChanged);

    connect(mTableCell                , &TableCell::signalEditorDataChanged,this, &SIConstant::onEditorDataChanged);
}

void SIConstant::blockBasicSignals(bool doBlock)
{
    mList->ctrlTableList()->blockSignals(doBlock);

    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlTypes()->blockSignals(doBlock);
    mDetails->ctrlValue()->blockSignals(doBlock);
    mDetails->ctrlDescription()->blockSignals(doBlock);
    mDetails->ctrlDeprecated()->blockSignals(doBlock);
    mDetails->ctrlDeprecateHint()->blockSignals(doBlock);
}

inline void SIConstant::setTexts(int row, const ConstantEntry & entry, bool insert /*= false*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || insert)
    {
        row = row < 0 ? table->topLevelItemCount() : row;
        QTreeWidgetItem* item = new QTreeWidgetItem();
        // Editable flag lets the TableCell delegate open an inline editor on double-click.
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setIcon(static_cast<int>(eColumn::ColName) , entry.getIcon(ElementBase::eDisplay::DisplayName));
        item->setText(static_cast<int>(eColumn::ColName) , entry.getString(ElementBase::eDisplay::DisplayName));
        item->setIcon(static_cast<int>(eColumn::ColType) , entry.getIcon(ElementBase::eDisplay::DisplayType));
        item->setText(static_cast<int>(eColumn::ColType) , entry.getString(ElementBase::eDisplay::DisplayType));
        item->setIcon(static_cast<int>(eColumn::ColValue), entry.getIcon(ElementBase::eDisplay::DisplayValue));
        item->setText(static_cast<int>(eColumn::ColValue), entry.getString(ElementBase::eDisplay::DisplayValue));
        item->setData(static_cast<int>(eColumn::ColName) , Qt::ItemDataRole::UserRole, entry.getId());
        item->setData(static_cast<int>(eColumn::ColType) , Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase *>(entry.getParamType()));
        table->insertTopLevelItem(row, item);
    }
    else
    {
        QTreeWidgetItem* item = table->topLevelItem(row);

        Q_ASSERT(item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt() == entry.getId());
        item->setData(static_cast<int>(eColumn::ColType), Qt::ItemDataRole::UserRole, QVariant::fromValue<DataTypeBase*>(entry.getParamType()));

        item->setIcon(static_cast<int>(eColumn::ColName) , entry.getIcon(ElementBase::eDisplay::DisplayName));
        item->setIcon(static_cast<int>(eColumn::ColType) , entry.getIcon(ElementBase::eDisplay::DisplayType));
        item->setIcon(static_cast<int>(eColumn::ColValue), entry.getIcon(ElementBase::eDisplay::DisplayValue));

        item->setText(static_cast<int>(eColumn::ColName) , entry.getString(ElementBase::eDisplay::DisplayName));
        item->setText(static_cast<int>(eColumn::ColType) , entry.getString(ElementBase::eDisplay::DisplayType));
        item->setText(static_cast<int>(eColumn::ColValue), entry.getString(ElementBase::eDisplay::DisplayValue));
    }
}

inline void SIConstant::updateDetails(const ConstantEntry* entry, bool updateAll /*= false*/)
{
    QTreeWidget* table = mList->ctrlTableList();
    if (entry != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlTypes()->setEnabled(true);
        mDetails->ctrlValue()->setEnabled(true);
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

        if (table->indexOfTopLevelItem(table->currentItem()) >= 0)
        {
            mList->ctrlButtonRemove()->setEnabled(true);
        }

        if (updateAll)
        {
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
            SICommon::enableDeprecated<ConstantDetailsView, ConstantEntry>(mDetails, entry, true);
        }
    }
    else
    {
        mDetails->ctrlName()->setText("");
        mDetails->ctrlTypes()->setCurrentText("");
        mDetails->ctrlValue()->setText("");
        mDetails->ctrlDescription()->setPlainText("");

        SICommon::enableDeprecated<ConstantDetailsView, ConstantEntry>(mDetails, nullptr, false);

        mDetails->ctrlName()->setEnabled(false);
        mDetails->ctrlTypes()->setEnabled(false);
        mDetails->ctrlValue()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline const ConstantEntry* SIConstant::findConstant(int row) const
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->topLevelItemCount()))
        return nullptr;

    QTreeWidgetItem* item = table->topLevelItem(row);
    uint32_t id = item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findConstant(id);
}

inline ConstantEntry* SIConstant::findConstant(int row)
{
    QTreeWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->topLevelItemCount()))
        return nullptr;

    QTreeWidgetItem* item = table->topLevelItem(row);
    uint32_t id = item->data(static_cast<int>(eColumn::ColName), Qt::ItemDataRole::UserRole).toUInt();
    return mModel.findConstant(id);
}

inline void SIConstant::swapConstants(int firstRow, int secondRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    Q_ASSERT(firstRow >= 0 && firstRow < table->topLevelItemCount());
    Q_ASSERT(secondRow >= 0 && secondRow < table->topLevelItemCount());

    const ConstantEntry* first = findConstant(firstRow);
    const ConstantEntry* second = findConstant(secondRow);

    Q_ASSERT((first != nullptr) && (second != nullptr));
    setTexts(firstRow, *first);
    setTexts(secondRow, *second);
    table->setCurrentItem(table->topLevelItem(secondRow));
    updateToolBottons(secondRow, table->topLevelItemCount());
}

inline void SIConstant::updateToolBottons(int row, int rowCount)
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

inline QString SIConstant::genName()
{
    static const QString _defName("NewConstant");

    QTreeWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly, static_cast<int>(eColumn::ColName)).isEmpty() == false);

    return name;
}
