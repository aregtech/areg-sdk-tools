#ifndef LUSAN_VIEW_SM_SMCONSTANTDETAILS_HPP
#define LUSAN_VIEW_SM_SMCONSTANTDETAILS_HPP
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
 *  \file        lusan/view/sm/SMConstantDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page — selected constant editor.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

/**
 * \brief   The selected constant editor, code-built to mirror SIConstantDetails: a
 *          "Details:" group with label-beside-control rows — Name, Type, Value (an editable
 *          combo: free text for a primitive type, a picker of enumerator names for an
 *          enumeration type, disabled for a structure/container/imported type — spec 6.9
 *          has no literal form for those), an inline validation hint below the value,
 *          Description, Deprecated.
 **/
class SMConstantDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMConstantDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QComboBox* ctrlTypes() const;
    QComboBox* ctrlValue() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows the given reason below the value control; an empty reason hides the hint.
    void showValueHint(const QString& reason);

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QComboBox*      mTypes;
    QComboBox*      mValue;
    QLabel*         mValueHint;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_SM_SMCONSTANTDETAILS_HPP
