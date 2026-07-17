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
 *  \file        lusan/view/common/AttributeDetailsView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "attribute details" editor implementation.
 *
 ************************************************************************/

#include "lusan/view/common/AttributeDetailsView.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/EditCancelFilter.hpp"

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

AttributeDetailsView::AttributeDetailsView(const AttributeViewConfig& config, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mConfig           (config)
    , mName             (nullptr)
    , mTypes            (nullptr)
    , mValue            (nullptr)
    , mValueCompleter   (nullptr)
    , mValueChoices     (nullptr)
    , mValueHint        (nullptr)
    , mNotify           (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void AttributeDetailsView::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QFormLayout* form = new QFormLayout(details);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    mName->setValidator(NELusanCommon::createIdentifierValidator(mName));
    form->addRow(tr("Name:"), mName);

    mTypes = new QComboBox(details);
    form->addRow(tr("Type:"), mTypes);

    // Value control (line edit + enumerator completer + inline validation hint). Always built
    // so ctrlValue()/showValueHint()/setValueChoices() are valid; only shown when configured.
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
    if (mConfig.hasValue)
    {
        form->addRow(tr("Value:"), valueCell);
    }
    else
    {
        valueCell->setVisible(false);
    }

    // Notification kind combo. Always built so ctrlNotification() is valid; the page controller
    // supplies the item model (the "OnChange" / "Always" kinds) exactly as it does for Type.
    mNotify = new QComboBox(details);
    if (mConfig.hasNotification)
    {
        form->addRow(tr("Notify:"), mNotify);
    }
    else
    {
        mNotify->setVisible(false);
    }

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe attribute here"));
    form->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    form->addRow(mDeprecated, mDeprecateHint);

    root->addWidget(details);

    // Re-emit the Name field edits as a controller-facing signal so the page can mirror the
    // typed name into its list row without the view knowing about the model or the tree/table.
    connect(mName, &QLineEdit::textChanged, this, &AttributeDetailsView::nameEdited);

    // Escape cancels the edit: the live-synced text fields restore their pre-edit value. Combos
    // (Type / Notify) commit immediately and need no cancel; the multi-line Description is left
    // as-is.
    EditCancelFilter::install(mName);
    EditCancelFilter::install(mValue);
    EditCancelFilter::install(mDeprecateHint);
}

QLineEdit* AttributeDetailsView::ctrlName() const
{
    return mName;
}

QComboBox* AttributeDetailsView::ctrlTypes() const
{
    return mTypes;
}

QLineEdit* AttributeDetailsView::ctrlValue() const
{
    return mValue;
}

QComboBox* AttributeDetailsView::ctrlNotification() const
{
    return mNotify;
}

QPlainTextEdit* AttributeDetailsView::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* AttributeDetailsView::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* AttributeDetailsView::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void AttributeDetailsView::showValueHint(const QString& reason)
{
    mValueHint->setText(reason);
    mValueHint->setVisible(reason.isEmpty() == false);
    mValue->setStyleSheet(reason.isEmpty() ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}

void AttributeDetailsView::setValueChoices(const QStringList& choices)
{
    mValueChoices->setStringList(choices);
}
