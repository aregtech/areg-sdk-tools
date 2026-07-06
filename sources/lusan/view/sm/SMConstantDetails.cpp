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
 *  \file        lusan/view/sm/SMConstantDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page — selected constant editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMConstantDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

SMConstantDetails::SMConstantDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mTypes            (nullptr)
    , mValue            (nullptr)
    , mValueHint        (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SMConstantDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    form->addRow(tr("Name:"), mName);

    mTypes = new QComboBox(details);
    form->addRow(tr("Type:"), mTypes);

    QWidget* valueCell = new QWidget(details);
    QVBoxLayout* valueCellLayout = new QVBoxLayout(valueCell);
    valueCellLayout->setContentsMargins(0, 0, 0, 0);
    valueCellLayout->setSpacing(2);
    mValue = new QComboBox(valueCell);
    mValue->setEditable(true);
    valueCellLayout->addWidget(mValue);
    mValueHint = new QLabel(valueCell);
    mValueHint->setStyleSheet(QStringLiteral("color: #c0392b;"));
    mValueHint->setWordWrap(true);
    mValueHint->setVisible(false);
    valueCellLayout->addWidget(mValueHint);
    form->addRow(tr("Value:"), valueCell);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe constant here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SMConstantDetails::ctrlName() const
{
    return mName;
}

QComboBox* SMConstantDetails::ctrlTypes() const
{
    return mTypes;
}

QComboBox* SMConstantDetails::ctrlValue() const
{
    return mValue;
}

QPlainTextEdit* SMConstantDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SMConstantDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SMConstantDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void SMConstantDetails::showValueHint(const QString& reason)
{
    mValueHint->setText(reason);
    mValueHint->setVisible(reason.isEmpty() == false);
    mValue->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}
