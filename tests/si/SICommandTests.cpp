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
 *  \file        tests/si/SICommandTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Undo/redo command framework tests for the Service Interface (.siml) editor.
 *               The Service Interface drives the same shared command framework (model/common)
 *               as the FSM editor. This proves the framework is document-agnostic: identical
 *               invariants over ServiceInterfaceData.
 *
 *  Self-contained (no external test framework), matching SMModelTests.cpp.
 *
 ************************************************************************/

#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/IncludeDataSection.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/si/SICommand.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/common/MethodDataSection.hpp"
#include "lusan/model/si/SIValidator.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"

#include <QUndoStack>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int gChecks = 0;
    int gFailures = 0;

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    QString serialize(const ServiceInterfaceData& doc)
    {
        QString out;
        QXmlStreamWriter writer(&out);
        writer.setAutoFormatting(true);
        doc.writeToXml(writer);
        return out;
    }

    AttributeEntry makeAttribute(const QString& name, ElementBase* parent)
    {
        return AttributeEntry(0u, name, AttributeEntry::eNotification::NotifyOnChange, parent);
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Scenario A: scripted mixed sequence, full undo/redo round-trip per step
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testScriptedSequence()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        int added = 0, removed = 0, changed = 0, reordered = 0;
        QObject::connect(&notifier, &DocModelNotifier::elementAdded,   [&](uint32_t, eDocElementKind){ ++added; });
        QObject::connect(&notifier, &DocModelNotifier::elementRemoved, [&](uint32_t, eDocElementKind){ ++removed; });
        QObject::connect(&notifier, &DocModelNotifier::elementChanged, [&](uint32_t, eDocElementKind){ ++changed; });
        QObject::connect(&notifier, &DocModelNotifier::listReordered,  [&](uint32_t, eDocElementKind){ ++reordered; });

        doc.getOverviewData().getId();
        doc.getAttributeData().getId();
        doc.getConstantData().getId();

        QStringList checkpoints;
        checkpoints << serialize(doc);
        auto pushAndSnap = [&](QUndoCommand* command)
        {
            stack.push(command);
            checkpoints << serialize(doc);
        };

        AttributeDataSection& attrs = doc.getAttributeData();
        pushAndSnap(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attrs, makeAttribute("speed", &attrs), eDocElementKind::Attribute, "Add speed"));
        pushAndSnap(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attrs, makeAttribute("gear",  &attrs), eDocElementKind::Attribute, "Add gear"));

        ConstantDataSection& consts = doc.getConstantData();
        pushAndSnap(new TDocAddCommand<ConstantEntry, DocumentElem>(notifier, consts, ConstantEntry(0u, "MaxSpeed", &consts), eDocElementKind::Constant, "Add MaxSpeed"));

        {
            const uint32_t overviewId = doc.getOverviewData().getId();
            auto getter = [&doc]() -> QString { return doc.getOverviewData().getName(); };
            auto setter = [&doc](const QString& value) { doc.getOverviewData().setName(value); };
            pushAndSnap(new TDocSetPropertyCommand<QString>(notifier, overviewId, eDocElementKind::Overview, getter, setter, QString("Renamed"), "Rename service"));
        }

        pushAndSnap(new TDocReorderCommand<AttributeEntry, DocumentElem>(notifier, attrs, 0, 1, attrs.getId(), eDocElementKind::Attribute, "Reorder attributes"));

        const int steps = static_cast<int>(checkpoints.size()) - 1;
        const QString built = serialize(doc);

        bool undoExact = true;
        for (int k = steps; k >= 1; --k)
        {
            stack.undo();
            undoExact = undoExact && (serialize(doc) == checkpoints[k - 1]);
        }
        CHECK(undoExact);
        CHECK(serialize(doc) == checkpoints[0]);

        bool redoExact = true;
        for (int k = 1; k <= steps; ++k)
        {
            stack.redo();
            redoExact = redoExact && (serialize(doc) == checkpoints[k]);
        }
        CHECK(redoExact);
        CHECK(serialize(doc) == built);

        CHECK(added > 0);
        CHECK(removed > 0);
        CHECK(changed > 0);
        CHECK(reordered > 0);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario B: composite (two removes as one undo step)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testComposite()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        AttributeDataSection& attrs = doc.getAttributeData();
        attrs.createAttribute("alpha");
        attrs.createAttribute("beta");
        // Read IDs from the container: createAttribute returns a pointer into the value
        // list, which the second insertion may reallocate.
        const uint32_t idA = attrs.getElements().at(0).getId();
        const uint32_t idB = attrs.getElements().at(1).getId();

        const QString before = serialize(doc);
        const int indexBefore = stack.index();

        SICompositeCommand* composite = new SICompositeCommand(doc, notifier, "Delete both");
        new TDocRemoveCommand<AttributeEntry, DocumentElem>(notifier, attrs, idA, eDocElementKind::Attribute, "Delete alpha", composite);
        new TDocRemoveCommand<AttributeEntry, DocumentElem>(notifier, attrs, idB, eDocElementKind::Attribute, "Delete beta", composite);
        stack.push(composite);

        CHECK(stack.index() == indexBefore + 1);            // both removals are one step
        CHECK(attrs.getElementCount() == 0);

        stack.undo();
        CHECK(serialize(doc) == before);                    // both restored with original IDs
        CHECK(attrs.getElementCount() == 2);

        stack.redo();
        CHECK(attrs.getElementCount() == 0);
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario C: deep history (100+ steps) and ID uniqueness
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDeepHistory()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        AttributeDataSection& attrs = doc.getAttributeData();
        const int count = 120;
        for (int i = 0; i < count; ++i)
        {
            stack.push(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attrs, makeAttribute(QString("attr%1").arg(i), &attrs), eDocElementKind::Attribute, "Add"));
        }

        CHECK(stack.count() == count);
        const QString full = serialize(doc);

        for (int i = 0; i < count; ++i)
        {
            stack.undo();
        }
        CHECK(attrs.getElementCount() == 0);

        for (int i = 0; i < count; ++i)
        {
            stack.redo();
        }
        CHECK(attrs.getElementCount() == count);
        CHECK(serialize(doc) == full);

        QList<uint32_t> ids;
        for (const AttributeEntry& attr : attrs.getElements())
        {
            ids.append(attr.getId());
        }
        const QSet<uint32_t> unique(ids.begin(), ids.end());
        CHECK(unique.size() == ids.size());
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario D: the data types section, shared with the FSM editor
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDataTypeSection()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        DataTypeDataSection& types = doc.getDataTypeData();
        const QString empty = serialize(doc);

        DataTypeStructure* point = new DataTypeStructure();
        point->setName(QStringLiteral("Point"));
        stack.push(new TDocAddCommand<DataTypeCustom*, DocumentElem>(notifier, types, static_cast<DataTypeCustom*>(point), eDocElementKind::DataType, "Add Point"));
        CHECK(types.getElementCount() == 1);
        CHECK(types.findCustomDataType(QStringLiteral("Point")) == point);

        stack.push(new TDocAddCommand<FieldEntry, DataTypeCustom>(notifier, *point, FieldEntry(0u, QStringLiteral("x"), point), eDocElementKind::DataType, "Add x"));
        stack.push(new TDocAddCommand<FieldEntry, DataTypeCustom>(notifier, *point, FieldEntry(0u, QStringLiteral("y"), point), eDocElementKind::DataType, "Add y"));
        CHECK(point->getElementCount() == 2);

        const QString built = serialize(doc);

        // A field reorder is one step, and it round-trips byte for byte.
        stack.push(new TDocReorderCommand<FieldEntry, DataTypeCustom>(notifier, *point, 0, 1, point->getId(), eDocElementKind::DataType, "Reorder fields"));
        const QString reordered = serialize(doc);
        CHECK(reordered != built);
        stack.undo();
        CHECK(serialize(doc) == built);
        stack.redo();
        CHECK(serialize(doc) == reordered);
        stack.undo();

        // Removing the type keeps it alive in the command, so undo restores it whole.
        stack.push(new TDocRemoveCommand<DataTypeCustom*, DocumentElem>(notifier, types, point->getId(), eDocElementKind::DataType, "Delete Point"));
        CHECK(types.getElementCount() == 0);
        stack.undo();
        CHECK(types.getElementCount() == 1);
        CHECK(serialize(doc) == built);

        // And back to nothing, from the top of the history down.
        while (stack.canUndo())
        {
            stack.undo();
        }
        CHECK(types.getElementCount() == 0);
        CHECK(serialize(doc) == empty);
    }

    //!< The `Overview` section is shared with the state machine, and every edit of it reaches the
    //!< document through the same property command -- which is what gives the interface's Overview
    //!< page an undo history it never had. What the interface adds to the shared rows, the service
    //!< category, is written by the interface's own section and by nothing else.
    void testOverviewSection()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        SIOverviewData& overview = doc.getOverviewData();
        const uint32_t id = overview.getId();
        const QString built = serialize(doc);
        CHECK(built.contains(QStringLiteral("Category=\"Private\"")));
        CHECK(built.contains(QStringLiteral("Threading=")) == false);

        // The description is one undo step, and it round-trips byte for byte.
        stack.push(new TDocSetPropertyCommand<QString>( notifier, id, eDocElementKind::Overview
                                                      , [&overview]() { return overview.getDescription(); }
                                                      , [&overview](const QString& value) { overview.setDescription(value); }
                                                      , QStringLiteral("What the interface is for."), "Set description"));
        const QString described = serialize(doc);
        CHECK(described.contains(QStringLiteral("<Description>What the interface is for.</Description>")));
        stack.undo();
        CHECK(serialize(doc) == built);
        stack.redo();
        CHECK(serialize(doc) == described);

        // So is the version.
        stack.push(new TDocSetPropertyCommand<VersionNumber>( notifier, id, eDocElementKind::Overview
                                                            , [&overview]() { return overview.getVersion(); }
                                                            , [&overview](const VersionNumber& value) { overview.setVersion(value); }
                                                            , VersionNumber(2u, 1u, 0u), "Set version"));
        CHECK(serialize(doc).contains(QStringLiteral("Version=\"2.1.0\"")));
        stack.undo();
        CHECK(serialize(doc) == described);
        stack.redo();

        // And so is the category, which only an interface declares.
        stack.push(new TDocSetPropertyCommand<SIOverviewData::eCategory>( notifier, id, eDocElementKind::Overview
                                                                        , [&overview]() { return overview.getCategory(); }
                                                                        , [&overview](const SIOverviewData::eCategory& value) { overview.setCategory(value); }
                                                                        , SIOverviewData::eCategory::InterfacePublic, "Set service category"));
        CHECK(overview.getCategory() == SIOverviewData::eCategory::InterfacePublic);
        CHECK(serialize(doc).contains(QStringLiteral("Category=\"Public\"")));
        stack.undo();
        CHECK(overview.getCategory() == SIOverviewData::eCategory::InterfacePrivate);
        stack.redo();

        // The deprecation mark and its hint move together, the way the page offers them.
        overview.setIsDeprecated(true);
        overview.setDeprecateHint(QStringLiteral("Use the v2 interface."));
        const QString deprecated = serialize(doc);
        CHECK(deprecated.contains(QStringLiteral("IsDeprecated=\"true\"")));
        CHECK(deprecated.contains(QStringLiteral("<DeprecateHint>Use the v2 interface.</DeprecateHint>")));
        overview.setIsDeprecated(false);
        CHECK(serialize(doc).contains(QStringLiteral("DeprecateHint")) == false);

        // And back to where it started, from the top of the history down.
        while (stack.canUndo())
        {
            stack.undo();
        }

        CHECK(serialize(doc) == built);
    }

    //!< A published `.siml` keeps its overview across a read and a write, including the category
    //!< the first format spelled as `isRemote`.
    void testOverviewRoundTrip()
    {
        const QString source = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<ServiceInterface FormatVersion=\"1.1.0\">"
            "  <Overview ID=\"1\" Name=\"Sample\" Version=\"1.2.3\" Category=\"Public\" IsDeprecated=\"true\">"
            "    <DeprecateHint>Use the v2 interface.</DeprecateHint>"
            "    <Description>Sample interface.</Description>"
            "  </Overview>"
            "</ServiceInterface>");

        ServiceInterfaceData doc;
        QXmlStreamReader reader(source);
        while (reader.readNextStartElement())
        {
            CHECK(doc.readFromXml(reader));
            break;
        }

        const SIOverviewData& overview = doc.getOverviewData();
        CHECK(overview.getName() == QStringLiteral("Sample"));
        CHECK(overview.getVersion() == VersionNumber(1u, 2u, 3u));
        CHECK(overview.getCategory() == SIOverviewData::eCategory::InterfacePublic);
        CHECK(overview.getIsDeprecated());
        CHECK(overview.getDeprecateHint() == QStringLiteral("Use the v2 interface."));
        CHECK(overview.getDescription() == QStringLiteral("Sample interface."));

        // Written back, it is the overview that was read, and no threading mode appeared.
        const QString written = serialize(doc);
        CHECK(written.contains(QStringLiteral("Name=\"Sample\"")));
        CHECK(written.contains(QStringLiteral("Version=\"1.2.3\"")));
        CHECK(written.contains(QStringLiteral("Category=\"Public\"")));
        CHECK(written.contains(QStringLiteral("IsDeprecated=\"true\"")));
        CHECK(written.contains(QStringLiteral("<DeprecateHint>Use the v2 interface.</DeprecateHint>")));
        CHECK(written.contains(QStringLiteral("Threading=")) == false);

        // And reading what was written gives the same overview again.
        ServiceInterfaceData again;
        QXmlStreamReader back(written);
        while (back.readNextStartElement())
        {
            CHECK(again.readFromXml(back));
            break;
        }

        CHECK(serialize(again) == written);

        // The first published format said `isRemote` instead of naming a category.
        const QString legacy = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<ServiceInterface FormatVersion=\"1.0.0\">"
            "  <Overview ID=\"1\" Name=\"Legacy\" Version=\"1.0.0\" isRemote=\"true\"/>"
            "</ServiceInterface>");

        ServiceInterfaceData old;
        QXmlStreamReader oldReader(legacy);
        while (oldReader.readNextStartElement())
        {
            CHECK(old.readFromXml(oldReader));
            break;
        }

        CHECK(old.getOverviewData().getCategory() == SIOverviewData::eCategory::InterfacePublic);
        CHECK(serialize(old).contains(QStringLiteral("Category=\"Public\"")));
    }

    //!< The `IncludeList` section is shared with the state machine, and every edit of it reaches
    //!< the document through the same commands. The section round-trips byte for byte across an
    //!< undo and a redo of each of them.
    void testIncludeSection()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        IncludeDataSection& includes = doc.getIncludeData();
        const QString empty = serialize(doc);

        stack.push(new TDocAddCommand<IncludeEntry, DocumentElem>(notifier, includes, IncludeEntry(0u, QStringLiteral("common/Global.hpp"), &includes), eDocElementKind::Include, "Add header"));
        stack.push(new TDocAddCommand<IncludeEntry, DocumentElem>(notifier, includes, IncludeEntry(0u, QStringLiteral("shared/Types.dtml"), &includes), eDocElementKind::Include, "Add data types"));
        CHECK(includes.getElementCount() == 2);
        CHECK(includes.findElement(QStringLiteral("common/Global.hpp")) != nullptr);

        // The location is the include's unique name, so the same file cannot be registered twice.
        CHECK(includes.createInclude(QStringLiteral("common/Global.hpp")) == nullptr);

        const QString built = serialize(doc);

        // An insert lands where it was asked to, and takes the whole list back on undo.
        stack.push(buildInsertCommand<IncludeEntry, DocumentElem>(notifier, includes, IncludeEntry(0u, QStringLiteral("common/First.hpp"), &includes), 0, 0u, eDocElementKind::Include, "Insert header"));
        CHECK(includes.getElementCount() == 3);
        CHECK(includes.getElements().at(0).getLocation() == QStringLiteral("common/First.hpp"));
        stack.undo();
        CHECK(serialize(doc) == built);
        stack.redo();
        const QString inserted = serialize(doc);
        stack.undo();

        // A reorder is one step and round-trips.
        stack.push(new TDocReorderCommand<IncludeEntry, DocumentElem>(notifier, includes, 0, 1, 0u, eDocElementKind::Include, "Reorder includes"));
        const QString reordered = serialize(doc);
        CHECK(reordered != built);
        stack.undo();
        CHECK(serialize(doc) == built);
        stack.redo();
        CHECK(serialize(doc) == reordered);
        stack.undo();

        // Removing a row keeps it alive in the command, so undo restores it whole -- description
        // and deprecation included.
        IncludeEntry* header = includes.findElement(QStringLiteral("common/Global.hpp"));
        CHECK(header != nullptr);
        header->setDescription(QStringLiteral("Project-wide declarations."));
        header->deprecateEntry(QStringLiteral("Use Global2.hpp"));
        const QString described = serialize(doc);
        CHECK(described != built);

        const uint32_t headerId = header->getId();
        stack.push(new TDocRemoveCommand<IncludeEntry, DocumentElem>(notifier, includes, headerId, eDocElementKind::Include, "Delete header"));
        CHECK(includes.getElementCount() == 1);
        stack.undo();
        CHECK(includes.getElementCount() == 2);
        CHECK(serialize(doc) == described);
        CHECK(inserted != described);

        // And back to nothing, from the top of the history down. The description and deprecation
        // were set on the entry directly, so the empty section is all that is left to compare.
        while (stack.canUndo())
        {
            stack.undo();
        }

        CHECK(includes.getElementCount() == 0);
        CHECK(serialize(doc) == empty);
    }

    //!< The `AttributeList` section is shared with the state machine, and every edit of it reaches
    //!< the document through the same commands. A service interface attribute carries a
    //!< notification kind and no value, so the section writes a `Notify` and never a `Value`, and
    //!< it round-trips byte for byte across an undo and a redo of each edit.

    //!< The shared method section over a service interface: request, response and broadcast, the
    //!< request-to-response link, per-kind names, parameters with defaults, and the written shape
    //!< -- a `Response` attribute, a `<Value IsDefault>` child, and never `Return`/`Implement`.
    void testMethodSection()
    {
        ServiceInterfaceData doc;
        MethodDataSection& methods = doc.getMethodData();

        MethodEntry* fetch = methods.createMethod(QStringLiteral("fetch"), NEMethod::SiRequest);
        CHECK(fetch != nullptr);
        CHECK(fetch->hasReply());
        CHECK(fetch->hasReturn() == false);
        CHECK(fetch->hasImplement() == false);

        MethodEntry* fetched = methods.createMethod(QStringLiteral("fetched"), NEMethod::SiResponse);
        CHECK(fetched != nullptr);
        CHECK(fetched->isReplyKind());
        CHECK(fetched->hasReply() == false);

        CHECK(methods.createMethod(QStringLiteral("ready"), NEMethod::SiBroadcast) != nullptr);

        // A name belongs to its kind: a request and a broadcast may share one, two requests
        // may not.
        CHECK(methods.createMethod(QStringLiteral("fetch"), NEMethod::SiBroadcast) != nullptr);
        CHECK(methods.createMethod(QStringLiteral("fetch"), NEMethod::SiRequest) == nullptr);

        fetch->setReply(QStringLiteral("fetched"));
        MethodParameter* count = fetch->addParam(QStringLiteral("count"));
        CHECK(count != nullptr);
        count->setType(QStringLiteral("uint32"));
        count->setDefault(true);
        count->setValue(QStringLiteral("10"));

        const QString built = serialize(doc);
        CHECK(built.contains(QStringLiteral("MethodType=\"Request\"")));
        CHECK(built.contains(QStringLiteral("Response=\"fetched\"")));
        CHECK(built.contains(QStringLiteral("IsDefault=\"true\"")));
        CHECK(built.contains(QStringLiteral("Default=\"10\"")) == false);
        CHECK(built.contains(QStringLiteral("Return=")) == false);
        CHECK(built.contains(QStringLiteral("Implement=")) == false);

        // Reading the document back gives the same section, link and default included.
        ServiceInterfaceData reread;
        QXmlStreamReader reader(built);
        CHECK(reader.readNextStartElement());
        CHECK(reread.readFromXml(reader));
        MethodEntry* back = reread.getMethodData().findMethod(QStringLiteral("fetch"), NEMethod::SiRequest);
        CHECK(back != nullptr);
        CHECK((back != nullptr) && (back->getReply() == QStringLiteral("fetched")));
        MethodParameter* backParam = (back != nullptr) ? back->findElement(QStringLiteral("count")) : nullptr;
        CHECK(backParam != nullptr);
        CHECK((backParam != nullptr) && backParam->hasDefault());
        CHECK((backParam != nullptr) && (backParam->getValue() == QStringLiteral("10")));
        CHECK(serialize(reread) == built);
    }

    //!< A parameter marked as a default has to come back as one. The reader compared the
    //!< `IsDefault` attribute and stored the comparison result rather than the answer, so every
    //!< default read back inverted -- a `.siml` round-trip lost the flag it had just written.
    void testMethodParamDefaultRoundTrip()
    {
        const QString xml = QStringLiteral(
            "<ServiceInterface FormatVersion=\"1.1.0\">"
            "<MethodList>"
            "<Method ID=\"7\" Name=\"fetch\" MethodType=\"Request\">"
            "<ParamList>"
            "<Parameter ID=\"8\" Name=\"count\" DataType=\"uint32\"><Value IsDefault=\"true\">10</Value></Parameter>"
            "<Parameter ID=\"9\" Name=\"tag\" DataType=\"String\"><Value IsDefault=\"false\">none</Value></Parameter>"
            "</ParamList>"
            "</Method>"
            "</MethodList>"
            "</ServiceInterface>");

        ServiceInterfaceData doc;
        QXmlStreamReader reader(xml);
        CHECK(reader.readNextStartElement());
        CHECK(doc.readFromXml(reader));

        MethodEntry* fetch = doc.getMethodData().findMethod(QStringLiteral("fetch"), NEMethod::SiRequest);
        CHECK(fetch != nullptr);
        if (fetch == nullptr)
            return;

        MethodParameter* count = fetch->findElement(QStringLiteral("count"));
        MethodParameter* tag = fetch->findElement(QStringLiteral("tag"));
        CHECK((count != nullptr) && count->hasDefault());
        CHECK((tag != nullptr) && (tag->hasDefault() == false));
        CHECK((count != nullptr) && (count->getValue() == QStringLiteral("10")));

        // And it survives being written and read again.
        const QString written = serialize(doc);
        CHECK(written.contains(QStringLiteral("IsDefault=\"true\"")));
        CHECK(written.contains(QStringLiteral("IsDefault=\"false\"")));
    }

    void testAttributeSection()
    {
        ServiceInterfaceData    doc;
        DocModelNotifier        notifier;
        QUndoStack              stack;

        AttributeDataSection& attributes = doc.getAttributeData();
        const QString empty = serialize(doc);

        stack.push(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attributes, makeAttribute(QStringLiteral("IsConnected"), &attributes), eDocElementKind::Attribute, "Add attribute"));
        stack.push(new TDocAddCommand<AttributeEntry, DocumentElem>(notifier, attributes, makeAttribute(QStringLiteral("RetryCount"), &attributes), eDocElementKind::Attribute, "Add attribute"));
        CHECK(attributes.getElementCount() == 2);
        CHECK(attributes.findElement(QStringLiteral("IsConnected")) != nullptr);

        // The name is the attribute's unique name, so it cannot be declared twice.
        CHECK(attributes.createAttribute(QStringLiteral("IsConnected")) == nullptr);

        const QString built = serialize(doc);
        CHECK(built.contains(QStringLiteral("Notify=\"OnChange\"")));
        CHECK(built.contains(QStringLiteral("Value=")) == false);

        // An insert lands where it was asked to, and takes the whole list back on undo.
        stack.push(buildInsertCommand<AttributeEntry, DocumentElem>(notifier, attributes, makeAttribute(QStringLiteral("Uptime"), &attributes), 0, 0u, eDocElementKind::Attribute, "Insert attribute"));
        CHECK(attributes.getElementCount() == 3);
        CHECK(attributes.getElements().at(0).getName() == QStringLiteral("Uptime"));
        stack.undo();
        CHECK(serialize(doc) == built);
        stack.redo();
        const QString inserted = serialize(doc);
        stack.undo();

        // A reorder is one step and round-trips.
        stack.push(new TDocReorderCommand<AttributeEntry, DocumentElem>(notifier, attributes, 0, 1, 0u, eDocElementKind::Attribute, "Reorder attributes"));
        const QString reordered = serialize(doc);
        CHECK(reordered != built);
        stack.undo();
        CHECK(serialize(doc) == built);
        stack.redo();
        CHECK(serialize(doc) == reordered);
        stack.undo();

        // A notification kind is stored and written as the canonical string.
        AttributeEntry* connected = attributes.findElement(QStringLiteral("IsConnected"));
        CHECK(connected != nullptr);
        connected->setNotification(AttributeEntry::eNotification::NotifyAlways);
        connected->setDescription(QStringLiteral("True while the link is up."));
        connected->setIsDeprecated(true);
        connected->setDeprecateHint(QStringLiteral("Use LinkState"));
        const QString described = serialize(doc);
        CHECK(described.contains(QStringLiteral("Notify=\"Always\"")));
        CHECK(described != built);

        // Removing a row keeps it alive in the command, so undo restores it whole -- notification,
        // description and deprecation included.
        const uint32_t connectedId = connected->getId();
        stack.push(new TDocRemoveCommand<AttributeEntry, DocumentElem>(notifier, attributes, connectedId, eDocElementKind::Attribute, "Delete attribute"));
        CHECK(attributes.getElementCount() == 1);
        stack.undo();
        CHECK(attributes.getElementCount() == 2);
        CHECK(serialize(doc) == described);
        CHECK(inserted != described);

        // A value the section does not carry never reaches the file, even when the entry holds one.
        attributes.findElement(QStringLiteral("RetryCount"))->setValue(QStringLiteral("5"));
        CHECK(serialize(doc).contains(QStringLiteral("Value=")) == false);

        // And back to nothing, from the top of the history down.
        while (stack.canUndo())
        {
            stack.undo();
        }

        CHECK(attributes.getElementCount() == 0);
        CHECK(serialize(doc) == empty);
    }

    //!< A published `.siml` keeps its attribute section across a read and a write: same fields,
    //!< same spelling, nothing gained and nothing lost.
    void testAttributeRoundTrip()
    {
        const QString source = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<ServiceInterface FormatVersion=\"1.1.0\">"
            "  <Overview ID=\"1\" Name=\"Sample\" Version=\"1.0.0\" isRemote=\"true\"/>"
            "  <AttributeList>"
            "    <Attribute DataType=\"uint16\" ID=\"15\" Name=\"SomeAttr1\" Notify=\"OnChange\">"
            "      <Description>Notify only when the value changed.</Description>"
            "    </Attribute>"
            "    <Attribute DataType=\"bool\" ID=\"16\" Name=\"SomeAttr2\" Notify=\"Always\"/>"
            "    <Attribute DataType=\"uint16\" ID=\"44\" Name=\"Old\" Notify=\"OnChange\" IsDeprecated=\"true\">"
            "      <Description>Marked as deprecated.</Description>"
            "      <DeprecateHint>Example to deprecate data.</DeprecateHint>"
            "    </Attribute>"
            "  </AttributeList>"
            "</ServiceInterface>");

        ServiceInterfaceData doc;
        QXmlStreamReader reader(source);
        while (reader.readNextStartElement())
        {
            CHECK(doc.readFromXml(reader));
            break;
        }

        const AttributeDataSection& attributes = doc.getAttributeData();
        CHECK(attributes.getElementCount() == 3);

        const AttributeEntry* always = attributes.findElement(QStringLiteral("SomeAttr2"));
        CHECK(always != nullptr);
        CHECK(always->getNotification() == AttributeEntry::eNotification::NotifyAlways);

        const AttributeEntry* old = attributes.findElement(QStringLiteral("Old"));
        CHECK(old != nullptr);
        CHECK(old->getIsDeprecated());
        CHECK(old->getDeprecateHint() == QStringLiteral("Example to deprecate data."));

        // Written back, every attribute is the one that was read, and no `Value` appeared.
        const QString written = serialize(doc);
        CHECK(written.contains(QStringLiteral("Name=\"SomeAttr1\"")));
        CHECK(written.contains(QStringLiteral("Notify=\"Always\"")));
        CHECK(written.contains(QStringLiteral("IsDeprecated=\"true\"")));
        CHECK(written.contains(QStringLiteral("<DeprecateHint>Example to deprecate data.</DeprecateHint>")));
        CHECK(written.contains(QStringLiteral("Value=")) == false);

        // And reading what was written gives the same section again.
        ServiceInterfaceData again;
        QXmlStreamReader back(written);
        while (back.readNextStartElement())
        {
            CHECK(again.readFromXml(back));
            break;
        }

        CHECK(again.getAttributeData().getElementCount() == 3);
        CHECK(serialize(again) == written);
    }

    //!< The first published `.siml` format spelled two categories differently. A document that
    //!< still uses those names must load with the same categories, not be skipped.
    void testLegacyTypeNames()
    {
        const QString source = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<ServiceInterface FormatVersion=\"1.0.0\">"
            "  <Overview ID=\"1\" Name=\"Legacy\" Version=\"1.0.0\" isRemote=\"true\"/>"
            "  <DataTypeList>"
            "    <DataType ID=\"2\" Name=\"SomeEnum\" Type=\"Enumerate\" Values=\"default\">"
            "      <FieldList>"
            "        <EnumEntry ID=\"3\" Name=\"Invalid\"/>"
            "      </FieldList>"
            "    </DataType>"
            "    <DataType ID=\"4\" Name=\"SomeArray\" Type=\"DefinedType\" Values=\"Array\">"
            "      <Container>Array</Container>"
            "      <BaseTypeValue>uint32</BaseTypeValue>"
            "    </DataType>"
            "  </DataTypeList>"
            "</ServiceInterface>");

        ServiceInterfaceData doc;
        QXmlStreamReader reader(source);
        bool read = false;
        while (reader.readNextStartElement())
        {
            read = doc.readFromXml(reader);
            break;
        }

        CHECK(read);
        const DataTypeDataSection& types = doc.getDataTypeData();
        CHECK(types.getElementCount() == 2);

        const DataTypeCustom* someEnum = types.findCustomDataType(QStringLiteral("SomeEnum"));
        CHECK(someEnum != nullptr);
        CHECK((someEnum != nullptr) && (someEnum->getCategory() == DataTypeBase::eCategory::Enumeration));

        const DataTypeCustom* someArray = types.findCustomDataType(QStringLiteral("SomeArray"));
        CHECK(someArray != nullptr);
        CHECK((someArray != nullptr) && (someArray->getCategory() == DataTypeBase::eCategory::Container));
    }

    //!< A structure field and a container key/value keep the type object their type name resolves
    //!< to, and that object is what tells a complete declaration from a broken one. The registry
    //!< re-reads all of them on demand, so a name entered before its type exists starts resolving
    //!< and a name whose type is gone stops.
    void testTypeReferenceRefresh()
    {
        ServiceInterfaceData doc;
        DataTypeDataSection& types = doc.getDataTypeData();

        DataTypeContainer* list = types.addContainer(QStringLiteral("List"));
        CHECK(list != nullptr);
        list->setValue(QStringLiteral("Record"));

        DataTypeStructure* rec = types.addStructure(QStringLiteral("Record"));
        CHECK(rec != nullptr);
        FieldEntry* field = rec->addField(QStringLiteral("id"));
        CHECK(field != nullptr);
        field->setType(QStringLiteral("uint32"));
        const uint32_t fieldId = field->getId();

        // A name alone resolves to nothing until the registry is asked to look it up.
        CHECK(list->getValueDataType() == nullptr);
        types.refreshTypeReferences();
        CHECK(list->getValueDataType() == rec);
        CHECK(rec->findElement(fieldId)->getParamType() != nullptr);

        // The reference reads its name off the type it resolved to, so a rename carries it along
        // and a later refresh looks up the new name, not the one stored when it first resolved.
        rec->setName(QStringLiteral("Entry"));
        CHECK(list->getValue() == QStringLiteral("Entry"));
        types.refreshTypeReferences();
        CHECK(list->getValueDataType() == rec);
        CHECK(list->getValue() == QStringLiteral("Entry"));

        // The default container declares Array<bool>, which is complete: bool is a primitive the
        // registry knows, so the type carries no warning.
        DataTypeContainer* flags = types.addContainer(QStringLiteral("Flags"));
        CHECK(flags != nullptr);
        CHECK(flags->getValue() == QStringLiteral("bool"));
        types.refreshTypeReferences();
        CHECK(flags->getValueDataType() != nullptr);

        // A type that leaves the registry takes its resolved object with it, and leaves the name.
        types.removeElement(rec->getId());
        types.refreshTypeReferences();
        CHECK(list->getValueDataType() == nullptr);
        CHECK(list->getValue() == QStringLiteral("Entry"));
    }
}

