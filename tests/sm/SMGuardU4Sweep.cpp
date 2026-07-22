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
 *  \file        tests/sm/SMGuardU4Sweep.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U4) acceptance sweep: items 16-18 and 20-22 end-to-end through the
 *               real widgets + the file + the validation path, the Try-it strip live checks,
 *               and the item-24 light/dark theme grabs of field states E0-E6.
 *
 *               Usage: lusan_sm_guard_u4_sweep <doc.fsml> [grab dir]
 *
 ************************************************************************/

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardEval.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMGuardValidation.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMGuardBar.hpp"
#include "lusan/view/sm/SMGuardCallsOutline.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/view/sm/SMPropertiesPanel.hpp"
#include "lusan/view/sm/SMTryStrip.hpp"
#include "lusan/view/sm/SMValidationPanel.hpp"

#include <QApplication>
#include <QComboBox>
#include "lusan/view/sm/SMAccordion.hpp"
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTabWidget>
#include <QTextCursor>
#include <QToolButton>
#include <cmath>
#include <cstdio>

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

    void checkEq(const QString& actual, const QString& expected, const char* what)
    {
        ++gChecks;
        if (actual != expected)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n         expected: %s\n         actual  : %s\n",
                        what, expected.toStdString().c_str(), actual.toStdString().c_str());
        }
    }

    // WCAG 2.1 relative-luminance contrast, used by the theme-contrast sweep to prove every dark
    // guard token clears AA (>= 4.5:1) against the real app dark editor surfaces.
    double srgbLinear(int channel)
    {
        const double v = channel / 255.0;
        return (v <= 0.03928) ? (v / 12.92) : std::pow((v + 0.055) / 1.055, 2.4);
    }

    double relativeLuminance(const QColor& c)
    {
        return (0.2126 * srgbLinear(c.red())) + (0.7152 * srgbLinear(c.green())) + (0.0722 * srgbLinear(c.blue()));
    }

    double contrastRatio(const QColor& a, const QColor& b)
    {
        const double la = relativeLuminance(a);
        const double lb = relativeLuminance(b);
        const double hi = qMax(la, lb);
        const double lo = qMin(la, lb);
        return (hi + 0.05) / (lo + 0.05);
    }

    void pump(int ms)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < ms)
        {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
        }
    }

    void grab(QWidget* widget, const QString& dir, const char* name)
    {
        const QString path = dir + QLatin1Char('/') + QString::fromLatin1(name);
        if (widget->grab().save(path))
        {
            std::printf("  [SHOT] %s\n", path.toStdString().c_str());
        }
        else
        {
            std::printf("  [WARN] could not save %s\n", path.toStdString().c_str());
        }
    }

    uint32_t firstTransition(const StateMachineData& data)
    {
        for (const SMStateEntry* state : data.getStates().getElements())
        {
            if ((state != nullptr) && (state->getTransitions().getElementCount() > 0))
            {
                return state->getTransitions().getElements().at(0)->getId();
            }
        }

        return 0u;
    }

    void setGuard(StateMachineModel& model, uint32_t transitionId, const QString& text, bool allowRaw = false)
    {
        SMGuard guard = SMGuardParser::parseToGuard(model.getData(), transitionId, text, allowRaw);
        model.getUndoStack().push(SMGuardCommands::setGuard(model.getData(), model.getNotifier(), transitionId, guard, QStringLiteral("set")));
    }

    QString guardText(const StateMachineModel& model, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = model.getData().findTransitionById(transitionId);
        return (transition != nullptr) ? SMGuardRender::guardText(model.getData(), transitionId, transition->getGuard()) : QString();
    }

    QString preview(const StateMachineModel& model, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = model.getData().findTransitionById(transitionId);
        return (transition != nullptr) ? SMGuardCodegenPreview::ifStatement(model.getData(), transitionId, transition->getGuard()) : QString();
    }

    QByteArray readBytes(const QString& path)
    {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }

    bool writeBytes(const QString& path, const QByteArray& bytes)
    {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && (file.write(bytes) == bytes.size());
    }

    //!< The <Expr>...</Expr> block of the saved document (the ID-bound tree).
    QString exprBlock(const QByteArray& bytes)
    {
        const QString text = QString::fromUtf8(bytes);
        const int start = text.indexOf(QStringLiteral("<Expr>"));
        const int end = text.indexOf(QStringLiteral("</Expr>"));
        return ((start >= 0) && (end > start)) ? text.mid(start, end - start) : QString();
    }
}

