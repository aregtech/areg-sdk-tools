#ifndef LUSAN_APPLICATION_SI_SIMETHODLIST_HPP
#define LUSAN_APPLICATION_SI_SIMETHODLIST_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIMethodList.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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
    
    QToolButton * ctrlButtonInsert(void) const;
    
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