//////////////////////////////////////////////////////////////////////////
// Scenario E: the validation engine
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< How many findings carry the given rule id.
    int countRule(const QList<DocIssue>& issues, int rule)
    {
        int count = 0;
        for (const DocIssue& issue : issues)
        {
            count += (issue.rule == rule ? 1 : 0);
        }

        return count;
    }

    //!< True when the rule was reported at all, and every finding of it carries the one
    //!< explanation the shape has.
    bool explains(const QList<DocIssue>& issues, int rule, DocRuleChecks::eShape shape)
    {
        const QString expected = DocRuleChecks::explainShape(shape);
        int found = 0;
        for (const DocIssue& issue : issues)
        {
            if (issue.rule != rule)
                continue;
            if (issue.detail != expected)
                return false;

            ++found;
        }

        return (found > 0);
    }

    //!< True when some finding carries the rule and quotes the text.
    bool namesIt(const QList<DocIssue>& issues, int rule, const QString& text)
    {
        for (const DocIssue& issue : issues)
        {
            if ((issue.rule == rule) && issue.message.contains(text))
                return true;
        }

        return false;
    }

    //!< A document with a name, a version and one attribute, so the overview rules stay quiet
    //!< and each case below reports only what it is about.
    void makeUsable(ServiceInterfaceData& doc)
    {
        doc.getOverviewData().setName(QStringLiteral("TestInterface"));
        doc.getOverviewData().setVersion(1, 0, 0);
        AttributeEntry* attribute = doc.getAttributeData().createAttribute(QStringLiteral("state"));
        if (attribute != nullptr)
        {
            attribute->setType(QStringLiteral("uint32"));
        }
    }

    void testValidatorUnreferenced()
    {
        const int unreferenced = SIValidator::ADVISORY_RULE_BASE + SIValidator::RULE_UNREFERENCED;

        {   // The reported case: a container nothing declares with is a warning, and the container
            // itself is complete, so it must not also be reported as an unresolved type.
            ServiceInterfaceData doc;
            makeUsable(doc);
            DataTypeContainer* list = doc.getDataTypeData().addContainer(QStringLiteral("NewDataType1"));
            CHECK(list != nullptr);

            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, unreferenced) == 1);
            CHECK(namesIt(issues, unreferenced, QStringLiteral("'NewDataType1'")));
            CHECK(countRule(issues, SIValidator::RULE_UNRESOLVED_TYPE) == 0);
        }

        {   // A type an attribute declares with is referenced, and so is a type reached only
            // through another type.
            ServiceInterfaceData doc;
            makeUsable(doc);
            DataTypeStructure* rec = doc.getDataTypeData().addStructure(QStringLiteral("Record"));
            CHECK(rec != nullptr);
            rec->addField(QStringLiteral("id"))->setType(QStringLiteral("uint32"));

            DataTypeContainer* list = doc.getDataTypeData().addContainer(QStringLiteral("Records"));
            CHECK(list != nullptr);
            list->setValue(QStringLiteral("Record"));

            AttributeEntry* attribute = doc.getAttributeData().createAttribute(QStringLiteral("all"));
            CHECK(attribute != nullptr);
            attribute->setType(QStringLiteral("Records"));

            CHECK(countRule(SIValidator::validate(doc), unreferenced) == 0);
        }

        {   // A constant nothing reads is advisory, not a warning, and a constant used as a
            // parameter default is read.
            ServiceInterfaceData doc;
            makeUsable(doc);
            ConstantEntry* limit = doc.getConstantData().createConstant(QStringLiteral("MaxItems"));
            CHECK(limit != nullptr);
            limit->setType(QStringLiteral("uint32"));
            limit->setValue(QStringLiteral("64"));

            QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, unreferenced) == 1);
            for (const DocIssue& issue : issues)
            {
                if (issue.rule == unreferenced)
                {
                    CHECK(issue.severity == DocIssue::eSeverity::Info);
                }
            }

            MethodEntry* request = doc.getMethodData().createMethod(QStringLiteral("fetch"), NEMethod::SiRequest);
            CHECK(request != nullptr);
            MethodParameter* count = request->addParam(QStringLiteral("count"));
            CHECK(count != nullptr);
            count->setType(QStringLiteral("uint32"));
            count->setValue(QStringLiteral("MaxItems"));
            CHECK(countRule(SIValidator::validate(doc), unreferenced) == 0);
        }
    }

    //!< A `.dtml` include contributes a type only when the interface declares with a name this
    //!< document does not answer to. When every declared type resolves here, the import is dead
    //!< weight and is reported.
    void testValidatorUnusedImport()
    {
        const int unusedImport = SIValidator::ADVISORY_RULE_BASE + SIValidator::RULE_UNUSED_IMPORT;

        {   // A header is not a data type document, so it is never reported by this rule.
            ServiceInterfaceData doc;
            makeUsable(doc);
            CHECK(doc.getIncludeData().createInclude(QStringLiteral("common/Global.hpp")) != nullptr);
            CHECK(countRule(SIValidator::validate(doc), unusedImport) == 0);
        }

        {   // Every type the interface uses is a primitive declared nowhere but in the catalog,
            // so nothing can be coming from the imported document.
            ServiceInterfaceData doc;
            makeUsable(doc);
            CHECK(doc.getIncludeData().createInclude(QStringLiteral("shared/Types.dtml")) != nullptr);

            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, unusedImport) == 1);
            CHECK(namesIt(issues, unusedImport, QStringLiteral("'shared/Types.dtml'")));
        }

        {   // A declared type this document does not answer to may be the imported one, so the
            // import is left alone. The unresolved name itself is still reported.
            ServiceInterfaceData doc;
            makeUsable(doc);
            CHECK(doc.getIncludeData().createInclude(QStringLiteral("shared/Types.dtml")) != nullptr);
            AttributeEntry* imported = doc.getAttributeData().createAttribute(QStringLiteral("shape"));
            CHECK(imported != nullptr);
            imported->setType(QStringLiteral("Polygon"));

            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, unusedImport) == 0);
            CHECK(countRule(issues, SIValidator::RULE_UNRESOLVED_TYPE) == 1);
        }
    }

    void testValidatorDeclarations()
    {
        {   // A declared type the registry does not answer to, on an attribute and inside a
            // container, and a name the generated code could not carry.
            ServiceInterfaceData doc;
            makeUsable(doc);
            AttributeEntry* broken = doc.getAttributeData().createAttribute(QStringLiteral("2nd value"));
            CHECK(broken != nullptr);
            broken->setType(QStringLiteral("Missing"));

            DataTypeContainer* list = doc.getDataTypeData().addContainer(QStringLiteral("Items"));
            CHECK(list != nullptr);
            list->setValue(QStringLiteral("Nothing"));

            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, SIValidator::RULE_UNRESOLVED_TYPE) == 2);
            CHECK(namesIt(issues, SIValidator::RULE_UNRESOLVED_TYPE, QStringLiteral("'Missing'")));
            CHECK(namesIt(issues, SIValidator::RULE_UNRESOLVED_TYPE, QStringLiteral("'Nothing'")));
            CHECK(countRule(issues, SIValidator::RULE_INVALID_IDENTIFIER) == 1);
        }

        {   // Two attributes cannot share a name, and a constant's value has to read as its type.
            // The section refuses to declare the same name twice, so the collision is built the
            // way it actually happens: the second attribute is renamed onto the first.
            ServiceInterfaceData doc;
            makeUsable(doc);
            doc.getAttributeData().createAttribute(QStringLiteral("speed"))->setType(QStringLiteral("uint32"));
            AttributeEntry* renamed = doc.getAttributeData().createAttribute(QStringLiteral("velocity"));
            CHECK(renamed != nullptr);
            renamed->setType(QStringLiteral("uint32"));
            renamed->setName(QStringLiteral("speed"));

            ConstantEntry* bad = doc.getConstantData().createConstant(QStringLiteral("Limit"));
            CHECK(bad != nullptr);
            bad->setType(QStringLiteral("uint32"));
            bad->setValue(QStringLiteral("not a number"));

            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, SIValidator::RULE_DUPLICATE_NAME) == 1);
            CHECK(countRule(issues, SIValidator::RULE_BAD_LITERAL) == 1);
        }

        {   // A request answers with a declared response and no other. A stale link is what a
            // document carries after the response was renamed or removed outside the editor, so
            // the case is built the way it reaches us: by reading it.
            const QString source = QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                "<ServiceInterface FormatVersion=\"1.1.0\">"
                "  <Overview ID=\"1\" Name=\"Linked\" Version=\"1.0.0\" isRemote=\"true\"/>"
                "  <MethodList>"
                "    <Method ID=\"2\" Name=\"start\" MethodType=\"request\" Response=\"started\"/>"
                "    <Method ID=\"3\" Name=\"stopped\" MethodType=\"response\"/>"
                "  </MethodList>"
                "</ServiceInterface>");

            ServiceInterfaceData doc;
            QXmlStreamReader reader(source);
            while (reader.readNextStartElement())
            {
                CHECK(doc.readFromXml(reader));
                break;
            }

            const int unbound = SIValidator::ADVISORY_RULE_BASE + SIValidator::RULE_UNBOUND_RESPONSE;
            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, SIValidator::RULE_RESPONSE_LINK) == 1);
            CHECK(namesIt(issues, SIValidator::RULE_RESPONSE_LINK, QStringLiteral("'started'")));
            CHECK(countRule(issues, unbound) == 1);
            CHECK(namesIt(issues, unbound, QStringLiteral("'stopped'")));
        }

        {   // Every finding reaches the results panel with a reason attached.
            ServiceInterfaceData doc;
            makeUsable(doc);
            doc.getDataTypeData().addContainer(QStringLiteral("Unused"));
            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(issues.isEmpty() == false);
            bool allExplained = true;
            for (const DocIssue& issue : issues)
            {
                allExplained = allExplained && (issue.detail.isEmpty() == false);
            }

            CHECK(allExplained);
        }
    }

    //!< The rule shapes both document engines share now come from one place: the same answer,
    //!< the same wording and the same explanation, filed under the number this engine has
    //!< always used. The state machine side asserts the same explanations.
    void testValidatorSharedShapes()
    {
        {   // A declared enumeration has a literal form of its own: its enumerators. Before the
            // shapes were shared the service interface skipped every declared type here.
            ServiceInterfaceData doc;
            makeUsable(doc);
            DataTypeEnum* colors = doc.getDataTypeData().addEnum(QStringLiteral("Color"));
            CHECK(colors != nullptr);
            CHECK(colors->addField(QStringLiteral("Red")) != nullptr);
            CHECK(colors->addField(QStringLiteral("Green")) != nullptr);

            ConstantEntry* wrong = doc.getConstantData().createConstant(QStringLiteral("Accent"));
            CHECK(wrong != nullptr);
            wrong->setType(QStringLiteral("Color"));
            wrong->setValue(QStringLiteral("Blue"));

            QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, SIValidator::RULE_BAD_LITERAL) == 1);
            CHECK(namesIt(issues, SIValidator::RULE_BAD_LITERAL, QStringLiteral("enumerator")));

            // The same constant with a declared enumerator is silent.
            wrong->setValue(QStringLiteral("Green"));
            CHECK(countRule(SIValidator::validate(doc), SIValidator::RULE_BAD_LITERAL) == 0);
        }

        {   // A structure carries no literal at all, and a name longer than a compiler would
            // take is the identifier fault it is.
            ServiceInterfaceData doc;
            makeUsable(doc);
            DataTypeStructure* record = doc.getDataTypeData().addStructure(QStringLiteral("Record"));
            CHECK(record != nullptr);
            record->addField(QStringLiteral("id"))->setType(QStringLiteral("uint32"));

            ConstantEntry* bad = doc.getConstantData().createConstant(QStringLiteral("Seed"));
            CHECK(bad != nullptr);
            bad->setType(QStringLiteral("Record"));
            bad->setValue(QStringLiteral("0"));

            AttributeEntry* longName = doc.getAttributeData().createAttribute(QStringLiteral("counter"));
            CHECK(longName != nullptr);
            longName->setType(QStringLiteral("uint32"));
            longName->setName(QString(NELusanCommon::MAX_IDENTIFIER_LENGTH + 1, QLatin1Char('a')));

            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(countRule(issues, SIValidator::RULE_BAD_LITERAL) == 1);
            CHECK(namesIt(issues, SIValidator::RULE_BAD_LITERAL, QStringLiteral("no literal form")));
            CHECK(countRule(issues, SIValidator::RULE_INVALID_IDENTIFIER) == 1);
        }

        {   // One shape, one explanation: a finding of a shared shape carries the shared text,
            // whichever document it came from, so the results panel cannot describe one defect
            // two ways.
            ServiceInterfaceData doc;
            makeUsable(doc);
            AttributeEntry* unnamed = doc.getAttributeData().createAttribute(QStringLiteral("temporary"));
            CHECK(unnamed != nullptr);
            unnamed->setType(QStringLiteral("Missing"));
            unnamed->setName(QString());

            ConstantEntry* twice = doc.getConstantData().createConstant(QStringLiteral("Limit"));
            CHECK(twice != nullptr);
            twice->setType(QStringLiteral("uint32"));
            twice->setValue(QStringLiteral("nine"));
            ConstantEntry* clash = doc.getConstantData().createConstant(QStringLiteral("Other"));
            CHECK(clash != nullptr);
            clash->setType(QStringLiteral("uint32"));
            clash->setName(QStringLiteral("Limit"));

            const int unreferenced = SIValidator::ADVISORY_RULE_BASE + SIValidator::RULE_UNREFERENCED;
            const QList<DocIssue> issues = SIValidator::validate(doc);
            CHECK(explains(issues, SIValidator::RULE_MISSING_NAME, DocRuleChecks::eShape::MissingName));
            CHECK(explains(issues, SIValidator::RULE_UNRESOLVED_TYPE, DocRuleChecks::eShape::UnresolvedType));
            CHECK(explains(issues, SIValidator::RULE_DUPLICATE_NAME, DocRuleChecks::eShape::DuplicateName));
            CHECK(explains(issues, SIValidator::RULE_BAD_LITERAL, DocRuleChecks::eShape::BadLiteral));
            CHECK(explains(issues, unreferenced, DocRuleChecks::eShape::Unreferenced));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("Service Interface command framework tests\n");
    testScriptedSequence();
    testComposite();
    testDeepHistory();
    testDataTypeSection();
    testOverviewSection();
    testOverviewRoundTrip();
    testIncludeSection();
    testAttributeSection();
    testAttributeRoundTrip();
    testMethodSection();
    testMethodParamDefaultRoundTrip();
    testLegacyTypeNames();
    testTypeReferenceRefresh();
    testValidatorUnreferenced();
    testValidatorUnusedImport();
    testValidatorDeclarations();
    testValidatorSharedShapes();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
