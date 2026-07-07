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
 *  \file        lusan/view/sm/SMEventDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — selected event editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMEventDetails.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

SMEventDetails::SMEventDetails(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mName             (nullptr)
    , mNameHint         (nullptr)
    , mDescription      (nullptr)
{
    buildUi();
}

void SMEventDetails::buildUi()
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

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe event here"));
    form->addRow(tr("Description:"), mDescription);

    root->addWidget(details);
}

QLineEdit* SMEventDetails::ctrlName() const
{
    return mName;
}

QPlainTextEdit* SMEventDetails::ctrlDescription() const
{
    return mDescription;
}

void SMEventDetails::showNameHint(const QString& reason)
{
    mNameHint->setText(reason);
    mNameHint->setVisible(reason.isEmpty() == false);
    mName->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}
