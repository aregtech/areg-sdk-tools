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
// SM-21-07: layout and logic are independently diffable
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< The trailing '<Layout ...</Layout>' block of a serialized document, or empty.
    QByteArray layoutBlock(const QByteArray& xml)
    {
        const int start = xml.indexOf("<Layout");
        const int end   = xml.indexOf("</Layout>");
        if ((start < 0) || (end < 0))
        {
            return QByteArray();
        }

        return xml.mid(start, (end + 9) - start);
    }

    //!< Everything before the trailing Layout block: the logic sections.
    QByteArray logicPart(const QByteArray& xml)
    {
        const int start = xml.indexOf("<Layout");
        return (start < 0) ? xml : xml.left(start);
    }

    void testLayoutLogicSeparation()
    {
        std::printf("[SM-21-07] layout and logic are independently diffable\n");

        const QString refPath = dataFile("TrafficLight.fsml");

        StateMachineData base;
        CHECK(base.readFromFile(refPath));
        const QString basePath = outFile("sm2107_base.fsml");
        CHECK(base.writeToFile(basePath));
        const QByteArray baseBytes = readAllBytes(basePath);
        CHECK(layoutBlock(baseBytes).isEmpty() == false);   // the reference carries geometry

        // Moving a state box changes only the Layout block: a drag-to-tidy never rewrites logic.
        StateMachineData moved;
        CHECK(moved.readFromFile(refPath));
        SMStateEntry* off = moved.findState("LightOff");
        CHECK(off != nullptr);
        SMLayoutNode* node = (off != nullptr) ? moved.getLayout().findNode(off->getId()) : nullptr;
        CHECK(node != nullptr);
        if (node != nullptr)
        {
            node->x += 64.0;
            node->y += 32.0;
        }
        const QString movePath = outFile("sm2107_move.fsml");
        CHECK(moved.writeToFile(movePath));
        const QByteArray moveBytes = readAllBytes(movePath);

        CHECK(logicPart(moveBytes)   == logicPart(baseBytes));      // logic untouched by a move
        CHECK(layoutBlock(moveBytes) != layoutBlock(baseBytes));    // only geometry changed

        // Renaming a state changes only logic: layout keys by ID, so it is byte-identical.
        StateMachineData renamed;
        CHECK(renamed.readFromFile(refPath));
        SMStateEntry* on = renamed.findState("LightOn");
        CHECK(on != nullptr);
        if (on != nullptr)
        {
            on->setName(QStringLiteral("LightOnRenamed"));
        }
        const QString renamePath = outFile("sm2107_rename.fsml");
        CHECK(renamed.writeToFile(renamePath));
        const QByteArray renameBytes = readAllBytes(renamePath);

        CHECK(layoutBlock(renameBytes) == layoutBlock(baseBytes));  // geometry untouched by a rename
        CHECK(logicPart(renameBytes)   != logicPart(baseBytes));    // only logic changed

        // Geometry survives a full round-trip byte-exactly and deterministically.
        StateMachineData reread;
        CHECK(reread.readFromFile(movePath));
        CHECK(reread.openSucceeded());
        const QString movePath2 = outFile("sm2107_move2.fsml");
        CHECK(reread.writeToFile(movePath2));
        CHECK(readAllBytes(movePath) == readAllBytes(movePath2));
    }
}

