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
 *  \file        lusan/data/sm/SMMethodData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM methods registry: triggers, actions, conditions
 *
 ************************************************************************/

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/common/NELusanCommon.hpp"
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

    //!< Writes a `.fsml` `ParamList`; the default value is the `Default` attribute (not the
    //!< `.siml` `<Value>` child). Empty lists are omitted.
    void writeParamList(QXmlStreamWriter& xml, const MethodBase& method)
    {
        const QList<MethodParameter>& params = method.getElements();
        if (params.isEmpty())
            return;

        xml.writeStartElement(XmlSM::xmlSMElementParamList);
        for (const MethodParameter& param : params)
        {
            xml.writeStartElement(XmlSM::xmlSMElementParameter);
            xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(param.getId()));
            xml.writeAttribute(XmlSM::xmlSMAttributeName, param.getName());
            xml.writeAttribute(XmlSM::xmlSMAttributeDataType, param.getType());
            if (param.hasDefault())
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeDefault, param.getValue());
            }
            if (param.getIsDeprecated())
            {
                xml.writeAttribute(XmlSM::xmlSMAttributeIsDeprecated, XmlSM::xmlSMValueTrue);
                if (param.getDeprecateHint().isEmpty() == false)
                {
                    xml.writeTextElement(XmlSM::xmlSMElementDeprecateHint, param.getDeprecateHint());
                }
            }
            if (param.getDescription().isEmpty() == false)
            {
                xml.writeTextElement(XmlSM::xmlSMElementDescription, param.getDescription());
            }
            xml.writeEndElement();
        }

        xml.writeEndElement();
    }

    //!< Reads a `.fsml` `ParamList` (reader positioned on its start element) into \p method.
    void readParamList(QXmlStreamReader& xml, MethodBase& method)
    {
        while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementParamList))
        {
            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementParameter)
            {
                MethodParameter param(&method);
                param.setParent(&method);   // MethodParameter's ctor does not forward the parent
                QXmlStreamAttributes attributes = xml.attributes();
                param.setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
                param.setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
                param.setType(attributes.value(XmlSM::xmlSMAttributeDataType).toString());
                if (attributes.hasAttribute(XmlSM::xmlSMAttributeDefault))
                {
                    param.setValue(attributes.value(XmlSM::xmlSMAttributeDefault).toString());
                    param.setDefault(true);
                }
                param.setIsDeprecated(attributes.hasAttribute(XmlSM::xmlSMAttributeIsDeprecated)
                    && (attributes.value(XmlSM::xmlSMAttributeIsDeprecated).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0));

                while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementParameter))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement)
                    {
                        if (xml.name() == XmlSM::xmlSMElementDeprecateHint)
                        {
                            param.setDeprecateHint(xml.readElementText());
                        }
                        else if (xml.name() == XmlSM::xmlSMElementDescription)
                        {
                            param.setDescription(xml.readElementText());
                        }
                    }

                    xml.readNext();
                }

                method.addElement(std::move(param), true);
            }

            xml.readNext();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SMMethodEntry static helpers
//////////////////////////////////////////////////////////////////////////

SMMethodEntry::eMethodType SMMethodEntry::fromTypeString(const QString& type)
{
    if (type.compare(SMMethodEntry::STR_TYPE_ACTION, Qt::CaseInsensitive) == 0)
        return eMethodType::Action;
    else if (type.compare(SMMethodEntry::STR_TYPE_CONDITION, Qt::CaseInsensitive) == 0)
        return eMethodType::Condition;
    else
        return eMethodType::Trigger;
}

const char* SMMethodEntry::toString(SMMethodEntry::eMethodType type)
{
    switch (type)
    {
    case eMethodType::Action:
        return SMMethodEntry::STR_TYPE_ACTION;
    case eMethodType::Condition:
        return SMMethodEntry::STR_TYPE_CONDITION;
    case eMethodType::Trigger:
    default:
        return SMMethodEntry::STR_TYPE_TRIGGER;
    }
}

SMMethodEntry::eImplement SMMethodEntry::fromImplementString(const QString& implement)
{
    return (implement.compare(SMMethodEntry::STR_IMPL_EMBEDDED, Qt::CaseInsensitive) == 0)
                ? eImplement::Embedded
                : eImplement::Handler;
}

const char* SMMethodEntry::toString(SMMethodEntry::eImplement implement)
{
    switch (implement)
    {
    case eImplement::Embedded:
        return SMMethodEntry::STR_IMPL_EMBEDDED;
    case eImplement::Handler:
    default:
        return SMMethodEntry::STR_IMPL_HANDLER;
    }
}

