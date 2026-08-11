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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIOverview.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIOverview.hpp"

#include "lusan/model/si/SIOverviewModel.hpp"
#include "lusan/view/si/ServiceInterface.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QSignalBlocker>

namespace
{
    //!< The row the category group takes in the details form, right under the name.
    constexpr int   CATEGORY_ROW    { 1 };

    int pageIndex(ServiceInterface::eSIPages page)
    {
        return static_cast<int>(page);
    }

    OverviewPageConfig makeConfig(void)
    {
        OverviewPageConfig config;
        config.headline         = QObject::tr("Service Interface Overview ...");
        config.versionTitle     = QObject::tr("Interface Version:");
        config.descriptionHint  = QObject::tr("Describe service interface here");
        // The name is the author's. The first save seeds it from the file name; after that the
        // two are free to differ, and the generated code follows the name, not the file.
        config.nameEditable     = true;
        config.links            =
        {
              { pageIndex(ServiceInterface::eSIPages::PageDataTypes) , QStringLiteral("linkDataTypes") , QObject::tr("Data Types ..."), QObject::tr("Click to open Interface Data Types page")      , QObject::tr("Open Service Interface Data Types Page ...")      }
            , { pageIndex(ServiceInterface::eSIPages::PageAttributes), QStringLiteral("linkAttributes"), QObject::tr("Attributes ..."), QObject::tr("Click to open Interface Attributes page")      , QObject::tr("Open Service Interface Data Attributes Page ...") }
            , { pageIndex(ServiceInterface::eSIPages::PageMethods)   , QStringLiteral("linkMethods")   , QObject::tr("Methods ...")   , QObject::tr("Click to open Interface Method page")          , QObject::tr("Open Service Interface Methods Page ...")         }
            , { pageIndex(ServiceInterface::eSIPages::PageConstants) , QStringLiteral("linkConstants") , QObject::tr("Constants ...") , QObject::tr("Click to open Interface available Constants")  , QObject::tr("Open Service Interface Constants Page ...")       }
            , { pageIndex(ServiceInterface::eSIPages::PageIncludes)  , QStringLiteral("linkIncludes")  , QObject::tr("Includes ...")  , QObject::tr("Click to open Interface additional Includes")  , QObject::tr("Open Service Interface Includes Page ...")        }
        };

        return config;
    }
}

SIOverview::SIOverview(SIOverviewModel& model, QWidget* parent /*= nullptr*/)
    : OverviewPage  (model, makeConfig(), parent)
    , mModel        (model)
    , mPublic       (nullptr)
    , mPrivate      (nullptr)
    , mInternet     (nullptr)
{
    buildCategoryRow();
    refreshAll();
}

void SIOverview::buildCategoryRow(void)
{
    QGroupBox* category = new QGroupBox(tr("Service Category:"), getDetailsGroup());
    category->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout* categoryLayout = new QHBoxLayout(category);
    mPublic = new QRadioButton(tr("Public"), category);
    mPublic->setObjectName(QStringLiteral("overviewCategoryPublic"));
    mPrivate = new QRadioButton(tr("Private"), category);
    mPrivate->setObjectName(QStringLiteral("overviewCategoryPrivate"));
    mInternet = new QRadioButton(tr("Internet"), category);
    mInternet->setObjectName(QStringLiteral("overviewCategoryInternet"));
    mInternet->setEnabled(false);
    categoryLayout->addWidget(mPublic);
    categoryLayout->addWidget(mPrivate);
    categoryLayout->addWidget(mInternet);
    categoryLayout->addStretch(1);
    getDetailsForm()->insertRow(CATEGORY_ROW, tr("Category:"), category);

    connect(mPublic , &QRadioButton::toggled, this, &SIOverview::onPublicToggled);
    connect(mPrivate, &QRadioButton::toggled, this, &SIOverview::onPrivateToggled);
}

void SIOverview::refreshAll(void)
{
    OverviewPage::refreshAll();
    if (mPublic == nullptr)
        return;

    const QSignalBlocker blockPublic(mPublic);
    const QSignalBlocker blockPrivate(mPrivate);
    const QSignalBlocker blockInternet(mInternet);

    switch (mModel.getCategory())
    {
    case SIOverviewData::eCategory::InterfacePublic:
        mPublic->setChecked(true);
        break;

    case SIOverviewData::eCategory::InterfaceInternet:
        // Not offered yet, but a document that declares it must be shown as it is.
        mInternet->setChecked(true);
        break;

    default:
        mPrivate->setChecked(true);
        break;
    }
}

void SIOverview::onPublicToggled(bool checked)
{
    if (checked == false)
        return;

    setCommitting(true);
    mModel.setCategory(SIOverviewData::eCategory::InterfacePublic);
    setCommitting(false);
}

void SIOverview::onPrivateToggled(bool checked)
{
    if (checked == false)
        return;

    setCommitting(true);
    mModel.setCategory(SIOverviewData::eCategory::InterfacePrivate);
    setCommitting(false);
}
