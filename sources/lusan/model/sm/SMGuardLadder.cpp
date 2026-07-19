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
 *  \file        lusan/model/sm/SMGuardLadder.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard promotion-ladder composite commands.
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardLadder.hpp"

#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMSymbolIndex.hpp"

#include <QHash>

namespace
{
    using eKind = SMGuardNode::eKind;

    //!< The node reached by following the child-index \p path from \p root, or nullptr.
    SMGuardNode* nodeAt(SMGuardNode* root, const QList<int>& path)
    {
        SMGuardNode* node = root;
        for (int index : path)
        {
            if (node == nullptr) { return nullptr; }
            node = node->childAt(index);
        }

        return node;
    }

    //!< Replaces the node at \p path with \p replacement (freeing the old node); an empty
    //!< path replaces the whole tree. False (and \p replacement freed) on an invalid path.
    bool replaceAt(SMGuardNode*& root, const QList<int>& path, SMGuardNode* replacement)
    {
        if (path.isEmpty())
        {
            delete root;
            root = replacement;
            return true;
        }

        QList<int> parentPath = path;
        const int index = parentPath.takeLast();
        SMGuardNode* parent = nodeAt(root, parentPath);
        if ((parent == nullptr) || (index < 0) || (index >= parent->getCount()))
        {
            delete replacement;
            return false;
        }

        delete parent->getChildren().at(index);
        parent->getChildren()[index] = replacement;
        return true;
    }

    //!< The declared property command for a condition's Implement mode.
    QUndoCommand* implementCommand(StateMachineData& data, DocModelNotifier& notifier, uint32_t methodId, SMMethodEntry::eImplement implement, const QString& text, QUndoCommand* parent)
    {
        StateMachineData* doc = &data;
        auto getter = [doc, methodId]() -> SMMethodEntry::eImplement
        {
            SMMethodEntry* m = doc->getMethods().findMethod(methodId);
            return (m != nullptr) ? m->getImplement() : SMMethodEntry::eImplement::Handler;
        };
        auto setter = [doc, methodId](const SMMethodEntry::eImplement& value)
        {
            SMMethodEntry* m = doc->getMethods().findMethod(methodId);
            if (m != nullptr) { m->setImplement(value); }
        };

        return new TDocSetPropertyCommand<SMMethodEntry::eImplement>(notifier, methodId, eDocElementKind::Method, getter, setter, implement, text, parent);
    }

    //!< The declared property command for a condition's verbatim body.
    QUndoCommand* bodyCommand(StateMachineData& data, DocModelNotifier& notifier, uint32_t methodId, const QString& body, const QString& text, QUndoCommand* parent)
    {
        StateMachineData* doc = &data;
        auto getter = [doc, methodId]() -> QString
        {
            SMMethodEntry* m = doc->getMethods().findMethod(methodId);
            return (m != nullptr) ? m->getBody() : QString();
        };
        auto setter = [doc, methodId](const QString& value)
        {
            SMMethodEntry* m = doc->getMethods().findMethod(methodId);
            if (m != nullptr) { m->setBody(value); }
        };

        return new TDocSetPropertyCommand<QString>(notifier, methodId, eDocElementKind::Method, getter, setter, body, text, parent);
    }
}

//////////////////////////////////////////////////////////////////////////
// SMNameIslandCommand
//////////////////////////////////////////////////////////////////////////

SMNameIslandCommand::SMNameIslandCommand(  StateMachineData& data, DocModelNotifier& notifier
                                         , uint32_t transitionId, const QList<int>& islandPath
                                         , const QString& name, SMMethodEntry::eImplement implement
                                         , const QString& text, QUndoCommand* parent /*= nullptr*/)
    : SMCommand     (data, notifier, text, parent)
    , mTransId      (transitionId)
    , mPath         (islandPath)
    , mName         (name)
    , mImplement    (implement)
    , mBody         ( )
    , mParams       ( )
    , mEntry        (nullptr)
    , mEntryOwned   (false)
    , mMethodId     (0u)
    , mOldGuard     ( )
    , mFirst        (true)
    , mValid        (true)
{
}

SMNameIslandCommand::~SMNameIslandCommand()
{
    if (mEntryOwned)
    {
        delete mEntry;
    }
}

