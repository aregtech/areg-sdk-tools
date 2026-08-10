#ifndef LUSAN_VIEW_SM_SMOVERVIEW_HPP
#define LUSAN_VIEW_SM_SMOVERVIEW_HPP
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
 *  \file        lusan/view/sm/SMOverview.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/OverviewPage.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMOverviewModel;
class QRadioButton;

/**
 * \brief   The FSM Overview page: the shared \ref OverviewPage plus the threading mode, which
 *          only a state machine declares.
 **/
class SMOverview : public OverviewPage
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMOverview(SMOverviewModel& model, QWidget* parent = nullptr);

    virtual ~SMOverview(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Fills the shared rows and the threading mode.
     **/
    virtual void refreshAll(void) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onThreadingToggled(bool checked);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Adds the threading row under the name, where the state machine has always shown it.
    void buildThreadingRow(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMOverviewModel&    mModel;     //!< The Overview of the state machine being edited.
    QRadioButton*       mShared;
    QRadioButton*       mLocal;
};

#endif  // LUSAN_VIEW_SM_SMOVERVIEW_HPP
