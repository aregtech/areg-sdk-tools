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
 *  \file        tests/dt/DTImportTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       A service interface reading types out of an included data type document: how a
 *               qualified name resolves, what shadows what, and what the engine reports.
 *
 *  Self-contained (no external test framework), matching DTDocumentTests.cpp.
 *
 ************************************************************************/

#include "lusan/model/common/DocRules.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/dt/DTDocumentCache.hpp"
#include "lusan/data/dt/DataTypeImportResolver.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/model/si/SIValidator.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <QTextStream>
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

    #define CHECK(expr)     check((expr), #expr)

    //!< A data type document declaring an enumeration, a structure and a container.
    const char* const SHARED_TYPES =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<DataTypeDocument FormatVersion=\"1.0.0\">\n"
        "    <Overview ID=\"50\" Name=\"Shared\" Version=\"1.0.0\" IsDeprecated=\"false\"/>\n"
        "    <DataTypeList>\n"
        "        <DataType ID=\"51\" Name=\"Unit\" Type=\"Enumeration\" Values=\"uint16\">\n"
        "            <FieldList>\n"
        "                <EnumEntry ID=\"52\" Name=\"Celsius\" Value=\"0\"/>\n"
        "                <EnumEntry ID=\"53\" Name=\"Kelvin\" Value=\"1\"/>\n"
        "            </FieldList>\n"
        "        </DataType>\n"
        "        <DataType ID=\"54\" Name=\"Reading\" Type=\"Structure\">\n"
        "            <FieldList>\n"
        "                <Field ID=\"55\" Name=\"value\" DataType=\"uint32\"/>\n"
        "            </FieldList>\n"
        "        </DataType>\n"
        "    </DataTypeList>\n"
        "</DataTypeDocument>\n";

    bool writeFile(const QString& path, const QString& content)
    {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) == false)
        {
            return false;
        }

        QTextStream stream(&file);
        stream << content;
        file.close();
        return true;
    }

    //!< A service interface whose attribute declares with \p attributeType, optionally including
    //!< the data type documents named in \p includes and declaring the types in \p ownTypes.
    QString interfaceXml(const QString& attributeType, const QStringList& includes
                        , const QString& ownTypes = QString())
    {
        QString xml =
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<ServiceInterface FormatVersion=\"1.1.0\">\n"
                           "    <Overview ID=\"1\" Name=\"Sensor\" Version=\"1.0.0\" Category=\"Public\"/>\n");
        if (ownTypes.isEmpty() == false)
        {
            xml += QStringLiteral("    <DataTypeList>\n") + ownTypes + QStringLiteral("    </DataTypeList>\n");
        }

        xml += QStringLiteral("    <AttributeList>\n"
                              "        <Attribute ID=\"10\" Name=\"unit\" DataType=\"%1\" Notify=\"OnChange\"/>\n"
                              "    </AttributeList>\n").arg(attributeType);

        if (includes.isEmpty() == false)
        {
            xml += QStringLiteral("    <IncludeList>\n");
            uint32_t id = 20u;
            for (const QString& location : includes)
            {
                xml += QStringLiteral("        <Location ID=\"%1\" Name=\"%2\"/>\n").arg(id++).arg(location);
            }

            xml += QStringLiteral("    </IncludeList>\n");
        }

        xml += QStringLiteral("</ServiceInterface>\n");
        return xml;
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

    //!< Every run starts from an empty cache: the fixtures reuse file names across temporary
    //!< directories, and a leftover entry would answer for the wrong one.
    void resetCache()
    {
        DTDocumentCache::getInstance().clear();
    }
}

//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////

