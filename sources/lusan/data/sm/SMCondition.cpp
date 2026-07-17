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
 *  \file        lusan/data/sm/SMCondition.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition conditions.
 *
 ************************************************************************/

#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <utility>

namespace
{
    //!< Writes \p text as a CDATA-wrapped child element (byte-exact, never normalized).
    void writeCDataElem(QXmlStreamWriter& xml, const char* elemName, const QString& text)
    {
        xml.writeStartElement(elemName);
        xml.writeCDATA(text);
        xml.writeEndElement();
    }
}

//////////////////////////////////////////////////////////////////////////
// SMConditionNode implementation
//////////////////////////////////////////////////////////////////////////

SMConditionNode::SMConditionNode(ElementBase* parent /*= nullptr*/)
    : DocumentElem(parent)
{
}

SMConditionNode::SMConditionNode(uint32_t id, ElementBase* parent)
    : DocumentElem(id, parent)
{
}

SMConditionNode::SMConditionNode(const SMConditionNode& src)
    : DocumentElem(src)
{
}

SMConditionNode::SMConditionNode(SMConditionNode&& src) noexcept
    : DocumentElem(std::move(src))
{
}

SMConditionNode& SMConditionNode::operator = (const SMConditionNode& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
    }

    return *this;
}

SMConditionNode& SMConditionNode::operator = (SMConditionNode&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////
// SMConditionEntry static helpers
//////////////////////////////////////////////////////////////////////////

SMConditionEntry::eOperator SMConditionEntry::fromOperatorString(const QString& op)
{
    if (op.compare(STR_OP_EQUAL, Qt::CaseInsensitive) == 0)
        return eOperator::Equal;
    else if (op.compare(STR_OP_NOTEQUAL, Qt::CaseInsensitive) == 0)
        return eOperator::NotEqual;
    else if (op.compare(STR_OP_LESS, Qt::CaseInsensitive) == 0)
        return eOperator::Less;
    else if (op.compare(STR_OP_LESSEQUAL, Qt::CaseInsensitive) == 0)
        return eOperator::LessEqual;
    else if (op.compare(STR_OP_GREATER, Qt::CaseInsensitive) == 0)
        return eOperator::Greater;
    else if (op.compare(STR_OP_GREATEREQUAL, Qt::CaseInsensitive) == 0)
        return eOperator::GreaterEqual;
    else
        return eOperator::None;
}

const char* SMConditionEntry::toString(SMConditionEntry::eOperator op)
{
    switch (op)
    {
    case eOperator::Equal:          return STR_OP_EQUAL;
    case eOperator::NotEqual:       return STR_OP_NOTEQUAL;
    case eOperator::Less:           return STR_OP_LESS;
    case eOperator::LessEqual:      return STR_OP_LESSEQUAL;
    case eOperator::Greater:        return STR_OP_GREATER;
    case eOperator::GreaterEqual:   return STR_OP_GREATEREQUAL;
    case eOperator::None:
    default:                        return "";
    }
}

//////////////////////////////////////////////////////////////////////////
// SMConditionEntry implementation
//////////////////////////////////////////////////////////////////////////

SMConditionEntry::SMConditionEntry(ElementBase* parent /*= nullptr*/)
    : SMConditionNode (parent)
    , mLhsKind      (eOperandKind::Attribute)
    , mLhs          ( )
    , mOperator     (eOperator::None)
    , mRhsKind      (eOperandKind::Value)
    , mRhs          ( )
    , mNegate       (false)
    , mExpression   ( )
    , mBody         ( )
    , mArguments    ( )
{
}

SMConditionEntry::SMConditionEntry(uint32_t id, ElementBase* parent)
    : SMConditionNode (id, parent)
    , mLhsKind      (eOperandKind::Attribute)
    , mLhs          ( )
    , mOperator     (eOperator::None)
    , mRhsKind      (eOperandKind::Value)
    , mRhs          ( )
    , mNegate       (false)
    , mExpression   ( )
    , mBody         ( )
    , mArguments    ( )
{
}

SMConditionEntry::SMConditionEntry(const SMConditionEntry& src)
    : SMConditionNode (src)
    , mLhsKind      (src.mLhsKind)
    , mLhs          (src.mLhs)
    , mOperator     (src.mOperator)
    , mRhsKind      (src.mRhsKind)
    , mRhs          (src.mRhs)
    , mNegate       (src.mNegate)
    , mExpression   (src.mExpression)
    , mBody         (src.mBody)
    , mArguments    (src.mArguments)
{
    for (SMArgumentEntry& arg : mArguments)
    {
        arg.setParent(this);
    }
}

SMConditionEntry::SMConditionEntry(SMConditionEntry&& src) noexcept
    : SMConditionNode (std::move(src))
    , mLhsKind      (src.mLhsKind)
    , mLhs          (std::move(src.mLhs))
    , mOperator     (src.mOperator)
    , mRhsKind      (src.mRhsKind)
    , mRhs          (std::move(src.mRhs))
    , mNegate       (src.mNegate)
    , mExpression   (std::move(src.mExpression))
    , mBody         (std::move(src.mBody))
    , mArguments    (std::move(src.mArguments))
{
    for (SMArgumentEntry& arg : mArguments)
    {
        arg.setParent(this);
    }
}

SMConditionEntry& SMConditionEntry::operator = (const SMConditionEntry& other)
{
    if (this != &other)
    {
        SMConditionNode::operator = (other);
        mLhsKind    = other.mLhsKind;
        mLhs        = other.mLhs;
        mOperator   = other.mOperator;
        mRhsKind    = other.mRhsKind;
        mRhs        = other.mRhs;
        mNegate     = other.mNegate;
        mExpression = other.mExpression;
        mBody       = other.mBody;
        mArguments  = other.mArguments;
        for (SMArgumentEntry& arg : mArguments)
        {
            arg.setParent(this);
        }
    }

    return *this;
}

SMConditionEntry& SMConditionEntry::operator = (SMConditionEntry&& other) noexcept
{
    if (this != &other)
    {
        SMConditionNode::operator = (std::move(other));
        mLhsKind    = other.mLhsKind;
        mLhs        = std::move(other.mLhs);
        mOperator   = other.mOperator;
        mRhsKind    = other.mRhsKind;
        mRhs        = std::move(other.mRhs);
        mNegate     = other.mNegate;
        mExpression = std::move(other.mExpression);
        mBody       = std::move(other.mBody);
        mArguments  = std::move(other.mArguments);
        for (SMArgumentEntry& arg : mArguments)
        {
            arg.setParent(this);
        }
    }

    return *this;
}

SMArgumentEntry* SMConditionEntry::addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value)
{
    mArguments.append(SMArgumentEntry(getNextId(), name, source, value, this));
    return &mArguments.last();
}

