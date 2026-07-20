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
 *  \file        tests/sm/SMArgMapTableTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-22 parameter-mapping editor: source kinds, compatibility + widening,
 *               resolution order, and undoable byte-exact round-trip -- driven offscreen
 *               through the real SMArgMapTable widget over an operation argument sink.
 *
 ************************************************************************/

#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/IArgSink.hpp"
#include "lusan/view/sm/SMArgMapTable.hpp"
#include "lusan/view/sm/SMArgSinkOperation.hpp"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFocusEvent>
#include <QHash>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QSet>
#include <QUndoStack>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cstdio>

namespace
{
    using eSource = SMArgumentEntry::eValueSource;

    int gChecks   = 0;
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

    //!< Builds the running example and returns (transitionId, the ActionCall owning the args).
    uint32_t buildDoc(StateMachineData& data, SMActionCall*& outCall)
    {
        data.getOverview().setName(QStringLiteral("MapMachine"));

        data.getAttributes().createAttribute(QStringLiteral("Waiting"))->setType(QStringLiteral("uint32"));
        data.getAttributes().createAttribute(QStringLiteral("IsNight"))->setType(QStringLiteral("bool"));

        ConstantEntry* konst = data.getConstants().createConstant(QStringLiteral("MAX_WAIT"));
        konst->setType(QStringLiteral("uint32"));
        konst->setValue(QStringLiteral("100"));

        SMMethodEntry* trigger = data.getMethods().createMethod(QStringLiteral("RequestWalk"), SMMethodEntry::eMethodType::Trigger);
        trigger->addParam(QStringLiteral("count"))->setType(QStringLiteral("uint16"));

        SMMethodEntry* ready = data.getMethods().createMethod(QStringLiteral("IsReady"), SMMethodEntry::eMethodType::Condition);
        ready->setImplement(SMMethodEntry::eImplement::Handler);       // parameterless, returns bool

        SMStateEntry* root = data.getStates().createState(QStringLiteral("Root"), SMStateEntry::eStateKind::Start);
        SMTransitionEntry* trans = root->getTransitions().createTransition(SMTransitionEntry::eStimulusKind::Trigger, QStringLiteral("RequestWalk"), QString());

        SMActionCall* call = new SMActionCall();
        call->setAction(QStringLiteral("Walk"));
        trans->getOperations().addOperation(call);
        outCall = call;

        return trans->getId();
    }

    //!< The declared signature the mapping editor renders (as the operations editor will feed it).
    QList<SMArgMapTable::Param> signature()
    {
        QList<SMArgMapTable::Param> params;
        params.append(SMArgMapTable::Param{ QStringLiteral("waiting"), QStringLiteral("uint32"), QString(),               false });
        params.append(SMArgMapTable::Param{ QStringLiteral("urgent"),  QStringLiteral("bool"),   QStringLiteral("false"), true  });
        params.append(SMArgMapTable::Param{ QStringLiteral("note"),    QStringLiteral("String"), QString(),               false });
        return params;
    }

    void selectSource(SMArgMapTable& ed, int row, eSource kind)
    {
        QComboBox* combo = ed.sourceCombo(row);
        const int index = combo->findData(static_cast<int>(kind));
        combo->setCurrentIndex(index);
        emit combo->activated(index);
        QCoreApplication::processEvents();
    }

    //!< Selects the leading clear item -- unmap the row back to its default.
    void clearSource(SMArgMapTable& ed, int row)
    {
        QComboBox* combo = ed.sourceCombo(row);
        combo->setCurrentIndex(0);
        emit combo->activated(0);
        QCoreApplication::processEvents();
    }

    void setLineText(SMArgMapTable& ed, int row, const QString& text)
    {
        QLineEdit* le = qobject_cast<QLineEdit*>(ed.valueWidget(row));
        le->setText(text);
        emit le->editingFinished();
        QCoreApplication::processEvents();
    }

