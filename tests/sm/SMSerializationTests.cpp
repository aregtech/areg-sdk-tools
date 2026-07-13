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
 *  \brief       SM-02/SM-03 unit tests: `.fsml` serialization robustness and versioning.
 *
 *  Self-contained test program (no external framework, no new dependency):
 *    1. `TrafficLight.fsml` loads and resaves byte-identically.
 *    3. Truncated/corrupted documents terminate with a clean error (no hang, no crash,
 *       and never open as an empty valid document).
 *    4. Code bodies and expressions round-trip byte-exactly through CDATA.
 *    5. FormatVersion migration/preservation/refusal behavior follows spec 7.8.
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
#include <QThread>
#include <QXmlStreamReader>
#include <cstdio>
#include <memory>

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

    bool writeAllBytes(const QString& path, const QByteArray& content)
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

    bool replaceOnce(QByteArray& haystack, const QByteArray& needle, const QByteArray& replacement)
    {
        const int pos = haystack.indexOf(needle);
        if (pos < 0)
        {
            return false;
        }

        haystack.replace(pos, needle.size(), replacement);
        return true;
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Acceptance 1: byte-identical round-trip of the reference document
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testRoundTrip()
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
    void testCData()
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
            CHECK(rtrans->getConditions().collectLeaves().isEmpty() == false);
            if (rtrans->getConditions().collectLeaves().isEmpty() == false)
            {
                CHECK(rtrans->getConditions().collectLeaves().first()->getExpression() == expr);
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
// SM-21-02: nested condition groups, group negate, and the Lambda leaf
//////////////////////////////////////////////////////////////////////////

namespace
{
    // Navigates to the first transition's condition tree of state "Root".
    SMConditionList* firstGuard(StateMachineData& doc)
    {
        SMStateEntry* root = doc.getStates().findState("Root");
        if ((root == nullptr) || root->getTransitions().getElements().isEmpty())
        {
            return nullptr;
        }

        return &root->getTransitions().getElements().first()->getConditions();
    }

    void testNestedConditions()
    {
        std::printf("[SM-21-02] nested groups + group negate + Lambda leaf round-trip\n");

        const QString lambdaBody = "int n = count;\nif (n < 0) { return false; }\nreturn n & 1;";

        StateMachineData doc;
        doc.getOverview().setName("NestedCond");
        doc.getMethods().createMethod("Go", SMMethodEntry::eMethodType::Trigger);

        SMStateEntry* root = doc.getStates().createState("Root", SMStateEntry::eStateKind::Start);
        CHECK(root != nullptr);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "Go");
        CHECK(trans != nullptr);

        // Build: WalkRequested && !(HasWaiting || count >= MIN_WAITING) && !IsNightMode && lambda{..}
        SMConditionList& conds = trans->getConditions();
        SMConditionEntry* c0 = conds.addCondition();
        c0->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
        c0->setLhs("WalkRequested");

        SMConditionGroup* grp = conds.addGroup();
        grp->setCombine(SMConditionGroup::eCombine::Or);
        grp->setNegated(true);
        SMConditionEntry* g0 = grp->addCondition();
        g0->setLhsKind(SMArgumentEntry::eValueSource::Condition);
        g0->setLhs("HasWaiting");
        SMConditionEntry* g1 = grp->addCondition();
        g1->setLhsKind(SMArgumentEntry::eValueSource::Param);
        g1->setLhs("count");
        g1->setOperator(SMConditionEntry::eOperator::GreaterEqual);
        g1->setRhsKind(SMArgumentEntry::eValueSource::Constant);
        g1->setRhs("MIN_WAITING");

        SMConditionEntry* c2 = conds.addCondition();
        c2->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
        c2->setLhs("IsNightMode");
        c2->setNegated(true);

        SMConditionEntry* c3 = conds.addCondition();
        c3->setLhsKind(SMArgumentEntry::eValueSource::Lambda);
        c3->setBody(lambdaBody);

        const QString outPath = outFile("sm21_nested.fsml");
        CHECK(doc.writeToFile(outPath));

        // The nested form must emit a ConditionGroup element.
        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("<ConditionGroup"));

        // Reread and verify the tree shape.
        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());

        SMConditionList* rc = firstGuard(reread);
        CHECK(rc != nullptr);
        if (rc != nullptr)
        {
            CHECK(rc->getCount() == 4);
            CHECK(rc->collectLeaves().size() == 5);

            const QList<SMConditionNode*>& kids = rc->getChildren();
            CHECK((kids.size() == 4) && kids.at(0)->isLeaf() && kids.at(1)->isGroup()
                  && kids.at(2)->isLeaf() && kids.at(3)->isLeaf());

            if ((kids.size() == 4) && kids.at(1)->isGroup())
            {
                SMConditionEntry* r0 = static_cast<SMConditionEntry*>(kids.at(0));
                CHECK(r0->getLhs() == "WalkRequested");

                SMConditionGroup* rg = static_cast<SMConditionGroup*>(kids.at(1));
                CHECK(rg->getCombine() == SMConditionGroup::eCombine::Or);
                CHECK(rg->isNegated());
                CHECK(rg->getCount() == 2);

                SMConditionEntry* r2 = static_cast<SMConditionEntry*>(kids.at(2));
                CHECK((r2->getLhs() == "IsNightMode") && r2->isNegated());

                SMConditionEntry* r3 = static_cast<SMConditionEntry*>(kids.at(3));
                CHECK(r3->isLambdaRow());
                CHECK(r3->getBody() == lambdaBody);
            }
        }

        // Idempotent resave (spec 7.7.4 determinism).
        const QString outPath2 = outFile("sm21_nested_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        CHECK(readAllBytes(outPath) == readAllBytes(outPath2));
    }

    void testFlatGuardStaysLegacy()
    {
        std::printf("[SM-21-02] a flat guard still serializes as legacy ConditionList (no ConditionGroup)\n");

        StateMachineData doc;
        doc.getOverview().setName("FlatCond");
        doc.getMethods().createMethod("Go", SMMethodEntry::eMethodType::Trigger);

        SMStateEntry* root = doc.getStates().createState("Root", SMStateEntry::eStateKind::Start);
        CHECK(root != nullptr);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "Go");
        CHECK(trans != nullptr);

        SMConditionEntry* c0 = trans->getConditions().addCondition();
        c0->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
        c0->setLhs("WalkRequested");
        SMConditionEntry* c1 = trans->getConditions().addCondition();
        c1->setLhsKind(SMArgumentEntry::eValueSource::Attribute);
        c1->setLhs("IsNightMode");

        const QString outPath = outFile("sm21_flat.fsml");
        CHECK(doc.writeToFile(outPath));

        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("<ConditionList"));
        CHECK(written.contains("<ConditionGroup") == false);
        CHECK(written.contains("Negate=") == false);

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        SMConditionList* rc = firstGuard(reread);
        CHECK((rc != nullptr) && (rc->getCount() == 2) && (rc->collectGroups().isEmpty()));

        const QString outPath2 = outFile("sm21_flat_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        CHECK(readAllBytes(outPath) == readAllBytes(outPath2));
    }
}

