#ifndef LUSAN_APPLICATION_SI_SIDATATYPEDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIDATATYPEDETAILS_HPP
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
 *  \file        lusan/view/si/SIDataTypeDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>
#include <utility>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;

class SIDataTypeDetails : public QWidget
{
    Q_OBJECT

public:
    using CtrlGroup = std::pair<QLabel*, QGroupBox*>;

public:
    explicit SIDataTypeDetails(QWidget *parent = nullptr);
    
    QLineEdit* ctrlName() const;
    
    QRadioButton* ctrlTypeStruct()  const;
    
    QRadioButton* ctrlTypeEnum()  const;
    
    QRadioButton* ctrlTypeImport() const;
    
    QRadioButton* ctrlTypeContainer() const;
    
    QComboBox* ctrlContainerObject() const;
    
    QComboBox* ctrlContainerKey() const;
    
    QComboBox* ctrlContainerValue() const;
    
    QPlainTextEdit* ctrlDescription() const;
    
    QCheckBox* ctrlDeprecated() const;
    
    QLineEdit* ctrlDeprecateHint() const;
    
    QComboBox* ctrlEnumDerived() const;
    
    QLineEdit* ctrlImportLocation() const;
    
    QPushButton* ctrlButtonBrowse() const;
    
    QLineEdit* ctrlImportNamespace() const;

    QLineEdit* ctrlImportObject() const;

    SIDataTypeDetails::CtrlGroup ctrlDetailsEnum() const;

    SIDataTypeDetails::CtrlGroup ctrlDetailsImport() const;

    SIDataTypeDetails::CtrlGroup ctrlDetailsContainer() const;

    QFormLayout* ctrlLayout() const;

private:
    void buildUi();

private:
    QFormLayout*    mForm;
    QLineEdit*      mName;
    QRadioButton*   mTypeStruct;
    QRadioButton*   mTypeEnum;
    QRadioButton*   mTypeImport;
    QRadioButton*   mTypeContainer;
    QLabel*         mLabelEnum;
    QGroupBox*      mGroupEnum;
    QComboBox*      mEnumDerived;
    QLabel*         mLabelImport;
    QGroupBox*      mGroupImport;
    QLineEdit*      mImportLocation;
    QPushButton*    mImportBrowse;
    QLineEdit*      mImportNamespace;
    QLineEdit*      mImportObject;
    QLabel*         mLabelContainer;
    QGroupBox*      mGroupContainer;
    QComboBox*      mContainerObject;
    QComboBox*      mContainerKey;
    QComboBox*      mContainerValue;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif // LUSAN_APPLICATION_SI_SIDATATYPEDETAILS_HPP
