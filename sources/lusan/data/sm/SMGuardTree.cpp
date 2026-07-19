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
 *  \file        lusan/data/sm/SMGuardTree.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition guard tree (AST) and state holder.
 *
 ************************************************************************/

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMGuardNode static helpers
//////////////////////////////////////////////////////////////////////////

QLatin1StringView SMGuardNode::toString(eKind kind)
{
    switch (kind)
    {
    case eKind::And:    return XmlSM::xmlSMElementGuardAnd;
    case eKind::Or:     return XmlSM::xmlSMElementGuardOr;
    case eKind::Not:    return XmlSM::xmlSMElementGuardNot;
    case eKind::Cmp:    return XmlSM::xmlSMElementGuardCmp;
    case eKind::Call:   return XmlSM::xmlSMElementGuardCall;
    case eKind::Attr:   return XmlSM::xmlSMElementGuardAttr;
    case eKind::Const:  return XmlSM::xmlSMElementGuardConst;
    case eKind::Param:  return XmlSM::xmlSMElementGuardParam;
    case eKind::Lit:    return XmlSM::xmlSMElementGuardLit;
    case eKind::Lambda: return XmlSM::xmlSMElementGuardLambda;
    case eKind::Raw:    return XmlSM::xmlSMElementGuardRaw;
    default:            return XmlSM::xmlSMElementGuardRaw;
    }
}

const char* SMGuardNode::toString(eCmpOp op)
{
    switch (op)
    {
    case eCmpOp::Eq:    return "eq";
    case eCmpOp::Ne:    return "ne";
    case eCmpOp::Lt:    return "lt";
    case eCmpOp::Le:    return "le";
    case eCmpOp::Gt:    return "gt";
    case eCmpOp::Ge:    return "ge";
    default:            return "eq";
    }
}

bool SMGuardNode::fromKindString(const QString& text, eKind& kind)
{
    if      (text == XmlSM::xmlSMElementGuardAnd)    { kind = eKind::And;    }
    else if (text == XmlSM::xmlSMElementGuardOr)     { kind = eKind::Or;     }
    else if (text == XmlSM::xmlSMElementGuardNot)    { kind = eKind::Not;    }
    else if (text == XmlSM::xmlSMElementGuardCmp)    { kind = eKind::Cmp;    }
    else if (text == XmlSM::xmlSMElementGuardCall)   { kind = eKind::Call;   }
    else if (text == XmlSM::xmlSMElementGuardAttr)   { kind = eKind::Attr;   }
    else if (text == XmlSM::xmlSMElementGuardConst)  { kind = eKind::Const;  }
    else if (text == XmlSM::xmlSMElementGuardParam)  { kind = eKind::Param;  }
    else if (text == XmlSM::xmlSMElementGuardLit)    { kind = eKind::Lit;    }
    else if (text == XmlSM::xmlSMElementGuardLambda) { kind = eKind::Lambda; }
    else if (text == XmlSM::xmlSMElementGuardRaw)    { kind = eKind::Raw;    }
    else                                             { return false;         }

    return true;
}