// ---------------------------------------------------------------------------
// Item 22: the S1-S15 objectName sweep (scripted widget-tree check).
// ---------------------------------------------------------------------------
static void sweepObjectNames(StateMachineModel& model, uint32_t transId, const QString& grabDir)
{
    std::printf("[ RUN  ] item22-objectNames\n");

    // S1..S6 host: the guard bar with the running example committed.
    SMGuardBar bar(model);
    bar.resize(680, 620);
    bar.show();
    bar.setTransition(transId);
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));
    pump(400);

    check(bar.findChild<QWidget*>(QStringLiteral("smGuardClear")) != nullptr, "S1: smGuardClear");
    check(bar.findChild<QWidget*>(QStringLiteral("smGuardHelp")) != nullptr, "S1: smGuardHelp");
    check(bar.findChild<QWidget*>(QStringLiteral("smGuardField")) != nullptr, "S1: smGuardField");
    check(bar.findChild<QWidget*>(QStringLiteral("smGuardStatus")) != nullptr, "S2: smGuardStatus");
    check(bar.findChild<QWidget*>(QStringLiteral("smGuardChips")) != nullptr, "S3: smGuardChips");
    check(bar.findChild<QWidget*>(QStringLiteral("smGuardTry")) != nullptr, "S6: smGuardTry");

    // S4: the island editor (open it for a synthetic island).
    bar.islandEditor()->openFor(model, transId, 0, QStringLiteral("return count >= MIN_WAITING;"));
    pump(100);
    check(bar.findChild<QWidget*>(QStringLiteral("smIslandBody")) != nullptr, "S4: smIslandBody");
    bar.islandEditor()->hide();

    // S7: the completion catalog popup. The old S7 `smSymbolPopup` was retired in SM-21-03; the
    // single top-level completer is now `SMRefCompleter` (objectName smRefCompleter, D-POPUP).
    SMGuardField* field = bar.field();
    field->setFocus();
    field->openCompletion();
    pump(100);
    check(QApplication::activeWindow() != nullptr, "a window is active for the popup");
    QWidget* popup = field->findChild<QWidget*>(QStringLiteral("smRefCompleter"));
    check(popup != nullptr, "S7: smRefCompleter");
    if (popup != nullptr)
    {
        popup->hide();
    }

    // S9 (retired grid -> inline accordion Arguments): the Conditions pickup list + the caret-driven
    // shared Arguments table (SM-21 bug-fix, 2026-07-21).
    check(bar.accordion()->objectName() == QStringLiteral("smGuardAccordion"), "S9: smGuardAccordion");
    check(bar.calls()->insertRowCount() >= 1, "S9: the Conditions pickup list offers condition method(s)");
    pump(150);
    check(bar.args() != nullptr, "S9: the shared Arguments table exists");

    // S6 open: try strip rows.
    bar.tryStrip()->setOpen(true);
    pump(300);
    check(bar.findChild<QWidget*>(QStringLiteral("smTryResult")) != nullptr, "S6: smTryResult");
    check(bar.findChild<QComboBox*>(QStringLiteral("smTryStub_1_0")) != nullptr, "S6: smTryStub_<path>");
    bar.tryStrip()->setOpen(false);
    pump(50);

    // S12 note: the canvas edge label has no objectName in the B-tables (it is a QGraphicsItem);
    // its severity/glyph rule is covered by the edge-label checks in the canvas smoke test
    // and the real-app screenshot.

    // S14: the Methods page kind split is verified by the U3 harness (methodsPage).

    // S15: the validation panel.
    SMValidationPanel panel(model);
    panel.resize(560, 240);
    panel.show();
    panel.refreshNow();
    pump(100);
    check(panel.objectName() == QStringLiteral("smValidation"), "S15: smValidation");
    check(panel.findChild<QWidget*>(QStringLiteral("smValidationList")) != nullptr, "S15: smValidationList");
    panel.hide();

    // S1 tabs: the properties panel hosts [General|Conditions|Actions] as smTransTabs (the
    // Actions tab was added in SM-23; Conditions stays index 1).
    SMPropertiesPanel props(model);
    props.resize(520, 640);
    props.show();
    model.getSelectionModel().setSelection({ transId });
    pump(200);
    QTabWidget* tabs = props.findChild<QTabWidget*>(QStringLiteral("smTransTabs"));
    check(tabs != nullptr, "S1: smTransTabs");
    if (tabs != nullptr)
    {
        check(tabs->count() == 3, "smTransTabs has General + Conditions + Actions");
        props.focusConditions(transId);
        pump(100);
        check(tabs->currentIndex() == 1, "focusConditions lands on the Conditions tab");
        grab(&props, grabDir, "u4-properties-conditions.png");
    }

    props.hide();
    bar.hide();
}

