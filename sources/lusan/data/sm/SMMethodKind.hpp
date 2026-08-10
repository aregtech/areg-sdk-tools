#ifndef LUSAN_DATA_SM_SMMETHODKIND_HPP
#define LUSAN_DATA_SM_SMMETHODKIND_HPP
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
 *  \file        lusan/data/sm/SMMethodKind.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the state machine's names for the shared method kinds.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/MethodDataSection.hpp"

/**
 * \brief   A method is one class with a configured kind, so the kind a state machine means is an
 *          index. These read it back in the machine's own words: a trigger is a stimulus the
 *          machine offers, an action is a callback it invokes, and a condition answers a guard --
 *          either from the author's handler or from a body written here.
 **/
namespace NESMMethod
{
    inline bool isTrigger(const MethodEntry* method)
    {
        return (method != nullptr) && (method->getKind() == NEMethod::SmTrigger);
    }

    inline bool isAction(const MethodEntry* method)
    {
        return (method != nullptr) && (method->getKind() == NEMethod::SmAction);
    }

    inline bool isCondition(const MethodEntry* method)
    {
        return (method != nullptr) && (method->getKind() == NEMethod::SmCondition);
    }

    inline bool isLambdaCondition(const MethodEntry* method)
    {
        return isCondition(method) && method->isEmbedded();
    }

    inline bool isHandlerCondition(const MethodEntry* method)
    {
        return isCondition(method) && (method->isEmbedded() == false);
    }
}

#endif  // LUSAN_DATA_SM_SMMETHODKIND_HPP
