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
 *  \file        lusan/view/sm/SMDataTypeDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page — selected data type editor.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDataTypeDetails.hpp"

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

SMDataTypeDetails::SMDataTypeDetails(QWidget* parent /*= nullptr*/)
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

void SMDataTypeDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QVBoxLayout* detailsLayout = new QVBoxLayout(details);
    mForm = new QFormLayout();
    mForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    mForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mName = new QLineEdit(details);
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
    importForm->addRow(tr("Namespace:"), mImportNamespace);
    mImportObject = new QLineEdit(mImportGroup);
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

QLineEdit* SMDataTypeDetails::ctrlName() const
{
    return mName;
}

QRadioButton* SMDataTypeDetails::ctrlTypeStruct() const
{
    return mTypeStruct;
}

QRadioButton* SMDataTypeDetails::ctrlTypeEnum() const
{
    return mTypeEnum;
}

QRadioButton* SMDataTypeDetails::ctrlTypeImport() const
{
    return mTypeImport;
}

QComboBox* SMDataTypeDetails::ctrlEnumDerived() const
{
    return mEnumDerived;
}

QLineEdit* SMDataTypeDetails::ctrlImportLocation() const
{
    return mImportLocation;
}

QPushButton* SMDataTypeDetails::ctrlButtonBrowse() const
{
    return mImportBrowse;
}

QLineEdit* SMDataTypeDetails::ctrlImportNamespace() const
{
    return mImportNamespace;
}

QLineEdit* SMDataTypeDetails::ctrlImportObject() const
{
    return mImportObject;
}

QRadioButton* SMDataTypeDetails::ctrlTypeContainer() const
{
    return mTypeContainer;
}

QComboBox* SMDataTypeDetails::ctrlContainerObject() const
{
    return mContainerObject;
}

QComboBox* SMDataTypeDetails::ctrlContainerKey() const
{
    return mContainerKey;
}

QComboBox* SMDataTypeDetails::ctrlContainerValue() const
{
    return mContainerValue;
}

QPlainTextEdit* SMDataTypeDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SMDataTypeDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SMDataTypeDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

void SMDataTypeDetails::setEnumRowVisible(bool visible)
{
    mForm->setRowVisible(mEnumLabel, visible);
    mForm->setRowVisible(mEnumGroup, visible);
}

void SMDataTypeDetails::setImportRowVisible(bool visible)
{
    mForm->setRowVisible(mImportLabel, visible);
    mForm->setRowVisible(mImportGroup, visible);
}

void SMDataTypeDetails::setContainerRowVisible(bool visible)
{
    mForm->setRowVisible(mContainerLabel, visible);
    mForm->setRowVisible(mContainerGroup, visible);
}
