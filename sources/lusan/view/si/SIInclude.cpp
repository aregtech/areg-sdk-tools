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
 *  \file        lusan/view/si/SIInclude.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIInclude.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIInclude.h"

#include "lusan/view/si/SIIncludeDetails.hpp"
#include "lusan/view/si/SIIncludeList.hpp"

SIIncludeWidget::SIIncludeWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIInclude)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIInclude::SIInclude(QWidget* parent)
    : QScrollArea   (parent)
    , mTopicDetails (new SIIncludeDetails(this))
    , mTopicList    (new SIIncludeList(this))
    , mWidget       (new SIIncludeWidget(this))
    , ui            (*mWidget->ui)
{
    ui.horizontalLayout->addWidget(mTopicList);
    ui.horizontalLayout->addWidget(mTopicDetails);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
}

SIInclude::~SIInclude(void)
{
    ui.horizontalLayout->removeWidget(mTopicList);
    ui.horizontalLayout->removeWidget(mTopicDetails);
}
