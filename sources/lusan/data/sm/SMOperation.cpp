/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMOperation.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operations.
 *
 ************************************************************************/

#include "lusan/data/sm/SMOperation.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

//////////////////////////////////////////////////////////////////////////
// SMArgumentEntry implementation
//////////////////////////////////////////////////////////////////////////

SMArgumentEntry::eValueSource SMArgumentEntry::fromSourceString(const QString& source)
{
    if (source.compare(STR_SRC_PARAM, Qt::CaseInsensitive) == 0)
        return eValueSource::Param;
    else if (source.compare(STR_SRC_ATTRIBUTE, Qt::CaseInsensitive) == 0)
        return eValueSource::Attribute;
    else if (source.compare(STR_SRC_CONSTANT, Qt::CaseInsensitive) == 0)
        return eValueSource::Constant;
    else if (source.compare(STR_SRC_CONDITION, Qt::CaseInsensitive) == 0)
        return eValueSource::Condition;
    else if (source.compare(STR_SRC_EXPRESSION, Qt::CaseInsensitive) == 0)
        return eValueSource::Expression;
    else
        return eValueSource::Value;
}

const char* SMArgumentEntry::toString(SMArgumentEntry::eValueSource source)
{
    switch (source)
    {
    case eValueSource::Param:       return STR_SRC_PARAM;
    case eValueSource::Attribute:   return STR_SRC_ATTRIBUTE;
    case eValueSource::Constant:    return STR_SRC_CONSTANT;
    case eValueSource::Condition:   return STR_SRC_CONDITION;
    case eValueSource::Expression:  return STR_SRC_EXPRESSION;
    case eValueSource::Value:
    default:                        return STR_SRC_VALUE;
    }
}

SMArgumentEntry::SMArgumentEntry(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ( )
    , mSource       (eValueSource::Value)
    , mValue        ( )
    , mExpression   ( )
{
}

SMArgumentEntry::SMArgumentEntry(  uint32_t id
                                 , const QString& name
                                 , eValueSource source /*= eValueSource::Value*/
                                 , const QString& value /*= QString()*/
                                 , ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mName         (name)
    , mSource       (source)
    , mValue        (value)
    , mExpression   ( )
{
}

SMArgumentEntry::SMArgumentEntry(const SMArgumentEntry& src)
    : DocumentElem  (src)
    , mName         (src.mName)
    , mSource       (src.mSource)
    , mValue        (src.mValue)
    , mExpression   (src.mExpression)
{
}

SMArgumentEntry::SMArgumentEntry(SMArgumentEntry&& src) noexcept
    : DocumentElem  (std::move(src))
    , mName         (std::move(src.mName))
    , mSource       (src.mSource)
    , mValue        (std::move(src.mValue))
    , mExpression   (std::move(src.mExpression))
{
}

SMArgumentEntry& SMArgumentEntry::operator = (const SMArgumentEntry& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mName       = other.mName;
        mSource     = other.mSource;
        mValue      = other.mValue;
        mExpression = other.mExpression;
    }

    return *this;
}

SMArgumentEntry& SMArgumentEntry::operator = (SMArgumentEntry&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mName       = std::move(other.mName);
        mSource     = other.mSource;
        mValue      = std::move(other.mValue);
        mExpression = std::move(other.mExpression);
    }

    return *this;
}

bool SMArgumentEntry::isValid(void) const
{
    return (mName.isEmpty() == false);
}

bool SMArgumentEntry::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMArgumentEntry::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

//////////////////////////////////////////////////////////////////////////
// SMOperationBase implementation
//////////////////////////////////////////////////////////////////////////

SMOperationBase::SMOperationBase(eOperation kind, ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mKind         (kind)
{
}

SMOperationBase::SMOperationBase(eOperation kind, uint32_t id, ElementBase* parent)
    : DocumentElem  (id, parent)
    , mKind         (kind)
{
}

SMOperationBase::SMOperationBase(const SMOperationBase& src)
    : DocumentElem  (src)
    , mKind         (src.mKind)
{
}

SMOperationBase::SMOperationBase(SMOperationBase&& src) noexcept
    : DocumentElem  (std::move(src))
    , mKind         (src.mKind)
{
}

SMOperationBase& SMOperationBase::operator = (const SMOperationBase& other)
{
    if (this != &other)
    {
        DocumentElem::operator = (other);
        mKind = other.mKind;
    }

    return *this;
}

SMOperationBase& SMOperationBase::operator = (SMOperationBase&& other) noexcept
{
    if (this != &other)
    {
        DocumentElem::operator = (std::move(other));
        mKind = other.mKind;
    }

    return *this;
}

const char* SMOperationBase::getElementName(void) const
{
    switch (mKind)
    {
    case eOperation::ActionCall:    return STR_ACTION_CALL;
    case eOperation::AttributeSet:  return STR_ATTRIBUTE_SET;
    case eOperation::TimerStart:    return STR_TIMER_START;
    case eOperation::TimerStop:     return STR_TIMER_STOP;
    case eOperation::EventSend:     return STR_EVENT_SEND;
    case eOperation::InlineCode:
    default:                        return STR_INLINE_CODE;
    }
}

bool SMOperationBase::isValid(void) const
{
    return true;
}

bool SMOperationBase::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMOperationBase::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}

