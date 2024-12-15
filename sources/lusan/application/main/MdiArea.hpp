#ifndef LUSAN_APPLICATION_MAIN_MDIAREA_HPP
#define LUSAN_APPLICATION_MAIN_MDIAREA_HPP
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
 *  \file        lusan/application/main/MdiArea.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include <QMdiArea>
#include <QWidget>

class MdiArea : public QMdiArea
{
    Q_OBJECT
public:
    MdiArea(QWidget * parent = nullptr);

public:
    inline QTabWidget& getTabControl(void);
private:
    QTabWidget  mTabWidget;
};

inline QTabWidget& MdiArea::getTabControl(void)
{
    return mTabWidget;
}

#endif // MDIAREA_HPP
