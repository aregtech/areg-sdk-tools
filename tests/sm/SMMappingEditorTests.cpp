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
 *  \file        tests/sm/SMMappingEditorTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       SM-22 parameter-mapping editor: source kinds, compatibility + widening,
 *               resolution order, and undoable byte-exact round-trip -- driven offscreen
 *               through the real SMMappingEditor widget.
 *
 ************************************************************************/

#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMMappingEditor.hpp"

#include <QApplication>
#include <QComboBox>
#include <QDir>
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
    QList<SMMappingEditor::Param> signature()
    {
        QList<SMMappingEditor::Param> params;
        params.append(SMMappingEditor::Param{ QStringLiteral("waiting"), QStringLiteral("uint32"), QString(),               false });
        params.append(SMMappingEditor::Param{ QStringLiteral("urgent"),  QStringLiteral("bool"),   QStringLiteral("false"), true  });
        params.append(SMMappingEditor::Param{ QStringLiteral("note"),    QStringLiteral("String"), QString(),               false });
        return params;
    }

    void selectSource(SMMappingEditor& ed, int row, eSource kind)
    {
        QComboBox* combo = ed.sourceCombo(row);
        const int index = combo->findData(static_cast<int>(kind));
        combo->setCurrentIndex(index);
        emit combo->activated(index);
        QCoreApplication::processEvents();
    }

    //!< Selects the leading clear item -- unmap the row back to its default.
    void clearSource(SMMappingEditor& ed, int row)
    {
        QComboBox* combo = ed.sourceCombo(row);
        combo->setCurrentIndex(0);
        emit combo->activated(0);
        QCoreApplication::processEvents();
    }

    void setLineText(SMMappingEditor& ed, int row, const QString& text)
    {
        QLineEdit* le = qobject_cast<QLineEdit*>(ed.valueWidget(row));
        le->setText(text);
        emit le->editingFinished();
        QCoreApplication::processEvents();
    }

    QSet<int> sourceKinds(SMMappingEditor& ed, int row)
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
}

int main(int argc, char* argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    QApplication app(argc, argv);

    const QString grabDir = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QString();

    StateMachineModel model;
    SMActionCall*     call = nullptr;
    const uint32_t    tid  = buildDoc(model.getData(), call);

    SMMappingEditor ed(model);
    ed.bind(tid, /*allowParam*/ true, call, &call->getArguments(), signature());
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
        SMMappingEditor entryScope(model);
        entryScope.bind(0u, /*allowParam*/ false, call, &call->getArguments(), signature());
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

    if (grabDir.isEmpty() == false)
    {
        QDir().mkpath(grabDir);
        ed.grab().save(grabDir + QStringLiteral("/g22-mapping.png"));
    }

    std::printf("SMMappingEditorTests: %d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
