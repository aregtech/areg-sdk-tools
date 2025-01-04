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
    : QScrollArea   (parent)
    , mModel        (model)
    , mPageDetails  (new SIIncludeDetails(this))
    , mPageList     (new SIIncludeList(model, this))
    , mWidget       (new SIIncludeWidget(this))
    , ui            (*mWidget->ui)
    , mCurUrl       ( )
    , mCurFile      ( )
    , mCurFilter    ( )
    , mCurView      ( -1 )
{
    ui.horizontalLayout->addWidget(mPageList);
    ui.horizontalLayout->addWidget(mPageDetails);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    
    setupSignals();
    updateData();
    
    QTableWidget * table = mPageList->ctrlTableIncludes();
    table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
}

SIInclude::~SIInclude(void)
{
    ui.horizontalLayout->removeWidget(mPageList);
    ui.horizontalLayout->removeWidget(mPageDetails);
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
    QTableWidget * table = mPageList->ctrlTableIncludes();
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
    Q_ASSERT(mPageDetails != nullptr);
    Q_ASSERT(mPageList != nullptr);
    
    connect(  mPageDetails->ctrlInclude()
            , &QLineEdit::textChanged
            , this
            , [this](const QString& text)
                {
                    this->mPageList->ctrlButtonAdd()->setEnabled(text.isEmpty() == false);
                }
            );
    
    connect(mPageList->ctrlButtonAdd(), &QToolButton::clicked, this, &SIInclude::onAddClicked);
    connect(mPageList->ctrlButtonRemove(), &QToolButton::clicked, this, &SIInclude::onRemoveClicked);
    connect(mPageList->ctrlButtonInsert(), &QToolButton::clicked, this, &SIInclude::onInsertClicked);
    connect(mPageList->ctrlButtonUpdate(), &QToolButton::clicked, this, &SIInclude::onUpdateClicked);
    connect(mPageList->ctrlTableIncludes(), &QTableWidget::currentCellChanged, this, &SIInclude::onCurCellChanged);
    connect(mPageDetails->ctrlBrowseButton(), &QPushButton::clicked, this, &SIInclude::onBrowseClicked);        
}

void SIInclude::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    QTableWidget* table = mPageList->ctrlTableIncludes();
    if (currentRow != -1)
    {
        QTableWidgetItem * current = table->item(currentRow, currentColumn);
        const IncludeEntry * data = mModel.data(current->text());
        Q_ASSERT(data != nullptr);
        mPageList->ctrlButtonRemove()->setEnabled(true);
        mPageList->ctrlButtonDown()->setEnabled(currentRow < (table->rowCount() - 1));
        mPageList->ctrlButtonUp()->setEnabled(currentRow != 0);
        mPageDetails->ctrlInclude()->setText(data->getLocation());
        mPageDetails->ctrlDescription()->setPlainText(data->getDescription());
        mPageDetails->ctrlDepcrecateCheck()->setChecked(data->isDeprecated());
        mPageDetails->ctrlDeprecateHint()->setText(data->getDeprecationHint());
    }
    else
    {
        mPageList->ctrlButtonRemove()->setEnabled(false);
        mPageList->ctrlButtonDown()->setEnabled(false);
        mPageList->ctrlButtonUp()->setEnabled(false);
        mPageDetails->ctrlInclude()->setText("");
        mPageDetails->ctrlDescription()->setPlainText("");
        mPageDetails->ctrlDepcrecateCheck()->setChecked(false);
        mPageDetails->ctrlDeprecateHint()->setText("");
    }
}

void SIInclude::onAddClicked(void)
{
    QString location = mPageDetails->ctrlInclude()->text();
    QString describe = mPageDetails->ctrlDescription()->toPlainText();
    bool isDeprecate = mPageDetails->ctrlDepcrecateCheck()->isChecked();
    QString hint     = mPageDetails->ctrlDeprecateHint()->text();
    if ( (location.isEmpty() == false) && mModel.addEntry(location, describe, isDeprecate, hint) )
    {
        Q_ASSERT(mModel.rowCount() != 0);
        QTableWidget * table = mPageList->ctrlTableIncludes();
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
    QTableWidget* table = mPageList->ctrlTableIncludes();
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
    QTableWidget* table = mPageList->ctrlTableIncludes();
    QString location = mPageDetails->ctrlInclude()->text();
    int row = table->currentRow();
    if ((row != -1) && (location.isEmpty() == false))
    {
        QTableWidgetItem* item = table->currentItem();
        QString before   = item->text();
        QString describe = mPageDetails->ctrlDescription()->toPlainText();
        bool isDeprecate = mPageDetails->ctrlDepcrecateCheck()->isChecked();
        QString hint = mPageDetails->ctrlDeprecateHint()->text();
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
    QTableWidget* table = mPageList->ctrlTableIncludes();
    int row = table->currentRow();
    if (row != -1)
    {
        QString location = mPageDetails->ctrlInclude()->text();
        QString describe = mPageDetails->ctrlDescription()->toPlainText();
        bool isDeprecate = mPageDetails->ctrlDepcrecateCheck()->isChecked();
        QString hint = mPageDetails->ctrlDeprecateHint()->text();
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
        mPageDetails->ctrlInclude()->setText(include);
        mPageDetails->ctrlDescription()->setFocus();
        mPageDetails->ctrlDescription()->selectAll();

        mCurUrl = dialog.directoryUrl().path();
        mCurFile = dialog.getSelectedFilePath();
        mCurFilter = dialog.selectedNameFilter();
        mCurView = static_cast<int>(dialog.viewMode());
    }
}
