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
 *  \file        tests/sm/SMGuardUiShots.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-21 (U3) UI verification harness: drives the REAL SMGuardBar and Methods
 *               page widgets programmatically (no input injection), checks the field
 *               invariants 23a/23b and the form-first parity at widget level, and saves
 *               QWidget::grab() screenshots (lens, grid, island, Methods page).
 *
 *               Usage: lusan_sm_guard_ui_shots <doc.fsml> [grab dir]
 *
 ************************************************************************/

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMGuardBar.hpp"
#include "lusan/view/sm/SMGuardCallsOutline.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"
#include "lusan/view/sm/SMMethod.hpp"
#include "lusan/view/common/MethodListView.hpp"
#include "lusan/view/sm/SMRefCompleter.hpp"
#include "lusan/view/sm/SMSignatureCard.hpp"
#include "lusan/view/sm/SMStructureLens.hpp"

#include <QApplication>
#include <QToolBox>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QScreen>
#include <QTextCursor>
#include <QTreeWidget>
#include <cstdio>

/**
 * \class   ProbeField
 * \brief   Lifts the field's clipboard overrides to public, so the copy/paste legs of
 *          acceptance 23a are exercised directly on the real code paths -- the system
 *          clipboard is not reachable from a harness process on every desktop.
 **/
