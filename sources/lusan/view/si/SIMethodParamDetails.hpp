#ifndef LUSAN_APPLICATION_SI_SIMETHODPARAMDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIMETHODPARAMDETAILS_HPP
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
 *  \file        lusan/view/si/SIMethodParamDetails.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit ;

namespace Ui {
    class SIMethodParamDetails;
}

class SIMethodParamDetails : public QWidget
{
    Q_OBJECT

public:
    explicit SIMethodParamDetails(QWidget* parent = nullptr);
    
    QLineEdit * ctrlParamName(void) const;
    
    QComboBox * ctrlParamTypes(void) const;

    QCheckBox* ctrlParamHasDefault(void) const;
    
    QLineEdit * ctrlParamDefaultValue(void) const;
        
    QPlainTextEdit * ctrlParamDescription(void) const;
    
    QCheckBox * ctrlDeprecated(void) const;
    
    QLineEdit * ctrlDeprecateHint(void) const;

private:
    Ui::SIMethodParamDetails* ui;
};

#endif // LUSAN_APPLICATION_SI_SIMETHODPARAMDETAILS_HPP
