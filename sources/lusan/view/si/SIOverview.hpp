#ifndef LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
#define LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIOverview.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/OverviewPage.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class SIOverviewModel;
class QRadioButton;

/**
 * \brief   The Service Interface Overview page: the shared \ref OverviewPage plus the service
 *          category, which only an interface declares.
 *
 *          The name is shown but not edited here: an interface is named by the file it lives in,
 *          and a save writes that name back into the document.
 **/
class SIOverview : public OverviewPage
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SIOverview(SIOverviewModel& model, QWidget* parent = nullptr);

    virtual ~SIOverview(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Fills the shared rows and the service category.
     **/
    virtual void refreshAll(void) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onPublicToggled(bool checked);
    void onPrivateToggled(bool checked);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Adds the category row under the name, where the interface has always shown it.
    void buildCategoryRow(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SIOverviewModel&    mModel;     //!< The Overview of the interface being edited.
    QRadioButton*       mPublic;
    QRadioButton*       mPrivate;
    QRadioButton*       mInternet;
};

#endif // LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