void SMConditionEntry::assignContent(const SMConditionEntry& src)
{
    mLhsKind    = src.mLhsKind;
    mLhs        = src.mLhs;
    mOperator   = src.mOperator;
    mRhsKind    = src.mRhsKind;
    mRhs        = src.mRhs;
    mNegate     = src.mNegate;
    mExpression = src.mExpression;
    mBody       = src.mBody;
    mArguments  = src.mArguments;
    for (SMArgumentEntry& arg : mArguments)
    {
        arg.setParent(this);
    }
}

SMConditionNode::eNodeKind SMConditionEntry::getNodeKind() const
{
    return eNodeKind::Leaf;
}

SMConditionNode* SMConditionEntry::clone() const
{
    return new SMConditionEntry(*this);
}

bool SMConditionEntry::isValid() const
{
    if (isExpressionRow())
    {
        return (mExpression.isEmpty() == false);
    }
    else if (isLambdaRow())
    {
        return (mBody.isEmpty() == false);
    }

    return (mLhs.isEmpty() == false);
}

bool SMConditionEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementCondition)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mLhsKind = SMArgumentEntry::fromSourceString(attributes.value(XmlSM::xmlSMAttributeLhsKind).toString());
    mLhs     = attributes.value(XmlSM::xmlSMAttributeLhs).toString();
    mOperator = attributes.hasAttribute(XmlSM::xmlSMAttributeOperator)
                    ? fromOperatorString(attributes.value(XmlSM::xmlSMAttributeOperator).toString())
                    : eOperator::None;
    mRhsKind = SMArgumentEntry::fromSourceString(attributes.value(XmlSM::xmlSMAttributeRhsKind).toString());
    mRhs     = attributes.value(XmlSM::xmlSMAttributeRhs).toString();
    mNegate  = (attributes.value(XmlSM::xmlSMAttributeNegate).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0);
    mExpression.clear();
    mBody.clear();
    mArguments.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementCondition))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementExpression)
            {
                mExpression = xml.readElementText();
            }
            else if (xml.name() == XmlSM::xmlSMElementBody)
            {
                mBody = xml.readElementText();
            }
            else if (xml.name() == XmlSM::xmlSMElementArgumentList)
            {
                SMArgumentEntry::readArgumentList(xml, mArguments, this);
            }
        }

        xml.readNext();
    }

    return true;
}

void SMConditionEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementCondition);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeLhsKind, SMArgumentEntry::toString(mLhsKind));

    if (isVerbatimRow() == false)
    {
        if (mLhs.isEmpty() == false)
        {
            xml.writeAttribute(XmlSM::xmlSMAttributeLhs, mLhs);
        }
        if (hasOperator())
        {
            xml.writeAttribute(XmlSM::xmlSMAttributeOperator, SMConditionEntry::toString(mOperator));
            xml.writeAttribute(XmlSM::xmlSMAttributeRhsKind, SMArgumentEntry::toString(mRhsKind));
            xml.writeAttribute(XmlSM::xmlSMAttributeRhs, mRhs);
        }
    }

    if (mNegate)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeNegate, XmlSM::xmlSMValueTrue);
    }

    if (isExpressionRow())
    {
        writeCDataElem(xml, XmlSM::xmlSMElementExpression, mExpression);
    }
    else if (isLambdaRow())
    {
        writeCDataElem(xml, XmlSM::xmlSMElementBody, mBody);
    }

    SMArgumentEntry::writeArgumentList(xml, mArguments);

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMConditionGroup static helpers
//////////////////////////////////////////////////////////////////////////

SMConditionGroup::eCombine SMConditionGroup::fromCombineString(const QString& combine)
{
    return (combine.compare(STR_COMBINE_OR, Qt::CaseInsensitive) == 0) ? eCombine::Or : eCombine::And;
}

const char* SMConditionGroup::toString(SMConditionGroup::eCombine combine)
{
    return (combine == eCombine::Or) ? STR_COMBINE_OR : STR_COMBINE_AND;
}

//////////////////////////////////////////////////////////////////////////
// SMConditionGroup implementation
//////////////////////////////////////////////////////////////////////////

SMConditionGroup::SMConditionGroup(ElementBase* parent /*= nullptr*/)
    : SMConditionNode (parent)
    , mCombine      (eCombine::And)
    , mNegate       (false)
    , mChildren     ( )
{
}

SMConditionGroup::SMConditionGroup(uint32_t id, ElementBase* parent)
    : SMConditionNode (id, parent)
    , mCombine      (eCombine::And)
    , mNegate       (false)
    , mChildren     ( )
{
}

SMConditionGroup::SMConditionGroup(const SMConditionGroup& src)
    : SMConditionNode (src)
    , mCombine      (src.mCombine)
    , mNegate       (src.mNegate)
    , mChildren     ( )
{
    cloneChildrenFrom(src);
}

SMConditionGroup::SMConditionGroup(SMConditionGroup&& src) noexcept
    : SMConditionNode (std::move(src))
    , mCombine      (src.mCombine)
    , mNegate       (src.mNegate)
    , mChildren     (std::move(src.mChildren))
{
    reparentChildren();
}

SMConditionGroup::~SMConditionGroup()
{
    removeAll();
}

