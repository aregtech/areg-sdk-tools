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
 *  \file        tests/common/DocSchemaTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The element table read from a schema: what it makes of one, that the copy
 *               delivered with the SDK is preferred, that the compiled-in copy answers when
 *               there is none, and that an element none of them defines survives a save.
 *
 *  Self-contained (no external test framework), matching DTDocumentTests.cpp.
 *
 ************************************************************************/

#include "lusan/common/DocElementTable.hpp"
#include "lusan/common/DocSchemaReader.hpp"
#include "lusan/data/dt/DataTypeDocumentData.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/model/common/DocUnknownScan.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
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

    using eDocument = DocElementTable::eDocument;

    bool writeFile(const QString& path, const QByteArray& content)
    {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly) == false)
        {
            return false;
        }

        const bool written = (file.write(content) == content.size());
        file.close();
        return written;
    }

    QByteArray readFile(const QString& path)
    {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }
}

//////////////////////////////////////////////////////////////////////////
// The reader
//////////////////////////////////////////////////////////////////////////

// A schema of every shape these formats use -- a named type shared by two elements, a type
// written out in place, and a group pulled into two types -- turned into name and parents.
void testReaderShapes()
{
    std::printf("[reader] element graph of a schema\n");

    const QByteArray schema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "  <xs:complexType name=\"cLeaf\">"
        "    <xs:sequence>"
        "      <xs:element name=\"Description\" type=\"xs:string\" minOccurs=\"0\"/>"
        "    </xs:sequence>"
        "    <xs:attribute name=\"ID\" type=\"xs:unsignedInt\"/>"
        "  </xs:complexType>"
        "  <xs:group name=\"gStep\">"
        "    <xs:choice>"
        "      <xs:element name=\"Start\" type=\"cLeaf\"/>"
        "      <xs:element name=\"Stop\" type=\"cLeaf\"/>"
        "    </xs:choice>"
        "  </xs:group>"
        "  <xs:complexType name=\"cSteps\">"
        "    <xs:sequence>"
        "      <xs:group ref=\"gStep\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "    </xs:sequence>"
        "  </xs:complexType>"
        "  <xs:element name=\"Root\">"
        "    <xs:complexType>"
        "      <xs:all>"
        "        <xs:element name=\"EntryList\" type=\"cSteps\"/>"
        "        <xs:element name=\"ExitList\" type=\"cSteps\"/>"
        "      </xs:all>"
        "    </xs:complexType>"
        "  </xs:element>"
        "</xs:schema>";

    const QList<DocSchemaReader::Element> elements = DocSchemaReader::read(schema);
    CHECK(elements.size() == 6);

    QMap<QString, QStringList> byName;
    for (const DocSchemaReader::Element& element : elements)
    {
        byName.insert(element.name, element.parents);
    }

    // The document element is the one with nowhere to sit.
    CHECK(byName.value(QStringLiteral("Root")).isEmpty());

    // A type written out in place belongs to the element that writes it.
    CHECK(byName.value(QStringLiteral("EntryList")) == QStringList { QStringLiteral("Root") });
    CHECK(byName.value(QStringLiteral("ExitList")) == QStringList { QStringLiteral("Root") });

    // A group pulled into a shared type reaches every element that type belongs to.
    CHECK(byName.value(QStringLiteral("Start"))
            == (QStringList { QStringLiteral("EntryList"), QStringLiteral("ExitList") }));
    CHECK(byName.value(QStringLiteral("Stop"))
            == (QStringList { QStringLiteral("EntryList"), QStringLiteral("ExitList") }));

    // A shared named type gives its children every element written with it.
    CHECK(byName.value(QStringLiteral("Description"))
            == (QStringList { QStringLiteral("Start"), QStringLiteral("Stop") }));

    // Text that is not a schema answers nothing rather than answering wrongly.
    CHECK(DocSchemaReader::read(QByteArray("<not-a-schema/>")).isEmpty());
    CHECK(DocSchemaReader::read(QByteArray()).isEmpty());
}

//////////////////////////////////////////////////////////////////////////
// The three tables
//////////////////////////////////////////////////////////////////////////

// Every format answers, and each answers about its own vocabulary.
void testTablesLoad()
{
    std::printf("[table] the three formats\n");

    CHECK(DocElementTable::documentElement(eDocument::StateMachine) == QStringLiteral("StateMachine"));
    CHECK(DocElementTable::documentElement(eDocument::ServiceInterface) == QStringLiteral("ServiceInterface"));
    CHECK(DocElementTable::documentElement(eDocument::DataType) == QStringLiteral("DataTypeDocument"));

    // The document element sits nowhere; a section sits in it.
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"StateMachine", u""));
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"StateList", u"StateMachine"));
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"StateList", u"State"));

    // The state machine's own vocabulary is its own.
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"Transition", u"TransitionList"));
    CHECK(DocElementTable::accepts(eDocument::ServiceInterface, u"Transition", u"TransitionList") == false);
    CHECK(DocElementTable::accepts(eDocument::DataType, u"AttributeList", u"DataTypeDocument") == false);

    // What the three share, they share.
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"DataType", u"DataTypeList"));
    CHECK(DocElementTable::accepts(eDocument::ServiceInterface, u"DataType", u"DataTypeList"));
    CHECK(DocElementTable::accepts(eDocument::DataType, u"DataType", u"DataTypeList"));

    // A real element written where the format does not put it is not accepted either.
    CHECK(DocElementTable::accepts(eDocument::ServiceInterface, u"Method", u"AttributeList") == false);

    // The guard tree, which the schema states through a group referred to five times.
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"Cmp", u"Expr"));
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"Lit", u"Arg"));
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"Arg", u"Call"));
}

