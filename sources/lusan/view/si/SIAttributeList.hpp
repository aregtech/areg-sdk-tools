#ifndef LUSAN_APPLICATION_SI_SIATTRIBUTELIST_HPP
#define LUSAN_APPLICATION_SI_SIATTRIBUTELIST_HPP
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
 *  \file        lusan/view/si/SIAttributeList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface, Data Attribute section.
 *
 ************************************************************************/
#include <QWidget>

class QKeySequence;
class QString;
class QToolButton;
class QTableWidget;

class SIAttributeList : public QWidget
{
    Q_OBJECT

public:
    explicit SIAttributeList(QWidget* parent = nullptr);

    // Getters to access controls

    QToolButton * ctrlButtonAdd();

    QToolButton* ctrlButtonRemove();

    QToolButton* ctrlButtonInsert();

    QToolButton* ctrlButtonMoveUp();

    QToolButton* ctrlButtonMoveDown();

    QTableWidget* ctrlTableList();

private:
    void buildUi();

private:
    QTableWidget* mTable;
    QToolButton*  mButtonAdd;
    QToolButton*  mButtonRemove;
    QToolButton*  mButtonInsert;
    QToolButton*  mButtonMoveUp;
    QToolButton*  mButtonMoveDown;
};

#endif // LUSAN_APPLICATION_SI_SIATTRIBUTELIST_HPP
