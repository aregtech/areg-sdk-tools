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


SIOverviewDetails::SIOverviewDetails(SIOverviewModel& model, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIOverviewDetails)
    , mModel(model)
    , mVersionValidator(0, 999999, this)
{
    QFont font{ this->font() };
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    
    // Set up validators for QLineEdit objects to accept only integer values
    ui->editVersionMajor->setValidator(&mVersionValidator);
    ui->editVersionMinor->setValidator(&mVersionValidator);
    ui->editVersionPatch->setValidator(&mVersionValidator);
    
    ui->editServiceName->setText(model.getName());
    ui->editServiceName->setReadOnly(true);
    ui->radioInternet->setEnabled(false);
    switch(model.getCategory())
    {
    case SIOverviewData::eCategory::InterfacePrivate:
        ui->radioPrivate->setChecked(true);
        break;
    
    case SIOverviewData::eCategory::InterfacePublic:
        ui->radioPublic->setChecked(true);
        break;
    
    case SIOverviewData::eCategory::InterfaceInternet:
        Q_ASSERT(false);
        ui->radioPublic->setChecked(true);
        break;
        
    default:        
        ui->radioPrivate->setChecked(true);
        break;
    }
    
    const VersionNumber& version {model.getVersion()};
    ui->editVersionMajor->setText(QString::number(version.getMajor()));
    ui->editVersionMinor->setText(QString::number(version.getMinor()));
    ui->editVersionPatch->setText(QString::number(version.getPatch()));
    
    ui->textDescribe->setPlainText(model.getDescription());
    ui->checkDeprecated->setChecked(model.isDeprecated());
    ui->editDeprecated->setText(model.getDeprecateHint());    
}

void SIOverviewDetails::saveData(void) const
{
    
    uint32_t major = ui->editVersionMajor->text().toUInt();
    uint32_t minor = ui->editVersionMinor->text().toUInt();
    uint32_t patch = ui->editVersionPatch->text().toUInt();
    mModel.setVersion(major, minor, patch);
    
    if (ui->radioPrivate->isChecked())
    {
        mModel.setCategory(SIOverviewData::eCategory::InterfacePrivate);
    }
    else if (ui->radioPublic->isChecked())
    {
        mModel.setCategory(SIOverviewData::eCategory::InterfacePublic);
    }
    else
    {
        mModel.setCategory(SIOverviewData::eCategory::InterfaceUnknown);
    }
    
    mModel.setDescription(ui->textDescribe->toPlainText());
    mModel.setIsDeprecated(ui->checkDeprecated->isChecked());
    mModel.setDeprecateHint(ui->editDeprecated->text());
}
