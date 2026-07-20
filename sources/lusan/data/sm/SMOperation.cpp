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
 *  \file        lusan/data/sm/SMOperation.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operations.
 *
 ************************************************************************/

#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    //!< Writes \p text as a CDATA-wrapped child element (byte-exact, never normalized).
    void writeCDataElem(QXmlStreamWriter& xml, QLatin1StringView elemName, const QString& text)
    {
        xml.writeStartElement(elemName);
        xml.writeCDATA(text);
        xml.writeEndElement();
    }

    //!< Creates the operation object matching an XML element name, or nullptr if unknown.
    SMOperationBase* createOperation(QStringView name, ElementBase* parent)
    {
        if (name == XmlSM::xmlSMElementActionCall)
            return new SMActionCall(parent);
        else if (name == XmlSM::xmlSMElementAttributeSet)
            return new SMAttributeSet(parent);
        else if (name == XmlSM::xmlSMElementTimerStart)
            return new SMTimerStart(parent);
        else if (name == XmlSM::xmlSMElementTimerStop)
            return new SMTimerStop(parent);
        else if (name == XmlSM::xmlSMElementEventSend)
            return new SMEventSend(parent);
        else if (name == XmlSM::xmlSMElementInlineCode)
            return new SMInlineCode(parent);
        else
            return nullptr;
    }
}

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
    else if (source.compare(STR_SRC_LAMBDA, Qt::CaseInsensitive) == 0)
        return eValueSource::Lambda;
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
    case eValueSource::Lambda:      return STR_SRC_LAMBDA;
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

bool SMArgumentEntry::isValid() const
{
    return (mName.isEmpty() == false);
}

bool SMArgumentEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementArgument)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mName   = attributes.value(XmlSM::xmlSMAttributeName).toString();
    mSource = fromSourceString(attributes.value(XmlSM::xmlSMAttributeSource).toString());
    mValue  = attributes.value(XmlSM::xmlSMAttributeValue).toString();
    mExpression.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementArgument))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementExpression)
        {
            mExpression = xml.readElementText();
        }

        xml.readNext();
    }

    return true;
}

void SMArgumentEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementArgument);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, mName);
    xml.writeAttribute(XmlSM::xmlSMAttributeSource, SMArgumentEntry::toString(mSource));
    if (mSource == eValueSource::Expression)
    {
        writeCDataElem(xml, XmlSM::xmlSMElementExpression, mExpression);
    }
    else
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeValue, mValue);
    }

    xml.writeEndElement();
}

void SMArgumentEntry::writeArgumentList(QXmlStreamWriter& xml, const QList<SMArgumentEntry>& args)
{
    if (args.isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementArgumentList);
    for (const SMArgumentEntry& arg : args)
    {
        arg.writeToXml(xml);
    }

    xml.writeEndElement();
}

bool SMArgumentEntry::readArgumentList(QXmlStreamReader& xml, QList<SMArgumentEntry>& args, ElementBase* parent)
{
    if (xml.name() != XmlSM::xmlSMElementArgumentList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementArgumentList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementArgument)
        {
            SMArgumentEntry arg(parent);
            if (arg.readFromXml(xml))
            {
                args.append(std::move(arg));
            }
        }

        xml.readNext();
    }

    return true;
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

const char* SMOperationBase::getElementName() const
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

bool SMOperationBase::isValid() const
{
    return true;
}

