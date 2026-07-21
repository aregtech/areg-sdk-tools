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
 *  \file        lusan/view/common/MethodDetailsView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "method details" editor implementation.
 *
 ************************************************************************/

#include "lusan/view/common/MethodDetailsView.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/EditCancelFilter.hpp"
#include "lusan/view/sm/SMCodeEditor.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QVBoxLayout>

MethodDetailsView::MethodDetailsView(const MethodViewConfig& config, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mConfig           (config)
    , mName             (nullptr)
    , mNameHint         (nullptr)
    , mTypes            ( )
    , mReply            (nullptr)
    , mReturn           (nullptr)
    , mHandler          (nullptr)
    , mEmbedded         (nullptr)
    , mImplementRow     (nullptr)
    , mGuardInfo        (nullptr)
    , mBody             (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
    , mForm             (nullptr)
{
    buildUi();
}

void MethodDetailsView::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    mForm = new QFormLayout(details);
    mForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    mForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

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
    mForm->addRow(tr("Name:"), nameCell);

    // The method-type radio group, driven by the config labels (and optional icons).
    QWidget* typeCell = new QWidget(details);
    QHBoxLayout* typeLayout = new QHBoxLayout(typeCell);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    for (const MethodTypeOption& option : mConfig.types)
    {
        QRadioButton* radio = new QRadioButton(option.label, typeCell);
        if (option.iconResource.isEmpty() == false)
        {
            radio->setIcon(QIcon(option.iconResource));
        }
        typeLayout->addWidget(radio);
        mTypes.append(radio);
    }
    typeLayout->addStretch(1);
    mForm->addRow(tr("Type:"), typeCell);

    // Reply combo: the request/response link (Service Interface only).
    mReply = new QComboBox(details);
    if (mConfig.hasReply)
    {
        mForm->addRow(tr("Reply:"), mReply);
    }
    else
    {
        mReply->setVisible(false);
    }

    // Return combo: the condition return type (State Machine only). Editable so a free-form
    // type name can be typed as well as picked.
    mReturn = new QComboBox(details);
    mReturn->setEditable(true);
    mReturn->setToolTip(tr("A condition always returns a value (default bool)."));
    if (mConfig.hasReturn)
    {
        mForm->addRow(tr("Return:"), mReturn);
    }
    else
    {
        mReturn->setVisible(false);
    }

    // Implement radios: Handler / Embedded (State Machine conditions only).
    mImplementRow = new QWidget(details);
    QHBoxLayout* implLayout = new QHBoxLayout(mImplementRow);
    implLayout->setContentsMargins(0, 0, 0, 0);
    mHandler = new QRadioButton(tr("Handler"), mImplementRow);
    mEmbedded = new QRadioButton(tr("Embedded"), mImplementRow);
    implLayout->addWidget(mHandler);
    implLayout->addWidget(mEmbedded);
    implLayout->addStretch(1);
    if (mConfig.hasImplement)
    {
        mForm->addRow(tr("Implement:"), mImplementRow);
    }
    else
    {
        mImplementRow->setVisible(false);
    }

    // Guard-use line: kind, generated call form, `used by N guards` link (State Machine).
    mGuardInfo = new QLabel(details);
    mGuardInfo->setObjectName(QStringLiteral("smMethodGuardInfo"));
    mGuardInfo->setTextFormat(Qt::RichText);
    mGuardInfo->setWordWrap(true);
    if (mConfig.hasGuardInfo)
    {
        mForm->addRow(tr("Guard use:"), mGuardInfo);
    }
    else
    {
        mGuardInfo->setVisible(false);
    }

    // Embedded-condition body editor (State Machine).
    mBody = new SMCodeEditor(details);
    mBody->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    if (mConfig.hasBody)
    {
        mForm->addRow(tr("Body:"), mBody);
    }
    else
    {
        mBody->setVisible(false);
    }

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe method here"));
    mForm->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    mForm->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);

    // Re-emit the Name field edits as a controller-facing signal so the page can mirror the
    // typed name into its list row without the view knowing about the model or the tree/table.
    connect(mName, &QLineEdit::textChanged, this, &MethodDetailsView::nameEdited);

    // Escape cancels the edit: the live-synced text fields restore their pre-edit value. Combos
    // (Type / Reply / Return) commit immediately and need no cancel; the multi-line Description
    // and the Body editor keep their own commit-on-focus-out behavior.
    EditCancelFilter::install(mName);
    EditCancelFilter::install(mDeprecateHint);

    setConditionVisible(false);
    setBodyVisible(false);
    setGuardInfoVisible(false);
}

QLineEdit* MethodDetailsView::ctrlName() const
{
    return mName;
}

int MethodDetailsView::typeCount() const
{
    return static_cast<int>(mTypes.size());
}

QRadioButton* MethodDetailsView::ctrlType(int index) const
{
    return ((index >= 0) && (index < mTypes.size())) ? mTypes.at(index) : nullptr;
}

QComboBox* MethodDetailsView::ctrlReply() const
{
    return mReply;
}

QComboBox* MethodDetailsView::ctrlReturn() const
{
    return mReturn;
}

QRadioButton* MethodDetailsView::ctrlHandler() const
{
    return mHandler;
}

QRadioButton* MethodDetailsView::ctrlEmbedded() const
{
    return mEmbedded;
}

QLabel* MethodDetailsView::ctrlGuardInfo() const
{
    return mGuardInfo;
}

SMCodeEditor* MethodDetailsView::ctrlBody() const
{
    return mBody;
}

QPlainTextEdit* MethodDetailsView::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* MethodDetailsView::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* MethodDetailsView::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void MethodDetailsView::showNameHint(const QString& reason)
{
    mNameHint->setText(reason);
    mNameHint->setVisible(reason.isEmpty() == false);
    mName->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}

void MethodDetailsView::setConditionVisible(bool visible)
{
    if (mConfig.hasReturn)
    {
        mForm->setRowVisible(mReturn, visible);
    }
    if (mConfig.hasImplement)
    {
        mForm->setRowVisible(mImplementRow, visible);
    }
}

void MethodDetailsView::setBodyVisible(bool visible)
{
    if (mConfig.hasBody)
    {
        mForm->setRowVisible(mBody, visible);
    }
}

void MethodDetailsView::setGuardInfoVisible(bool visible)
{
    if (mConfig.hasGuardInfo)
    {
        mForm->setRowVisible(mGuardInfo, visible);
    }
}
