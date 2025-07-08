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
 *  \file        lusan/view/log/OfflineLogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, offline log viewer widget.
 *
 ************************************************************************/

#include "lusan/view/log/OfflineLogViewer.hpp"
#include "ui/ui_OfflineLogViewer.h"

#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"
#include <QMdiSubWindow>
#include <QMenu>
#include <QFileInfo>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QLabel>

const QString& OfflineLogViewer::fileExtension(void)
{
    return OfflineLogsModel::getFileExtension();
}

OfflineLogViewer::OfflineLogViewer(MdiMainWindow *wndMain, const QString& filePath, QWidget *parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiOfflineLogViewer, wndMain, parent)

    , ui            (new Ui::OfflineLogViewer)
    , mLogModel     (nullptr)
    , mFilter       (nullptr)
    , mMdiWindow    (new QWidget())
    , mFilePath     (filePath)
{
    ui->setupUi(mMdiWindow);
    mLogModel   = new OfflineLogsModel(this);
    mFilter     = new LogViewerFilterProxy(mLogModel);
    
    QTableView* view = ctrlTable();
    view->setHorizontalHeader(new LogTableHeader(view, mLogModel));
    QHeaderView* header = ctrlHeader();
    
    header->setVisible(true);
    header->show();
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    header->setSectionsMovable(true);
    
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setShowGrid(false);
    view->setCurrentIndex(QModelIndex());
    view->horizontalHeader()->setStretchLastSection(true);
    view->verticalHeader()->hide();
    view->setAutoScroll(true);
    view->setVerticalScrollMode(QTableView::ScrollPerItem);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    view->setModel(mFilter);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    ctrlTable()->setAutoScroll(true);
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

    // Connect signals
    connect(mLogModel       , &OfflineLogsModel::signalDatabaseIsOpened   , this, &OfflineLogViewer::onDatabaseOpened);
    connect(mLogModel       , &OfflineLogsModel::signalDatabaseIsClosed   , this, &OfflineLogViewer::onDatabaseClosed);
    connect(header          , &QHeaderView::customContextMenuRequested   , this, &OfflineLogViewer::onHeaderContextMenu);
    connect(view            , &QTableView::customContextMenuRequested    , this, &OfflineLogViewer::onTableContextMenu);
    connect(header, SIGNAL(signalComboFilterChanged(int, QStringList))  , mFilter, SLOT(setComboFilter(int, QStringList)));
    connect(header, SIGNAL(signalTextFilterChanged(int, QString))       , mFilter, SLOT(setTextFilter(int, QString)));

    // Try to open the database file
    if (mFilePath.isEmpty() == false)
    {
        mLogModel->openDatabase(mFilePath, true);
        if (mLogModel->isOperable() == false)
        {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open log database file: %1").arg(mFilePath));
        }
    }
}

bool OfflineLogViewer::isDatabaseOpen(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isOperable();
}

void OfflineLogViewer::onHeaderContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QModelIndex idx{ctrlTable()->currentIndex()};
    populateColumnsMenu(&menu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlHeader()->mapToGlobal(pos));
}

void OfflineLogViewer::onTableContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QMenu* columnsMenu = menu.addMenu(tr("Columns"));
    QModelIndex idx{ctrlTable()->currentIndex()};    
    populateColumnsMenu(columnsMenu, idx.isValid() ? idx.row() : -1);
    menu.exec(ctrlTable()->viewport()->mapToGlobal(pos));
}

void OfflineLogViewer::onDatabaseOpened(const QString& dbPath)
{
    QFileInfo info(dbPath);
    QString fileName = info.fileName();
    
    ctrlFile()->setText(fileName);
    ctrlFile()->setToolTip(dbPath);
    
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(tr("Offline Logs - %1").arg(fileName));
    }
}

void OfflineLogViewer::onDatabaseClosed(const QString& dbPath)
{
    Q_UNUSED(dbPath);
    ctrlFile()->setText("");
    ctrlFile()->setToolTip("");
    
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(tr("Offline Logs"));
    }
}

QTableView* OfflineLogViewer::ctrlTable(void)
{
    return ui->logView;
}

QHeaderView* OfflineLogViewer::ctrlHeader(void)
{
    return ui->logView->horizontalHeader();
}

QLabel* OfflineLogViewer::ctrlFile(void)
{
    return ui->lableFile;
}

void OfflineLogViewer::populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<OfflineLogsModel::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ OfflineLogsModel::getHeaderList() };

    // Add actions for each available column
    for (int i = 0; i < static_cast<int>(OfflineLogsModel::eColumn::LogColumnCount); ++i)
    {
        OfflineLogsModel::eColumn col = static_cast<OfflineLogsModel::eColumn>(i);
        QString headerName = headers.at(i);
        
        QAction* action = menu->addAction(headerName);
        action->setCheckable(true);
        action->setChecked(activeCols.contains(col));
        
        connect(action, &QAction::triggered, [this, col, action](){
            if (action->isChecked())
            {
                mLogModel->addColumn(col);
            }
            else
            {
                mLogModel->removeColumn(col);
            }
        });
    }
}

void OfflineLogViewer::resetColumnOrder()
{
    // Reset to default column order
    mLogModel->setActiveColumns(OfflineLogsModel::getDefaultColumns());
}
