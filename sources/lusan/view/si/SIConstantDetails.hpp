#ifndef LUSAN_APPLICATION_SI_SICONSTANTDETAILS_HPP
#define LUSAN_APPLICATION_SI_SICONSTANTDETAILS_HPP
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
 *  \file        lusan/view/si/SIConstantDetails.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;

//////////////////////////////////////////////////////////////////////////
// SIConstantDetails class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The Service Interface Constant Details widget.
 *          The widget is used to display and edit the details of the constant.
 **/
class SIConstantDetails : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Public members
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent widget of the constant details.
     **/
    explicit SIConstantDetails(QWidget* parent = nullptr);

    /**
     * \brief   Returns the constant name widget object.
     **/
    QLineEdit* ctrlName();

    /**
     * \brief   Returns the constant type widget object.
     **/
    QComboBox* ctrlTypes();

    /**
     * \brief   Returns the constant value widget object.
     **/
    QLineEdit* ctrlValue();

    /**
     * \brief   Returns the constant description widget object.
     **/
    QPlainTextEdit* ctrlDescription();

    /**
     * \brief   Returns the constant deprecated flag widget object.
     **/
    QCheckBox* ctrlDeprecated();

    /**
     * \brief   Returns the constant deprecation hint widget object.
     **/
    QLineEdit* ctrlDeprecateHint();

private:
    void buildUi();

private:
    QLineEdit*      mName;
    QComboBox*      mTypes;
    QLineEdit*      mValue;
    QPlainTextEdit* mDescription;
    QCheckBox*      mDeprecated;
    QLineEdit*      mDeprecateHint;
};

#endif // LUSAN_APPLICATION_SI_SICONSTANTDETAILS_HPP
