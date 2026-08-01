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
 *  \file        tests/sm/SML3ProofTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Task L3 acceptance: the `Do` activity is a timer loop and nothing else, and its
 *               `Until` stop condition is the same ID-bound guard tree every other predicate in
 *               the format already is.
 *
 *               Three things are proved, one per section, in the order the task asks for them:
 *
 *                 1. `Interval="0"` and an ABSENT `Interval` are both refused -- the trigger-driven
 *                    mode is gone, and neither spelling of "no period" is quietly accepted.
 *                 2. A legacy `Until` STRING converts to a tree and writes back as one; a string
 *                    the grammar has no node for survives whole as a single `Raw`.
 *                 3. Renaming an attribute an `Until` references updates what the tree renders and
 *                    does not break the binding -- the reason the change was made at all.
 *
 *               Headless: the data layer, the guard model and the validator. No widgets.
 *
 ************************************************************************/

#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/SMValidator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>

#include <cstdio>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int gChecks   { 0 };
    int gFailures { 0 };

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }
}

#define CHECK(cond)  check((cond), #cond)

namespace
{
    using eKind  = SMStateEntry::eStateKind;
    using eTrans = SMTransitionEntry::eTransitionKind;
    using eStim  = SMTransitionEntry::eStimulusKind;

    QString gOutDir;

    QString outFile(const QString& name)
    {
        return QDir(gOutDir).filePath(name);
    }

    int countRule(const QList<SMIssue>& issues, int rule)
    {
        int n = 0;
        for (const SMIssue& issue : issues)
        {
            if (issue.rule == rule)
            {
                ++n;
            }
        }

        return n;
    }

    /**
     * \brief   A minimal but VALID machine: a pseudo-start wired to one Normal state that owns a
     *          Do activity of one operation. Everything each section needs beyond that -- the
     *          period, the stop condition -- it sets itself, so a failure names one cause.
     **/
    SMStateEntry* buildMachine(StateMachineData& doc, const QString& name)
    {
        doc.getOverview().setName(name);
        SMStateEntry* start = doc.getStates().createState("Begin", eKind::Start);
        SMStateEntry* run   = doc.getStates().createState("Running", eKind::Normal);
        start->getTransitions().createTransition(eStim::Trigger, QString(), run->getId(), eTrans::Initial);

        SMInlineCode* tick = new SMInlineCode();
        tick->setBody("poll();");
        run->getDoList().addOperation(tick);
        return run;
    }