//!< A qualified name reaches the included document's type; a bare one never does.
static void testQualifiedResolution()
{
    std::printf("[qualified resolution]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    CHECK(writeFile(dir.filePath(QStringLiteral("Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("Shared::Unit"), { QStringLiteral("./Shared.dtml") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));

    const DataTypeDataSection& types = data.getDataTypeData();
    CHECK(types.getImports().size() == 1);
    CHECK((types.getImports().size() == 1) && types.getImports().first().isResolved());
    CHECK((types.getImports().size() == 1) && (types.getImports().first().space == QStringLiteral("Shared")));
    CHECK(types.getImportedTypes().size() == 2);
    CHECK(types.hasImportSpace(QStringLiteral("Shared")));
    CHECK(types.hasImportSpace(QStringLiteral("Other")) == false);

    // The qualified spelling answers, the bare one does not: a bare name is the host's own.
    CHECK(types.findCustomDataType(QStringLiteral("Shared::Unit")) != nullptr);
    CHECK(types.findCustomDataType(QStringLiteral("Shared::Reading")) != nullptr);
    CHECK(types.findCustomDataType(QStringLiteral("Unit")) == nullptr);
    CHECK(types.findDataType(QStringLiteral("Shared::Unit")) != nullptr);

    // The resolution scope carries both groups; the document's own list carries only its own.
    CHECK(types.getCustomDataTypes().isEmpty());
    CHECK(types.getResolutionTypes().size() == 2);

    // The attribute holds the resolved type object, not just the name it was given.
    const AttributeEntry* attribute = data.getAttributeData().findElement(10u);
    CHECK(attribute != nullptr);
    CHECK((attribute != nullptr) && (attribute->getParamType() != nullptr));

    const QList<DocIssue> issues = SIValidator::validate(data);
    CHECK(countRule(issues, DocRules::RULE_UNRESOLVED_TYPE) == 0);
    CHECK(countRule(issues, DocRules::RULE_BROKEN_IMPORT) == 0);
    CHECK(countRule(issues, DocRuleChecks::ADVISORY_RULE_BASE + DocRules::RULE_UNREFERENCED) == 0);
}

//!< A qualified name whose namespace is imported but whose type is not declared there is a fault;
//!< one whose namespace is nothing this document includes is a C++ name and is left alone.
static void testUnresolvedQualifiedName()
{
    std::printf("[unresolved qualified name]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    CHECK(writeFile(dir.filePath(QStringLiteral("Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("Shared::Missing"), { QStringLiteral("./Shared.dtml") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));
    CHECK(countRule(SIValidator::validate(data), DocRules::RULE_UNRESOLVED_TYPE) == 1);

    // The same spelling with a namespace nothing includes: a hand-written C++ name, not a finding.
    resetCache();
    QTemporaryDir plain;
    CHECK(plain.isValid());
    const QString bare = plain.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(bare, interfaceXml(QStringLiteral("std::string"), QStringList())));

    ServiceInterfaceData other;
    CHECK(other.readFromFile(bare));
    CHECK(countRule(SIValidator::validate(other), DocRules::RULE_UNRESOLVED_TYPE) == 0);
}

//!< A type the host declares itself wins over an imported one of the same name.
static void testHostTypeShadowsImport()
{
    std::printf("[host type shadows the import]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    CHECK(writeFile(dir.filePath(QStringLiteral("Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));

    const QString ownUnit =
        QStringLiteral("        <DataType ID=\"5\" Name=\"Unit\" Type=\"Enumeration\" Values=\"uint16\">\n"
                       "            <FieldList>\n"
                       "                <EnumEntry ID=\"6\" Name=\"Local\" Value=\"0\"/>\n"
                       "            </FieldList>\n"
                       "        </DataType>\n");

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("Unit"), { QStringLiteral("./Shared.dtml") }, ownUnit)));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));

    const DataTypeDataSection& types = data.getDataTypeData();
    const DataTypeCustom* bare = types.findCustomDataType(QStringLiteral("Unit"));
    CHECK(bare != nullptr);
    CHECK((bare != nullptr) && (bare->getId() == 5u));
    CHECK((bare != nullptr) && (bare->isDocumentImport() == false));

    // The imported one is still reachable, qualified, and is a different object.
    const DataTypeCustom* imported = types.findCustomDataType(QStringLiteral("Shared::Unit"));
    CHECK(imported != nullptr);
    CHECK((imported != nullptr) && (imported != bare));
    CHECK((imported != nullptr) && imported->isDocumentImport());
    CHECK((imported != nullptr) && (imported->getQualifiedName() == QStringLiteral("Shared::Unit")));
}

