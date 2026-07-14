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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
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
    : DocumentElem  (parent)
    , mLhsKind      (eOperandKind::Attribute)
    , mLhs          ( )
    , mOperator     (eOperator::None)
    , mRhsKind      (eOperandKind::Value)
    , mRhs          ( )
    , mNegate       (false)
    , mExpression   ( )
    , mArguments    ( )
{
}

SMConditionEntry::SMConditionEntry(uint32_t id, ElementBase* parent)
    : DocumentElem  (id, parent)
    , mLhsKind      (eOperandKind::Attribute)
    , mLhs          ( )
    , mOperator     (eOperator::None)
    , mRhsKind      (eOperandKind::Value)
    , mRhs          ( )
    , mNegate       (false)
    , mExpression   ( )
    , mArguments    ( )
{
}

SMConditionEntry::SMConditionEntry(const SMConditionEntry& src)
    : DocumentElem  (src)
    , mLhsKind      (src.mLhsKind)
    , mLhs          (src.mLhs)
    , mOperator     (src.mOperator)
    , mRhsKind      (src.mRhsKind)
    , mRhs          (src.mRhs)
    , mNegate       (src.mNegate)
    , mExpression   (src.mExpression)
    , mArguments    (src.mArguments)
{
    for (SMArgumentEntry& arg : mArguments)
    {
        arg.setParent(this);
    }
}

SMConditionEntry::SMConditionEntry(SMConditionEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mLhsKind      (src.mLhsKind)
    , mLhs          (std::move(src.mLhs))
    , mOperator     (src.mOperator)
    , mRhsKind      (src.mRhsKind)
    , mRhs          (std::move(src.mRhs))
    , mNegate       (src.mNegate)
    , mExpression   (std::move(src.mExpression))
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
        DocumentElem::operator = (other);
        mLhsKind    = other.mLhsKind;
        mLhs        = other.mLhs;
        mOperator   = other.mOperator;
        mRhsKind    = other.mRhsKind;
        mRhs        = other.mRhs;
        mNegate     = other.mNegate;
        mExpression = other.mExpression;
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
        DocumentElem::operator = (std::move(other));
        mLhsKind    = other.mLhsKind;
        mLhs        = std::move(other.mLhs);
        mOperator   = other.mOperator;
        mRhsKind    = other.mRhsKind;
        mRhs        = std::move(other.mRhs);
        mNegate     = other.mNegate;
        mExpression = std::move(other.mExpression);
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

bool SMConditionEntry::isValid() const
{
    if (isExpressionRow())
    {
        return (mExpression.isEmpty() == false);
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
    mArguments.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementCondition))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementExpression)
            {
                mExpression = xml.readElementText();
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

    if (isExpressionRow() == false)
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

    SMArgumentEntry::writeArgumentList(xml, mArguments);

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMConditionList static helpers
//////////////////////////////////////////////////////////////////////////

SMConditionList::eCombine SMConditionList::fromCombineString(const QString& combine)
{
    return (combine.compare(STR_COMBINE_OR, Qt::CaseInsensitive) == 0) ? eCombine::Or : eCombine::And;
}

const char* SMConditionList::toString(SMConditionList::eCombine combine)
{
    return (combine == eCombine::Or) ? STR_COMBINE_OR : STR_COMBINE_AND;
}

//////////////////////////////////////////////////////////////////////////
// SMConditionList implementation
//////////////////////////////////////////////////////////////////////////

SMConditionList::SMConditionList(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMConditionEntry*, DocumentElem>(parent)
    , mCombine(eCombine::And)
{
}

SMConditionList::SMConditionList(const SMConditionList& src)
    : TEDataContainer<SMConditionEntry*, DocumentElem>(src.getParent())
    , mCombine(src.mCombine)
{
    cloneFrom(src);
}

SMConditionList::SMConditionList(SMConditionList&& src) noexcept
    : TEDataContainer<SMConditionEntry*, DocumentElem>(std::move(src))
    , mCombine(src.mCombine)
{
}

SMConditionList::~SMConditionList()
{
    removeAll();
}

SMConditionList& SMConditionList::operator = (const SMConditionList& other)
{
    if (this != &other)
    {
        removeAll();
        setParent(other.getParent());
        mCombine = other.mCombine;
        cloneFrom(other);
    }

    return *this;
}

SMConditionList& SMConditionList::operator = (SMConditionList&& other) noexcept
{
    if (this != &other)
    {
        removeAll();
        TEDataContainer<SMConditionEntry*, DocumentElem>::operator = (std::move(other));
        mCombine = other.mCombine;
    }

    return *this;
}

void SMConditionList::cloneFrom(const SMConditionList& src)
{
    for (const SMConditionEntry* row : src.getElements())
    {
        SMConditionEntry* copy = new SMConditionEntry(*row);
        copy->setParent(this);
        addElement(copy, false);
    }
}

SMConditionEntry* SMConditionList::addCondition()
{
    SMConditionEntry* entry = new SMConditionEntry(getNextId(), this);
    addElement(entry, false);
    return entry;
}

void SMConditionList::removeAll()
{
    for (SMConditionEntry* row : getElements())
    {
        delete row;
    }

    removeAllElements();
}

bool SMConditionList::isValid() const
{
    return true;
}

bool SMConditionList::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementConditionList)
        return false;

    mCombine = xml.attributes().hasAttribute(XmlSM::xmlSMAttributeCombine)
                    ? fromCombineString(xml.attributes().value(XmlSM::xmlSMAttributeCombine).toString())
                    : eCombine::And;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementConditionList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementCondition)
        {
            SMConditionEntry* row = new SMConditionEntry(this);
            if (row->readFromXml(xml))
            {
                addElement(row, false);
            }
            else
            {
                delete row;
            }
        }

        xml.readNext();
    }

    return true;
}

void SMConditionList::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementConditionList);
    if (mCombine == eCombine::Or)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeCombine, SMConditionList::toString(mCombine));
    }

    for (const SMConditionEntry* row : getElements())
    {
        row->writeToXml(xml);
    }

    xml.writeEndElement();
}
