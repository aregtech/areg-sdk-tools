#ifndef LUSAN_APPLICATION_SI_SIMETHODDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIMETHODDETAILS_HPP
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
 *  \file        lusan/view/si/SIMethodDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;

class SIMethodDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SIMethodDetails(QWidget* parent = nullptr);

    QLineEdit * ctrlName() const;

    QRadioButton * ctrlBroadcast() const;

    QRadioButton * ctrlRequest() const;

    QRadioButton * ctrlResponse() const;

    QComboBox * ctrlConnectedResponse() const;

    QPlainTextEdit * ctrlDescription() const;

    QCheckBox * ctrlDeprecated() const;

    QLineEdit * ctrlDeprecateHint() const;

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QRadioButton*   mRequest;
    QRadioButton*   mResponse;
    QRadioButton*   mBroadcast;
    QComboBox*      mReply;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif // LUSAN_APPLICATION_SI_SIMETHODDETAILS_HPP