    QSet<int> sourceKinds(SMArgMapTable& ed, int row)
    {
        QSet<int> kinds;
        QComboBox* combo = ed.sourceCombo(row);
        for (int i = 0; i < combo->count(); ++i)
        {
            const int data = combo->itemData(i).toInt();
            if (data >= 0) { kinds.insert(data); }
        }
        return kinds;
    }

    const SMArgumentEntry* argByName(const QList<SMArgumentEntry>& args, const QString& name)
    {
        for (const SMArgumentEntry& a : args)
        {
            if (a.getName() == name) { return &a; }
        }
        return nullptr;
    }

    /**
     * \brief   A bare IArgSink that records what the table commits, with no model, no
     *          commands and no undo stack behind it -- so a check on \ref commits() proves
     *          the table itself did or did not try to write.
     **/
    class StubSink : public IArgSink
    {
    public:
        StubSink(void) : mArgs(), mCommits(0) { }

        virtual void setArg(const QString& paramName, eSource src, const QString& value, const QString& expr) override
        {
            SMArgumentEntry entry(0u, paramName, src, value);
            entry.setExpression(expr);
            mArgs.insert(paramName, entry);
            ++mCommits;
        }

        virtual void clearArg(const QString& paramName) override
        {
            mArgs.remove(paramName);
            ++mCommits;
        }

        virtual const SMArgumentEntry* argFor(const QString& paramName) const override
        {
            const auto it = mArgs.constFind(paramName);
            return (it != mArgs.constEnd()) ? &it.value() : nullptr;
        }

        inline int commits(void) const { return mCommits; }

    private:
        QHash<QString, SMArgumentEntry> mArgs;
        int                             mCommits;
    };

    //!< The editable combo of a Compact row.
    QComboBox* compactCombo(SMArgMapTable& table, int row)
    {
        return qobject_cast<QComboBox*>(table.valueWidget(row));
    }

    //!< Delivers one key press to \p target through the normal filter chain.
    void sendKey(QWidget* target, int key)
    {
        QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
        QApplication::sendEvent(target, &press);
    }
}

