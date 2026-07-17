#ifndef LUSAN_VIEW_COMMON_DATATYPEFIELDDETAILSVIEW_HPP
#define LUSAN_VIEW_COMMON_DATATYPEFIELDDETAILSVIEW_HPP
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
 *  \file        lusan/view/common/DataTypeFieldDetailsView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared Data Types page -- structure/enumeration field editor.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

/**
 * \brief   Structure field / enumeration entry editor shared by the Service Interface
 *          (.siml), State Machine (.fsml) and future Data Type (.dtml) documents: a
 *          "Details:" group with label-beside-control rows (Name, a "Field Type && Value
 *          Details:" sub-group with Type + Value, Description, Deprecated).
 *
 *          The view is controller-agnostic: it builds widgets, installs the shared
 *          identifier validator on the Name field, and exposes ctrl*() accessors.
 **/
class DataTypeFieldDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit DataTypeFieldDetailsView(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QComboBox* ctrlTypes() const;
    QLineEdit* ctrlValue() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows/hides the Type combo row -- enumeration entries have no type, only a value.
    void setTypeRowVisible(bool visible);

    //!< Shows the given reason below the value control; an empty reason hides the hint.
    void showValueHint(const QString& reason);

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QComboBox*      mType;
    QLineEdit*      mValue;
    QLabel*         mValueHint;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
    QFormLayout*    mFieldForm;
    QFormLayout*    mTypeValueForm;
};

#endif  // LUSAN_VIEW_COMMON_DATATYPEFIELDDETAILSVIEW_HPP
