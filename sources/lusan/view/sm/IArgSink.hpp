#ifndef LUSAN_VIEW_SM_IARGSINK_HPP
#define LUSAN_VIEW_SM_IARGSINK_HPP
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
 *  \file        lusan/view/sm/IArgSink.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM argument-mapping commit target.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMOperation.hpp"

#include <QString>

/**
 * \class   IArgSink
 * \brief   The commit target of one argument-mapping row -- the seam that lets ONE mapping
 *          widget (\ref SMArgMapTable) serve two different backing stores (design F3).
 *
 *          An action or event argument lives in an `SMArgumentEntry` list keyed by parameter
 *          name (`SMArgSinkOperation` -> `SMSetArgumentCommand`); a condition-call argument
 *          lives in the guard AST addressed by call-node path (`SMArgSinkGuard` ->
 *          `SMGuardCommands::replaceArg`, added in a later phase). The table knows neither:
 *          it projects the rows the sink reports and hands every edit back through this
 *          interface, so it owns no argument state and carries no business logic.
 *
 *          An implementation is expected to make one edit exactly one undoable command, and
 *          to reject a commit that would not change anything (so a re-commit of the current
 *          value never grows the undo stack).
 **/
class IArgSink
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    virtual ~IArgSink() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the formal parameter \p paramName to a value.
     * \param   paramName   The callee's formal parameter (the slot being filled).
     * \param   src         The kind of value supplied.
     * \param   value       The referenced element name, or the literal text for `Value`.
     * \param   expr        The verbatim C++ text, used only for `Expression`.
     **/
    virtual void setArg( const QString& paramName
                       , SMArgumentEntry::eValueSource src
                       , const QString& value
                       , const QString& expr) = 0;

    //!< Unmaps \p paramName -- the slot falls back to its default (or renders as unmapped).
    virtual void clearArg(const QString& paramName) = 0;

    //!< The current mapping of \p paramName, or nullptr when the slot is unmapped.
    virtual const SMArgumentEntry* argFor(const QString& paramName) const = 0;
};

#endif  // LUSAN_VIEW_SM_IARGSINK_HPP
