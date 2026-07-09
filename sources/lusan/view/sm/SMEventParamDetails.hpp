#ifndef LUSAN_VIEW_SM_SMEVENTPARAMDETAILS_HPP
#define LUSAN_VIEW_SM_SMEVENTPARAMDETAILS_HPP
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
 *  \file        lusan/view/sm/SMEventParamDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — event payload parameter editor.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

/**
 * \brief   The selected event payload parameter editor: a "Details:" group with
 *          label-beside-control rows — Name, Type, an optional Default (a check-box next to
 *          the literal value field, disabled until checked: `ParamList` entries
 *          carry an optional `Default`), Description and a Deprecated row (a check-box next to
 *          its hint field). The deprecated flag and hint are round-tripped in the `.fsml`
 *          `Parameter` (`IsDeprecated` attribute + `DeprecateHint` child).
 **/
class SMEventParamDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMEventParamDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QComboBox* ctrlTypes() const;
    QCheckBox* ctrlHasDefault() const;
    QLineEdit* ctrlValue() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows the given reason below the name field; an empty reason hides the hint.
    void showNameHint(const QString& reason);

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QLabel*         mNameHint;
    QComboBox*      mType;
    QCheckBox*      mHasDefault;
    QLineEdit*      mValue;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_SM_SMEVENTPARAMDETAILS_HPP
