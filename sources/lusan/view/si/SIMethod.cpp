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
 *  \file        lusan/view/si/SIMethod.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIMethod.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIMethod.h"

#include "lusan/view/si/SIMethodDetails.hpp"
#include "lusan/view/si/SIMethodParamDetails.hpp"
#include "lusan/view/si/SIMethodList.hpp"

SIMethodWidget::SIMethodWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIMethod)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIMethod::SIMethod(QWidget* parent)
    : QScrollArea   (parent)
    , mMethodDetails(new SIMethodDetails(this))
    , mMethodList   (new SIMethodList(this))
    , mParamDetails (new SIMethodParamDetails(this))
    , mWidget       (new SIMethodWidget(this))
    , ui            (*mWidget->ui)
{
    mParamDetails->setHidden(true);

    ui.horizontalLayout->addWidget(mMethodList);
    ui.horizontalLayout->addWidget(mMethodDetails);

    // setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
}

SIMethod::~SIMethod(void)
{
    ui.horizontalLayout->removeWidget(mMethodList);
    ui.horizontalLayout->removeWidget(mMethodDetails);
}
