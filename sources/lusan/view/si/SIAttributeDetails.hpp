#ifndef LUSAN_APPLICATION_SI_SIATTRIBUTEDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIATTRIBUTEDETAILS_HPP
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
 *  \file        lusan/view/si/SIAttributeDetails.hpp
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

class SIAttributeDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SIAttributeDetails(QWidget* parent = nullptr);

    // Getters to access controls

    QLineEdit* ctrlName();

    QComboBox* ctrlTypes();

    QComboBox* ctrlNotification();

    QPlainTextEdit* ctrlDescription();

    QCheckBox* ctrlDeprecated();

    QLineEdit* ctrlDeprecateHint();

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QComboBox*      mTypes;
    QComboBox*      mNotify;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif // LUSAN_APPLICATION_SI_SIATTRIBUTEDETAILS_HPP