    //!< Writes \p text to \p path verbatim -- the hand-authored legacy documents of section 2.
    bool writeText(const QString& path, const QString& text)
    {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
        {
            return false;
        }

        file.write(text.toUtf8());
        file.close();
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////
// 1. Interval="0" and an absent Interval are both refused
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testIntervalRefused()
    {
        std::printf("[L3.1] Interval=\"0\" and an absent Interval are both refused\n");

        // A state created in the editor is valid the moment it exists: the activity gets the
        // default period, not a 0 the author never typed.
        StateMachineData fresh;
        SMStateEntry* ok = buildMachine(fresh, "Fresh");
        CHECK(ok->getDoInterval() == SMStateEntry::DEFAULT_DO_INTERVAL);
        CHECK(countRule(SMValidator::validate(fresh), SMValidator::RULE_DO_ACTIVITY) == 0);

        // The removed trigger-driven mode. It is NOT converted -- there is no way to guess which
        // stimulus was meant -- it is refused, and the state is named.
        StateMachineData zero;
        SMStateEntry* triggerDriven = buildMachine(zero, "TriggerDriven");
        triggerDriven->setDoInterval(0u);
        const QList<SMIssue> zeroIssues = SMValidator::validate(zero);
        CHECK(countRule(zeroIssues, SMValidator::RULE_DO_ACTIVITY) == 1);
        for (const SMIssue& issue : zeroIssues)
        {
            if (issue.rule == SMValidator::RULE_DO_ACTIVITY)
            {
                CHECK(issue.elementId == triggerDriven->getId());
                CHECK(issue.kind == eDocElementKind::State);
                CHECK(issue.severity == SMIssue::eSeverity::Error);
                CHECK(issue.message.contains(QStringLiteral("Running")));
                // The message says where the behaviour went, because the author has to re-author
                // the activity and the internal transition is what they have to re-author it AS.
                CHECK(issue.message.contains(QStringLiteral("internal transition")));
            }
        }

        // An absent Interval is the same fault: a Do that does not know when to run. A document
        // that omits it reads back as 0 -- the reader never invents a period.
        const QString legacy = QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<StateMachine FormatVersion=\"1.1.0\">\n"
            "    <Overview ID=\"1\" Name=\"NoInterval\" Version=\"1.0.0\"/>\n"
            "    <StateList>\n"
            "        <State ID=\"2\" Name=\"Begin\" Kind=\"Start\">\n"
            "            <TransitionList>\n"
            "                <Transition ID=\"3\" Kind=\"Initial\" To=\"4\"/>\n"
            "            </TransitionList>\n"
            "        </State>\n"
            "        <State ID=\"4\" Name=\"Running\" Kind=\"Normal\">\n"
            "            <DoList>\n"
            "                <InlineCode ID=\"5\"><Body><![CDATA[poll();]]></Body></InlineCode>\n"
            "            </DoList>\n"
            "        </State>\n"
            "    </StateList>\n"
            "</StateMachine>\n");

        const QString legacyPath = outFile("l3_no_interval.fsml");
        CHECK(writeText(legacyPath, legacy));

        StateMachineData absent;
        CHECK(absent.readFromFile(legacyPath));
        const SMStateEntry* running = absent.findState(QStringLiteral("Running"));
        CHECK(running != nullptr);
        CHECK((running != nullptr) && (running->getDoInterval() == 0u));
        CHECK(countRule(SMValidator::validate(absent), SMValidator::RULE_DO_ACTIVITY) == 1);

        // The fault survives a save: writing the 0 back is what keeps it visible -- both in the
        // validation panel and against the schema -- until the activity is re-authored.
        const QString resaved = outFile("l3_no_interval_2.fsml");
        CHECK(absent.writeToFile(resaved));
        QFile file(resaved);
        CHECK(file.open(QIODevice::ReadOnly));
        const QByteArray bytes = file.readAll();
        file.close();
        CHECK(bytes.contains("Interval=\"0\""));

        // ... and a period at the minimum is accepted, so the rule fires on the fault and not on
        // every activity.
        StateMachineData minimum;
        buildMachine(minimum, "Minimum")->setDoInterval(SMStateEntry::MIN_DO_INTERVAL);
        CHECK(countRule(SMValidator::validate(minimum), SMValidator::RULE_DO_ACTIVITY) == 0);
    }
}

