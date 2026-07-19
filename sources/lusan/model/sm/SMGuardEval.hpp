#ifndef LUSAN_MODEL_SM_SMGUARDEVAL_HPP
#define LUSAN_MODEL_SM_SMGUARDEVAL_HPP
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
 *  \file        lusan/model/sm/SMGuardEval.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard what-if evaluator (v7 B9, Try-it strip).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMGuardTree.hpp"

#include <QHash>
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \class   SMGuardEval
 * \brief   Evaluates a resolved guard tree against stub values for the Try-it strip (B9).
 *          References (Param/Attr/Const) read literal values -- supplied by the strip, or
 *          falling back to the declared value of an attribute/constant. Call, Lambda and
 *          Raw nodes NEVER execute in the tool: each is a manual true/false stub site, and
 *          a missing stub makes the result Unknown (three-valued logic), never a guess.
 *          Deterministic and view-free, so the semantics are headless-testable. This is a
 *          what-if display only -- no user code runs.
 **/
class SMGuardEval
{
public:
    /**
     * \enum    eTruth
     * \brief   A three-valued boolean: Unknown when a stub or value is missing.
     **/
    enum class eTruth
    {
          False
        , True
        , Unknown
    };

    /**
     * \struct  StubSite
     * \brief   One node that requires a manual stub (Call / Lambda / Raw at boolean
     *          position), addressed by its child-index path from the root.
     **/
    struct StubSite
    {
        QList<int>          path;       //!< The node's child-index path from the root.
        SMGuardNode::eKind  kind;       //!< Call, Lambda, or Raw.
        uint32_t            symbolId;   //!< The called method's document ID (Call only).
        QString             text;       //!< The verbatim body/fragment (Lambda / Raw).
    };

    /**
     * \struct  Inputs
     * \brief   The evaluation inputs: literal values per referenced symbol ID and a
     *          stubbed result per stub-site path key.
     **/
    struct Inputs
    {
        QHash<uint32_t, QString>    values; //!< Param/Attr/Const ID -> literal text.
        QHash<QString, bool>        stubs;  //!< pathKey -> the stubbed boolean result.
    };

    /**
     * \struct  NodeTruth
     * \brief   The truth of one boolean-position node (the lens tints its pill by it).
     **/
    struct NodeTruth
    {
        QList<int>  path;
        eTruth      truth;
    };

    /**
     * \struct  Result
     * \brief   The guard truth plus the per-node truths of every boolean-position node.
     **/
    struct Result
    {
        eTruth              truth { eTruth::Unknown };
        QList<NodeTruth>    nodes;
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The stable string key of a node path (stub maps, truth lookups).
    static QString pathKey(const QList<int>& path);

    //!< Every stub site of the tree, in pre-order.
    static QList<StubSite> stubSites(const SMGuardNode& tree);

    //!< The distinct referenced symbol IDs of one reference kind, in first-use order.
    static QList<uint32_t> referencedIds(const SMGuardNode& tree, SMGuardNode::eKind kind);

    /**
     * \brief   The literal a reference reads during evaluation: the strip's value when
     *          supplied, else the declared value of an attribute/constant, else empty
     *          (empty evaluates to Unknown).
     **/
    static QString effectiveValue(const StateMachineData& data, SMGuardNode::eKind kind
                                 , uint32_t symbolId, const Inputs& inputs);

    //!< Evaluates the tree; per-node truths cover every boolean-position node.
    static Result evaluate(const StateMachineData& data, const SMGuardNode& tree, const Inputs& inputs);

    /**
     * \brief   The guard-level truth: an empty guard is always True (the transition fires
     *          unconditionally), a draft is Unknown, an ok tree evaluates.
     **/
    static eTruth guardTruth(const StateMachineData& data, const SMGuard& guard, const Inputs& inputs);

    /**
     * \brief   The display text of a call/island stub row: the called name with each
     *          argument replaced by its effective value (`HasWaiting(3)`), the island body
     *          in braces, or the raw fragment verbatim.
     **/
    static QString stubDisplay(const StateMachineData& data, const SMGuardNode& node, const Inputs& inputs);

    /**
     * \brief   The transitions of the same state reacting to the same stimulus as
     *          \p transitionId (including it), in document (priority) order.
     **/
    static QList<uint32_t> siblingTransitions(const StateMachineData& data, uint32_t transitionId);
};

#endif  // LUSAN_MODEL_SM_SMGUARDEVAL_HPP
