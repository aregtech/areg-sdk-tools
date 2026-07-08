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
 *  \file        lusan/view/sm/SMIncludeDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page — selected include editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMIncludeDetails.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

SMIncludeDetails::SMIncludeDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mInclude          (nullptr)
    , mBrowse           (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SMIncludeDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    QWidget* locationCell = new QWidget(details);
    QHBoxLayout* locationLayout = new QHBoxLayout(locationCell);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    locationLayout->setSpacing(4);
    mInclude = new QLineEdit(locationCell);
    mBrowse = new QPushButton(tr("Browse..."), locationCell);
    locationLayout->addWidget(mInclude, 1);
    locationLayout->addWidget(mBrowse, 0);
    form->addRow(tr("Include File:"), locationCell);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe include here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SMIncludeDetails::ctrlInclude() const
{
    return mInclude;
}

QPushButton* SMIncludeDetails::ctrlBrowseButton() const
{
    return mBrowse;
}

QPlainTextEdit* SMIncludeDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SMIncludeDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SMIncludeDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}
