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
 *  \file        lusan/view/si/SIOverviewLinks.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIOverviewLinks.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIOverviewLinks.h"

SIOverviewLinks::SIOverviewLinks(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIOverviewLinks)
{
    QFont font{ this->font() };
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
}

QPushButton* SIOverviewLinks::linkDataTypes(void) const
{
    return ui->linkDataTypes;
}

QPushButton* SIOverviewLinks::linkAttributes(void) const
{
    return ui->linkAttributes;
}

QPushButton* SIOverviewLinks::linkMethods(void) const
{
    return ui->linkMethods;
}

QPushButton* SIOverviewLinks::linkConstants(void) const
{
    return ui->linkConstants;
}

QPushButton* SIOverviewLinks::linkIncludes(void) const
{
    return ui->linkIncludes;
}