bool SMOperationBase::readFromXml(QXmlStreamReader& xml)
{
    // The reader is positioned on the operation start element; the concrete subclass
    // (and therefore mKind) was already selected by the operation-list factory.
    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    const QString elemName = xml.name().toString();

    switch (mKind)
    {
    case eOperation::ActionCall:
        static_cast<SMActionCall*>(this)->setAction(attributes.value(XmlSM::xmlSMAttributeAction).toString());
        break;

    case eOperation::AttributeSet:
    {
        SMAttributeSet* set = static_cast<SMAttributeSet*>(this);
        set->setAttribute(attributes.value(XmlSM::xmlSMAttributeAttribute).toString());
        set->setSource(SMArgumentEntry::fromSourceString(attributes.value(XmlSM::xmlSMAttributeSource).toString()));
        set->setValue(attributes.value(XmlSM::xmlSMAttributeValue).toString());
        break;
    }

    case eOperation::TimerStart:
    {
        SMTimerStart* timer = static_cast<SMTimerStart*>(this);
        timer->setTimer(attributes.value(XmlSM::xmlSMAttributeTimer).toString());
        if (attributes.hasAttribute(XmlSM::xmlSMAttributeTimeout))
        {
            timer->setTimeout(attributes.value(XmlSM::xmlSMAttributeTimeout).toUInt());
        }
        if (attributes.hasAttribute(XmlSM::xmlSMAttributeRepeat))
        {
            timer->setRepeat(attributes.value(XmlSM::xmlSMAttributeRepeat).toUInt());
        }
        break;
    }

    case eOperation::TimerStop:
        static_cast<SMTimerStop*>(this)->setTimer(attributes.value(XmlSM::xmlSMAttributeTimer).toString());
        break;

    case eOperation::EventSend:
        static_cast<SMEventSend*>(this)->setEvent(attributes.value(XmlSM::xmlSMAttributeEvent).toString());
        break;

    case eOperation::InlineCode:
    default:
        break;
    }

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == elemName))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            QStringView name = xml.name();
            if (name == XmlSM::xmlSMElementArgumentList)
            {
                if (mKind == eOperation::ActionCall)
                {
                    SMArgumentEntry::readArgumentList(xml, static_cast<SMActionCall*>(this)->getArguments(), this);
                }
                else if (mKind == eOperation::EventSend)
                {
                    SMArgumentEntry::readArgumentList(xml, static_cast<SMEventSend*>(this)->getArguments(), this);
                }
            }
            else if ((name == XmlSM::xmlSMElementExpression) && (mKind == eOperation::AttributeSet))
            {
                static_cast<SMAttributeSet*>(this)->setExpression(xml.readElementText());
            }
            else if ((name == XmlSM::xmlSMElementBody) && (mKind == eOperation::InlineCode))
            {
                static_cast<SMInlineCode*>(this)->setBody(xml.readElementText());
            }
        }

        xml.readNext();
    }

    return true;
}

void SMOperationBase::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(getElementName());
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));

    switch (mKind)
    {
    case eOperation::ActionCall:
    {
        const SMActionCall* call = static_cast<const SMActionCall*>(this);
        xml.writeAttribute(XmlSM::xmlSMAttributeAction, call->getAction());
        SMArgumentEntry::writeArgumentList(xml, call->getArguments());
        break;
    }

    case eOperation::AttributeSet:
    {
        const SMAttributeSet* set = static_cast<const SMAttributeSet*>(this);
        xml.writeAttribute(XmlSM::xmlSMAttributeAttribute, set->getAttribute());
        xml.writeAttribute(XmlSM::xmlSMAttributeSource, SMArgumentEntry::toString(set->getSource()));
        if (set->getSource() == SMArgumentEntry::eValueSource::Expression)
        {
            writeCDataElem(xml, XmlSM::xmlSMElementExpression, set->getExpression());
        }
        else
        {
            xml.writeAttribute(XmlSM::xmlSMAttributeValue, set->getValue());
        }
        break;
    }

    case eOperation::TimerStart:
    {
        const SMTimerStart* timer = static_cast<const SMTimerStart*>(this);
        xml.writeAttribute(XmlSM::xmlSMAttributeTimer, timer->getTimer());
        if (timer->hasTimeoutOverride())
        {
            xml.writeAttribute(XmlSM::xmlSMAttributeTimeout, QString::number(timer->getTimeout()));
        }
        if (timer->hasRepeatOverride())
        {
            xml.writeAttribute(XmlSM::xmlSMAttributeRepeat, QString::number(timer->getRepeat()));
        }
        break;
    }

    case eOperation::TimerStop:
        xml.writeAttribute(XmlSM::xmlSMAttributeTimer, static_cast<const SMTimerStop*>(this)->getTimer());
        break;

    case eOperation::EventSend:
    {
        const SMEventSend* send = static_cast<const SMEventSend*>(this);
        xml.writeAttribute(XmlSM::xmlSMAttributeEvent, send->getEvent());
        SMArgumentEntry::writeArgumentList(xml, send->getArguments());
        break;
    }

    case eOperation::InlineCode:
    default:
        writeCDataElem(xml, XmlSM::xmlSMElementBody, static_cast<const SMInlineCode*>(this)->getBody());
        break;
    }

    xml.writeEndElement();
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