void SMNameIslandCommand::redo()
{
    if (mValid == false)
    {
        return;
    }

    StateMachineData& doc = data();
    SMTransitionEntry* transition = doc.findTransitionById(mTransId);
    if (transition == nullptr)
    {
        mValid = false;
        return;
    }

    if (mFirst)
    {
        SMGuardNode* island = (transition->getGuard().isOk()) ? nodeAt(transition->getGuard().getTree(), mPath) : nullptr;
        if ((island == nullptr) || (island->getKind() != eKind::Lambda))
        {
            mValid = false;
            return;
        }

        mBody = island->getText();
        mOldGuard = transition->getGuard();

        // The declaration: the island's body, bool return, and one parameter per in-scope
        // stimulus parameter the body references (the deterministic auto-map).
        mEntry = new SMMethodEntry(0, mName, SMMethodEntry::eMethodType::Condition);
        mEntry->setReturn(QStringLiteral("bool"));
        mEntry->setImplement(mImplement);
        if (mImplement == SMMethodEntry::eImplement::Embedded)
        {
            mEntry->setBody(mBody);
        }

        mEntryOwned = true;
    }

    doc.getMethods().addElement(mEntry);
    mEntryOwned = false;
    mMethodId = mEntry->getId();

    if (mFirst)
    {
        // Parameters are added after the first insertion so their IDs allocate through the
        // document's parent chain, not a detached entry's own counter.
        const QStringList names = SMGuardSymbols::paramNames(doc, mTransId);
        const QStringList types = SMGuardSymbols::paramTypes(doc, mTransId);
        for (int i = 0; i < names.size(); ++i)
        {
            if (SMSymbolIndex::referencesSymbol(mBody, names.at(i)))
            {
                MethodParameter* param = mEntry->addParam(names.at(i));
                if ((param != nullptr) && (i < types.size()))
                {
                    param->setType(types.at(i));
                }

                mParams.append(names.at(i));
            }
        }
    }

    // Replace the island with the mapped call on a clone of the captured tree.
    SMGuardNode* root = (mOldGuard.getTree() != nullptr) ? mOldGuard.getTree()->clone() : nullptr;
    QList<SMGuardNode*> args;
    for (const QString& paramName : mParams)
    {
        args.append(SMGuardNode::makeRef(eKind::Param, SMGuardSymbols::paramId(doc, mTransId, paramName)));
    }

    if ((root == nullptr) || (replaceAt(root, mPath, SMGuardNode::makeCall(mMethodId, args)) == false))
    {
        delete root;
        mValid = false;
        doc.getMethods().removeElement(mMethodId, &mEntry);
        mEntryOwned = true;
        return;
    }

    SMGuard guard;
    guard.setTree(root);
    guard.setRendered(SMGuardRender::text(doc, mTransId, *root));
    transition->getGuard() = guard;

    mFirst = false;
    notifier().notifyElementAdded(mMethodId, eDocElementKind::Method);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

void SMNameIslandCommand::undo()
{
    if ((mValid == false) || (mMethodId == 0u))
    {
        return;
    }

    StateMachineData& doc = data();
    SMTransitionEntry* transition = doc.findTransitionById(mTransId);
    if (transition != nullptr)
    {
        transition->getGuard() = mOldGuard;
    }

    doc.getMethods().removeElement(mMethodId, &mEntry);
    mEntryOwned = true;

    notifier().notifyElementRemoved(mMethodId, eDocElementKind::Method);
    notifier().notifyElementChanged(mTransId, eDocElementKind::Transition);
}

//////////////////////////////////////////////////////////////////////////
// SMGuardLadder factories
//////////////////////////////////////////////////////////////////////////

QStringList SMGuardLadder::referencedParams(const StateMachineData& data, uint32_t transitionId, const QString& body)
{
    QStringList result;
    for (const QString& name : SMGuardSymbols::paramNames(data, transitionId))
    {
        if (SMSymbolIndex::referencesSymbol(body, name))
        {
            result.append(name);
        }
    }

    return result;
}

bool SMGuardLadder::isSingleReturnBody(const QString& body)
{
    const QString trimmed = body.trimmed();
    if (trimmed.startsWith(QStringLiteral("return")) == false)
    {
        return false;
    }

    // Exactly the `return` keyword (not an identifier prefix), one statement, nothing after.
    if ((trimmed.length() > 6) && (trimmed.at(6).isLetterOrNumber() || (trimmed.at(6) == QLatin1Char('_'))))
    {
        return false;
    }

    const int semi = trimmed.indexOf(QLatin1Char(';'));
    return (semi >= 0) && (semi == trimmed.length() - 1);
}

SMNameIslandCommand* SMGuardLadder::nameIsland(  StateMachineData& data, DocModelNotifier& notifier
                                               , uint32_t transitionId, const QList<int>& islandPath
                                               , const QString& name, SMMethodEntry::eImplement implement
                                               , const QString& text)
{
    SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false)
        || (name.isEmpty()) || (data.getMethods().findMethod(name) != nullptr))
    {
        return nullptr;
    }

    const SMGuardNode* island = nodeAt(transition->getGuard().getTree(), islandPath);
    if ((island == nullptr) || (island->getKind() != eKind::Lambda))
    {
        return nullptr;
    }

    return new SMNameIslandCommand(data, notifier, transitionId, islandPath, name, implement, text);
}