//////////////////////////////////////////////////////////////////////////
// 2. An Until string converts to a tree and back
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< The load shim, exactly as StateMachineModel runs it (that class is a QObject wired to the
    //!< application; the RULE it applies is what this test is about, so it is applied here).
    void applyUntilShim(StateMachineData& doc, SMStateData& level)
    {
        for (SMStateEntry* state : level.getElements())
        {
            const QString legacy = state->getDoUntilLegacy().trimmed();
            if ((legacy.isEmpty() == false) && state->getDoUntil().isEmpty())
            {
                SMGuard parsed = SMGuardParser::parseToGuard(doc, 0u, legacy, true);
                if (parsed.isOk() == false)
                {
                    parsed = SMGuard();
                    parsed.setTree(SMGuardNode::makeVerbatim(SMGuardNode::eKind::Raw, legacy));
                }

                state->setDoUntil(parsed);
            }

            state->clearDoUntilLegacy();
            if (state->hasNestedStates())
            {
                applyUntilShim(doc, *state->getNestedStates());
            }
        }
    }

    QString legacyDocument(const QString& untilText)
    {
        return QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<StateMachine FormatVersion=\"1.1.0\">\n"
            "    <Overview ID=\"1\" Name=\"LegacyUntil\" Version=\"1.0.0\"/>\n"
            "    <AttributeList>\n"
            "        <Attribute ID=\"10\" Name=\"Credit\" DataType=\"uint32\" Value=\"0\"/>\n"
            "        <Attribute ID=\"11\" Name=\"Maintenance\" DataType=\"bool\" Value=\"false\"/>\n"
            "    </AttributeList>\n"
            "    <StateList>\n"
            "        <State ID=\"2\" Name=\"Begin\" Kind=\"Start\">\n"
            "            <TransitionList>\n"
            "                <Transition ID=\"3\" Kind=\"Initial\" To=\"4\"/>\n"
            "            </TransitionList>\n"
            "        </State>\n"
            "        <State ID=\"4\" Name=\"Running\" Kind=\"Normal\">\n"
            "            <DoList Interval=\"2000\" Until=\"%1\">\n"
            "                <InlineCode ID=\"5\"><Body><![CDATA[poll();]]></Body></InlineCode>\n"
            "            </DoList>\n"
            "        </State>\n"
            "    </StateList>\n"
            "</StateMachine>\n").arg(untilText);
    }

    //!< Loads a hand-authored legacy document and runs the shim; returns the `Running` state.
    SMStateEntry* loadLegacy(StateMachineData& doc, const QString& fileName, const QString& untilText)
    {
        const QString path = outFile(fileName);
        CHECK(writeText(path, legacyDocument(untilText)));
        CHECK(doc.readFromFile(path));
        applyUntilShim(doc, doc.getStates());
        return doc.findState(QStringLiteral("Running"));
    }

    void testUntilConverts()
    {
        std::printf("[L3.2] a legacy Until string converts to a tree and writes back as one\n");

        // (a) A bare attribute name -- the spelling that used to break silently on a rename.
        StateMachineData bare;
        SMStateEntry* running = loadLegacy(bare, "l3_until_bare.fsml", QStringLiteral("Maintenance"));
        CHECK(running != nullptr);
        CHECK((running != nullptr) && running->getDoUntilLegacy().isEmpty());
        CHECK((running != nullptr) && running->getDoUntil().isOk());
        const SMGuardNode* tree = (running != nullptr) ? running->getDoUntil().getTree() : nullptr;
        CHECK(tree != nullptr);
        CHECK((tree != nullptr) && (tree->getKind() == SMGuardNode::eKind::Attr));
        CHECK((tree != nullptr) && (tree->getSymbolId() == 11u));

        // (b) An expression: the operator and both operands are nodes, not text.
        StateMachineData expr;
        SMStateEntry* running2 = loadLegacy(expr, "l3_until_expr.fsml", QStringLiteral("Credit &gt; 0"));
        const SMGuardNode* cmp = (running2 != nullptr) ? running2->getDoUntil().getTree() : nullptr;
        CHECK(cmp != nullptr);
        CHECK((cmp != nullptr) && (cmp->getKind() == SMGuardNode::eKind::Cmp));
        CHECK((cmp != nullptr) && (cmp->getOp() == SMGuardNode::eCmpOp::Gt));
        CHECK((cmp != nullptr) && (cmp->getCount() == 2));
        CHECK((cmp != nullptr) && (cmp->childAt(0) != nullptr) && (cmp->childAt(0)->getSymbolId() == 10u));

        // ... and it renders back to what it came from: the round trip is text -> tree -> text.
        CHECK(SMGuardRender::text(expr, 0u, *cmp) == QStringLiteral("Credit > 0"));

        // (c) Something the grammar has no node for stays WHOLE as a single Raw -- which is what
        // the whole attribute already was. Nothing is dropped and nothing is invented.
        StateMachineData raw;
        SMStateEntry* running3 = loadLegacy(raw, "l3_until_raw.fsml", QStringLiteral("mHelper-&gt;done( )"));
        const SMGuardNode* rawTree = (running3 != nullptr) ? running3->getDoUntil().getTree() : nullptr;
        CHECK(rawTree != nullptr);
        CHECK((rawTree != nullptr) && (rawTree->getKind() == SMGuardNode::eKind::Raw));
        CHECK((rawTree != nullptr) && (rawTree->getText() == QStringLiteral("mHelper->done( )")));

        // The write-back: a tree child, and no `Until=` attribute anywhere in the format.
        const QString outPath = outFile("l3_until_bare_saved.fsml");
        CHECK(bare.writeToFile(outPath));
        QFile file(outPath);
        CHECK(file.open(QIODevice::ReadOnly));
        const QByteArray bytes = file.readAll();
        file.close();
        CHECK(bytes.contains("<Until state=\"ok\">"));
        CHECK(bytes.contains("<Attr id=\"11\""));
        CHECK(bytes.contains("Until=\"") == false);

        // Reloading the saved tree needs no shim at all and yields the same binding.
        StateMachineData reread;
        CHECK(reread.readFromFile(outPath));
        const SMStateEntry* back = reread.findState(QStringLiteral("Running"));
        CHECK(back != nullptr);
        CHECK((back != nullptr) && back->getDoUntilLegacy().isEmpty());
        CHECK((back != nullptr) && back->getDoUntil().isOk());
        CHECK((back != nullptr) && (back->getDoUntil().getTree() != nullptr)
              && (back->getDoUntil().getTree()->getSymbolId() == 11u));

        // A resave is byte-identical: the stop condition is as deterministic as every other tree.
        const QString outPath2 = outFile("l3_until_bare_saved_2.fsml");
        CHECK(reread.writeToFile(outPath2));
        QFile again(outPath2);
        CHECK(again.open(QIODevice::ReadOnly));
        const QByteArray bytes2 = again.readAll();
        again.close();
        CHECK(bytes == bytes2);
    }
}

