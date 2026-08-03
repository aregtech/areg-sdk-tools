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
 *  \file        lusan/model/sm/SMWhereUsed.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM where-used union.
 *
 ************************************************************************/

#include "lusan/model/sm/SMWhereUsed.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"

QList<SMReferences::Use> SMWhereUsed::collect(const StateMachineData& data, SMReferences::eTarget target, const QString& name, uint32_t id)
{
    // Name-based and transition-target references from the headless data-layer walker.
    QList<SMReferences::Use> out = SMReferences::whereUsed(data, target, name, id);

    // Guard trees bind method/attribute/constant symbols by ID; that reference knowledge
    // lives once in SMGuardWhereUsed, so union rather than re-scan the trees here.
    if ((target == SMReferences::eTarget::Condition)
        || (target == SMReferences::eTarget::Attribute)
        || (target == SMReferences::eTarget::Constant))
    {
        for (const SMGuardWhereUsed::Use& u : SMGuardWhereUsed::symbolUses(data, id))
        {
            // `isState` says which registry the id belongs to: a Do stop condition is owned by
            // its state, a transition guard by the transition.
            out.append({ u.target.getId(), (u.target.getOwner() == SMGuardRef::eOwner::DoActivity), u.location });
        }
    }

    return out;
}
