#ifndef LUSAN_VIEW_SM_SMTIMERDETAILS_HPP
#define LUSAN_VIEW_SM_SMTIMERDETAILS_HPP
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
 *  \file        lusan/view/sm/SMTimerDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page — selected timer editor.
 *
 ************************************************************************/

#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

/**
 * \brief   The selected timer editor: a "Details:" group with label-beside-control rows —
 *          Name, Timeout (ms, minimum 1), Repeat (disabled while Continuous is checked),
 *          Continuous, Description. An inline hint below Name flags a stimulus-name collision
 *          with an existing trigger method, event or another timer.
 **/
class SMTimerDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SMTimerDetails(QWidget* parent = nullptr);

    QLineEdit* ctrlName() const;
    QSpinBox* ctrlTimeout() const;
    QSpinBox* ctrlRepeat() const;
    QCheckBox* ctrlContinuous() const;
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
    QSpinBox*       mTimeout;
    QSpinBox*       mRepeat;
    QCheckBox*      mContinuous;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif  // LUSAN_VIEW_SM_SMTIMERDETAILS_HPP
