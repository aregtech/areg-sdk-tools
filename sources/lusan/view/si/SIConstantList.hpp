#ifndef LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
#define LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
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
 *  \file        lusan/view/si/SIConstantList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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

class QKeySequence;
class QString;
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
    QToolButton* ctrlButtonAdd();

    /**
     * \brief   Returns remove button object.
     **/
    QToolButton* ctrlButtonRemove();

    /**
     * \brief   Returns insert button object.
     **/
    QToolButton* ctrlButtonInsert();
    
    /**
     * \brief   Returns move up button object.
     **/
    QToolButton* ctrlButtonMoveUp();

    /**
     * \brief   Returns move down button object.
     **/
    QToolButton* ctrlButtonMoveDown();

    /**
     * \brief   Returns the table widget object.
     **/
    QTableWidget* ctrlTableList();

private:
    void buildUi();
    QToolButton* createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut);

private:
    QTableWidget* mTable;
    QToolButton*  mButtonAdd;
    QToolButton*  mButtonRemove;
    QToolButton*  mButtonInsert;
    QToolButton*  mButtonMoveUp;
    QToolButton*  mButtonMoveDown;
};

#endif // LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
