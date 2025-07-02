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
 *  \file        lusan/view/log/LogOfflineViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, offline log viewer widget.
 *
 ************************************************************************/

#include "lusan/view/log/LogOfflineViewer.hpp"
#include "ui/ui_LogOfflineViewer.h"

#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/model/log/LogOfflineModel.hpp"
#include "lusan/model/log/LogViewerFilterProxy.hpp"
#include <QMdiSubWindow>
#include <QMenu>
#include <QFileInfo>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QLabel>

const QString& LogOfflineViewer::fileExtension(void)
{
    return LogOfflineModel::getFileExtension();
}

LogOfflineViewer::LogOfflineViewer(MdiMainWindow *wndMain, const QString& filePath, QWidget *parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiOfflineLogViewer, wndMain, parent)

    , ui            (new Ui::LogOfflineViewer)
    , mLogModel     (nullptr)
    , mMdiWindow    (new QWidget())
    , mFilePath     (filePath)
{
    ui->setupUi(mMdiWindow);
    mLogModel = new LogOfflineModel(this);
    
    LogViewerFilterProxy* filter = mLogModel->getFilter();
    QTableView* view = ctrlTable();
    
    // For now, use standard QHeaderView instead of LogTableHeader to avoid dependency issues
    // TODO: Create LogOfflineTableHeader or make LogTableHeader generic
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

    view->setModel(filter);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    ctrlTable()->setAutoScroll(true);
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);

    // Connect signals
    connect(mLogModel       , &LogOfflineModel::signalDatabaseIsOpened   , this, &LogOfflineViewer::onDatabaseOpened);
    connect(mLogModel       , &LogOfflineModel::signalDatabaseIsClosed   , this, &LogOfflineViewer::onDatabaseClosed);
    connect(header          , &QHeaderView::customContextMenuRequested   , this, &LogOfflineViewer::onHeaderContextMenu);
    connect(view            , &QTableView::customContextMenuRequested    , this, &LogOfflineViewer::onTableContextMenu);

    // Try to open the database file
    if (!mFilePath.isEmpty())
    {
        if (!mLogModel->openDatabase(mFilePath))
        {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open log database file: %1").arg(mFilePath));
        }
    }
}

bool LogOfflineViewer::isDatabaseOpen(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isDatabaseOpen();
}

void LogOfflineViewer::onHeaderContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QModelIndex idx{ctrlTable()->currentIndex()};
    populateColumnsMenu(&menu, idx.isValid() ? idx.row() : -1);
    
    menu.exec(ctrlHeader()->mapToGlobal(pos));
}

void LogOfflineViewer::onTableContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    menu.addAction(tr("Copy"), [this](){
        // TODO: Implement copy functionality
    });
    
    menu.exec(ctrlTable()->mapToGlobal(pos));
}

void LogOfflineViewer::onDatabaseOpened(const QString& dbPath)
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

void LogOfflineViewer::onDatabaseClosed(const QString& dbPath)
{
    Q_UNUSED(dbPath);
    ctrlFile()->setText("");
    ctrlFile()->setToolTip("");
    
    if (mMdiSubWindow != nullptr)
    {
        mMdiSubWindow->setWindowTitle(tr("Offline Logs"));
    }
}

QTableView* LogOfflineViewer::ctrlTable(void)
{
    return ui->logView;
}

QHeaderView* LogOfflineViewer::ctrlHeader(void)
{
    return ui->logView->horizontalHeader();
}

QLabel* LogOfflineViewer::ctrlFile(void)
{
    return ui->lableFile;
}

void LogOfflineViewer::populateColumnsMenu(QMenu* menu, int curRow)
{
    // Get current active columns from the model
    const QList<LogOfflineModel::eColumn>& activeCols = mLogModel->getActiveColumns();
    const QStringList& headers{ LogOfflineModel::getHeaderList() };

    // Add actions for each available column
    for (int i = 0; i < static_cast<int>(LogOfflineModel::eColumn::LogColumnCount); ++i)
    {
        LogOfflineModel::eColumn col = static_cast<LogOfflineModel::eColumn>(i);
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

void LogOfflineViewer::resetColumnOrder()
{
    // Reset to default column order
    mLogModel->setActiveColumns(LogOfflineModel::getDefaultColumns());
}