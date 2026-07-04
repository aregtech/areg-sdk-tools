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
 *  \file        lusan/data/sm/StateMachineData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSML document root.
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/common/XmlSM.hpp"

#include <QByteArray>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    const VersionNumber& currentFormatVersion(void)
    {
        static const VersionNumber kCurrent(StateMachineData::XML_FORMAT_DEFAULT);
        return kCurrent;
    }

    int getSectionIndex(const QStringView& name)
    {
        if (name == XmlSM::xmlSMElementOverview)         return 0;
        if (name == XmlSM::xmlSMElementDataTypeList)     return 1;
        if (name == XmlSM::xmlSMElementAttributeList)    return 2;
        if (name == XmlSM::xmlSMElementEventList)        return 3;
        if (name == XmlSM::xmlSMElementTimerList)        return 4;
        if (name == XmlSM::xmlSMElementMethodList)       return 5;
        if (name == XmlSM::xmlSMElementConstantList)     return 6;
        if (name == XmlSM::xmlSMElementIncludeList)      return 7;
        if (name == XmlSM::xmlSMElementImportList)       return 8;
        if (name == XmlSM::xmlSMElementStateList)        return 9;
        if (name == XmlSM::xmlSMElementLayout)           return 10;
        return -1;
    }

    void writeStartElementWithAttributes(QXmlStreamWriter& xml, const QXmlStreamReader& src)
    {
        xml.writeStartElement(src.name().toString());
        const QXmlStreamAttributes attrs = src.attributes();
        for (const QXmlStreamAttribute& attr : attrs)
        {
            xml.writeAttribute(attr.name().toString(), attr.value().toString());
        }
    }

    QString captureCurrentElementXml(QXmlStreamReader& xml)
    {
        QByteArray buffer;
        QXmlStreamWriter writer(&buffer);
        writeStartElementWithAttributes(writer, xml);

        int depth = 1;
        while ((depth > 0) && (xml.atEnd() == false) && (xml.hasError() == false))
        {
            const QXmlStreamReader::TokenType token = xml.readNext();
            switch (token)
            {
            case QXmlStreamReader::StartElement:
                writeStartElementWithAttributes(writer, xml);
                ++depth;
                break;

            case QXmlStreamReader::EndElement:
                writer.writeEndElement();
                --depth;
                break;

            case QXmlStreamReader::Characters:
                if (xml.isCDATA())
                {
                    writer.writeCDATA(xml.text().toString());
                }
                else
                {
                    writer.writeCharacters(xml.text().toString());
                }
                break;

            case QXmlStreamReader::Comment:
                writer.writeComment(xml.text().toString());
                break;

            case QXmlStreamReader::ProcessingInstruction:
                writer.writeProcessingInstruction(xml.processingInstructionTarget().toString(), xml.processingInstructionData().toString());
                break;

            case QXmlStreamReader::EntityReference:
                writer.writeEntityReference(xml.name().toString());
                break;

            default:
                break;
            }
        }

        return QString::fromUtf8(buffer);
    }

    void writeRawElementXml(QXmlStreamWriter& xml, const QString& rawElement)
    {
        QXmlStreamReader reader(rawElement);
        while ((reader.atEnd() == false) && (reader.hasError() == false))
        {
            const QXmlStreamReader::TokenType token = reader.readNext();
            switch (token)
            {
            case QXmlStreamReader::StartElement:
                writeStartElementWithAttributes(xml, reader);
                break;

            case QXmlStreamReader::EndElement:
                xml.writeEndElement();
                break;

            case QXmlStreamReader::Characters:
                if (reader.isCDATA())
                {
                    xml.writeCDATA(reader.text().toString());
                }
                else
                {
                    xml.writeCharacters(reader.text().toString());
                }
                break;

            case QXmlStreamReader::Comment:
                xml.writeComment(reader.text().toString());
                break;

            case QXmlStreamReader::ProcessingInstruction:
                xml.writeProcessingInstruction(reader.processingInstructionTarget().toString(), reader.processingInstructionData().toString());
                break;

            case QXmlStreamReader::EntityReference:
                xml.writeEntityReference(reader.name().toString());
                break;

            default:
                break;
            }
        }
    }
}

