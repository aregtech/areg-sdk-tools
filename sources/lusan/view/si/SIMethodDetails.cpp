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
 *  \file        lusan/view/si/SIMethodDetails.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIMethodDetails.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIMethodDetails.h"

SIMethodDetails::SIMethodDetails(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIMethodDetails)
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

QLineEdit * SIMethodDetails::ctrlName(void) const
{
    return ui->editName;
}

QRadioButton * SIMethodDetails::ctrlBroadcast(void) const
{
    return ui->radioBroadcast;
}

QRadioButton * SIMethodDetails::ctrlRequest(void) const
{
    return ui->radioRequest;
}

QRadioButton * SIMethodDetails::ctrlResponse(void) const
{
    return ui->radioResponse;
}

QComboBox * SIMethodDetails::ctrlConnectedResponse(void) const
{
    return  ui->comboReply;
}

QPlainTextEdit * SIMethodDetails::ctrlDescription(void) const
{
    return ui->textDescribe;
}

QCheckBox * SIMethodDetails::ctrlIsDeprecated(void) const
{
    return ui->checkDeprecated;
}

QLineEdit * SIMethodDetails::ctrlDeprecateHint(void) const
{
    return ui->editDeprecated;
}
