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
 *  \file        lusan/view/si/SIInclude.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIInclude.hpp"
#include "ui/ui_SIInclude.h"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/model/si/SIIncludeModel.hpp"
#include "lusan/view/common/WorkspaceFileDialog.hpp"
#include "lusan/view/si/SIIncludeDetails.hpp"
#include "lusan/view/si/SIIncludeList.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QCheckBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

#include "lusan/view/si/SICommon.hpp"

namespace
{
    const QString _defName("NewInclude");
}

QStringList SIInclude::getSupportedExtensions(void)
{
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::getInternalFileExtensions());
    return exts;
}

SIIncludeWidget::SIIncludeWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIInclude)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIInclude::SIInclude(SIIncludeModel & model, QWidget* parent)
    : QScrollArea(parent)
    , mModel    (model)
    , mDetails  (new SIIncludeDetails(this))
    , mList     (new SIIncludeList(model, this))
    , mWidget   (new SIIncludeWidget(this))
    , ui        (*mWidget->ui)
    , mTableCell(nullptr)
    , mCurUrl   ( )
    , mCurFile  ( )
    , mCurFilter( )
    , mCurView  ( -1 )
    , mCount    ( 0 )
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

SIInclude::~SIInclude(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIInclude::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == previousRow)
        return;

    blockBasicSignals(true);
    QTableWidget* table = mList->ctrlTableList();
    QTableWidgetItem* col0 = currentRow >= 0 ? table->item(currentRow, 0) : nullptr;
    const IncludeEntry* entry = findInclude(currentRow);
    updateDetails(entry, true);
    updateToolBottons(entry != nullptr ? currentRow : -1, table->rowCount());
    blockBasicSignals(false);
}

void SIInclude::onAddClicked(void)
{
    static const QString _defName("NewInclude");

    QTableWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly).isEmpty() == false);

    blockBasicSignals(true);
    IncludeEntry* entry = mModel.createInclude(name);
    if (entry != nullptr)
    {
        mDetails->ctrlInclude()->setEnabled(true);
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
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();
        updateToolBottons(row, row + 1);
    }

    blockBasicSignals(false);
}

void SIInclude::onRemoveClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    IncludeEntry* entry = findInclude(row);
    IncludeEntry* nextEntry{ nullptr };
    if (entry == nullptr)
        return;

    blockBasicSignals(true);
    int nextRow = row + 1 == table->rowCount() ? row - 1 : row + 1;
    QTableWidgetItem* next = (nextRow >= 0) && (nextRow < table->rowCount()) ? table->item(nextRow, 0) : nullptr;
    if (next != nullptr)
    {
        nextEntry = findInclude(nextRow);
        table->setCurrentItem(next);
        next->setSelected(true);
    }

    QTableWidgetItem* col0 = table->item(row, 0);
    col0->setSelected(false);

    updateDetails(nextEntry, true);

    delete col0;
    table->removeRow(row);
    mModel.deleteInclude(entry->getId());
    updateToolBottons(next != nullptr ? table->indexFromItem(next).row() : -1, mList->ctrlTableList()->rowCount());
    blockBasicSignals(false);
}

void SIInclude::onInsertClicked(void)
{
}

void SIInclude::onMoveUpClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row > 0)
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->item(row, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->item(row - 1, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapIncludes(idFirst, idSecond);
        swapIncludes(row, row - 1);
        blockBasicSignals(false);
    }
}

void SIInclude::onMoveDownClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if ((row >= 0) && (row < (table->rowCount() - 1)))
    {
        blockBasicSignals(true);
        uint32_t idFirst = table->item(row, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        uint32_t idSecond = table->item(row + 1, 0)->data(Qt::ItemDataRole::UserRole).toUInt();
        mModel.swapIncludes(idFirst, idSecond);
        swapIncludes(row, row + 1);
        blockBasicSignals(false);
    }
}

