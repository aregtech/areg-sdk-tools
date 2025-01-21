﻿/************************************************************************
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
 *  \file        lusan/view/si/SIDataTypeFieldDetails.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIDataTypeFieldDetails.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataTypeFieldDetails.h"

SIDataTypeFieldDetails::SIDataTypeFieldDetails(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIDataTypeFieldDetails)
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

QLineEdit* SIDataTypeFieldDetails::ctrlName(void) const
{
    return ui->editFieldName;
}

QComboBox* SIDataTypeFieldDetails::ctrlTypes(void) const
{
    return ui->comboFieldType;
}

QLineEdit* SIDataTypeFieldDetails::ctrlValue(void) const
{
    return ui->editFieldValue;
}

QPlainTextEdit* SIDataTypeFieldDetails::ctrlDescription(void) const
{
    return ui->textDescribe;
}

QCheckBox* SIDataTypeFieldDetails::ctrlDeprecated(void) const
{
    return ui->checkDeprecated;
}

QLineEdit* SIDataTypeFieldDetails::ctrlDeprecateHint(void) const
{
    return ui->editDeprecated;
}