// ---------------------------------------------------------------------------
// Item 16: IDs only in storage; hand-delete <Rendered> -> identical display+preview.
// ---------------------------------------------------------------------------
static void sweepItem16(const QString& docPath, const QString& tmpDir)
{
    std::printf("[ RUN  ] item16-storage\n");

    StateMachineModel model;
    check(model.loadFromFile(docPath), "document loads");
    const uint32_t transId = firstTransition(model.getData());
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));

    const QString fileA = tmpDir + QStringLiteral("/u4-item16-a.fsml");
    check(model.getData().writeToFile(fileA), "document saves");
    const QByteArray bytesA = readBytes(fileA);
    check(bytesA.contains("<Guard"), "storage has a <Guard> element");
    check(bytesA.contains("<Rendered>"), "storage carries the <Rendered> cache");

    const QString expr = exprBlock(bytesA);
    check(expr.isEmpty() == false, "storage has the <Expr> tree");
    check(expr.contains(QStringLiteral("id=")), "the tree references symbols by id");
    check(expr.contains(QStringLiteral("HasWaiting")) == false, "no name-based binding inside the tree (16)");
    check(expr.contains(QStringLiteral("WalkRequested")) == false, "no attribute name inside the tree (16)");

    const QString displayA = guardText(model, transId);
    const QString previewA = preview(model, transId);

    // Hand-delete the <Rendered> line and reload: identical display and preview.
    QString text = QString::fromUtf8(bytesA);
    text.remove(QRegularExpression(QStringLiteral("[^\\n]*<Rendered>.*</Rendered>[^\\n]*\\n")));
    check(text.contains(QStringLiteral("<Rendered>")) == false, "the <Rendered> line was removed by hand");
    const QString fileB = tmpDir + QStringLiteral("/u4-item16-b.fsml");
    check(writeBytes(fileB, text.toUtf8()), "modified document written");

    StateMachineModel reread;
    check(reread.loadFromFile(fileB), "modified document loads");
    const uint32_t transB = firstTransition(reread.getData());
    checkEq(guardText(reread, transB), displayA, "display is identical without <Rendered> (16)");
    checkEq(preview(reread, transB), previewA, "preview is identical without <Rendered> (16)");
}

// ---------------------------------------------------------------------------
// Item 17: triple rename with zero guard edits; delete refused with where-used.
// ---------------------------------------------------------------------------
static void sweepItem17(const QString& docPath, const QString& tmpDir)
{
    std::printf("[ RUN  ] item17-rename\n");

    StateMachineModel model;
    check(model.loadFromFile(docPath), "document loads");
    StateMachineData& data = model.getData();
    const uint32_t transId = firstTransition(data);
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));

    const QString fileA = tmpDir + QStringLiteral("/u4-item17-a.fsml");
    model.getData().writeToFile(fileA);
    const QString exprBefore = exprBlock(readBytes(fileA));

    // Three declaration edits, zero guard edits.
    SMMethodEntry* handler = data.getMethods().findMethod(QStringLiteral("HasWaiting"));
    handler->setName(QStringLiteral("CanPass"));
    model.getNotifier().notifyElementChanged(handler->getId(), eDocElementKind::Method);

    SMMethodEntry* trigger = data.getMethods().findMethod(QStringLiteral("RequestWalk"));
    for (MethodParameter& param : trigger->getElements())
    {
        if (param.getName() == QStringLiteral("count"))
        {
            param.setName(QStringLiteral("num"));
        }
    }
    model.getNotifier().notifyElementChanged(trigger->getId(), eDocElementKind::Method);

    SMAttributeEntry* night = data.getAttributes().findElement(QStringLiteral("IsNightMode"));
    night->setName(QStringLiteral("Night"));
    model.getNotifier().notifyElementChanged(night->getId(), eDocElementKind::Attribute);

    checkEq(guardText(model, transId)
           , QStringLiteral("WalkRequested && (CanPass(num) || num >= MIN_WAITING) && !Night")
           , "the guard re-renders with the three new names (17)");
    check(preview(model, transId).contains(QStringLiteral("handler().CanPass(num)")), "preview shows the renamed call (17)");
    check(preview(model, transId).contains(QStringLiteral("!Night()")), "preview shows the renamed attribute (17)");

    // The stored tree is untouched by the renames.
    const QString fileB = tmpDir + QStringLiteral("/u4-item17-b.fsml");
    model.getData().writeToFile(fileB);
    checkEq(exprBlock(readBytes(fileB)), exprBefore, "the stored <Expr> tree is byte-identical after the renames (17)");

    // Delete refused: the where-used surface names the transition.
    const uint32_t walkId = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
    const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(data, walkId);
    check(uses.size() == 1, "the referenced attribute reports one guard use (17)");
    check((uses.size() == 1) && uses.at(0).location.contains(QStringLiteral("Idle : RequestWalk")), "the where-used entry names the transition (17)");
}