void SIInclude::onBrowseClicked(void)
{
    WorkspaceFileDialog dialog(   true
                                , false
                                , LusanApplication::getWorkspaceDirectories()
                                , SIInclude::getSupportedExtensions()
                                , tr("Select Include File")
                                , this);

    if (mCurUrl.isEmpty() == false)
    {
        dialog.setDirectoryUrl(QUrl::fromLocalFile(mCurUrl));
        dialog.setDirectory(mCurUrl);
    }

    if (mCurFile.isEmpty() == false)
    {
        QFileInfo info(mCurFile);
        dialog.setDirectory(info.absoluteDir());
        dialog.selectFile(mCurFile);
    }

    if (mCurFilter.isEmpty() == false)
    {
        dialog.setNameFilter(mCurFilter);
    }

    if (mCurView != -1)
    {
        dialog.setViewMode(static_cast<QFileDialog::ViewMode>(mCurView));
    }

    dialog.clearHistory();
    if (dialog.exec() == static_cast<int>(QDialog::DialogCode::Accepted))
    {
        blockBasicSignals(true);

        QString location = dialog.getSelectedFileRelativePath();
        mDetails->ctrlInclude()->setText(location);
        mDetails->ctrlDescription()->setFocus();
        mDetails->ctrlDescription()->selectAll();

        QTableWidget* table = mList->ctrlTableList();
        int row = table->currentRow();
        if (row >= 0)
        {
            IncludeEntry* entry = findInclude(row);
            entry->setLocation(location);
            setTexts(row, *entry);
        }
        
        mCurUrl = dialog.directoryUrl().path();
        mCurFile = dialog.getSelectedFilePath();
        mCurFilter = dialog.selectedNameFilter();
        mCurView = static_cast<int>(dialog.viewMode());
        
        blockBasicSignals(false);
    }
}

void SIInclude::onIncludeChanged(const QString& newText)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    IncludeEntry* entry = findInclude(row);
    if (entry != nullptr)
    {
        blockBasicSignals(true);
        entry->setLocation(newText);
        setTexts(row, *entry);
        blockBasicSignals(false);
    }
}

void SIInclude::onDescriptionChanged(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        IncludeEntry* entry = findInclude(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIInclude::onDeprecatedChecked(bool isChecked)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row >= 0)
    {
        IncludeEntry* entry = findInclude(row);
        Q_ASSERT(entry != nullptr);
        SICommon::checkedDeprecated<SIIncludeDetails, IncludeEntry>(mDetails, entry, isChecked);
    }
}

void SIInclude::onDeprecateHint(const QString& newText)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        IncludeEntry* entry = findInclude(row);
        Q_ASSERT(entry != nullptr);
        SICommon::setDeprecateHint<SIIncludeDetails, IncludeEntry>(mDetails, entry, newText);
    }
}

void SIInclude::onEditorDataChanged(const QModelIndex &index, const QString &newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->rowCount()) || (index.column() < 0))
        return;
    
    cellChanged(index.row(), index.column(), newValue);
}

void SIInclude::cellChanged(int row, int col, const QString& newValue)
{
    IncludeEntry* entry = findInclude(row);
    Q_ASSERT(entry != nullptr);

    if (col == 0)
    {
        if (mDetails->ctrlInclude()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setLocation(newValue);
            setTexts(row, *entry);
            updateDetails(entry, false);
            blockBasicSignals(false);
        }
    }
}

void SIInclude::updateData(void)
{
    QTableWidget* table = mList->ctrlTableList();
    const QList<IncludeEntry>& list = mModel.getIncludes();
    if (list.isEmpty() == false)
    {
        for (const IncludeEntry& entry : list)
        {
            setTexts(-1, entry);
        }

        table->scrollToTop();
    }
}

void SIInclude::updateWidgets(void)
{
    mTableCell = new TableCell(QList<QAbstractItemModel*>(), QList<int>(), mList->ctrlTableList());
    mList->ctrlTableList()->setItemDelegateForColumn(0, mTableCell);
    SICommon::enableDeprecated<SIIncludeDetails, IncludeEntry>(mDetails, nullptr, false);

    mDetails->ctrlInclude()->setEnabled(false);
    SICommon::enableDeprecated<SIIncludeDetails, IncludeEntry>(mDetails, nullptr, false);
}

