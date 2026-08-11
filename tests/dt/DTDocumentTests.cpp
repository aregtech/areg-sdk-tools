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
 *  \file        tests/dt/DTDocumentTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Data Type document (.dtml) tests: the file format, the name the document
 *               declares, and every rule the validation engine files.
 *
 *  Self-contained (no external test framework), matching SICommandTests.cpp.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/dt/DataTypeDocumentData.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"
#include "lusan/model/dt/DTValidator.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
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

    QString serialize(const DataTypeDocumentData& doc)
    {
        QString out;
        QXmlStreamWriter writer(&out);
        writer.setAutoFormatting(true);
        doc.writeToXml(writer);
        return out;
    }

    bool readInto(DataTypeDocumentData& doc, const QString& xml)
    {
        QXmlStreamReader reader(xml);
        while (reader.readNextStartElement())
        {
            return doc.readFromXml(reader);
        }

        return false;
    }

    int countRule(const QList<DocIssue>& issues, int rule)
    {
        int count = 0;
        for (const DocIssue& issue : issues)
        {
            count += (issue.rule == rule ? 1 : 0);
        }

        return count;
    }

    bool namesIt(const QList<DocIssue>& issues, int rule, const QString& fragment)
    {
        for (const DocIssue& issue : issues)
        {
            if ((issue.rule == rule) && issue.message.contains(fragment, Qt::CaseInsensitive))
                return true;
        }

        return false;
    }

    bool explains(const QList<DocIssue>& issues, int rule, DocRuleChecks::eShape shape)
    {
        const QString expected = DocRuleChecks::explainShape(shape);
        for (const DocIssue& issue : issues)
        {
            if ((issue.rule == rule) && (issue.detail == expected))
                return true;
        }

        return false;
    }

    //!< A document that passes everything, so a test can add exactly one fault.
    void makeUsable(DataTypeDocumentData& doc)
    {
        doc.getOverviewData().setName(QStringLiteral("Common"));
        doc.getOverviewData().setVersion(1, 0, 0);
        DataTypeStructure* point = doc.getDataTypeData().addStructure(QStringLiteral("Point"));
        if (point != nullptr)
        {
            point->addField(QStringLiteral("x"))->setType(QStringLiteral("uint32"));
        }
    }

    const int ADVISORY = DTValidator::ADVISORY_RULE_BASE;
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// The document element and the three sections it carries
//////////////////////////////////////////////////////////////////////////

void testDocumentShape()
{
    std::printf("[dt] document shape\n");

    DataTypeDocumentData doc;
    makeUsable(doc);
    doc.getOverviewData().setDescription(QStringLiteral("Types shared between the sensor interfaces."));
    doc.getIncludeData().createInclude(QStringLiteral("my/legacy_types.hpp"));

    const QString xml = serialize(doc);
    CHECK(xml.contains(QStringLiteral("<DataTypeDocument")));
    CHECK(xml.contains(QStringLiteral("FormatVersion=\"1.0.0\"")));
    CHECK(xml.contains(QStringLiteral("<Overview")));
    CHECK(xml.contains(QStringLiteral("<DataTypeList")));
    CHECK(xml.contains(QStringLiteral("<IncludeList")));

    // The three sections a data type document does not have must not reach the file.
    CHECK(xml.contains(QStringLiteral("<AttributeList")) == false);
    CHECK(xml.contains(QStringLiteral("<MethodList")) == false);
    CHECK(xml.contains(QStringLiteral("<ConstantList")) == false);
    // Neither of the two rows the other documents carry on their overview.
    CHECK(xml.contains(QStringLiteral("Category=")) == false);
    CHECK(xml.contains(QStringLiteral("Threading=")) == false);

    // Read back gives the same document, and writing it again gives the same bytes.
    DataTypeDocumentData reread;
    CHECK(readInto(reread, xml));
    CHECK(reread.getOverviewData().getName() == QStringLiteral("Common"));
    CHECK(reread.getOverviewData().getDescription().contains(QStringLiteral("sensor")));
    CHECK(reread.getDataTypeData().getCustomDataTypes().size() == 1);
    CHECK(reread.getIncludeData().getElements().size() == 1);
    CHECK(serialize(reread) == xml);

    // An empty description is left out rather than written as an empty element.
    DataTypeDocumentData bare;
    bare.getOverviewData().setName(QStringLiteral("Bare"));
    CHECK(serialize(bare).contains(QStringLiteral("<Description")) == false);
}