// ---------------------------------------------------------------------------
// Item 18: `param` vs `param()` resolve and preview correctly.
// ---------------------------------------------------------------------------
static void sweepItem18(const QString& docPath)
{
    std::printf("[ RUN  ] item18-paramVsCall\n");

    StateMachineModel model;
    check(model.loadFromFile(docPath), "document loads");
    StateMachineData& data = model.getData();
    const uint32_t transId = firstTransition(data);

    SMMethodEntry* lambda = data.getMethods().createMethod(QStringLiteral("param"), SMMethodEntry::eMethodType::Condition);
    lambda->setImplement(SMMethodEntry::eImplement::Embedded);
    lambda->setBody(QStringLiteral("return true;"));
    data.getMethods().findMethod(QStringLiteral("RequestWalk"))->addParam(QStringLiteral("param"))->setType(QStringLiteral("uint16"));
    model.getNotifier().notifyElementChanged(lambda->getId(), eDocElementKind::Method);

    setGuard(model, transId, QStringLiteral("param() && param > 0"));
    const SMTransitionEntry* transition = data.findTransitionById(transId);
    check(transition->getGuard().isOk(), "'param() && param > 0' resolves (18)");
    const SMGuardNode* tree = transition->getGuard().getTree();
    check((tree != nullptr) && (tree->getKind() == SMGuardNode::eKind::And) && (tree->getCount() == 2), "root is And(2)");
    if ((tree != nullptr) && (tree->getCount() == 2))
    {
        check(tree->childAt(0)->getKind() == SMGuardNode::eKind::Call, "call form binds the lambda (18)");
        check(tree->childAt(1)->getKind() == SMGuardNode::eKind::Cmp, "bare form compares");
        check(tree->childAt(1)->childAt(0)->getKind() == SMGuardNode::eKind::Param, "bare form binds the stimulus parameter (18)");
    }

    check(preview(model, transId).contains(QStringLiteral("mparam()")), "the named lambda previews as the std::function member (18/D7)");
}

// ---------------------------------------------------------------------------
// Item 20: determinism + the draft refusal chain.
// ---------------------------------------------------------------------------
static void sweepItem20(const QString& docPath, const QString& tmpDir)
{
    std::printf("[ RUN  ] item20-determinism\n");

    StateMachineModel model;
    check(model.loadFromFile(docPath), "document loads");
    const uint32_t transId = firstTransition(model.getData());
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));

    // The same document renders/generates identically on repeated runs.
    checkEq(preview(model, transId), preview(model, transId), "the preview is identical across runs (20)");
    const QString fileA = tmpDir + QStringLiteral("/u4-item20-a.fsml");
    const QString fileB = tmpDir + QStringLiteral("/u4-item20-b.fsml");
    check(model.getData().writeToFile(fileA), "first save");
    check(model.getData().writeToFile(fileB), "second save");
    check(readBytes(fileA) == readBytes(fileB), "two saves are byte-identical (20)");

    // A draft: validation ERR; the preview and the Try-it strip refuse cleanly.
    setGuard(model, transId, QStringLiteral("WalkRequsted &&"));
    const SMTransitionEntry* transition = model.getData().findTransitionById(transId);
    check(transition->getGuard().isDraft(), "the unresolved text persisted as a draft (20)");
    check(preview(model, transId).isEmpty(), "the codegen preview refuses a draft (20)");

    bool draftError = false;
    for (const SMGuardValidation::Finding& finding : SMGuardValidation::validate(model.getData()))
    {
        if ((finding.kind == SMGuardValidation::eKind::Draft) && (finding.severity == SMGuardValidation::eSeverity::Error))
        {
            draftError = finding.message.contains(QStringLiteral("generation refuses"));
        }
    }
    check(draftError, "validation lists the draft as ERR / generation refuses (20)");

    SMGuardBar bar(model);
    bar.resize(680, 560);
    bar.show();
    bar.setTransition(transId);
    bar.tryStrip()->setOpen(true);
    pump(300);
    // The Try-it strip is no longer SURFACED on the Conditions tab (SM-21, 2026-07-21: the what-if
    // evaluator moves to a dedicated FSM Play page), so its note cannot be visible here -- but the
    // refusal logic it carries is still built and still refuses, which is what this item checks.
    QLabel* note = bar.findChild<QLabel*>(QStringLiteral("smTryNote"));
    check(note != nullptr, "the Try-it strip refuses a draft with a note (20)");
    check((note != nullptr) && note->text().contains(QStringLiteral("draft"))
         , "the refusal note names the draft as the reason (20)");
    check(bar.tryStrip()->isVisible() == false, "the Try-it strip is not surfaced on the Conditions tab (20)");
    check(bar.findChild<QLabel*>(QStringLiteral("smTryResult")) == nullptr, "no result line is shown for a draft (20)");
    bar.hide();
}

