/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
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
    QItemSelectionModel *selModel = ctrlTable()->selectionModel();
    Q_ASSERT(selModel != nullptr);
    connect(ctrlLogShow()       , &QToolButton::clicked     , this, [this]()              { onShowLog(getSelectedIndex());              });
    connect(ctrlScopeBegin()    , &QToolButton::clicked     , this, [this]()              { onShowLog(mFilter->getIndexStart(false));   });
    connect(ctrlScopeEnd()      , &QToolButton::clicked     , this, [this]()              { onShowLog(mFilter->getIndexEnd(false));     });
    connect(ctrlScopeNext()     , &QToolButton::clicked     , this, [this]()              { onShowNextLog();                            });
    connect(ctrlScopePrev()     , &QToolButton::clicked     , this, [this]()              { onShowPrevLog();                            });

    connect(ctrlTable()         , &QTableView::doubleClicked, this, [this](const QModelIndex& index)  {onShowLog(index);    });
    connect(ctrlTable()         , &QTableView::clicked      , this, [this](const QModelIndex& index)  {ctrlLogShow()->setEnabled(index.isValid());});
    connect(ctrlRadioSession()  , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioSession);});
    connect(ctrlRadioSublogs()  , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioSublogs);});
    connect(ctrlRadioScope()    , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioScope);  });
    connect(ctrlRadioThread()   , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioThread); });
    connect(ctrlRadioProcess()  , &QRadioButton::toggled    , this, [this](bool checked)  { onRadioChecked(checked, eRadioType::RadioProcess);});
    connect(mFilter, &ScopeLogViewerFilter::signalFilterSelected, this
            , [this](const QModelIndex& start, const QModelIndex& end) {
                onFilterChanged(start, end);
    });
    connect(selModel            , &QItemSelectionModel::currentRowChanged, this
            , [this](const QModelIndex &current, const QModelIndex &previous){
                updateToolbuttons(mFilter->rowCount(), current);
    });

    updateControls(true);
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
            float duration = static_cast<float>(static_cast<double>(log->logDuration) / 1000.0f);
            ctrlDuration()->setText(QString::number(duration));
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

inline QToolButton* ScopeOutputViewer::ctrlLogShow(void) const
{
    return ui->toolLogShow;
}

inline QToolButton* ScopeOutputViewer::ctrlScopeBegin(void) const
{
    return ui->toolScopeBegin;
}

inline QToolButton* ScopeOutputViewer::ctrlScopeEnd(void) const
{
    return ui->toolScopeEnd;
}

inline QToolButton* ScopeOutputViewer::ctrlScopeNext(void) const
{
    return ui->toolScopeNext;
}

inline QToolButton* ScopeOutputViewer::ctrlScopePrev(void) const
{
    return ui->toolScopePrev;
}

inline void ScopeOutputViewer::updateLogTable(void)
{
    QTableView *logTable = mMdiChild != nullptr ? static_cast<LogViewerBase *>(mMdiChild)->getLoggingTable() : nullptr;
    if (logTable != nullptr)
    {
        logTable->viewport()->update();
    }

    updateControls(true);
}

inline void ScopeOutputViewer::updateControls(bool selectSession)
{
    int count{ mFilter != nullptr ? mFilter->rowCount() : 0 };
    bool hasEntries{count != 0};
    blockSignals(true);
    
    if (hasEntries == false)
    {
        ctrlRadioSession()->setChecked(false);
        ctrlRadioSublogs()->setChecked(false);
        ctrlRadioScope()->setChecked(false);
        ctrlRadioThread()->setChecked(false);
        ctrlRadioProcess()->setChecked(false);
    }
    else if (selectSession)
    {
        ctrlRadioSession()->setChecked(true);
    }

    ctrlRadioSession()->setEnabled(hasEntries);
    ctrlRadioSublogs()->setEnabled(hasEntries);
    ctrlRadioScope()->setEnabled(hasEntries);
    ctrlRadioThread()->setEnabled(hasEntries);
    ctrlRadioProcess()->setEnabled(hasEntries);

    updateToolbuttons(count, getSelectedIndex());
    
    blockSignals(false);
}

inline void ScopeOutputViewer::updateToolbuttons(int rowCount, const QModelIndex& selIndex)
{
    if (selIndex.isValid() == false)
    {
        ctrlDuration()->setText(QString("N/A"));
    }
    
    bool hasEntries(rowCount != 0);
    QModelIndex start = mFilter->getIndexStart(true);
    QModelIndex end = mFilter->getIndexEnd(true);
    
    ctrlLogShow()->setEnabled(selIndex.isValid());
    ctrlScopeBegin()->setEnabled(start.isValid() && (selIndex != start));
    ctrlScopeEnd()->setEnabled(end.isValid() && (selIndex != end));
    ctrlScopeNext()->setEnabled(hasEntries && ((selIndex.isValid() == false) || (selIndex.row() < (rowCount - 1))));
    ctrlScopePrev()->setEnabled(hasEntries && ((selIndex.isValid() == false) || (selIndex.row() > 0)));
}

void ScopeOutputViewer::onShowLog(const QModelIndex& idxTarget)
{
    if ((mMdiChild != nullptr) && (mFilter != nullptr))
    {
        if (idxTarget.isValid())
        {
            blockSignals(true);
            QTableView* table = ctrlTable();
            table->selectionModel()->setCurrentIndex(idxTarget, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            table->selectionModel()->select(idxTarget, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            table->selectRow(idxTarget.row());
            table->scrollTo(idxTarget);
            
            QModelIndex srcIndex = mFilter->mapToSource(idxTarget);
            static_cast<LogViewerBase*>(mMdiChild)->selectSourceElement(srcIndex);
            updateToolbuttons(mFilter->rowCount(), idxTarget);
            blockSignals(false);
        }
    }
}

void ScopeOutputViewer::onShowNextLog(void)
{
    QModelIndex idxTarget = getSelectedIndex();
    idxTarget = mFilter->getIndexNextScope(idxTarget, false);
    onShowLog(idxTarget.isValid() ? idxTarget : mFilter->index(mFilter->rowCount() - 1, 0));
}

void ScopeOutputViewer::onShowPrevLog(void)
{
    QModelIndex idxTarget = getSelectedIndex();
    idxTarget = mFilter->getIndexPrevScope(idxTarget, false);
    onShowLog(idxTarget.isValid() ? idxTarget : mFilter->index(0, 0));
}

inline QModelIndex ScopeOutputViewer::getSelectedIndex(void) const
{
    return ctrlTable()->selectionModel()->currentIndex();
}
