#ifndef LUSAN_VIEW_SM_SMARGSINKGUARD_HPP
#define LUSAN_VIEW_SM_SMARGSINKGUARD_HPP
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
 *  \file        lusan/view/sm/SMArgSinkGuard.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM argument sink over a guard call's arguments.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/IArgSink.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"

#include <QCoreApplication>
#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;

/**
 * \class   SMArgSinkGuard
 * \brief   The condition side of \ref IArgSink: commits one mapping row into a guard's Call
 *          node through an undoable `SMGuardCommands::setArgByFormal` / `clearArgByFormal`.
 *          The Call is addressed by node PATH (nth call, so `a() && a()` map independently);
 *          the formal is addressed by its document id (Option A), never its position -- so a
 *          later signature edit never re-binds a filled slot.
 *
 *          The sink owns NO argument state: it PROJECTS the Call's arg children back into a
 *          transient `SMArgumentEntry` for \ref argFor (the same shape the shared
 *          \ref SMArgMapTable reads for the Actions side) and turns each committed row into
 *          the equivalent guard operand by parsing its value text -- so a clicked mapping
 *          resolves to the very node typing that text into the field produces (parity).
 *
 *          A commit whose operand does not resolve to a node (an empty value) unmaps the
 *          slot instead of storing a meaningless node, so an unmapped formal is always a
 *          clean ghost (spec 4.3), and re-committing an unchanged value never grows the
 *          undo stack (the underlying command drops a no-op).
 **/
class SMArgSinkGuard : public IArgSink
{
    Q_DECLARE_TR_FUNCTIONS(SMArgSinkGuard)

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMArgSinkGuard(StateMachineModel& model);
    virtual ~SMArgSinkGuard(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the sink to the Call node reached by \p callPath (child indices from the
     *          guard-tree root) in transition \p transitionId. An EMPTY path is valid -- it
     *          addresses the guard's root node (a single-call guard). A path that does not
     *          resolve to a Call makes the sink inert (\ref isBound is false).
     **/
    void bind(uint32_t transitionId, const QList<int>& callPath);

    //!< Drops the binding; the sink becomes inert.
    void clearBinding(void);

    //!< True when the bound path currently resolves to a Call node.
    bool isBound(void) const;

    //!< The bound transition (0 when unbound).
    inline uint32_t transitionId(void) const;

    //!< The bound Call node's path (child indices from the root).
    inline const QList<int>& callPath(void) const;

    //!< The bound Call's method (symbol) id, or 0 when the sink is inert.
    uint32_t methodId(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual void setArg( const QString& paramName
                       , SMArgumentEntry::eValueSource src
                       , const QString& value
                       , const QString& expr) override;

    virtual void clearArg(const QString& paramName) override;

    virtual const SMArgumentEntry* argFor(const QString& paramName) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    //!< The currently bound Call node, or nullptr when the path no longer resolves to one.
    const SMGuardNode* callNode(void) const;

    //!< The document id of the formal named \p paramName in the bound Call's method, or 0.
    uint32_t formalId(const QString& paramName) const;

    //!< The Call arg child bound to \p formalId, or nullptr when the slot is unmapped.
    const SMGuardNode* argNodeFor(uint32_t formalId) const;

    /**
     * \brief   Builds the guard operand a mapping row describes: verbatim C++ (`Expression`)
     *          becomes a Raw node; every other source parses its value text exactly as the
     *          field would, so the produced node is identical to the typed one. Returns
     *          nullptr for an empty/unresolvable value (the caller then unmaps the slot).
     **/
    SMGuardNode* buildOperand(SMArgumentEntry::eValueSource src, const QString& value, const QString& expr) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;
    uint32_t                mTransId;
    QList<int>              mPath;
    bool                    mBound;     //!< True between bind and clearBinding (an empty path is a valid root call).
    mutable SMArgumentEntry mProjected; //!< Transient storage backing \ref argFor's returned pointer.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMArgSinkGuard::transitionId(void) const
{
    return mTransId;
}

inline const QList<int>& SMArgSinkGuard::callPath(void) const
{
    return mPath;
}

#endif  // LUSAN_VIEW_SM_SMARGSINKGUARD_HPP
