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
 *  \file        lusan/view/sm/SMEventParamDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — event payload parameter editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMEventParamDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

SMEventParamDetails::SMEventParamDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mNameHint         (nullptr)
    , mType             (nullptr)
    , mHasDefault       (nullptr)
    , mValue            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SMEventParamDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    QWidget* nameCell = new QWidget(details);
    QVBoxLayout* nameCellLayout = new QVBoxLayout(nameCell);
    nameCellLayout->setContentsMargins(0, 0, 0, 0);
    nameCellLayout->setSpacing(2);
    mName = new QLineEdit(nameCell);
    nameCellLayout->addWidget(mName);
    mNameHint = new QLabel(nameCell);
    mNameHint->setStyleSheet(QStringLiteral("color: #c0392b;"));
    mNameHint->setWordWrap(true);
    mNameHint->setVisible(false);
    nameCellLayout->addWidget(mNameHint);
    form->addRow(tr("Name:"), nameCell);

    mType = new QComboBox(details);
    form->addRow(tr("Type:"), mType);

    QWidget* defaultCell = new QWidget(details);
    QHBoxLayout* defaultCellLayout = new QHBoxLayout(defaultCell);
    defaultCellLayout->setContentsMargins(0, 0, 0, 0);
    mHasDefault = new QCheckBox(defaultCell);
    mValue = new QLineEdit(defaultCell);
    defaultCellLayout->addWidget(mHasDefault);
    defaultCellLayout->addWidget(mValue, 1);
    form->addRow(tr("Default:"), defaultCell);

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

QLineEdit* SMEventParamDetails::ctrlName() const
{
    return mName;
}

QComboBox* SMEventParamDetails::ctrlTypes() const
{
    return mType;
}

QCheckBox* SMEventParamDetails::ctrlHasDefault() const
{
    return mHasDefault;
}

QLineEdit* SMEventParamDetails::ctrlValue() const
{
    return mValue;
}

QPlainTextEdit* SMEventParamDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SMEventParamDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SMEventParamDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void SMEventParamDetails::showNameHint(const QString& reason)
{
    mNameHint->setText(reason);
    mNameHint->setVisible(reason.isEmpty() == false);
    mName->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}