int main(int argc, char* argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    QApplication app(argc, argv);

    const QString grabDir = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QString();

    StateMachineModel model;
    SMActionCall*     call = nullptr;
    const uint32_t    tid  = buildDoc(model.getData(), call);

    SMArgSinkOperation sink(model);
    sink.bind(call, &call->getArguments());

    SMArgMapTable ed(model);
    ed.bind(tid, /*allowParam*/ true, &sink, signature());
    ed.resize(780, 240);

    check(ed.rowCount() == 3, "one row per declared parameter");

    // --- AC1: every legal source kind selectable; illegal (Param off-scope) absent. ---
    const QSet<int> kinds = sourceKinds(ed, 0);
    check(kinds.contains(static_cast<int>(eSource::Value)),      "Value kind offered");
    check(kinds.contains(static_cast<int>(eSource::Param)),      "Param kind offered in transition scope");
    check(kinds.contains(static_cast<int>(eSource::Attribute)),  "Attribute kind offered");
    check(kinds.contains(static_cast<int>(eSource::Constant)),   "Constant kind offered");
    check(kinds.contains(static_cast<int>(eSource::Condition)),  "Condition kind offered");
    check(kinds.contains(static_cast<int>(eSource::Expression)), "Expression kind offered");
    check(kinds.size() == 6, "exactly the six source kinds are offered where all are legal");

    {
        SMArgMapTable entryScope(model);
        entryScope.bind(0u, /*allowParam*/ false, &sink, signature());
        check(sourceKinds(entryScope, 0).contains(static_cast<int>(eSource::Param)) == false,
              "Param kind absent outside transition scope (spec 6.8 scope rule)");
    }

    // --- AC3: resolution order reflected -- unmapped defaulted shows its default, else required. ---
    checkEq(ed.statusLabel(0)->text(), QStringLiteral("required"),        "non-defaulted unmapped parameter flagged required");
    checkEq(ed.statusLabel(1)->text(), QStringLiteral("default: false"),  "defaulted unmapped parameter shows its default");

    // --- AC2: compatibility filtering + visible widening. ---
    // The Attribute dropdown must offer only type-compatible entries (Waiting:uint32, not IsNight:bool).
    selectSource(ed, 0, eSource::Attribute);
    {
        QComboBox* refs = qobject_cast<QComboBox*>(ed.valueWidget(0));
        QStringList names;
        for (int i = 0; i < refs->count(); ++i) { names << refs->itemText(i); }
        check(names.contains(QStringLiteral("Waiting")),  "compatible attribute (uint32) offered");
        check(names.contains(QStringLiteral("IsNight")) == false, "incompatible attribute (bool) filtered out");
    }

    // waiting : uint32  <- Param count : uint16  (implicit widening, shown on the row). This is
    // the mapping kept for the round-trip below.
    selectSource(ed, 0, eSource::Param);
    {
        const SMArgumentEntry* a = argByName(call->getArguments(), QStringLiteral("waiting"));
        check((a != nullptr) && (a->getSource() == eSource::Param) && (a->getValue() == QStringLiteral("count")),
              "Param source auto-picks the sole compatible stimulus parameter");
    }
    checkEq(ed.typeLabel(0)->text(),  QStringLiteral("uint16 -> uint32"), "implicit widening shown explicitly on the row");
    checkEq(ed.statusLabel(0)->text(), QStringLiteral("ok, converts"),    "widening mapping flagged as a lossless conversion");

    // urgent : bool -- the Condition dropdown offers the parameterless bool condition method;
    // then clear urgent back to its default so the file omits it (resolution via default).
    selectSource(ed, 1, eSource::Condition);
    {
        QComboBox* refs = qobject_cast<QComboBox*>(ed.valueWidget(1));
        QStringList names;
        for (int i = 0; i < refs->count(); ++i) { names << refs->itemText(i); }
        check(names.contains(QStringLiteral("IsReady")), "parameterless bool condition offered as a Condition source");
    }
    clearSource(ed, 1);
    check(argByName(call->getArguments(), QStringLiteral("urgent")) == nullptr, "clearing a mapping removes the argument (revert to default)");
    checkEq(ed.statusLabel(1)->text(), QStringLiteral("default: false"), "a cleared defaulted parameter shows its default again");

    // --- AC4: undoable + byte-exact round-trip (including an Expression cell). ---
    // note : String  <- Expression (verbatim C++, stored byte-exact).
    const QString verbatim = QStringLiteral("QString::number(count) + \" waiting /*note*/\"");
    selectSource(ed, 2, eSource::Expression);
    setLineText(ed, 2, verbatim);
    {
        const SMArgumentEntry* a = argByName(call->getArguments(), QStringLiteral("note"));
        check((a != nullptr) && (a->getSource() == eSource::Expression), "Expression source stored for note");
        checkEq((a != nullptr) ? a->getExpression() : QString(), verbatim, "Expression text stored verbatim in the model");
    }

    // Undo the last edit (the Expression text) then redo -- both drive the widget through the notifier.
    const int mappedBefore = call->getArguments().size();
    model.getUndoStack().undo();
    QCoreApplication::processEvents();
    {
        const SMArgumentEntry* a = argByName(call->getArguments(), QStringLiteral("note"));
        check((a != nullptr) && (a->getExpression().isEmpty()), "undo reverts the Expression edit");
    }
    model.getUndoStack().redo();
    QCoreApplication::processEvents();
    checkEq(argByName(call->getArguments(), QStringLiteral("note"))->getExpression(), verbatim, "redo restores the Expression edit");
    check(call->getArguments().size() == mappedBefore, "argument count is stable across undo/redo");

    // Serialize the whole document and reload it: the mappings (incl. verbatim) round-trip byte-exactly.
    QString xml;
    {
        QXmlStreamWriter writer(&xml);
        writer.setAutoFormatting(true);
        model.getData().writeToXml(writer);
    }

    StateMachineData reread;
    {
        QXmlStreamReader reader(xml);
        while (reader.readNextStartElement())
        {
            reread.readFromXml(reader);
        }
    }

    SMTransitionEntry* rt = reread.findTransitionById(tid);
    check((rt != nullptr) && (rt->getOperations().getCount() == 1), "operation survives the round-trip");
    if ((rt != nullptr) && (rt->getOperations().getCount() == 1))
    {
        SMActionCall* rc = static_cast<SMActionCall*>(rt->getOperations().at(0));
        const SMArgumentEntry* waiting = argByName(rc->getArguments(), QStringLiteral("waiting"));
        const SMArgumentEntry* note    = argByName(rc->getArguments(), QStringLiteral("note"));
        check((waiting != nullptr) && (waiting->getSource() == eSource::Param) && (waiting->getValue() == QStringLiteral("count")),
              "Param mapping round-trips");
        checkEq((note != nullptr) ? note->getExpression() : QString(), verbatim, "Expression cell round-trips byte-exactly");
        check(argByName(rc->getArguments(), QStringLiteral("urgent")) == nullptr,
              "defaulted-and-unmapped parameter is omitted from the file (resolution via default)");
    }

    // ---------------------------------------------------------------------------------
    // SM-21-01: the same widget in its Compact (Actions) shape, over a bare stub sink --
    // it projects rows and commits through IArgSink only, owning no argument state.
    // ---------------------------------------------------------------------------------
    StubSink       stub;
    SMArgMapTable  compact(model);
    compact.setRowStyle(SMArgMapTable::eRowStyle::Compact);
    compact.bind(tid, /*allowParam*/ true, &stub, signature());
    compact.resize(420, 140);

    check(compact.rowCount() == 3, "Compact: one row per formal parameter");
    check(stub.commits() == 0, "binding is a pure projection -- it commits nothing");

    // Row labels read `name (Type)`.
    checkEq(compact.nameLabel(0)->text(), QStringLiteral("waiting (uint32)"), "Compact: `name (Type)` label format");
    checkEq(compact.nameLabel(1)->text(), QStringLiteral("urgent (bool)"),    "Compact: `name (Type)` label on a defaulted parameter");
    checkEq(compact.nameLabel(2)->text(), QStringLiteral("note (String)"),    "Compact: `name (Type)` label on a String parameter");

    // Unmapped (ghost) rendering: the field is empty and hints what filling it would mean.
    checkEq(compactCombo(compact, 0)->currentText(), QString(), "Compact: an unmapped slot renders empty");
    checkEq(compactCombo(compact, 0)->lineEdit()->placeholderText(), QStringLiteral("value, or pick a source"),
            "Compact: an unmapped required slot hints at its sources");
    checkEq(compactCombo(compact, 1)->lineEdit()->placeholderText(), QStringLiteral("default: false"),
            "Compact: an unmapped defaulted slot shows its default as the grey hint");

    // The Actions source set: stimulus parameters + attributes, and no constants.
    {
        QStringList offered;
        QComboBox* combo = compactCombo(compact, 0);
        for (int i = 0; i < combo->count(); ++i) { offered << combo->itemText(i); }
        check(offered.contains(QStringLiteral("count")),    "Compact: the stimulus parameter is offered in a transition scope");
        check(offered.contains(QStringLiteral("Waiting")),  "Compact: attributes are offered");
        check(offered.contains(QStringLiteral("IsNight")),  "Compact: an attribute is offered whatever its type (no compat filtering)");
        check(offered.contains(QStringLiteral("MAX_WAIT")) == false, "Compact: constants are NOT offered (Actions parity)");
    }

    // A picked entry resolves to its source kind; unmatched text stays a free literal.
    {
        QComboBox* combo = compactCombo(compact, 0);
        combo->setEditText(QStringLiteral("Waiting"));
        emit combo->lineEdit()->editingFinished();
        const SMArgumentEntry* a = stub.argFor(QStringLiteral("waiting"));
        check((a != nullptr) && (a->getSource() == eSource::Attribute), "Compact: a picked attribute commits as an Attribute source");

        combo->setEditText(QStringLiteral("count"));
        emit combo->lineEdit()->editingFinished();
        a = stub.argFor(QStringLiteral("waiting"));
        check((a != nullptr) && (a->getSource() == eSource::Param), "Compact: a picked stimulus parameter commits as a Param source");

        combo->setEditText(QStringLiteral("41 + 1"));
        emit combo->lineEdit()->editingFinished();
        a = stub.argFor(QStringLiteral("waiting"));
        check((a != nullptr) && (a->getSource() == eSource::Value) && (a->getValue() == QStringLiteral("41 + 1")),
              "Compact: unmatched text is kept as a free literal (Value source)");

        combo->setEditText(QString());
        emit combo->lineEdit()->editingFinished();
        check(stub.argFor(QStringLiteral("waiting")) == nullptr, "Compact: emptying a field unmaps the slot");
    }

    // Esc reverts the field to its pre-edit value and commits NOTHING (spec 6, item 13).
    {
        QComboBox* combo = compactCombo(compact, 2);
        combo->setEditText(QStringLiteral("keep me"));
        emit combo->lineEdit()->editingFinished();
        const int committed = stub.commits();
        checkEq(stub.argFor(QStringLiteral("note"))->getValue(), QStringLiteral("keep me"), "Compact: the pre-Esc value is committed normally");

        // Focus marks the revert point, then the edit is abandoned.
        QFocusEvent focusIn(QEvent::FocusIn);
        QApplication::sendEvent(combo->lineEdit(), &focusIn);
        combo->lineEdit()->setText(QStringLiteral("abandoned typing"));
        sendKey(combo->lineEdit(), Qt::Key_Escape);

        checkEq(combo->lineEdit()->text(), QStringLiteral("keep me"), "Esc reverts the field to the value it held when editing began");
        check(stub.commits() == committed, "Esc commits nothing -- the sink is never called");
        checkEq(stub.argFor(QStringLiteral("note"))->getValue(), QStringLiteral("keep me"), "Esc leaves the stored mapping untouched");
    }

    // Undo granularity through the REAL operation sink: one edit is one step, and a commit
    // that changes nothing never reaches the stack.
    {
        const int before = model.getUndoStack().count();
        sink.setArg(QStringLiteral("note"), eSource::Value, QStringLiteral("hello"), QString());
        const int afterFirst = model.getUndoStack().count();
        sink.setArg(QStringLiteral("note"), eSource::Value, QStringLiteral("hello"), QString());
        const int afterSecond = model.getUndoStack().count();

        check(afterFirst == (before + 1), "one mapping edit pushes exactly one undo command");
        check(afterSecond == afterFirst,  "re-committing the value already stored pushes nothing");

        model.getUndoStack().undo();
        QCoreApplication::processEvents();
        checkEq(argByName(call->getArguments(), QStringLiteral("note"))->getExpression(), verbatim,
                "one undo restores the row's previous mapping");
    }

    // Width (hazard 12.4): no signature length may widen the dock that hosts the table.
    check(compact.minimumSizeHint().width() == 0, "Compact: minimumSizeHint width is 0");
    check(ed.minimumSizeHint().width() == 0,      "Detailed: minimumSizeHint width is 0");
    {
        QList<SMArgMapTable::Param> wide;
        wide.append(SMArgMapTable::Param{ QStringLiteral("a_very_long_formal_parameter_name_indeed"), QStringLiteral("uint32"), QString(), false });
        SMArgMapTable stretched(model);
        stretched.setRowStyle(SMArgMapTable::eRowStyle::Compact);
        stretched.bind(tid, true, &stub, wide);
        check(stretched.minimumSizeHint().width() == 0, "a long signature still cannot widen the dock");
    }

    if (grabDir.isEmpty() == false)
    {
        QDir().mkpath(grabDir);
        ed.grab().save(grabDir + QStringLiteral("/g22-mapping.png"));
        compact.grab().save(grabDir + QStringLiteral("/g21-01-argmap-compact.png"));
    }

    std::printf("SMArgMapTableTests: %d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
