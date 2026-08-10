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
 *  \file        lusan/data/common/MethodEntry.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, one declared method, shared by every document editor.
 *
 ************************************************************************/

#include "lusan/data/common/MethodEntry.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/common/XmlSI.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    //!< The kind an entry answers with when its index names no configured kind: it carries
    //!< nothing, which is exactly what a method with no configuration is.
    const MethodKind& emptyKind()
    {
        static const MethodKind _empty{ QString(), QString(), QString(), false, false, false, false };
        return _empty;
    }

    bool isTrue(const QStringView& value)
    {
        return value.compare(XmlSI::xmlSIValueTrue, Qt::CaseInsensitive) == 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// Static helpers
//////////////////////////////////////////////////////////////////////////

MethodEntry::eImplement MethodEntry::fromImplementString(const QString& implement)
{
    return (implement.compare(MethodEntry::STR_IMPL_EMBEDDED, Qt::CaseInsensitive) == 0)
                ? eImplement::Embedded
                : eImplement::Handler;
}

const char* MethodEntry::toString(MethodEntry::eImplement implement)
{
    return (implement == eImplement::Embedded) ? MethodEntry::STR_IMPL_EMBEDDED : MethodEntry::STR_IMPL_HANDLER;
}

const MethodConfig& MethodEntry::defaultConfig()
{
    // Every field and no kind: an entry that no section has stamped yet keeps whatever it was
    // given, and the parameter default takes the attribute spelling section 5's rule asks for.
    static const MethodConfig _config{ QList<MethodKind>{}, QString::fromLatin1(MethodEntry::DEFAULT_RETURN), true, true };
    return _config;
}

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////

MethodEntry::MethodEntry(ElementBase* parent /*= nullptr*/)
    : MethodBase    (parent)
    , mConfig       (defaultConfig())
    , mKind         (0)
    , mReply        ( )
    , mReturn       (QString::fromLatin1(MethodEntry::DEFAULT_RETURN))
    , mImplement    (eImplement::Handler)
    , mBody         ( )
    , mIsDeprecated (false)
    , mDeprecateHint( )
{
}

MethodEntry::MethodEntry(uint32_t id, const QString& name, int kind, const MethodConfig& config, ElementBase* parent /*= nullptr*/)
    : MethodBase    (id, name, QString(), parent)
    , mConfig       (config)
    , mKind         (kind)
    , mReply        ( )
    , mReturn       (config.defaultReturn.isEmpty() ? QString::fromLatin1(MethodEntry::DEFAULT_RETURN) : config.defaultReturn)
    , mImplement    (eImplement::Handler)
    , mBody         ( )
    , mIsDeprecated (false)
    , mDeprecateHint( )
{
}

MethodEntry::MethodEntry(const MethodEntry& src)
    : MethodBase    (src)
    , mConfig       (src.mConfig)
    , mKind         (src.mKind)
    , mReply        (src.mReply)
    , mReturn       (src.mReturn)
    , mImplement    (src.mImplement)
    , mBody         (src.mBody)
    , mIsDeprecated (src.mIsDeprecated)
    , mDeprecateHint(src.mDeprecateHint)
{
}

MethodEntry::MethodEntry(MethodEntry&& src) noexcept
    : MethodBase    (std::move(src))
    , mConfig       (std::move(src.mConfig))
    , mKind         (src.mKind)
    , mReply        (std::move(src.mReply))
    , mReturn       (std::move(src.mReturn))
    , mImplement    (src.mImplement)
    , mBody         (std::move(src.mBody))
    , mIsDeprecated (src.mIsDeprecated)
    , mDeprecateHint(std::move(src.mDeprecateHint))
{
}

MethodEntry& MethodEntry::operator = (const MethodEntry& other)
{
    if (this != &other)
    {
        MethodBase::operator = (other);
        mConfig        = other.mConfig;
        mKind          = other.mKind;
        mReply         = other.mReply;
        mReturn        = other.mReturn;
        mImplement     = other.mImplement;
        mBody          = other.mBody;
        mIsDeprecated  = other.mIsDeprecated;
        mDeprecateHint = other.mDeprecateHint;
    }

    return *this;
}

MethodEntry& MethodEntry::operator = (MethodEntry&& other) noexcept
{
    if (this != &other)
    {
        MethodBase::operator = (std::move(other));
        mConfig        = std::move(other.mConfig);
        mKind          = other.mKind;
        mReply         = std::move(other.mReply);
        mReturn        = std::move(other.mReturn);
        mImplement     = other.mImplement;
        mBody          = std::move(other.mBody);
        mIsDeprecated  = other.mIsDeprecated;
        mDeprecateHint = std::move(other.mDeprecateHint);
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void MethodEntry::setKind(int kind)
{
    mKind = kind;
    if (hasReturn() && mReturn.isEmpty())
    {
        mReturn = mConfig.defaultReturn.isEmpty() ? QString::fromLatin1(MethodEntry::DEFAULT_RETURN) : mConfig.defaultReturn;
    }
}

const MethodKind& MethodEntry::kind() const
{
    return ((mKind >= 0) && (mKind < mConfig.kinds.size())) ? mConfig.kinds.at(mKind) : emptyKind();
}

QString MethodEntry::getType() const
{
    return kind().token;
}

void MethodEntry::setConfig(const MethodConfig& config)
{
    mConfig = config;
}

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////

bool MethodEntry::isValid() const
{
    return (getName().isEmpty() == false);
}

QIcon MethodEntry::getIcon(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
    {
        const QString& icon = kind().icon;
        return icon.isEmpty() ? QIcon() : NELusanCommon::loadIcon(icon, NELusanCommon::SizeSmall);
    }

    case ElementBase::eDisplay::DisplayLink:
    {
        // A method that names no answer shows nothing; one that names an answer no longer
        // there shows the warning the check reports.
        if (hasReply() == false)
            return QIcon();
        else if (mReply.isEmpty())
            return QIcon();
        else
            return NELusanCommon::iconMethodResponse(NELusanCommon::SizeSmall);
    }

    default:
        return QIcon();
    }
}

QString MethodEntry::getString(ElementBase::eDisplay display) const
{
    switch (display)
    {
    case ElementBase::eDisplay::DisplayName:
        return getName();

    case ElementBase::eDisplay::DisplayType:
        return kind().label.isEmpty() ? kind().token : kind().label;

    case ElementBase::eDisplay::DisplayValue:
        // A method that returns a value is described by the type it returns; the rest by how
        // many parameters they take.
        return hasReturn() ? mReturn
                           : ((getElementCount() > 0) ? QStringLiteral("%1 param(s)").arg(getElementCount()) : QString());

    case ElementBase::eDisplay::DisplayLink:
        return hasReply() ? mReply : QString();

    default:
        return QString();
    }
}

bool MethodEntry::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSI::xmlSIElementMethod)
        return false;

    const QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
    setName(attributes.value(XmlSI::xmlSIAttributeName).toString());

    // The kind is stored by its file spelling; an unknown spelling falls back to the first kind
    // the document offers, which is what an editor would have shown for it anyway.
    const QString token = attributes.value(XmlSI::xmlSIAttributeMethodType).toString();
    mKind = 0;
    for (int i = 0; i < mConfig.kinds.size(); ++i)
    {
        if (mConfig.kinds.at(i).token.compare(token, Qt::CaseInsensitive) == 0)
        {
            mKind = i;
            break;
        }
    }

    mReply = attributes.hasAttribute(XmlSI::xmlSIAttributeResponse)
                ? attributes.value(XmlSI::xmlSIAttributeResponse).toString()
                : QString();
    mReturn = attributes.hasAttribute(XmlSI::xmlSIAttributeReturn)
                ? attributes.value(XmlSI::xmlSIAttributeReturn).toString()
                : (mConfig.defaultReturn.isEmpty() ? QString::fromLatin1(MethodEntry::DEFAULT_RETURN) : mConfig.defaultReturn);
    mImplement = attributes.hasAttribute(XmlSI::xmlSIAttributeImplement)
                ? fromImplementString(attributes.value(XmlSI::xmlSIAttributeImplement).toString())
                : eImplement::Handler;
    mIsDeprecated = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated)
                && isTrue(attributes.value(XmlSI::xmlSIAttributeIsDeprecated));

    setDescription(QString());
    mDeprecateHint.clear();
    mBody.clear();

    while (xml.readNextStartElement())
    {
        if (xml.name() == XmlSI::xmlSIElementDescription)
        {
            setDescription(xml.readElementText());
        }
        else if (xml.name() == XmlSI::xmlSIElementDeprecateHint)
        {
            mDeprecateHint = xml.readElementText();
        }
        else if (xml.name() == XmlSI::xmlSIElementBody)
        {
            mBody = xml.readElementText();
        }
        else if (xml.name() == XmlSI::xmlSIElementParamList)
        {
            readParamList(xml);
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return true;
}

void MethodEntry::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementMethod);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSI::xmlSIAttributeName, getName());
    xml.writeAttribute(XmlSI::xmlSIAttributeMethodType, getType());

    if (hasReply() && (mReply.isEmpty() == false))
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeResponse, mReply);
    }

    if (hasReturn() && (mReturn.isEmpty() == false))
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeReturn, mReturn);
    }

    if (hasImplement())
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeImplement, MethodEntry::toString(mImplement));
    }

    if (mIsDeprecated)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
        writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, mDeprecateHint, true);
    }

    writeTextElem(xml, XmlSI::xmlSIElementDescription, getDescription(), mConfig.omitEmptyDescription);
    writeParamList(xml);

    if (isEmbedded())
    {
        xml.writeStartElement(XmlSI::xmlSIElementBody);
        xml.writeCDATA(mBody);
        xml.writeEndElement();
    }

    xml.writeEndElement();
}

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////

