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

#include <QCheckBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

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
    , mCurUrl   ( )
    , mCurFile  ( )
    , mCurFilter( )
    , mCurView  ( -1 )
{
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    
    setupSignals();
    updateData();
    
    QTableWidget * table = mList->ctrlTableList();
    table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
}

SIInclude::~SIInclude(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

QStringList SIInclude::getSupportedExtensions(void)
{
    QStringList exts{};
    exts.append(LusanApplication::getExternalFileExtensions());
    exts.append(LusanApplication::getInternalFileExtensions());
    return exts;
}

void SIInclude::updateData(void)
{
    QTableWidget * table = mList->ctrlTableList();
    const QList<IncludeEntry> & entries = mModel.entries();
    if (entries.isEmpty() == false)
    {
        table->setRowCount(entries.size());
        int row{0};
        for (const IncludeEntry & entry : entries)
        {
            table->setItem(row ++, 0, new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), entry.getLocation()));
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
    
    connect(mList->ctrlButtonAdd(), &QToolButton::clicked, this, &SIInclude::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked, this, &SIInclude::onRemoveClicked);
    connect(mList->ctrlButtonInsert(), &QToolButton::clicked, this, &SIInclude::onInsertClicked);
    connect(mList->ctrlButtonUpdate(), &QToolButton::clicked, this, &SIInclude::onUpdateClicked);
    connect(mList->ctrlTableList(), &QTableWidget::currentCellChanged, this, &SIInclude::onCurCellChanged);
    connect(mDetails->ctrlBrowseButton(), &QPushButton::clicked, this, &SIInclude::onBrowseClicked);
}

void SIInclude::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    QTableWidget* table = mList->ctrlTableList();
    if (currentRow != -1)
    {
        QTableWidgetItem * current = table->item(currentRow, currentColumn);
        const IncludeEntry * data = mModel.data(current->text());
        Q_ASSERT(data != nullptr);
        mList->ctrlButtonRemove()->setEnabled(true);
        mList->ctrlButtonMoveDown()->setEnabled(currentRow < (table->rowCount() - 1));
        mList->ctrlButtonMoveUp()->setEnabled(currentRow != 0);
        mDetails->ctrlInclude()->setText(data->getLocation());
        mDetails->ctrlDescription()->setPlainText(data->getDescription());
        mDetails->ctrlDepcrecateCheck()->setChecked(data->isDeprecated());
        mDetails->ctrlDeprecateHint()->setText(data->getDeprecationHint());
    }
    else
    {
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mDetails->ctrlInclude()->setText("");
        mDetails->ctrlDescription()->setPlainText("");
        mDetails->ctrlDepcrecateCheck()->setChecked(false);
        mDetails->ctrlDeprecateHint()->setText("");
    }
}

void SIInclude::onAddClicked(void)
{
    QString location = mDetails->ctrlInclude()->text();
    QString describe = mDetails->ctrlDescription()->toPlainText();
    bool isDeprecate = mDetails->ctrlDepcrecateCheck()->isChecked();
    QString hint     = mDetails->ctrlDeprecateHint()->text();
    if ( (location.isEmpty() == false) && mModel.addEntry(location, describe, isDeprecate, hint) )
    {
        Q_ASSERT(mModel.rowCount() != 0);
        QTableWidget * table = mList->ctrlTableList();
        QTableWidgetItem * current = table->currentItem();
        int rowCount = table->rowCount();
        table->setRowCount(rowCount + 1);
        QTableWidgetItem * newItem = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), location);
        table->setItem(rowCount, 0, newItem);
        if (current != nullptr)
        {
            current->setSelected(false);
        }
        
        newItem->setSelected(true);
    }
}

void SIInclude::onRemoveClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        QTableWidgetItem * item = table->currentItem();
        mModel.removeEntry(item->text());
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
    QString location = mDetails->ctrlInclude()->text();
    int row = table->currentRow();
    if ((row != -1) && (location.isEmpty() == false))
    {
        QTableWidgetItem* item = table->currentItem();
        QString before   = item->text();
        QString describe = mDetails->ctrlDescription()->toPlainText();
        bool isDeprecate = mDetails->ctrlDepcrecateCheck()->isChecked();
        QString hint = mDetails->ctrlDeprecateHint()->text();
        if (mModel.insertEntry(before, location, describe, isDeprecate, hint))
        {
            int rowCount = table->rowCount();
            table->insertRow(row);
            QTableWidgetItem * newItem = new QTableWidgetItem(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), location);
            table->setItem(row, 0, newItem);
            item->setSelected(false);
            newItem->setSelected(true);
        }
        else
        {
            QList<QTableWidgetItem*> found { table->findItems(location, Qt::MatchExactly) };
            Q_ASSERT(found.isEmpty() == false);
            table->selectRow(found[0]->row());
            found[0]->setSelected(true);
        }
    }
}

void SIInclude::onUpdateClicked(void)
{
    QTableWidget* table = mList->ctrlTableList();
    int row = table->currentRow();
    if (row != -1)
    {
        QString location = mDetails->ctrlInclude()->text();
        QString describe = mDetails->ctrlDescription()->toPlainText();
        bool isDeprecate = mDetails->ctrlDepcrecateCheck()->isChecked();
        QString hint = mDetails->ctrlDeprecateHint()->text();
        if (mModel.updateEntry(row, location, describe, isDeprecate, hint))
        {
            QTableWidgetItem* item = table->currentItem();
            Q_ASSERT(item != nullptr);
            item->setText(location);
        }
    }
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
        QString include = dialog.getSelectedFileRelativePath();
        mDetails->ctrlInclude()->setText(include);
        mDetails->ctrlDescription()->setFocus();
        mDetails->ctrlDescription()->selectAll();

        mCurUrl = dialog.directoryUrl().path();
        mCurFile = dialog.getSelectedFilePath();
        mCurFilter = dialog.selectedNameFilter();
        mCurView = static_cast<int>(dialog.viewMode());
    }
}
