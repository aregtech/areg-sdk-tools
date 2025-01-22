#ifndef LUSAN_APPLICATION_SI_SIINCLUDEDETAILS_HPP
#define LUSAN_APPLICATION_SI_SIINCLUDEDETAILS_HPP
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
 *  \file        lusan/view/si/SIIncludeDetails.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
namespace Ui {
    class SIIncludeDetails;
}
    
class QPushButton;
class QCheckBox;
class QLineEdit;
class QPlainTextEdit;

//////////////////////////////////////////////////////////////////////////
// SIIncludeDetails class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The Service Interface Include file details page.
 **/
class SIIncludeDetails : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor.
     * \param   parent  The parent widget.
     **/
    explicit SIIncludeDetails(QWidget* parent = nullptr);

    virtual ~SIIncludeDetails(void);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the selected file.
     **/
    QString getSelectedFile(void) const;

    /**
     * \brief   Returns the description of the selected file.
     **/
    QString getDescription(void) const;

    /**
     * \brief   Returns true if the selected file is deprecated.
     **/
    bool isDeprecated(void) const;

    /**
     * \brief   Returns the hint of the deprecated file.
     **/
    QString getDeprecateHint(void) const;

    /**
     * \brief   Returns the control of the include file path.
     **/
    QLineEdit * ctrlInclude(void);

    /**
     * \brief   Returns the control of the deprecation hint.
     **/
    QLineEdit * ctrlDeprecateHint(void);

    /**
     * \brief   Returns the control of the deprecation flag.
     **/
    QCheckBox * ctrlDeprecated(void);

    /**
     * \brief   Returns the control of the description.
     **/
    QPlainTextEdit * ctrlDescription(void);

    /**
     * \brief   Returns the control of the browse button.
     **/
    QPushButton * ctrlBrowseButton(void);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::SIIncludeDetails*   ui; //!< The user interface object.

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeDetails(const SIIncludeDetails& /*src*/) = delete;
    SIIncludeDetails& operator = (const SIIncludeDetails& /*src*/) = delete;
    SIIncludeDetails(SIIncludeDetails&& /*src*/) = delete;
    SIIncludeDetails& operator = (SIIncludeDetails&& /*src*/) = delete;
};

#endif // LUSAN_APPLICATION_SI_SIINCLUDEDETAILS_HPP
