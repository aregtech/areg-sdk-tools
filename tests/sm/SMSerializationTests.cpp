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
 *  \file        tests/sm/SMSerializationTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-02 unit tests: `.fsml` deterministic XML round-trip and parse robustness.
 *
 *  Self-contained test program (no external framework, no new dependency):
 *    1. `TrafficLight.fsml` loads and resaves byte-identically.
 *    3. Truncated/corrupted documents terminate with a clean error (no hang, no crash,
 *       and never open as an empty valid document).
 *    4. Code bodies and expressions round-trip byte-exactly through CDATA.
 *  Acceptance 2 (validation against `fsml.xsd`) is checked out of process with lxml — the
 *  written output is produced here and validated by the build/verify step.
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMMethodData.hpp"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <cstdio>

#ifndef LUSAN_TEST_DATA_DIR
#define LUSAN_TEST_DATA_DIR "."
#endif

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

    QString dataFile(const char* name)
    {
        return QString(LUSAN_TEST_DATA_DIR) + QDir::separator() + QString::fromLatin1(name);
    }

    QString outFile(const char* name)
    {
        return QDir::tempPath() + QDir::separator() + QString::fromLatin1(name);
    }

    QByteArray readAllBytes(const QString& path)
    {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly) == false)
        {
            return QByteArray();
        }

        const QByteArray content = file.readAll();
        file.close();
        return content;
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Acceptance 1: byte-identical round-trip of the reference document
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testRoundTrip(void)
    {
        std::printf("[SM-02] byte-identical round-trip (TrafficLight.fsml)\n");

        const QString refPath = dataFile("TrafficLight.fsml");
        const QByteArray original = readAllBytes(refPath);
        CHECK(original.isEmpty() == false);

        StateMachineData doc;
        const bool opened = doc.readFromFile(refPath);
        CHECK(opened);
        CHECK(doc.openSucceeded());

        const QString outPath = outFile("sm02_roundtrip.fsml");
        CHECK(doc.writeToFile(outPath));

        const QByteArray written = readAllBytes(outPath);
        CHECK(written == original);

        if (written != original)
        {
            const int limit = std::min(original.size(), written.size());
            int diff = 0;
            while ((diff < limit) && (original.at(diff) == written.at(diff)))
            {
                ++diff;
            }

            std::printf("  [DIFF] sizes original=%lld written=%lld; first difference at byte %d\n",
                        static_cast<long long>(original.size()), static_cast<long long>(written.size()), diff);
            const int from = (diff > 40) ? diff - 40 : 0;
            std::printf("  [DIFF] expected: ...%s\n", original.mid(from, 90).toStdString().c_str());
            std::printf("  [DIFF] actual  : ...%s\n", written.mid(from, 90).toStdString().c_str());
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Acceptance 4: bodies and expressions round-trip byte-exactly through CDATA
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testCData(void)
    {
        std::printf("[SM-02] CDATA bodies/expressions round-trip byte-exactly\n");

        // Content that exercises XML-significant characters, newlines and a nested CDATA
        // terminator (which the writer must split and the reader must rejoin).
        const QString body =
            "if (a < b && c > d) {\n"
            "    s = \"x\";\n"
            "    // a]]>b marker & <tag>\n"
            "}\nreturn a & b;";
        const QString expr = "count >= 3 && flag == true /* n<o>te */";

        StateMachineData doc;
        doc.getOverview().setName("CdataTest");

        SMMethodData& methods = doc.getMethods();
        methods.createMethod("Go", SMMethodEntry::eMethodType::Trigger);
        SMMethodEntry* cond = methods.createMethod("Check", SMMethodEntry::eMethodType::Condition);
        CHECK(cond != nullptr);
        cond->setReturn("bool");
        cond->setImplement(SMMethodEntry::eImplement::Embedded);
        cond->setBody(body);

        SMStateEntry* root = doc.getStates().createState("Root", SMStateEntry::eStateKind::Start);
        CHECK(root != nullptr);

        SMInlineCode* inline1 = new SMInlineCode();
        inline1->setBody(body);
        root->getEntryList().addOperation(inline1);

        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "Go");
        SMConditionEntry* row = trans->getConditions().addCondition();
        row->setLhsKind(SMArgumentEntry::eValueSource::Expression);
        row->setExpression(expr);

        const QString outPath = outFile("sm02_cdata.fsml");
        CHECK(doc.writeToFile(outPath));

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());

        SMMethodEntry* rcond = reread.getMethods().findMethod("Check");
        CHECK(rcond != nullptr);
        CHECK((rcond != nullptr) && (rcond->getBody() == body));

        SMStateEntry* rroot = reread.getStates().findState("Root");
        CHECK(rroot != nullptr);
        CHECK((rroot != nullptr) && (rroot->getEntryList().getCount() == 1));
        if ((rroot != nullptr) && (rroot->getEntryList().getCount() == 1))
        {
            SMOperationBase* op = rroot->getEntryList().at(0);
            CHECK(op->getOperationType() == SMOperationBase::eOperation::InlineCode);
            CHECK(static_cast<SMInlineCode*>(op)->getBody() == body);
        }

        if ((rroot != nullptr) && (rroot->getTransitions().getElements().isEmpty() == false))
        {
            SMTransitionEntry* rtrans = rroot->getTransitions().getElements().first();
            CHECK(rtrans->getConditions().getElements().isEmpty() == false);
            if (rtrans->getConditions().getElements().isEmpty() == false)
            {
                CHECK(rtrans->getConditions().getElements().first()->getExpression() == expr);
            }
        }

        // A second resave of the reloaded model must be byte-identical to the first (spec
        // 7.7.4 determinism / idempotence).
        const QString outPath2 = outFile("sm02_cdata_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        CHECK(readAllBytes(outPath) == readAllBytes(outPath2));
    }
}

