/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIOverviewDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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

QLineEdit* SIOverviewDetails::ctrlMajor()
{
    return ui->editVersionMajor;
}

QLineEdit* SIOverviewDetails::ctrlMinor()
{
    return ui->editVersionMinor;
}

QLineEdit* SIOverviewDetails::ctrlPatch()
{
    return ui->editVersionPatch;
}

QLineEdit* SIOverviewDetails::ctrlName()
{
    return ui->editServiceName;
}

QRadioButton* SIOverviewDetails::ctrlPublic()
{
    return ui->radioPublic;
}

QRadioButton* SIOverviewDetails::ctrlPrivate()
{
    return ui->radioPrivate;
}

QRadioButton* SIOverviewDetails::ctrlInternet()
{
    return ui->radioInternet;
}

QPlainTextEdit* SIOverviewDetails::ctrlDescription()
{
    return ui->textDescribe;
}

QCheckBox* SIOverviewDetails::ctrlDeprecated()
{
    return ui->checkDeprecated;
}

QLineEdit* SIOverviewDetails::ctrlDeprecateHint()
{
    return ui->editDeprecated;
}