//////////////////////////////////////////////////////////////////////////
// SMActionCall implementation
//////////////////////////////////////////////////////////////////////////

SMActionCall::SMActionCall(ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::ActionCall, parent)
    , mAction           ( )
    , mArguments        ( )
{
}

SMActionCall::SMActionCall(uint32_t id, const QString& action, ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::ActionCall, id, parent)
    , mAction           (action)
    , mArguments        ( )
{
}

SMActionCall::SMActionCall(const SMActionCall& src)
    : SMOperationBase   (src)
    , mAction           (src.mAction)
    , mArguments        (src.mArguments)
{
    for (SMArgumentEntry& arg : mArguments)
    {
        arg.setParent(this);
    }
}

SMArgumentEntry* SMActionCall::addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value)
{
    mArguments.append(SMArgumentEntry(getNextId(), name, source, value, this));
    return &mArguments.last();
}

SMOperationBase* SMActionCall::clone(void) const
{
    return new SMActionCall(*this);
}

QString SMActionCall::getName(void) const
{
    return mAction;
}

//////////////////////////////////////////////////////////////////////////
// SMAttributeSet implementation
//////////////////////////////////////////////////////////////////////////

SMAttributeSet::SMAttributeSet(ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::AttributeSet, parent)
    , mAttribute        ( )
    , mSource           (SMArgumentEntry::eValueSource::Value)
    , mValue            ( )
    , mExpression       ( )
{
}

SMAttributeSet::SMAttributeSet(uint32_t id, const QString& attribute, ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::AttributeSet, id, parent)
    , mAttribute        (attribute)
    , mSource           (SMArgumentEntry::eValueSource::Value)
    , mValue            ( )
    , mExpression       ( )
{
}

SMAttributeSet::SMAttributeSet(const SMAttributeSet& src)
    : SMOperationBase   (src)
    , mAttribute        (src.mAttribute)
    , mSource           (src.mSource)
    , mValue            (src.mValue)
    , mExpression       (src.mExpression)
{
}

SMOperationBase* SMAttributeSet::clone(void) const
{
    return new SMAttributeSet(*this);
}

QString SMAttributeSet::getName(void) const
{
    return mAttribute;
}

//////////////////////////////////////////////////////////////////////////
// SMTimerStart implementation
//////////////////////////////////////////////////////////////////////////

SMTimerStart::SMTimerStart(ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::TimerStart, parent)
    , mTimer            ( )
    , mHasTimeout       (false)
    , mTimeout          (0)
    , mHasRepeat        (false)
    , mRepeat           (0)
{
}

SMTimerStart::SMTimerStart(uint32_t id, const QString& timer, ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::TimerStart, id, parent)
    , mTimer            (timer)
    , mHasTimeout       (false)
    , mTimeout          (0)
    , mHasRepeat        (false)
    , mRepeat           (0)
{
}

SMTimerStart::SMTimerStart(const SMTimerStart& src)
    : SMOperationBase   (src)
    , mTimer            (src.mTimer)
    , mHasTimeout       (src.mHasTimeout)
    , mTimeout          (src.mTimeout)
    , mHasRepeat        (src.mHasRepeat)
    , mRepeat           (src.mRepeat)
{
}

void SMTimerStart::setTimeout(uint32_t timeout)
{
    mHasTimeout = true;
    mTimeout    = timeout;
}

void SMTimerStart::clearTimeout(void)
{
    mHasTimeout = false;
    mTimeout    = 0;
}

void SMTimerStart::setRepeat(uint32_t repeat)
{
    mHasRepeat = true;
    mRepeat    = repeat;
}

void SMTimerStart::clearRepeat(void)
{
    mHasRepeat = false;
    mRepeat    = 0;
}

SMOperationBase* SMTimerStart::clone(void) const
{
    return new SMTimerStart(*this);
}

QString SMTimerStart::getName(void) const
{
    return mTimer;
}

//////////////////////////////////////////////////////////////////////////
// SMTimerStop implementation
//////////////////////////////////////////////////////////////////////////