//////////////////////////////////////////////////////////////////////////
// Acceptance 3: truncated / corrupted input terminates with a clean error
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testRobustness(void)
    {
        std::printf("[SM-02] truncated/corrupted documents terminate with a clean error\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        const QString truncPath = outFile("sm02_trunc.fsml");

        // Every prefix of the document is malformed (unclosed elements): opening must fail
        // and, crucially, must terminate (no infinite loop — reaching here proves it).
        // Every length here cuts before the root `</StateMachine>` closes, so each prefix
        // is genuinely incomplete (an unclosed document), never a valid whole.
        const int lengths[] = { 1, 10, 42, 100, 250, 600, 1500, 4000, 9000, 12000, 16000, 17900 };
        for (int len : lengths)
        {
            QFile file(truncPath);
            CHECK(file.open(QIODevice::WriteOnly));
            file.write(original.left(len));
            file.close();

            StateMachineData doc;
            const bool opened = doc.readFromFile(truncPath);
            CHECK(opened == false);
            CHECK(doc.openSucceeded() == false);
        }

        // Byte-level corruption at several positions: must not hang or crash (result value
        // is unconstrained; termination is the guarantee).
        const int positions[] = { 5, 60, 300, 1200, 5000, 11000, 16000 };
        for (int pos : positions)
        {
            if (pos >= original.size())
            {
                continue;
            }

            QByteArray corrupt = original;
            corrupt[pos] = static_cast<char>(corrupt.at(pos) ^ 0x7F);
            corrupt[(pos + 7) % corrupt.size()] = '<';

            QFile file(truncPath);
            CHECK(file.open(QIODevice::WriteOnly));
            file.write(corrupt);
            file.close();

            StateMachineData doc;
            doc.readFromFile(truncPath);   // must simply return
        }

        std::printf("  [OK] all malformed inputs terminated\n");
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM-02 XML serialization tests ====\n");

    testRoundTrip();
    testCData();
    testRobustness();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
