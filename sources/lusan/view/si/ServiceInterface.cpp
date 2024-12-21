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
#include "lusan/view/si/SICommon.hpp"

#include <QVBoxLayout>

ServiceInterface::ServiceInterface(QWidget *parent)
    : MdiChild  (parent)
    , mTabWidget(this)
    , mOverview (this)
    , mDataType (this)
    , mDataTopic(this)
    , mMethod   (this)
{
    mTabWidget.setTabPosition(QTabWidget::South);
    // Add the sioverview widget as the first tab
    mTabWidget.addTab(&mOverview, tr("Service Overview"));
    mTabWidget.addTab(&mDataType, tr("Data Types"));
    mTabWidget.addTab(&mDataTopic, tr("Data Topics"));
    mTabWidget.addTab(&mMethod, tr("Methods"));

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&mTabWidget);
    setLayout(layout);
    
    // mOverview.resize(SICommon::FRAME_WIDTH, SICommon::FRAME_WIDTH);
    // mDataType.resize(SICommon::FRAME_WIDTH, SICommon::FRAME_WIDTH);
    // mDataTopic.resize(SICommon::FRAME_WIDTH, SICommon::FRAME_WIDTH);
}

ServiceInterface::~ServiceInterface(void)
{
}