// The compiled-in copy is the whole description, not a stand-in for one: a build with no SDK
// schemas beside it has to validate exactly as well. The counts are not pinned -- the schemas
// own them -- but a table this small means the resource did not load at all.
void testBuiltInCopyIsComplete()
{
    std::printf("[table] the compiled-in copy\n");

    CHECK(DocSchemaReader::readFile(QStringLiteral(":/schema/fsml.xsd")).size() > 50);
    CHECK(DocSchemaReader::readFile(QStringLiteral(":/schema/siml.xsd")).size() > 20);
    CHECK(DocSchemaReader::readFile(QStringLiteral(":/schema/dtml.xsd")).size() > 10);

    // Whichever copy answered, the source is one of the two and it is named.
    CHECK(DocElementTable::sourcePath(eDocument::StateMachine).isEmpty() == false);
    CHECK((DocElementTable::source(eDocument::StateMachine) == DocElementTable::eSource::BuiltIn)
            == DocElementTable::sourcePath(eDocument::StateMachine).startsWith(QLatin1Char(':')));
}

//////////////////////////////////////////////////////////////////////////
// A document carrying what the format does not define
//////////////////////////////////////////////////////////////////////////

// The service interface: the tag is reported, and the block is still there after a save.
void testServiceInterfaceKeepsUnknownBlock()
{
    std::printf("[siml] an element the format does not define\n");

    QTemporaryDir dir;
    CHECK(dir.isValid());
    if (dir.isValid() == false)
        return;

    const QString path = dir.filePath(QStringLiteral("Sample.siml"));
    const QByteArray source =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<ServiceInterface FormatVersion=\"1.1.0\">\n"
        "    <Overview ID=\"1\" Name=\"Sample\" Version=\"1.0.0\" Category=\"Public\"/>\n"
        "    <AttributeList>\n"
        "        <Attribute ID=\"2\" Name=\"speed\" DataType=\"uint32\" Notify=\"OnChange\">\n"
        "            <FutureThing Mode=\"loud\"/>\n"
        "        </Attribute>\n"
        "    </AttributeList>\n"
        "</ServiceInterface>\n";
    CHECK(writeFile(path, source));

    ServiceInterfaceData data(path);
    CHECK(data.openSucceeded());
    CHECK(data.getUnknownElements().size() == 1);
    if (data.getUnknownElements().isEmpty() == false)
    {
        const DocUnknownElement& unknown = data.getUnknownElements().constFirst();
        CHECK(unknown.name == QStringLiteral("FutureThing"));
        CHECK(unknown.parent == QStringLiteral("Attribute"));
        CHECK(unknown.ownerId == 2u);
    }

    // The document still reads: the attribute is there beside the block that was not understood.
    CHECK(data.getAttributeData().getElements().size() == 1);

    CHECK(data.writeToFile());
    const QByteArray written = readFile(path);
    CHECK(written.contains("<FutureThing"));
    CHECK(written.contains("Mode=\"loud\""));
    CHECK(written.contains("speed"));
}

// The data type document, the same way.
void testDataTypeDocumentKeepsUnknownBlock()
{
    std::printf("[dtml] an element the format does not define\n");

    QTemporaryDir dir;
    CHECK(dir.isValid());
    if (dir.isValid() == false)
        return;

    const QString path = dir.filePath(QStringLiteral("Shared.dtml"));
    const QByteArray source =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<DataTypeDocument FormatVersion=\"1.0.0\">\n"
        "    <Overview ID=\"1\" Name=\"Shared\" Version=\"1.0.0\"/>\n"
        "    <DataTypeList>\n"
        "        <DataType ID=\"2\" Name=\"Colour\" Type=\"Enumeration\">\n"
        "            <Palette Depth=\"8\"/>\n"
        "            <FieldList>\n"
        "                <EnumEntry ID=\"3\" Name=\"Red\"/>\n"
        "            </FieldList>\n"
        "        </DataType>\n"
        "    </DataTypeList>\n"
        "</DataTypeDocument>\n";
    CHECK(writeFile(path, source));

    DataTypeDocumentData data(path);
    CHECK(data.openSucceeded());
    CHECK(data.getUnknownElements().size() == 1);
    if (data.getUnknownElements().isEmpty() == false)
    {
        CHECK(data.getUnknownElements().constFirst().name == QStringLiteral("Palette"));
        CHECK(data.getUnknownElements().constFirst().parent == QStringLiteral("DataType"));
    }

    CHECK(data.getDataTypeData().getCustomDataTypes().size() == 1);

    CHECK(data.writeToFile());
    const QByteArray written = readFile(path);
    CHECK(written.contains("<Palette"));
    CHECK(written.contains("Depth=\"8\""));
    CHECK(written.contains("Colour"));
}

