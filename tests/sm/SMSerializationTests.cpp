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
 *  Acceptance 2 (validation against `fsml.xsd`) is checked out of process with lxml -- the
 *  written output is produced here and validated by the build/verify step.
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMMethodKind.hpp"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QString>
#include <QThread>
#include <QXmlStreamReader>
#include <cstdio>
#include <iterator>
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
    void roundTripDocument(const char* docName, const char* outName)
    {
        const QString refPath = dataFile(docName);
        const QByteArray original = readAllBytes(refPath);
        CHECK(original.isEmpty() == false);

        StateMachineData doc;
        const bool opened = doc.readFromFile(refPath);
        CHECK(opened);
        CHECK(doc.openSucceeded());

        const QString outPath = outFile(outName);
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

            std::printf("  [DIFF] %s: sizes original=%lld written=%lld; first difference at byte %d\n",
                        docName,
                        static_cast<long long>(original.size()), static_cast<long long>(written.size()), diff);
            const int from = (diff > 40) ? diff - 40 : 0;
            std::printf("  [DIFF] expected: ...%s\n", original.mid(from, 90).toStdString().c_str());
            std::printf("  [DIFF] actual  : ...%s\n", written.mid(from, 90).toStdString().c_str());
            std::printf("  [DIFF] full output kept at %s\n", outPath.toStdString().c_str());
        }
    }

    // A 1.0.0 document keeps its machine imports in a separate <ImportList>. Loading folds them
    // into the include list and saving writes 1.1.0 without the old section -- the one deliberate
    // exception to the byte-identical save rule, so it gets its own fixture and its own test.
    void testLegacyImportMigration()
    {
        std::printf("[SM-29-EXT] 1.0.0 ImportList folds into the include list and saves as 1.1.0\n");

        const QString refPath = dataFile("LegacyImports.fsml");
        StateMachineData doc;
        CHECK(doc.readFromFile(refPath));
        CHECK(doc.openSucceeded());
        CHECK(doc.getFormatVersion() == VersionNumber(StateMachineData::XML_FORMAT_110));

        // Document order survives the fold: the source include first, the folded import after it.
        CHECK(doc.getIncludes().getElementCount() == 2);
        CHECK(doc.getIncludes().getElements().at(0).getLocation() == QStringLiteral("areg/base/GEGlobal.h"));
        CHECK(doc.getIncludes().getElements().at(0).getAlias().isEmpty());

        const IncludeEntry* folded = doc.findImportByAlias(QStringLiteral("TurnCycle"));
        CHECK(folded != nullptr);
        CHECK((folded != nullptr) && (folded->getId() == 30u));
        CHECK((folded != nullptr) && (folded->getLocation() == QStringLiteral("./TurnCycle.fsml")));
        CHECK((folded != nullptr) && (folded->getVersion().toString() == QStringLiteral("1.2.0")));
        CHECK((folded != nullptr) && (folded->getDescription().isEmpty() == false));
        CHECK(doc.machineImports().size() == 1);

        const QString outPath = outFile("sm29ext_legacy.fsml");
        CHECK(doc.writeToFile(outPath));
        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("FormatVersion=\"1.1.0\""));
        CHECK(written.contains("<ImportList>") == false);
        CHECK(written.contains("<MachineImport") == false);
        CHECK(written.contains("Alias=\"TurnCycle\""));
        CHECK(written.contains("Version=\"1.2.0\""));

        // Second save is byte-identical: the migration is one-way, not per-save churn.
        StateMachineData migrated;
        CHECK(migrated.readFromFile(outPath));
        const QString twicePath = outFile("sm29ext_legacy_twice.fsml");
        CHECK(migrated.writeToFile(twicePath));
        CHECK(readAllBytes(twicePath) == written);
    }

    void testRoundTrip()
    {
        std::printf("[SM-02] byte-identical round-trip (TrafficLight.fsml)\n");
        roundTripDocument("TrafficLight.fsml", "sm02_roundtrip.fsml");
    }

    // The coverage fixture carries every persisted element and attribute, so a writer that
    // forgets one is caught here rather than by a user losing data. The other two reference
    // documents ride along: every `tests/data` fixture is kept in canonical writer form.
    void testFullFeatureRoundTrip()
    {
        std::printf("[SM-27] byte-identical round-trip (reference documents)\n");
        roundTripDocument("FullFeature.fsml", "sm27_fullfeature.fsml");
        roundTripDocument("GuardDemo.fsml", "sm27_guarddemo.fsml");
        // The import pair: one import instantiated by two states, both resolving.
        roundTripDocument("SubmachineDemo.fsml", "sm29_submachinedemo.fsml");
        roundTripDocument("TurnCycle.fsml", "sm29_turncycle.fsml");
        // An import that does not resolve persists like any other: the registration is kept so the
        // user can repair it, which is why it round-trips rather than being dropped on save.
        roundTripDocument("UnresolvedImport.fsml", "sm29_unresolvedimport.fsml");
        // Invalid by design (a Shared host importing a Local machine), but a document that fails
        // validation is still a document: it must persist in canonical form like any other.
        roundTripDocument("ThreadingMismatch.fsml", "sm29_threadingmismatch.fsml");
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

        MethodDataSection& methods = doc.getMethods();
        methods.createMethod("Go", NEMethod::SmTrigger);
        MethodEntry* cond = methods.createMethod("Check", NEMethod::SmCondition);
        CHECK(cond != nullptr);
        cond->setReturn("bool");
        cond->setImplement(MethodEntry::eImplement::Embedded);
        cond->setBody(body);

        // `Root` carries behaviour, so it is an ordinary state: a Kind="Start" is a pseudo-state
        // and the read shim would rewrite a merged-form one on the next load.
        SMStateEntry* begin = doc.getStates().createState("Begin", SMStateEntry::eStateKind::Start);
        SMStateEntry* root  = doc.getStates().createState("Root", SMStateEntry::eStateKind::Normal);
        CHECK(begin != nullptr);
        CHECK(root != nullptr);
        begin->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QString(), root->getId(), SMTransitionEntry::eTransitionKind::Initial);

        SMAttributeSet* set = new SMAttributeSet();
        set->setAttribute("Count");
        set->setSource(SMArgumentEntry::eValueSource::Expression);
        set->setExpression(body);
        root->getEntryList().addOperation(set);

        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, "Go");
        SMConditionEntry* row = trans->getConditions().addCondition();
        row->setLhsKind(SMArgumentEntry::eValueSource::Expression);
        row->setExpression(expr);

        const QString outPath = outFile("sm02_cdata.fsml");
        CHECK(doc.writeToFile(outPath));

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());

        MethodEntry* rcond = reread.getMethods().findMethod("Check");
        CHECK(rcond != nullptr);
        CHECK((rcond != nullptr) && (rcond->getBody() == body));

        SMStateEntry* rroot = reread.getStates().findState("Root");
        CHECK(rroot != nullptr);
        CHECK((rroot != nullptr) && (rroot->getEntryList().getCount() == 1));
        if ((rroot != nullptr) && (rroot->getEntryList().getCount() == 1))
        {
            SMOperationBase* op = rroot->getEntryList().at(0);
            CHECK(op->getOperationType() == SMOperationBase::eOperation::AttributeSet);
            CHECK(static_cast<SMAttributeSet*>(op)->getExpression() == body);
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
        doc.getMethods().createMethod("Go", NEMethod::SmTrigger);

        // `Root` carries behaviour, so it is an ordinary state: a Kind="Start" is a pseudo-state
        // and the read shim would rewrite a merged-form one on the next load.
        SMStateEntry* begin = doc.getStates().createState("Begin", SMStateEntry::eStateKind::Start);
        SMStateEntry* root  = doc.getStates().createState("Root", SMStateEntry::eStateKind::Normal);
        CHECK(begin != nullptr);
        CHECK(root != nullptr);
        begin->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QString(), root->getId(), SMTransitionEntry::eTransitionKind::Initial);
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
        doc.getMethods().createMethod("Go", NEMethod::SmTrigger);

        // `Root` carries behaviour, so it is an ordinary state: a Kind="Start" is a pseudo-state
        // and the read shim would rewrite a merged-form one on the next load.
        SMStateEntry* begin = doc.getStates().createState("Begin", SMStateEntry::eStateKind::Start);
        SMStateEntry* root  = doc.getStates().createState("Root", SMStateEntry::eStateKind::Normal);
        CHECK(begin != nullptr);
        CHECK(root != nullptr);
        begin->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QString(), root->getId(), SMTransitionEntry::eTransitionKind::Initial);
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

        AttributeEntry* attr = doc.getAttributes().createAttribute("mCount");
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

        MethodEntry* m = doc.getMethods().createMethod("Go", NEMethod::SmTrigger);
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

        AttributeEntry* rattr = reread.getAttributes().findElement("mCount");
        CHECK((rattr != nullptr) && rattr->getIsDeprecated() && (rattr->getDeprecateHint() == "use mTotal"));

        SMEventEntry* rev = reread.getEvents().findEvent("Started");
        CHECK((rev != nullptr) && rev->getIsDeprecated() && (rev->getDeprecateHint() == "replaced by Ready"));
        if (rev != nullptr)
        {
            MethodParameter* rp = rev->findElement("code");
            CHECK((rp != nullptr) && rp->getIsDeprecated() && (rp->getDeprecateHint() == "unused payload"));
        }

        MethodEntry* rm = reread.getMethods().findMethod("Go");
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
        CHECK(replaceOnce(older, "FormatVersion=\"1.1.0\"", "FormatVersion=\"0.9.0\""));

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

    //!< A newer minor used to be tolerated. It is refused now (owner ruling 2026-08-09): an
    //!< element this build has never heard of cannot be edited, and saving it back would be
    //!< the way its meaning is lost.
    void testRejectNewerMinor()
    {
        std::printf("[SM-03] newer minor is refused\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        QByteArray future = original;
        CHECK(replaceOnce(future,
                          "<StateMachine FormatVersion=\"1.1.0\">",
                          "<StateMachine FormatVersion=\"1.2.0\">"));

        const QString inPath = outFile("sm03_future_minor_in.fsml");
        CHECK(writeAllBytes(inPath, future));

        StateMachineData doc;
        CHECK(doc.readFromFile(inPath) == false);
        CHECK(doc.openSucceeded() == false);
    }

    //!< A document of a version this build reads keeps every element the format does not
    //!< define, wherever it sits. The block is reported as an error, and it still survives the
    //!< save: that is what lets the author take the document to a build that understands it.
    void testUnknownPreservation()
    {
        std::printf("[SM-03] unknown elements survive a save, nested and at the root\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        QByteArray odd = original;
        CHECK(replaceOnce(odd, "<TimerList>", "<TimerList>\n        <Time ID=\"901\" Name=\"Blink\"/>"));
        CHECK(replaceOnce(odd,
                          "</StateMachine>",
                          "    <FutureSection Flag=\"x\"><FutureLeaf Value=\"42\"/></FutureSection>\n</StateMachine>"));

        const QString inPath = outFile("sm03_unknown_in.fsml");
        CHECK(writeAllBytes(inPath, odd));

        StateMachineData doc;
        CHECK(doc.readFromFile(inPath));
        CHECK(doc.openSucceeded());

        // Both are reported, and the nested one carries the line the author has to go to. The
        // leaf inside FutureSection is not reported separately: the whole block is one finding.
        CHECK(doc.getUnknownElements().size() == 2);    // Time, FutureSection
        bool foundTime = false;
        for (const DocUnknownElement& unknown : doc.getUnknownElements())
        {
            if (unknown.name == QString("Time"))
            {
                foundTime = true;
                CHECK(unknown.parent == QString("TimerList"));
                CHECK(unknown.line > 0);
            }
        }
        CHECK(foundTime);

        const QString outPath = outFile("sm03_unknown_out.fsml");
        CHECK(doc.writeToFile(outPath));

        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("<Time "));
        CHECK(written.contains("<FutureSection"));
        CHECK(written.contains("<FutureLeaf"));
    }

    void testRejectNewerMajor()
    {
        std::printf("[SM-03] newer major is refused with both versions in the message\n");

        const QByteArray original = readAllBytes(dataFile("TrafficLight.fsml"));
        CHECK(original.isEmpty() == false);

        QByteArray newerMajor = original;
        CHECK(replaceOnce(newerMajor, "FormatVersion=\"1.1.0\"", "FormatVersion=\"2.0.0\""));

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
        // and, crucially, must terminate (no infinite loop -- reaching here proves it).
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

    // A structural fuzz sweep on top of the byte-level one above: attribute values replaced by
    // nonsense of the wrong shape, duplicated IDs, and references to elements that do not exist.
    // None of these is a programming error, so none may assert, hang, or crash.
    void testHostileAttributes()
    {
        std::printf("[SM-27] hostile attribute values and illegal references terminate cleanly\n");

        const QByteArray original = readAllBytes(dataFile("FullFeature.fsml"));
        CHECK(original.isEmpty() == false);

        struct Mutation
        {
            const char* find;
            const char* replace;
        };

        const Mutation mutations[] =
        {
            { "Type=\"Enumeration\"",           "Type=\"NoSuchKind\""            },  // unknown data-type kind
            { "Type=\"Container\"",             "Type=\"\""                      },  // empty data-type kind
            { "ID=\"11\"",                      "ID=\"13\""                      },  // duplicated element ID
            { "ID=\"40\"",                      "ID=\"notanumber\""              },  // non-numeric ID
            { "To=\"43\"",                      "To=\"9999\""                    },  // target that does not exist
            { "To=\"60\"",                      "To=\"\""                        },  // empty target
            { "StimulusKind=\"Trigger\"",       "StimulusKind=\"Rumor\""         },  // unknown stimulus kind
            { "Stimulus=\"power_on\"",          "Stimulus=\"NeverDeclared\""     },  // undeclared stimulus
            { "Kind=\"Start\"",                 "Kind=\"Pseudo\""                },  // unknown state kind
            { "History=\"Shallow\"",            "History=\"Sideways\""           },  // unknown history mode
            { "Source=\"Constant\"",            "Source=\"Telepathy\""           },  // unknown value source
            { "Implement=\"Handler\"",          "Implement=\"Magic\""            },  // unknown condition body kind
            { "Timeout=\"30000\"",              "Timeout=\"-1\""                 },  // negative timeout
            { "Timeout=\"500\"",                "Timeout=\"99999999999999999999\"" },  // overflowing timeout
            { "Threading=\"Shared\"",           "Threading=\"Quantum\""          },  // unknown threading mode
            { "Version=\"2.1.0\"",              "Version=\"not.a.version\""      },  // malformed user version
            { "op=\"ge\"",                      "op=\"approximately\""           },  // unknown guard operator
            { "state=\"ok\"",                   "state=\"perfect\""              },  // unknown guard state
            { "id=\"11\"",                      "id=\"0\""                       },  // guard reference to nothing
            { "GridSize=\"16\"",                "GridSize=\"-8\""                },  // negative grid size
            { "Owner=\"43\"",                   "Owner=\"nope\""                 },  // non-numeric layout owner
            { "Bulge=\"0.35\"",                 "Bulge=\"NaN\""                  },  // non-finite bulge
            { "Level=\"1\"",                    "Level=\"4294967295\""           },  // level that does not exist
        };

        const QString hostilePath = outFile("sm27_hostile.fsml");
        for (const Mutation& mutation : mutations)
        {
            QByteArray mutated = original;
            CHECK(replaceOnce(mutated, mutation.find, mutation.replace));
            CHECK(writeAllBytes(hostilePath, mutated));

            // The guarantee is termination without a crash; whether the document opens is up
            // to the individual reader, and a rejected document must stay rejected.
            StateMachineData doc;
            const bool opened = doc.readFromFile(hostilePath);
            CHECK(opened == doc.openSucceeded());

            // Whatever survived must still be writable and re-readable -- a half-parsed
            // document may never take the writer down with it.
            const QString echoPath = outFile("sm27_hostile_echo.fsml");
            if (doc.writeToFile(echoPath))
            {
                StateMachineData echo;
                echo.readFromFile(echoPath);
            }
        }

        std::printf("  [OK] %d hostile mutations terminated\n", static_cast<int>(std::size(mutations)));
    }

    // Transitions stored their target by name before SM-26. Reading such a document must bind
    // the name to the target's ID, not silently turn every external transition into an internal
    // one -- and the resolution has to survive a forward reference.
    void testLegacyTargetByName()
    {
        std::printf("[SM-27] pre-SM-26 by-name transition targets resolve to IDs\n");

        const QByteArray legacy =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<StateMachine FormatVersion=\"1.0.0\">\n"
            "    <Overview ID=\"1\" Name=\"Legacy\" Version=\"1.0.0\" Threading=\"Local\"/>\n"
            "    <MethodList>\n"
            "        <Method ID=\"2\" Name=\"Go\" MethodType=\"Trigger\"/>\n"
            "    </MethodList>\n"
            "    <StateList>\n"
            "        <State ID=\"3\" Name=\"First\" Kind=\"Start\">\n"
            "            <TransitionList>\n"
            "                <Transition ID=\"4\" StimulusKind=\"Trigger\" Stimulus=\"Go\" To=\"Second\"/>\n"
            "                <Transition ID=\"5\" StimulusKind=\"Trigger\" Stimulus=\"Go\" To=\"Nowhere\"/>\n"
            "                <Transition ID=\"6\" StimulusKind=\"Trigger\" Stimulus=\"Go\"/>\n"
            "            </TransitionList>\n"
            "        </State>\n"
            "        <State ID=\"7\" Name=\"Second\" Kind=\"Normal\"/>\n"
            "    </StateList>\n"
            "</StateMachine>\n";

        const QString legacyPath = outFile("sm27_legacy_target.fsml");
        CHECK(writeAllBytes(legacyPath, legacy));

        StateMachineData doc;
        CHECK(doc.readFromFile(legacyPath));

        const SMStateEntry* first = doc.findState(QStringLiteral("First"));
        const SMStateEntry* second = doc.findState(QStringLiteral("Second"));
        CHECK(first != nullptr);
        CHECK(second != nullptr);

        const SMTransitionEntry* forward = doc.findTransitionById(4u);
        const SMTransitionEntry* dangling = doc.findTransitionById(5u);
        const SMTransitionEntry* internal = doc.findTransitionById(6u);
        CHECK(forward != nullptr);
        CHECK((forward != nullptr) && (second != nullptr) && (forward->getToId() == second->getId()));
        CHECK((dangling != nullptr) && (dangling->getToId() == 0u));   // unknown name -> internal
        CHECK((internal != nullptr) && (internal->getToId() == 0u));   // no To at all -> internal

        // The next save writes the ID, so the name form is a one-way migration.
        const QString migratedPath = outFile("sm27_legacy_target_saved.fsml");
        CHECK(doc.writeToFile(migratedPath));
        const QByteArray migrated = readAllBytes(migratedPath);
        CHECK(migrated.contains("To=\"Second\"") == false);
        CHECK(migrated.contains("To=\"7\""));
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

        // A fresh document deliberately persists NO overview View entry (see
        // StateMachineData::createNewDocument): the Design page anchors the first view to the
        // top-left content, since a stored center of 0;0 would push the lone Start state into the
        // middle of the viewport. The default layout is just the Start node.
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
// History mode: the attribute round-trips on both composite flavours, is absent
// at the default, and a resave stays byte-identical (SM-28).
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testHistoryModes()
    {
        std::printf("[SM-28] history modes round-trip on painted and imported composites\n");

        StateMachineData doc;
        doc.getOverview().setName("HistoryModes");
        if (IncludeEntry* import = doc.getIncludes().createInclude("./Lib.fsml"))
        {
            import->setAlias("Lib");
        }

        doc.getStates().createState("Idle", SMStateEntry::eStateKind::Start);

        SMStateEntry* painted = doc.getStates().createState("Painted", SMStateEntry::eStateKind::Normal);
        SMStateData* nested = painted->getOrCreateNestedStates();
        nested->createState("SubStart", SMStateEntry::eStateKind::Start);
        nested->createState("Work", SMStateEntry::eStateKind::Normal);
        painted->setHistory(SMStateEntry::eHistory::Shallow);

        SMStateEntry* hosted = doc.getStates().createState("Hosted", SMStateEntry::eStateKind::Normal);
        hosted->setSubmachine("Lib");
        hosted->setHistory(SMStateEntry::eHistory::Deep);

        SMStateEntry* plain = doc.getStates().createState("Plain", SMStateEntry::eStateKind::Normal);
        CHECK(plain->getHistory() == SMStateEntry::eHistory::None);

        const QString outPath = outFile("sm28_history.fsml");
        CHECK(doc.writeToFile(outPath));

        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("History=\"Shallow\""));
        CHECK(written.contains("History=\"Deep\""));
        CHECK(written.count("History=") == 2);          // the default writes nothing

        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        CHECK(reread.openSucceeded());
        CHECK(reread.getStates().findState("Painted")->getHistory() == SMStateEntry::eHistory::Shallow);
        CHECK(reread.getStates().findState("Hosted")->getHistory() == SMStateEntry::eHistory::Deep);
        CHECK(reread.getStates().findState("Plain")->getHistory() == SMStateEntry::eHistory::None);

        const QString outPath2 = outFile("sm28_history_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        CHECK(readAllBytes(outPath) == readAllBytes(outPath2));
    }
}

//////////////////////////////////////////////////////////////////////////
// L1: the legacy merged Kind="Start" is rewritten on load
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< The state \p transition targets, or nullptr.
    const SMStateEntry* targetOf(const StateMachineData& doc, const SMTransitionEntry& transition)
    {
        return doc.findStateById(transition.getToId());
    }

    void testLegacyMergedStart()
    {
        std::printf("[L1] the legacy merged Kind=\"Start\" converts on load and saves corrected\n");

        StateMachineData doc;
        CHECK(doc.readFromFile(dataFile("LegacyStart.fsml")));
        CHECK(doc.openSucceeded());

        // (b) the old state keeps everything it carried, and is now Normal.
        const SMStateEntry* idle = doc.findState(QStringLiteral("Idle"));
        CHECK(idle != nullptr);
        CHECK((idle != nullptr) && (idle->getKind() == SMStateEntry::eStateKind::Normal));
        CHECK((idle != nullptr) && (idle->getEntryList().getOperations().size() == 2));
        CHECK((idle != nullptr) && (idle->getExitList().getOperations().size() == 1));
        CHECK((idle != nullptr) && (idle->getTransitions().getElementCount() == 1));
        CHECK((idle != nullptr) && (idle->getTransitions().getElements().at(0)->getStimulus() == QStringLiteral("begin")));

        // (a) a pseudo-state was inserted at that level, (c) wired by ONE unguarded initial
        // transition that names no stimulus, and it sits in front of its target in document order.
        const SMStateEntry* rootStart = doc.getStates().getStartState();
        CHECK(rootStart != nullptr);
        CHECK((rootStart != nullptr) && (rootStart->getName() == QStringLiteral("Start")));
        CHECK((rootStart != nullptr) && (rootStart->hasOperations() == false));
        CHECK((rootStart != nullptr) && (rootStart->getTransitions().getElementCount() == 1));
        CHECK(doc.getStates().getElements().at(0) == rootStart);
        if ((rootStart != nullptr) && (rootStart->getTransitions().getElementCount() == 1))
        {
            const SMTransitionEntry* initial = rootStart->getTransitions().getElements().at(0);
            CHECK(initial->getStimulus().isEmpty());
            CHECK(initial->hasCondition() == false);
            CHECK(targetOf(doc, *initial) == idle);
        }

        // The nested level was in the merged form too, and gets its own uniquely named pseudo-state.
        const SMStateEntry* running = doc.findState(QStringLiteral("Running"));
        CHECK(running != nullptr);
        const SMStateData* nested = (running != nullptr ? running->getNestedStates() : nullptr);
        CHECK(nested != nullptr);
        const SMStateEntry* polling = doc.findState(QStringLiteral("Polling"));
        CHECK((polling != nullptr) && (polling->getKind() == SMStateEntry::eStateKind::Normal));
        if (nested != nullptr)
        {
            const SMStateEntry* innerStart = nested->getStartState();
            CHECK(innerStart != nullptr);
            CHECK((innerStart != nullptr) && (innerStart->getName() == QStringLiteral("Start1")));   // unique document-wide
            CHECK(nested->getElements().at(0) == innerStart);
            CHECK((innerStart != nullptr) && (innerStart->getTransitions().getElementCount() == 1));
            if ((innerStart != nullptr) && (innerStart->getTransitions().getElementCount() == 1))
            {
                CHECK(targetOf(doc, *innerStart->getTransitions().getElements().at(0)) == polling);
            }
        }

        // (d) each pseudo-state's node sits above its target.
        if ((rootStart != nullptr) && (idle != nullptr))
        {
            const SMLayoutNode* pseudoNode = doc.getLayout().findNode(rootStart->getId());
            const SMLayoutNode* targetNode = doc.getLayout().findNode(idle->getId());
            CHECK(pseudoNode != nullptr);
            CHECK(targetNode != nullptr);
            CHECK((pseudoNode != nullptr) && (targetNode != nullptr) && (pseudoNode->y < targetNode->y));
        }

        // The conversion is lossless in both directions of a save: the next save writes the
        // corrected form, and reloading THAT changes nothing -- the shim is one-way, not churn.
        const QString outPath = outFile("l1_legacystart.fsml");
        CHECK(doc.writeToFile(outPath));
        const QByteArray written = readAllBytes(outPath);
        CHECK(written.contains("Name=\"Idle\" Kind=\"Normal\""));
        CHECK(written.contains("Name=\"Start\" Kind=\"Start\""));
        CHECK(written.contains("Name=\"Start1\" Kind=\"Start\""));
        CHECK(written.contains("Name=\"Polling\" Kind=\"Normal\""));

        StateMachineData again;
        CHECK(again.readFromFile(outPath));
        const QString twicePath = outFile("l1_legacystart_twice.fsml");
        CHECK(again.writeToFile(twicePath));
        CHECK(readAllBytes(twicePath) == written);
    }

    void testTransitionKind()
    {
        std::printf("[L2] the transition Kind round-trips, and a document without it converts\n");

        // 1. Each of the three kinds survives a save and a load, in a document built in memory.
        {
            StateMachineData doc;
            SMStateEntry* start = doc.getStates().createState(QStringLiteral("Start"), SMStateEntry::eStateKind::Start);
            SMStateEntry* idle  = doc.getStates().createState(QStringLiteral("Idle"), SMStateEntry::eStateKind::Normal);
            SMStateEntry* work  = doc.getStates().createState(QStringLiteral("Work"), SMStateEntry::eStateKind::Normal);
            doc.getMethods().createMethod(QStringLiteral("Begin"), NEMethod::SmTrigger);
            doc.getMethods().createMethod(QStringLiteral("Poke"), NEMethod::SmTrigger);

            const uint32_t initialId = start->getTransitions().createTransition(
                    SMTransitionEntry::eStimulusKind::Trigger, QString(), idle->getId()
                    , SMTransitionEntry::eTransitionKind::Initial)->getId();
            const uint32_t externalId = idle->getTransitions().createTransition(
                    SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("Begin"), work->getId()
                    , SMTransitionEntry::eTransitionKind::External)->getId();
            const uint32_t internalId = work->getTransitions().createTransition(
                    SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("Poke"), 0u
                    , SMTransitionEntry::eTransitionKind::Internal)->getId();

            const QString outPath = outFile("l2_kinds.fsml");
            CHECK(doc.writeToFile(outPath));
            const QByteArray written = readAllBytes(outPath);
            // Written explicitly, at the default too: it is what keeps an unconnected external
            // edge and an internal transition from being the same bytes again.
            CHECK(written.contains("Kind=\"Initial\" To="));
            CHECK(written.contains("Kind=\"External\" StimulusKind=\"Trigger\" Stimulus=\"Begin\" To="));
            CHECK(written.contains("Kind=\"Internal\" StimulusKind=\"Trigger\" Stimulus=\"Poke\""));
            // The initial transition carries no stimulus attributes at all -- the two placeholders
            // the format used to demand of it are gone.
            CHECK(written.contains("Kind=\"Initial\" StimulusKind") == false);

            StateMachineData back;
            CHECK(back.readFromFile(outPath));
            CHECK(back.findTransitionById(initialId) != nullptr);
            CHECK((back.findTransitionById(initialId) != nullptr) && back.findTransitionById(initialId)->isInitial());
            CHECK((back.findTransitionById(initialId) != nullptr) && back.findTransitionById(initialId)->hasTarget());
            CHECK((back.findTransitionById(initialId) != nullptr) && back.findTransitionById(initialId)->getStimulus().isEmpty());
            CHECK((back.findTransitionById(externalId) != nullptr) && back.findTransitionById(externalId)->isExternal());
            CHECK((back.findTransitionById(externalId) != nullptr) && back.findTransitionById(externalId)->hasTarget());
            CHECK((back.findTransitionById(internalId) != nullptr) && back.findTransitionById(internalId)->isInternal());
            CHECK((back.findTransitionById(internalId) != nullptr) && (back.findTransitionById(internalId)->hasTarget() == false));
            CHECK((back.findTransitionById(internalId) != nullptr) && (back.findTransitionById(internalId)->getStimulus() == QStringLiteral("Poke")));

            // An UNCONNECTED external transition survives as one. Before Kind this was the whole
            // problem: saved and reloaded, it came back as an internal transition -- a half-drawn
            // edge silently turned into behaviour.
            SMTransitionEntry* dangling = work->getTransitions().createTransition(
                    SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("Begin"), 0u
                    , SMTransitionEntry::eTransitionKind::External);
            const uint32_t danglingId = dangling->getId();
            const QString danglingPath = outFile("l2_dangling.fsml");
            CHECK(doc.writeToFile(danglingPath));

            StateMachineData reread;
            CHECK(reread.readFromFile(danglingPath));
            CHECK((reread.findTransitionById(danglingId) != nullptr) && reread.findTransitionById(danglingId)->isExternal());
            CHECK((reread.findTransitionById(danglingId) != nullptr) && (reread.findTransitionById(danglingId)->hasTarget() == false));
        }

        // 2. A document written before the attribute existed converts on load: the meaning is
        //    recovered from the absences that used to carry it, and the next save states it.
        {
            StateMachineData doc;
            CHECK(doc.readFromFile(dataFile("LegacyKind.fsml")));
            CHECK(doc.openSucceeded());

            const SMTransitionEntry* initial  = doc.findTransitionById(23u);
            const SMTransitionEntry* external = doc.findTransitionById(22u);
            const SMTransitionEntry* internal = doc.findTransitionById(25u);
            CHECK(initial != nullptr);
            CHECK(external != nullptr);
            CHECK(internal != nullptr);
            // Owned by a Kind="Start" and naming no stimulus -> Initial, placeholders dropped.
            CHECK((initial != nullptr) && initial->isInitial());
            CHECK((initial != nullptr) && initial->getStimulus().isEmpty());
            CHECK((initial != nullptr) && (initial->getToId() == 21u));
            // A To -> External. No To -> Internal.
            CHECK((external != nullptr) && external->isExternal());
            CHECK((internal != nullptr) && internal->isInternal());
            CHECK((internal != nullptr) && (internal->getOperations().getOperations().size() == 1));

            const QString outPath = outFile("l2_legacykind.fsml");
            CHECK(doc.writeToFile(outPath));
            const QByteArray written = readAllBytes(outPath);
            CHECK(written.contains("<Transition ID=\"23\" Kind=\"Initial\" To=\"21\"/>"));
            CHECK(written.contains("Stimulus=\"\"") == false);

            // One-way, not per-save churn: reloading the corrected form and saving it again is
            // byte-identical.
            StateMachineData again;
            CHECK(again.readFromFile(outPath));
            const QString twicePath = outFile("l2_legacykind_twice.fsml");
            CHECK(again.writeToFile(twicePath));
            CHECK(readAllBytes(twicePath) == written);
        }
    }

    void testConditionSourceMigration()
    {
        std::printf("- argument Source=\"Condition\" migrates to an invalid mapping\n");

        // An older document that still names a condition as an argument value source loads with the
        // argument marked invalid; the bound value is kept, and nothing is silently converted.
        QString in = QStringLiteral("<Argument ID=\"7\" Name=\"x\" Source=\"Condition\" Value=\"IsReady\"/>");
        QXmlStreamReader reader(in);
        reader.readNextStartElement();
        SMArgumentEntry arg;
        CHECK(arg.readFromXml(reader));
        CHECK(arg.getSource() == SMArgumentEntry::eValueSource::Invalid);
        CHECK(arg.getValue() == QStringLiteral("IsReady"));

        // It saves with its marker and reopens invalid, so the error survives a save/load cycle.
        QString out;
        QXmlStreamWriter writer(&out);
        arg.writeToXml(writer);
        QXmlStreamReader back(out);
        back.readNextStartElement();
        SMArgumentEntry reloaded;
        CHECK(reloaded.readFromXml(back));
        CHECK(reloaded.getSource() == SMArgumentEntry::eValueSource::Invalid);
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    std::printf("==== SM serialization/versioning tests ====\n");

    testRoundTrip();
    testFullFeatureRoundTrip();
    testLegacyImportMigration();
    testLayoutLogicSeparation();
    testCData();
    testNestedConditions();
    testFlatGuardStaysLegacy();
    testDeprecation();
    testRobustness();
    testHostileAttributes();
    testLegacyTargetByName();
    testVersionMigration();
    testRejectNewerMinor();
    testUnknownPreservation();
    testRejectNewerMajor();
    testNewDocumentSkeleton();
    testAutosaveHelpers();
    testEphemeralSubmachine();
    testHistoryModes();
    testLegacyMergedStart();
    testTransitionKind();
    testConditionSourceMigration();

    std::printf("---- %d checks, %d failure(s) ----\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