SMConditionGroup& SMConditionGroup::operator = (const SMConditionGroup& other)
{
    if (this != &other)
    {
        removeAll();
        SMConditionNode::operator = (other);
        mCombine = other.mCombine;
        mNegate  = other.mNegate;
        cloneChildrenFrom(other);
    }

    return *this;
}

SMConditionGroup& SMConditionGroup::operator = (SMConditionGroup&& other) noexcept
{
    if (this != &other)
    {
        removeAll();
        SMConditionNode::operator = (std::move(other));
        mCombine  = other.mCombine;
        mNegate   = other.mNegate;
        mChildren = std::move(other.mChildren);
        reparentChildren();
    }

    return *this;
}

SMConditionEntry* SMConditionGroup::addCondition()
{
    SMConditionEntry* entry = new SMConditionEntry(getNextId(), this);
    mChildren.append(entry);
    return entry;
}

SMConditionGroup* SMConditionGroup::addGroup()
{
    SMConditionGroup* group = new SMConditionGroup(getNextId(), this);
    mChildren.append(group);
    return group;
}

SMConditionNode* SMConditionGroup::addChild(SMConditionNode* node)
{
    if (node != nullptr)
    {
        node->setParent(this);
        mChildren.append(node);
    }

    return node;
}

bool SMConditionGroup::removeChild(SMConditionNode* node)
{
    const int index = mChildren.indexOf(node);
    if (index < 0)
    {
        return false;
    }

    delete mChildren.takeAt(index);
    return true;
}

void SMConditionGroup::removeAll()
{
    for (SMConditionNode* child : mChildren)
    {
        delete child;
    }

    mChildren.clear();
}

QList<SMConditionEntry*> SMConditionGroup::collectLeaves() const
{
    QList<SMConditionEntry*> out;
    for (SMConditionNode* child : mChildren)
    {
        if (child->getNodeKind() == eNodeKind::Leaf)
        {
            out.append(static_cast<SMConditionEntry*>(child));
        }
        else
        {
            out.append(static_cast<SMConditionGroup*>(child)->collectLeaves());
        }
    }

    return out;
}

QList<SMConditionGroup*> SMConditionGroup::collectGroups() const
{
    QList<SMConditionGroup*> out;
    for (SMConditionNode* child : mChildren)
    {
        if (child->getNodeKind() == eNodeKind::Group)
        {
            SMConditionGroup* group = static_cast<SMConditionGroup*>(child);
            out.append(group);
            out.append(group->collectGroups());
        }
    }

    return out;
}

SMConditionNode* SMConditionGroup::findNode(uint32_t id)
{
    if (getId() == id)
    {
        return this;
    }

    for (SMConditionNode* child : mChildren)
    {
        if (child->getId() == id)
        {
            return child;
        }

        if (child->getNodeKind() == eNodeKind::Group)
        {
            SMConditionNode* found = static_cast<SMConditionGroup*>(child)->findNode(id);
            if (found != nullptr)
            {
                return found;
            }
        }
    }

    return nullptr;
}

SMConditionGroup* SMConditionGroup::findGroup(uint32_t id)
{
    SMConditionNode* node = findNode(id);
    return ((node != nullptr) && node->isGroup()) ? static_cast<SMConditionGroup*>(node) : nullptr;
}

SMConditionEntry* SMConditionGroup::findLeaf(uint32_t id)
{
    SMConditionNode* node = findNode(id);
    return ((node != nullptr) && node->isLeaf()) ? static_cast<SMConditionEntry*>(node) : nullptr;
}

int SMConditionGroup::indexOfChild(const SMConditionNode* node) const
{
    return mChildren.indexOf(const_cast<SMConditionNode*>(node));
}

SMConditionNode* SMConditionGroup::detachChild(SMConditionNode* node)
{
    const int index = mChildren.indexOf(node);
    return (index >= 0) ? mChildren.takeAt(index) : nullptr;
}

void SMConditionGroup::insertChild(int index, SMConditionNode* node)
{
    if (node == nullptr)
    {
        return;
    }

    node->setParent(this);
    mChildren.insert(qBound(0, index, static_cast<int>(mChildren.size())), node);
}

