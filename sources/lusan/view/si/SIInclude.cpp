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
#include "lusan/view/si/SICommon.hpp"
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

namespace
{
    const QString _defName("NewInclude");
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
}

SIInclude::~SIInclude(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

inline IncludeEntry* SIInclude::_findInclude(int row)
{
    QTableWidget * table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;
    
    QTableWidgetItem * item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findInclude(id);
}

inline const IncludeEntry* SIInclude::_findInclude(int row) const
{
    QTableWidget * table = mList->ctrlTableList();
    if ((row < 0) || (row >= table->rowCount()))
        return nullptr;
    
    QTableWidgetItem * item = table->item(row, 0);
    uint32_t id = item->data(static_cast<int>(Qt::ItemDataRole::UserRole)).toUInt();
    return mModel.findInclude(id);
}

QStringList SIInclude::getSupportedExtensions(void)
{
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::getInternalFileExtensions());
    return exts;
}

void SIInclude::updateWidgets(void)
{
    mTableCell = new TableCell(QList<QAbstractItemModel *>(), QList<int>(), mList->ctrlTableList());
    mList->ctrlTableList()->setItemDelegateForColumn(0, mTableCell);
}

void SIInclude::updateData(void)
{
    QTableWidget * table = mList->ctrlTableList();
    const QList<IncludeEntry> & entries = mModel.getIncludes();
    if (entries.isEmpty() == false)
    {
        table->setRowCount(entries.size());
        int row{0};
        for (const IncludeEntry & entry : entries)
        {
            QTableWidgetItem * item = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), entry.getLocation());
            item->setData(static_cast<int>(Qt::ItemDataRole::UserRole), entry.getId());
            table->setItem(row ++, 0, item);
        }
    }
}

void SIInclude::setupSignals(void)
{
    Q_ASSERT(mDetails != nullptr);
    Q_ASSERT(mList != nullptr);
    
    connect(  mDetails->ctrlInclude()
            , &QLineEdit::textChanged
            , this
            , [this](const QString& text)
                {
                    this->mList->ctrlButtonAdd()->setEnabled(text.isEmpty() == false);
                }
            );
    
    connect(mList->ctrlTableList()      , &QTableWidget::currentCellChanged, this, &SIInclude::onCurCellChanged);
    connect(mList->ctrlButtonAdd()      , &QToolButton::clicked         , this, &SIInclude::onAddClicked);
    connect(mList->ctrlButtonRemove()   , &QToolButton::clicked         , this, &SIInclude::onRemoveClicked);
    connect(mList->ctrlButtonInsert()   , &QToolButton::clicked         , this, &SIInclude::onInsertClicked);
    connect(mDetails->ctrlBrowseButton(), &QPushButton::clicked         , this, &SIInclude::onBrowseClicked);    
    connect(mDetails->ctrlInclude()     , &QLineEdit::textChanged       , this, &SIInclude::onIncludeChanged);
    connect(mDetails->ctrlDescription() , &QPlainTextEdit::textChanged  , this, &SIInclude::onDescriptionChanged);
    connect(mDetails->ctrlDeprecated()  , &QCheckBox::toggled           , this, &SIInclude::onDeprecatedChecked);
    connect(mDetails->ctrlDeprecateHint(), &QLineEdit::textChanged      , this, &SIInclude::onDeprecateHint);
    
    connect(mTableCell, &TableCell::editorDataChanged, this, &SIInclude::onEditorDataChanged);
}

void SIInclude::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    blockBasicSignals(true);

    QTableWidget* table = mList->ctrlTableList();
    if (currentRow == -1)
    {
        mDetails->ctrlInclude()->setText("");
        mDetails->ctrlDescription()->setPlainText("");
        mDetails->ctrlDeprecated()->setChecked(false);
        mDetails->ctrlDeprecateHint()->setText("");
        mDetails->ctrlBrowseButton()->setEnabled(false);
        mDetails->ctrlDeprecateHint()->setEnabled(false);

        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
    }
    else if (currentRow != previousRow)
    {
        const IncludeEntry * entry = _findInclude(currentRow);
        Q_ASSERT(entry != nullptr);
        mDetails->ctrlInclude()->setText(entry->getLocation());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecated()->setChecked(entry->getIsDeprecated());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlBrowseButton()->setEnabled(true);
        mDetails->ctrlDeprecateHint()->setEnabled(entry->getIsDeprecated());
        mList->ctrlButtonRemove()->setEnabled(true);

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
        
        QTableWidgetItem * item = table->item(currentRow, currentColumn);
        mList->ctrlButtonRemove()->setEnabled(true);
        mList->ctrlButtonMoveDown()->setEnabled(currentRow < (table->rowCount() - 1));
        mList->ctrlButtonMoveUp()->setEnabled(currentRow != 0);
    }
    else
    {
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mDetails->ctrlInclude()->setText("");
        mDetails->ctrlDescription()->setPlainText("");
        mDetails->ctrlDeprecated()->setChecked(false);
        mDetails->ctrlDeprecateHint()->setText("");
    }

    blockBasicSignals(false);
}

