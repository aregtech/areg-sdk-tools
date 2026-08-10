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
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/si/SICommand.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/si/SIMethodBase.hpp"
#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/model/si/SIValidator.hpp"

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

        SIAttributeData& attrs = doc.getAttributeData();
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

        SIAttributeData& attrs = doc.getAttributeData();
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

        SIAttributeData& attrs = doc.getAttributeData();
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

            SIMethodBase* request = doc.getMethodData().addMethod(QStringLiteral("fetch"), SIMethodBase::eMethodType::MethodRequest);
            CHECK(request != nullptr);
            MethodParameter* count = request->addParam(QStringLiteral("count"));
            CHECK(count != nullptr);
            count->setType(QStringLiteral("uint32"));
            count->setValue(QStringLiteral("MaxItems"));
            CHECK(countRule(SIValidator::validate(doc), unreferenced) == 0);
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
            ServiceInterfaceData doc;
            makeUsable(doc);
            doc.getAttributeData().createAttribute(QStringLiteral("speed"))->setType(QStringLiteral("uint32"));
            doc.getAttributeData().createAttribute(QStringLiteral("speed"))->setType(QStringLiteral("uint32"));

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
    testLegacyTypeNames();
    testTypeReferenceRefresh();
    testValidatorUnreferenced();
    testValidatorDeclarations();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
