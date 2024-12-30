#include "lusan/data/si/SIMethodBroadcast.hpp"
#include "lusan/common/XmlSI.hpp"

SIMethodBroadcast::SIMethodBroadcast(void)
    : SIMethodBase()
{
    mMethodType = eMethodType::MethodBroadcast;
}

SIMethodBroadcast::SIMethodBroadcast(uint32_t id, const QString& name, const QString& description)
    : SIMethodBase(id, name, description, eMethodType::MethodBroadcast)
{
}

SIMethodBroadcast::SIMethodBroadcast(const SIMethodBroadcast& src)
    : SIMethodBase(src)
{
}

SIMethodBroadcast::SIMethodBroadcast(SIMethodBroadcast&& src) noexcept
    : SIMethodBase(std::move(src))
{
}

SIMethodBroadcast::~SIMethodBroadcast(void)
{
}

SIMethodBroadcast& SIMethodBroadcast::operator=(const SIMethodBroadcast& other)
{
    if (this != &other)
    {
        SIMethodBase::operator=(other);
    }
    return *this;
}

SIMethodBroadcast& SIMethodBroadcast::operator=(SIMethodBroadcast&& other) noexcept
{
    if (this != &other)
    {
        SIMethodBase::operator=(std::move(other));
    }
    return *this;
}

bool SIMethodBroadcast::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() == XmlSI::xmlSIElementAttribute && xml.attributes().value(XmlSI::xmlSIAttributeType) == getType())
    {
        QXmlStreamAttributes attributes = xml.attributes();
        mId = attributes.value(XmlSI::xmlSIAttributeID).toUInt();
        mName = attributes.value(XmlSI::xmlSIAttributeName).toString();
        bool isDeprecated = attributes.hasAttribute(XmlSI::xmlSIAttributeIsDeprecated) ? attributes.value("IsDeprecated").toString() == "true" : false;
        mDeprecateHint.clear();

        while (xml.readNextStartElement())
        {
            if (xml.name() == XmlSI::xmlSIElementDescription)
            {
                mDescription = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementDeprecateHint && isDeprecated)
            {
                mDeprecateHint = xml.readElementText();
            }
            else if (xml.name() == XmlSI::xmlSIElementAttributeList)
            {
                while (xml.readNextStartElement())
                {
                    if (xml.name() == XmlSI::xmlSIElementParameter)
                    {
                        MethodParameter parameter;
                        if (parameter.readFromXml(xml))
                        {
                            mParameters.append(parameter);
                        }
                        else
                        {
                            xml.skipCurrentElement();
                        }
                    }
                    else
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
        return true;
    }
    return false;
}

void SIMethodBroadcast::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSI::xmlSIElementAttribute);
    xml.writeAttribute(XmlSI::xmlSIAttributeID, QString::number(mId));
    xml.writeAttribute(XmlSI::xmlSIAttributeType, getType());
    xml.writeAttribute(XmlSI::xmlSIAttributeName, mName);

    if (!mDescription.isEmpty())
    {
        xml.writeTextElement(XmlSI::xmlSIElementDescription, mDescription);
    }

    if (mIsDeprecated)
    {
        xml.writeAttribute(XmlSI::xmlSIAttributeIsDeprecated, "true");
        if (!mDeprecateHint.isEmpty())
        {
            xml.writeTextElement(XmlSI::xmlSIElementDeprecateHint, mDeprecateHint);
        }
    }

    if (!mParameters.isEmpty())
    {
        xml.writeStartElement(XmlSI::xmlSIElementAttributeList);
        for (const auto& parameter : mParameters)
        {
            parameter.writeToXml(xml);
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
}