void MethodEntry::readParamList(QXmlStreamReader& xml)
{
    while (xml.readNextStartElement())
    {
        if (xml.name() != XmlSI::xmlSIElementParameter)
        {
            xml.skipCurrentElement();
            continue;
        }

        MethodParameter param(this);
        param.setParent(this);      // MethodParameter's constructor does not forward the parent

        const QXmlStreamAttributes attributes = xml.attributes();
        param.setId(attributes.value(XmlSI::xmlSIAttributeID).toUInt());
        param.setName(attributes.value(XmlSI::xmlSIAttributeName).toString());
        param.setType(attributes.value(XmlSI::xmlSIAttributeDataType).toString());
        param.setIsDeprecated(attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated)
                              && isTrue(attributes.value(XmlSI::xmlSIAttributeIsDeprecated)));

        // Both spellings of a default are read, whichever document this is: the attribute the
        // format rule asks for, and the child element the service interface files still carry.
        if (attributes.hasAttribute(XmlSI::xmlSIAttributeDefault))
        {
            param.setValue(attributes.value(XmlSI::xmlSIAttributeDefault).toString());
            param.setDefault(true);
        }

        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementValue)
            {
                const bool isDefault = isTrue(xml.attributes().value(XmlSI::xmlSIAttributeIsDefault));
                param.setValue(xml.readElementText());
                param.setDefault(isDefault);
            }
            else if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                param.setDescription(xml.readElementText());
            }
            else if (xml.name() == XmlSI::xmlSIElementDeprecateHint)
            {
                param.setDeprecateHint(xml.readElementText());
            }
            else
            {
                xml.skipCurrentElement();
            }
        }

        addElement(std::move(param), false);
    }
}

