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
 *  \file        lusan/view/si/SIMethodParamDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface method parameter editor.
 *
 ************************************************************************/
#include "lusan/view/si/SIMethodParamDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

SIMethodParamDetails::SIMethodParamDetails(QWidget* parent)
    : QWidget           (parent)
    , mName             (nullptr)
    , mType             (nullptr)
    , mHasDefault       (nullptr)
    , mValue            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SIMethodParamDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Parameter Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    form->addRow(tr("Name:"), mName);

    mType = new QComboBox(details);
    form->addRow(tr("Type:"), mType);

    QWidget* valueCell = new QWidget(details);
    QHBoxLayout* valueLayout = new QHBoxLayout(valueCell);
    valueLayout->setContentsMargins(0, 0, 0, 0);
    mHasDefault = new QCheckBox(valueCell);
    mValue = new QLineEdit(valueCell);
    valueLayout->addWidget(mHasDefault);
    valueLayout->addWidget(mValue, 1);
    form->addRow(tr("Value:"), valueCell);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe parameter here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SIMethodParamDetails::ctrlParamName() const
{
    return mName;
}

QComboBox* SIMethodParamDetails::ctrlParamTypes() const
{
    return mType;
}

QCheckBox* SIMethodParamDetails::ctrlParamHasDefault() const
{
    return mHasDefault;
}

QLineEdit* SIMethodParamDetails::ctrlParamDefaultValue() const
{
    return mValue;
}

QPlainTextEdit* SIMethodParamDetails::ctrlParamDescription() const
{
    return mDescription;
}

QCheckBox* SIMethodParamDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SIMethodParamDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}