//////////////////////////////////////////////////////////////////////////
// SMMethodEntry implementation
//////////////////////////////////////////////////////////////////////////

SMMethodEntry::SMMethodEntry(ElementBase* parent /*= nullptr*/)
    : MethodBase    (parent)
    , mMethodType   (eMethodType::Trigger)
    , mReturn       (SMMethodEntry::DEFAULT_RETURN)
    , mImplement    (eImplement::Handler)
    , mBody         ( )
    , mIsDeprecated (false)
    , mDeprecateHint( )
{
}

SMMethodEntry::SMMethodEntry(uint32_t id, const QString& name, eMethodType type, ElementBase* parent /*= nullptr*/)
    : MethodBase    (id, name, QString(), parent)
    , mMethodType   (type)
    , mReturn       (SMMethodEntry::DEFAULT_RETURN)
    , mImplement    (eImplement::Handler)
    , mBody         ( )
    , mIsDeprecated (false)
    , mDeprecateHint( )
{
}

SMMethodEntry::SMMethodEntry(const SMMethodEntry& src)
    : MethodBase    (src)
    , mMethodType   (src.mMethodType)
    , mReturn       (src.mReturn)
    , mImplement    (src.mImplement)
    , mBody         (src.mBody)
    , mIsDeprecated (src.mIsDeprecated)
    , mDeprecateHint(src.mDeprecateHint)
{
}

SMMethodEntry::SMMethodEntry(SMMethodEntry&& src) noexcept
    : MethodBase    (std::move(src))
    , mMethodType   (src.mMethodType)
    , mReturn       (std::move(src.mReturn))
    , mImplement    (src.mImplement)
    , mBody         (std::move(src.mBody))
    , mIsDeprecated (src.mIsDeprecated)
    , mDeprecateHint(std::move(src.mDeprecateHint))
{
}

SMMethodEntry& SMMethodEntry::operator = (const SMMethodEntry& other)
{
    if (this != &other)
    {
        MethodBase::operator = (other);
        mMethodType    = other.mMethodType;
        mReturn        = other.mReturn;
        mImplement     = other.mImplement;
        mBody          = other.mBody;
        mIsDeprecated  = other.mIsDeprecated;
        mDeprecateHint = other.mDeprecateHint;
    }

    return *this;
}

SMMethodEntry& SMMethodEntry::operator = (SMMethodEntry&& other) noexcept
{
    if (this != &other)
    {
        MethodBase::operator = (std::move(other));
        mMethodType    = other.mMethodType;
        mReturn        = std::move(other.mReturn);
        mImplement     = other.mImplement;
        mBody          = std::move(other.mBody);
        mIsDeprecated  = other.mIsDeprecated;
        mDeprecateHint = std::move(other.mDeprecateHint);
    }

    return *this;
}

bool SMMethodEntry::isValid() const
{
    return (getName().isEmpty() == false);
}

QIcon SMMethodEntry::getIcon(ElementBase::eDisplay display) const
{
    if (display != ElementBase::eDisplay::DisplayName)
        return QIcon();

    switch (mMethodType)
    {
    case eMethodType::Action:
        return NELusanCommon::iconMethodAction(NELusanCommon::SizeSmall);
    case eMethodType::Condition:
        return NELusanCommon::iconMethodCondition(NELusanCommon::SizeSmall);
    case eMethodType::Trigger:
    default:
        return NELusanCommon::iconMethodTrigger(NELusanCommon::SizeSmall);
    }
}

QString SMMethodEntry::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();
    case ElementBase::eDisplay::DisplayType:
        return QString::fromLatin1(SMMethodEntry::toString(mMethodType));
    case ElementBase::eDisplay::DisplayValue:
        // Conditions are defined by their return type; the others by their parameter count.
        return (mMethodType == eMethodType::Condition)
            ? mReturn
            : ((getElementCount() > 0) ? QStringLiteral("%1 param(s)").arg(getElementCount()) : QString());
    default:
        return QString();
    }
}

