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
 *  \file        lusan/model/sm/SMGuardSymbols.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard symbol resolution (name <-> document ID).
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardSymbols.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"

#include <QHash>

SMGuardSymbols::eScoped SMGuardSymbols::scopedValue(const StateMachineData& data, const QStringList& parts, QString& typeNameOut)
{
    typeNameOut.clear();
    if (parts.size() < 2)
    {
        return eScoped::NoType;
    }

    const DataTypeCustom* type = data.getDataTypes().findCustomDataType(parts.first());
    if (type == nullptr)
    {
        return eScoped::NoType;
    }

    typeNameOut = type->getName();
    if (type->isImported())
    {
        // An imported type is a name borrowed from a foreign header: the document declares that the
        // name exists and nothing else, so judging what hangs off it would be inventing knowledge.
        return eScoped::Opaque;
    }

    // Only one level is ours to check: `Enum::value` and `Struct::field`. A deeper chain reaches
    // into a member's own type, which the registry does not model.
    if (parts.size() > 2)
    {
        return eScoped::NoMember;
    }

    if (type->isEnumeration())
    {
        const DataTypeEnum* enumType = static_cast<const DataTypeEnum*>(type);
        return enumType->hasElement(parts.at(1)) ? eScoped::Ok : eScoped::NoMember;
    }

    if (type->isStructure())
    {
        const DataTypeStructure* structType = static_cast<const DataTypeStructure*>(type);
        return structType->hasElement(parts.at(1)) ? eScoped::Ok : eScoped::NoMember;
    }

    return eScoped::NoMember;
}

QStringList SMGuardSymbols::scopedMembers(const StateMachineData& data, const QString& typeName)
{
    QStringList members;
    const DataTypeCustom* type = data.getDataTypes().findCustomDataType(typeName);
    if (type == nullptr)
    {
        return members;
    }

    if (type->isEnumeration())
    {
        for (const EnumEntry& entry : static_cast<const DataTypeEnum*>(type)->getElements())
        {
            members.append(entry.getName());
        }
    }
    else if (type->isStructure())
    {
        for (const FieldEntry& field : static_cast<const DataTypeStructure*>(type)->getElements())
        {
            members.append(field.getName());
        }
    }

    return members;
}

bool SMGuardSymbols::conditionBindsBare(const SMMethodEntry& method)
{
    for (const MethodParameter& param : method.getElements())
    {
        if (param.hasDefault() == false)
        {
            return false;
        }
    }

    return true;
}

SMGuardSymbols::BareResult SMGuardSymbols::bindBare(const StateMachineData& data, uint32_t transitionId, const QString& name, eSurface surface)
{
    BareResult result;

    const uint32_t pid = paramId(data, transitionId, name);
    if (pid != 0u)
    {
        result.bind = eBind::Param;
        result.id   = pid;
        return result;
    }

    if (surface == eSurface::Guard)
    {
        const SMMethodEntry* cond = conditionMethod(data, name);
        if ((cond != nullptr) && conditionBindsBare(*cond))
        {
            result.bind = eBind::Condition;
            result.id   = cond->getId();
            return result;
        }
    }

    const uint32_t aid = attributeId(data, name);
    if (aid != 0u)
    {
        result.bind = eBind::Attribute;
        result.id   = aid;
        return result;
    }

    const uint32_t cid = constantId(data, name);
    if (cid != 0u)
    {
        result.bind = eBind::Constant;
        result.id   = cid;
        return result;
    }

    return result;
}

uint32_t SMGuardSymbols::attributeId(const StateMachineData& data, const QString& name)
{
    const SMAttributeEntry* attribute = SMDocumentIndex(data).attribute(name);
    return (attribute != nullptr) ? attribute->getId() : 0u;
}

uint32_t SMGuardSymbols::constantId(const StateMachineData& data, const QString& name)
{
    const ConstantEntry* constant = SMDocumentIndex(data).constant(name);
    return (constant != nullptr) ? constant->getId() : 0u;
}

uint32_t SMGuardSymbols::paramId(const StateMachineData& data, uint32_t transitionId, const QString& name)
{
    const MethodParameter* param = SMDocumentIndex(data).paramScope(transitionId).byName(name);
    return (param != nullptr) ? param->getId() : 0u;
}

const SMMethodEntry* SMGuardSymbols::conditionMethod(const StateMachineData& data, const QString& name)
{
    const SMMethodEntry* method = SMDocumentIndex(data).method(name);
    return ((method != nullptr) && method->isCondition()) ? method : nullptr;
}

QStringList SMGuardSymbols::paramNames(const StateMachineData& data, uint32_t transitionId)
{
    return SMDocumentIndex(data).paramScope(transitionId).names();
}

QStringList SMGuardSymbols::paramTypes(const StateMachineData& data, uint32_t transitionId)
{
    return SMDocumentIndex(data).paramScope(transitionId).types();
}

QString SMGuardSymbols::attributeName(const StateMachineData& data, uint32_t id)
{
    const SMAttributeEntry* attribute = SMDocumentIndex(data).attribute(id);
    return (attribute != nullptr) ? attribute->getName() : QString();
}

QString SMGuardSymbols::constantName(const StateMachineData& data, uint32_t id)
{
    const ConstantEntry* constant = SMDocumentIndex(data).constant(id);
    return (constant != nullptr) ? constant->getName() : QString();
}

QString SMGuardSymbols::paramName(const StateMachineData& data, uint32_t transitionId, uint32_t id)
{
    const MethodParameter* param = SMDocumentIndex(data).paramScope(transitionId).byId(id);
    return (param != nullptr) ? param->getName() : QString();
}

QString SMGuardSymbols::attributeType(const StateMachineData& data, uint32_t id)
{
    const SMAttributeEntry* attribute = SMDocumentIndex(data).attribute(id);
    return (attribute != nullptr) ? attribute->getType() : QString();
}

QString SMGuardSymbols::constantType(const StateMachineData& data, uint32_t id)
{
    const ConstantEntry* constant = SMDocumentIndex(data).constant(id);
    return (constant != nullptr) ? constant->getType() : QString();
}

QString SMGuardSymbols::paramType(const StateMachineData& data, uint32_t transitionId, uint32_t id)
{
    const MethodParameter* param = SMDocumentIndex(data).paramScope(transitionId).byId(id);
    return (param != nullptr) ? param->getType() : QString();
}

const SMMethodEntry* SMGuardSymbols::method(const StateMachineData& data, uint32_t id)
{
    return SMDocumentIndex(data).method(id);
}

QList<int> SMGuardSymbols::bindArguments(const SMGuardNode& call, const SMMethodEntry& method)
{
    const QList<SMGuardNode*>& args = call.getChildren();
    const QList<MethodParameter>& formals = method.getElements();

    QHash<uint32_t, int> childByFormal;
    QList<int>           unnamed;      // argument children that name no parameter
    for (int i = 0; i < args.size(); ++i)
    {
        const uint32_t formalId = args.at(i)->getArgFormalId();
        if (formalId != 0u)
        {
            childByFormal.insert(formalId, i);
        }
        else
        {
            unnamed.append(i);
        }
    }

    QList<int> bound;
    bound.reserve(formals.size());
    int cursor = 0;
    for (const MethodParameter& formal : formals)
    {
        int index = childByFormal.value(formal.getId(), -1);
        if ((index < 0) && (cursor < unnamed.size()))
        {
            index = unnamed.at(cursor++);
        }

        bound.append(index);
    }

    return bound;
}
