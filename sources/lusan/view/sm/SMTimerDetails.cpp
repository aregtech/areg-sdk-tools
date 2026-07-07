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
 *  \file        lusan/view/sm/SMTimerDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — selected timer editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMTimerDetails.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <climits>

SMTimerDetails::SMTimerDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mNameHint         (nullptr)
    , mTimeout          (nullptr)
    , mRepeat           (nullptr)
    , mContinuous       (nullptr)
    , mDescription      (nullptr)
{
    buildUi();
}

void SMTimerDetails::buildUi()
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

    mTimeout = new QSpinBox(details);
    mTimeout->setRange(1, INT_MAX);
    mTimeout->setSuffix(tr(" ms"));
    form->addRow(tr("Timeout:"), mTimeout);

    mRepeat = new QSpinBox(details);
    mRepeat->setRange(1, INT_MAX);
    form->addRow(tr("Repeat:"), mRepeat);

    mContinuous = new QCheckBox(tr("Continuous:"), details);
    mContinuous->setLayoutDirection(Qt::RightToLeft);
    form->addRow(mContinuous);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe timer here"));
    form->addRow(tr("Description:"), mDescription);

    root->addWidget(details);
}

QLineEdit* SMTimerDetails::ctrlName() const
{
    return mName;
}

QSpinBox* SMTimerDetails::ctrlTimeout() const
{
    return mTimeout;
}

QSpinBox* SMTimerDetails::ctrlRepeat() const
{
    return mRepeat;
}

QCheckBox* SMTimerDetails::ctrlContinuous() const
{
    return mContinuous;
}

QPlainTextEdit* SMTimerDetails::ctrlDescription() const
{
    return mDescription;
}

void SMTimerDetails::showNameHint(const QString& reason)
{
    mNameHint->setText(reason);
    mNameHint->setVisible(reason.isEmpty() == false);
    mName->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}
