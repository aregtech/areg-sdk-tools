#ifndef LUSAN_VIEW_SM_SMDATATYPEDETAILS_HPP
#define LUSAN_VIEW_SM_SMDATATYPEDETAILS_HPP
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
 *  \file        lusan/view/sm/SMDataTypeDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page — selected data type editor.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;

/**
 * \brief   The selected data type editor, code-built to mirror SIDataTypeDetails: a
 *          "Details:" group with label-beside-control rows — Name, a "Type Definition:"
 *          radio group (Structure/Enumeration/Imported/Container, spec 6.9/6.10), an
 *          "Enumeration Details:" group (Derived), an "Import Details:" group
 *          (Location/Browse, Namespace, Object), a "Container Details:" group (Object,
 *          Key, Value), Description, Deprecated.
 **/
class SMDataTypeDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMDataTypeDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlName(void) const;
    QRadioButton* ctrlTypeStruct(void) const;
    QRadioButton* ctrlTypeEnum(void) const;
    QRadioButton* ctrlTypeImport(void) const;
    QRadioButton* ctrlTypeContainer(void) const;
    QComboBox* ctrlEnumDerived(void) const;
    QLineEdit* ctrlImportLocation(void) const;
    QPushButton* ctrlButtonBrowse(void) const;
    QLineEdit* ctrlImportNamespace(void) const;
    QLineEdit* ctrlImportObject(void) const;
    QComboBox* ctrlContainerObject(void) const;
    QComboBox* ctrlContainerKey(void) const;
    QComboBox* ctrlContainerValue(void) const;
    QPlainTextEdit* ctrlDescription(void) const;
    QCheckBox* ctrlDeprecated(void) const;
    QLineEdit* ctrlDeprecateHint(void) const;

    //!< Shows/hides the "Enumeration Details:" row.
    void setEnumRowVisible(bool visible);
    //!< Shows/hides the "Import Details:" row.
    void setImportRowVisible(bool visible);
    //!< Shows/hides the "Container Details:" row.
    void setContainerRowVisible(bool visible);

private:
    void buildUi(void);

private:
    QFormLayout*    mForm;
    QLineEdit*      mName;
    QRadioButton*   mTypeStruct;
    QRadioButton*   mTypeEnum;
    QRadioButton*   mTypeImport;
    QRadioButton*   mTypeContainer;
    QLabel*         mEnumLabel;
    QGroupBox*      mEnumGroup;
    QComboBox*      mEnumDerived;
    QLabel*         mImportLabel;
    QGroupBox*      mImportGroup;
    QLineEdit*      mImportLocation;
    QPushButton*    mImportBrowse;
    QLineEdit*      mImportNamespace;
    QLineEdit*      mImportObject;
    QLabel*         mContainerLabel;
    QGroupBox*      mContainerGroup;
    QComboBox*      mContainerObject;
    QComboBox*      mContainerKey;
    QComboBox*      mContainerValue;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_SM_SMDATATYPEDETAILS_HPP