StateMachineData::StateMachineData(void)
    : ElementBase       (MINIMUM_ID, nullptr)
    , mFilePath         ( )
    , mFormatVersion    (XML_FORMAT_DEFAULT)
    , mOverview         (this)
    , mDataTypes        (this)
    , mAttributes       (this)
    , mEvents           (this)
    , mTimers           (this)
    , mMethods          (this)
    , mConstants        (this)
    , mIncludes         (this)
    , mImports          (this)
    , mStates           (this)
    , mLayout           (this)
    , mOpenSuccess      (false)
{
    // A fresh document gets a valid Overview ID from the document-wide counter, mirroring ServiceInterfaceData.
    mOverview.setId(getNextId());
}

bool StateMachineData::readFromFile(const QString& filePath)
{
    mOpenSuccess = false;
    mFilePath.clear();

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        mFilePath = filePath;

        QXmlStreamReader xml(&file);
        while (!xml.atEnd() && !xml.hasError())
        {
            if (xml.readNextStartElement())
            {
                if (readFromXml(xml) == false)
                {
                    if (xml.hasError() == false)
                    {
                        xml.raiseError("Invalid FSML format");
                    }
                }
            }
        }

        file.close();
        mOpenSuccess = (xml.hasError() == false);
    }

    return mOpenSuccess;
}

bool StateMachineData::writeToFile(const QString& filePath /*= QString()*/)
{
    const QString path = filePath.isEmpty() ? mFilePath : filePath;
    if (path.isEmpty())
        return false;

    QByteArray buffer;
    {
        QXmlStreamWriter xml(&buffer);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(4);
        xml.writeStartDocument("1.0", true);
        writeToXml(xml);
    }
    buffer.append('\n');

    QFile file(path);
    if (file.open(QFile::WriteOnly) == false)
        return false;

    const bool written = (file.write(buffer) == buffer.size());
    file.close();
    if (written == false)
        return false;

    if (filePath.isEmpty() == false)
    {
        mFilePath = filePath;
    }

    return true;
}

bool StateMachineData::readFromXml(QXmlStreamReader& xml)
{
    if (xml.name() != XmlSM::xmlSMElementStateMachine)
        return false;

    clearUnknownContent();
    mFormatVersion = currentFormatVersion();

    const QXmlStreamAttributes attributes = xml.attributes();
    const QString formatVersion = xml.attributes().value(XmlSM::xmlSMAttributeFormatVersion).toString();
    if (formatVersion.isEmpty() == false)
    {
        const VersionNumber parsedVersion(formatVersion);
        if (parsedVersion.isValid() == false)
        {
            xml.raiseError(QString("Invalid FormatVersion '%1'").arg(formatVersion));
            return false;
        }

        mFormatVersion = parsedVersion;
    }

    for (const QXmlStreamAttribute& attr : attributes)
    {
        if (attr.name() != XmlSM::xmlSMAttributeFormatVersion)
        {
            mUnknownRootAttributes.push_back({ attr.name().toString(), attr.value().toString() });
        }
    }

    const VersionNumber& current = currentFormatVersion();
    if (mFormatVersion.getMajor() > current.getMajor())
    {
        xml.raiseError(QString("Unsupported FSML format version %1; supported version is %2")
                           .arg(mFormatVersion.toString(), current.toString()));
        return false;
    }

    int unknownBucket = 0;
    while (xml.readNextStartElement())
    {
        const QStringView name = xml.name();
        const int sectionIndex = getSectionIndex(name);
        if (sectionIndex == 0)
        {
            mOverview.readFromXml(xml);
            unknownBucket = 1;
        }
        else if (sectionIndex == 1)
        {
            mDataTypes.readFromXml(xml);
            unknownBucket = 2;
        }
        else if (sectionIndex == 2)
        {
            mAttributes.readFromXml(xml);
            unknownBucket = 3;
        }
        else if (sectionIndex == 3)
        {
            mEvents.readFromXml(xml);
            unknownBucket = 4;
        }
        else if (sectionIndex == 4)
        {
            mTimers.readFromXml(xml);
            unknownBucket = 5;
        }
        else if (sectionIndex == 5)
        {
            mMethods.readFromXml(xml);
            unknownBucket = 6;
        }
        else if (sectionIndex == 6)
        {
            mConstants.readFromXml(xml);
            unknownBucket = 7;
        }
        else if (sectionIndex == 7)
        {
            mIncludes.readFromXml(xml);
            unknownBucket = 8;
        }
        else if (sectionIndex == 8)
        {
            mImports.readFromXml(xml);
            unknownBucket = 9;
        }
        else if (sectionIndex == 9)
        {
            mStates.readFromXml(xml);
            unknownBucket = 10;
        }
        else if (sectionIndex == 10)
        {
            mLayout.readFromXml(xml);
            unknownBucket = 11;
        }
        else
        {
            mUnknownRootElements.push_back({ unknownBucket, captureCurrentElementXml(xml) });
        }
    }

    if (xml.hasError())
    {
        return false;
    }

    if (mFormatVersion < current)
    {
        if (migrateFromVersion(mFormatVersion) == false)
        {
            xml.raiseError(QString("Cannot migrate FSML format version %1 to %2")
                               .arg(mFormatVersion.toString(), current.toString()));
            return false;
        }
    }

    return true;
}