void SIInclude::onAddClicked(void)
{
    _addInclude(-1);    
}

void SIInclude::onRemoveClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        const IncludeEntry * entry = _findInclude(row);
        Q_ASSERT(entry != nullptr);
        mModel.deleteInclude(entry->getId());
        table->removeRow(row);
        
        if (row < table->rowCount())
        {
            table->selectRow(row);
        }
        else if (table->rowCount() > 0)
        {
            table->selectRow(table->rowCount() - 1);
        }
    }
}

void SIInclude::onInsertClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    _addInclude(row);
}

void SIInclude::onBrowseClicked(void)
{
    WorkspaceFileDialog dialog(true
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

        QString include = dialog.getSelectedFileRelativePath();
        mDetails->ctrlInclude()->setText(include);
        mDetails->ctrlDescription()->setFocus();
        mDetails->ctrlDescription()->selectAll();

        QTableWidget* table = mList->ctrlTableList();
        int row = table->currentRow();
        if (row >= 0)
        {
            QTableWidgetItem* item = table->item(row, 0);
            item->setText(include);
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
    if (row != -1)
    {
        blockBasicSignals(true);

        IncludeEntry* entry = _findInclude(row);
        Q_ASSERT(entry != nullptr);
        entry->setLocation(newText);
        QTableWidgetItem* item = table->item(row, 0);
        item->setText(newText);

        blockBasicSignals(false);
    }
}

void SIInclude::onDescriptionChanged(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        IncludeEntry* entry = _findInclude(row);
        Q_ASSERT(entry != nullptr);
        entry->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIInclude::onDeprecatedChecked(bool isChecked)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        IncludeEntry* entry = _findInclude(row);
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

void SIInclude::onDeprecateHint(const QString& newText)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        IncludeEntry* entry = _findInclude(row);
        Q_ASSERT(entry != nullptr);
        if (entry->getIsDeprecated())
        {
            entry->setDeprecateHint(newText);
        }
    }
}

void SIInclude::onEditorDataChanged(const QModelIndex &index, const QString &newValue)
{
    QTableWidget* table = mList->ctrlTableList();
    if ((index.row() < 0) || (index.row() >= table->rowCount()) || (index.column() < 0))
        return;
    
    cellChanged(index.row(), index.column(), newValue);
}

void SIInclude::_addInclude(int pos)
{
    QTableWidget *table = mList->ctrlTableList();
    QString name;
    do
    {
        name =_defName + QString::number(++ mCount);
    } while (table->findItems(name, Qt::MatchFlag::MatchExactly).isEmpty() == false);
    
    uint32_t id = mModel.createInclude(name);
    if (id != 0)
    {
        blockBasicSignals(true);

        const IncludeEntry * entry = mModel.findInclude(id);
        mDetails->ctrlInclude()->setText(entry->getName());
        mDetails->ctrlDescription()->setPlainText(entry->getDescription());
        mDetails->ctrlDeprecateHint()->setText(entry->getDeprecateHint());
        mDetails->ctrlDeprecated()->setChecked(entry->getIsDeprecated());
        
        QTableWidgetItem * current = table->currentItem();
        if (current != nullptr)
        {
            current->setSelected(false);
        }
        
        int row = pos == -1 ? table->rowCount() : pos;
        table->insertRow(row);
        QTableWidgetItem * item = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), name);
        item->setData(static_cast<int>(Qt::ItemDataRole::UserRole), entry->getId());
        table->setItem(row, 0, item);
        table->selectRow(row);
        table->scrollToItem(item);
        
        mDetails->ctrlDeprecateHint()->setEnabled(entry->getIsDeprecated());
        mDetails->ctrlInclude()->setFocus();
        mDetails->ctrlInclude()->selectAll();

        blockBasicSignals(false);
    }
}

void SIInclude::blockBasicSignals(bool doBlock)
{
    mList->ctrlTableList()->blockSignals(doBlock);
    mDetails->ctrlInclude()->blockSignals(doBlock);
}

void SIInclude::cellChanged(int row, int col, const QString& newValue)
{
    IncludeEntry* entry = _findInclude(row);
    Q_ASSERT(entry != nullptr);

    if (col == 0)
    {
        if (mDetails->ctrlInclude()->text() != newValue)
        {
            blockBasicSignals(true);
            entry->setName(newValue);
            mDetails->ctrlInclude()->setText(newValue);
            blockBasicSignals(false);
        }
    }
}
