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
 *  \file        lusan/view/common/IncludeDetailsView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "include details" editor implementation.
 *
 ************************************************************************/

#include "lusan/view/common/IncludeDetailsView.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/EditCancelFilter.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

IncludeDetailsView::IncludeDetailsView(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mInclude          (nullptr)
    , mBrowse           (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void IncludeDetailsView::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    QWidget* locationCell = new QWidget(details);
    QHBoxLayout* locationLayout = new QHBoxLayout(locationCell);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    locationLayout->setSpacing(4);
    mInclude = new QLineEdit(locationCell);
    mInclude->setValidator(NELusanCommon::createPathValidator(mInclude));
    mBrowse = new QPushButton(tr("Browse..."), locationCell);
    locationLayout->addWidget(mInclude, 1);
    locationLayout->addWidget(mBrowse, 0);
    form->addRow(tr("Include File:"), locationCell);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe include here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);

    // Re-emit the location field edits as a controller-facing signal so the page can mirror the
    // typed path into its list row without the view knowing about the model or the tree/table.
    connect(mInclude, &QLineEdit::textChanged, this, &IncludeDetailsView::nameEdited);

    // Escape cancels the edit: the live-synced text fields restore their pre-edit value.
    EditCancelFilter::install(mInclude);
    EditCancelFilter::install(mDeprecateHint);
}

QLineEdit* IncludeDetailsView::ctrlInclude() const
{
    return mInclude;
}

QPushButton* IncludeDetailsView::ctrlBrowseButton() const
{
    return mBrowse;
}

QPlainTextEdit* IncludeDetailsView::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* IncludeDetailsView::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* IncludeDetailsView::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}
