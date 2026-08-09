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
 *  \file        lusan/common/DocElementTable.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the elements a document may contain and where each may sit.
 *
 ************************************************************************/

#include "lusan/common/DocElementTable.hpp"

#include "lusan/common/VersionNumber.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

namespace
{
    using eStatus = DocElementTable::eStatus;
    using Row     = DocElementTable::Row;

    constexpr QLatin1StringView NONE { "" };

    //!< The five operations, and so the content of every operation list.
    constexpr QLatin1StringView OPERATION_PARENTS { "EntryList|ExitList|OperationList" };

    //!< A guard expression node may sit directly under the expression, under a boolean or
    //!< comparison node, or as a call argument.
    constexpr QLatin1StringView GUARD_NODE_PARENTS { "Expr|And|Or|Not|Cmp|Arg" };

    /**
     * Every element of the state machine format.
     *
     * The parent list of each row is what the readers accept, checked one reader at a time:
     * an element written anywhere else was not put there by this editor.
     */
    constexpr Row TABLE[]
    {
          { QLatin1StringView("StateMachine")   , NONE                                          , eStatus::Valid, NONE }

        // Document sections.
        , { QLatin1StringView("Overview")       , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("DataTypeList")   , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("AttributeList")  , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("EventList")      , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("TimerList")      , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("MethodList")     , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("ConstantList")   , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("IncludeList")    , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("ImportList")     , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }
        , { QLatin1StringView("StateList")      , QLatin1StringView("StateMachine|State")       , eStatus::Valid, NONE }
        , { QLatin1StringView("Layout")         , QLatin1StringView("StateMachine")             , eStatus::Valid, NONE }

        // Carried by almost every declaration.
        , { QLatin1StringView("Description")    , QLatin1StringView("Overview|DataType|Field|EnumEntry|Attribute|Event|Parameter"
                                                                   "|Timer|Method|Constant|Location|MachineImport|State|Transition")
                                                                                               , eStatus::Valid, NONE }
        , { QLatin1StringView("DeprecateHint")  , QLatin1StringView("DataType|Field|EnumEntry|Attribute|Event|Parameter"
                                                                   "|Timer|Method|Constant|Location")
                                                                                               , eStatus::Valid, NONE }

        // Data types.
        , { QLatin1StringView("DataType")       , QLatin1StringView("DataTypeList")             , eStatus::Valid, NONE }
        , { QLatin1StringView("FieldList")      , QLatin1StringView("DataType")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("Field")          , QLatin1StringView("FieldList")                , eStatus::Valid, NONE }
        , { QLatin1StringView("EnumEntry")      , QLatin1StringView("FieldList")                , eStatus::Valid, NONE }
        , { QLatin1StringView("Value")          , QLatin1StringView("Field|EnumEntry|Constant|Parameter")
                                                                                               , eStatus::Valid, NONE }
        , { QLatin1StringView("Container")      , QLatin1StringView("DataType")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("BaseTypeKey")    , QLatin1StringView("DataType")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("BaseTypeValue")  , QLatin1StringView("DataType")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("Namespace")      , QLatin1StringView("DataType")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("ImportedObject") , QLatin1StringView("DataType")                 , eStatus::Valid, NONE }

        // Registries.
        , { QLatin1StringView("Attribute")      , QLatin1StringView("AttributeList")            , eStatus::Valid, NONE }
        , { QLatin1StringView("Event")          , QLatin1StringView("EventList")                , eStatus::Valid, NONE }
        , { QLatin1StringView("Timer")          , QLatin1StringView("TimerList")                , eStatus::Valid, NONE }
        , { QLatin1StringView("Method")         , QLatin1StringView("MethodList")               , eStatus::Valid, NONE }
        , { QLatin1StringView("Constant")       , QLatin1StringView("ConstantList")             , eStatus::Valid, NONE }
        , { QLatin1StringView("Location")       , QLatin1StringView("IncludeList|DataType")     , eStatus::Valid, NONE }
        , { QLatin1StringView("MachineImport")  , QLatin1StringView("ImportList")               , eStatus::Valid, NONE }
        , { QLatin1StringView("ParamList")      , QLatin1StringView("Event|Method")             , eStatus::Valid, NONE }
        , { QLatin1StringView("Parameter")      , QLatin1StringView("ParamList")                , eStatus::Valid, NONE }
        , { QLatin1StringView("Body")           , QLatin1StringView("Method|Condition")         , eStatus::Valid, NONE }

        // States and transitions.
        , { QLatin1StringView("State")          , QLatin1StringView("StateList")                , eStatus::Valid, NONE }
        , { QLatin1StringView("EntryList")      , QLatin1StringView("State")                    , eStatus::Valid, NONE }
        , { QLatin1StringView("ExitList")       , QLatin1StringView("State")                    , eStatus::Valid, NONE }
        , { QLatin1StringView("TransitionList") , QLatin1StringView("State")                    , eStatus::Valid, NONE }
        , { QLatin1StringView("DoList")         , QLatin1StringView("State")                    , eStatus::Removed
                                                , QLatin1StringView("a Timer started on entry and stopped on exit") }
        , { QLatin1StringView("Transition")     , QLatin1StringView("TransitionList")           , eStatus::Valid, NONE }
        , { QLatin1StringView("OperationList")  , QLatin1StringView("Transition")               , eStatus::Valid, NONE }
        , { QLatin1StringView("ConditionList")  , QLatin1StringView("Transition")               , eStatus::Valid, NONE }
        , { QLatin1StringView("ConditionGroup") , QLatin1StringView("ConditionList|ConditionGroup")
                                                                                               , eStatus::Valid, NONE }
        , { QLatin1StringView("Condition")      , QLatin1StringView("ConditionList|ConditionGroup")
                                                                                               , eStatus::Valid, NONE }

        // Operations: the whole set, and nothing else may sit in an operation list.
        , { QLatin1StringView("ActionCall")     , OPERATION_PARENTS                             , eStatus::Valid, NONE }
        , { QLatin1StringView("AttributeSet")   , OPERATION_PARENTS                             , eStatus::Valid, NONE }
        , { QLatin1StringView("TimerStart")     , OPERATION_PARENTS                             , eStatus::Valid, NONE }
        , { QLatin1StringView("TimerStop")      , OPERATION_PARENTS                             , eStatus::Valid, NONE }
        , { QLatin1StringView("EventSend")      , OPERATION_PARENTS                             , eStatus::Valid, NONE }
        , { QLatin1StringView("InlineCode")     , OPERATION_PARENTS                             , eStatus::Removed
                                                , QLatin1StringView("an Action that carries the code") }

        , { QLatin1StringView("ArgumentList")   , QLatin1StringView("ActionCall|EventSend|Condition")
                                                                                               , eStatus::Valid, NONE }
        , { QLatin1StringView("Argument")       , QLatin1StringView("ArgumentList")             , eStatus::Valid, NONE }
        , { QLatin1StringView("Expression")     , QLatin1StringView("AttributeSet|Argument|Condition")
                                                                                               , eStatus::Valid, NONE }

        // Guards.
        , { QLatin1StringView("Guard")          , QLatin1StringView("Transition")               , eStatus::Valid, NONE }
        , { QLatin1StringView("Expr")           , QLatin1StringView("Guard")                    , eStatus::Valid, NONE }
        , { QLatin1StringView("Draft")          , QLatin1StringView("Guard")                    , eStatus::Valid, NONE }
        , { QLatin1StringView("Rendered")       , QLatin1StringView("Guard")                    , eStatus::Valid, NONE }
        , { QLatin1StringView("And")            , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Or")             , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Not")            , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Cmp")            , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Call")           , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Attr")           , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Const")          , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Param")          , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Lit")            , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Lambda")         , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Raw")            , GUARD_NODE_PARENTS                            , eStatus::Valid, NONE }
        , { QLatin1StringView("Arg")            , QLatin1StringView("Call")                     , eStatus::Valid, NONE }

        // Layout.
        , { QLatin1StringView("ViewList")       , QLatin1StringView("Layout")                   , eStatus::Valid, NONE }
        , { QLatin1StringView("View")           , QLatin1StringView("ViewList")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("NodeList")       , QLatin1StringView("Layout")                   , eStatus::Valid, NONE }
        , { QLatin1StringView("Node")           , QLatin1StringView("NodeList")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("EdgeList")       , QLatin1StringView("Layout")                   , eStatus::Valid, NONE }
        , { QLatin1StringView("Edge")           , QLatin1StringView("EdgeList")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("Point")          , QLatin1StringView("Edge")                     , eStatus::Valid, NONE }
        , { QLatin1StringView("Label")          , QLatin1StringView("Edge")                     , eStatus::Valid, NONE }
        , { QLatin1StringView("NoteList")       , QLatin1StringView("Layout")                   , eStatus::Valid, NONE }
        , { QLatin1StringView("Note")           , QLatin1StringView("NoteList")                 , eStatus::Valid, NONE }
        , { QLatin1StringView("Text")           , QLatin1StringView("Note")                     , eStatus::Valid, NONE }
    };

    //!< True when \p list, a '|' separated set of names, contains \p name whole.
    bool listContains(QLatin1StringView list, QStringView name)
    {
        qsizetype from = 0;
        while (from <= list.size())
        {
            qsizetype end = from;
            while ((end < list.size()) && (list.at(end) != QLatin1Char('|')))
            {
                ++end;
            }

            if (name == list.sliced(from, end - from))
            {
                return true;
            }

            from = end + 1;
        }

        return false;
    }
}

const VersionNumber& DocElementTable::maxFormatVersion()
{
    static const VersionNumber highest(StateMachineData::XML_FORMAT_DEFAULT);
    return highest;
}

const DocElementTable::Row* DocElementTable::find(QStringView name)
{
    for (const Row& row : TABLE)
    {
        if (name == row.name)
        {
            return &row;
        }
    }

    return nullptr;
}

bool DocElementTable::accepts(QStringView name, QStringView parent)
{
    const Row* row = find(name);
    if ((row == nullptr) || (row->status == eStatus::Removed))
    {
        return false;
    }

    return parent.isEmpty() ? row->parents.isEmpty() : listContains(row->parents, parent);
}
