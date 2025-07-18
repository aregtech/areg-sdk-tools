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

#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviOfflineLogsScopes.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"

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

OfflineLogViewer::OfflineLogViewer(MdiMainWindow *wndMain, QWidget *parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiOfflineLogViewer, wndMain, parent)

    , ui            (new Ui::OfflineLogViewer)
    , mLogModel     (nullptr)
    , mFilter       (nullptr)
    , mMdiWindow    (new QWidget())
    , mHeader       (nullptr)
{
    ui->setupUi(mMdiWindow);
    mLogModel   = new OfflineLogsModel(this);
    mFilter     = new LogViewerFilterProxy(mLogModel);
    
    setupWidgets();
    setupSignals(true);
}

OfflineLogViewer::OfflineLogViewer(MdiMainWindow* wndMain, LiveLogViewer& liveLogs, QWidget* parent)
    : MdiChild      (MdiChild::eMdiWindow::MdiOfflineLogViewer, wndMain, parent)

    , ui            (new Ui::OfflineLogViewer)
    , mLogModel     (nullptr)
    , mFilter       (nullptr)
    , mMdiWindow    (new QWidget())
    , mHeader       (nullptr)
{
    ui->setupUi(mMdiWindow);
    mLogModel   = new OfflineLogsModel(this);
    mFilter     = new LogViewerFilterProxy(mLogModel);

    LiveLogsModel* liveModel = liveLogs.getLoggingModel();
    if (liveModel != nullptr)
    {
        mLogModel->dataTransfer(*liveModel);
        setCurrentFile(mLogModel->getDatabasePath());
    }
    
    setupWidgets();
    setupSignals(true);
    const QModelIndex idxSelected = mLogModel->getSelectedLog();
    if (idxSelected.isValid())
    {
        ctrlTable()->selectionModel()->setCurrentIndex(idxSelected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ctrlTable()->selectRow(idxSelected.row());
        ctrlTable()->scrollTo(idxSelected);
    }
    else
    {
        ctrlTable()->setCurrentIndex(idxSelected);
        ctrlTable()->scrollToTop();
    }
}

OfflineLogViewer::~OfflineLogViewer(void)
{
    cleanResources();
}

bool OfflineLogViewer::isDatabaseOpen(void) const
{
    Q_ASSERT(mLogModel != nullptr);
    return mLogModel->isOperable();
}

bool OfflineLogViewer::openDatabase(const QString & logPath)
{
    bool result{false};
    mLogModel->closeDatabase();
    if (logPath.isEmpty() == false)
    {
        mLogModel->openDatabase(logPath, true);
        if (mLogModel->isOperable())
        {
            setCurrentFile(mLogModel->getDatabasePath());
            result = true;
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open log database file: %1").arg(logPath));
        }
    }
    
    return result;
}

void OfflineLogViewer::onWindowClosing(bool isActive)
{
    setupSignals(false);
    Q_ASSERT(mMainWindow != nullptr);
    if (isActive)
    {
        mMainWindow->getNaviOfflineScopes().setLoggingModel(nullptr);
        cleanResources();
    }
}

void OfflineLogViewer::onWindowActivated(void)
{
    Q_ASSERT(mMainWindow != nullptr);
    if (mMainWindow->getNaviOfflineScopes().getLoggingModel() != mLogModel)
    {
        mMainWindow->getNaviOfflineScopes().setLoggingModel(nullptr);
        mMainWindow->getNaviOfflineScopes().setLoggingModel(mLogModel);
    }

    mMainWindow->getNaviOfflineScopes().activateWindow();
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

QLineEdit* OfflineLogViewer::ctrlSearchText(void)
{
    return ui->textSearcb;
}

QToolButton* OfflineLogViewer::ctrlButtonSearch(void)
{
    return ui->btnSearch;
}

QToolButton* OfflineLogViewer::ctrlButtonCaseSensitive(void)
{
    return ui->btnMatchCase;
}

QToolButton* OfflineLogViewer::ctrlButtonWholeWords(void)
{
    return ui->btnMatchWord;
}

QToolButton* OfflineLogViewer::ctrlSearchDirection(void)
{
    return ui->btnSearchDirection;
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

void OfflineLogViewer::setupSignals(bool doSetup)
{
    if (ui == nullptr)
    {
        Q_ASSERT(mLogModel == nullptr);
        Q_ASSERT(mFilter == nullptr);
        return;
    }

    QTableView* view = ctrlTable();
    if (doSetup)
    {
        // Connect signals
        connect(mLogModel, &OfflineLogsModel::signalDatabaseIsOpened, this      , &OfflineLogViewer::onDatabaseOpened);
        connect(mLogModel, &OfflineLogsModel::signalDatabaseIsClosed, this      , &OfflineLogViewer::onDatabaseClosed);
        connect(mHeader  , &QHeaderView::customContextMenuRequested , this      , &OfflineLogViewer::onHeaderContextMenu);
        connect(mHeader  , &LogTableHeader::signalComboFilterChanged, mFilter   , &LogViewerFilterProxy::setComboFilter);
        connect(mHeader  , &LogTableHeader::signalTextFilterChanged , mFilter   , &LogViewerFilterProxy::setTextFilter);
        connect(view     , &QTableView::customContextMenuRequested  , this      , &OfflineLogViewer::onTableContextMenu);
    }
    else
    {
        // Disconnect signals
        disconnect(mLogModel, &OfflineLogsModel::signalDatabaseIsOpened, this      , &OfflineLogViewer::onDatabaseOpened);
        disconnect(mLogModel, &OfflineLogsModel::signalDatabaseIsClosed, this      , &OfflineLogViewer::onDatabaseClosed);
        disconnect(mHeader  , &QHeaderView::customContextMenuRequested , this      , &OfflineLogViewer::onHeaderContextMenu);
        disconnect(mHeader  , &LogTableHeader::signalComboFilterChanged, mFilter   , &LogViewerFilterProxy::setComboFilter);
        disconnect(mHeader  , &LogTableHeader::signalTextFilterChanged , mFilter   , &LogViewerFilterProxy::setTextFilter);
        disconnect(view     , &QTableView::customContextMenuRequested  , this      , &OfflineLogViewer::onTableContextMenu);
    }
}

void OfflineLogViewer::setupWidgets(void)
{
    QTableView* view = ctrlTable();
    mHeader = new LogTableHeader(view, mLogModel);
    view->setHorizontalHeader(mHeader);

    mHeader->setVisible(true);
    mHeader->show();
    mHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    mHeader->setSectionsMovable(true);

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
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mMdiWindow);
    setLayout(layout);

    setAttribute(Qt::WA_DeleteOnClose);
    ctrlTable()->setAutoScroll(true);
    ctrlFile()->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
}

void OfflineLogViewer::cleanResources(void)
{
    if (ui == nullptr)
    {
        Q_ASSERT(mLogModel == nullptr);
        Q_ASSERT(mFilter == nullptr);
        return;
    }

    Q_ASSERT(mLogModel != nullptr);
    Q_ASSERT(mFilter != nullptr);

    setupSignals(false);

    QTableView* view = ctrlTable();
    view->setModel(nullptr);
    view->setHorizontalHeader(nullptr);
    mFilter->setSourceModel(nullptr);
    mLogModel->closeDatabase();

    delete ui;
    ui = nullptr;

    delete mMdiWindow;
    mMdiWindow = nullptr;

    delete mFilter;
    mFilter = nullptr;

    delete mLogModel;
    mLogModel = nullptr;
}
