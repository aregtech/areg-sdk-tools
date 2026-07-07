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
 *  \file        lusan/view/sm/SMMethodDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page — selected method editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMMethodDetails.hpp"

#include "lusan/view/sm/SMCodeEditor.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QVBoxLayout>

SMMethodDetails::SMMethodDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mNameHint         (nullptr)
    , mTrigger          (nullptr)
    , mAction           (nullptr)
    , mCondition        (nullptr)
    , mReturn           (nullptr)
    , mHandler          (nullptr)
    , mEmbedded         (nullptr)
    , mReturnRow        (nullptr)
    , mImplementRow     (nullptr)
    , mBody             (nullptr)
    , mDescription      (nullptr)
    , mForm             (nullptr)
{
    buildUi();
}

void SMMethodDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    mForm = new QFormLayout(details);
    mForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    mForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

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
    mForm->addRow(tr("Name:"), nameCell);

    QWidget* typeCell = new QWidget(details);
    QHBoxLayout* typeLayout = new QHBoxLayout(typeCell);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    mTrigger = new QRadioButton(tr("Trigger"), typeCell);
    mAction = new QRadioButton(tr("Action"), typeCell);
    mCondition = new QRadioButton(tr("Condition"), typeCell);
    typeLayout->addWidget(mTrigger);
    typeLayout->addWidget(mAction);
    typeLayout->addWidget(mCondition);
    typeLayout->addStretch(1);
    mForm->addRow(tr("Type:"), typeCell);

    mReturnRow = new QComboBox(details);
    mReturn = static_cast<QComboBox*>(mReturnRow);
    mReturn->setEditable(true);
    mReturn->setToolTip(tr("A condition always returns a value (default bool)."));
    mForm->addRow(tr("Return:"), mReturnRow);

    mImplementRow = new QWidget(details);
    QHBoxLayout* implLayout = new QHBoxLayout(mImplementRow);
    implLayout->setContentsMargins(0, 0, 0, 0);
    mHandler = new QRadioButton(tr("Handler"), mImplementRow);
    mEmbedded = new QRadioButton(tr("Embedded"), mImplementRow);
    implLayout->addWidget(mHandler);
    implLayout->addWidget(mEmbedded);
    implLayout->addStretch(1);
    mForm->addRow(tr("Implement:"), mImplementRow);

    mBody = new SMCodeEditor(details);
    mBody->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mForm->addRow(tr("Body:"), mBody);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe method here"));
    mForm->addRow(tr("Description:"), mDescription);

    root->addWidget(details);

    setConditionVisible(false);
    setBodyVisible(false);
}

QLineEdit* SMMethodDetails::ctrlName() const
{
    return mName;
}

QRadioButton* SMMethodDetails::ctrlTrigger() const
{
    return mTrigger;
}

QRadioButton* SMMethodDetails::ctrlAction() const
{
    return mAction;
}

QRadioButton* SMMethodDetails::ctrlCondition() const
{
    return mCondition;
}

QComboBox* SMMethodDetails::ctrlReturn() const
{
    return mReturn;
}

QRadioButton* SMMethodDetails::ctrlHandler() const
{
    return mHandler;
}

QRadioButton* SMMethodDetails::ctrlEmbedded() const
{
    return mEmbedded;
}

SMCodeEditor* SMMethodDetails::ctrlBody() const
{
    return mBody;
}

QPlainTextEdit* SMMethodDetails::ctrlDescription() const
{
    return mDescription;
}

void SMMethodDetails::showNameHint(const QString& reason)
{
    mNameHint->setText(reason);
    mNameHint->setVisible(reason.isEmpty() == false);
    mName->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}

void SMMethodDetails::setConditionVisible(bool visible)
{
    mForm->setRowVisible(mReturnRow, visible);
    mForm->setRowVisible(mImplementRow, visible);
}

void SMMethodDetails::setBodyVisible(bool visible)
{
    mForm->setRowVisible(mBody, visible);
}
