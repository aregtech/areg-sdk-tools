#ifndef LUSAN_APPLICATION_SI_SIOVERVIEWLINKS_HPP
#define LUSAN_APPLICATION_SI_SIOVERVIEWLINKS_HPP
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
 *  \file        lusan/view/si/SIOverviewLinks.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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
