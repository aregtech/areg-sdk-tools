#ifndef LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
#define LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
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
 *  \file        lusan/view/si/SIConstantList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface, Data Attribute section.
 *
 ************************************************************************/
/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/

namespace Ui {
    class SIConstantList;
}

class QToolButton;
class QTableWidget;

//////////////////////////////////////////////////////////////////////////
// SIConstantList class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The SIConstantList class is a widget to display the list of constants.
 *          The widget contains the list of constants, and buttons to add, remove,
 *          insert, move up, and move down the constants in the list.
 **/
class SIConstantList : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Public members
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Default constructor.
     * \param   parent  The parent widget.
     **/
    explicit SIConstantList(QWidget* parent = nullptr);

    /**
     * \brief   Returns add button object.
     **/
    QToolButton* ctrlButtonAdd(void);

    /**
     * \brief   Returns remove button object.
     **/
    QToolButton* ctrlButtonRemove(void);

    /**
     * \brief   Returns insert button object.
     **/
    QToolButton* ctrlButtonInsert(void);
    
    /**
     * \brief   Returns move up button object.
     **/
    QToolButton* ctrlButtonMoveUp(void);

    /**
     * \brief   Returns move down button object.
     **/
    QToolButton* ctrlButtonMoveDown(void);

    /**
     * \brief   Returns the table widget object.
     **/
    QTableWidget* ctrlTableList(void);

private:
    Ui::SIConstantList* ui; //!< The user interface object.
};

#endif // LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
