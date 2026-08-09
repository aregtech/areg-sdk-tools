#ifndef LUSAN_VIEW_SM_SMCONSTANT_HPP
#define LUSAN_VIEW_SM_SMCONSTANT_HPP
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
 *  \file        lusan/view/sm/SMConstant.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/ConstantPage.hpp"

#include "lusan/data/sm/SMReferences.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;

/**
 * \brief   The FSM Constants page. The editing itself is the shared \ref ConstantPage; what a
 *          state machine adds is knowing where a constant is used -- the where-used popup, the
 *          search seed for the canvas, and the warning before deleting something a guard or an
 *          action still refers to.
 **/
class SMConstant : public ConstantPage
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMConstant(ConstantModel& model, StateMachineModel& facade, QWidget* parent = nullptr);

    virtual ~SMConstant(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Shows the where-used popup for the currently selected constant (Find Usages /
     *          Shift+F12). Shows an information box if no constant is selected.
     **/
    void whereUsedForCurrent(void);

    /**
     * \brief   Fills the search seed (kind/id/name) for the currently selected constant.
     *          False when no constant is selected.
     **/
    bool currentReference(SMReferences::eTarget& target, uint32_t& id, QString& name) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Lists where the constant is still used and lets the author decide.
     **/
    virtual bool confirmRemove(uint32_t id) override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;    //!< The document, for the where-used search.
};

#endif  // LUSAN_VIEW_SM_SMCONSTANT_HPP
