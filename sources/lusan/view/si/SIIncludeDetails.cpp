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
 *  \file        lusan/view/si/SIIncludeDetails.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIIncludeDetails.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIIncludeDetails.h"

SIIncludeDetails::SIIncludeDetails(QWidget* parent)
    : QWidget   (parent)
    , ui        (new Ui::SIIncludeDetails)
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

SIIncludeDetails::~SIIncludeDetails(void)
{
    delete ui;
    ui = nullptr;
}

QString SIIncludeDetails::getSelectedFile(void) const
{
    return ui->editInclude->text();
}

QString SIIncludeDetails::getDescription(void) const
{
    return ui->textDescribe->toPlainText();
}

bool SIIncludeDetails::isDeprecated(void) const
{
    return ui->checkDeprecated->isChecked();
}

QString SIIncludeDetails::getDeprecateHint(void) const
{
    return ui->editDeprecated->text();
}

QLineEdit * SIIncludeDetails::ctrlInclude(void)
{
    return ui->editInclude;
}

QLineEdit * SIIncludeDetails::ctrlDeprecateHint(void)
{
    return ui->editDeprecated;
}

QCheckBox * SIIncludeDetails::ctrlDeprecated(void)
{
    return ui->checkDeprecated;
}

QPlainTextEdit * SIIncludeDetails::ctrlDescription(void)
{
    return ui->textDescribe;
}

QPushButton * SIIncludeDetails::ctrlBrowseButton(void)
{
    return ui->buttonBrowse;
}
