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
 *  \file        lusan/view/common/OutputDock.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Output window.
 *
 ************************************************************************/

#include "lusan/view/common/OutputDock.hpp"

#include "lusan/view/common/MdiMainWindow.hpp"

const QString   OutputDock::TabNameLogging{ tr("Log analyzes") };
const QSize     OutputDock::IconSize{ 32, 32 };

QIcon OutputDock::getLoggingIcon(void)
{
    return QIcon();
}

const QString& OutputDock::getTabName(OutputDock::eOutputDock wndOutput)
{
    static const QString   _empty;
    switch (wndOutput)
    {
    case OutputDock::eOutputDock::OutputLogging:
        return OutputDock::TabNameLogging;
    default:
        return _empty;
    }
}

OutputDock::eOutputDock OutputDock::getOutputDock(const QString& tabName)
{
    if (tabName == OutputDock::TabNameLogging)
        return OutputDock::eOutputDock::OutputLogging;
    else
        return OutputDock::eOutputDock::OutputUnknown;
}

OutputDock::OutputDock(MdiMainWindow* parent)
    : QDockWidget   (tr("Output"), parent)
    , mMainWindow   (parent)
    , mTabs         (this)
    , mScopeOutput  (parent, this)
{
    mTabs.addTab(&mScopeOutput, QIcon(), tr("Scopes Analyzes"));
    mTabs.setTabPosition(QTabWidget::South);
    setWidget(&mTabs);
    
    initSize();
}

OutputDock::~OutputDock(void)
{
    mTabs.removeTab(0);
    mTabs.setParent(nullptr);    
}

void OutputDock::initSize(void)
{
    QSize szSO{mScopeOutput.size()};
    int maxWidth = szSO.width();
    int maxHeight= szSO.height();
    
    resize(QSize { maxWidth,  maxHeight });
    setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
}
