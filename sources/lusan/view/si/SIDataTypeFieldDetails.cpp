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
 *  \file        lusan/view/si/SIDataTypeFieldDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface data type field editor.
 *
 ************************************************************************/
#include "lusan/view/si/SIDataTypeFieldDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

SIDataTypeFieldDetails::SIDataTypeFieldDetails(QWidget* parent)
    : QWidget           (parent)
    , mName             (nullptr)
    , mType             (nullptr)
    , mValue            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SIDataTypeFieldDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    form->addRow(tr("Name:"), mName);

    mType = new QComboBox(details);
    form->addRow(tr("Type:"), mType);

    mValue = new QLineEdit(details);
    form->addRow(tr("Value:"), mValue);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe field here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SIDataTypeFieldDetails::ctrlName() const
{
    return mName;
}

QComboBox* SIDataTypeFieldDetails::ctrlTypes() const
{
    return mType;
}

QLineEdit* SIDataTypeFieldDetails::ctrlValue() const
{
    return mValue;
}

QPlainTextEdit* SIDataTypeFieldDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SIDataTypeFieldDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SIDataTypeFieldDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}
