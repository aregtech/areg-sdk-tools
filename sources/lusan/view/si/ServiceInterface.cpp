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
 *  \file        lusan/view/si/ServiceInterface.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include "lusan/view/si/ServiceInterface.hpp"

#include <QVBoxLayout>

ServiceInterface::ServiceInterface(QWidget *parent)
    : MdiChild(parent)
    , mTabWidget(this)
    , mOverview(this)
{
    mTabWidget.setTabPosition(QTabWidget::South);
    // Add the sioverview widget as the first tab
    mTabWidget.addTab(&mOverview, tr("Overview"));
    
    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&mTabWidget);
    setLayout(layout);
    
    mOverview.setMinimumSize(480, 360);
    mOverview.resize(930, 520);
}

ServiceInterface::~ServiceInterface(void)
{
}
