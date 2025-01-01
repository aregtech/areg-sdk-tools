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
 *  \file        lusan/view/si/SIOverview.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIOverview.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIOverview.h"

#include "lusan/view/si/SIOverviewDetails.hpp"
#include "lusan/view/si/SIOverviewLinks.hpp"

SIOverviewWidget::SIOverviewWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIOverview)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIOverview::SIOverview(SIOverviewModel& model, QWidget* parent)
    : QScrollArea       (parent)
    , mModel            (model)
    , mOverviewDetails  (new SIOverviewDetails(model, this))
    , mOverviewLinks    (new SIOverviewLinks(this))
    , mWidget           (new SIOverviewWidget(this))
    , ui                (*mWidget->ui)
{
    ui.horizontalLayout->addWidget(mOverviewDetails);
    ui.horizontalLayout->addWidget(mOverviewLinks);

    // setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
}

SIOverview::~SIOverview(void)
{
    ui.horizontalLayout->removeWidget(mOverviewLinks);
    ui.horizontalLayout->removeWidget(mOverviewDetails);
}
