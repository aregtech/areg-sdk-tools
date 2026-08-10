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
#include "lusan/common/DocElementTable.hpp"

#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
    const VersionNumber& currentFormatVersion()
    {
        static const VersionNumber kCurrent(StateMachineData::XML_FORMAT_DEFAULT);
        return kCurrent;
    }

    //!< Binds transitions whose `To` still holds a state name to the matching state id. Runs after
    //!< the whole tree is read, because a target may be declared later in the file.
    void resolvePendingTargets(const StateMachineData& doc, SMStateData& level)
    {
        for (SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            for (SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if ((transition == nullptr) || transition->getPendingTargetName().isEmpty())
                {
                    continue;
                }

                const SMStateEntry* target = doc.findState(transition->getPendingTargetName());
                transition->resolvePendingTarget(target != nullptr ? target->getId() : 0u);
            }

            if (state->hasNestedStates())
            {
                resolvePendingTargets(doc, *state->getNestedStates());
            }
        }
    }

    //!< The default geometry of a pseudo-state's node, matching what the designer draws for the
    //!< Start marker, and the gap it is placed above its target by.
    constexpr double PSEUDO_NODE_WIDTH  { 64.0 };
    constexpr double PSEUDO_NODE_HEIGHT { 32.0 };
    constexpr double PSEUDO_NODE_GAP    { 48.0 };

    //!< A state name no state in the document uses yet: `Start`, then `Start1`, `Start2` and so on.
    //!< Names are unique document-wide, not only within a level, so the search is document-wide.
    QString uniqueStartName(const StateMachineData& doc)
    {
        const QString base{ QString::fromLatin1(SMStateEntry::STR_KIND_START) };
        if (doc.findState(base) == nullptr)
        {
            return base;
        }

        for (int suffix = 1; ; ++suffix)
        {
            const QString candidate = base + QString::number(suffix);
            if (doc.findState(candidate) == nullptr)
            {
                return candidate;
            }
        }
    }

    //!< Gives the new pseudo-state a node above its target, so the level reads top-down. With no
    //!< node for the target the canvas auto-places it like any other node without geometry.
    void placePseudoNode(StateMachineData& doc, uint32_t pseudoId, uint32_t targetId)
    {
        const SMLayoutNode* target = doc.getLayout().findNode(targetId);
        if (target == nullptr)
        {
            return;
        }

        SMLayoutNode& node = doc.getLayout().addNode(pseudoId);
        node.x      = target->x;
        node.y      = target->y - (PSEUDO_NODE_HEIGHT + PSEUDO_NODE_GAP);
        node.width  = PSEUDO_NODE_WIDTH;
        node.height = PSEUDO_NODE_HEIGHT;
    }

    //!< Splits a legacy merged Start state into a pseudo-state and a Normal state, wired by one
    //!< unguarded initial transition. Nothing is dropped, so the next save writes the new form.
    void convertLegacyStartStates(StateMachineData& doc, SMStateData& level)
    {
        // The list is mutated while walking it, so take the states first.
        const QList<SMStateEntry*> states = level.getElements();
        for (SMStateEntry* state : states)
        {
            if (state == nullptr)
            {
                continue;
            }

            if (state->hasNestedStates())
            {
                convertLegacyStartStates(doc, *state->getNestedStates());
            }

            if (state->isLegacyMergedStart() == false)
            {
                continue;
            }

            SMStateEntry* pseudo = new SMStateEntry(doc.getNextId(), uniqueStartName(doc), SMStateEntry::eStateKind::Start, &level);
            state->setKind(SMStateEntry::eStateKind::Normal);

            // The demoted state is a real state now, so nothing it owns is an initial transition
            // any more. Leaving them Initial would put an initial transition on a Normal state.
            for (SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if ((transition != nullptr) && transition->isInitial())
                {
                    transition->setKind(transition->hasTarget()
                                        ? SMTransitionEntry::eTransitionKind::External
                                        : SMTransitionEntry::eTransitionKind::Internal);
                }
            }

            // One initial transition, no stimulus and no guard: the old state was where the level
            // began unconditionally, and that is what the pseudo-state now says.
            pseudo->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QString()
                                                     , state->getId(), SMTransitionEntry::eTransitionKind::Initial);

            // In front of its target, so document order still opens with the level's entry point.
            const int at = level.findIndex(state->getId());
            if (level.insertElement(at >= 0 ? at : 0, pseudo, false) == false)
            {
                delete pseudo;
                state->setKind(SMStateEntry::eStateKind::Start);
                continue;
            }

            placePseudoNode(doc, pseudo->getId(), state->getId());
        }
    }

    //!< Collects every layout owner ID (each state and its transitions) in a submachine subtree.
    void collectSubtreeOwners(const SMStateData& level, QSet<uint32_t>& owners)
    {
        for (const SMStateEntry* state : level.getElements())
        {
            if (state == nullptr)
            {
                continue;
            }

            owners.insert(state->getId());
            for (const SMTransitionEntry* transition : state->getTransitions().getElements())
            {
                if (transition != nullptr)
                {
                    owners.insert(transition->getId());
                }
            }

            if (state->hasNestedStates())
            {
                collectSubtreeOwners(*state->getNestedStates(), owners);
            }
        }
    }

    //!< Collects the layout that a not-real submachine drops from the saved file: the composite's
    //!< own sublevel id and every nested owner. Keeps the saved Layout section free of orphans.
    void collectDroppedLayout(const SMStateData& level, QSet<uint32_t>& dropOwners, QSet<uint32_t>& dropLevels)
    {
        for (const SMStateEntry* state : level.getElements())
        {
            if ((state == nullptr) || (state->hasNestedStates() == false))
            {
                continue;
            }

            const SMStateData* nested = state->getNestedStates();
            if (nested->hasRealState())
            {
                collectDroppedLayout(*nested, dropOwners, dropLevels);
            }
            else
            {
                dropLevels.insert(state->getId());
                collectSubtreeOwners(*nested, dropOwners);
            }
        }
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

        // An element this build cannot show goes back where it was found. Dropping it would
        // destroy the very document the author has to open elsewhere to recover.
        buffer = DocUnknownScan::restore(buffer, data.getUnknownElements());
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
    , mAttributes       (NEAttribute::StateMachine, this)
    , mEvents           (this)
    , mTimers           (this)
    , mMethods          (this)
    , mConstants        (this)
    , mIncludes         (this)
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
        // The Start marker sits at the top-left of the level. The values mirror the view-layer
        // marker size and auto-placement origin, which the data layer cannot include.
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

        const QByteArray content = file.readAll();
        QXmlStreamReader xml(content);
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
            // The readers build no element they do not recognize, so by the time the model
            // exists the name and the line are gone. The text is read once more to keep both,
            // and to keep the block itself for the save. It runs after the parse because
            // reading the document clears what the last one left behind.
            mUnknownElements = DocUnknownScan::scan(content);

            // A freshly parsed entry only carries its type name; resolve the cached type
            // pointer now so icons/pickers match an interactively edited document.
            mDataTypes.validate(mDataTypes);
            mAttributes.validate(mDataTypes);
            mConstants.validate(mDataTypes.getCustomDataTypes());
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

    // A newer document may use elements this build has never heard of, and an editor that cannot
    // show them cannot save them back safely either. Refusing names both versions, because the
    // way out is to update the tool or to refresh the format description beside it.
    const VersionNumber& current = DocElementTable::maxFormatVersion();
    if (current < mFormatVersion)
    {
        xml.raiseError(tr("This document is written in FSML format %1, and this build reads up to format %2. "
                          "Update Lusan, or refresh the format description delivered with the Areg SDK, then open it again.")
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
            readLegacyImportList(xml);
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

    resolvePendingTargets(*this, mStates);
    // The legacy merged `Kind="Start"` is a content shim, not a version one, so the test is what
    // the states say. It runs after the by-name targets are bound, so a demoted state is reachable.
    convertLegacyStartStates(*this, mStates);

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
    writeUnknownBucket(9);
    mStates.writeToXml(xml);
    writeUnknownBucket(10);
    // A not-real submachine is omitted from the StateList above, so its layout goes too. An orphan
    // node would otherwise linger and a future element could inherit it through id reuse.
    QSet<uint32_t> dropOwners;
    QSet<uint32_t> dropLevels;
    collectDroppedLayout(mStates, dropOwners, dropLevels);
    mLayout.writeToXml(xml, dropOwners, dropLevels);
    writeUnknownBucket(11);

    xml.writeEndElement();
}

void StateMachineData::readLegacyImportList(QXmlStreamReader& xml)
{
    // An older document keeps its machine imports in their own section. They become ordinary
    // include entries with an alias and a pinned version, appended so document order survives.
    while ((xml.atEnd() == false)
           && ((xml.tokenType() != QXmlStreamReader::EndElement) || (xml.name() != XmlSM::xmlSMElementImportList)))
    {
        if ((xml.tokenType() == QXmlStreamReader::StartElement) && (xml.name() == XmlSM::xmlSMElementMachineImport))
        {
            const QXmlStreamAttributes attributes = xml.attributes();
            IncludeEntry entry(&mIncludes);
            entry.setId(attributes.value(XmlSM::xmlSMAttributeID).toUInt());
            entry.setLocation(attributes.value(XmlSM::xmlSMAttributeLocation).toString());
            entry.setAlias(attributes.value(XmlSM::xmlSMAttributeName).toString());
            entry.setVersion(VersionNumber(attributes.value(XmlSM::xmlSMAttributeVersion).toString()));

            while ((xml.atEnd() == false)
                   && ((xml.tokenType() != QXmlStreamReader::EndElement) || (xml.name() != XmlSM::xmlSMElementMachineImport)))
            {
                if ((xml.tokenType() == QXmlStreamReader::StartElement) && (xml.name() == XmlSM::xmlSMElementDescription))
                {
                    entry.setDescription(xml.readElementText());
                }

                xml.readNext();
            }

            mIncludes.addElement(std::move(entry), true);
        }

        xml.readNext();
    }
}

QList<const IncludeEntry*> StateMachineData::machineImports() const
{
    QList<const IncludeEntry*> result;
    for (const IncludeEntry& entry : mIncludes.getElements())
    {
        if (includeKindOf(entry.getLocation(), QStringLiteral("fsml")) == eIncludeKind::Document)
        {
            result.append(&entry);
        }
    }

    return result;
}

const IncludeEntry* StateMachineData::findImportByAlias(const QString& alias) const
{
    if (alias.isEmpty())
    {
        return nullptr;
    }

    for (const IncludeEntry* entry : machineImports())
    {
        if (entry->getAlias() == alias)
        {
            return entry;
        }
    }

    return nullptr;
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

    if (working < VersionNumber(XML_FORMAT_110))
    {
        if (migrateTo110(working) == false)
        {
            return false;
        }

        working = VersionNumber(XML_FORMAT_110);
    }

    mFormatVersion = VersionNumber(XML_FORMAT_DEFAULT);
    return true;
}

bool StateMachineData::migrateTo100(const VersionNumber& sourceVersion)
{
    Q_UNUSED(sourceVersion);
    return true;
}

bool StateMachineData::migrateTo110(const VersionNumber& sourceVersion)
{
    // The <ImportList> fold happens while reading, because the reader is the only place that sees
    // both orders. Nothing is left to transform here; the step keeps the chain readable.
    Q_UNUSED(sourceVersion);
    return true;
}

void StateMachineData::clearUnknownContent()
{
    mUnknownRootAttributes.clear();
    mUnknownRootElements.clear();
    mUnknownElements.clear();
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

SMGuard* StateMachineData::findGuard(const SMGuardRef& ref) const
{
    switch (ref.getOwner())
    {
    case SMGuardRef::eOwner::Transition:
    {
        SMTransitionEntry* transition = findTransitionById(ref.getId());
        return (transition != nullptr) ? &transition->getGuard() : nullptr;
    }

    case SMGuardRef::eOwner::None:
    default:
        return nullptr;
    }
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

const QString& StateMachineData::identifierPattern()
{
    static const QString _pattern{ QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$") };
    return _pattern;
}

bool StateMachineData::isValidIdentifier(const QString& name)
{
    static const QRegularExpression _identifier{ StateMachineData::identifierPattern() };
    return (name.size() <= StateMachineData::MAX_IDENTIFIER_LENGTH) && _identifier.match(name).hasMatch();
}

int StateMachineData::getStateCount() const
{
    return mStates.countStatesRecursive();
}
