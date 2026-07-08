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
 *  \file        lusan/view/si/SIIncludeDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface include file editor.
 *
 ************************************************************************/
#include "lusan/view/si/SIIncludeDetails.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

SIIncludeDetails::SIIncludeDetails(QWidget* parent)
    : QWidget           (parent)
    , mInclude          (nullptr)
    , mBrowse           (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

SIIncludeDetails::~SIIncludeDetails()
{
}

void SIIncludeDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    QWidget* includeCell = new QWidget(details);
    QHBoxLayout* includeLayout = new QHBoxLayout(includeCell);
    includeLayout->setContentsMargins(0, 0, 0, 0);
    includeLayout->setSpacing(4);
    mInclude = new QLineEdit(includeCell);
    mBrowse = new QPushButton(tr("Browse ..."), includeCell);
    includeLayout->addWidget(mInclude, 1);
    includeLayout->addWidget(mBrowse);
    form->addRow(tr("Include:"), includeCell);

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

QString SIIncludeDetails::getSelectedFile() const
{
    return mInclude->text();
}

QString SIIncludeDetails::getDescription() const
{
    return mDescription->toPlainText();
}

bool SIIncludeDetails::isDeprecated() const
{
    return mDeprecated->isChecked();
}

QString SIIncludeDetails::getDeprecateHint() const
{
    return mDeprecateHint->text();
}

QLineEdit * SIIncludeDetails::ctrlInclude()
{
    return mInclude;
}

QLineEdit * SIIncludeDetails::ctrlDeprecateHint()
{
    return mDeprecateHint;
}

QCheckBox * SIIncludeDetails::ctrlDeprecated()
{
    return mDeprecated;
}

QPlainTextEdit * SIIncludeDetails::ctrlDescription()
{
    return mDescription;
}

QPushButton * SIIncludeDetails::ctrlBrowseButton()
{
    return mBrowse;
}