//////////////////////////////////////////////////////////////////////////
// The document keeps the name it declares, whatever the file is called
//////////////////////////////////////////////////////////////////////////

void testNameSurvivesTheFile()
{
    std::printf("[dt] the declared name outlives the file name\n");

    QTemporaryDir dir;
    CHECK(dir.isValid());
    if (dir.isValid() == false)
        return;

    const QString first = dir.filePath(QStringLiteral("Common.dtml"));
    const QString second = dir.filePath(QStringLiteral("Shared.dtml"));

    DataTypeDocumentData doc;
    doc.getOverviewData().setName(QStringLiteral("WhateverWasTypedHere"));
    doc.getDataTypeData().addEnum(QStringLiteral("Unit"));
    CHECK(doc.writeToFile(first));
    // The name belongs to the author, so writing it into another file leaves it alone.
    CHECK(doc.getOverviewData().getName() == QStringLiteral("WhateverWasTypedHere"));

    DataTypeDocumentData opened(first);
    CHECK(opened.openSucceeded());
    CHECK(opened.getOverviewData().getName() == QStringLiteral("WhateverWasTypedHere"));
    CHECK(opened.getDataTypeData().getCustomDataTypes().size() == 1);

    // Saving under another name does not rename the document either.
    CHECK(opened.writeToFile(second));
    CHECK(opened.getOverviewData().getName() == QStringLiteral("WhateverWasTypedHere"));

    DataTypeDocumentData reopened(second);
    CHECK(reopened.openSucceeded());
    CHECK(reopened.getOverviewData().getName() == QStringLiteral("WhateverWasTypedHere"));

    // A document that declares no name of its own falls back to the file, spelled the way C++
    // can carry it.
    const QString odd = dir.filePath(QStringLiteral("123 Odd Name.dtml"));
    QFile oddFile(odd);
    CHECK(oddFile.open(QIODevice::WriteOnly | QIODevice::Text));
    oddFile.write("<?xml version=\"1.0\"?>\n<DataTypeDocument FormatVersion=\"1.0.0\"/>\n");
    oddFile.close();

    DataTypeDocumentData unnamed(odd);
    CHECK(unnamed.getOverviewData().getName() == QStringLiteral("NNNOddName"));

    // A file that is not a data type document is refused rather than half read.
    const QString wrong = dir.filePath(QStringLiteral("NotOurs.dtml"));
    QFile file(wrong);
    CHECK(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("<?xml version=\"1.0\"?>\n<ServiceInterface FormatVersion=\"1.1.0\"/>\n");
    file.close();
    DataTypeDocumentData refused(wrong);
    CHECK(refused.openSucceeded() == false);
}

//////////////////////////////////////////////////////////////////////////
// Validation
//////////////////////////////////////////////////////////////////////////

void testValidatorClean()
{
    std::printf("[dt] a clean document says nothing\n");

    DataTypeDocumentData doc;
    makeUsable(doc);
    doc.getIncludeData().createInclude(QStringLiteral("my/legacy_types.hpp"));
    CHECK(DTValidator::validate(doc).isEmpty());
}

void testValidatorDuplicateEnumValue()
{
    std::printf("[dt] two enumerators counting the same\n");

    {   // enum Numbers { one = 1, two = 1 }
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeEnum* numbers = doc.getDataTypeData().addEnum(QStringLiteral("Numbers"));
        CHECK(numbers != nullptr);
        numbers->addField(QStringLiteral("one"))->setValue(QStringLiteral("1"));
        numbers->addField(QStringLiteral("two"))->setValue(QStringLiteral("1"));

        const QList<DocIssue> issues = DTValidator::validate(doc);
        CHECK(countRule(issues, DTValidator::RULE_DUPLICATE_ENUM_VALUE) == 1);
        CHECK(namesIt(issues, DTValidator::RULE_DUPLICATE_ENUM_VALUE, QStringLiteral("two")));
        CHECK(explains(issues, DTValidator::RULE_DUPLICATE_ENUM_VALUE, DocRuleChecks::eShape::DuplicateEnumValue));

        // Give the second one a value of its own and the finding goes.
        numbers->getElements()[1].setValue(QStringLiteral("2"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_DUPLICATE_ENUM_VALUE) == 0);
    }

    {   // The counting follows C++: enum { one, two = 0 } collides just the same, because an
        // enumerator with no value of its own counts on from the one before it.
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeEnum* implicit = doc.getDataTypeData().addEnum(QStringLiteral("Implicit"));
        CHECK(implicit != nullptr);
        implicit->addField(QStringLiteral("one"));
        implicit->addField(QStringLiteral("two"))->setValue(QStringLiteral("0"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_DUPLICATE_ENUM_VALUE) == 1);
    }

    {   // ... and the plain ascending case is silent, however it is written.
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeEnum* ok = doc.getDataTypeData().addEnum(QStringLiteral("Fine"));
        CHECK(ok != nullptr);
        ok->addField(QStringLiteral("a"));
        ok->addField(QStringLiteral("b"));
        ok->addField(QStringLiteral("c"))->setValue(QStringLiteral("10"));
        ok->addField(QStringLiteral("d"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_DUPLICATE_ENUM_VALUE) == 0);
    }

    {   // 0x10 and 16 are one value, written two ways.
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeEnum* hex = doc.getDataTypeData().addEnum(QStringLiteral("Hex"));
        CHECK(hex != nullptr);
        hex->addField(QStringLiteral("low"))->setValue(QStringLiteral("0x10"));
        hex->addField(QStringLiteral("high"))->setValue(QStringLiteral("16"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_DUPLICATE_ENUM_VALUE) == 1);
    }

    {   // A value that is not a plain number cannot be counted, so it is left out rather than
        // guessed at -- and so is everything that would count on from it.
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeEnum* named = doc.getDataTypeData().addEnum(QStringLiteral("Named"));
        CHECK(named != nullptr);
        named->addField(QStringLiteral("base"))->setValue(QStringLiteral("SomeConstant"));
        named->addField(QStringLiteral("next"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_DUPLICATE_ENUM_VALUE) == 0);
    }
}

void testValidatorDeprecation()
{
    std::printf("[dt] deprecation is said out loud\n");

    {   // A deprecated document is a warning.
        DataTypeDocumentData doc;
        makeUsable(doc);
        doc.getOverviewData().setIsDeprecated(true);
        doc.getOverviewData().setDeprecateHint(QStringLiteral("Use Shared.dtml"));

        const QList<DocIssue> issues = DTValidator::validate(doc);
        const int deprecated = ADVISORY + DTValidator::RULE_DEPRECATED;
        CHECK(countRule(issues, deprecated) == 1);
        CHECK(namesIt(issues, deprecated, QStringLiteral("Use Shared.dtml")));
        CHECK(explains(issues, deprecated, DocRuleChecks::eShape::Deprecated));
        for (const DocIssue& issue : issues)
        {
            if (issue.rule == deprecated)
            {
                CHECK(issue.severity == DocIssue::eSeverity::Warning);
                CHECK(issue.kind == eDocElementKind::Overview);
            }
        }
    }

    {   // A deprecated type is a note, not a warning: the document is still sound.
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeEnum* old = doc.getDataTypeData().addEnum(QStringLiteral("OldUnit"));
        CHECK(old != nullptr);
        old->addField(QStringLiteral("Celsius"));
        old->setIsDeprecated(true);

        const QList<DocIssue> issues = DTValidator::validate(doc);
        const int deprecated = ADVISORY + DTValidator::RULE_DEPRECATED;
        CHECK(countRule(issues, deprecated) == 1);
        CHECK(namesIt(issues, deprecated, QStringLiteral("OldUnit")));
        for (const DocIssue& issue : issues)
        {
            if (issue.rule == deprecated)
            {
                CHECK(issue.severity == DocIssue::eSeverity::Info);
                CHECK(issue.kind == eDocElementKind::DataType);
            }
        }
    }
}

void testValidatorTypes()
{
    std::printf("[dt] declared types have to exist\n");

    {   // A structure field naming a type nothing answers to.
        DataTypeDocumentData doc;
        makeUsable(doc);
        DataTypeStructure* reading = doc.getDataTypeData().addStructure(QStringLiteral("Reading"));
        CHECK(reading != nullptr);
        reading->addField(QStringLiteral("unit"))->setType(QStringLiteral("Unit"));

        const QList<DocIssue> issues = DTValidator::validate(doc);
        CHECK(countRule(issues, DTValidator::RULE_UNRESOLVED_TYPE) == 1);
        CHECK(namesIt(issues, DTValidator::RULE_UNRESOLVED_TYPE, QStringLiteral("Unit")));
        CHECK(explains(issues, DTValidator::RULE_UNRESOLVED_TYPE, DocRuleChecks::eShape::UnresolvedType));

        // Declare it here and the field resolves.
        doc.getDataTypeData().addEnum(QStringLiteral("Unit"))->addField(QStringLiteral("Celsius"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_UNRESOLVED_TYPE) == 0);
    }

    {   // Two declarations of one name, and a name generated code could not carry.
        DataTypeDocumentData doc;
        makeUsable(doc);
        doc.getDataTypeData().addEnum(QStringLiteral("Point"))->addField(QStringLiteral("a"));
        const QList<DocIssue> issues = DTValidator::validate(doc);
        CHECK(countRule(issues, DTValidator::RULE_DUPLICATE_NAME) == 1);
        CHECK(explains(issues, DTValidator::RULE_DUPLICATE_NAME, DocRuleChecks::eShape::DuplicateName));
    }

    {   // A structure with no fields generates an empty declaration.
        DataTypeDocumentData doc;
        makeUsable(doc);
        CHECK(doc.getDataTypeData().addStructure(QStringLiteral("Hollow")) != nullptr);
        CHECK(countRule(DTValidator::validate(doc), ADVISORY + DTValidator::RULE_EMPTY_TYPE) == 1);
    }
}

void testValidatorDocumentName()
{
    std::printf("[dt] the document name becomes a namespace\n");

    {   // A file name C++ cannot carry is an error the author fixes by renaming the file.
        DataTypeDocumentData doc;
        makeUsable(doc);
        doc.getOverviewData().setName(QStringLiteral("my-types"));
        const QList<DocIssue> issues = DTValidator::validate(doc);
        CHECK(countRule(issues, DTValidator::RULE_INVALID_IDENTIFIER) == 1);
        CHECK(namesIt(issues, DTValidator::RULE_INVALID_IDENTIFIER, QStringLiteral("namespace")));
    }

    {   // A document that declares nothing is worth a note, not a complaint.
        DataTypeDocumentData doc;
        doc.getOverviewData().setName(QStringLiteral("Empty"));
        doc.getOverviewData().setVersion(1, 0, 0);
        const QList<DocIssue> issues = DTValidator::validate(doc);
        CHECK(countRule(issues, ADVISORY + DTValidator::RULE_EMPTY_DOCUMENT) == 1);
    }
}

void testValidatorIncludes()
{
    std::printf("[dt] a data type document is a leaf\n");

    {   // A header is what belongs here.
        DataTypeDocumentData doc;
        makeUsable(doc);
        doc.getIncludeData().createInclude(QStringLiteral("my/legacy_types.hpp"));
        CHECK(countRule(DTValidator::validate(doc), DTValidator::RULE_NOT_A_HEADER) == 0);
    }

    {   // Another data type document is not.
        DataTypeDocumentData doc;
        makeUsable(doc);
        doc.getIncludeData().createInclude(QStringLiteral("../shared/Base.dtml"));
        const QList<DocIssue> issues = DTValidator::validate(doc);
        CHECK(countRule(issues, DTValidator::RULE_NOT_A_HEADER) == 1);
        CHECK(namesIt(issues, DTValidator::RULE_NOT_A_HEADER, QStringLiteral("Base.dtml")));
    }

    {   // The page refuses to register one file twice, so the only way a document holds a
        // duplicate is a file written elsewhere that already carries one.
        DataTypeDocumentData doc;
        makeUsable(doc);
        CHECK(doc.getIncludeData().createInclude(QStringLiteral("my/legacy_types.hpp")) != nullptr);
        CHECK(doc.getIncludeData().createInclude(QStringLiteral("my/legacy_types.hpp")) == nullptr);

        DataTypeDocumentData fromFile;
        CHECK(readInto(fromFile, QStringLiteral(
            "<DataTypeDocument FormatVersion=\"1.0.0\">"
            "  <Overview ID=\"50\" Name=\"Common\" Version=\"1.0.0\"/>"
            "  <IncludeList>"
            "    <Location ID=\"60\" Name=\"my/legacy_types.hpp\"/>"
            "    <Location ID=\"61\" Name=\"my/legacy_types.hpp\"/>"
            "  </IncludeList>"
            "</DataTypeDocument>")));
        CHECK(fromFile.getIncludeData().getElements().size() == 2);
        CHECK(countRule(DTValidator::validate(fromFile), ADVISORY + DTValidator::RULE_DUPLICATE_NAME) == 1);
    }
}

void testFieldOfRule()
{
    std::printf("[dt] a finding says which field to fix\n");

    CHECK(DTValidator::fieldOfRule(DTValidator::RULE_INVALID_IDENTIFIER) == eIssueField::Name);
    CHECK(DTValidator::fieldOfRule(DTValidator::RULE_DUPLICATE_NAME) == eIssueField::Name);
    CHECK(DTValidator::fieldOfRule(DTValidator::RULE_UNRESOLVED_TYPE) == eIssueField::Type);
    CHECK(DTValidator::fieldOfRule(DTValidator::RULE_BAD_LITERAL) == eIssueField::Value);
    // The one the new rule adds: the enumerator's value is what has to change.
    CHECK(DTValidator::fieldOfRule(DTValidator::RULE_DUPLICATE_ENUM_VALUE) == eIssueField::Value);
    CHECK(DTValidator::fieldOfRule(ADVISORY + DTValidator::RULE_DEPRECATED) == eIssueField::None);
}

//////////////////////////////////////////////////////////////////////////
// The document that ships beside the spec
//////////////////////////////////////////////////////////////////////////

void testShippedFixture()
{
    std::printf("[dt] the reference document\n");

#ifdef LUSAN_TEST_DATA_DIR
    const QString path = QStringLiteral(LUSAN_TEST_DATA_DIR) + QStringLiteral("/SharedTypes.dtml");
    if (QFile::exists(path) == false)
    {
        std::printf("  (skipped: %s not found)\n", qPrintable(path));
        return;
    }

    DataTypeDocumentData doc(path);
    CHECK(doc.openSucceeded());
    CHECK(doc.getOverviewData().getName() == QStringLiteral("SharedTypes"));

    // Every declared type came through, with the shape it was written in.
    const QList<DataTypeCustom*>& types = doc.getDataTypeData().getCustomDataTypes();
    CHECK(types.size() == 3);
    const DataTypeEnum* unit = static_cast<const DataTypeEnum*>(doc.getDataTypeData().findCustomDataType(QStringLiteral("Unit")));
    CHECK(unit != nullptr);
    CHECK((unit != nullptr) && unit->isEnumeration());
    CHECK((unit != nullptr) && (unit->getElements().size() == 3));

    const DataTypeStructure* reading = static_cast<const DataTypeStructure*>(doc.getDataTypeData().findCustomDataType(QStringLiteral("Reading")));
    CHECK(reading != nullptr);
    CHECK((reading != nullptr) && reading->isStructure());
    CHECK((reading != nullptr) && (reading->getElements().size() == 2));

    const DataTypeContainer* list = static_cast<const DataTypeContainer*>(doc.getDataTypeData().findCustomDataType(QStringLiteral("ReadingList")));
    CHECK(list != nullptr);
    CHECK((list != nullptr) && list->isContainer());
    CHECK((list != nullptr) && (list->getValue() == QStringLiteral("Reading")));

    CHECK(doc.getIncludeData().getElements().size() == 1);

    // The document the editor ships as an example has to be a clean one.
    doc.getDataTypeData().validate(doc.getDataTypeData());
    const QList<DocIssue> issues = DTValidator::validate(doc);
    for (const DocIssue& issue : issues)
    {
        std::printf("  [unexpected] rule %d: %s\n", issue.rule, qPrintable(issue.message));
    }

    CHECK(issues.isEmpty());
#else
    std::printf("  (skipped: no data directory configured)\n");
#endif
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("Data Type document tests\n");
    testDocumentShape();
    testNameSurvivesTheFile();
    testValidatorClean();
    testValidatorDuplicateEnumValue();
    testValidatorDeprecation();
    testValidatorTypes();
    testValidatorDocumentName();
    testValidatorIncludes();
    testFieldOfRule();
    testShippedFixture();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