bool SMGuardNode::fromOpString(const QString& text, eCmpOp& op)
{
    if      (text == "eq") { op = eCmpOp::Eq; }
    else if (text == "ne") { op = eCmpOp::Ne; }
    else if (text == "lt") { op = eCmpOp::Lt; }
    else if (text == "le") { op = eCmpOp::Le; }
    else if (text == "gt") { op = eCmpOp::Gt; }
    else if (text == "ge") { op = eCmpOp::Ge; }
    else                   { return false;    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// SMGuardNode factories
//////////////////////////////////////////////////////////////////////////

SMGuardNode* SMGuardNode::makeGroup(eKind kind, const QList<SMGuardNode*>& children)
{
    SMGuardNode* node = new SMGuardNode(kind);
    node->mChildren = children;
    return node;
}

SMGuardNode* SMGuardNode::makeNot(SMGuardNode* child)
{
    SMGuardNode* node = new SMGuardNode(eKind::Not);
    if (child != nullptr)
    {
        node->mChildren.append(child);
    }

    return node;
}

SMGuardNode* SMGuardNode::makeCmp(eCmpOp op, SMGuardNode* lhs, SMGuardNode* rhs)
{
    SMGuardNode* node = new SMGuardNode(eKind::Cmp);
    node->mOp = op;
    if (lhs != nullptr) { node->mChildren.append(lhs); }
    if (rhs != nullptr) { node->mChildren.append(rhs); }
    return node;
}

SMGuardNode* SMGuardNode::makeCall(uint32_t symbolId, const QList<SMGuardNode*>& args)
{
    SMGuardNode* node = new SMGuardNode(eKind::Call);
    node->mSymbolId = symbolId;
    node->mChildren = args;
    return node;
}

SMGuardNode* SMGuardNode::makeRef(eKind kind, uint32_t symbolId)
{
    SMGuardNode* node = new SMGuardNode(kind);
    node->mSymbolId = symbolId;
    return node;
}

SMGuardNode* SMGuardNode::makeVerbatim(eKind kind, const QString& text)
{
    SMGuardNode* node = new SMGuardNode(kind);
    node->mText = text;
    return node;
}

//////////////////////////////////////////////////////////////////////////
// SMGuardNode construction
//////////////////////////////////////////////////////////////////////////

SMGuardNode::SMGuardNode(eKind kind)
    : mKind     (kind)
    , mOp       (eCmpOp::Eq)
    , mSymbolId (0u)
    , mText     ( )
    , mChildren ( )
{
}

SMGuardNode::SMGuardNode(const SMGuardNode& src)
    : mKind     (src.mKind)
    , mOp       (src.mOp)
    , mSymbolId (src.mSymbolId)
    , mText     (src.mText)
    , mChildren ( )
{
    mChildren.reserve(src.mChildren.size());
    for (const SMGuardNode* child : src.mChildren)
    {
        mChildren.append(child->clone());
    }
}

SMGuardNode::~SMGuardNode()
{
    qDeleteAll(mChildren);
    mChildren.clear();
}

SMGuardNode& SMGuardNode::operator = (const SMGuardNode& other)
{
    if (this != &other)
    {
        qDeleteAll(mChildren);
        mChildren.clear();

        mKind     = other.mKind;
        mOp       = other.mOp;
        mSymbolId = other.mSymbolId;
        mText     = other.mText;
        mChildren.reserve(other.mChildren.size());
        for (const SMGuardNode* child : other.mChildren)
        {
            mChildren.append(child->clone());
        }
    }

    return *this;
}

SMGuardNode* SMGuardNode::addChild(SMGuardNode* child)
{
    mChildren.append(child);
    return child;
}

bool SMGuardNode::equals(const SMGuardNode& other) const
{
    if ((mKind != other.mKind) || (mChildren.size() != other.mChildren.size()))
        return false;

    if ((mKind == eKind::Cmp) && (mOp != other.mOp))
        return false;

    if (isReference() || (mKind == eKind::Call))
    {
        if (mSymbolId != other.mSymbolId)
            return false;
    }

    if (isVerbatim() && (mText != other.mText))
        return false;

    for (int i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren.at(i)->equals(*other.mChildren.at(i)) == false)
            return false;
    }

    return true;
}

SMGuardNode* SMGuardNode::clone() const
{
    return new SMGuardNode(*this);
}

//////////////////////////////////////////////////////////////////////////
// SMGuardNode XML persistence
//////////////////////////////////////////////////////////////////////////

void SMGuardNode::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(toString(mKind));

    switch (mKind)
    {
    case eKind::Cmp:
        xml.writeAttribute(XmlSM::xmlSMAttributeGuardOp, toString(mOp));
        break;

    case eKind::Call:
    case eKind::Attr:
    case eKind::Const:
    case eKind::Param:
        xml.writeAttribute(XmlSM::xmlSMAttributeGuardRefId, QString::number(mSymbolId));
        break;

    default:
        break;
    }

    if (isVerbatim())
    {
        xml.writeCharacters(mText);
    }
    else if (mKind == eKind::Call)
    {
        // Each argument wraps in <Arg>, in declared-parameter order.
        for (const SMGuardNode* child : mChildren)
        {
            xml.writeStartElement(XmlSM::xmlSMElementGuardArg);
            child->writeToXml(xml);
            xml.writeEndElement();
        }
    }
    else
    {
        for (const SMGuardNode* child : mChildren)
        {
            child->writeToXml(xml);
        }
    }

    xml.writeEndElement();
}

SMGuardNode* SMGuardNode::readFromXml(QXmlStreamReader& xml)
{
    eKind kind;
    if (fromKindString(xml.name().toString(), kind) == false)
    {
        xml.skipCurrentElement();
        return nullptr;
    }

    SMGuardNode* node = new SMGuardNode(kind);
    const QXmlStreamAttributes attributes = xml.attributes();

    if (kind == eKind::Cmp)
    {
        fromOpString(attributes.value(XmlSM::xmlSMAttributeGuardOp).toString(), node->mOp);
    }
    else if ((kind == eKind::Call) || node->isReference())
    {
        node->mSymbolId = attributes.value(XmlSM::xmlSMAttributeGuardRefId).toUInt();
    }

    if (node->isVerbatim())
    {
        node->mText = xml.readElementText(QXmlStreamReader::IncludeChildElements);
        return node;
    }

    while (xml.readNextStartElement())
    {
        if (kind == eKind::Call)
        {
            if (xml.name() == XmlSM::xmlSMElementGuardArg)
            {
                if (xml.readNextStartElement())
                {
                    SMGuardNode* arg = readFromXml(xml);
                    if (arg != nullptr)
                    {
                        node->mChildren.append(arg);
                    }
                    // Consume the rest of the <Arg> element.
                    while (xml.readNextStartElement())
                    {
                        xml.skipCurrentElement();
                    }
                }
            }
            else
            {
                xml.skipCurrentElement();
            }
        }
        else
        {
            SMGuardNode* child = readFromXml(xml);
            if (child != nullptr)
            {
                node->mChildren.append(child);
            }
        }
    }

    return node;
}

