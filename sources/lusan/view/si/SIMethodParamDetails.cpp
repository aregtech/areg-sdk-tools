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
 *  \file        lusan/view/si/SIMethodParamDetails.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIMethodParamDetails.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIMethodParamDetails.h"

SIMethodParamDetails::SIMethodParamDetails(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIMethodParamDetails)
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

QLineEdit * SIMethodParamDetails::ctrlParamName(void) const
{
    return ui->editParamName;
}

QComboBox * SIMethodParamDetails::ctrlParamType(void) const
{
    return ui->comboParamType;
}

QCheckBox* SIMethodParamDetails::ctrlParamHasDefault(void) const
{
    return ui->checkDefaultValue;
}

QLineEdit * SIMethodParamDetails::ctrlParamDefaultValue(void) const
{
    return ui->editDefaultValue;
}

QPlainTextEdit * SIMethodParamDetails::ctrlParamDescription(void) const
{
    return ui->textParamDescribe;
}

QCheckBox * SIMethodParamDetails::ctrlDeprecated(void) const
{
    return ui->checkDeprecated;
}

QLineEdit * SIMethodParamDetails::ctrlDeprecateHint(void) const
{
    return ui->editDeprecated;
}
