#ifndef LUSAN_APPLICATION_SI_SIDATATYPEDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIDATATYPEDETAILS_HPP
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
 *  \file        lusan/view/si/SIDataTypeDetails.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>
#include <utility>

namespace Ui {
    class SIDataTypeDetails;
}
    
class QCheckBox;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;

class SIDataTypeDetails : public QWidget
{
    Q_OBJECT

public:
    using CtrlGroup = std::pair<QLabel*, QGroupBox*>;

public:
    explicit SIDataTypeDetails(QWidget *parent = nullptr);
    
    QLineEdit* ctrlName(void) const;
    
    QRadioButton* ctrlTypeStruct(void)  const;
    
    QRadioButton* ctrlTypeEnum(void)  const;
    
    QRadioButton* ctrlTypeImport(void) const;
    
    QRadioButton* ctrlContainer(void) const;
    
    QComboBox* ctrlContainerObject(void) const;
    
    QComboBox* ctrlContainerKey(void) const;
    
    QComboBox* ctrlContainerValue(void) const;
    
    QPlainTextEdit* ctrlDescription(void) const;
    
    QCheckBox* ctrlDeprecated(void) const;
    
    QLineEdit* ctrlDeprecateHint(void) const;
    
    QComboBox* ctrlEnumDerived(void) const;
    
    QLineEdit* ctrlImportLocation(void) const;
    
    QPushButton* ctrlButtonBrowse(void) const;
    
    QLineEdit* ctrlImportNamespace(void) const;

    SIDataTypeDetails::CtrlGroup ctrlDetailsEnum(void) const;

    SIDataTypeDetails::CtrlGroup ctrDetailsImport(void) const;

    SIDataTypeDetails::CtrlGroup ctrlDetailsContainer(void) const;

    QFormLayout* ctrlLayout(void) const;
    
private:
    Ui::SIDataTypeDetails* ui;
};

#endif // LUSAN_APPLICATION_SI_SIDATATYPEDETAILS_HPP