void SMConditionGroup::swapChildren(int index1, int index2)
{
    if ((index1 >= 0) && (index2 >= 0) && (index1 < mChildren.size()) && (index2 < mChildren.size()) && (index1 != index2))
    {
        mChildren.swapItemsAt(index1, index2);
    }
}

SMConditionNode::eNodeKind SMConditionGroup::getNodeKind() const
{
    return eNodeKind::Group;
}

SMConditionNode* SMConditionGroup::clone() const
{
    return new SMConditionGroup(*this);
}

bool SMConditionGroup::isValid() const
{
    return true;
}

void SMConditionGroup::cloneChildrenFrom(const SMConditionGroup& src)
{
    for (const SMConditionNode* child : src.mChildren)
    {
        SMConditionNode* copy = child->clone();
        copy->setParent(this);
        mChildren.append(copy);
    }
}

void SMConditionGroup::reparentChildren()
{
    for (SMConditionNode* child : mChildren)
    {
        child->setParent(this);
    }
}

void SMConditionGroup::writeGroupBody(QXmlStreamWriter& xml) const
{
    if (mCombine == eCombine::Or)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeCombine, SMConditionGroup::toString(mCombine));
    }

    if (mNegate)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeNegate, XmlSM::xmlSMValueTrue);
    }

    for (const SMConditionNode* child : mChildren)
    {
        child->writeToXml(xml);
    }
}

void SMConditionGroup::readGroupBody(QXmlStreamReader& xml, const char* endElem)
{
    const QXmlStreamAttributes attributes = xml.attributes();
    mCombine = attributes.hasAttribute(XmlSM::xmlSMAttributeCombine)
                    ? fromCombineString(attributes.value(XmlSM::xmlSMAttributeCombine).toString())
                    : eCombine::And;
    mNegate  = (attributes.value(XmlSM::xmlSMAttributeNegate).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0);

    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == endElem)
        {
            break;
        }

        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementCondition)
            {
                SMConditionEntry* leaf = new SMConditionEntry(this);
                if (leaf->readFromXml(xml))
                {
                    mChildren.append(leaf);
                }
                else
                {
                    delete leaf;
                }
            }
            else if (xml.name() == XmlSM::xmlSMElementConditionGroup)
            {
                SMConditionGroup* group = new SMConditionGroup(this);
                if (group->readFromXml(xml))
                {
                    mChildren.append(group);
                }
                else
                {
                    delete group;
                }
            }
        }
    }
}

bool SMConditionGroup::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementConditionGroup)
        return false;

    setId(xml.attributes().value(XmlSM::xmlSMAttributeID).toUInt());
    removeAll();
    readGroupBody(xml, XmlSM::xmlSMElementConditionGroup);
    return true;
}

void SMConditionGroup::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementConditionGroup);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    writeGroupBody(xml);
    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMConditionList implementation
//////////////////////////////////////////////////////////////////////////

SMConditionList::SMConditionList(ElementBase* parent /*= nullptr*/)
    : SMConditionGroup(parent)
{
}

SMConditionList::SMConditionList(const SMConditionList& src)
    : SMConditionGroup(src)
{
}

SMConditionList::SMConditionList(SMConditionList&& src) noexcept
    : SMConditionGroup(std::move(src))
{
}

SMConditionList& SMConditionList::operator = (const SMConditionList& other)
{
    if (this != &other)
    {
        SMConditionGroup::operator = (other);
    }

    return *this;
}

SMConditionList& SMConditionList::operator = (SMConditionList&& other) noexcept
{
    if (this != &other)
    {
        SMConditionGroup::operator = (std::move(other));
    }

    return *this;
}

bool SMConditionList::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementConditionList)
        return false;

    removeAll();
    readGroupBody(xml, XmlSM::xmlSMElementConditionList);
    return true;
}

void SMConditionList::writeToXml(QXmlStreamWriter& xml) const
{
    if (getChildren().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementConditionList);
    writeGroupBody(xml);
    xml.writeEndElement();
}