SMTimerStop::SMTimerStop(ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::TimerStop, parent)
    , mTimer            ( )
{
}

SMTimerStop::SMTimerStop(uint32_t id, const QString& timer, ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::TimerStop, id, parent)
    , mTimer            (timer)
{
}

SMTimerStop::SMTimerStop(const SMTimerStop& src)
    : SMOperationBase   (src)
    , mTimer            (src.mTimer)
{
}

SMOperationBase* SMTimerStop::clone(void) const
{
    return new SMTimerStop(*this);
}

QString SMTimerStop::getName(void) const
{
    return mTimer;
}

//////////////////////////////////////////////////////////////////////////
// SMEventSend implementation
//////////////////////////////////////////////////////////////////////////

SMEventSend::SMEventSend(ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::EventSend, parent)
    , mEvent            ( )
    , mArguments        ( )
{
}

SMEventSend::SMEventSend(uint32_t id, const QString& event, ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::EventSend, id, parent)
    , mEvent            (event)
    , mArguments        ( )
{
}

SMEventSend::SMEventSend(const SMEventSend& src)
    : SMOperationBase   (src)
    , mEvent            (src.mEvent)
    , mArguments        (src.mArguments)
{
    for (SMArgumentEntry& arg : mArguments)
    {
        arg.setParent(this);
    }
}

SMArgumentEntry* SMEventSend::addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value)
{
    mArguments.append(SMArgumentEntry(getNextId(), name, source, value, this));
    return &mArguments.last();
}

SMOperationBase* SMEventSend::clone(void) const
{
    return new SMEventSend(*this);
}

QString SMEventSend::getName(void) const
{
    return mEvent;
}

//////////////////////////////////////////////////////////////////////////
// SMInlineCode implementation
//////////////////////////////////////////////////////////////////////////

SMInlineCode::SMInlineCode(ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::InlineCode, parent)
    , mBody             ( )
{
}

SMInlineCode::SMInlineCode(uint32_t id, const QString& body, ElementBase* parent /*= nullptr*/)
    : SMOperationBase   (eOperation::InlineCode, id, parent)
    , mBody             (body)
{
}

SMInlineCode::SMInlineCode(const SMInlineCode& src)
    : SMOperationBase   (src)
    , mBody             (src.mBody)
{
}

SMOperationBase* SMInlineCode::clone(void) const
{
    return new SMInlineCode(*this);
}

QString SMInlineCode::getName(void) const
{
    return QString();
}

//////////////////////////////////////////////////////////////////////////
// SMOperationList implementation
//////////////////////////////////////////////////////////////////////////

SMOperationList::SMOperationList(ElementBase* parent /*= nullptr*/)
    : mParent       (parent)
    , mOperations   ( )
{
}

SMOperationList::SMOperationList(const SMOperationList& src)
    : mParent       (src.mParent)
    , mOperations   ( )
{
    mOperations.reserve(src.mOperations.size());
    for (const SMOperationBase* op : src.mOperations)
    {
        SMOperationBase* copy = op->clone();
        copy->setParent(mParent);
        mOperations.append(copy);
    }
}

SMOperationList::SMOperationList(SMOperationList&& src) noexcept
    : mParent       (src.mParent)
    , mOperations   (std::move(src.mOperations))
{
    src.mOperations.clear();
}

SMOperationList::~SMOperationList(void)
{
    clear();
}

SMOperationList& SMOperationList::operator = (const SMOperationList& other)
{
    if (this != &other)
    {
        clear();
        mParent = other.mParent;
        mOperations.reserve(other.mOperations.size());
        for (const SMOperationBase* op : other.mOperations)
        {
            SMOperationBase* copy = op->clone();
            copy->setParent(mParent);
            mOperations.append(copy);
        }
    }

    return *this;
}

SMOperationList& SMOperationList::operator = (SMOperationList&& other) noexcept
{
    if (this != &other)
    {
        clear();
        mParent = other.mParent;
        mOperations = std::move(other.mOperations);
        other.mOperations.clear();
    }

    return *this;
}

void SMOperationList::setParent(ElementBase* parent)
{
    mParent = parent;
    for (SMOperationBase* op : mOperations)
    {
        op->setParent(parent);
    }
}

SMOperationBase* SMOperationList::addOperation(SMOperationBase* operation)
{
    if (operation != nullptr)
    {
        // The operation is created without an ID (default/parent constructor)
        operation->setParent(mParent);
        operation->setId((mParent != nullptr) ? mParent->getNextId() : operation->getNextId());
        mOperations.append(operation);
    }

    return operation;
}

void SMOperationList::clear(void)
{
    for (SMOperationBase* op : mOperations)
    {
        delete op;
    }

    mOperations.clear();
}