// A document with nothing unexpected in it is not touched by any of this.
void testCleanDocumentReportsNothing()
{
    std::printf("[siml] a document the format does define\n");

    QTemporaryDir dir;
    CHECK(dir.isValid());
    if (dir.isValid() == false)
        return;

    const QString path = dir.filePath(QStringLiteral("Clean.siml"));
    const QByteArray source =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<ServiceInterface FormatVersion=\"1.1.0\">\n"
        "    <Overview ID=\"1\" Name=\"Clean\" Version=\"1.0.0\" Category=\"Public\">\n"
        "        <Description>Nothing unusual.</Description>\n"
        "    </Overview>\n"
        "    <MethodList>\n"
        "        <Method ID=\"2\" Name=\"start\" MethodType=\"Request\">\n"
        "            <ParamList>\n"
        "                <Parameter ID=\"3\" Name=\"speed\" DataType=\"uint32\"/>\n"
        "            </ParamList>\n"
        "        </Method>\n"
        "    </MethodList>\n"
        "</ServiceInterface>\n";
    CHECK(writeFile(path, source));

    ServiceInterfaceData data(path);
    CHECK(data.openSucceeded());
    CHECK(data.getUnknownElements().isEmpty());
}

//////////////////////////////////////////////////////////////////////////
// The delivered copy
//////////////////////////////////////////////////////////////////////////

// A schema put where the build puts the delivered one is taken instead of the compiled-in
// copy, and removing it puts the compiled-in copy back. This is the whole point of delivering
// them: a format change reaches an installed Lusan without rebuilding it.
void testDeliveredCopyWins()
{
    std::printf("[table] the delivered copy is preferred\n");

    const QString directory = DocSchemaReader::deliveryDirectory();
    CHECK(directory.isEmpty() == false);
    if (directory.isEmpty())
        return;

    const QString path = directory + QStringLiteral("/dtml.xsd");
    const bool existed = QFile::exists(path);
    QByteArray saved;
    if (existed)
    {
        saved = readFile(path);
    }

    CHECK(QDir().mkpath(directory));

    // One element, named nothing the real format knows, so taking it cannot be mistaken.
    const QByteArray schema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "  <xs:element name=\"DataTypeDocument\">"
        "    <xs:complexType>"
        "      <xs:all>"
        "        <xs:element name=\"DeliveredOnly\" type=\"xs:string\"/>"
        "      </xs:all>"
        "    </xs:complexType>"
        "  </xs:element>"
        "</xs:schema>";
    CHECK(writeFile(path, schema));

    DocElementTable::reload();
    CHECK(DocElementTable::source(eDocument::DataType) == DocElementTable::eSource::Delivered);
    CHECK(DocElementTable::sourcePath(eDocument::DataType) == path);
    CHECK(DocElementTable::accepts(eDocument::DataType, u"DeliveredOnly", u"DataTypeDocument"));
    CHECK(DocElementTable::accepts(eDocument::DataType, u"DataTypeList", u"DataTypeDocument") == false);

    // Only the format whose schema was replaced changed; the other two are untouched by it.
    CHECK(DocElementTable::accepts(eDocument::StateMachine, u"StateList", u"StateMachine"));
    CHECK(DocElementTable::accepts(eDocument::ServiceInterface, u"MethodList", u"ServiceInterface"));

    // Taking it away puts the compiled-in copy back, which is what has to happen when an SDK
    // that carries no schemas is used.
    if (existed)
    {
        CHECK(writeFile(path, saved));
    }
    else
    {
        CHECK(QFile::remove(path));
    }

    DocElementTable::reload();
    CHECK(DocElementTable::source(eDocument::DataType) == (existed ? DocElementTable::eSource::Delivered
                                                                  : DocElementTable::eSource::BuiltIn));
    CHECK(DocElementTable::accepts(eDocument::DataType, u"DataTypeList", u"DataTypeDocument"));
    CHECK(DocElementTable::accepts(eDocument::DataType, u"DeliveredOnly", u"DataTypeDocument") == false);
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::printf("Document schema tests\n");
    testReaderShapes();
    testTablesLoad();
    testBuiltInCopyIsComplete();
    testServiceInterfaceKeepsUnknownBlock();
    testDataTypeDocumentKeepsUnknownBlock();
    testCleanDocumentReportsNothing();
    testDeliveredCopyWins();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
