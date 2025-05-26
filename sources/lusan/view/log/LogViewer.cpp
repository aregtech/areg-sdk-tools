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
 *  \file        lusan/view/log/LogViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/view/log/LogViewer.hpp"

#include "ui/ui_LogViewer.h"
#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"
#include "lusan/model/log/LogViewerModel.hpp"
#include "areg/base/File.hpp"
#include "areglogger/client/LogObserverApi.h"

#include <filesystem>

LogViewer::LogViewer(QWidget *parent)
    : MdiChild      (parent)
    , IEMdiWindow   (IEMdiWindow::eMdiWindow::MdiLogViewer)

    , ui(new Ui::LogViewer)
    , mLogModel(nullptr)
    , mMdiWindow(new QWidget())
{
    ui->setupUi(mMdiWindow);
    
    mLogModel = new LogViewerModel(this);
    getHeader()->setVisible(true);
    getHeader()->show();
    
    QTableView* view = getTable();    
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
    
    view->setModel(mLogModel);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
    getTable()->setAutoScroll(true);

    connect(mLogModel, &LogViewerModel::rowsInserted, this, &LogViewer::onRowsInserted);
    
}


void LogViewer::logServiceConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    Q_ASSERT(mLogModel != nullptr);
    mLogModel->serviceConnected(isConnected, address, port, dbPath);
}

bool LogViewer::isServiceConnected(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isConnected();
}

void LogViewer::moveToBottom(bool lastSelect)
{
    QTableView* logs = getTable();
    Q_ASSERT(logs != nullptr);
    logs->scrollToBottom();
    if (lastSelect)
    {
        int count = mLogModel->rowCount(QModelIndex());
        if (count > 0)
        {
            logs->selectRow(count - 1);
        }
    }
}

void LogViewer::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    QModelIndex curIndex = getTable()->currentIndex();
    int row = curIndex.isValid() ? curIndex.row() : -1;
    int count = mLogModel->rowCount(parent);
    if ((row < 0) || (row >= count - 2))
    {
        getTable()->scrollToBottom();
        if (row >= 0)
        {
            getTable()->selectRow(count - 1);
        }
    }
}

QTableView* LogViewer::getTable(void)
{
    return ui->logView;
}

QHeaderView* LogViewer::getHeader(void)
{
    return ui->logView->horizontalHeader();
}