bool SMMethodEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementMethod)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    setName(attributes.value(XmlSM::xmlSMAttributeName).toString());
    mMethodType = fromTypeString(attributes.value(XmlSM::xmlSMAttributeMethodType).toString());
    if (attributes.hasAttribute(XmlSM::xmlSMAttributeReturn))
    {
        mReturn = attributes.value(XmlSM::xmlSMAttributeReturn).toString();
    }
    if (attributes.hasAttribute(XmlSM::xmlSMAttributeImplement))
    {
        mImplement = fromImplementString(attributes.value(XmlSM::xmlSMAttributeImplement).toString());
    }
    setDescription(QString());
    mBody.clear();
    mIsDeprecated = attributes.hasAttribute(XmlSM::xmlSMAttributeIsDeprecated)
        && (attributes.value(XmlSM::xmlSMAttributeIsDeprecated).toString().compare(XmlSM::xmlSMValueTrue, Qt::CaseInsensitive) == 0);
    mDeprecateHint.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementMethod))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == XmlSM::xmlSMElementDeprecateHint)
            {
                mDeprecateHint = xml.readElementText();
            }
            else if (xml.name() == XmlSM::xmlSMElementDescription)
            {
                setDescription(xml.readElementText());
            }
            else if (xml.name() == XmlSM::xmlSMElementParamList)
            {
                readParamList(xml, *this);
            }
            else if (xml.name() == XmlSM::xmlSMElementBody)
            {
                mBody = xml.readElementText();
            }
        }

        xml.readNext();
    }

    return true;
}

void SMMethodEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementMethod);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, getName());
    xml.writeAttribute(XmlSM::xmlSMAttributeMethodType, SMMethodEntry::toString(mMethodType));
    if (mMethodType == eMethodType::Condition)
    {
        if (mReturn.isEmpty() == false)
        {
            xml.writeAttribute(XmlSM::xmlSMAttributeReturn, mReturn);
        }
        xml.writeAttribute(XmlSM::xmlSMAttributeImplement, SMMethodEntry::toString(mImplement));
    }
    if (mIsDeprecated)
    {
        xml.writeAttribute(XmlSM::xmlSMAttributeIsDeprecated, XmlSM::xmlSMValueTrue);
        writeTextElem(xml, XmlSM::xmlSMElementDeprecateHint, mDeprecateHint, true);
    }

    writeTextElem(xml, XmlSM::xmlSMElementDescription, getDescription(), true);
    writeParamList(xml, *this);

    if ((mMethodType == eMethodType::Condition) && (mImplement == eImplement::Embedded))
    {
        writeCDataElem(xml, XmlSM::xmlSMElementBody, mBody);
    }

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// SMMethodData implementation
//////////////////////////////////////////////////////////////////////////

SMMethodData::SMMethodData(ElementBase* parent /*= nullptr*/)
    : TEDataContainer<SMMethodEntry*, DocumentElem>(parent)
{
}

SMMethodData::~SMMethodData()
{
    removeAll();
}

bool SMMethodData::isValid() const
{
    return true;
}

bool SMMethodData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementMethodList)
        return false;

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementMethodList))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementMethod)
        {
            SMMethodEntry* entry = new SMMethodEntry(this);
            if (entry->readFromXml(xml))
            {
                addElement(entry, true);
            }
            else
            {
                delete entry;
            }
        }

        xml.readNext();
    }

    return true;
}

void SMMethodData::writeToXml(QXmlStreamWriter& xml) const
{
    if (getElements().isEmpty())
        return;

    xml.writeStartElement(XmlSM::xmlSMElementMethodList);
    for (const SMMethodEntry* entry : getElements())
    {
        entry->writeToXml(xml);
    }

    xml.writeEndElement();
}

SMMethodEntry* SMMethodData::createMethod(const QString& name, SMMethodEntry::eMethodType type)
{
    if (findMethod(name) != nullptr)
    {
        return nullptr;
    }

    SMMethodEntry* entry = new SMMethodEntry(getNextId(), name, type, this);
    addElement(entry, true);
    return entry;
}

SMMethodEntry* SMMethodData::findMethod(const QString& name) const
{
    SMMethodEntry* const* found = findElement(name);
    return (found != nullptr) ? *found : nullptr;
}

SMMethodEntry* SMMethodData::findMethod(uint32_t id) const
{
    SMMethodEntry* const* found = findElement(id);
    return (found != nullptr) ? *found : nullptr;
}

SMMethodEntry* SMMethodData::findTrigger(const QString& name) const
{
    SMMethodEntry* method = findMethod(name);
    return ((method != nullptr) && method->isTrigger()) ? method : nullptr;
}

void SMMethodData::removeAll()
{
    for (SMMethodEntry* entry : getElements())
    {
        delete entry;
    }

    removeAllElements();
}