QUndoCommand* SMGuardLadder::moveToHandler(StateMachineData& data, DocModelNotifier& notifier, uint32_t methodId, const QString& text)
{
    SMMethodEntry* method = data.getMethods().findMethod(methodId);
    if ((method == nullptr) || (method->isLambdaCondition() == false))
    {
        return nullptr;
    }

    DocCompositeCommand* composite = new DocCompositeCommand(notifier, text);
    implementCommand(data, notifier, methodId, SMMethodEntry::eImplement::Handler, text, composite);
    bodyCommand(data, notifier, methodId, QString(), text, composite);
    return composite;
}

QUndoCommand* SMGuardLadder::adoptBody(StateMachineData& data, DocModelNotifier& notifier, uint32_t methodId, const QString& body, const QString& text)
{
    SMMethodEntry* method = data.getMethods().findMethod(methodId);
    if ((method == nullptr) || (method->isHandlerCondition() == false))
    {
        return nullptr;
    }

    DocCompositeCommand* composite = new DocCompositeCommand(notifier, text);
    implementCommand(data, notifier, methodId, SMMethodEntry::eImplement::Embedded, text, composite);
    bodyCommand(data, notifier, methodId, body, text, composite);
    return composite;
}

QUndoCommand* SMGuardLadder::inlineBody(StateMachineData& data, DocModelNotifier& notifier, uint32_t transitionId, const QList<int>& callPath, const QString& text)
{
    SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return nullptr;
    }

    SMGuardNode* call = nodeAt(transition->getGuard().getTree(), callPath);
    if ((call == nullptr) || (call->getKind() != eKind::Call))
    {
        return nullptr;
    }

    const SMMethodEntry* method = SMGuardSymbols::method(data, call->getSymbolId());
    if ((method == nullptr) || (method->isLambdaCondition() == false) || (isSingleReturnBody(method->getBody()) == false))
    {
        return nullptr;
    }

    // Substitute the declared parameter names with the call's rendered arguments in one
    // pass (token-wise, so an argument that mentions another parameter's name is safe).
    QHash<QString, QString> byName;
    const QList<MethodParameter>& params = method->getElements();
    for (int i = 0; (i < params.size()) && (i < call->getCount()); ++i)
    {
        byName.insert(params.at(i).getName(), SMGuardRender::text(data, transitionId, *call->childAt(i)));
    }

    const QString body = method->getBody();
    QString inlined;
    inlined.reserve(body.size());
    int i = 0;
    const int n = body.length();
    while (i < n)
    {
        const QChar c = body.at(i);
        if (c.isLetter() || (c == QLatin1Char('_')))
        {
            const int start = i;
            while ((i < n) && (body.at(i).isLetterOrNumber() || (body.at(i) == QLatin1Char('_')))) { ++i; }
            const QString word = body.mid(start, i - start);
            inlined += byName.value(word, word);
            continue;
        }

        inlined += c;
        ++i;
    }

    return SMGuardCommands::replaceSubtree(data, notifier, transitionId, callPath
                                          , SMGuardNode::makeVerbatim(eKind::Lambda, QStringLiteral(" ") + inlined.trimmed() + QStringLiteral(" "))
                                          , text);
}