// ---------------------------------------------------------------------------
// Item 21: no silent raw -- error first, explicit raw node on request, audited.
// ---------------------------------------------------------------------------
static void sweepItem21(const QString& docPath, const QString& tmpDir, const QString& grabDir)
{
    std::printf("[ RUN  ] item21-raw\n");

    StateMachineModel model;
    check(model.loadFromFile(docPath), "document loads");
    const uint32_t transId = firstTransition(model.getData());

    SMGuardBar bar(model);
    bar.resize(680, 560);
    bar.show();
    bar.setTransition(transId);
    pump(200);

    // Paste the unresolvable text: an error, not code.
    SMGuardField* field = bar.field();
    field->setFocus();
    field->setPlainText(QStringLiteral("count $ 3"));
    pump(400);      // debounce + analyze
    SMGuardParser::Result direct = SMGuardParser::parse(model.getData(), transId, QStringLiteral("count $ 3"));
    check(direct.hasError(), "the pasted fragment is an error, not code (21)");
    delete direct.tree;

    // The explicit quick-fix stores a <Raw> node.
    field->applyFix(QStringLiteral("raw"), QStringLiteral("count $ 3"));
    pump(300);
    const SMTransitionEntry* transition = model.getData().findTransitionById(transId);
    check(transition->getGuard().isOk(), "'Keep as raw C++' commits the guard (21)");
    bool hasRaw = false;
    if (transition->getGuard().getTree() != nullptr)
    {
        const QList<SMGuardEval::StubSite> sites = SMGuardEval::stubSites(*transition->getGuard().getTree());
        for (const SMGuardEval::StubSite& site : sites)
        {
            hasRaw = hasRaw || (site.kind == SMGuardNode::eKind::Raw);
        }
    }
    check(hasRaw, "the committed tree carries the Raw node (21)");

    const QString fileA = tmpDir + QStringLiteral("/u4-item21.fsml");
    model.getData().writeToFile(fileA);
    check(readBytes(fileA).contains("<Raw>"), "<Raw> is in storage (21)");

    bool audited = false;
    for (const SMGuardValidation::Finding& finding : SMGuardValidation::validate(model.getData()))
    {
        if ((finding.kind == SMGuardValidation::eKind::RawFragment) && (finding.severity == SMGuardValidation::eSeverity::Info))
        {
            audited = finding.message.contains(QStringLiteral("count $ 3"));
        }
    }
    check(audited, "the validation audit lists the raw fragment (21)");

    // The validation panel shows a draft ERR and the raw INFO audit side by side: keep the
    // raw guard on the first transition, give a second transition a draft.
    SMStateEntry* idle = model.getData().findState(QStringLiteral("Idle"));
    SMTransitionEntry* second = idle->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QStringLiteral("Walking"));
    second->getGuard().setDraft(QStringLiteral("WalkRequsted &&"));
    model.getNotifier().notifyElementAdded(second->getId(), eDocElementKind::Transition);

    SMValidationPanel panel(model);
    panel.resize(640, 220);
    panel.show();
    panel.refreshNow();
    pump(150);
    check(panel.list()->count() >= 1, "the validation panel lists the guard findings");
    grab(&panel, grabDir, "u4-validation-panel.png");
    panel.hide();
    bar.hide();
}