void MethodEntry::writeParamList(QXmlStreamWriter& xml) const
{
    const QList<MethodParameter>& params = getElements();
    if (params.isEmpty())
        return;

    xml.writeStartElement(XmlSI::xmlSIElementParamList);
    for (const MethodParameter& param : params)
    {
        xml.writeStartElement(XmlSI::xmlSIElementParameter);
        xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(param.getId()));
        xml.writeAttribute(XmlSI::xmlSIAttributeName, param.getName());
        xml.writeAttribute(XmlSI::xmlSIAttributeDataType, param.getType());

        if (mConfig.paramDefaultAsAttribute && param.hasDefault())
        {
            xml.writeAttribute(XmlSI::xmlSIAttributeDefault, param.getValue());
        }

        if (param.getIsDeprecated())
        {
            xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, XmlSI::xmlSIValueTrue);
            writeTextElem(xml, XmlSI::xmlSIElementDeprecateHint, param.getDeprecateHint(), true);
        }

        if (mConfig.paramDefaultAsAttribute == false)
        {
            // The flag is what the document stores, so a default with no literal yet still
            // comes back as a default.
            if (param.hasDefault() || (param.getValue().isEmpty() == false))
            {
                xml.writeStartElement(XmlSI::xmlSIElementValue);
                xml.writeAttribute(XmlSI::xmlSIAttributeIsDefault, param.hasDefault() ? XmlSI::xmlSIValueTrue : XmlSI::xmlSIValueFalse);
                xml.writeCharacters(param.getValue());
                xml.writeEndElement();
            }
        }

        writeTextElem(xml, XmlSI::xmlSIElementDescription, param.getDescription(), mConfig.omitEmptyDescription);
        xml.writeEndElement();
    }

    xml.writeEndElement();
}
