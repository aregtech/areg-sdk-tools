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
 *  \file        lusan/view/si/SIMethodDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface method editor.
 *
 ************************************************************************/
#include "lusan/view/si/SIMethodDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QVBoxLayout>

SIMethodDetails::SIMethodDetails(QWidget* parent)
    : QWidget           (parent)
    , mName             (nullptr)
    , mRequest          (nullptr)
    , mResponse         (nullptr)
    , mBroadcast        (nullptr)
    , mReply            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SIMethodDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    form->addRow(tr("Name:"), mName);

    QWidget* typeCell = new QWidget(details);
    QHBoxLayout* typeLayout = new QHBoxLayout(typeCell);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    mRequest   = new QRadioButton(tr("Request"), typeCell);
    mResponse  = new QRadioButton(tr("Response"), typeCell);
    mBroadcast = new QRadioButton(tr("Broadcast"), typeCell);
    typeLayout->addWidget(mRequest);
    typeLayout->addWidget(mResponse);
    typeLayout->addWidget(mBroadcast);
    typeLayout->addStretch(1);
    form->addRow(tr("Type:"), typeCell);

    mReply = new QComboBox(details);
    form->addRow(tr("Reply:"), mReply);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe method here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SIMethodDetails::ctrlName() const
{
    return mName;
}

QRadioButton* SIMethodDetails::ctrlBroadcast() const
{
    return mBroadcast;
}

QRadioButton* SIMethodDetails::ctrlRequest() const
{
    return mRequest;
}

QRadioButton* SIMethodDetails::ctrlResponse() const
{
    return mResponse;
}

QComboBox* SIMethodDetails::ctrlConnectedResponse() const
{
    return mReply;
}

QPlainTextEdit* SIMethodDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SIMethodDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SIMethodDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}
