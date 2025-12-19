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
 *  \file        lusan/view/si/SIOverview.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIOverview.hpp"
#include "ui/ui_SIOverview.h"

#include "lusan/model/si/SIOverviewModel.hpp"
#include "lusan/view/si/ServiceInterface.hpp"
#include "lusan/view/si/SIOverviewDetails.hpp"
#include "lusan/view/si/SIOverviewLinks.hpp"

#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>

#include "lusan/view/si/SICommon.hpp"

SIOverviewWidget::SIOverviewWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIOverview)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIOverview::SIOverview(SIOverviewModel& model, QWidget* parent)
    : QScrollArea       (parent)
    , mModel    (model)
    , mDetails  (new SIOverviewDetails(this))
    , mLinks    (new SIOverviewLinks(this))
    , mWidget   (new SIOverviewWidget(this))
    , ui        (*mWidget->ui)
    , mVersionValidator(0, 999999, this)
{
    ui.horizontalLayout->addWidget(mDetails);
    ui.horizontalLayout->addWidget(mLinks);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    
    setWidgetResizable(true);
    setWidget(mWidget);

    updateWidgets();
    updateData();
    setupSignals();
}

SIOverview::~SIOverview(void)
{
    ui.horizontalLayout->removeWidget(mLinks);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIOverview::setServiceInterfaceName(const QString & siName)
{
    mDetails->ctrlName()->setText(siName);
}
    
void SIOverview::onCheckedPublic(bool isChecked)
{
    if (isChecked)
    {
        mModel.setCategory(SIOverviewData::eCategory::InterfacePublic);
    }
}

void SIOverview::onCheckedPrivate(bool isChecked)
{
    if (isChecked)
    {
        mModel.setCategory(SIOverviewData::eCategory::InterfacePrivate);
    }
}

void SIOverview::onCheckedInternet(bool isChecked)
{
    if (isChecked)
    {
        mModel.setCategory(SIOverviewData::eCategory::InterfaceInternet);
    }
}

void SIOverview::onDeprecatedChecked(bool isChecked)
{
    SICommon::checkedDeprecated<SIOverviewDetails, SIOverviewModel>(mDetails, &mModel, isChecked);
}

void SIOverview::onDescriptionChanged(void)
{
    mModel.setDescription(mDetails->ctrlDescription()->toPlainText());
}

void SIOverview::onDeprecateHintChanged(const QString& newText)
{
    SICommon::setDeprecateHint<SIOverviewDetails, SIOverviewModel>(mDetails, &mModel, newText);
}

void SIOverview::onMajorChanged(const QString& major)
{
    mModel.setVersion(major.toUInt(), mDetails->ctrlMinor()->text().toUInt(), mDetails->ctrlPatch()->text().toUInt());
}

void SIOverview::onMinorChanged(const QString& minor)
{
    mModel.setVersion(mDetails->ctrlMajor()->text().toUInt(), minor.toUInt(),  mDetails->ctrlPatch()->text().toUInt());
}

void SIOverview::onPatchChanged(const QString& patch)
{
    mModel.setVersion(mDetails->ctrlMajor()->text().toUInt(), mDetails->ctrlMinor()->text().toUInt(), patch.toUInt());
}

void SIOverview::onLinkConstantsClicked(bool /*checked*/)
{
    emit signalPageLinkClicked(static_cast<int>(ServiceInterface::eSIPages::PageConstants));
}

void SIOverview::onLinkDataTypesClicked(bool /*checked*/)
{
    emit signalPageLinkClicked(static_cast<int>(ServiceInterface::eSIPages::PageDataTypes));
}

void SIOverview::onLinkIncludesClicked(bool /*checked*/)
{
    emit signalPageLinkClicked(static_cast<int>(ServiceInterface::eSIPages::PageIncludes));
}

void SIOverview::onLinkMethodsClicked(bool /*checked*/)
{
    emit signalPageLinkClicked(static_cast<int>(ServiceInterface::eSIPages::PageMethods));
}

void SIOverview::onLinkAttributesClicked(bool /*checked*/)
{
    emit signalPageLinkClicked(static_cast<int>(ServiceInterface::eSIPages::PageAttributes));
}

void SIOverview::updateWidgets(void)
{
    mDetails->ctrlMajor()->setValidator(&mVersionValidator);
    mDetails->ctrlMinor()->setValidator(&mVersionValidator);
    mDetails->ctrlPatch()->setValidator(&mVersionValidator);
    mDetails->ctrlName()->setReadOnly(true);
    mDetails->ctrlInternet()->setEnabled(false);
}

void SIOverview::updateData(void)
{
    const VersionNumber& version{ mModel.getVersion() };

    mDetails->ctrlMajor()->setText(QString::number(version.getMajor()));
    mDetails->ctrlMinor()->setText(QString::number(version.getMinor()));
    mDetails->ctrlPatch()->setText(QString::number(version.getPatch()));
    mDetails->ctrlName()->setText(mModel.getName());
    mDetails->ctrlDescription()->setPlainText(mModel.getDescription());

    SICommon::enableDeprecated<SIOverviewDetails, SIOverviewModel>(mDetails, &mModel, true);

    switch (mModel.getCategory())
    {
    case SIOverviewData::eCategory::InterfacePrivate:
        mDetails->ctrlPrivate()->setChecked(true);
        break;

    case SIOverviewData::eCategory::InterfacePublic:
        mDetails->ctrlPublic()->setChecked(true);
        break;

    case SIOverviewData::eCategory::InterfaceInternet:
        Q_ASSERT(false);
        mDetails->ctrlPublic()->setChecked(true);
        break;

    default:
        mDetails->ctrlPrivate()->setChecked(true);
        break;
    }
}

void SIOverview::setupSignals(void)
{
    connect(mDetails->ctrlMajor()       , &QLineEdit::textEdited    , this, &SIOverview::onMajorChanged);
    connect(mDetails->ctrlMinor()       , &QLineEdit::textEdited    , this, &SIOverview::onMinorChanged);
    connect(mDetails->ctrlPatch()       , &QLineEdit::textEdited    , this, &SIOverview::onPatchChanged);
    connect(mDetails->ctrlPublic()      , &QRadioButton::toggled    , this, &SIOverview::onCheckedPublic);
    connect(mDetails->ctrlPrivate()     , &QRadioButton::toggled    , this, &SIOverview::onCheckedPrivate);
    connect(mDetails->ctrlInternet()    , &QRadioButton::toggled    , this, &SIOverview::onCheckedInternet);
    connect(mDetails->ctrlDeprecated()  , &QCheckBox::toggled       , this, &SIOverview::onDeprecatedChecked);
    connect(mDetails->ctrlDeprecateHint(),&QLineEdit::textEdited    , this, &SIOverview::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription() , &QPlainTextEdit::textChanged, this, &SIOverview::onDescriptionChanged);

    connect(mLinks->linkConstants()     , &QPushButton::clicked     , this, &SIOverview::onLinkConstantsClicked);
    connect(mLinks->linkDataTypes()     , &QPushButton::clicked     , this, &SIOverview::onLinkDataTypesClicked);
    connect(mLinks->linkIncludes()      , &QPushButton::clicked     , this, &SIOverview::onLinkIncludesClicked);
    connect(mLinks->linkMethods()       , &QPushButton::clicked     , this, &SIOverview::onLinkMethodsClicked);
    connect(mLinks->linkAttributes()    , &QPushButton::clicked     , this, &SIOverview::onLinkAttributesClicked);
}
