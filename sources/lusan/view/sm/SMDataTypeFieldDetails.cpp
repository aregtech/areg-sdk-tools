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
 *  \file        lusan/view/sm/SMDataTypeFieldDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page — structure/enumeration field editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDataTypeFieldDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

SMDataTypeFieldDetails::SMDataTypeFieldDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mType             (nullptr)
    , mValue            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
    , mFieldForm        (nullptr)
    , mTypeValueForm    (nullptr)
{
    buildUi();
}

void SMDataTypeFieldDetails::buildUi(void)
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QVBoxLayout* detailsLayout = new QVBoxLayout(details);
    mFieldForm = new QFormLayout();
    mFieldForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    mFieldForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    mFieldForm->addRow(tr("Name:"), mName);

    QGroupBox* typeValue = new QGroupBox(tr("Field Type && Value Details:"), details);
    typeValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mTypeValueForm = new QFormLayout(typeValue);
    mType = new QComboBox(typeValue);
    mTypeValueForm->addRow(tr("Type:"), mType);
    mValue = new QLineEdit(typeValue);
    mTypeValueForm->addRow(tr("Value:"), mValue);
    mFieldForm->addRow(tr("Field:"), typeValue);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe field here"));
    mFieldForm->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    mFieldForm->addRow(mDeprecated, mDeprecateHint);

    detailsLayout->addLayout(mFieldForm);
    root->addWidget(details);
}

QLineEdit* SMDataTypeFieldDetails::ctrlName(void) const
{
    return mName;
}

QComboBox* SMDataTypeFieldDetails::ctrlTypes(void) const
{
    return mType;
}

QLineEdit* SMDataTypeFieldDetails::ctrlValue(void) const
{
    return mValue;
}

QPlainTextEdit* SMDataTypeFieldDetails::ctrlDescription(void) const
{
    return mDescription;
}

QCheckBox* SMDataTypeFieldDetails::ctrlDeprecated(void) const
{
    return mDeprecated;
}

QLineEdit* SMDataTypeFieldDetails::ctrlDeprecateHint(void) const
{
    return mDeprecateHint;
}

void SMDataTypeFieldDetails::setTypeRowVisible(bool visible)
{
    mTypeValueForm->setRowVisible(mType, visible);
}
