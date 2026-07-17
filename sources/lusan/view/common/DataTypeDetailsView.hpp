#ifndef LUSAN_VIEW_COMMON_DATATYPEDETAILSVIEW_HPP
#define LUSAN_VIEW_COMMON_DATATYPEDETAILSVIEW_HPP
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
 *  \file        lusan/view/common/DataTypeDetailsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared Data Types page -- selected data type editor.
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
 * \brief   The selected data type editor shared by the Service Interface (.siml), State
 *          Machine (.fsml) and future Data Type (.dtml) documents: a "Details:" group with
 *          label-beside-control rows -- Name, a "Type Definition:" radio group
 *          (Structure/Enumeration/Imported/Container), an "Enumeration Details:" group
 *          (Derived), an "Import Details:" group (Location/Browse, Namespace, Object), a
 *          "Container Details:" group (Object, Key, Value), Description, Deprecated.
 *
 *          The view is controller-agnostic: it only builds widgets, installs the shared
 *          identifier validator on the Name field, and exposes ctrl*() accessors. Each
 *          document's page controller owns the model/tree and wires the signals.
 **/
class DataTypeDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit DataTypeDetailsView(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QRadioButton* ctrlTypeStruct() const;
    QRadioButton* ctrlTypeEnum() const;
    QRadioButton* ctrlTypeImport() const;
    QRadioButton* ctrlTypeContainer() const;
    QComboBox* ctrlEnumDerived() const;
    QLineEdit* ctrlImportLocation() const;
    QPushButton* ctrlButtonBrowse() const;
    QLineEdit* ctrlImportNamespace() const;
    QLineEdit* ctrlImportObject() const;
    QComboBox* ctrlContainerObject() const;
    QComboBox* ctrlContainerKey() const;
    QComboBox* ctrlContainerValue() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows/hides the "Enumeration Details:" row.
    void setEnumRowVisible(bool visible);
    //!< Shows/hides the "Import Details:" row.
    void setImportRowVisible(bool visible);
    //!< Shows/hides the "Container Details:" row.
    void setContainerRowVisible(bool visible);

private:
    void buildUi();

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

#endif  // LUSAN_VIEW_COMMON_DATATYPEDETAILSVIEW_HPP
