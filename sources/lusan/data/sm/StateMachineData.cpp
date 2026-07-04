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
                    xml.raiseError("Invalid FSML format");
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

    // Serialize into a buffer first so a single trailing newline can be appended and the
    // whole document written in one binary write. Binary mode (no QIODevice::Text) keeps
    // the writer's '\n' newlines as '\n' on every platform, so the output is byte-identical
    // and diff-stable (spec 7.7); QIODevice::Text would translate them to CRLF on Windows.
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

    const QString formatVersion = xml.attributes().value(XmlSM::xmlSMAttributeFormatVersion).toString();
    if (formatVersion.isEmpty() == false)
    {
        mFormatVersion = VersionNumber(formatVersion);
    }

    while (xml.readNextStartElement())
    {
        const QStringView name = xml.name();
        if (name == XmlSM::xmlSMElementOverview)
        {
            mOverview.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementDataTypeList)
        {
            mDataTypes.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementAttributeList)
        {
            mAttributes.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementEventList)
        {
            mEvents.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementTimerList)
        {
            mTimers.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementMethodList)
        {
            mMethods.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementConstantList)
        {
            mConstants.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementIncludeList)
        {
            mIncludes.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementImportList)
        {
            mImports.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementStateList)
        {
            mStates.readFromXml(xml);
        }
        else if (name == XmlSM::xmlSMElementLayout)
        {
            mLayout.readFromXml(xml);
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    return true;
}

void StateMachineData::writeToXml(QXmlStreamWriter& xml) const
{
    xml.writeStartElement(XmlSM::xmlSMElementStateMachine);
    xml.writeAttribute(XmlSM::xmlSMAttributeFormatVersion, mFormatVersion.toString());

    mOverview.writeToXml(xml);
    mDataTypes.writeToXml(xml);
    mAttributes.writeToXml(xml);
    mEvents.writeToXml(xml);
    mTimers.writeToXml(xml);
    mMethods.writeToXml(xml);
    mConstants.writeToXml(xml);
    mIncludes.writeToXml(xml);
    mImports.writeToXml(xml);
    mStates.writeToXml(xml);
    mLayout.writeToXml(xml);

    xml.writeEndElement();
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