//////////////////////////////////////////////////////////////////////////
// SMGuard static helpers
//////////////////////////////////////////////////////////////////////////

const char* SMGuard::toString(eState state)
{
    switch (state)
    {
    case eState::Ok:    return "ok";
    case eState::Draft: return "draft";
    default:            return "ok";
    }
}

SMGuard::eState SMGuard::fromStateString(const QString& text)
{
    return (text == "draft") ? eState::Draft : eState::Ok;
}

//////////////////////////////////////////////////////////////////////////
// SMGuard construction
//////////////////////////////////////////////////////////////////////////

SMGuard::SMGuard()
    : mState    (eState::Empty)
    , mTree     (nullptr)
    , mDraftText( )
    , mRendered ( )
{
}

SMGuard::SMGuard(const SMGuard& src)
    : mState    (src.mState)
    , mTree     (src.mTree != nullptr ? src.mTree->clone() : nullptr)
    , mDraftText(src.mDraftText)
    , mRendered (src.mRendered)
{
}

SMGuard::SMGuard(SMGuard&& src) noexcept
    : mState    (src.mState)
    , mTree     (src.mTree)
    , mDraftText(std::move(src.mDraftText))
    , mRendered (std::move(src.mRendered))
{
    src.mTree  = nullptr;
    src.mState = eState::Empty;
}

SMGuard::~SMGuard()
{
    delete mTree;
}

SMGuard& SMGuard::operator = (const SMGuard& other)
{
    if (this != &other)
    {
        delete mTree;
        mState     = other.mState;
        mTree      = (other.mTree != nullptr) ? other.mTree->clone() : nullptr;
        mDraftText = other.mDraftText;
        mRendered  = other.mRendered;
    }

    return *this;
}

SMGuard& SMGuard::operator = (SMGuard&& other) noexcept
{
    if (this != &other)
    {
        delete mTree;
        mState     = other.mState;
        mTree      = other.mTree;
        mDraftText = std::move(other.mDraftText);
        mRendered  = std::move(other.mRendered);
        other.mTree  = nullptr;
        other.mState = eState::Empty;
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////
// SMGuard operations
//////////////////////////////////////////////////////////////////////////

void SMGuard::setTree(SMGuardNode* tree)
{
    if (tree != mTree)
    {
        delete mTree;
        mTree = tree;
    }

    mState = eState::Ok;
    mDraftText.clear();
}

void SMGuard::setDraft(const QString& text, SMGuardNode* lastGood /*= nullptr*/)
{
    if (lastGood != mTree)
    {
        delete mTree;
        mTree = lastGood;
    }

    mState     = eState::Draft;
    mDraftText = text;
}

void SMGuard::clear()
{
    delete mTree;
    mTree = nullptr;
    mState = eState::Empty;
    mDraftText.clear();
    mRendered.clear();
}

//////////////////////////////////////////////////////////////////////////
// SMGuard XML persistence
//////////////////////////////////////////////////////////////////////////

void SMGuard::writeToXml(QXmlStreamWriter& xml) const
{
    if (mState == eState::Empty)
        return;

    xml.writeStartElement(XmlSM::xmlSMElementGuard);
    xml.writeAttribute(XmlSM::xmlSMAttributeGuardState, toString(mState));

    if (mState == eState::Draft)
    {
        xml.writeStartElement(XmlSM::xmlSMElementGuardDraft);
        xml.writeCharacters(mDraftText);
        xml.writeEndElement();
    }

    if (mTree != nullptr)
    {
        xml.writeStartElement(XmlSM::xmlSMElementGuardExpr);
        mTree->writeToXml(xml);
        xml.writeEndElement();
    }

    if ((mState == eState::Ok) && (mRendered.isEmpty() == false))
    {
        xml.writeStartElement(XmlSM::xmlSMElementGuardRendered);
        xml.writeCharacters(mRendered);
        xml.writeEndElement();
    }

    xml.writeEndElement();
}

bool SMGuard::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementGuard)
        return false;

    clear();
    const eState state = fromStateString(xml.attributes().value(XmlSM::xmlSMAttributeGuardState).toString());

    SMGuardNode* tree = nullptr;
    QString draft;
    QString rendered;
    bool hasDraft = false;

    while (xml.readNextStartElement())
    {
        if (xml.name() == XmlSM::xmlSMElementGuardDraft)
        {
            draft = xml.readElementText();
            hasDraft = true;
        }
        else if (xml.name() == XmlSM::xmlSMElementGuardExpr)
        {
            if (xml.readNextStartElement())
            {
                delete tree;
                tree = SMGuardNode::readFromXml(xml);
                while (xml.readNextStartElement())
                {
                    xml.skipCurrentElement();
                }
            }
        }
        else if (xml.name() == XmlSM::xmlSMElementGuardRendered)
        {
            rendered = xml.readElementText();
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    if ((state == eState::Draft) || hasDraft)
    {
        setDraft(draft, tree);
    }
    else
    {
        setTree(tree);
    }

    mRendered = rendered;
    return true;
}
