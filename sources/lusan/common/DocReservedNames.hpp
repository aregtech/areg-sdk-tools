#ifndef LUSAN_COMMON_DOCRESERVEDNAMES_HPP
#define LUSAN_COMMON_DOCRESERVEDNAMES_HPP
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
 *  \file        lusan/common/DocReservedNames.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the names a generated class already owns.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QLatin1StringView>
#include <QList>
#include <QStringView>

#include <cstdint>

/**
 * \namespace   DocReservedNames
 * \brief   The names a generated class already owns, in one table.
 *
 *          A row says what the name is (a member the class always declares, a prefix one kind
 *          of declaration owns, or the opening of a generated function name), which document
 *          type it belongs to, which declarations it can be reached from, and what claims it in
 *          words a message can print. Everything that needs a reserved name reads this table, so
 *          a name found later is a row rather than an edit in several places.
 *
 *          The code generator carries the same table. The two have to stay identical: two tools
 *          with different lists refuse different documents, which is worse than one short list.
 **/
namespace DocReservedNames
{
    /**
     * \enum    eKind
     * \brief   What a row forbids.
     **/
    enum class eKind
    {
          NameFixed         //!< The whole member spelling. A name producing it redeclares what is there.
        , NamePrefix        //!< The opening of a member spelling; only the named kind may produce it.
        , FunctionPrefix    //!< The opening of a generated function name.
    };

    /**
     * \enum    eScope
     * \brief   Which document type a row belongs to.
     **/
    enum class eScope
    {
          StateMachine
        , ServiceInterface
    };

    /**
     * \enum    eApplies
     * \brief   The declarations a row can be reached from. A state that hosts no imported
     *          machine generates no member at all, which is why a plain state is not here.
     **/
    enum eApplies : uint32_t
    {
          AppliesNone         = 0x00u
        , AppliesHostingState = 0x01u   //!< A state that hosts an imported machine.
        , AppliesTimer        = 0x02u   //!< A declared timer.
        , AppliesAttribute    = 0x04u   //!< A machine attribute.
        , AppliesCondition    = 0x08u   //!< A condition method.
        , AppliesMethod       = 0x10u   //!< A trigger or an action.
        , AppliesParameter    = 0x20u   //!< A method or event parameter.
    };

    /**
     * \struct  Row
     * \brief   One reserved name: what it is, whose it is, and where it can be reached from.
     **/
    struct Row
    {
        eKind               kind;       //!< What the row forbids.
        eScope              scope;      //!< The document type it belongs to.
        uint32_t            applies;    //!< The declarations it can be reached from (\ref eApplies).
        QLatin1StringView   member;     //!< The member spelling, the leading 'm' included, or the function prefix.
        QLatin1StringView   owner;      //!< What claims the name, in words a message prints.
        bool                locking;    //!< True when the row exists only on a machine that synchronizes.
    };

    /**
     * \brief   The row \p member lands on, or nullptr when nothing claims it.
     * \param   member  The member spelling a document name produces, the leading 'm' included.
     * \param   locking True when the machine synchronizes, which is what makes the two extra
     *                  fixed members exist.
     **/
    const Row* fixedMember(QStringView member, bool locking);

    /**
     * \brief   Every row of \p kind that applies to a state machine.
     * \param   kind    Which half of the table is wanted.
     * \param   locking True when the machine synchronizes.
     **/
    QList<const Row*> stateMachine(eKind kind, bool locking);

    /**
     * \brief   The reserved word of a prefix row: the member spelling without its leading 'm',
     *          which is what a document name would start with.
     **/
    QLatin1StringView reservedWord(const Row& row);
}

#endif  // LUSAN_COMMON_DOCRESERVEDNAMES_HPP
