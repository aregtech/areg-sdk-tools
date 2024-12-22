#ifndef LUSAN_APPLICATION_SI_SIINCLUDE_HPP
#define LUSAN_APPLICATION_SI_SIINCLUDE_HPP
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
 *  \file        lusan/view/si/SIInclude.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QScrollArea>

namespace Ui {
    class SIInclude;
}

class SIIncludeDetails;
class SIIncludeList;

class SIIncludeWidget : public QWidget
{
    friend class SIInclude;

    Q_OBJECT

public:
    explicit SIIncludeWidget(QWidget* parent);

private:
    Ui::SIInclude* ui;
};

class SIInclude : public QScrollArea
{
    Q_OBJECT

public:
    explicit SIInclude(QWidget* parent = nullptr);

    virtual ~SIInclude(void);

private:
    SIIncludeDetails*  mTopicDetails;
    SIIncludeList*     mTopicList;
    SIIncludeWidget*   mWidget;
    Ui::SIInclude&     ui;
};

#endif // LUSAN_APPLICATION_SI_SIINCLUDE_HPP
