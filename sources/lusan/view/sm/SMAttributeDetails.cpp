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
 *  \file        lusan/view/sm/SMAttributeDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Attributes page — selected attribute editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMAttributeDetails.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QStringListModel>
#include <QVBoxLayout>

SMAttributeDetails::SMAttributeDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mTypes            (nullptr)
    , mValue            (nullptr)
    , mValueCompleter   (nullptr)
    , mValueChoices     (nullptr)
    , mValueHint        (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SMAttributeDetails::buildUi()
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
    mValue = new QLineEdit(valueCell);
    mValueChoices = new QStringListModel(mValue);
    mValueCompleter = new QCompleter(mValueChoices, mValue);
    mValueCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    // Unfiltered popup shows every enumerator once the user starts typing, replacing the
    // picker role of the former editable combo box.
    mValueCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mValue->setCompleter(mValueCompleter);
    valueCellLayout->addWidget(mValue);
    mValueHint = new QLabel(valueCell);
    mValueHint->setStyleSheet(QStringLiteral("color: #c0392b;"));
    mValueHint->setWordWrap(true);
    mValueHint->setVisible(false);
    valueCellLayout->addWidget(mValueHint);
    form->addRow(tr("Value:"), valueCell);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe attribute here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);
}

QLineEdit* SMAttributeDetails::ctrlName() const
{
    return mName;
}

QComboBox* SMAttributeDetails::ctrlTypes() const
{
    return mTypes;
}

QLineEdit* SMAttributeDetails::ctrlValue() const
{
    return mValue;
}

QPlainTextEdit* SMAttributeDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SMAttributeDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SMAttributeDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void SMAttributeDetails::showValueHint(const QString& reason)
{
    mValueHint->setText(reason);
    mValueHint->setVisible(reason.isEmpty() == false);
    mValue->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}

void SMAttributeDetails::setValueChoices(const QStringList& choices)
{
    mValueChoices->setStringList(choices);
}
