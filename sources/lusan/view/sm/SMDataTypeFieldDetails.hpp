#ifndef LUSAN_VIEW_SM_SMDATATYPEFIELDDETAILS_HPP
#define LUSAN_VIEW_SM_SMDATATYPEFIELDDETAILS_HPP
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
 *  \file        lusan/view/sm/SMDataTypeFieldDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page — structure/enumeration field editor.
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
 * \brief   Structure field / enumeration entry editor, code-built to mirror
 *          SIDataTypeFieldDetails: a "Details:" group with label-beside-control rows
 *          (Name, a "Field Type && Value Details:" sub-group with Type + Value, Description,
 *          Deprecated).
 **/
class SMDataTypeFieldDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMDataTypeFieldDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlName(void) const;
    QComboBox* ctrlTypes(void) const;
    QLineEdit* ctrlValue(void) const;
    QPlainTextEdit* ctrlDescription(void) const;
    QCheckBox* ctrlDeprecated(void) const;
    QLineEdit* ctrlDeprecateHint(void) const;

    //!< Shows/hides the Type combo row — enumeration entries have no type, only a value.
    void setTypeRowVisible(bool visible);

private:
    void buildUi(void);

private:
    QLineEdit*      mName;
    QComboBox*      mType;
    QLineEdit*      mValue;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
    QFormLayout*    mFieldForm;
    QFormLayout*    mTypeValueForm;
};

#endif  // LUSAN_VIEW_SM_SMDATATYPEFIELDDETAILS_HPP
