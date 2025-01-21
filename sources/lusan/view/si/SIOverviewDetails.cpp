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
 *  \file        lusan/view/si/SIOverviewDetails.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIOverviewDetails.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIOverviewDetails.h"
#include "lusan/model/si/SIOverviewModel.hpp"


SIOverviewDetails::SIOverviewDetails(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIOverviewDetails)
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

QLineEdit* SIOverviewDetails::ctrlMajor(void)
{
    return ui->editVersionMajor;
}

QLineEdit* SIOverviewDetails::ctrlMinor(void)
{
    return ui->editVersionMinor;
}

QLineEdit* SIOverviewDetails::ctrlPatch(void)
{
    return ui->editVersionPatch;
}

QLineEdit* SIOverviewDetails::ctrlName(void)
{
    return ui->editServiceName;
}

QRadioButton* SIOverviewDetails::ctrlPublic(void)
{
    return ui->radioPublic;
}

QRadioButton* SIOverviewDetails::ctrlPrivate(void)
{
    return ui->radioPrivate;
}

QRadioButton* SIOverviewDetails::ctrlInternet(void)
{
    return ui->radioInternet;
}

QPlainTextEdit* SIOverviewDetails::ctrlDescription(void)
{
    return ui->textDescribe;
}

QCheckBox* SIOverviewDetails::ctrlDeprecated(void)
{
    return ui->checkDeprecated;
}

QLineEdit* SIOverviewDetails::ctrlDeprecateHint(void)
{
    return ui->editDeprecated;
}
