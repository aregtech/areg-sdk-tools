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
 *  \file        lusan/view/common/OutputWindow.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation windows elements.
 *
 ************************************************************************/

#include "lusan/view/common/OutputWindow.hpp"
#include "lusan/view/common/OutputDock.hpp"

OutputWindow::OutputWindow(int outWindow, MdiMainWindow * wndMain, QWidget* parent)
    : QWidget(parent)

    , mOutWindowType    (outWindow)
    , mMainWindow       (wndMain)
{
}

void OutputWindow::optionOpenning(void)
{
}

void OutputWindow::optionApplied(void)
{
}

void OutputWindow::optionClosed(bool OKpressed)
{
}

bool OutputWindow::isScopesOutputWindow(void) const
{
    return (mOutWindowType == static_cast<int>(OutputDock::eOutputDock::OutputLogging));
}
