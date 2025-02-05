#ifndef LUSAN_APPLICATION_SI_SIOVERVIEWLINKS_HPP
#define LUSAN_APPLICATION_SI_SIOVERVIEWLINKS_HPP
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
 *  \file        lusan/view/si/SIOverviewLinks.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QWidget>

class QPushButton;
namespace Ui {
    class SIOverviewLinks;
}

class SIOverviewLinks : public QWidget
{
    Q_OBJECT

public:
    explicit SIOverviewLinks(QWidget* parent = nullptr);

    /**
     * \brief   Link to the data types tabbed page.
     **/
    QPushButton* linkDataTypes(void) const;

    /**
     * \brief   Link to the attributes tabbed page.
     **/
    QPushButton* linkAttributes(void) const;

    /**
     * \brief   Link to the methods tabbed page.
     **/
    QPushButton* linkMethods(void) const;

    /**
     * \brief   Link to the constants tabbed page.
     **/
    QPushButton* linkConstants(void) const;

    /**
     * \brief   Link to the includes tabbed page.
     **/
    QPushButton* linkIncludes(void) const;

private:
    Ui::SIOverviewLinks* ui;
};

#endif // LUSAN_APPLICATION_SI_SIOVERVIEWLINKS_HPP