void StateMachineData::writeToXml(QXmlStreamWriter& xml) const
{
    auto writeUnknownBucket = [this, &xml](int bucket) {
        for (const UnknownElement& unknown : mUnknownRootElements)
        {
            if (unknown.bucket == bucket)
            {
                writeRawElementXml(xml, unknown.xml);
            }
        }
    };

    xml.writeStartElement(XmlSM::xmlSMElementStateMachine);
    xml.writeAttribute(XmlSM::xmlSMAttributeFormatVersion, mFormatVersion.toString());
    for (const UnknownAttribute& attr : mUnknownRootAttributes)
    {
        xml.writeAttribute(attr.name, attr.value);
    }

    writeUnknownBucket(0);
    mOverview.writeToXml(xml);
    writeUnknownBucket(1);
    mDataTypes.writeToXml(xml);
    writeUnknownBucket(2);
    mAttributes.writeToXml(xml);
    writeUnknownBucket(3);
    mEvents.writeToXml(xml);
    writeUnknownBucket(4);
    mTimers.writeToXml(xml);
    writeUnknownBucket(5);
    mMethods.writeToXml(xml);
    writeUnknownBucket(6);
    mConstants.writeToXml(xml);
    writeUnknownBucket(7);
    mIncludes.writeToXml(xml);
    writeUnknownBucket(8);
    mImports.writeToXml(xml);
    writeUnknownBucket(9);
    mStates.writeToXml(xml);
    writeUnknownBucket(10);
    mLayout.writeToXml(xml);
    writeUnknownBucket(11);

    xml.writeEndElement();
}

bool StateMachineData::migrateFromVersion(const VersionNumber& sourceVersion)
{
    VersionNumber working(sourceVersion);
    if (working < VersionNumber(XML_FORMAT_100))
    {
        if (migrateTo100(working) == false)
        {
            return false;
        }

        working = VersionNumber(XML_FORMAT_100);
    }

    mFormatVersion = VersionNumber(XML_FORMAT_DEFAULT);
    return true;
}

bool StateMachineData::migrateTo100(const VersionNumber& sourceVersion)
{
    (void) sourceVersion;
    return true;
}

void StateMachineData::clearUnknownContent(void)
{
    mUnknownRootAttributes.clear();
    mUnknownRootElements.clear();
}

StateMachineData::StimulusRef StateMachineData::findStimulus(const QString& name) const
{
    StimulusRef result;

    if (SMMethodEntry* trigger = mMethods.findTrigger(name))
    {
        result.type    = eStimulusType::Trigger;
        result.element = trigger;
    }
    else if (SMEventEntry* event = mEvents.findEvent(name))
    {
        result.type    = eStimulusType::Event;
        result.element = event;
    }
    else if (SMTimerEntry* timer = mTimers.findElement(name))
    {
        result.type    = eStimulusType::Timer;
        result.element = timer;
    }

    return result;
}

bool StateMachineData::isStimulusName(const QString& name) const
{
    return (findStimulus(name).type != eStimulusType::None);
}

SMStateEntry* StateMachineData::findState(const QString& name) const
{
    return mStates.findStateRecursive(name);
}

int StateMachineData::getStateCount(void) const
{
    return mStates.countStatesRecursive();
}