//!< Two included documents of one base name would generate one namespace twice.
static void testDuplicateNamespace()
{
    std::printf("[duplicate namespace]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    CHECK(QDir(dir.path()).mkpath(QStringLiteral("nested")));
    CHECK(writeFile(dir.filePath(QStringLiteral("Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));
    CHECK(writeFile(dir.filePath(QStringLiteral("nested/Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("Shared::Unit")
                                      , { QStringLiteral("./Shared.dtml"), QStringLiteral("./nested/Shared.dtml") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));

    const QList<DataTypeDataSection::ImportedTypes>& imports = data.getDataTypeData().getImports();
    CHECK(imports.size() == 2);
    CHECK((imports.size() == 2) && imports.at(0).isResolved());
    CHECK((imports.size() == 2) && (imports.at(1).state == DataTypeDataSection::eImportState::DuplicateSpace));

    // Only the first one contributes, so the namespace still holds exactly its two types.
    CHECK(data.getDataTypeData().getImportedTypes().size() == 2);

    // The finding lands on the second row, which is the one the author can drop.
    const QList<DocIssue> issues = SIValidator::validate(data);
    bool blamedSecond = false;
    for (const DocIssue& issue : issues)
    {
        blamedSecond = blamedSecond
                    || ((issue.rule == DocRules::RULE_DUPLICATE_NAME) && (issue.elementId == 21u));
    }

    CHECK(blamedSecond);
}

//!< An include that leads nowhere, and one that leads to a file that is not a data type document.
static void testBrokenImport()
{
    std::printf("[broken import]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    CHECK(writeFile(dir.filePath(QStringLiteral("NotADocument.dtml")), QStringLiteral("this is not xml at all")));

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("uint32")
                                      , { QStringLiteral("./Gone.dtml"), QStringLiteral("./NotADocument.dtml") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));

    const QList<DataTypeDataSection::ImportedTypes>& imports = data.getDataTypeData().getImports();
    CHECK(imports.size() == 2);
    CHECK((imports.size() == 2) && (imports.at(0).state == DataTypeDataSection::eImportState::NotFound));
    CHECK((imports.size() == 2) && (imports.at(1).state == DataTypeDataSection::eImportState::ParseFailed));
    CHECK(data.getDataTypeData().getImportedTypes().isEmpty());

    // Two errors, one per row, and no unused-import advisory on top: a row that contributes
    // nothing because it is broken is already being reported.
    const QList<DocIssue> issues = SIValidator::validate(data);
    CHECK(countRule(issues, DocRules::RULE_BROKEN_IMPORT) == 2);
    CHECK(countRule(issues, DocRuleChecks::ADVISORY_RULE_BASE + DocRules::RULE_UNREFERENCED) == 0);
}

//!< A document included but never declared with is worth a word; one used inside a template is not.
static void testUnusedImport()
{
    std::printf("[unused import]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    CHECK(writeFile(dir.filePath(QStringLiteral("Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("uint32"), { QStringLiteral("./Shared.dtml") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));
    const QList<DocIssue> issues = SIValidator::validate(data);
    CHECK(countRule(issues, DocRuleChecks::ADVISORY_RULE_BASE + DocRules::RULE_UNREFERENCED) == 1);

    // Declared with inside a container's element type, which is where the name is not the whole
    // spelling: still a use.
    resetCache();
    QTemporaryDir other;
    CHECK(other.isValid());
    CHECK(writeFile(other.filePath(QStringLiteral("Shared.dtml")), QString::fromLatin1(SHARED_TYPES)));

    const QString ownList =
        QStringLiteral("        <DataType ID=\"5\" Name=\"Readings\" Type=\"Container\">\n"
                       "            <Container>Array</Container>\n"
                       "            <BaseTypeValue>Shared::Reading</BaseTypeValue>\n"
                       "        </DataType>\n");

    const QString nested = other.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(nested, interfaceXml(QStringLiteral("Readings"), { QStringLiteral("./Shared.dtml") }, ownList)));

    ServiceInterfaceData used;
    CHECK(used.readFromFile(nested));
    const QList<DocIssue> nestedIssues = SIValidator::validate(used);
    CHECK(countRule(nestedIssues, DocRuleChecks::ADVISORY_RULE_BASE + DocRules::RULE_UNREFERENCED) == 0);
    CHECK(countRule(nestedIssues, DocRules::RULE_UNRESOLVED_TYPE) == 0);
}

//!< The included file changing on disk reaches the reading document on the next resolve.
static void testChangedDocumentIsReread()
{
    std::printf("[changed document is re-read]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    const QString shared = dir.filePath(QStringLiteral("Shared.dtml"));
    CHECK(writeFile(shared, QString::fromLatin1(SHARED_TYPES)));

    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("Shared::Unit"), { QStringLiteral("./Shared.dtml") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));
    CHECK(data.getDataTypeData().findCustomDataType(QStringLiteral("Shared::Pressure")) == nullptr);

    // A third type is added to the included document, the way saving it in another window would.
    QString grown = QString::fromLatin1(SHARED_TYPES);
    grown.replace(QStringLiteral("    </DataTypeList>")
                 , QStringLiteral("        <DataType ID=\"60\" Name=\"Pressure\" Type=\"Structure\">\n"
                                  "            <FieldList>\n"
                                  "                <Field ID=\"61\" Name=\"pascal\" DataType=\"uint32\"/>\n"
                                  "            </FieldList>\n"
                                  "        </DataType>\n"
                                  "    </DataTypeList>"));
    CHECK(writeFile(shared, grown));

    // The cache is keyed on the file timestamp, which a test can outrun, so the entry is dropped
    // the way the file watch drops it.
    DTDocumentCache::getInstance().invalidate(QDir(dir.path()).absoluteFilePath(QStringLiteral("Shared.dtml")));

    CHECK(DataTypeImportResolver::refresh(data.getDataTypeData(), host, data.getIncludeData()));
    CHECK(data.getDataTypeData().getImportedTypes().size() == 3);
    CHECK(data.getDataTypeData().findCustomDataType(QStringLiteral("Shared::Pressure")) != nullptr);

    // Nothing changed since, so the next rebuild reports no change and leaves every reader alone.
    CHECK(DataTypeImportResolver::refresh(data.getDataTypeData(), host, data.getIncludeData()) == false);
}

//!< Where a location is measured from, and what gets stored for a file the author picked.
static void testPathResolution()
{
    std::printf("[path resolution]\n");

    QTemporaryDir dir;
    CHECK(dir.isValid());
    const QString host     = dir.filePath(QStringLiteral("Sensor.siml"));
    const QString expected = QDir::cleanPath(dir.filePath(QStringLiteral("Shared.dtml")));

    CHECK(DataTypeImportResolver::absolutePath(host, QStringLiteral("./Shared.dtml")) == expected);
    CHECK(DataTypeImportResolver::absolutePath(host, expected) == expected);

    // An unsaved host has no directory, so only an absolute location can be measured.
    CHECK(DataTypeImportResolver::absolutePath(QString(), QStringLiteral("./Shared.dtml")).isEmpty());
    CHECK(DataTypeImportResolver::absolutePath(QString(), expected) == expected);

    // With no workspace root configured there is nothing to measure against, so the file keeps
    // its absolute path rather than being spelled against the host document.
    NELusanCommon::setSearchRoots(QStringList());
    CHECK(DataTypeImportResolver::storableLocation(host, expected) == expected);
    CHECK(DataTypeImportResolver::storableLocation(QString(), expected) == expected);
}

//!< A location is anchored at a workspace root, so every host spells one file the same way.
static void testLocationAnchoring()
{
    std::printf("[location anchoring]\n");

    QTemporaryDir dir;
    CHECK(dir.isValid());
    const QString root = QDir::cleanPath(dir.path());
    QDir(root).mkpath(QStringLiteral("src/common"));
    QDir(root).mkpath(QStringLiteral("src/svc"));
    QDir(root).mkpath(QStringLiteral("src2"));

    const QString shared = QDir::cleanPath(root + QStringLiteral("/src/common/Shared.dtml"));
    const QString decoy  = QDir::cleanPath(root + QStringLiteral("/src2/Shared.dtml"));
    CHECK(writeFile(shared, QString::fromLatin1(SHARED_TYPES)));
    CHECK(writeFile(decoy, QString::fromLatin1(SHARED_TYPES)));

    NELusanCommon::setSearchRoots(QStringList{ root });

    // Stored the same way whichever document does the storing, and the folders survive.
    CHECK(NELusanCommon::toStorableLocation(shared) == QStringLiteral("src/common/Shared.dtml"));

    // The root is a string prefix of "src2" as well; a separator boundary is what tells them apart.
    NELusanCommon::setSearchRoots(QStringList{ root + QStringLiteral("/src") });
    CHECK(NELusanCommon::toStorableLocation(decoy) == decoy);
    CHECK(NELusanCommon::toStorableLocation(shared) == QStringLiteral("common/Shared.dtml"));

    // Priority order decides, not the deepest match: the first root that contains the file wins.
    NELusanCommon::setSearchRoots(QStringList{ root, root + QStringLiteral("/src") });
    CHECK(NELusanCommon::toStorableLocation(shared) == QStringLiteral("src/common/Shared.dtml"));

    // A file under no root at all is not portable, and keeps the only path that resolves.
    NELusanCommon::setSearchRoots(QStringList{ root + QStringLiteral("/src2") });
    CHECK(NELusanCommon::toStorableLocation(shared) == shared);

    CHECK(NELusanCommon::relativeToRoots(QString(), QStringList{ root }).isEmpty());

    // Resolution: two hosts in different folders, one location, one file.
    NELusanCommon::setSearchRoots(QStringList{ root });
    const QString hostDeep    = QDir::cleanPath(root + QStringLiteral("/src/svc"));
    const QString hostShallow = root;
    const QString stored      = QStringLiteral("src/common/Shared.dtml");
    CHECK(NELusanCommon::resolveLocation(hostDeep, stored) == shared);
    CHECK(NELusanCommon::resolveLocation(hostShallow, stored) == shared);

    // A document written before locations were anchored still resolves against its own folder.
    CHECK(NELusanCommon::resolveLocation(QDir::cleanPath(root + QStringLiteral("/src/common"))
                                         , QStringLiteral("./Shared.dtml")) == shared);

    NELusanCommon::setSearchRoots(QStringList());
}

//!< A C++ header in the include list is not a data type document and contributes nothing.
static void testHeaderIsNotAnImport()
{
    std::printf("[a header is not an import]\n");
    resetCache();

    QTemporaryDir dir;
    CHECK(dir.isValid());
    const QString host = dir.filePath(QStringLiteral("Sensor.siml"));
    CHECK(writeFile(host, interfaceXml(QStringLiteral("uint32"), { QStringLiteral("areg/base/GEGlobal.h") })));

    ServiceInterfaceData data;
    CHECK(data.readFromFile(host));
    CHECK(data.getDataTypeData().getImports().isEmpty());
    CHECK(countRule(SIValidator::validate(data), DocRules::RULE_BROKEN_IMPORT) == 0);
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char* /*argv*/[])
{
    std::printf("Data Type document import tests\n");
    testQualifiedResolution();
    testUnresolvedQualifiedName();
    testHostTypeShadowsImport();
    testDuplicateNamespace();
    testBrokenImport();
    testUnusedImport();
    testChangedDocumentIsReread();
    testPathResolution();
    testLocationAnchoring();
    testHeaderIsNotAnImport();

    std::printf("Checks: %d, Failures: %d\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
