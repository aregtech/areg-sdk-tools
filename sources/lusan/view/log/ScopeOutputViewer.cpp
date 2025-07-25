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

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/OutputDock.hpp"
#include "lusan/model/log/ScopeLogViewerFilter.hpp"

ScopeOutputViewer::ScopeOutputViewer(MdiMainWindow* wndMain, QWidget* parent)
    : OutputWindow  (static_cast<int>(OutputDock::eOutputDock::OutputLogging), wndMain, parent)
    , ui            (new Ui::ScopeOutputViewer)
    , mFilter       (new ScopeLogViewerFilter())
    , mLogModel     (nullptr)
{
    ui->setupUi(this);    
    ctrlTable()->setModel(nullptr);
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

void ScopeOutputViewer::setupFilter(LoggingModelBase* logModel, uint32_t scopeId, uint32_t sessionId, ITEM_ID instance)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
        ctrlTable()->update();
        return;
    }

    mLogModel = logModel;
    mFilter->setScopeFilter(logModel, scopeId, std::vector<uint32_t>{sessionId}, std::vector<ITEM_ID>{instance}, 0);
    if (ctrlTable()->model() == nullptr)
        ctrlTable()->setModel(mFilter);
}

void ScopeOutputViewer::setupFilter(LoggingModelBase* logModel, const QModelIndex& index)
{
    if (mFilter == nullptr)
    {
        ctrlTable()->setModel(nullptr);
        ctrlTable()->update();
        return;
    }
    
    mLogModel = logModel;
    mFilter->setScopeFilter(logModel, index);
    if (ctrlTable()->model() == nullptr)
        ctrlTable()->setModel(mFilter);
}

inline QTableView* ScopeOutputViewer::ctrlTable(void) const
{
    return ui->logTable;
}
