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
 *  \file        lusan/view/sm/SMOverview.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMOverview.hpp"

#include "lusan/model/sm/SMOverviewModel.hpp"
#include "lusan/view/sm/StateMachine.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QSignalBlocker>

namespace
{
    //!< The row the threading group takes in the details form, right under the name.
    constexpr int   THREADING_ROW   { 1 };

    OverviewPageConfig makeConfig(void)
    {
        OverviewPageConfig config;
        config.headline         = QObject::tr("State Machine Overview ...");
        config.versionTitle     = QObject::tr("Machine Version:");
        config.descriptionHint  = QObject::tr("Describe the state machine here");
        config.nameEditable     = true;
        config.links            =
        {
              { StateMachine::PageDataTypes , QStringLiteral("smLinkDataTypes") , QObject::tr("Data Types ...") , QObject::tr("Click to open the Data Types page") , QObject::tr("Declare enumerations, structures and imported types") }
            , { StateMachine::PageAttributes, QStringLiteral("smLinkAttributes"), QObject::tr("Attributes ...") , QObject::tr("Click to open the Attributes page")  , QObject::tr("Declare the machine's internal data variables")      }
            , { StateMachine::PageEvents    , QStringLiteral("smLinkEvents")    , QObject::tr("Events ...")     , QObject::tr("Click to open the Events page")      , QObject::tr("Declare events and timers")                          }
            , { StateMachine::PageMethods   , QStringLiteral("smLinkMethods")   , QObject::tr("Methods ...")    , QObject::tr("Click to open the Methods page")     , QObject::tr("Declare triggers, actions and conditions")           }
            , { StateMachine::PageConstants , QStringLiteral("smLinkConstants") , QObject::tr("Constants ...")  , QObject::tr("Click to open the Constants page")   , QObject::tr("Declare named typed literals")                       }
            , { StateMachine::PageIncludes  , QStringLiteral("smLinkIncludes")  , QObject::tr("Includes ...")   , QObject::tr("Click to open the Includes page")    , QObject::tr("Declare include files and imported machines")        }
            , { StateMachine::PageDesign    , QStringLiteral("smLinkDesign")    , QObject::tr("Design ...")     , QObject::tr("Click to open the Design page")      , QObject::tr("Edit the state graph on the canvas")                 }
        };

        return config;
    }
}

SMOverview::SMOverview(SMOverviewModel& model, QWidget* parent /*= nullptr*/)
    : OverviewPage  (model, makeConfig(), parent)
    , mModel        (model)
    , mShared       (nullptr)
    , mLocal        (nullptr)
{
    buildThreadingRow();
    refreshAll();
}

void SMOverview::buildThreadingRow(void)
{
    QGroupBox* threading = new QGroupBox(tr("Threading Mode:"), getDetailsGroup());
    threading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout* threadingLayout = new QHBoxLayout(threading);
    mShared = new QRadioButton(tr("Shared"), threading);
    mShared->setObjectName(QStringLiteral("overviewThreadingShared"));
    mShared->setToolTip(tr("The machine may be driven from more than one thread; generated code is thread-safe."));
    mLocal = new QRadioButton(tr("Local"), threading);
    mLocal->setObjectName(QStringLiteral("overviewThreadingLocal"));
    mLocal->setToolTip(tr("The whole machine is guaranteed to live in a single thread; generated code has no locking."));
    threadingLayout->addWidget(mShared);
    threadingLayout->addWidget(mLocal);
    threadingLayout->addStretch(1);
    getDetailsForm()->insertRow(THREADING_ROW, tr("Threading:"), threading);

    connect(mShared, &QRadioButton::toggled, this, &SMOverview::onThreadingToggled);
}

void SMOverview::refreshAll(void)
{
    OverviewPage::refreshAll();
    if (mShared == nullptr)
        return;

    const QSignalBlocker blockShared(mShared);
    const QSignalBlocker blockLocal(mLocal);
    if (mModel.getThreading() == SMOverviewData::eThreading::Local)
    {
        mLocal->setChecked(true);
    }
    else
    {
        mShared->setChecked(true);
    }
}

void SMOverview::onThreadingToggled(bool checked)
{
    setCommitting(true);
    mModel.setThreading(checked ? SMOverviewData::eThreading::Shared : SMOverviewData::eThreading::Local);
    setCommitting(false);
}