//////////////////////////////////////////////////////////////////////////
// Deprecation flags/hints round-trip on events, methods, timers, attributes and params
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDeprecation()
    {
        std::printf("[deprecation] flags + hints round-trip on events/methods/timers/attributes/params\n");

        StateMachineData doc;
        doc.getOverview().setName("DeprecationTest");

        SMAttributeEntry* attr = doc.getAttributes().createAttribute("mCount");
        CHECK(attr != nullptr);
        if (attr != nullptr) { attr->setIsDeprecated(true); attr->setDeprecateHint("use mTotal"); }

        SMEventEntry* ev = doc.getEvents().createEvent("Started");
        CHECK(ev != nullptr);
        if (ev != nullptr)
        {
            ev->setIsDeprecated(true);
            ev->setDeprecateHint("replaced by Ready");
            MethodParameter* p = ev->addParam("code");
            CHECK(p != nullptr);
            if (p != nullptr) { p->setIsDeprecated(true); p->setDeprecateHint("unused payload"); }
        }

        SMMethodEntry* m = doc.getMethods().createMethod("Go", SMMethodEntry::eMethodType::Trigger);
        CHECK(m != nullptr);
        if (m != nullptr) { m->setIsDeprecated(true); m->setDeprecateHint("call Start instead"); }

        SMTimerEntry* t = doc.getTimers().createTimer("Tick");
        CHECK(t != nullptr);
        if (t != nullptr) { t->setIsDeprecated(true); t->setDeprecateHint("no longer used"); }

        const QString outPath = outFile("dep_roundtrip.fsml");
        CHECK(doc.writeToFile(outPath));

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());

        SMAttributeEntry* rattr = reread.getAttributes().findElement("mCount");
        CHECK((rattr != nullptr) && rattr->getIsDeprecated() && (rattr->getDeprecateHint() == "use mTotal"));

        SMEventEntry* rev = reread.getEvents().findEvent("Started");
        CHECK((rev != nullptr) && rev->getIsDeprecated() && (rev->getDeprecateHint() == "replaced by Ready"));
        if (rev != nullptr)
        {
            MethodParameter* rp = rev->findElement("code");
            CHECK((rp != nullptr) && rp->getIsDeprecated() && (rp->getDeprecateHint() == "unused payload"));
        }

        SMMethodEntry* rm = reread.getMethods().findMethod("Go");
        CHECK((rm != nullptr) && rm->getIsDeprecated() && (rm->getDeprecateHint() == "call Start instead"));

        SMTimerEntry* rt = reread.getTimers().findElement("Tick");
        CHECK((rt != nullptr) && rt->getIsDeprecated() && (rt->getDeprecateHint() == "no longer used"));

        // Idempotent resave.
        const QString outPath2 = outFile("dep_roundtrip_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        CHECK(readAllBytes(outPath) == readAllBytes(outPath2));
    }
}

