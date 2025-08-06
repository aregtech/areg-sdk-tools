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
 *  \file        lusan/view/log/ScopeOutputViewer.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Scope output viewer widget.
 *
 ************************************************************************/
#include "lusan/view/log/ScopeOutputViewer.hpp"
#include "ui/ui_ScopeOutputViewer.h"

#include "lusan/model/log/ScopeLogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/common/OutputDock.hpp"
#include "lusan/view/log/LogViewerBase.hpp"

#include "areg/logging/NELogging.hpp"

ScopeOutputViewer::ScopeOutputViewer(MdiMainWindow* wndMain, QWidget* parent)
    : OutputWindow  (static_cast<int>(OutputDock::eOutputDock::OutputLogging), wndMain, parent)
    , ui            (new Ui::ScopeOutputViewer)
    , mFilter       (new ScopeLogViewerFilter())
    , mLogModel     (nullptr)
{
    ui->setupUi(this);    
    ctrlTable()->setModel(nullptr);
    connect(ctrlTable()         , &QTableView::doubleClicked, [this](const QModelIndex &index){onMouseDoubleClicked(index);});
    connect(ctrlRadioSession()  , &QRadioButton::toggled    , [this](bool checked) {onRadioChecked(checked, eRadioType::RadioSession);});
    connect(ctrlRadioSublogs()  , &QRadioButton::toggled    , [this](bool checked) {onRadioChecked(checked, eRadioType::RadioSublogs);});
    connect(ctrlRadioScope()    , &QRadioButton::toggled    , [this](bool checked) {onRadioChecked(checked, eRadioType::RadioScope);});
    connect(ctrlRadioThread()   , &QRadioButton::toggled    , [this](bool checked) {onRadioChecked(checked, eRadioType::RadioThread);});
    connect(ctrlRadioProcess()  , &QRadioButton::toggled    , [this](bool checked) {onRadioChecked(checked, eRadioType::RadioProcess);});
    connect(mFilter, &ScopeLogViewerFilter::signalFilterSelected
            , [this](const QModelIndex& start, const QModelIndex& end) {
                onFilterChanged(start, end);
    });

    updateControls();
}

ScopeOutputViewer::~ScopeOutputViewer(void)
{
    ctrlTable()->setModel(nullptr);
    if (mFilter != nullptr)
    {
        mFilter->setSourceModel(nullptr);
        delete mFilter;
        mFilter = nullptr;
    }
    
    delete ui;
    ui = nullptr;
}

bool ScopeOutputViewer::releaseWindow(MdiChild& mdiChild)
{
    bool result = OutputWindow::releaseWindow(mdiChild);
    if (result)
    {
        if (mFilter != nullptr)
        {
            mFilter->setScopeFilter(nullptr, 0, 0, 0, 0);
        }

        mLogModel = nullptr;
        ctrlTable()->setModel(nullptr);
        updateLogTable();
    }

    return result;
}

void ScopeOutputViewer::setupFilter(LoggingModelBase* logModel, uint32_t scopeId, uint32_t sessionId, ITEM_ID instance)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
    }
    else
    {
        mLogModel = logModel;
        mFilter->setScopeFilter(logModel, scopeId, sessionId, 0, instance);
        if (ctrlTable()->model() == nullptr)
            ctrlTable()->setModel(logModel == nullptr ? nullptr : mFilter);
    }

    updateLogTable();
}

void ScopeOutputViewer::setupFilter(LoggingModelBase* logModel, const QModelIndex& index)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
    }
    else
    {
        mLogModel = logModel;
        mFilter->setScopeFilter(logModel, index);
        if (ctrlTable()->model() == nullptr)
            ctrlTable()->setModel(logModel == nullptr ? nullptr : mFilter);
    }
    
    updateLogTable();
}

void ScopeOutputViewer::onMouseDoubleClicked(const QModelIndex& index)
{
    QTableView *logTable = (index.isValid() && (mMdiChild != nullptr)) ? static_cast<LogViewerBase *>(mMdiChild)->getLoggingTable() : nullptr;
    if ((logTable != nullptr) && (mFilter != nullptr))
    {
        QModelIndex srcIndex = mFilter->mapToSource(index);
        logTable->selectionModel()->setCurrentIndex(srcIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        logTable->selectRow(srcIndex.row());
        logTable->scrollTo(srcIndex);
    }
}

void ScopeOutputViewer::onRadioChecked(bool checked, eRadioType radio)
{
    if ((mFilter == nullptr) || (checked == false))
        return;

    switch (radio)
    {
    case eRadioType::RadioSession:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterSession);
        break;
    
    case eRadioType::RadioSublogs:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterSublogs);
        break;

    case eRadioType::RadioScope:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterScope);
        break;

    case eRadioType::RadioThread:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterThread);
        break;

    case eRadioType::RadioProcess:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::FilterProcess);
        break;

    case eRadioType::RadioNone:
    default:
        mFilter->filterData(ScopeLogViewerFilter::eDataFilter::NoFilter);
        break;
    }
}

void ScopeOutputViewer::onFilterChanged(const QModelIndex & indexStart, const QModelIndex& indexEnd)
{
    ctrlDuration()->setText(QString("N/A"));
    if (indexEnd.isValid())
    {
        const NELogging::sLogMessage * log = mLogModel != nullptr ? mLogModel->data(indexEnd, static_cast<int>(Qt::UserRole)).value<const NELogging::sLogMessage *>() : nullptr;
        if (log != nullptr)
        {
            ctrlDuration()->setText(QString::number(log->logDuration));
        }
    }
}

inline QTableView* ScopeOutputViewer::ctrlTable(void) const
{
    return ui->logTable;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioSession(void) const
{
    return ui->radioSession;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioSublogs(void) const
{
    return ui->radioSublogs;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioScope(void) const
{
    return ui->radioScope;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioThread(void) const
{
    return ui->radioThread;
}

inline QRadioButton* ScopeOutputViewer::ctrlRadioProcess(void) const
{
    return ui->radioProcess;
}

inline QLineEdit* ScopeOutputViewer::ctrlDuration(void) const
{
    return ui->editDuration;
}

inline void ScopeOutputViewer::updateLogTable(void)
{
    QTableView *logTable = mMdiChild != nullptr ? static_cast<LogViewerBase *>(mMdiChild)->getLoggingTable() : nullptr;
    if (logTable != nullptr)
    {
        logTable->viewport()->update();
    }

    updateControls();
}

inline void ScopeOutputViewer::updateControls(void)
{
    bool hasElems{(mFilter != nullptr) && (mFilter->rowCount() != 0)};
    blockSignals(true);
    if (hasElems)
    {
        ctrlRadioSession()->setChecked(true);

        ctrlRadioSession()->setEnabled(true);
        ctrlRadioSublogs()->setEnabled(true);
        ctrlRadioScope()->setEnabled(true);
        ctrlRadioThread()->setEnabled(true);
        ctrlRadioProcess()->setEnabled(true);
    }
    else
    {
        ctrlRadioSession()->setChecked(false);
        ctrlRadioSublogs()->setChecked(false);
        ctrlRadioScope()->setChecked(false);
        ctrlRadioThread()->setChecked(false);
        ctrlRadioProcess()->setChecked(false);

        ctrlRadioSession()->setEnabled(false);
        ctrlRadioSublogs()->setEnabled(false);
        ctrlRadioScope()->setEnabled(false);
        ctrlRadioThread()->setEnabled(false);
        ctrlRadioProcess()->setEnabled(false);
    }

    ctrlDuration()->setText(QString("N/A"));
    blockSignals(false);
}