// ---------------------------------------------------------------------------
// Try-it live: stubs + value flips update pills and result; sibling winner.
// ---------------------------------------------------------------------------
static void sweepTryIt(const QString& docPath, const QString& grabDir)
{
    std::printf("[ RUN  ] tryIt-live\n");

    StateMachineModel model;
    check(model.loadFromFile(docPath), "document loads");
    StateMachineData& data = model.getData();
    const uint32_t transId = firstTransition(data);
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));

    SMGuardBar bar(model);
    bar.resize(680, 640);
    bar.show();
    bar.setTransition(transId);
    pump(300);

    bar.tryStrip()->setOpen(true);
    pump(300);

    QLabel* result = bar.findChild<QLabel*>(QStringLiteral("smTryResult"));
    check(result != nullptr, "the result line exists");

    // Defaults: WalkRequested declared default is empty -> combo default false.
    const uint32_t walkId = SMGuardSymbols::attributeId(data, QStringLiteral("WalkRequested"));
    QComboBox* walk = bar.findChild<QComboBox*>(QStringLiteral("smTryValue_%1").arg(walkId));
    check(walk != nullptr, "the WalkRequested value combo exists");
    QLineEdit* count = bar.findChild<QLineEdit*>(QStringLiteral("smTryValue_%1").arg(SMGuardSymbols::paramId(data, transId, QStringLiteral("count"))));
    check(count != nullptr, "the count value field exists");
    QComboBox* stub = bar.findChild<QComboBox*>(QStringLiteral("smTryStub_1_0"));
    check(stub != nullptr, "the HasWaiting stub combo exists");

    if ((walk == nullptr) || (count == nullptr) || (stub == nullptr) || (result == nullptr))
    {
        return;
    }

    // WalkRequested=true, count=3 -> TRUE regardless of the stub (my_attribute-style flip).
    walk->setCurrentText(QStringLiteral("true"));
    count->setText(QStringLiteral("3"));
    pump(150);
    check(result->text().contains(QStringLiteral("TRUE")), "true/3 -> the guard is TRUE live");
    grab(&bar, grabDir, "u4-tryit-open.png");

    // Flip WalkRequested -> FALSE; the result flips to FALSE live.
    walk->setCurrentText(QStringLiteral("false"));
    pump(150);
    check(result->text().contains(QStringLiteral("FALSE")), "flipping the attribute updates the result live");

    // count=2 + stub false -> the Or clause decides FALSE.
    walk->setCurrentText(QStringLiteral("true"));
    count->setText(QStringLiteral("2"));
    stub->setCurrentText(QStringLiteral("false"));
    pump(150);
    check(result->text().contains(QStringLiteral("FALSE")), "stub false + count=2 -> FALSE");
    stub->setCurrentText(QStringLiteral("true"));
    pump(150);
    check(result->text().contains(QStringLiteral("TRUE")), "stub true -> TRUE");

    // Sibling winner: a second transition on the same stimulus, lower priority, empty guard.
    SMStateEntry* idle = data.findState(QStringLiteral("Idle"));
    SMTransitionEntry* second = idle->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QStringLiteral("Idle"));
    model.getNotifier().notifyElementAdded(second->getId(), eDocElementKind::Transition);
    pump(300);

    result = bar.findChild<QLabel*>(QStringLiteral("smTryResult"));
    check(result != nullptr, "the result line exists after the sibling appeared");
    if (result != nullptr)
    {
        check(result->text().contains(QStringLiteral("wins over")), "ours TRUE -> wins over the sibling");

        // Make ours FALSE: the empty-guard sibling fires instead.
        walk = bar.findChild<QComboBox*>(QStringLiteral("smTryValue_%1").arg(walkId));
        if (walk != nullptr)
        {
            walk->setCurrentText(QStringLiteral("false"));
            pump(150);
            result = bar.findChild<QLabel*>(QStringLiteral("smTryResult"));
            check((result != nullptr) && result->text().contains(QStringLiteral("fires instead")), "ours FALSE -> the sibling wins (priority)");
        }
    }

    grab(&bar, grabDir, "u4-tryit-sibling.png");
    bar.hide();
}

// ---------------------------------------------------------------------------
// Item 24: both themes render the field states E0-E6 from B15 roles.
// ---------------------------------------------------------------------------
static void sweepThemes(const QString& docPath, const QString& grabDir)
{
    std::printf("[ RUN  ] item24-themes\n");

    struct Theme
    {
        const char* tag;
        bool        dark;
    };

    const Theme themes[] = { { "light", false }, { "dark", true } };
    for (const Theme& theme : themes)
    {
        if (theme.dark)
        {
            // The app's dark theme installs a dark palette; NEGuardStyle keys off it.
            QPalette dark;
            dark.setColor(QPalette::Window, QColor(0x2B, 0x2B, 0x2B));
            dark.setColor(QPalette::WindowText, QColor(0xE0, 0xE0, 0xE0));
            dark.setColor(QPalette::Base, QColor(0x23, 0x23, 0x23));
            dark.setColor(QPalette::Text, QColor(0xE0, 0xE0, 0xE0));
            dark.setColor(QPalette::Button, QColor(0x33, 0x33, 0x33));
            dark.setColor(QPalette::ButtonText, QColor(0xE0, 0xE0, 0xE0));
            QApplication::setPalette(dark);
        }
        else
        {
            QApplication::setPalette(QApplication::style()->standardPalette());
        }

        check(NEGuardStyle::isDark() == theme.dark, theme.dark ? "the dark palette selects the dark tokens" : "the light palette selects the light tokens");

        StateMachineModel model;
        model.loadFromFile(docPath);
        StateMachineData& data = model.getData();
        const uint32_t transId = firstTransition(data);

        SMGuardBar bar(model);
        bar.resize(680, 620);
        bar.show();
        bar.setTransition(transId);
        pump(200);

        char name[64];

        // E0: empty (ghost text).
        setGuard(model, transId, QString());
        pump(350);
        std::snprintf(name, sizeof(name), "u4-%s-e0-empty.png", theme.tag);
        grab(&bar, grabDir, name);

        // E1: resolved example B.
        setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));
        pump(350);
        std::snprintf(name, sizeof(name), "u4-%s-e1-resolved.png", theme.tag);
        grab(&bar, grabDir, name);

        // E2: unresolved name (squiggle + fix bar).
        bar.field()->setPlainText(QStringLiteral("WalkRequsted && count > 2"));
        pump(400);
        std::snprintf(name, sizeof(name), "u4-%s-e2-unresolved.png", theme.tag);
        grab(&bar, grabDir, name);

        // E3: an accepted raw node (dotted underline + audit badge).
        setGuard(model, transId, QStringLiteral("(count & 0x3) == 0 && WalkRequested"), true);
        pump(350);
        std::snprintf(name, sizeof(name), "u4-%s-e3-raw.png", theme.tag);
        grab(&bar, grabDir, name);

        // E4: a folded island token.
        setGuard(model, transId, QStringLiteral("WalkRequested && { return count + 2 >= MIN_WAITING; }"));
        pump(350);
        std::snprintf(name, sizeof(name), "u4-%s-e4-island.png", theme.tag);
        grab(&bar, grabDir, name);

        // E5: mapping slots after accepting a call completion. The completer is `@`-mention
        // driven (SM-21-03): typing `@HasWait` filters to the HasWaiting condition; Enter accepts
        // the canonical `#cond:HasWaiting()`. A folded chip means the name lives in the committable
        // text, not the plain text, so assert on committableText().
        setGuard(model, transId, QString());
        pump(300);
        bar.field()->setFocus();
        for (const QChar c : QStringLiteral("#HasWait"))
        {
            QKeyEvent key(QEvent::KeyPress, c.unicode(), Qt::NoModifier, QString(c));
            QApplication::sendEvent(bar.field(), &key);
        }
        pump(200);
        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(bar.field(), &enter);
        pump(250);
        check(bar.field()->committableText().contains(QStringLiteral("HasWaiting")), "completion accepted the call (E5)");
        std::snprintf(name, sizeof(name), "u4-%s-e5-slots.png", theme.tag);
        grab(&bar, grabDir, name);
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QApplication::sendEvent(bar.field(), &esc);
        pump(100);

        // E6: shadowing (a stimulus parameter hides an attribute).
        SMMethodEntry* trigger = data.getMethods().findMethod(QStringLiteral("RequestWalk"));
        if (trigger->findElement(QStringLiteral("Backlog")) == nullptr)
        {
            trigger->addParam(QStringLiteral("Backlog"))->setType(QStringLiteral("uint32"));
            model.getNotifier().notifyElementChanged(trigger->getId(), eDocElementKind::Method);
            pump(200);
        }

        bar.field()->setPlainText(QStringLiteral("Backlog > 0"));
        pump(400);
        std::snprintf(name, sizeof(name), "u4-%s-e6-shadowing.png", theme.tag);
        grab(&bar, grabDir, name);

        bar.hide();
    }

    QApplication::setPalette(QApplication::style()->standardPalette());
}