SMOperationBase* SMActionCall::clone() const
{
    return new SMActionCall(*this);
}

QString SMActionCall::getName() const
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

SMOperationBase* SMAttributeSet::clone() const
{
    return new SMAttributeSet(*this);
}

QString SMAttributeSet::getName() const
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

void SMTimerStart::clearTimeout()
{
    mHasTimeout = false;
    mTimeout    = 0;
}

void SMTimerStart::setRepeat(uint32_t repeat)
{
    mHasRepeat = true;
    mRepeat    = repeat;
}

void SMTimerStart::clearRepeat()
{
    mHasRepeat = false;
    mRepeat    = 0;
}

SMOperationBase* SMTimerStart::clone() const
{
    return new SMTimerStart(*this);
}

QString SMTimerStart::getName() const
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

SMOperationBase* SMTimerStop::clone() const
{
    return new SMTimerStop(*this);
}

QString SMTimerStop::getName() const
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

SMOperationBase* SMEventSend::clone() const
{
    return new SMEventSend(*this);
}

QString SMEventSend::getName() const
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

SMOperationBase* SMInlineCode::clone() const
{
    return new SMInlineCode(*this);
}

QString SMInlineCode::getName() const
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

SMOperationList::~SMOperationList()
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

SMOperationBase* SMOperationList::insertOperation(int index, SMOperationBase* operation)
{
    if (operation != nullptr)
    {
        operation->setParent(mParent);
        if ((index < 0) || (index >= mOperations.size()))
        {
            mOperations.append(operation);
        }
        else
        {
            mOperations.insert(index, operation);
        }
    }

    return operation;
}

SMOperationBase* SMOperationList::takeAt(int index)
{
    if ((index < 0) || (index >= mOperations.size()))
    {
        return nullptr;
    }

    return mOperations.takeAt(index);
}

void SMOperationList::swapOperations(int index1, int index2)
{
    if ((index1 >= 0) && (index1 < mOperations.size()) && (index2 >= 0) && (index2 < mOperations.size()) && (index1 != index2))
    {
        mOperations.swapItemsAt(index1, index2);
    }
}

int SMOperationList::indexOf(uint32_t id) const
{
    for (int i = 0; i < mOperations.size(); ++i)
    {
        if (mOperations.at(i)->getId() == id)
        {
            return i;
        }
    }

    return -1;
}

SMOperationBase* SMOperationList::findById(uint32_t id) const
{
    const int index = indexOf(id);
    return (index >= 0) ? mOperations.at(index) : nullptr;
}

void SMOperationList::clear()
{
    for (SMOperationBase* op : mOperations)
    {
        delete op;
    }

    mOperations.clear();
}

void SMOperationList::writeToXml(QXmlStreamWriter& xml, QLatin1StringView wrapperName) const
{
    if (mOperations.isEmpty())
        return;

    xml.writeStartElement(wrapperName);
    for (const SMOperationBase* op : mOperations)
    {
        op->writeToXml(xml);
    }

    xml.writeEndElement();
}

bool SMOperationList::readFromXml(QXmlStreamReader& xml, QLatin1StringView wrapperName)
{
    if (xml.name() != QLatin1String(wrapperName))
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String(wrapperName)))
    {
        // Skip the wrapper's own start element; only child operation elements are handled.
        if ((xml.tokenType() == QXmlStreamReader::StartElement) && (xml.name() != QLatin1String(wrapperName)))
        {
            SMOperationBase* op = createOperation(xml.name(), mParent);
            if (op != nullptr)
            {
                // readFromXml sets the ID from the file (document order == priority order);
                // the operation is appended directly, never re-numbered.
                op->readFromXml(xml);
                mOperations.append(op);
            }
            else
            {
                xml.skipCurrentElement();
            }
        }

        xml.readNext();
    }

    return true;
}
