#ifndef LUSAN_VIEW_SM_SMINCLUDEDETAILS_HPP
#define LUSAN_VIEW_SM_SMINCLUDEDETAILS_HPP
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
 *  \file        lusan/view/sm/SMIncludeDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page — selected include editor.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

/**
 * \brief   The selected include editor, code-built to mirror the Service Interface include
 *          details: a "Details:" group with label-beside-control rows — the include file
 *          location with a Browse button, Description, and Deprecated flag with hint.
 **/
class SMIncludeDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMIncludeDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlInclude() const;
    QPushButton* ctrlBrowseButton() const;
    QPlainTextEdit* ctrlDescription() const;
    QCheckBox* ctrlDeprecated() const;
    QLineEdit* ctrlDeprecateHint() const;

private:
    void buildUi();

private:
    QLineEdit*      mInclude;
    QPushButton*    mBrowse;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_SM_SMINCLUDEDETAILS_HPP
