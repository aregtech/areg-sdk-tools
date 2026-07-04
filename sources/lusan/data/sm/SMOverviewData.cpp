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
 *  \file        lusan/data/sm/SMOverviewData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview data (name, user version, threading mode).
 *
 ************************************************************************/

#include "lusan/data/sm/SMOverviewData.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMOverviewData::eThreading SMOverviewData::fromThreadingString(const QString& threading)
{
    return (threading.compare(SMOverviewData::STR_THREADING_LOCAL, Qt::CaseInsensitive) == 0)
                ? eThreading::Local
                : eThreading::Shared;
}

const char* SMOverviewData::toString(SMOverviewData::eThreading threading)
{
    switch (threading)
    {
    case eThreading::Local:
        return SMOverviewData::STR_THREADING_LOCAL;
    case eThreading::Shared:
    default:
        return SMOverviewData::STR_THREADING_SHARED;
    }
}

SMOverviewData::SMOverviewData(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ( )
    , mVersion      (1, 0, 0)
    , mThreading    (eThreading::Shared)
    , mDescription  ( )
{
}

SMOverviewData::SMOverviewData(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mName         (name)
    , mVersion      (1, 0, 0)
    , mThreading    (eThreading::Shared)
    , mDescription  ( )
{
}

bool SMOverviewData::isValid(void) const
{
    return (mName.isEmpty() == false);
}

bool SMOverviewData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementOverview)
        return false;

    QXmlStreamAttributes attributes = xml.attributes();
    setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
    mName = attributes.value(XmlSM::xmlSMAttributeName).toString();
    mVersion = VersionNumber(attributes.value(XmlSM::xmlSMAttributeVersion).toString());
    mThreading = fromThreadingString(attributes.value(XmlSM::xmlSMAttributeThreading).toString());
    mDescription.clear();

    while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == XmlSM::xmlSMElementOverview))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == XmlSM::xmlSMElementDescription)
        {
            mDescription = xml.readElementText();
        }

        xml.readNext();
    }

    return true;
}

void SMOverviewData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementOverview);
    xml.writeAttribute(XmlSM::xmlSMAttributeID, QString::number(getId()));
    xml.writeAttribute(XmlSM::xmlSMAttributeName, mName);
    xml.writeAttribute(XmlSM::xmlSMAttributeVersion, mVersion.toString());
    xml.writeAttribute(XmlSM::xmlSMAttributeThreading, SMOverviewData::toString(mThreading));
    writeTextElem(xml, XmlSM::xmlSMElementDescription, mDescription, true);
    xml.writeEndElement();
}