// ---------------------------------------------------------------------------
// Item 24 (SM-21-07): the dark variant is a real second palette (not the light values reused),
// the surface differs per mode, the owner glyph (shape) channel is theme-independent, and every
// dark token clears WCAG AA against all three real app dark editor surfaces. All colors come from
// NEGuardStyle -- this asserts that one place, no rendering needed.
// ---------------------------------------------------------------------------
static void sweepThemeContrast()
{
    std::printf("[ RUN  ] item24-theme-contrast\n");

    const NEGuardStyle::eOwner owners[] =
    {
          NEGuardStyle::eOwner::Stimulus
        , NEGuardStyle::eOwner::Fsm
        , NEGuardStyle::eOwner::Handler
        , NEGuardStyle::eOwner::Literal
        , NEGuardStyle::eOwner::Raw
    };
    const NEGuardStyle::eSeverity sevs[] =
    {
          NEGuardStyle::eSeverity::Ok
        , NEGuardStyle::eSeverity::Warn
        , NEGuardStyle::eSeverity::Err
    };
    const char* ownerName[] = { "Stimulus", "Fsm", "Handler", "Literal", "Raw" };
    const char* sevName[]   = { "Ok", "Warn", "Err" };
    char label[128];

    // Capture the light token set from the standard (light) palette.
    QApplication::setPalette(QApplication::style()->standardPalette());
    check(NEGuardStyle::isDark() == false, "the standard palette selects the light tokens");
    const QColor lightSurface = QApplication::palette().color(QPalette::Base);
    QColor  lightOwner[5];
    QColor  lightSev[3];
    QString glyphLight[5];
    for (int i = 0; i < 5; ++i)
    {
        lightOwner[i] = NEGuardStyle::ownerColor(owners[i]);
        glyphLight[i] = NEGuardStyle::ownerGlyph(owners[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        lightSev[i] = NEGuardStyle::severityColor(sevs[i]);
    }

    // Install a real app dark palette. Nord is the lightest of the three shipped dark themes, so
    // its base (#3b4252) is the worst case for token-vs-surface contrast.
    QPalette darkPal;
    darkPal.setColor(QPalette::Window, QColor(0x2e, 0x34, 0x40));
    darkPal.setColor(QPalette::WindowText, QColor(0xe5, 0xe9, 0xf0));
    darkPal.setColor(QPalette::Base, QColor(0x3b, 0x42, 0x52));
    darkPal.setColor(QPalette::Text, QColor(0xe5, 0xe9, 0xf0));
    QApplication::setPalette(darkPal);
    check(NEGuardStyle::isDark(), "the dark palette selects the dark tokens");
    const QColor darkSurface = QApplication::palette().color(QPalette::Base);
    QColor darkOwner[5];
    QColor darkSev[3];
    for (int i = 0; i < 5; ++i)
    {
        darkOwner[i] = NEGuardStyle::ownerColor(owners[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        darkSev[i] = NEGuardStyle::severityColor(sevs[i]);
    }

    // The editor surface differs per mode (a light stab in a dark IDE was the risk this closes).
    check(lightSurface != darkSurface, "the editor surface differs between light and dark modes");

    // Critique-04 item 4: the glyph channel only disambiguates if no two glyphs are the SAME. Two
    // look-alike kind badges would quietly break disambiguation for grayscale / color-blind users.
    for (int i = 0; i < 5; ++i)
    {
        for (int j = i + 1; j < 5; ++j)
        {
            std::snprintf(label, sizeof(label), "owner glyphs '%s' and '%s' are distinct", ownerName[i], ownerName[j]);
            check(glyphLight[i] != glyphLight[j], label);
        }
    }

    // Each token has a distinct light and dark value: the dark variant is a real second palette.
    for (int i = 0; i < 5; ++i)
    {
        std::snprintf(label, sizeof(label), "owner token '%s' has a distinct dark value", ownerName[i]);
        check(lightOwner[i] != darkOwner[i], label);

        // The glyph (shape) channel is the color-blind / grayscale safety net; it must be stable
        // and non-empty in BOTH modes so it stays load-bearing when hues collide in grayscale.
        std::snprintf(label, sizeof(label), "owner glyph '%s' is theme-independent and non-empty", ownerName[i]);
        check((glyphLight[i] == NEGuardStyle::ownerGlyph(owners[i])) && (glyphLight[i].isEmpty() == false), label);
    }
    for (int i = 0; i < 3; ++i)
    {
        std::snprintf(label, sizeof(label), "severity token '%s' has a distinct dark value", sevName[i]);
        check(lightSev[i] != darkSev[i], label);
    }

    // Every dark token clears WCAG AA (>= 4.5:1) against all three shipped dark surfaces.
    const QColor darkSurfaces[] = { QColor(0x26, 0x2b, 0x34), QColor(0x14, 0x21, 0x40), QColor(0x3b, 0x42, 0x52) };
    const char*  surfaceName[]  = { "ModernDark", "MidnightBlue", "Nord" };
    for (int s = 0; s < 3; ++s)
    {
        for (int i = 0; i < 5; ++i)
        {
            std::snprintf(label, sizeof(label), "dark owner '%s' clears AA on %s (%.2f)",
                          ownerName[i], surfaceName[s], contrastRatio(darkOwner[i], darkSurfaces[s]));
            check(contrastRatio(darkOwner[i], darkSurfaces[s]) >= 4.5, label);
        }
        for (int i = 0; i < 3; ++i)
        {
            std::snprintf(label, sizeof(label), "dark severity '%s' clears AA on %s (%.2f)",
                          sevName[i], surfaceName[s], contrastRatio(darkSev[i], darkSurfaces[s]));
            check(contrastRatio(darkSev[i], darkSurfaces[s]) >= 4.5, label);
        }
    }

    QApplication::setPalette(QApplication::style()->standardPalette());
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    QApplication app(argc, argv);
    if (argc < 2)
    {
        std::printf("usage: lusan_sm_guard_u4_sweep <doc.fsml> [grab dir]\n");
        return 2;
    }

    const QString docPath = QString::fromLocal8Bit(argv[1]);
    const QString grabDir = (argc > 2) ? QString::fromLocal8Bit(argv[2]) : QStringLiteral(".");
    QDir().mkpath(grabDir);
    const QString tmpDir = QDir::tempPath();

    {
        StateMachineModel model;
        if (model.loadFromFile(docPath) == false)
        {
            std::printf("cannot load %s\n", docPath.toStdString().c_str());
            return 2;
        }

        const uint32_t transId = firstTransition(model.getData());
        check(transId != 0u, "the demo document has a transition");
        sweepObjectNames(model, transId, grabDir);
    }

    sweepItem16(docPath, tmpDir);
    sweepItem17(docPath, tmpDir);
    sweepItem18(docPath);
    sweepItem20(docPath, tmpDir);
    sweepItem21(docPath, tmpDir, grabDir);
    sweepTryIt(docPath, grabDir);
    sweepThemes(docPath, grabDir);
    sweepThemeContrast();

    std::printf("Done: %d checks, %d failure%s\n", gChecks, gFailures, (gFailures == 1) ? "" : "s");
    return (gFailures == 0) ? 0 : 1;
}
