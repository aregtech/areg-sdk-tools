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
#include <QTextEdit>

#include "lusan/model/sm/SMGuardRender.hpp"
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
class SMHoverCard;
class SMInlineToken;
class SMRefCompleter;
class SMSignatureCard;
enum class eDocElementKind;

/**
 * \class   SMGuardField
 * \brief   The editable guard surface (B2): a single-line code editor (visually wraps to at
 *          most four lines) that parses and resolves as the user types (150 ms debounce),
 *          colors by owner, squiggles unresolved names, and commits the resolved tree on Enter
 *          / focus-out via \ref SMGuardCommands (the D9 commit gate: a resolved tree commits,
 *          an unresolved edit persists as a draft, Esc reverts to the last committed text).
 *          Completion (\ref SMRefCompleter), the signature card (\ref SMSignatureCard) and
 *          mapping slots ride on top; the container owns the status line and fix bar and wires
 *          them to this field's signals. Rebuild-from-model is deferred (0 ms) so an undo /
 *          redo / rename never rebuilds inside the emitting command.
 *
 *          Derives QTextEdit directly (a documented deviation from v7's literal "derives
 *          SMCodeEditor": that class is a composite widget with its own signature/note labels
 *          and completer, unsuited to a single inline field that must own its document for the
 *          highlighter and slot spans). U2 used QPlainTextEdit; U3 moved the base to QTextEdit
 *          because QPlainTextDocumentLayout never paints QTextObjectInterface objects -- the
 *          folded island tokens (E4) require the rich-text layout.
 **/
class SMGuardField : public QTextEdit
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

    /**
     * \brief   Inserts \p symbol's canonical reference at the caret (the Data-catalog and
     *          Insert-button path): `@kind:name` for a value, or `@cond:name()` with the caret
     *          between the parens and signature help for a call. Identical to accepting the
     *          symbol in the completer, so a catalog insert and a typed reference commit the
     *          same tree (P3). A no-op when no transition is bound.
     **/
    void insertReference(const SMGuardSymbol& symbol);

    //!< Re-opens the fix bar for the caret's diagnostic (Ctrl+.).
    void reopenFixBar();

    //!< Applies a quick-fix chosen in the fix bar.
    void applyFix(const QString& id, const QString& payload);

    //!< The shared hover card (owned by the bar; nullptr disables symbol hovers).
    void setHoverCard(SMHoverCard* card);

    //!< Selects the span [start, start+length) of the CANONICAL text (lens pill click).
    void selectSpan(int start, int length);

    /**
     * \brief   The clause-popover path (B7): visibly inserts `<combiner> <clauseText>` at
     *          the end of the field, then commits shortly after -- the novice path that
     *          teaches the syntax by showing the text it builds.
     **/
    void appendClause(const QString& combiner, const QString& clauseText);

    //!< Appends a new empty island (`+ lambda`) and requests its editor.
    void appendIsland();

    //!< Commits the current text now (the bar's ladder steps need a committed tree).
    void commitNow();

    //!< The committable (expanded, slot-stripped) text -- the grid's live-refresh input.
    QString committableText() const;

    // ---- reference chips (SM-21-03) --------------------------------------
    //!< The number of committed reference chips in the field, in text order.
    int chipCount() const;

    // ---- islands (E4) ----------------------------------------------------
    //!< The number of island tokens in the field, in text order.
    int islandCount() const;

    //!< The body of island \p index (empty when out of range).
    QString islandBody(int index) const;

    //!< Rewrites the body of island \p index (the island editor's apply path).
    void setIslandBody(int index, const QString& body);

