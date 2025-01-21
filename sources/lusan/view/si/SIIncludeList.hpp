#ifndef LUSAN_APPLICATION_SI_SIINCLUDELIST_HPP
#define LUSAN_APPLICATION_SI_SIINCLUDELIST_HPP
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
 *  \file        lusan/view/si/SIIncludeList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface, Data Topic section.
 *
 ************************************************************************/
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
namespace Ui {
    class SIIncludeList;
}

class QTableWidget;
class QToolButton;
class SIIncludeModel;

//////////////////////////////////////////////////////////////////////////
// SIIncludeList class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The Service Interface Include list page.
 **/
class SIIncludeList : public QWidget
{
    friend class SIInclude;
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SIIncludeList(SIIncludeModel & model, QWidget* parent = nullptr);
    
//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Returns the control of the add entry tool button.
     **/
    QToolButton* ctrlButtonAdd(void) const;

    /**
     * \brief   Returns the control of the remove entry tool button.
     **/
    QToolButton* ctrlButtonRemove(void) const;

    /**
     * \brief   Returns the control of the up entry tool button.
     **/
    QToolButton* ctrlButtonMoveUp(void) const;

    /**
     * \brief   Returns the control of the down entry tool button.
     **/
    QToolButton* ctrlButtonMoveDown(void) const;

    /**
     * \brief   Returns the control of the insert entry tool button.
     **/
    QToolButton* ctrlButtonInsert(void) const;

    /**
     * \brief   Returns the control of the includes table widget.
     **/
    QTableWidget* ctrlTableList(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::SIIncludeList*  ui;     //!< The user interface object.
    SIIncludeModel&     mModel; //!< The model of the include list.

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeList(const SIIncludeList& /*src*/) = delete;
    SIIncludeList& operator = (const SIIncludeList& /*src*/) = delete;
    SIIncludeList(SIIncludeList&& /*src*/) = delete;
    SIIncludeList& operator = (SIIncludeList&& /*src*/) = delete;
};

#endif // LUSAN_APPLICATION_SI_SIINCLUDELIST_HPP
