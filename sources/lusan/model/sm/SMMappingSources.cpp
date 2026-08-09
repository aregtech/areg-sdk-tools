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
#include "lusan/data/common/ConstantDataSection.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"

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
    const SMDocumentIndex index(data);

    switch (kind)
    {
    case SMArgumentEntry::eValueSource::Param:
        for (const MethodParameter& param : index.paramScope(transitionId).parameters())
        {
            offer(result, param.getName(), param.getType(), targetType);
        }
        break;

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
        for (const SMMethodEntry* method : index.methodsOf(SMMethodEntry::eMethodType::Condition))
        {
            if (method->getElements().isEmpty())
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

    return (transitionId != 0u) && (SMDocumentIndex(data).paramScope(transitionId).isEmpty() == false);
}

QString SMMappingSources::referencedType( const StateMachineData& data
                                        , uint32_t transitionId
                                        , SMArgumentEntry::eValueSource kind
                                        , const QString& name)
{
    const SMDocumentIndex index(data);

    switch (kind)
    {
    case SMArgumentEntry::eValueSource::Param:
    {
        const MethodParameter* param = index.paramScope(transitionId).byName(name);
        return (param != nullptr) ? param->getType() : QString();
    }

    case SMArgumentEntry::eValueSource::Attribute:
    {
        const SMAttributeEntry* attr = index.attribute(name);
        return (attr != nullptr) ? attr->getType() : QString();
    }

    case SMArgumentEntry::eValueSource::Constant:
    {
        const ConstantEntry* constant = index.constant(name);
        return (constant != nullptr) ? constant->getType() : QString();
    }

    case SMArgumentEntry::eValueSource::Condition:
    {
        // By bare name, not by kind: the row stores what the picker offered, and a stale name
        // must still resolve to whatever declaration still carries it.
        const SMMethodEntry* method = index.method(name);
        return (method != nullptr) ? method->getReturn() : QString();
    }

    default:
        return QString();
    }
}
