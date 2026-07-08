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
 *  \file        lusan/view/si/SIOverviewDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface overview details editor.
 *
 ************************************************************************/
#include "lusan/view/si/SIOverviewDetails.hpp"
#include "lusan/model/si/SIOverviewModel.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QVBoxLayout>

namespace
{
    QLineEdit* makeVersionEdit(QWidget* parent)
    {
        QLineEdit* edit = new QLineEdit(parent);
        edit->setLayoutDirection(Qt::RightToLeft);
        edit->setInputMask(QStringLiteral("99999"));
        edit->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
        return edit;
    }
}

SIOverviewDetails::SIOverviewDetails(QWidget* parent)
    : QWidget           (parent)
    , mName             (nullptr)
    , mPublic           (nullptr)
    , mPrivate          (nullptr)
    , mInternet         (nullptr)
    , mMajor            (nullptr)
    , mMinor            (nullptr)
    , mPatch            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SIOverviewDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details :"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    mName->setReadOnly(true);
    form->addRow(tr("Name:"), mName);

    // Category
    QGroupBox* categoryGroup = new QGroupBox(tr("Service Category:"), details);
    categoryGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout* categoryLayout = new QHBoxLayout(categoryGroup);
    mPublic   = new QRadioButton(tr("Public"), categoryGroup);
    mPrivate  = new QRadioButton(tr("Private"), categoryGroup);
    mInternet = new QRadioButton(tr("Internet"), categoryGroup);
    mInternet->setEnabled(false);
    categoryLayout->addWidget(mPublic);
    categoryLayout->addWidget(mPrivate);
    categoryLayout->addWidget(mInternet);
    categoryLayout->addStretch(1);
    form->addRow(tr("Category:"), categoryGroup);

    // Version
    QGroupBox* versionGroup = new QGroupBox(tr("Interface Version:"), details);
    QGridLayout* versionGrid = new QGridLayout(versionGroup);
    QLabel* majorLabel = new QLabel(tr("Major"), versionGroup);
    QLabel* minorLabel = new QLabel(tr("Minor"), versionGroup);
    QLabel* patchLabel = new QLabel(tr("Patch"), versionGroup);
    mMajor = makeVersionEdit(versionGroup);
    mMinor = makeVersionEdit(versionGroup);
    mPatch = makeVersionEdit(versionGroup);
    QLabel* dot1 = new QLabel(QStringLiteral("."), versionGroup);
    QLabel* dot2 = new QLabel(QStringLiteral("."), versionGroup);
    versionGrid->addWidget(majorLabel, 0, 0);
    versionGrid->addWidget(minorLabel, 0, 2);
    versionGrid->addWidget(patchLabel, 0, 4);
    versionGrid->addWidget(mMajor, 1, 0);
    versionGrid->addWidget(dot1  , 1, 1);
    versionGrid->addWidget(mMinor, 1, 2);
    versionGrid->addWidget(dot2  , 1, 3);
    versionGrid->addWidget(mPatch, 1, 4);
    form->addRow(tr("Version:"), versionGroup);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe service interface here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SIOverviewDetails::ctrlMajor()
{
    return mMajor;
}

QLineEdit* SIOverviewDetails::ctrlMinor()
{
    return mMinor;
}

QLineEdit* SIOverviewDetails::ctrlPatch()
{
    return mPatch;
}

QLineEdit* SIOverviewDetails::ctrlName()
{
    return mName;
}

QRadioButton* SIOverviewDetails::ctrlPublic()
{
    return mPublic;
}

QRadioButton* SIOverviewDetails::ctrlPrivate()
{
    return mPrivate;
}

QRadioButton* SIOverviewDetails::ctrlInternet()
{
    return mInternet;
}

QPlainTextEdit* SIOverviewDetails::ctrlDescription()
{
    return mDescription;
}

QCheckBox* SIOverviewDetails::ctrlDeprecated()
{
    return mDeprecated;
}

QLineEdit* SIOverviewDetails::ctrlDeprecateHint()
{
    return mDeprecateHint;
}
