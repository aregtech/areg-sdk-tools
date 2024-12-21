#ifndef LUSAN_APPLICATION_SI_SIMETHOD_HPP
#define LUSAN_APPLICATION_SI_SIMETHOD_HPP
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
 *  \file        lusan/view/si/SIMethod.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QScrollArea>

class SIMethodDetails;
class SIMethodList;
class SIMethodParamDetails;

namespace Ui {
    class SIMethod;
}

class SIMethodWidget : public QWidget
{
    friend class SIMethod;

    Q_OBJECT

public:
    explicit SIMethodWidget(QWidget* parent);

private:
    Ui::SIMethod* ui;
};

class SIMethod : public QScrollArea
{
    Q_OBJECT

public:
    explicit SIMethod(QWidget* parent = nullptr);

    virtual ~SIMethod(void);

private:
    SIMethodDetails*        mMethodDetails;
    SIMethodList*           mMethodList;
    SIMethodParamDetails*   mParamDetails;
    SIMethodWidget*         mWidget;
    Ui::SIMethod&          ui;
};

#endif // LUSAN_APPLICATION_SI_SIMETHOD_HPP
