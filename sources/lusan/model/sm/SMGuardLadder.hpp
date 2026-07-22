#ifndef LUSAN_MODEL_SM_SMGUARDLADDER_HPP
#define LUSAN_MODEL_SM_SMGUARDLADDER_HPP
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
 *  \file        lusan/model/sm/SMGuardLadder.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard promotion-ladder composite commands
 *               (island <-> named lambda <-> handler).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"

#include <QList>
#include <QString>
#include <QStringList>
#include <cstdint>

/**
 * The promotion ladder: each rung is ONE undo step. `Name it...`
 * turns an anonymous island into a declared condition method (kind `lambda` by default,
 * `handler` for the direct island-to-handler path) plus a call with the referenced stimulus
 * parameters mapped; `Move to handler...` / `Adopt body...` flip a declared condition's kind
 * (call sites are untouched -- previews re-derive `m<Name>(...)` vs `handler().Name(...)`
 * from the declaration); `Inline body here` folds a single-return named lambda back into an
 * island at one call site (the declaration stays; delete it from the Methods page).
 **/

/**
 * \class   SMNameIslandCommand
 * \brief   Replaces the Lambda island at \p islandPath of a transition's guard with a call
 *          to a newly declared condition method carrying the island's body. The method ID is
 *          allocated by the first redo and reused verbatim afterwards (the shared command
 *          discipline), which is why this is a bespoke command and not stock children: the
 *          call node cannot be built before the declaration exists.
 **/
class SMNameIslandCommand : public SMCommand
{
public:
    SMNameIslandCommand(  StateMachineData& data, DocModelNotifier& notifier
                        , uint32_t transitionId, const QList<int>& islandPath
                        , const QString& name, SMMethodEntry::eImplement implement
                        , const QString& text, QUndoCommand* parent = nullptr);
    virtual ~SMNameIslandCommand();

    void redo() override;
    void undo() override;

    //!< The declared method's ID (valid after the first redo), or 0 when the redo failed.
    inline uint32_t methodId() const;

private:
    uint32_t                    mTransId;   //!< The owning transition ID.
    QList<int>                  mPath;      //!< The island node's child-index path.
    QString                     mName;      //!< The declared method name.
    SMMethodEntry::eImplement   mImplement; //!< The condition kind (Embedded = lambda).
    QString                     mBody;      //!< The island body (read on first redo).
    QStringList                 mParams;    //!< The mapped stimulus parameter names.
    SMMethodEntry*              mEntry;     //!< The declaration (owned while out of the model).
    bool                        mEntryOwned;//!< True when this command owns \ref mEntry.
    uint32_t                    mMethodId;  //!< The allocated method ID.
    SMGuard                     mOldGuard;  //!< The captured previous guard.
    bool                        mFirst;     //!< True until the first redo completed.
    bool                        mValid;     //!< False when the target island was not found.
};

/**
 * \namespace   SMGuardLadder
 * \brief   Factories for the ladder steps plus the deterministic helpers the UI shares
 *          (referenced-parameter derivation, single-return check, call-site inlining).
 **/
namespace SMGuardLadder
{
    //!< The in-scope stimulus parameter names referenced (whole-word) in \p body, declared order.
    QStringList referencedParams(const StateMachineData& data, uint32_t transitionId, const QString& body);

    //!< True when \p body is a single `return <expr>;` statement (the inline-body gate).
    bool isSingleReturnBody(const QString& body);

    /**
     * \brief   `Name it...` / island-to-handler: declare a condition method (kind per
     *          \p implement) named \p name with the island's body and the referenced stimulus
     *          parameters, and replace the island with the mapped call. One undo step.
     * \return  The ready-to-push command; nullptr when the transition has no ok tree.
     **/
    SMNameIslandCommand* nameIsland(  StateMachineData& data, DocModelNotifier& notifier
                                    , uint32_t transitionId, const QList<int>& islandPath
                                    , const QString& name, SMMethodEntry::eImplement implement
                                    , const QString& text);

    /**
     * \brief   `Move to handler...`: flips a lambda condition to kind `handler` and drops the
     *          body Lusan no longer owns (undo restores both). The caller offers the body as
     *          a copyable stub before pushing. Call sites are untouched.
     **/
    QUndoCommand* moveToHandler(StateMachineData& data, DocModelNotifier& notifier, uint32_t methodId, const QString& text);

    /**
     * \brief   `Adopt body...`: flips a handler condition to kind `lambda` with \p body.
     **/
    QUndoCommand* adoptBody(StateMachineData& data, DocModelNotifier& notifier, uint32_t methodId, const QString& body, const QString& text);

    /**
     * \brief   `Inline body here`: replaces the named-lambda call at \p callPath with an
     *          island carrying the lambda's body, its parameter names substituted by the
     *          call's rendered arguments. Only for a single-return body.
     * \return  The command, or nullptr (no tree / not a lambda call / not single-return).
     **/
    QUndoCommand* inlineBody(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, const QString& text);
}

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMNameIslandCommand::methodId() const
{
    return mMethodId;
}

#endif  // LUSAN_MODEL_SM_SMGUARDLADDER_HPP
