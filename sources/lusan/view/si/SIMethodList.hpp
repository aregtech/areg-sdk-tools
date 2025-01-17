#ifndef LUSAN_APPLICATION_SI_SIMETHODLIST_HPP
#define LUSAN_APPLICATION_SI_SIMETHODLIST_HPP
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
 *  \file        lusan/view/si/SIMethodList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

class QToolButton;
class QTreeWidget;

namespace Ui {
    class SIMethodList;
}

class SIMethodList : public QWidget
{
    Q_OBJECT

public:
    explicit SIMethodList(QWidget* parent = nullptr);
    
    QToolButton * ctrlButtonAdd(void) const;
    
    QToolButton * ctrlButtonRemove(void) const;
    
    QToolButton * ctrlButtonParamAdd(void) const;
    
    QToolButton * ctrlButtonParamRemove(void) const;
    
    QToolButton * ctrlButtonParamInsert(void) const;
    
    QToolButton * ctrlButtonMoveUp(void) const;
    
    QToolButton * ctrlButtonMoveDown(void) const;
    
    QTreeWidget * ctrlTableList(void) const;

private:
    Ui::SIMethodList* ui;
};

#endif // LUSAN_APPLICATION_SI_SIMETHODLIST_HPP
