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

namespace Ui {
    class SIIncludeList;
}

class SIIncludeList : public QWidget
{
    Q_OBJECT

public:
    explicit SIIncludeList(QWidget* parent = nullptr);

private:
    Ui::SIIncludeList* ui;
};

#endif // LUSAN_APPLICATION_SI_SIINCLUDELIST_HPP
