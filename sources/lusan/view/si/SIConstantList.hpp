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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIConstantList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface, Data Topic section.
 *
 ************************************************************************/
#include <QWidget>

namespace Ui {
    class SIConstantList;
}

class QToolButton;
class QTableWidget;

class SIConstantList : public QWidget
{
    Q_OBJECT

public:
    explicit SIConstantList(QWidget* parent = nullptr);

    QToolButton* ctrlButtonAdd(void);

    QToolButton* ctrlButtonUpdate(void);

    QToolButton* ctrlButtonRemove(void);

    QToolButton* ctrlButtonInsert(void);

    QToolButton* ctrlButtonMoveUp(void);

    QToolButton* ctrlButtonMoveDown(void);

    QTableWidget* ctrlTableList(void);

private:
    Ui::SIConstantList* ui;
};

#endif // LUSAN_APPLICATION_SI_SICONSTANTLIST_HPP
