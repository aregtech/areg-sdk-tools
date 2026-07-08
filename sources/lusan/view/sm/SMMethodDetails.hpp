#ifndef LUSAN_VIEW_SM_SMMETHODDETAILS_HPP
#define LUSAN_VIEW_SM_SMMETHODDETAILS_HPP
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
 *  \file        lusan/view/sm/SMMethodDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page — selected method editor.
 *
 ************************************************************************/

#include <QWidget>

class SMCodeEditor;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;

/**
 * \brief   The selected method editor: a "Details:" group with label-beside-control rows —
 *          Name, a Type radio group (Trigger / Action / Condition), and, for conditions only,
 *          a Return type combo, an Implement radio group (Handler / Embedded) and — when
 *          Embedded — the code Body editor. Description closes the form. An inline hint below
 *          Name flags a stimulus-name collision (trigger methods only share the name space).
 **/
class SMMethodDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMMethodDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QRadioButton* ctrlTrigger() const;
    QRadioButton* ctrlAction() const;
    QRadioButton* ctrlCondition() const;
    QComboBox* ctrlReturn() const;
    QRadioButton* ctrlHandler() const;
    QRadioButton* ctrlEmbedded() const;
    SMCodeEditor* ctrlBody() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

    //!< Shows the given reason below the name field; an empty reason hides the hint.
    void showNameHint(const QString& reason);

    //!< Shows or hides the condition-only Return and Implement rows.
    void setConditionVisible(bool visible);
    //!< Shows or hides the embedded-condition Body editor row.
    void setBodyVisible(bool visible);

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QLabel*         mNameHint;
    QRadioButton*   mTrigger;
    QRadioButton*   mAction;
    QRadioButton*   mCondition;
    QComboBox*      mReturn;
    QRadioButton*   mHandler;
    QRadioButton*   mEmbedded;
    QWidget*        mReturnRow;     //!< The Return field cell (row toggled with the mode).
    QWidget*        mImplementRow;  //!< The Implement field cell (row toggled with the mode).
    SMCodeEditor*   mBody;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
    QFormLayout*    mForm;          //!< The details form (used to toggle whole rows).
};

#endif  // LUSAN_VIEW_SM_SMMETHODDETAILS_HPP
