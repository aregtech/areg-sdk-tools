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

const QString& LogViewer::fileExtension(void)
{
    static const QString _extSI{ "sqlog" };
    return _extSI;
}

QString LogViewer::generateFileName(void)
{
    String name{ File::normalizePath("log_%time%.sqlog") };
    return QString(name.getString());
}

LogViewer::LogViewer(QWidget *parent)
    : MdiChild(parent)
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
    
    view->setModel(mLogModel);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);
    
    setAttribute(Qt::WA_DeleteOnClose);
}

QString LogViewer::newLogFile(void) const
{
    WorkspaceEntry workspace = LusanApplication::getActiveWorkspace();
    QString result = workspace.getDirLogs();
    if ( result.isEmpty() == false )
    {
        std::filesystem::path fPath(result.toStdString().c_str());
        fpath /= generateFileName().toStdString().c_str();
        result = QString(fPath.c_str());
    }
    else
    {
        result = logObserverGetConfigDatabasePath().getString();
    }

    return result;
}

QTableView* LogViewer::getTable(void)
{
    return ui->logView;
}

QHeaderView* LogViewer::getHeader(void)
{
    return ui->logView->horizontalHeader();
}
