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
 *  \file        lusan/view/sm/SMArgSinkGuard.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM argument sink over a guard call's arguments.
 *
 ************************************************************************/

#include "lusan/view/sm/SMArgSinkGuard.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include <QUndoStack>

namespace
{
    using eKind   = SMGuardNode::eKind;
    using eSource = SMArgumentEntry::eValueSource;

    //!< The node reached by following the child-index \p path from \p root, or nullptr.
    const SMGuardNode* nodeAt(const SMGuardNode* root, const QList<int>& path)
    {
        const SMGuardNode* node = root;
        for (int index : path)
        {
            if (node == nullptr) { return nullptr; }
            node = node->childAt(index);
        }

        return node;
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMArgSinkGuard::SMArgSinkGuard(StateMachineModel& model)
    : IArgSink  ( )
    , mModel    (model)
    , mTransId  (0u)
    , mPath     ( )
    , mBound    (false)
    , mProjected( )
{
}

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

void SMArgSinkGuard::bind(uint32_t transitionId, const QList<int>& callPath)
{
    mTransId = transitionId;
    mPath    = callPath;
    mBound   = true;
}

void SMArgSinkGuard::clearBinding(void)
{
    mTransId = 0u;
    mPath.clear();
    mBound   = false;
}

bool SMArgSinkGuard::isBound(void) const
{
    return (callNode() != nullptr);
}

uint32_t SMArgSinkGuard::methodId(void) const
{
    const SMGuardNode* call = callNode();
    return (call != nullptr) ? call->getSymbolId() : 0u;
}

//////////////////////////////////////////////////////////////////////////
// Resolution helpers
//////////////////////////////////////////////////////////////////////////

const SMGuardNode* SMArgSinkGuard::callNode(void) const
{
    // An empty path is a VALID address: the guard's root node is itself the call (a single-call
    // guard). `mBound` -- not path emptiness -- distinguishes bound from unbound.
    if ((mTransId == 0u) || (mBound == false))
    {
        return nullptr;
    }

    const SMTransitionEntry* transition = mModel.getData().findTransitionById(mTransId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return nullptr;
    }

    const SMGuardNode* call = nodeAt(transition->getGuard().getTree(), mPath);
    return ((call != nullptr) && (call->getKind() == eKind::Call)) ? call : nullptr;
}

uint32_t SMArgSinkGuard::formalId(const QString& paramName) const
{
    const SMGuardNode* call = callNode();
    if (call == nullptr)
    {
        return 0u;
    }

    const SMMethodEntry* method = SMGuardSymbols::method(mModel.getData(), call->getSymbolId());
    if (method != nullptr)
    {
        const QList<MethodParameter>& formals = method->getElements();
        for (const MethodParameter& formal : formals)
        {
            if (formal.getName() == paramName)
            {
                return formal.getId();
            }
        }
    }

    return 0u;
}

const SMGuardNode* SMArgSinkGuard::argNodeFor(uint32_t formalId) const
{
    const SMGuardNode* call = callNode();
    if ((call != nullptr) && (formalId != 0u))
    {
        for (int i = 0; i < call->getCount(); ++i)
        {
            if (call->childAt(i)->getArgFormalId() == formalId)
            {
                return call->childAt(i);
            }
        }
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Operand build
//////////////////////////////////////////////////////////////////////////

SMGuardNode* SMArgSinkGuard::buildOperand(SMArgumentEntry::eValueSource src, const QString& value, const QString& expr) const
{
    if (src == eSource::Expression)
    {
        return (expr.trimmed().isEmpty()) ? nullptr : SMGuardNode::makeVerbatim(eKind::Raw, expr);
    }

    // Build the equivalent operand text and parse it exactly as the field would, so a clicked
    // mapping resolves to the very node typing that text produces (parity). A parameterless
    // condition source is spelled `name()` so it parses as a zero-arg Call.
    QString text = value.trimmed();
    if (text.isEmpty())
    {
        return nullptr;
    }

    if (src == eSource::Condition)
    {
        text += QStringLiteral("()");
    }

    // Unresolved text is not stored: the slot stays a clean ghost rather than a stray Raw.
    SMGuardParser::Result res = SMGuardParser::parse(mModel.getData(), mTransId, text, false);
    SMGuardNode* node = nullptr;
    if (res.resolved() && (res.tree != nullptr))
    {
        node = res.tree;
        res.tree = nullptr;
    }

    delete res.tree;
    return node;
}

//////////////////////////////////////////////////////////////////////////
// Commits
//////////////////////////////////////////////////////////////////////////

void SMArgSinkGuard::setArg( const QString& paramName
                           , SMArgumentEntry::eValueSource src
                           , const QString& value
                           , const QString& expr)
{
    const uint32_t formal = formalId(paramName);
    if (formal == 0u)
    {
        return;
    }

    SMGuardNode* operand = buildOperand(src, value, expr);
    if (operand == nullptr)
    {
        // An empty/unresolvable value is not a mapping -- unmap the slot to a clean ghost.
        clearArg(paramName);
        return;
    }

    SMSetGuardCommand* command = SMGuardCommands::setArgByFormal( mModel.getData()
                                                               , mModel.getNotifier()
                                                               , mTransId, mPath, formal, operand
                                                               , tr("Map argument '%1'").arg(paramName));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }
}

void SMArgSinkGuard::clearArg(const QString& paramName)
{
    const uint32_t formal = formalId(paramName);
    if (formal == 0u)
    {
        return;
    }

    // A no-op (the slot was already unmapped) yields a null command, so the undo stack never
    // grows on a plain focus-out.
    SMSetGuardCommand* command = SMGuardCommands::clearArgByFormal( mModel.getData()
                                                                 , mModel.getNotifier()
                                                                 , mTransId, mPath, formal
                                                                 , tr("Unmap argument '%1'").arg(paramName));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }
}

//////////////////////////////////////////////////////////////////////////
// Query -- project the bound Call's arg child back into an SMArgumentEntry
//////////////////////////////////////////////////////////////////////////

const SMArgumentEntry* SMArgSinkGuard::argFor(const QString& paramName) const
{
    const uint32_t formal = formalId(paramName);
    const SMGuardNode* arg = (formal != 0u) ? argNodeFor(formal) : nullptr;
    if (arg == nullptr)
    {
        return nullptr;     // Unmapped -> the table renders a ghost.
    }

    const StateMachineData& data = mModel.getData();

    mProjected = SMArgumentEntry();
    mProjected.setName(paramName);

    switch (arg->getKind())
    {
    case eKind::Attr:
        mProjected.setSource(eSource::Attribute);
        mProjected.setValue(SMGuardSymbols::attributeName(data, arg->getSymbolId()));
        break;

    case eKind::Const:
        mProjected.setSource(eSource::Constant);
        mProjected.setValue(SMGuardSymbols::constantName(data, arg->getSymbolId()));
        break;

    case eKind::Param:
        mProjected.setSource(eSource::Param);
        mProjected.setValue(SMGuardSymbols::paramName(data, mTransId, arg->getSymbolId()));
        break;

    case eKind::Call:
        if (arg->getCount() == 0)
        {
            // A parameterless condition-method call used as a boolean value.
            const SMMethodEntry* method = SMGuardSymbols::method(data, arg->getSymbolId());
            mProjected.setSource(eSource::Condition);
            mProjected.setValue((method != nullptr) ? method->getName() : QString());
            break;
        }
        // A call WITH args is not a simple value source -- fall through to the verbatim form.
        [[fallthrough]];

    case eKind::Lit:
        mProjected.setSource(eSource::Value);
        mProjected.setValue(arg->getText());
        break;

    default:
        // Any richer operand (a comparison, a group, a lambda, raw C++) projects as verbatim
        // C++ so it is shown and preserved; an untouched row never commits, so nothing is lost.
        mProjected.setSource(eSource::Expression);
        mProjected.setExpression(SMGuardRender::text(data, mTransId, *arg));
        break;
    }

    return &mProjected;
}
