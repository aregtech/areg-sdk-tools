#ifndef LUSAN_MODEL_SM_SMGUARDRENDER_HPP
#define LUSAN_MODEL_SM_SMGUARDRENDER_HPP
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
 *  \file        lusan/model/sm/SMGuardRender.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard renderer: AST -> canonical text + span roles.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;
class SMGuardNode;
class SMGuard;

/**
 * \class   SMGuardRender
 * \brief   Renders a resolved guard tree back to its canonical, human-readable surface text
 *          (bare declared names, minimal precedence parentheses) plus a span list that the
 *          highlighter colors by owner. The tree is the truth: `render(parse(text))` is the
 *          canonical form of \p text, and `parse(render(tree))` reproduces \p tree. Because
 *          names are looked up by ID at render time, a rename re-renders every guard with the
 *          new name and no guard edit (v6 Section 3.3).
 **/
class SMGuardRender
{
public:
    /**
     * \enum    eRole
     * \brief   The owner/kind role of a text span, mapped to a visual token by the
     *          highlighter (B15). Roles, never literal colors.
     **/
    enum class eRole
    {
          Stim      //!< A stimulus parameter reference.
        , Fsm       //!< An attribute, constant, or named-lambda reference.
        , Handler   //!< A handler-condition call.
        , Literal   //!< A verbatim literal.
        , Raw       //!< A raw-C++ fragment.
        , Lambda    //!< An anonymous-lambda body token.
        , Operator  //!< A logical/comparison operator.
        , Punct     //!< Parentheses, commas.
    };

    /**
     * \struct  Span
     * \brief   One colored span of the rendered text.
     **/
    struct Span
    {
        int     start;      //!< The 0-based start offset in the rendered text.
        int     length;     //!< The span length.
        eRole   role;       //!< The owner/kind role.
    };

    /**
     * \struct  Rendered
     * \brief   The rendered text and its span list.
     **/
    struct Rendered
    {
        QString         text;
        QList<Span>     spans;
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The canonical text of a node sub-tree.
    static QString text(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node);

    //!< The canonical text plus span list of a node sub-tree.
    static Rendered render(const StateMachineData& data, uint32_t transitionId, const SMGuardNode& node);

    /**
     * \brief   The display text of a whole guard: the rendered tree (ok), the raw draft text
     *          (draft), or an empty string (empty).
     **/
    static QString guardText(const StateMachineData& data, uint32_t transitionId, const SMGuard& guard);
};

#endif  // LUSAN_MODEL_SM_SMGUARDRENDER_HPP
