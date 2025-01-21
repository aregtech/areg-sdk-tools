#ifndef LUSAN_APPLICATION_SI_SIDATATOPICDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIDATATOPICDETAILS_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIDataTopicDetails.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

namespace Ui {
    class SIDataTopicDetails;
}

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;

class SIDataTopicDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SIDataTopicDetails(QWidget* parent = nullptr);

    // Getters to access controls

    QLineEdit* ctrlName(void);

    QComboBox* ctrlTypes(void);

    QComboBox* ctrlNotification(void);

    QPlainTextEdit* ctrlDescription(void);

    QCheckBox* ctrlDeprecated(void);

    QLineEdit* ctrlDeprecateHint(void);

private:
    Ui::SIDataTopicDetails* ui;
};

#endif // LUSAN_APPLICATION_SI_SIDATATOPICDETAILS_HPP