void SIInclude::setupSignals(void)
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);

    connect(mList->ctrlTableList(), &QTableWidget::currentCellChanged, this, &SIInclude::onCurCellChanged);
    connect(mList->ctrlButtonAdd(), &QToolButton::clicked, this, &SIInclude::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked, this, &SIInclude::onRemoveClicked);
    connect(mList->ctrlButtonInsert(), &QToolButton::clicked, this, &SIInclude::onInsertClicked);
    connect(mList->ctrlButtonMoveUp(), &QToolButton::clicked, this, &SIInclude::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked, this, &SIInclude::onMoveDownClicked);

    connect(mDetails->ctrlInclude(), &QLineEdit::textChanged, this, &SIInclude::onIncludeChanged);
    connect(mDetails->ctrlBrowseButton(), &QPushButton::clicked, this, &SIInclude::onBrowseClicked);
    connect(mDetails->ctrlDeprecated(), &QCheckBox::toggled, this, &SIInclude::onDeprecatedChecked);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::textChanged, this, &SIInclude::onDeprecateHint);
    connect(mDetails->ctrlDescription(), &QPlainTextEdit::textChanged, this, &SIInclude::onDescriptionChanged);

    connect(mTableCell, &TableCell::editorDataChanged, this, &SIInclude::onEditorDataChanged);
}

void SIInclude::blockBasicSignals(bool doBlock)
{
    mList->ctrlTableList()->blockSignals(doBlock);

    mDetails->ctrlInclude()->blockSignals(doBlock);
    mDetails->ctrlDescription()->blockSignals(doBlock);
    mDetails->ctrlDeprecated()->blockSignals(doBlock);
    mDetails->ctrlDeprecateHint()->blockSignals(doBlock);
}

inline void SIInclude::setTexts(int row, const IncludeEntry& entry)
{
    QTableWidget* table = mList->ctrlTableList();
    if (row < 0)
    {
        row = table->rowCount();
        QTableWidgetItem* col0 = new QTableWidgetItem(entry.getIcon(ElementBase::eDisplay::DisplayName), entry.getString(ElementBase::eDisplay::DisplayName));
        col0->setData(Qt::ItemDataRole::UserRole, entry.getId());
        table->setRowCount(row + 1);
        table->setItem(row, 0, col0);
    }
    else
    {
        QTableWidgetItem* col0 = table->item(row, 0);
        Q_ASSERT(col0->data(Qt::ItemDataRole::UserRole).toUInt() == entry.getId());

        col0->setIcon(entry.getIcon(ElementBase::eDisplay::DisplayName));
        col0->setText(entry.getString(ElementBase::eDisplay::DisplayName));
    }
}

inline void SIInclude::updateDetails(const IncludeEntry* entry, bool updateAll /*= false*/)
{
    if (entry != nullptr)
    {
        mDetails->ctrlInclude()->setText(entry->getName());
        if (mList->ctrlTableList()->currentRow() >= 0)
        {
            mDetails->ctrlBrowseButton()->setEnabled(true);
            mList->ctrlButtonRemove()->setEnabled(true);
        }

        if (updateAll)
        {
            mDetails->ctrlDescription()->setPlainText(entry->getDescription());
            SICommon::enableDeprecated<SIIncludeDetails, IncludeEntry>(mDetails, entry, true);
        }
    }
    else
    {
        mDetails->ctrlInclude()->setText("");
        mDetails->ctrlDescription()->setPlainText("");

        SICommon::enableDeprecated<SIIncludeDetails, IncludeEntry>(mDetails, nullptr, false);

        mDetails->ctrlInclude()->setEnabled(false);
        mDetails->ctrlBrowseButton()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
}

inline IncludeEntry* SIInclude::findInclude(int row)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findInclude(id);
}

inline const IncludeEntry* SIInclude::findInclude(int row) const
{
    QTableWidget* table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;

    QTableWidgetItem* item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findInclude(id);
}

inline void SIInclude::swapIncludes(int firstRow, int secondRow)
{
    QTableWidget* table = mList->ctrlTableList();
    Q_ASSERT(firstRow >= 0 && firstRow < table->rowCount());
    Q_ASSERT(secondRow >= 0 && secondRow < table->rowCount());

    const IncludeEntry* first = findInclude(firstRow);
    const IncludeEntry* second = findInclude(secondRow);

    Q_ASSERT((first != nullptr) && (second != nullptr));
    setTexts(firstRow, *first);
    setTexts(secondRow, *second);
    table->item(firstRow, 0)->setSelected(false);
    table->setCurrentItem(table->item(secondRow, 0));
    table->selectRow(secondRow);
    updateToolBottons(secondRow, mList->ctrlTableList()->rowCount());
}

inline void SIInclude::updateToolBottons(int row, int rowCount)
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
