#ifndef LUSAN_VIEW_SM_SMGUARDFIELD_HPP
#define LUSAN_VIEW_SM_SMGUARDFIELD_HPP
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
 *  \file        lusan/view/sm/SMGuardField.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard field: the editable guard surface (v7 B2, B3).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QPlainTextEdit>

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMFixBar.hpp"
#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QHash>
#include <QList>
#include <QString>
#include <QTextCursor>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QTimer;
class StateMachineModel;
class SMGuardHighlighter;
class SMSymbolPopup;
class SMSignatureCard;
enum class eDocElementKind;

/**
 * \class   SMGuardField
 * \brief   The editable guard surface (B2): a single-line code editor (visually wraps to at
 *          most four lines) that parses and resolves as the user types (150 ms debounce),
 *          colors by owner, squiggles unresolved names, and commits the resolved tree on Enter
 *          / focus-out via \ref SMGuardCommands (the D9 commit gate: a resolved tree commits,
 *          an unresolved edit persists as a draft, Esc reverts to the last committed text).
 *          Completion (\ref SMSymbolPopup), the signature card (\ref SMSignatureCard) and
 *          mapping slots ride on top; the container owns the status line and fix bar and wires
 *          them to this field's signals. Rebuild-from-model is deferred (0 ms) so an undo /
 *          redo / rename never rebuilds inside the emitting command.
 *
 *          Derives QPlainTextEdit directly (a documented deviation from v7's literal "derives
 *          SMCodeEditor": that class is a composite widget with its own signature/note labels
 *          and completer, unsuited to a single inline field that must own its document for the
 *          highlighter and slot spans).
 **/
class SMGuardField : public QPlainTextEdit
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardField(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMGuardField();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Points the field at a transition (0 clears it); rebuilds the catalog and the text.
    void setTransition(uint32_t transitionId);

    inline uint32_t transitionId() const;

    //!< Opens the completion catalog at the caret (Ctrl+Space).
    void openCompletion();

    //!< Re-opens the fix bar for the caret's diagnostic (Ctrl+.).
    void reopenFixBar();

    //!< Applies a quick-fix chosen in the fix bar.
    void applyFix(const QString& id, const QString& payload);

signals:
    //!< The status changed; the container updates the status line.
    void statusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& handlerChips);
    //!< The quick-fix set changed (empty message hides the bar).
    void fixesUpdated(const QString& message, const QList<SMFixBar::Fix>& fixes);
    //!< The tab badge state changed (draft => `*`, warnings => warn glyph).
    void badgeUpdated(bool isDraft, bool hasWarnings);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void insertFromMimeData(const QMimeData* source) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onDebounce();
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded();
    void onCompletionAccepted(const SMGuardSymbol& symbol);

private:
    void scheduleRebuild();
    void rebuildFromModel();
    void buildCatalog();

    //!< Parses the current text, repaints decorations, and emits status / fixes / badge.
    void analyze();

    //!< Commits the current text (resolved tree, draft, or clear) as one undo step.
    void commit();

    //!< Reverts the field to the last committed text (Esc).
    void revert();

    //!< The plain text with slot placeholders and newlines removed (never commits slot text).
    QString committableText() const;

    //!< Caps the widget height to at most four visual lines.
    void updateHeight();

    //!< The identifier the caret sits on (for typed completion); empty when none.
    QString wordUnderCursor(int& start, int& length) const;

    //!< Shows / filters the completion popup for the caret word.
    void updateCompletion();

    // ---- slots (B5) ----------------------------------------------------
    void enterSlotMode(const SMGuardSymbol& call);
    void clearSlotMode();
    void runAutoMap();
    bool selectSlot(int direction);     //!< +1 next, -1 previous; false when none left.
    int  activeSlotParam() const;       //!< The param index of the slot at the caret, or -1.
    void updateSignatureCard();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;         //!< The document facade.
    uint32_t                mTransitionId;  //!< The edited transition (0 = none).
    QString                 mCommittedText; //!< The last committed text (Esc target).
    bool                    mAllowRaw;      //!< Unresolved fragments become raw nodes (E3).
    bool                    mRebuildPending;//!< Coalesces deferred rebuilds.
    bool                    mSuppressAnalyze;//!< True while programmatically setting text.

    SMGuardHighlighter*     mHighlighter;   //!< Owner colors + diagnostic underlines.
    SMSymbolPopup*          mPopup;         //!< The completion catalog.
    SMSignatureCard*        mSignature;     //!< The call signature card.
    QTimer*                 mDebounce;      //!< The 150 ms parse debounce.

    QList<SMGuardSymbol>    mCatalog;       //!< The in-scope symbol universe.

    QString                 mLastMessage;   //!< The last fix-bar message (for Ctrl+.).
    QList<SMFixBar::Fix>    mLastFixes;     //!< The last fix set (for Ctrl+.).

    bool                    mSlotMode;      //!< True while mapping a call's arguments.
    SMGuardSymbol           mSlotCall;      //!< The call whose slots are being filled.
    QList<QTextCursor>      mSlots;         //!< One tracking cursor per argument slot.
    int                     mCurrentSlot;   //!< The active slot index, or -1.

    int                     mErrorStart;    //!< The first error span start (fix target), or -1.
    int                     mErrorLength;   //!< The first error span length.
    QHash<QString, int>     mOwnerByName;   //!< Symbol name -> NEGuardStyle::eOwner (for coloring).
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMGuardField::transitionId() const
{
    return mTransitionId;
}

#endif  // LUSAN_VIEW_SM_SMGUARDFIELD_HPP
