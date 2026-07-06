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
 *  \file        lusan/view/si/SIMethodParamDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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

QLineEdit * SIMethodParamDetails::ctrlParamName() const
{
    return ui->editParamName;
}

QComboBox * SIMethodParamDetails::ctrlParamTypes() const
{
    return ui->comboParamType;
}

QCheckBox* SIMethodParamDetails::ctrlParamHasDefault() const
{
    return ui->checkDefaultValue;
}

QLineEdit * SIMethodParamDetails::ctrlParamDefaultValue() const
{
    return ui->editDefaultValue;
}

QPlainTextEdit * SIMethodParamDetails::ctrlParamDescription() const
{
    return ui->textParamDescribe;
}

QCheckBox * SIMethodParamDetails::ctrlDeprecated() const
{
    return ui->checkDeprecated;
}

QLineEdit * SIMethodParamDetails::ctrlDeprecateHint() const
{
    return ui->editDeprecated;
}
