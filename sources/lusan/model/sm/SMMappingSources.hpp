#ifndef LUSAN_MODEL_SM_SMMAPPINGSOURCES_HPP
#define LUSAN_MODEL_SM_SMMAPPINGSOURCES_HPP
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
 *  \file        lusan/model/sm/SMMappingSources.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM parameter-mapping value-source enumeration.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \struct  SMSourceEntry
 * \brief   One selectable value source for a mapping cell: the referenced element name, its
 *          declared type, and how that type fits the target parameter type.
 **/
struct SMSourceEntry
{
    QString                 name;   //!< The referenced element name.
    QString                 type;   //!< The referenced element's declared type.
    SMTypeCompat::eRank     rank;   //!< Fit of \ref type against the target parameter type.
};

/**
 * \namespace   SMMappingSources
 * \brief   Headless enumeration of the value sources a parameter-mapping cell may offer,
 *          filtered to type-compatible entries per the widening table (spec 6.9). Shared by
 *          the mapping editor and reusable by the operations editors; no widget dependency.
 **/
namespace SMMappingSources
{
    /**
     * \brief   The type-compatible entries of one reference source \p kind for a cell whose
     *          declared target type is \p targetType, in the scope of \p transitionId.
     *          \p kind must be Param, Attribute, Constant, or Condition; other kinds have no
     *          reference universe and yield an empty list. Entries ranked Mismatch are dropped.
     **/
    QList<SMSourceEntry> candidates( const StateMachineData& data
                                   , uint32_t transitionId
                                   , SMArgumentEntry::eValueSource kind
                                   , const QString& targetType);

    /**
     * \brief   True when \p kind is a legal source in this scope. Only Param is scope-limited:
     *          it requires a transition scope (\p transitionId != 0) whose stimulus declares
     *          parameters (spec 6.8 scope rule); every other kind is always legal.
     **/
    bool isKindLegal(const StateMachineData& data, uint32_t transitionId, SMArgumentEntry::eValueSource kind);

    /**
     * \brief   The declared type of the element \p name referenced as \p kind, or an empty
     *          string when it cannot be resolved. Used to annotate an implicit widening on a row.
     **/
    QString referencedType( const StateMachineData& data
                          , uint32_t transitionId
                          , SMArgumentEntry::eValueSource kind
                          , const QString& name);
}

#endif  // LUSAN_MODEL_SM_SMMAPPINGSOURCES_HPP
