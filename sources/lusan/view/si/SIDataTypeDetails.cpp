/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIDataTypeDetails.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIDataTypeDetails.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataTypeDetails.h"

SIDataTypeDetails::SIDataTypeDetails(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SIDataTypeDetails)
{
    QFont font{this->font()};
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
}


QLineEdit* SIDataTypeDetails::ctrlName(void) const
{
    return ui->editName;
}

QRadioButton* SIDataTypeDetails::ctrlTypeStruct(void) const
{
    return ui->radioTypeStruct;
}

QRadioButton* SIDataTypeDetails::ctrlTypeEnum(void) const
{
    return ui->radioTypeEnum;
}

QRadioButton* SIDataTypeDetails::ctrlTypeImport(void) const
{
    return ui->radioTypeImport;
}

QRadioButton* SIDataTypeDetails::ctrlTypeContainer(void) const
{
    return ui->radioTypeContainer;
}

QComboBox* SIDataTypeDetails::ctrlContainerObject(void) const
{
    return ui->comboContainerObject;
}

QComboBox* SIDataTypeDetails::ctrlContainerKey(void) const
{
    return ui->comboContainerKey;
}

QComboBox* SIDataTypeDetails::ctrlContainerValue(void) const
{
    return ui->comboContainerValue;
}

QPlainTextEdit* SIDataTypeDetails::ctrlDescription(void) const
{
    return ui->textDescribe;
}

QCheckBox* SIDataTypeDetails::ctrlDeprecated(void) const
{
    return ui->checkDeprecated;
}

QLineEdit* SIDataTypeDetails::ctrlDeprecateHint(void) const
{
    return ui->editDeprecated;
}

QComboBox* SIDataTypeDetails::ctrlEnumDerived(void) const
{
    return ui->comboEnumDerive;
}

QLineEdit* SIDataTypeDetails::ctrlImportLocation(void) const
{
    return ui->editImportInclude;
}

QPushButton* SIDataTypeDetails::ctrlButtonBrowse(void) const
{
    return ui->buttonImportInclude;
}

QLineEdit* SIDataTypeDetails::ctrlImportNamespace(void) const
{
    return ui->editImportNamespace;
}

QLineEdit* SIDataTypeDetails::ctrlImportObject(void) const
{
    return ui->editImportObject;
}

SIDataTypeDetails::CtrlGroup SIDataTypeDetails::ctrlDetailsEnum(void) const
{
    return CtrlGroup{ui->labelEnum, ui->groupEnum};
}

SIDataTypeDetails::CtrlGroup SIDataTypeDetails::ctrlDetailsImport(void) const
{
    return CtrlGroup{ui->labelImport, ui->groupImport};
}

SIDataTypeDetails::CtrlGroup SIDataTypeDetails::ctrlDetailsContainer(void) const
{
    return CtrlGroup{ui->labelContainer, ui->groupContainer};
}

QFormLayout* SIDataTypeDetails::ctrlLayout(void) const
{
    return ui->formLayout;
}

QSpacerItem* SIDataTypeDetails::ctrlSpacer1(void) const
{
    return ui->verticalSpacer1;
}

QSpacerItem* SIDataTypeDetails::ctrlSpacer2(void) const
{
    return ui->verticalSpacer2;
}

SIDataTypeDetails::SpaceItem SIDataTypeDetails::ctrlSpacer(void) const
{
    return SpaceItem{ ui->verticalSpacer1, ui->verticalSpacer2 };
}

void SIDataTypeDetails::setSpace(int newHeight)
{
    newHeight = newHeight > 0 ? newHeight : 1;
    ui->verticalSpacer1->changeSize(20, newHeight, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
    ui->verticalSpacer2->changeSize(20, newHeight, QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
}

void SIDataTypeDetails::changeSpace(int delta)
{
    int newHeight = ui->verticalSpacer1->minimumSize().height();
    setSpace(newHeight + delta);
}
