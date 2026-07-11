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
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    const VersionNumber& currentFormatVersion()
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

    QByteArray buildXmlBuffer(const StateMachineData& data)
    {
        QByteArray buffer;
        {
            QXmlStreamWriter xml(&buffer);
            xml.setAutoFormatting(true);
            xml.setAutoFormattingIndent(4);
            xml.writeStartDocument("1.0", true);
            data.writeToXml(xml);
        }

        buffer.append('\n');
        return buffer;
    }
}

StateMachineData::StateMachineData()
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

std::unique_ptr<StateMachineData> StateMachineData::createNewDocument(const QString& machineName)
{
    std::unique_ptr<StateMachineData> result{ std::make_unique<StateMachineData>() };
    result->mOpenSuccess = true;
    result->mOverview.setName(machineName);

    SMStateEntry* start = result->mStates.createState(QStringLiteral("Start"), SMStateEntry::eStateKind::Start);
    if (start != nullptr)
    {
        // The Start state is a compact marker box placed at the top-left of the level so it
        // reads as the machine's entry point. The geometry mirrors the view-layer marker
        // size (NESMDesign::MarkerStateWidth/Height) and auto-placement origin (64;64); the
        // data layer cannot include the view constants, so the values are spelled out here.
        // No View entry is persisted for a fresh document: the Design page anchors the first
        // view to the top-left content itself (a stored center of 0;0 would otherwise push
        // the single Start state into the middle of the viewport).
        SMLayoutNode& node = result->mLayout.addNode(start->getId());
        node.x      = 64.0;
        node.y      = 64.0;
        node.width  = 64.0;
        node.height = 32.0;
    }

    return result;
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
                        xml.raiseError(tr("Invalid FSML format"));
                    }
                }
            }
        }

        file.close();
        mOpenSuccess = (xml.hasError() == false);
        if (mOpenSuccess)
        {
            // A freshly parsed entry only carries its type name; resolve the cached type
            // pointer now so icons/pickers match an interactively edited document.
            mDataTypes.validate(mDataTypes);
            mAttributes.validate(mDataTypes);
            mConstants.validate(mDataTypes);
        }
    }

    return mOpenSuccess;
}

bool StateMachineData::writeToFile(const QString& filePath /*= QString()*/)
{
    const QString path = filePath.isEmpty() ? mFilePath : filePath;
    return path.isEmpty() ? false : writeToPathAtomic(path, true);
}

bool StateMachineData::writeToAutosaveFile(const QString& autosavePath) const
{
    return autosavePath.isEmpty() ? false : writeToPathAtomicConst(autosavePath);
}

QString StateMachineData::autosavePathForDocument(const QString& documentPath)
{
    return documentPath.isEmpty() ? QString() : documentPath + QStringLiteral(".autosave");
}

bool StateMachineData::hasRecoverableAutosave(const QString& documentPath, QString* autosavePath /*= nullptr*/)
{
    const QString path = autosavePathForDocument(documentPath);
    if (autosavePath != nullptr)
    {
        *autosavePath = path;
    }

    QFileInfo autosaveInfo(path);
    if ((autosaveInfo.exists() == false) || (autosaveInfo.isFile() == false))
    {
        return false;
    }

    QFileInfo documentInfo(documentPath);
    if ((documentInfo.exists() == false) || (documentInfo.isFile() == false))
    {
        return true;
    }

    return autosaveInfo.lastModified() > documentInfo.lastModified();
}

bool StateMachineData::removeAutosave(const QString& documentPath)
{
    const QString path = autosavePathForDocument(documentPath);
    return path.isEmpty() ? true : (QFile::exists(path) == false || QFile::remove(path));
}

bool StateMachineData::writeToPathAtomic(const QString& path, bool updateFilePath)
{
    const QByteArray buffer = buildXmlBuffer(*this);

    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        return false;
    }

    if (file.write(buffer) != buffer.size())
    {
        file.cancelWriting();
        return false;
    }

    if (file.commit() == false)
    {
        return false;
    }

    if (updateFilePath)
    {
        mFilePath = path;
    }

    return true;
}

bool StateMachineData::writeToPathAtomicConst(const QString& path) const
{
    const QByteArray buffer = buildXmlBuffer(*this);

    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        return false;
    }

    if (file.write(buffer) != buffer.size())
    {
        file.cancelWriting();
        return false;
    }

    return file.commit();
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
            xml.raiseError(tr("Invalid FormatVersion \'%1\'").arg(formatVersion));
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
        xml.raiseError(tr("Unsupported FSML format version %1; supported version is %2").arg(mFormatVersion.toString(), current.toString()));
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
            xml.raiseError(QString(tr("Cannot migrate FSML format version %1 to %2"))
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
    Q_UNUSED(sourceVersion);
    return true;
}

void StateMachineData::clearUnknownContent()
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

SMStateEntry* StateMachineData::findStateById(uint32_t id) const
{
    return mStates.findStateByIdRecursive(id);
}

SMStateEntry* StateMachineData::findTransitionOwner(uint32_t transitionId) const
{
    return mStates.findTransitionOwnerRecursive(transitionId);
}

SMTransitionEntry* StateMachineData::findTransitionById(uint32_t transitionId) const
{
    SMStateEntry* owner = mStates.findTransitionOwnerRecursive(transitionId);
    if (owner != nullptr)
    {
        SMTransitionEntry** slot = owner->getTransitions().findElement(transitionId);
        return (slot != nullptr ? *slot : nullptr);
    }

    return nullptr;
}

SMStateData* StateMachineData::findLevel(uint32_t levelId)
{
    if (levelId == mOverview.getId())
    {
        return &mStates;
    }

    SMStateEntry* state = mStates.findStateByIdRecursive(levelId);
    return (state != nullptr ? state->getNestedStates() : nullptr);
}

const SMStateData* StateMachineData::findLevel(uint32_t levelId) const
{
    return const_cast<StateMachineData*>(this)->findLevel(levelId);
}

namespace
{
    //!< Depth-first search for the composite-state chain leading to a level owner.
    bool buildLevelPath(const SMStateData& level, uint32_t levelId, QList<uint32_t>& path)
    {
        for (const SMStateEntry* state : level.getElements())
        {
            const SMStateData* nested = state->getNestedStates();
            if (nested == nullptr)
            {
                continue;
            }

            path.append(state->getId());
            if ((state->getId() == levelId) || buildLevelPath(*nested, levelId, path))
            {
                return true;
            }

            path.removeLast();
        }

        return false;
    }
}

QList<uint32_t> StateMachineData::getLevelPath(uint32_t levelId) const
{
    QList<uint32_t> path{ mOverview.getId() };
    if (levelId == mOverview.getId())
    {
        return path;
    }

    return buildLevelPath(mStates, levelId, path) ? path : QList<uint32_t>{};
}

bool StateMachineData::isValidIdentifier(const QString& name)
{
    static const QRegularExpression _identifier{ QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$") };
    return _identifier.match(name).hasMatch();
}

int StateMachineData::getStateCount() const
{
    return mStates.countStatesRecursive();
}
