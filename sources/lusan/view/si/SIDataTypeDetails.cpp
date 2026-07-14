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
 *  \file        lusan/view/si/SIDataTypeDetails.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface data type editor.
 *
 ************************************************************************/
#include "lusan/view/si/SIDataTypeDetails.hpp"

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

SIDataTypeDetails::SIDataTypeDetails(QWidget *parent)
    : QWidget           (parent)
    , mForm             (nullptr)
    , mName             (nullptr)
    , mTypeStruct       (nullptr)
    , mTypeEnum         (nullptr)
    , mTypeImport       (nullptr)
    , mTypeContainer    (nullptr)
    , mLabelEnum        (nullptr)
    , mGroupEnum        (nullptr)
    , mEnumDerived      (nullptr)
    , mLabelImport      (nullptr)
    , mGroupImport      (nullptr)
    , mImportLocation   (nullptr)
    , mImportBrowse     (nullptr)
    , mImportNamespace  (nullptr)
    , mImportObject     (nullptr)
    , mLabelContainer   (nullptr)
    , mGroupContainer   (nullptr)
    , mContainerObject  (nullptr)
    , mContainerKey     (nullptr)
    , mContainerValue   (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
{
    buildUi();
}

void SIDataTypeDetails::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* details = new QGroupBox(tr("Details:"), this);
    QVBoxLayout* detailsLayout = new QVBoxLayout(details);

    mForm = new QFormLayout();
    mForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    mForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Row: Name
    mName = new QLineEdit(details);
    mForm->addRow(tr("Name:"), mName);

    // Row: Type Definition (4 radios in a row)
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

    // Row: Enumeration details
    mLabelEnum = new QLabel(tr("Enumeration:"), details);
    mGroupEnum = new QGroupBox(tr("Enumeration Details:"), details);
    QFormLayout* enumForm = new QFormLayout(mGroupEnum);
    mEnumDerived = new QComboBox(mGroupEnum);
    enumForm->addRow(tr("Derived:"), mEnumDerived);
    mForm->addRow(mLabelEnum, mGroupEnum);

    // Row: Import details
    mLabelImport = new QLabel(tr("Imported:"), details);
    mGroupImport = new QGroupBox(tr("Import Details:"), details);
    QFormLayout* importForm = new QFormLayout(mGroupImport);
    QWidget* includeCell = new QWidget(mGroupImport);
    QHBoxLayout* includeLayout = new QHBoxLayout(includeCell);
    includeLayout->setContentsMargins(0, 0, 0, 0);
    mImportLocation = new QLineEdit(includeCell);
    mImportBrowse = new QPushButton(tr("Browse ..."), includeCell);
    includeLayout->addWidget(mImportLocation);
    includeLayout->addWidget(mImportBrowse);
    importForm->addRow(tr("Location:"), includeCell);
    mImportNamespace = new QLineEdit(mGroupImport);
    importForm->addRow(tr("Namespace:"), mImportNamespace);
    mImportObject = new QLineEdit(mGroupImport);
    importForm->addRow(tr("Object:"), mImportObject);
    mForm->addRow(mLabelImport, mGroupImport);

    // Row: Container details
    mLabelContainer = new QLabel(tr("Container:"), details);
    mGroupContainer = new QGroupBox(tr("Container Details:"), details);
    QFormLayout* containerForm = new QFormLayout(mGroupContainer);
    mContainerObject = new QComboBox(mGroupContainer);
    containerForm->addRow(tr("Base:"), mContainerObject);
    mContainerKey = new QComboBox(mGroupContainer);
    containerForm->addRow(tr("Key:"), mContainerKey);
    mContainerValue = new QComboBox(mGroupContainer);
    containerForm->addRow(tr("Value:"), mContainerValue);
    mForm->addRow(mLabelContainer, mGroupContainer);

    // Row: Description
    mDescription = new QPlainTextEdit(details);
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(tr("Describe data here"));
    mForm->addRow(tr("Description:"), mDescription);

    // Row: Deprecated
    mDeprecated = new QCheckBox(tr("Deprecated:"), details);
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(details);
    mForm->addRow(mDeprecated, mDeprecateHint);

    detailsLayout->addLayout(mForm);
    root->addWidget(details);
}


QLineEdit* SIDataTypeDetails::ctrlName() const
{
    return mName;
}

QRadioButton* SIDataTypeDetails::ctrlTypeStruct() const
{
    return mTypeStruct;
}

QRadioButton* SIDataTypeDetails::ctrlTypeEnum() const
{
    return mTypeEnum;
}

QRadioButton* SIDataTypeDetails::ctrlTypeImport() const
{
    return mTypeImport;
}

QRadioButton* SIDataTypeDetails::ctrlTypeContainer() const
{
    return mTypeContainer;
}

QComboBox* SIDataTypeDetails::ctrlContainerObject() const
{
    return mContainerObject;
}

QComboBox* SIDataTypeDetails::ctrlContainerKey() const
{
    return mContainerKey;
}

QComboBox* SIDataTypeDetails::ctrlContainerValue() const
{
    return mContainerValue;
}

QPlainTextEdit* SIDataTypeDetails::ctrlDescription() const
{
    return mDescription;
}

QCheckBox* SIDataTypeDetails::ctrlDeprecated() const
{
    return mDeprecated;
}

QLineEdit* SIDataTypeDetails::ctrlDeprecateHint() const
{
    return mDeprecateHint;
}

QComboBox* SIDataTypeDetails::ctrlEnumDerived() const
{
    return mEnumDerived;
}

QLineEdit* SIDataTypeDetails::ctrlImportLocation() const
{
    return mImportLocation;
}

QPushButton* SIDataTypeDetails::ctrlButtonBrowse() const
{
    return mImportBrowse;
}

QLineEdit* SIDataTypeDetails::ctrlImportNamespace() const
{
    return mImportNamespace;
}

QLineEdit* SIDataTypeDetails::ctrlImportObject() const
{
    return mImportObject;
}

SIDataTypeDetails::CtrlGroup SIDataTypeDetails::ctrlDetailsEnum() const
{
    return CtrlGroup{mLabelEnum, mGroupEnum};
}

SIDataTypeDetails::CtrlGroup SIDataTypeDetails::ctrlDetailsImport() const
{
    return CtrlGroup{mLabelImport, mGroupImport};
}

SIDataTypeDetails::CtrlGroup SIDataTypeDetails::ctrlDetailsContainer() const
{
    return CtrlGroup{mLabelContainer, mGroupContainer};
}

QFormLayout* SIDataTypeDetails::ctrlLayout() const
{
    return mForm;
}
