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
 *  \file        lusan/view/sm/SMGuardPopout.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard pop-out: a bigger multiline guard editor (SM-21-05).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardPopout.hpp"

#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMGuardField.hpp"
#include "lusan/view/sm/SMGuardStatusLine.hpp"
#include "lusan/view/sm/SMIslandEditor.hpp"

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardPopout::SMGuardPopout(StateMachineModel& model, uint32_t transitionId, QWidget* parent /*= nullptr*/)
    : QWidget   (parent, Qt::Tool | Qt::WindowStaysOnTopHint)
    , mModel    (model)
    , mTransId  (transitionId)
    , mField    (nullptr)
    , mStatus   (nullptr)
    , mIsland   (nullptr)
{
    setObjectName(QStringLiteral("smGuardPopout"));
    setWindowTitle(tr("Edit guard"));
    // Closing the window (OK / Cancel / the close box) destroys it; the bar tracks it by QPointer.
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(6);

    // The body is its OWN SMGuardField (its own QTextDocument -- the two highlighters and slot
    // cursors never collide, per the hazard). Auto-commit-on-focus-out is disabled so Cancel (which
    // blurs the field before its slot runs) cannot slip the discarded text into the model; OK
    // commits explicitly instead.
    mField = new SMGuardField(mModel, this);
    mField->setObjectName(QStringLiteral("smGuardPopoutField"));
    mField->setAutoCommit(false);
    outer->addWidget(mField);

    mStatus = new SMGuardStatusLine(this);
    outer->addWidget(mStatus);

    // S4: the island editor, hidden until an island opens (identical to the base bar).
    mIsland = new SMIslandEditor(this);
    mIsland->setVisible(false);
    outer->addWidget(mIsland);

    outer->addStretch(1);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setObjectName(QStringLiteral("smGuardPopoutButtons"));
    QPushButton* okButton = buttons->button(QDialogButtonBox::Ok);
    if (okButton != nullptr)
    {
        okButton->setObjectName(QStringLiteral("smGuardPopoutOk"));
    }

    QPushButton* cancelButton = buttons->button(QDialogButtonBox::Cancel);
    if (cancelButton != nullptr)
    {
        cancelButton->setObjectName(QStringLiteral("smGuardPopoutCancel"));
    }

    outer->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &SMGuardPopout::onCommit);
    connect(buttons, &QDialogButtonBox::rejected, this, &SMGuardPopout::close);   // discard: WA_DeleteOnClose

    // Mirror the base bar's status line (connected BEFORE the seeding reflow so the first status shows).
    connect(mField, &SMGuardField::statusUpdated, this, &SMGuardPopout::onStatusUpdated);

    // Islands work identically: open the editor, edit the body into the field's token, close. The
    // body edits stay a draft carried by the eventual OK commit (no immediate commit -- the pop-out
    // gates every edit behind OK/Cancel). The `Name it...` / `Move to handler...` ladder is a
    // bar-owned flow, so it is forwarded to the bar which runs it over the shared model.
    connect(mField, &SMGuardField::islandEditRequested, this, [this](int islandIndex, const QString& body)
    {
        mIsland->openFor(mModel, mTransId, islandIndex, body);
    });
    connect(mIsland, &SMIslandEditor::bodyCommitted, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
    });
    connect(mIsland, &SMIslandEditor::closeRequested, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mIsland->hide();
        mField->setFocus();
    });
    connect(mIsland, &SMIslandEditor::nameRequested, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mIsland->hide();
        emit nameIslandRequested(islandIndex, body, false);
    });
    connect(mIsland, &SMIslandEditor::moveToHandlerRequested, this, [this](int islandIndex, const QString& body)
    {
        mField->setIslandBody(islandIndex, body);
        mIsland->hide();
        emit nameIslandRequested(islandIndex, body, true);
    });

    // Seed from the base field's committable text: the bar commits the base before opening, so a
    // reflow from the model shows exactly what the base showed -- chips and islands folded the same
    // way (identical to a fresh commit render).
    mField->setTransition(mTransId);

    resize(560, 220);
}

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////

void SMGuardPopout::onCommit()
{
    // OK: commit through the standard text -> parse -> SMGuardCommands path as ONE undoable command
    // (SMGuardField::commitNow -> commit). The base bar's field reflows from the model on the
    // resulting elementChanged(transitionId). A no-change edit pushes nothing.
    mField->commitNow();
    close();    // WA_DeleteOnClose -> closeEvent emits closed().
}

void SMGuardPopout::onStatusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& chips)
{
    if (verdict.isEmpty())
    {
        mStatus->clearStatus();
    }
    else
    {
        mStatus->setStatus(static_cast<NEGuardStyle::eSeverity>(severity), verdict, preview, chips);
    }
}

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////

void SMGuardPopout::closeEvent(QCloseEvent* event)
{
    emit closed();      // the bar restores the base field (read-only -> editable) and refocuses it.
    QWidget::closeEvent(event);
}