//////////////////////////////////////////////////////////////////////////
// SM-21-R24: state `Do` activity (interval + stop condition)
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDoActivity()
    {
        std::printf("[SM-21-R24] state Do activity: interval + stop-condition round-trip\n");

        StateMachineData doc;
        doc.getOverview().setName("DoActivity");
        doc.getMethods().createMethod("Go", SMMethodEntry::eMethodType::Trigger);

        // Root: a timer-loop Do activity -- interval and stop-condition both set.
        SMStateEntry* root = doc.getStates().createState("Root", SMStateEntry::eStateKind::Start);
        CHECK(root != nullptr);
        SMInlineCode* work = new SMInlineCode();
        work->setBody("tick();");
        root->getDoList().addOperation(work);
        root->setDoInterval(200u);
        root->setDoUntil("isDone");

        // Loop: the trigger-driven variant -- a Do list with interval 0 and no stop-condition must
        // omit both attributes on the wrapper.
        SMStateEntry* loop = doc.getStates().createState("Loop", SMStateEntry::eStateKind::Normal);
        CHECK(loop != nullptr);
        SMInlineCode* pump = new SMInlineCode();
        pump->setBody("pump();");
        loop->getDoList().addOperation(pump);

        const QString outPath = outFile("sm21_do.fsml");
        CHECK(doc.writeToFile(outPath));

        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("<DoList"));
        CHECK(written.contains("Interval=\"200\""));
        CHECK(written.contains("Until=\"isDone\""));

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());

        SMStateEntry* rroot = reread.getStates().findState("Root");
        CHECK(rroot != nullptr);
        if (rroot != nullptr)
        {
            CHECK(rroot->getDoList().getCount() == 1);
            CHECK(rroot->getDoInterval() == 200u);
            CHECK(rroot->getDoUntil() == QString("isDone"));
        }

        SMStateEntry* rloop = reread.getStates().findState("Loop");
        CHECK(rloop != nullptr);
        if (rloop != nullptr)
        {
            CHECK(rloop->getDoList().getCount() == 1);
            CHECK(rloop->getDoInterval() == 0u);
            CHECK(rloop->getDoUntil().isEmpty());
        }

        // A second resave of the reloaded model must be byte-identical (determinism / idempotence).
        const QString outPath2 = outFile("sm21_do_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        CHECK(readAllBytes(outPath) == readAllBytes(outPath2));
    }
}

//////////////////////////////////////////////////////////////////////////
// Ephemeral submachine: a not-real submachine (only Start/Final, or empty) is
// created in RAM while the user builds it but is never persisted (issue #514 follow-up).
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testEphemeralSubmachine()
    {
        std::printf("[SM-514] a submachine with no Normal state is not saved (and neither is its layout)\n");

        StateMachineData doc;
        doc.getOverview().setName("Ephemeral");

        SMStateEntry* start  = doc.getStates().createState("Root", SMStateEntry::eStateKind::Start);
        SMStateEntry* worker = doc.getStates().createState("Worker", SMStateEntry::eStateKind::Normal);
        CHECK((start != nullptr) && (worker != nullptr));
        doc.getLayout().addNode(worker->getId()).x = 100.0;         // Worker's own box: must be kept

        // Give Worker a submachine that holds only a Start marker -- not a real state.
        SMStateData* nested = worker->getOrCreateNestedStates();
        SMStateEntry* subStart = nested->createState("SubStart", SMStateEntry::eStateKind::Start);
        CHECK(subStart != nullptr);
        const uint32_t subStartId = subStart->getId();
        doc.getLayout().addNode(subStartId).x = 200.0;              // nested Node: must be dropped
        doc.getLayout().addView(worker->getId()).zoom = 150;        // Worker's sublevel View: must be dropped

        const QString outPath = outFile("sm514_empty.fsml");
        CHECK(doc.writeToFile(outPath));
        const QByteArray written = readAllBytes(outPath);

        // The nested Start is omitted from the StateList, and its layout Node is omitted too.
        CHECK(written.contains("SubStart") == false);
        CHECK(written.contains(QString("Owner=\"%1\"").arg(subStartId).toLatin1()) == false);
        CHECK(written.contains("Worker"));                          // the composite reverts to a plain state

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());
        SMStateEntry* rworker = reread.getStates().findState("Worker");
        CHECK((rworker != nullptr) && (rworker->hasNestedStates() == false));
        CHECK(reread.getStates().findStateRecursive("SubStart") == nullptr);
        CHECK(reread.getLayout().findNode(worker->getId()) != nullptr);   // Worker's own box survived

        // Adding a Normal state makes the submachine real: now it IS persisted.
        SMStateData* nested2 = worker->getOrCreateNestedStates();
        SMStateEntry* inner = nested2->createState("Inner", SMStateEntry::eStateKind::Normal);
        CHECK(inner != nullptr);
        const QString outPath2 = outFile("sm514_real.fsml");
        CHECK(doc.writeToFile(outPath2));
        const QByteArray written2 = readAllBytes(outPath2);
        CHECK(written2.contains("Inner"));
        CHECK(written2.contains("SubStart"));                       // the whole real submachine persists

        StateMachineData reread2;
        CHECK(reread2.readFromFile(outPath2));
        SMStateEntry* rworker2 = reread2.getStates().findState("Worker");
        CHECK((rworker2 != nullptr) && rworker2->hasNestedStates());
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM serialization/versioning tests ====\n");

    testRoundTrip();
    testLayoutLogicSeparation();
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
    testDoActivity();
    testEphemeralSubmachine();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
