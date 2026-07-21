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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/MethodParamDetailsView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method parameter details" editor implementation.
 *
 ************************************************************************/

#include "lusan/view/common/MethodParamDetailsView.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/EditCancelFilter.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

MethodParamDetailsView::MethodParamDetailsView(const QString& title, QWidget* parent /*= nullptr*/)
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
    buildUi(title);
}

void MethodParamDetailsView::buildUi(const QString& title)
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(title, this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Name with an inline collision hint below it (used by the State Machine editor; the
    // Service Interface editor simply never sets a hint, leaving it hidden).
    QWidget* nameCell = new QWidget(details);
    QVBoxLayout* nameCellLayout = new QVBoxLayout(nameCell);
    nameCellLayout->setContentsMargins(0, 0, 0, 0);
    nameCellLayout->setSpacing(2);
    mName = new QLineEdit(nameCell);
    mName->setValidator(NELusanCommon::createIdentifierValidator(mName));
    nameCellLayout->addWidget(mName);
    mNameHint = new QLabel(nameCell);
    mNameHint->setStyleSheet(QStringLiteral("color: #c0392b;"));
    mNameHint->setWordWrap(true);
    mNameHint->setVisible(false);
    nameCellLayout->addWidget(mNameHint);
    form->addRow(tr("Name:"), nameCell);

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

    connect(mName, &QLineEdit::textChanged, this, &MethodParamDetailsView::nameEdited);

    // Escape cancels the edit: the live-synced text fields restore their pre-edit value. The
    // Type combo commits immediately; the multi-line Description keeps its focus-out behavior.
    EditCancelFilter::install(mName);
    EditCancelFilter::install(mValue);
    EditCancelFilter::install(mDeprecateHint);
}

QLineEdit* MethodParamDetailsView::ctrlName() const
{
    return mName;
}

QComboBox* MethodParamDetailsView::ctrlTypes() const
{
    return mType;
}

QCheckBox* MethodParamDetailsView::ctrlHasDefault() const
{
    return mHasDefault;
}

QLineEdit* MethodParamDetailsView::ctrlValue() const
{
    return mValue;
}

QPlainTextEdit* MethodParamDetailsView::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* MethodParamDetailsView::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* MethodParamDetailsView::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void MethodParamDetailsView::showNameHint(const QString& reason)
{
    mNameHint->setText(reason);
    mNameHint->setVisible(reason.isEmpty() == false);
    mName->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}
