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
 *  \brief       Lusan application, FSM guard pop-out: a bigger multiline guard editor.
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
#include <QGuiApplication>
#include <QMdiArea>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

namespace
{
    /**
     * rief   Centers \p window over the MDI area that hosts \p anchor (the document surface the
     *          user is looking at), falling back to the anchor's own window and then to the primary
     *          screen. The result is clamped to the available screen area, so the whole dialog --
     *          including its OK / Cancel row -- is always on screen.
     **/
    void centerOnHost(QWidget* window, QWidget* anchor)
    {
        QRect host;
        for (QWidget* walk = anchor; walk != nullptr; walk = walk->parentWidget())
        {
            QMdiArea* mdi = qobject_cast<QMdiArea*>(walk);
            if (mdi != nullptr)
            {
                host = QRect(mdi->viewport()->mapToGlobal(QPoint(0, 0)), mdi->viewport()->size());
                break;
            }
        }

        if (host.isEmpty() && (anchor != nullptr) && (anchor->window() != nullptr))
        {
            host = anchor->window()->frameGeometry();
        }

        QScreen* screen = (anchor != nullptr) ? anchor->screen() : QGuiApplication::primaryScreen();
        const QRect available = (screen != nullptr) ? screen->availableGeometry() : host;
        if (host.isEmpty())
        {
            host = available;
        }

        QRect target(QPoint(0, 0), window->frameSize().isEmpty() ? window->size() : window->frameSize());
        target.moveCenter(host.center());

        // Never let an edge fall off the screen; shrink first if the dialog is simply too big.
        if (target.width() > available.width())     { target.setWidth(available.width()); }
        if (target.height() > available.height())   { target.setHeight(available.height()); }
        if (target.right() > available.right())     { target.moveRight(available.right()); }
        if (target.bottom() > available.bottom())   { target.moveBottom(available.bottom()); }
        if (target.left() < available.left())       { target.moveLeft(available.left()); }
        if (target.top() < available.top())         { target.moveTop(available.top()); }

        window->setGeometry(target);
    }
}

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
    // The large editor fills the window: a tall, multi-line writing area is the whole point of
    // popping the guard out.
    mField->setAutoHeight(false);
    mField->setHeightLines(8, 8);
    mField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    outer->addWidget(mField, 1);

    mStatus = new SMGuardStatusLine(this);
    outer->addWidget(mStatus);

    // The island editor, hidden until an island opens (identical to the base bar).
    mIsland = new SMIslandEditor(this);
    mIsland->setVisible(false);
    outer->addWidget(mIsland);

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

    // The pop-out IS the large editor: a field tall enough for a multi-line guard, and a window
    // centered on the design surface rather than dropped at the parent's corner where its right
    // edge fell off the screen.
    resize(760, 420);
    centerOnHost(this, parent);
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
