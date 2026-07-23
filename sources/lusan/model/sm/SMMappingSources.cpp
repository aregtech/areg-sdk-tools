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
 *  \file        lusan/model/sm/SMMappingSources.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM parameter-mapping value-source enumeration.
 *
 ************************************************************************/

#include "lusan/model/sm/SMMappingSources.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

namespace
{
    //!< Appends \p name : \p type ranked against \p targetType, unless the fit is Mismatch.
    void offer(QList<SMSourceEntry>& out, const QString& name, const QString& type, const QString& targetType)
    {
        const SMTypeCompat::eRank rank = SMTypeCompat::rank(type, targetType);
        if (rank != SMTypeCompat::eRank::Mismatch)
        {
            out.append(SMSourceEntry{ name, type, rank });
        }
    }
}

QList<SMSourceEntry> SMMappingSources::candidates( const StateMachineData& data
                                                 , uint32_t transitionId
                                                 , SMArgumentEntry::eValueSource kind
                                                 , const QString& targetType)
{
    QList<SMSourceEntry> result;

    switch (kind)
    {
    case SMArgumentEntry::eValueSource::Param:
    {
        const QStringList names = SMGuardSymbols::paramNames(data, transitionId);
        const QStringList types = SMGuardSymbols::paramTypes(data, transitionId);
        for (int i = 0; i < names.size(); ++i)
        {
            offer(result, names.at(i), (i < types.size()) ? types.at(i) : QString(), targetType);
        }
        break;
    }

    case SMArgumentEntry::eValueSource::Attribute:
        for (const SMAttributeEntry& attr : data.getAttributes().getElements())
        {
            offer(result, attr.getName(), attr.getType(), targetType);
        }
        break;

    case SMArgumentEntry::eValueSource::Constant:
        for (const ConstantEntry& constant : data.getConstants().getElements())
        {
            offer(result, constant.getName(), constant.getType(), targetType);
        }
        break;

    case SMArgumentEntry::eValueSource::Condition:
        // A Condition source is a parameterless condition-method call.
        for (const SMMethodEntry* method : data.getMethods().getElements())
        {
            if ((method != nullptr) && method->isCondition() && method->getElements().isEmpty())
            {
                offer(result, method->getName(), method->getReturn(), targetType);
            }
        }
        break;

    default:
        break;
    }

    return result;
}

bool SMMappingSources::isKindLegal(const StateMachineData& data, uint32_t transitionId, SMArgumentEntry::eValueSource kind)
{
    if (kind != SMArgumentEntry::eValueSource::Param)
    {
        return true;
    }

    return (transitionId != 0u) && (SMGuardSymbols::paramNames(data, transitionId).isEmpty() == false);
}

QString SMMappingSources::referencedType( const StateMachineData& data
                                        , uint32_t transitionId
                                        , SMArgumentEntry::eValueSource kind
                                        , const QString& name)
{
    switch (kind)
    {
    case SMArgumentEntry::eValueSource::Param:
    {
        const QStringList names = SMGuardSymbols::paramNames(data, transitionId);
        const QStringList types = SMGuardSymbols::paramTypes(data, transitionId);
        const int index = names.indexOf(name);
        return ((index >= 0) && (index < types.size())) ? types.at(index) : QString();
    }

    case SMArgumentEntry::eValueSource::Attribute:
        for (const SMAttributeEntry& attr : data.getAttributes().getElements())
        {
            if (attr.getName() == name) { return attr.getType(); }
        }
        return QString();

    case SMArgumentEntry::eValueSource::Constant:
        for (const ConstantEntry& constant : data.getConstants().getElements())
        {
            if (constant.getName() == name) { return constant.getType(); }
        }
        return QString();

    case SMArgumentEntry::eValueSource::Condition:
        for (const SMMethodEntry* method : data.getMethods().getElements())
        {
            if ((method != nullptr) && (method->getName() == name)) { return method->getReturn(); }
        }
        return QString();

    default:
        return QString();
    }
}