//////////////////////////////////////////////////////////////////////////
// 3. A rename updates the tree and does not break it
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testRenameFollows()
    {
        std::printf("[L3.3] renaming an attribute an Until references updates it and does not break it\n");

        StateMachineData doc;
        SMStateEntry* running = loadLegacy(doc, "l3_until_rename.fsml", QStringLiteral("Maintenance"));
        CHECK(running != nullptr);

        SMAttributeEntry* attribute = doc.getAttributes().findElement(QStringLiteral("Maintenance"));
        CHECK(attribute != nullptr);

        const SMGuardNode* tree = running->getDoUntil().getTree();
        CHECK(tree != nullptr);
        CHECK(SMGuardRender::text(doc, 0u, *tree) == QStringLiteral("Maintenance"));

        // Where-used sees it: an attribute referenced ONLY by a Do stop condition is referenced,
        // and the delete refusal that protects guard trees now protects this one too.
        const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(doc, attribute->getId());
        CHECK(uses.size() == 1);
        CHECK((uses.isEmpty() == false) && (uses.at(0).target.getOwner() == SMGuardRef::eOwner::DoActivity));
        CHECK((uses.isEmpty() == false) && (uses.at(0).target.getId() == running->getId()));

        // THE rename. Nothing touches the state, the DoList or the tree -- only the declaration.
        attribute->setName(QStringLiteral("ServiceMode"));

        // The tree is unchanged and still bound (this is the whole point: it binds by ID)...
        CHECK(running->getDoUntil().isOk());
        CHECK(running->getDoUntil().getTree() == tree);
        CHECK(tree->getSymbolId() == attribute->getId());

        // ... and everything that displays it now says the new name, with no edit to the Until.
        CHECK(SMGuardRender::text(doc, 0u, *tree) == QStringLiteral("ServiceMode"));

        // Nothing broke: the guard checker reports no fault against the renamed reference. (The
        // free-text spelling would have produced a compile error in generated code instead --
        // naming a symbol no document contains.)
        for (const SMGuardValidation::Finding& finding : SMGuardValidation::validateDoActivity(doc, running->getId()))
        {
            CHECK(finding.severity != SMGuardValidation::eSeverity::Error);
        }

        CHECK(countRule(SMValidator::validate(doc), SMValidator::RULE_GUARD) == 0);

        // The advisory display name written to `.fsml` is refreshed from the id on save, so the
        // file a human reads never carries the old name either.
        SMGuardNode* mutableTree = running->getDoUntil().getTree();
        SMGuardRender::refreshNames(doc, 0u, *mutableTree);
        CHECK(mutableTree->getCacheName() == QStringLiteral("ServiceMode"));

        // And DELETING the attribute is now visible as the broken reference it is, rather than as
        // a string that still reads fine.
        const uint32_t deleted = attribute->getId();
        CHECK(doc.getAttributes().removeElement(QStringLiteral("ServiceMode")));
        bool brokenReported = false;
        for (const SMGuardValidation::Finding& finding : SMGuardValidation::validateDoActivity(doc, running->getId()))
        {
            if ((finding.kind == SMGuardValidation::eKind::BrokenRef) && (finding.symbolId == deleted))
            {
                brokenReported = true;
                CHECK(finding.target.getOwner() == SMGuardRef::eOwner::DoActivity);
                CHECK(finding.message.contains(QStringLiteral("Do stop condition")));
                CHECK(finding.location.contains(QStringLiteral("do/")));
            }
        }

        CHECK(brokenReported);
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    gOutDir = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QDir::temp().filePath("lusan_l3");
    QDir().mkpath(gOutDir);
    std::printf("Task L3 proof -- output directory: %s\n", gOutDir.toLocal8Bit().constData());

    testIntervalRefused();
    testUntilConverts();
    testRenameFollows();

    std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
