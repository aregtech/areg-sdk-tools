#ifndef LUSAN_APPLICATION_SI_SIDATATOPIC_HPP
#define LUSAN_APPLICATION_SI_SIDATATOPIC_HPP
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
 *  \file        lusan/view/si/SIDataTopic.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QScrollArea>

namespace Ui {
    class SIDataTopic;
}

class SIDataTopicDetails;
class SIDataTopicList;

class SIDataTopicWidget : public QWidget
{
    friend class SIDataTopic;

    Q_OBJECT

public:
    explicit SIDataTopicWidget(QWidget* parent);

private:
    Ui::SIDataTopic* ui;
};

class SIDataTopic : public QScrollArea
{
    Q_OBJECT

public:
    explicit SIDataTopic(QWidget* parent = nullptr);

    virtual ~SIDataTopic(void);

private:
    SIDataTopicDetails* mTopicDetails;
    SIDataTopicList*    mTopicList;
    SIDataTopicWidget*  mWidget;
    Ui::SIDataTopic&    ui;
};

#endif // LUSAN_APPLICATION_SI_SIDATATOPIC_HPP