//////////////////////////////////////////////////////////////////////////
// Acceptance 3: truncated / corrupted input terminates with a clean error
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testVersionMigration()
    {
        std::printf("[SM-03] older FormatVersion migrates in memory only\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        QByteArray older = original;
        CHECK(replaceOnce(older, "FormatVersion=\"1.0.0\"", "FormatVersion=\"0.9.0\""));

        const QString olderPath = outFile("sm03_older.fsml");
        CHECK(writeAllBytes(olderPath, older));
        const QByteArray beforeOpen = readAllBytes(olderPath);

        StateMachineData doc;
        CHECK(doc.readFromFile(olderPath));
        CHECK(doc.openSucceeded());
        CHECK(doc.getFormatVersion().toString() == StateMachineData::XML_FORMAT_DEFAULT);

        const QByteArray afterOpen = readAllBytes(olderPath);
        CHECK(beforeOpen == afterOpen);
    }

    void testUnknownPreservation()
    {
        std::printf("[SM-03] newer minor preserves unknown root content\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        QByteArray future = original;
        CHECK(replaceOnce(future,
                          "<StateMachine FormatVersion=\"1.0.0\">",
                          "<StateMachine FormatVersion=\"1.1.0\" FutureAttr=\"KeepMe\">"));
        CHECK(replaceOnce(future,
                          "</StateMachine>",
                          "    <FutureSection Flag=\"x\"><FutureLeaf Value=\"42\"/></FutureSection>\n</StateMachine>"));

        const QString inPath = outFile("sm03_future_minor_in.fsml");
        CHECK(writeAllBytes(inPath, future));

        StateMachineData doc;
        CHECK(doc.readFromFile(inPath));
        CHECK(doc.openSucceeded());
        CHECK(doc.getFormatVersion().toString() == QString("1.1.0"));

        const QString outPath = outFile("sm03_future_minor_out.fsml");
        CHECK(doc.writeToFile(outPath));

        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("FormatVersion=\"1.1.0\""));
        CHECK(written.contains("FutureAttr=\"KeepMe\""));
        CHECK(written.contains("<FutureSection"));
        CHECK(written.contains("<FutureLeaf"));
    }

    void testRejectNewerMajor()
    {
        std::printf("[SM-03] newer major is refused with both versions in the message\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        QByteArray newerMajor = original;
        CHECK(replaceOnce(newerMajor, "FormatVersion=\"1.0.0\"", "FormatVersion=\"2.0.0\""));

        const QString newerPath = outFile("sm03_newer_major.fsml");
        CHECK(writeAllBytes(newerPath, newerMajor));
        const QByteArray beforeOpen = readAllBytes(newerPath);

        StateMachineData doc;
        const bool opened = doc.readFromFile(newerPath);
        CHECK(opened == false);
        CHECK(doc.openSucceeded() == false);
        CHECK(readAllBytes(newerPath) == beforeOpen);

        QXmlStreamReader xml(newerMajor);
        CHECK(xml.readNextStartElement());

        StateMachineData direct;
        CHECK(direct.readFromXml(xml) == false);
        CHECK(xml.hasError());
        const QString error = xml.errorString();
        CHECK(error.contains("2.0.0"));
        CHECK(error.contains(StateMachineData::XML_FORMAT_DEFAULT));
    }
}

////////////////////////////////////////////////////////////////////////////
// Acceptance criteria for SM-03
////////////////////////////////////////////////////////////////////////////

namespace
{
    void testRobustness()
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

//////////////////////////////////////////////////////////////////////////////
// Acceptance criteria for SM-05 (data-layer part)
//////////////////////////////////////////////////////////////////////////////

namespace
{
    void testNewDocumentSkeleton()
    {
        std::printf("[SM-05] new document skeleton has Overview + root Start + default layout\n");

        std::unique_ptr<StateMachineData> doc = StateMachineData::createNewDocument("NewMachine");
        CHECK(doc != nullptr);
        CHECK((doc != nullptr) && doc->openSucceeded());
        CHECK((doc != nullptr) && (doc->getOverview().getName() == QString("NewMachine")));

        SMStateEntry* start = (doc != nullptr) ? doc->getStates().getStartState() : nullptr;
        CHECK(start != nullptr);
        CHECK((start != nullptr) && (start->getKind() == SMStateEntry::eStateKind::Start));

        const SMLayoutView* rootView = (doc != nullptr) ? doc->getLayout().findView(doc->getOverview().getId()) : nullptr;
        CHECK(rootView != nullptr);
        const SMLayoutNode* startNode = (doc != nullptr && start != nullptr) ? doc->getLayout().findNode(start->getId()) : nullptr;
        CHECK(startNode != nullptr);
    }

    void testAutosaveHelpers()
    {
        std::printf("[SM-05] autosave helper path/detection/remove\n");

        const QString docPath = outFile("sm05_autosave_doc.fsml");
        const QString autosavePath = StateMachineData::autosavePathForDocument(docPath);
        CHECK(autosavePath.endsWith(".fsml.autosave"));

        std::unique_ptr<StateMachineData> created = StateMachineData::createNewDocument("AutosaveMachine");
        CHECK(created != nullptr);
        CHECK((created != nullptr) && created->writeToFile(docPath));

        QThread::msleep(20);

        StateMachineData dirty;
        dirty.getOverview().setName("AutosaveMachine");
        CHECK(dirty.writeToAutosaveFile(autosavePath));
        CHECK(StateMachineData::hasRecoverableAutosave(docPath));

        QString detectedPath;
        CHECK(StateMachineData::hasRecoverableAutosave(docPath, &detectedPath));
        CHECK(detectedPath == autosavePath);

        CHECK(StateMachineData::removeAutosave(docPath));
        CHECK(StateMachineData::hasRecoverableAutosave(docPath) == false);
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM serialization/versioning tests ====\n");

    testRoundTrip();
    testCData();
    testNestedConditions();
    testFlatGuardStaysLegacy();
    testDeprecation();
    testRobustness();
    testVersionMigration();
    testUnknownPreservation();
    testRejectNewerMajor();
    testNewDocumentSkeleton();
    testAutosaveHelpers();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
