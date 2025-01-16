#ifndef LUSAN_APPLICATION_SI_SIMETHODDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIMETHODDETAILS_HPP
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
 *  \file        lusan/view/si/SIMethodDetails.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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

namespace Ui {
    class SIMethodDetails;
}

class SIMethodDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SIMethodDetails(QWidget* parent = nullptr);
    
    QLineEdit * ctrlName(void) const;
    
    QRadioButton * ctrlBroadcast(void) const;
    
    QRadioButton * ctrlRequest(void) const;
    
    QRadioButton * ctrlResponse(void) const;
    
    QComboBox * ctrlConnectedResponse(void) const;
    
    QPlainTextEdit * ctrlDescription(void) const;
    
    QCheckBox * ctrlIsDeprecated(void) const;
    
    QLineEdit * ctrlDeprecateHint(void) const;

private:
    Ui::SIMethodDetails* ui;
};

#endif // LUSAN_APPLICATION_SI_SIMETHODDETAILS_HPP
