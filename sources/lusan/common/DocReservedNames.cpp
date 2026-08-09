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
 *  \file        lusan/common/DocReservedNames.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the names a generated class already owns.
 *
 ************************************************************************/

#include "lusan/common/DocReservedNames.hpp"

namespace
{
    using eKind    = DocReservedNames::eKind;
    using eScope   = DocReservedNames::eScope;
    using Row      = DocReservedNames::Row;

    constexpr uint32_t MEMBER_REACH { DocReservedNames::AppliesHostingState | DocReservedNames::AppliesTimer };

    constexpr Row fixed(QLatin1StringView member, QLatin1StringView owner)
    {
        return Row{ eKind::NameFixed, eScope::StateMachine, MEMBER_REACH, member, owner, false };
    }

    constexpr Row locking(QLatin1StringView member, QLatin1StringView owner)
    {
        return Row{ eKind::NameFixed, eScope::StateMachine, MEMBER_REACH, member, owner, true };
    }

    constexpr Row prefix(QLatin1StringView member, QLatin1StringView owner)
    {
        return Row{ eKind::NamePrefix, eScope::StateMachine, DocReservedNames::AppliesHostingState, member, owner, false };
    }

    constexpr Row function(QLatin1StringView name, QLatin1StringView owner, uint32_t applies)
    {
        return Row{ eKind::FunctionPrefix, eScope::StateMachine, applies, name, owner, false };
    }

    /**
     * Every reserved name of every document type.
     *
     * The state machine block is the members the generated machine class declares whatever the
     * document says, then the prefixes each kind of declaration owns, then the openings of the
     * function names the generator builds. A name that lands on a fixed member writes a header
     * that redeclares it; a name that opens with a reserved prefix writes one that can meet a
     * real member of that kind.
     *
     * The two locking rows exist only on a machine that synchronizes, so a document that does
     * not is never refused for a name it may carry.
     */
    constexpr Row TABLE[]
    {
          fixed(QLatin1StringView("mActionHandler"), QLatin1StringView("the action handler the machine calls"))
        , fixed(QLatin1StringView("mInstanceName") , QLatin1StringView("the name every log record of this instance carries"))
        , fixed(QLatin1StringView("mLock")         , QLatin1StringView("the synchronization object of the import tree"))
        , fixed(QLatin1StringView("mMasterThread") , QLatin1StringView("the dispatcher events and timers are processed on"))
        , fixed(QLatin1StringView("mOwnProcessing"), QLatin1StringView("the storage of the dispatch flag"))
        , fixed(QLatin1StringView("mProcessing")   , QLatin1StringView("the dispatch flag of the import tree"))
        , fixed(QLatin1StringView("mState")        , QLatin1StringView("the active state"))
        , fixed(QLatin1StringView("mCurrentStates"), QLatin1StringView("the active state of every level"))
        , fixed(QLatin1StringView("mHistoryRecord"), QLatin1StringView("the history record of every level"))
        , fixed(QLatin1StringView("mEventConsumer"), QLatin1StringView("the event consumer of the machine"))
        , fixed(QLatin1StringView("mTimerConsumer"), QLatin1StringView("the timer consumer of the machine"))
        , fixed(QLatin1StringView("mFinalObserver"), QLatin1StringView("the observer told when the machine completes"))
        , locking(QLatin1StringView("mOwnLock")    , QLatin1StringView("the machine's own synchronization object"))
        , locking(QLatin1StringView("mEpoch")      , QLatin1StringView("the activation an event is stamped with"))

        , prefix(QLatin1StringView("mAttr")        , QLatin1StringView("attributes"))
        , prefix(QLatin1StringView("mCond")        , QLatin1StringView("embedded conditions"))
        , prefix(QLatin1StringView("mTimer")       , QLatin1StringView("declared timers"))
        , prefix(QLatin1StringView("mParam")       , QLatin1StringView("cached service interface parameters"))

        // A declaration does not only become a member: the generator builds function names from
        // the document name too, and those openings are owned in the same way.
        , function(QLatin1StringView("action_")    , QLatin1StringView("generated action handlers")
                  , DocReservedNames::AppliesCondition)
        , function(QLatin1StringView("on_timer_")  , QLatin1StringView("generated timer handlers")
                  , DocReservedNames::AppliesCondition | DocReservedNames::AppliesMethod)
        , function(QLatin1StringView("on_event_")  , QLatin1StringView("generated event handlers")
                  , DocReservedNames::AppliesCondition | DocReservedNames::AppliesMethod)
        , function(QLatin1StringView("send_event_"), QLatin1StringView("generated event senders")
                  , DocReservedNames::AppliesCondition | DocReservedNames::AppliesMethod)
    };
}

const DocReservedNames::Row* DocReservedNames::fixedMember(QStringView member, bool locking)
{
    if (member.isEmpty())
    {
        return nullptr;
    }

    for (const Row& row : TABLE)
    {
        if ((row.kind == eKind::NameFixed) && (locking || (row.locking == false)) && (member == row.member))
        {
            return &row;
        }
    }

    return nullptr;
}

QList<const DocReservedNames::Row*> DocReservedNames::stateMachine(eKind kind, bool locking)
{
    QList<const Row*> result;
    for (const Row& row : TABLE)
    {
        if ((row.scope == eScope::StateMachine) && (row.kind == kind) && (locking || (row.locking == false)))
        {
            result.append(&row);
        }
    }

    return result;
}

QLatin1StringView DocReservedNames::reservedWord(const DocReservedNames::Row& row)
{
    // Every member prefix opens with the 'm' of a member variable; the word a document name
    // would start with is what follows it.
    return (row.kind == eKind::NamePrefix) ? row.member.sliced(1) : row.member;
}
