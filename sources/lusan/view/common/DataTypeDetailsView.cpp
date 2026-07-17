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
 *  \file        lusan/view/common/DataTypeDetailsView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared Data Types page -- selected data type editor.
 *
 ************************************************************************/

#include "lusan/view/common/DataTypeDetailsView.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

DataTypeDetailsView::DataTypeDetailsView(QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mForm             (nullptr)
    , mName             (nullptr)
    , mTypeStruct       (nullptr)
    , mTypeEnum         (nullptr)
    , mTypeImport       (nullptr)
    , mTypeContainer    (nullptr)
    , mEnumLabel        (nullptr)
    , mEnumGroup        (nullptr)
    , mEnumDerived      (nullptr)
    , mImportLabel      (nullptr)
    , mImportGroup      (nullptr)
    , mImportLocation   (nullptr)
    , mImportBrowse     (nullptr)
    , mImportNamespace  (nullptr)
    , mImportObject     (nullptr)
    , mContainerLabel   (nullptr)
    , mContainerGroup   (nullptr)
    , mContainerObject  (nullptr)
    , mContainerKey     (nullptr)
    , mContainerValue   (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void DataTypeDetailsView::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QVBoxLayout* detailsLayout = new QVBoxLayout(details);
    mForm = new QFormLayout();
    mForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    mForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
    // Single source of truth for name validation: forbid invalid identifier characters at
    // the keystroke level, so every document that reuses this view behaves identically.
    mName->setValidator(NELusanCommon::createIdentifierValidator(mName));
    mForm->addRow(tr("Name:"), mName);

    QGroupBox* typeGroup = new QGroupBox(tr("Type Definition:"), details);
    typeGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout* typeLayout = new QHBoxLayout(typeGroup);
    mTypeStruct    = new QRadioButton(tr("Structure"), typeGroup);
    mTypeEnum      = new QRadioButton(tr("Enumeration"), typeGroup);
    mTypeImport    = new QRadioButton(tr("Imported"), typeGroup);
    mTypeContainer = new QRadioButton(tr("Container"), typeGroup);
    typeLayout->addWidget(mTypeStruct);
    typeLayout->addWidget(mTypeEnum);
    typeLayout->addWidget(mTypeImport);
    typeLayout->addWidget(mTypeContainer);
    typeLayout->addStretch(1);
    mForm->addRow(tr("Type:"), typeGroup);

    mEnumGroup = new QGroupBox(tr("Enumeration Details:"), details);
    QFormLayout* enumForm = new QFormLayout(mEnumGroup);
    mEnumDerived = new QComboBox(mEnumGroup);
    enumForm->addRow(tr("Derived:"), mEnumDerived);
    mEnumLabel = new QLabel(tr("Enumeration:"), details);
    mForm->addRow(mEnumLabel, mEnumGroup);

    mImportGroup = new QGroupBox(tr("Import Details:"), details);
    QFormLayout* importForm = new QFormLayout(mImportGroup);
    QWidget* locationRow = new QWidget(mImportGroup);
    QHBoxLayout* locationLayout = new QHBoxLayout(locationRow);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    mImportLocation = new QLineEdit(locationRow);
    mImportBrowse = new QPushButton(tr("Browse ..."), locationRow);
    locationLayout->addWidget(mImportLocation);
    locationLayout->addWidget(mImportBrowse);
    importForm->addRow(tr("Location:"), locationRow);
    mImportNamespace = new QLineEdit(mImportGroup);
    // The namespace must be a valid C++ qualified name (identifiers joined by '::'); forbid
    // any other character at the keystroke level so the table never receives invalid input.
    mImportNamespace->setValidator(NELusanCommon::createQualifiedNameValidator(mImportNamespace));
    importForm->addRow(tr("Namespace:"), mImportNamespace);
    mImportObject = new QLineEdit(mImportGroup);
    // The object is a single C++ identifier; forbid invalid characters as they are typed.
    mImportObject->setValidator(NELusanCommon::createIdentifierValidator(mImportObject));
    importForm->addRow(tr("Object:"), mImportObject);
    mImportLabel = new QLabel(tr("Imported:"), details);
    mForm->addRow(mImportLabel, mImportGroup);

    mContainerGroup = new QGroupBox(tr("Container Details:"), details);
    QFormLayout* containerForm = new QFormLayout(mContainerGroup);
    mContainerObject = new QComboBox(mContainerGroup);
    containerForm->addRow(tr("Object:"), mContainerObject);
    mContainerKey = new QComboBox(mContainerGroup);
    containerForm->addRow(tr("Key:"), mContainerKey);
    mContainerValue = new QComboBox(mContainerGroup);
    containerForm->addRow(tr("Value:"), mContainerValue);
    mContainerLabel = new QLabel(tr("Container:"), details);
    mForm->addRow(mContainerLabel, mContainerGroup);

    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe data here"));
    mForm->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    mForm->addRow(mDeprecated, mDeprecateHint);

    detailsLayout->addLayout(mForm);
    root->addWidget(details);
}

QLineEdit* DataTypeDetailsView::ctrlName() const
{
    return mName;
}

QRadioButton* DataTypeDetailsView::ctrlTypeStruct() const
{
    return mTypeStruct;
}

QRadioButton* DataTypeDetailsView::ctrlTypeEnum() const
{
    return mTypeEnum;
}

QRadioButton* DataTypeDetailsView::ctrlTypeImport() const
{
    return mTypeImport;
}

QComboBox* DataTypeDetailsView::ctrlEnumDerived() const
{
    return mEnumDerived;
}

QLineEdit* DataTypeDetailsView::ctrlImportLocation() const
{
    return mImportLocation;
}

QPushButton* DataTypeDetailsView::ctrlButtonBrowse() const
{
    return mImportBrowse;
}

QLineEdit* DataTypeDetailsView::ctrlImportNamespace() const
{
    return mImportNamespace;
}

QLineEdit* DataTypeDetailsView::ctrlImportObject() const
{
    return mImportObject;
}

QRadioButton* DataTypeDetailsView::ctrlTypeContainer() const
{
    return mTypeContainer;
}

QComboBox* DataTypeDetailsView::ctrlContainerObject() const
{
    return mContainerObject;
}

QComboBox* DataTypeDetailsView::ctrlContainerKey() const
{
    return mContainerKey;
}

QComboBox* DataTypeDetailsView::ctrlContainerValue() const
{
    return mContainerValue;
}

QPlainTextEdit* DataTypeDetailsView::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* DataTypeDetailsView::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* DataTypeDetailsView::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void DataTypeDetailsView::setEnumRowVisible(bool visible)
{
    mForm->setRowVisible(mEnumLabel, visible);
    mForm->setRowVisible(mEnumGroup, visible);
}

void DataTypeDetailsView::setImportRowVisible(bool visible)
{
    mForm->setRowVisible(mImportLabel, visible);
    mForm->setRowVisible(mImportGroup, visible);
}

void DataTypeDetailsView::setContainerRowVisible(bool visible)
{
    mForm->setRowVisible(mContainerLabel, visible);
    mForm->setRowVisible(mContainerGroup, visible);
}