class ProbeField : public SMGuardField
{
public:
    using SMGuardField::SMGuardField;
    using SMGuardField::createMimeDataFromSelection;
    using SMGuardField::insertFromMimeData;
};

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

    //!< Spins the event loop for \p ms (deferred rebuilds, debounce, delayed commits).
    void pump(int ms)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < ms)
        {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
        }
    }

    //!< Sends a synthetic key press+release carrying \p text to \p w (drives the field's keys).
    void typeChar(QWidget* w, int key, const QString& text)
    {
        QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, text);
        QApplication::sendEvent(w, &press);
        QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier, text);
        QApplication::sendEvent(w, &release);
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

    //!< The first transition of the document (the demo machine has exactly one).
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

    //!< Installs guard \p text through the same command path the field commit uses.
    void setGuard(StateMachineModel& model, uint32_t transitionId, const QString& text)
    {
        SMGuard guard = SMGuardParser::parseToGuard(model.getData(), transitionId, text);
        model.getUndoStack().push(SMGuardCommands::setGuard(model.getData(), model.getNotifier(), transitionId, guard, QStringLiteral("set")));
    }

    QString guardText(const StateMachineModel& model, uint32_t transitionId)
    {
        const SMTransitionEntry* transition = model.getData().findTransitionById(transitionId);
        return (transition != nullptr) ? SMGuardRender::guardText(model.getData(), transitionId, transition->getGuard()) : QString();
    }
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    QApplication app(argc, argv);
    if (argc < 2)
    {
        std::printf("usage: lusan_sm_guard_ui_shots <doc.fsml> [grab dir]\n");
        return 2;
    }

    const QString docPath = QString::fromLocal8Bit(argv[1]);
    const QString grabDir = (argc > 2) ? QString::fromLocal8Bit(argv[2]) : QStringLiteral(".");
    QDir().mkpath(grabDir);

    StateMachineModel model;
    if (model.loadFromFile(docPath) == false)
    {
        std::printf("cannot load %s\n", docPath.toStdString().c_str());
        return 2;
    }

    const uint32_t transId = firstTransition(model.getData());
    check(transId != 0u, "the demo document has a transition");
    if (transId == 0u)
    {
        return 1;
    }


    SMGuardBar bar(model);
    bar.resize(640, 520);
    bar.show();
    bar.setTransition(transId);
    pump(150);

    // ---- S5: the lens over the running example B ---------------------------
    std::printf("[ RUN  ] lens\n");
    setGuard(model, transId, QStringLiteral("WalkRequested && (HasWaiting(count) || count >= MIN_WAITING) && !IsNightMode"));
    pump(300);
    check(bar.lens()->findChild<QWidget*>(QStringLiteral("smLensPill_0")) != nullptr, "lens renders the first clause pill");
    check(bar.lens()->findChild<QWidget*>(QStringLiteral("smLensJoin_root")) != nullptr, "lens renders the root joiner");
    grab(&bar, grabDir, "u3-lens-open.png");

    // ---- 23a: the island token is ONE character to caret/selection/clipboard --
    std::printf("[ RUN  ] islandAtomicity\n");
    const QString body = QStringLiteral(" const uint16_t effective = count + 2u;\n return effective >= MIN_WAITING; ");
    const QString islandText = QStringLiteral("WalkRequested && {") + body + QStringLiteral("}");
    setGuard(model, transId, islandText);
    pump(300);

    SMGuardField* field = bar.field();
    check(field->islandCount() == 1, "the island folded to one token");
    checkEq(field->islandBody(0), body, "the token carries the body byte-exact");

    // Caret: crossing the token is one step.
    const QString docText = field->toPlainText();
    const int tokenAt = docText.indexOf(QChar(0xFFFC));
    check(tokenAt >= 0, "the token is one character in the document");
    QTextCursor cursor = field->textCursor();
    cursor.setPosition(tokenAt);
    field->setTextCursor(cursor);
    cursor.movePosition(QTextCursor::Right);
    check(cursor.position() == tokenAt + 1, "caret crosses the island in ONE step");

    // Selection + copy: the mime data built from the selection expands the island
    // byte-exact (the real createMimeDataFromSelection override, no system clipboard).
    ProbeField probe(model);
    probe.resize(600, 60);
    probe.show();
    probe.setTransition(transId);
    pump(300);
    check(probe.islandCount() == 1, "the probe field folded the island");

    probe.selectAll();
    QMimeData* copied = probe.createMimeDataFromSelection();
    check(copied != nullptr, "copy builds mime data");
    if (copied != nullptr)
    {
        checkEq(copied->text(), islandText, "copy round-trips the island byte-exact");
    }

    // Paste: the balanced block re-enters as a token with the same body (the real
    // insertFromMimeData override).
    probe.selectAll();
    probe.textCursor().removeSelectedText();
    check(probe.islandCount() == 0, "the probe field was cleared");
    if (copied != nullptr)
    {
        probe.insertFromMimeData(copied);
        pump(250);
        check(probe.islandCount() == 1, "paste recreated the island token");
        checkEq(probe.islandBody(0), body, "pasted body is byte-exact");
        checkEq(probe.committableText(), islandText, "pasted committable text is byte-exact");
        delete copied;
    }

    probe.hide();
    checkEq(field->committableText(), islandText, "committable text round-trips byte-exact");

    // ---- S4: the island editor -----------------------------------------------
    std::printf("[ RUN  ] islandEditor\n");
    bar.islandEditor()->openFor(model, transId, 0, body);
    pump(150);
    check(bar.islandEditor()->isVisible(), "the island editor is open");
    grab(&bar, grabDir, "u3-island-open.png");
    bar.islandEditor()->hide();

    // ---- accordion: selecting a call binds the inline Arguments table (retired grid) ----
    // The old two-way mapping popover (SMMappingGrid) is replaced by the accordion Arguments
    // table (SMArgMapTable + SMArgSinkGuard). The inline-mapping parity (a mapping rewrites the
    // tree, one undo restores both) is covered headless in SMArgMapTableTests; here we assert the
    // outline lists the call and selecting it binds the shared table.
    std::printf("[ RUN  ] accordionArguments\n");
    setGuard(model, transId, QStringLiteral("HasWaiting(count)"));
    pump(300);

    bar.accordion()->setCurrentIndex(0);        // the Calls section.
    pump(50);
    check(bar.calls()->callRowCount() >= 1, "the Calls outline lists the call");
    bar.calls()->selectCallPath({});            // the guard's root call (HasWaiting).
    pump(150);
    check((bar.args() != nullptr) && (bar.args()->rowCount() >= 1), "selecting the call binds the Arguments table");
    grab(&bar, grabDir, "u3-accordion-arguments.png");

    // ---- SM-21-04: catalog / Insert produces the same id-bound tree as typing --------
    std::printf("[ RUN  ] catalogInsert\n");
    {
        const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(model.getData(), transId);
        const SMGuardSymbol* value = nullptr;
        const SMGuardSymbol* call  = nullptr;
        for (const SMGuardSymbol& sym : catalog)
        {
            if ((value == nullptr) && (sym.isCall == false)) { value = &sym; }
            if ((call == nullptr) && sym.isCall)             { call = &sym; }
        }

        check(value != nullptr, "the catalog offers a value symbol to insert");
        if (value != nullptr)
        {
            setGuard(model, transId, QString());
            pump(200);
            field->setFocus();
            field->insertReference(*value);         // inserts the canonical reference and commits.
            pump(300);
            const QString viaInsert = guardText(model, transId);

            setGuard(model, transId, value->mention());     // type the same canonical reference.
            pump(300);
            const QString viaType = guardText(model, transId);
            checkEq(viaInsert, viaType, "inserting a symbol == typing its canonical reference (id-bound)");
        }

        if (call != nullptr)
        {
            setGuard(model, transId, QString());
            pump(200);
            field->setFocus();
            field->insertReference(*call);          // inserts `@cond:name()`, caret in the parens.
            pump(150);
            check(field->committableText().contains(call->name + QLatin1Char('(')), "inserting a call inserts name( with the caret in the parens");
        }
    }

    // ---- SM-21-04: the one warning channel raises for an unmapped argument -----------
    std::printf("[ RUN  ] warningChannel\n");
    {
        SMFixBar* warn = bar.findChild<SMFixBar*>(QStringLiteral("smGuardWarnBar"));
        check(warn != nullptr, "the warning channel widget exists");

        setGuard(model, transId, QStringLiteral("HasWaiting()"));    // a call with an unmapped formal.
        pump(400);
        check((warn != nullptr) && warn->isVisible(), "an unmapped argument raises the warning channel");

        setGuard(model, transId, QStringLiteral("HasWaiting(count)"));   // now fully mapped.
        pump(400);
        check((warn != nullptr) && (warn->isVisible() == false), "mapping the argument clears the warning channel");
    }

    // ---- form-first parity: the clause popover path equals the typed path -----
    std::printf("[ RUN  ] formFirstParity\n");
    setGuard(model, transId, QString());
    pump(300);
    field->appendClause(QStringLiteral("&&"), QStringLiteral("HasWaiting(count)"));
    pump(600);      // the visible-insert delay plus the deferred rebuild
    checkEq(guardText(model, transId), QStringLiteral("HasWaiting(count)"), "the popover-built clause committed");

    const SMTransitionEntry* transition = model.getData().findTransitionById(transId);
    SMGuardParser::Result typed = SMGuardParser::parse(model.getData(), transId, QStringLiteral("HasWaiting(count)"));
    check((transition != nullptr) && typed.resolved() && (typed.tree != nullptr)
          && transition->getGuard().isOk()
          && transition->getGuard().getTree()->equals(*typed.tree)
        , "the stored tree equals the typed-path tree");
    delete typed.tree;

    // ---- SM-21-03: committed references fold into chips; committable/clipboard exact ----
    std::printf("[ RUN  ] chips\n");
    setGuard(model, transId, QStringLiteral("WalkRequested && count >= MIN_WAITING"));
    pump(300);
    check(field->chipCount() == 3, "three references folded into chips (attr, param, const)");
    checkEq(field->committableText(), QStringLiteral("WalkRequested && count >= MIN_WAITING")
          , "committableText expands chips to canonical text (no markers reach the tree, 12.7)");
    grab(&bar, grabDir, "sm2103-chips.png");

    {
        ProbeField chipProbe(model);
        chipProbe.resize(600, 60);
        chipProbe.show();
        chipProbe.setTransition(transId);
        pump(300);
        check(chipProbe.chipCount() == 3, "the probe folded the same three chips");
        chipProbe.selectAll();
        QMimeData* chipCopy = chipProbe.createMimeDataFromSelection();
        check(chipCopy != nullptr, "copy builds mime data over chips");
        if (chipCopy != nullptr)
        {
            checkEq(chipCopy->text(), QStringLiteral("WalkRequested && count >= MIN_WAITING")
                  , "chip clipboard round-trips byte-exact");
            delete chipCopy;
        }
        chipProbe.hide();
    }

    // ---- SM-21-03: the reference completer (D-POPUP): pass-through, D-ESC, filter, clamp ----
    std::printf("[ RUN  ] completer\n");
    setGuard(model, transId, QString());
    pump(200);
    field->setFocus();
    SMRefCompleter* comp = field->findChild<SMRefCompleter*>(QStringLiteral("smRefCompleter"));
    check(comp != nullptr, "the field owns a reference completer");
    if (comp != nullptr)
    {
        typeChar(field, Qt::Key_At, QStringLiteral("@"));
        pump(120);
        check(comp->isVisible(), "typing @ opens the completer");

        // 12.6 pass-through: typing narrows the list AND the characters reach the document.
        typeChar(field, Qt::Key_W, QStringLiteral("W"));
        typeChar(field, Qt::Key_A, QStringLiteral("a"));
        pump(120);
        check(field->committableText().contains(QStringLiteral("@Wa")), "typed characters pass through to the document (no swallow, 12.6)");
        check(comp->isVisible(), "the completer stays open while typing");

        // D-ESC: Esc closes but keeps the text; it does not reopen for the same token.
        typeChar(field, Qt::Key_Escape, QString());
        pump(80);
        check(comp->isVisible() == false, "Esc closes the completer");
        check(field->committableText().contains(QStringLiteral("@Wa")), "Esc keeps the typed characters (D-ESC)");
        typeChar(field, Qt::Key_L, QStringLiteral("l"));
        pump(80);
        check(comp->isVisible() == false, "the completer does not reopen for the same token after Esc (D-ESC)");
    }

    // Widget-level: a `@kind:` filter shows only that kind and inserts the canonical @kind:name;
    // a caret at the screen corner keeps the popup fully on-screen (12.5).
    {
        SMRefCompleter probe;
        probe.setSymbols(SMGuardCatalog::build(model.getData(), transId));
        probe.showFor({ SMGuardSymbol::eRefKind::Attr }, QString(), QRect(QPoint(120, 120), QSize(2, 16)));
        const SMGuardSymbol* cur = probe.currentSymbol();
        check((cur != nullptr) && (cur->refkind == SMGuardSymbol::eRefKind::Attr), "a @attr: filter shows only attributes");
        if (cur != nullptr)
        {
            check(cur->mention().startsWith(QStringLiteral("@attr:")), "an attribute inserts as @attr:name");
        }

        const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
        probe.showFor({}, QString(), QRect(QPoint(avail.right() - 2, avail.bottom() - 2), QSize(2, 16)));
        const QRect g = probe.geometry();
        check((g.left() >= avail.left()) && (g.top() >= avail.top())
              && (g.right() <= avail.right()) && (g.bottom() <= avail.bottom())
            , "the completer clamps within the screen at a corner caret (12.5)");
        probe.hide();
    }

    setGuard(model, transId, QString());
    pump(150);

    // ---- SM-21-03: signature help + D-PRECEDENCE (the completer yields, then returns) ----
    std::printf("[ RUN  ] signatureHelp\n");
    setGuard(model, transId, QString());
    pump(150);
    field->setFocus();
    SMSignatureCard* sig = field->findChild<SMSignatureCard*>(QStringLiteral("smSignatureCard"));
    SMRefCompleter* comp2 = field->findChild<SMRefCompleter*>(QStringLiteral("smRefCompleter"));
    check(sig != nullptr, "the field owns a signature card");
    if ((sig != nullptr) && (comp2 != nullptr))
    {
        for (const QChar c : QStringLiteral("HasWaiting("))
        {
            typeChar(field, c.unicode(), QString(c));
        }
        pump(150);
        check(sig->isVisible(), "signature help shows while the caret is inside a call's parentheses");

        // D-PRECEDENCE: typing '@' opens the completer OVER the tooltip and hides it.
        typeChar(field, Qt::Key_At, QStringLiteral("@"));
        pump(150);
        check(comp2->isVisible(), "typing @ inside the call opens the completer");
        check(sig->isVisible() == false, "D-PRECEDENCE: the completer hides the signature card");

        // Closing the completer restores the signature card.
        typeChar(field, Qt::Key_Escape, QString());
        pump(150);
        check(comp2->isVisible() == false, "Esc closes the completer");
        check(sig->isVisible(), "closing the completer restores the signature card (D-PRECEDENCE)");
    }
    setGuard(model, transId, QString());
    pump(150);

    // ---- S14: the Methods page kind split -------------------------------------
    std::printf("[ RUN  ] methodsPage\n");
    SMMethod page(model.getMethodModel());
    page.resize(980, 620);
    page.show();
    pump(250);

    QTreeWidget* tree = page.getList()->ctrlTableList();
    bool sawHandler = false;
    bool sawLambda = false;
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
    {
        const QString kind = tree->topLevelItem(i)->text(1);
        sawHandler = sawHandler || (kind == QStringLiteral("h handler"));
        sawLambda = sawLambda || (kind == QStringLiteral("{} lambda"));
        if ((kind == QStringLiteral("h handler")) || (kind == QStringLiteral("{} lambda")))
        {
            tree->setCurrentItem(tree->topLevelItem(i));
        }
    }

    check(sawHandler, "the Methods page shows an 'h handler' condition");
    check(sawLambda, "the Methods page shows a '{} lambda' condition");
    pump(150);
    grab(&page, grabDir, "u3-methods-page.png");

    std::printf("Done: %d checks, %d failure%s\n", gChecks, gFailures, (gFailures == 1) ? "" : "s");
    return (gFailures == 0) ? 0 : 1;
}
