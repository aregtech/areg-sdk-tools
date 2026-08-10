#ifndef LUSAN_VIEW_SM_SMMETHOD_HPP
#define LUSAN_VIEW_SM_SMMETHOD_HPP
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
 *  \file        lusan/view/sm/SMMethod.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the state machine's Methods page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/MethodPage.hpp"

#include "lusan/data/sm/SMReferences.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;

/**
 * \class   SMMethod
 * \brief   The shared Methods page plus what only a state machine can answer: its guards,
 *          actions and operations name a method, so the page can say where a method is used,
 *          seed the canvas search with it, refuse to delete a condition a guard still names,
 *          and report a trigger name that collides with an event or a timer.
 *
 *          Everything else -- the list, both forms, the undo commands, the parameters and the
 *          inline editing -- is \ref MethodPage.
 **/
class SMMethod : public MethodPage
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMMethod(MethodModel& model, StateMachineModel& facade, QWidget* parent = nullptr);

    virtual ~SMMethod(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Shows the where-used popup for the selected method (Find Usages / Shift+F12).
     *          Says so and does nothing else when no method is selected.
     **/
    void whereUsedForCurrent(void);

    /**
     * \brief   Fills the search seed (kind, ID and name) for the selected method, so Ctrl+F
     *          searches that method's usages. False when no method is selected.
     **/
    bool currentReference(SMReferences::eTarget& target, uint32_t& id, QString& name) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Refuses to delete a condition an ID-bound guard still names, and asks before
     *          deleting a method something else refers to by name.
     **/
    virtual bool confirmRemove(uint32_t id) override;

    /**
     * \brief   Adds the shared stimulus name space to the shared per-kind duplicate check: a
     *          trigger shares its names with the machine's events and timers.
     **/
    virtual QString nameCollisionReason(const MethodEntry* method, const QString& name, uint32_t selfId) const override;

    /**
     * \brief   Shows a condition's implementation mode as a glyph and a word in the type column.
     **/
    virtual void decorateMethodNode(QTreeWidgetItem* node, const MethodEntry& method) const override;

    /**
     * \brief   Fills the guard-use line of a condition, and hides it for anything else.
     **/
    virtual void updateExtraFields(MethodEntry* method) override;

    /**
     * \brief   Adds the note and the completion words the body editor offers, on top of the
     *          signature the shared page sets.
     **/
    virtual void updateBodyEditor(MethodEntry* method) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< The reference target of the given method: a trigger, an action or a condition.
    SMReferences::eTarget targetOf(const MethodEntry& method) const;

    //!< The `used by N guards` popup: pick a usage, select what it belongs to.
    void showMethodWhereUsed(uint32_t methodId);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;    //!< The document, for its guards and its name spaces.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    SMMethod(const SMMethod& /*src*/) = delete;
    SMMethod& operator = (const SMMethod& /*src*/) = delete;
};

#endif  // LUSAN_VIEW_SM_SMMETHOD_HPP
