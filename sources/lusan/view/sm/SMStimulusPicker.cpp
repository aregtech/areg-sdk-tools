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
 *  \file        lusan/view/sm/SMStimulusPicker.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM: the shared stimulus combo (fill it, read it, commit it).
 *
 ************************************************************************/

#include "lusan/view/sm/SMStimulusPicker.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMKindGlyph.hpp"

#include <QComboBox>
#include <QCoreApplication>
#include <QPalette>

QString SMStimulusPicker::displayLabel(SMTransitionEntry::eStimulusKind kind, const QString& name)
{
    return SMKindGlyph::prefix(SMKindGlyph::stimulusGlyph(kind)) + name;
}

int SMStimulusPicker::fill(QComboBox& picker, const StateMachineData& data, int currentKind, const QString& currentName)
{
    picker.clear();

    // Row 0 detaches the stimulus (an internal/initial transition may have none).
    picker.addItem(QCoreApplication::translate("SMStimulusPicker", "(none)"));
    picker.setItemData(0, static_cast<int>(SMTransitionEntry::eStimulusKind::Trigger), RoleKind);
    picker.setItemData(0, QString(), RoleName);

    const auto addRow = [&picker](SMTransitionEntry::eStimulusKind kind, const QString& name)
    {
        if (name.isEmpty())
        {
            return;
        }

        const SMKindGlyph::eGlyph glyph = SMKindGlyph::stimulusGlyph(kind);
        const int row = picker.count();
        picker.addItem(SMKindGlyph::icon(glyph, picker.palette().color(QPalette::Text)), displayLabel(kind, name));
        picker.setItemData(row, static_cast<int>(kind), RoleKind);
        picker.setItemData(row, name, RoleName);
        // The kind is a mark, not a word, so the tooltip is what a screen reader and a hovering
        // user get: `event NewEvent`.
        picker.setItemData(row, SMKindGlyph::word(glyph) + QLatin1Char(' ') + name, Qt::ToolTipRole);
    };

    for (const SMDocumentIndex::Stimulus& stimulus : SMDocumentIndex(data).stimuli())
    {
        addRow(stimulus.kind, stimulus.name);
    }

    if (currentName.isEmpty())
    {
        return 0;   // row 0 is "(none)"
    }

    // By DATA, never by text: two registries may legally hold the same name until validation
    // objects, and the rows no longer carry a kind prefix to tell them apart.
    for (int row = 1; row < picker.count(); ++row)
    {
        if ((picker.itemData(row, RoleKind).toInt() == currentKind)
            && (picker.itemData(row, RoleName).toString() == currentName))
        {
            return row;
        }
    }

    return 0;
}

void SMStimulusPicker::apply(StateMachineModel& model, const QComboBox& picker, uint32_t transitionId)
{
    StateMachineData& data = model.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    const int row = picker.currentIndex();
    if ((transition == nullptr) || (row < 0))
    {
        return;
    }

    const SMTransitionEntry::eStimulusKind kind =
            static_cast<SMTransitionEntry::eStimulusKind>(picker.itemData(row, RoleKind).toInt());
    const QString name = picker.itemData(row, RoleName).toString();

    // "(none)" (empty name) detaches the stimulus; the picker never renames or creates a registry
    // entry.
    if (name.isEmpty())
    {
        if (transition->getStimulus().isEmpty() == false)
        {
            model.getUndoStack().push(new SMSetStimulusCommand(data, model.getNotifier(), transitionId
                                                             , transition->getStimulusKind(), QString()
                                                             , QCoreApplication::translate("SMStimulusPicker", "Clear stimulus")));
        }

        return;
    }

    if ((kind == transition->getStimulusKind()) && (name == transition->getStimulus()))
    {
        return;
    }

    model.getUndoStack().push(new SMSetStimulusCommand(data, model.getNotifier(), transitionId, kind, name
                                                     , QCoreApplication::translate("SMStimulusPicker", "Set stimulus")));
}