signals:
    //!< The status changed; the container updates the status line.
    void statusUpdated(int severity, const QString& verdict, const QString& preview, const QStringList& handlerChips);
    //!< The quick-fix set changed (empty message hides the bar).
    void fixesUpdated(const QString& message, const QList<SMFixBar::Fix>& fixes);
    //!< The tab badge state changed (draft => `*`, warnings => warn glyph).
    void badgeUpdated(bool isDraft, bool hasWarnings);
    //!< An island token wants its editor (S4): click, caret entry, `{`, or `+ lambda`.
    void islandEditRequested(int islandIndex, const QString& body);
    //!< The `Map arguments...` quick-fix: the bar opens the grid for the first call.
    void mapArgumentsRequested();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void insertFromMimeData(const QMimeData* source) override;
    QMimeData* createMimeDataFromSelection() const override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onDebounce();
    void onElementAdded(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded();
    void onCompleterAccepted(const SMGuardSymbol& symbol);

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

    // ---- islands (E4) ----------------------------------------------------
    //!< The document positions of every island token, in text order.
    QList<int> islandPositions() const;

    //!< Inserts an island token carrying \p body at the caret; returns its island index.
    int insertIslandAtCaret(const QString& body);

    /**
     * \brief   The expanded text: island tokens replaced by `{body}`. \p docStarts maps
     *          each document position to its expanded offset (size = doc length + 1), so
     *          parser diagnostics and lens spans translate both ways.
     **/
    QString expandedText(QList<int>& docStarts) const;

    //!< The document position of expanded offset \p expandedPos.
    int expandedToDoc(int expandedPos) const;

    //!< Folds every textual `{...}` block of the document into an island token.
    void foldIslands();

    // ---- reference chips (SM-21-03) --------------------------------------
    //!< Folds each rendered reference span \p chips into a compact chip token. Called only
    //!< from a model-driven reflow (a committed tree), back-to-front so offsets stay valid.
    void foldChips(const QList<SMGuardRender::Chip>& chips);

    //!< The document positions of every chip token, in text order.
    QList<int> chipPositions() const;

    //!< De-renders the chip at \p docPos back to its editable `@kind:name` text (double-click).
    bool deRenderChipAt(int docPos);

    //!< Reveals the chip at \p docPos (its `@kind:name` + a symbol popover) when the click at
    //!< \p viewportPos landed on the chip's leading glyph hot-zone. Returns true when handled.
    bool revealChipAt(int docPos, const QPoint& viewportPos);

    //!< Opens the island editor when the character at \p docPos is an island token.
    bool maybeOpenIslandAt(int docPos);

    //!< Caps the widget height to at most four visual lines.
    void updateHeight();

    //!< The identifier the caret sits on (for typed completion); empty when none.
    QString wordUnderCursor(int& start, int& length) const;

    // ---- reference completer (SM-21-03) ----------------------------------
    /**
     * \brief   Parses the `@[kind][:][name]` mention the caret sits inside. Fills \p atPos
     *          (the `@`'s document position), \p kindWord (before the colon, empty when none),
     *          \p namePart (after the colon, or the whole body when there is no colon), and
     *          \p hasColon. Returns false when the caret is not inside a mention.
     **/
    bool mentionUnderCursor(int& atPos, QString& kindWord, QString& namePart, bool& hasColon) const;

    //!< Opens / filters / hides the completer for the caret's mention (the swallow-free path).
    void updateCompleter();

    // ---- signature help (SM-21-03) ---------------------------------------
    /**
     * \brief   Finds the call the caret is inside (its parentheses). Fills \p calleeName (the
     *          bare method name) and \p argIndex (the 0-based argument the caret is on, counting
     *          top-level commas). Returns false when the caret is not inside a call's arguments.
     **/
    bool callContextAtCaret(QString& calleeName, int& argIndex) const;

    //!< Shows / hides the signature-help tooltip for the caret's call. It yields to the
    //!< completer while that is open and returns when the completer closes (D-PRECEDENCE).
    void updateSignatureHelp();

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
    SMRefCompleter*         mCompleter;     //!< The top-level reference completer (D-POPUP).
    int                     mCompleterDismissedAt;//!< The '@' pos dismissed by Esc, or -1 (D-ESC).
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

    SMInlineToken*          mTokenHandler;  //!< The registered island text-object painter.
    SMHoverCard*            mHover;         //!< The shared hover card (not owned; may be nullptr).
    QTimer*                 mHoverTimer;    //!< The 300 ms symbol-hover delay.
    QString                 mHoverWord;     //!< The word under the pending hover.
    int                     mLastCursorPos; //!< The previous caret position (island caret-entry).
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMGuardField::transitionId() const
{
    return mTransitionId;
}

#endif  // LUSAN_VIEW_SM_SMGUARDFIELD_HPP
