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
 *  \brief       Lusan application, FSM guard field: the editable guard surface.
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
 * \brief   The editable guard surface: a single-line code editor (visually wraps to at
 *          most four lines) that parses and resolves as the user types (150 ms debounce),
 *          colors by owner, squiggles unresolved names, and commits the resolved tree on Enter
 *          / focus-out via \ref SMGuardCommands (the D9 commit gate: a resolved tree commits,
 *          an unresolved edit persists as a draft, Esc reverts to the last committed text).
 *          Completion (\ref SMRefCompleter), the signature card (\ref SMSignatureCard) and
 *          mapping slots ride on top; the container owns the status line and fix bar and wires
 *          them to this field's signals. Rebuild-from-model is deferred (0 ms) so an undo /
 *          redo / rename never rebuilds inside the emitting command.
 *
 *          Derives QTextEdit directly (a documented deviation from the original design's literal "derives
 *          SMCodeEditor": that class is a composite widget with its own signature/note labels
 *          and completer, unsuited to a single inline field that must own its document for the
 *          highlighter and slot spans). An earlier revision used QPlainTextEdit; the base moved to QTextEdit
 *          because QPlainTextDocumentLayout never paints QTextObjectInterface objects -- the
 *          folded island tokens require the rich-text layout.
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
     *          Insert-button path): `#kind:name` for a value, or `#cond:name()` with the caret
     *          between the parens and signature help for a call. Identical to accepting the
     *          symbol in the completer, so a catalog insert and a typed reference commit the
     *          same tree (P3). A no-op when no transition is bound.
     **/
    void insertReference(const SMGuardSymbol& symbol);

    //!< Re-opens the fix bar for the caret's diagnostic (Ctrl+.).
    void reopenFixBar();

    //!< Applies a quick-fix chosen in the fix bar.
    void applyFix(const QString& id, const QString& payload);

    /**
     * \brief   The raw-collision quick-fix: binds the raw node at \p rawPath (a bare identifier
     *          that collides with a symbol \p name) to that symbol. When the name resolves to a
     *          single kind it binds directly (one undo step through \ref SMGuardCommands); when
     *          it matches several kinds it opens the same reference completer used for bare
     *          typing, filtered to that name, and binds on the pick. Advisory and never
     *          automatic: a no-op when the name no longer matches any in-scope symbol.
     **/
    void bindRaw(const QList<int>& rawPath, const QString& name);

    //!< The shared hover card (owned by the bar; nullptr disables symbol hovers).
    void setHoverCard(SMHoverCard* card);

    //!< Selects the span [start, start+length) of the CANONICAL text (lens pill click).
    void selectSpan(int start, int length);

    /**
     * \brief   The clause-popover path: visibly inserts `<combiner> <clauseText>` at
     *          the end of the field, then commits shortly after -- the novice path that
     *          teaches the syntax by showing the text it builds.
     **/
    void appendClause(const QString& combiner, const QString& clauseText);

    //!< Appends a new empty island (`+ lambda`) and requests its editor.
    void appendIsland();

    //!< Commits the current text now (the bar's ladder steps need a committed tree).
    void commitNow();

    //!< Reflows the field from the committed model NOW (no deferred turn): the click-to-insert
    //!< path needs the chip visible within the gesture that created it.
    void reflowNow();

    /**
     * \brief   Sets how many text lines the field may occupy: it never shrinks below \p minLines
     *          and never grows past \p maxLines, scrolling instead. Defaults to 3 and 12.
     **/
    void setHeightLines(int minLines, int maxLines);

    /**
     * \brief   Turns the grow-with-content height rule off (the default is on). With it off the
     *          field keeps only its 
ef setHeightLines minimum and otherwise fills the space its
     *          layout gives it -- what the large pop-out editor wants.
     **/
    void setAutoHeight(bool autoHeight);

    /**
     * \brief   Selects the FIRST unmapped-formal ghost (`<name>`) in the field, so the next
     *          keystroke fills it. Returns false when the guard has no unmapped formal.
     **/
    bool selectFirstGhost();

    /**
     * \brief   Enables / disables the implicit commit on focus-out (default enabled). The pop-out
     *          editor disables it so its Cancel button -- which blurs the field before
     *          its clicked() slot runs -- cannot silently commit the discarded text; the pop-out
     *          commits explicitly on OK instead. The base bar leaves it enabled.
     **/
    void setAutoCommit(bool enable);

    //!< The committable (expanded, slot-stripped) text -- the grid's live-refresh input.
    QString committableText() const;

    //!< The callee method name of the call whose parentheses the caret is inside (empty when the
    //!< caret is not inside any call's arguments). Lets a host bind the Arguments table to the
    //!< call the caret sits in.
    bool caretCallee(QString& name) const;

    // ---- reference chips --------------------------------------------------
    //!< The number of committed reference chips in the field, in text order.
    int chipCount() const;

    // ---- islands -----------------------------------------------------
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
    //!< An island token wants its editor: click, caret entry, `{`, or `+ lambda`.
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
    void contextMenuEvent(QContextMenuEvent* event) override;

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

    // ---- islands -----------------------------------------------------
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

    // ---- reference chips --------------------------------------------------
    //!< Folds each rendered reference span \p chips into a compact chip token. Called only
    //!< from a model-driven reflow (a committed tree), back-to-front so offsets stay valid.
    void foldChips(const QList<SMGuardRender::Chip>& chips);

    //!< Re-projects every folded chip's DISPLAY name / owner hue / prefix / reveal and its
    //!< committable body from the committed tree, IN PLACE, without reflowing the field text
    //!< : a foreign rename/retype must update chips while an uncommitted half-typed
    //!< token is preserved. A no-op when the guard is a draft or a chip was de-rendered mid-edit.
    void reprojectChips();

    //!< The document positions of every chip token, in text order.
    QList<int> chipPositions() const;

    //!< De-renders the chip at \p docPos back to its editable `#kind:name` text (double-click).
    bool deRenderChipAt(int docPos);

    //!< Reveals the chip at \p docPos (its `#kind:name` + a symbol popover) when the click at
    //!< \p viewportPos landed on the chip's leading glyph hot-zone. Returns true when handled.
    bool revealChipAt(int docPos, const QPoint& viewportPos);

    //!< Opens the island editor when the character at \p docPos is an island token.
    bool maybeOpenIslandAt(int docPos);

    //!< Caps the widget height to at most four visual lines.
    void updateHeight();

    //!< The identifier the caret sits on (for typed completion); empty when none.
    QString wordUnderCursor(int& start, int& length) const;

    // ---- reference completer ----------------------------------------------
    /**
     * \brief   Parses the `@[kind][:][name]` mention the caret sits inside. Fills \p atPos
     *          (the `@`'s document position), \p kindWord (before the colon, empty when none),
     *          \p namePart (after the colon, or the whole body when there is no colon), and
     *          \p hasColon. Returns false when the caret is not inside a mention.
     **/
    bool mentionUnderCursor(int& atPos, QString& kindWord, QString& namePart, bool& hasColon) const;

    //!< Opens / filters / hides the completer for the caret's mention (the swallow-free path).
    void updateCompleter();

    // ---- Raw-collision bind -----------------------------------------------
    //!< Replaces the Raw node at \p rawPath with a resolved reference to \p symbol (one undo
    //!< step via \ref SMGuardCommands::replaceSubtree); the field reflows from the model.
    void applyRawBind(const QList<int>& rawPath, const SMGuardSymbol& symbol);

    //!< The canonical-text span of the Raw node at \p rawPath in the committed tree; false when
    //!< the guard has no tree or the path is not addressable (used to select the token to bind).
    bool rawNodeSpan(const QList<int>& rawPath, int& start, int& length) const;

    // ---- signature help ---------------------------------------------------
    /**
     * \brief   Finds the call the caret is inside (its parentheses). Fills \p calleeName (the
     *          bare method name) and \p argIndex (the 0-based argument the caret is on, counting
     *          top-level commas). Returns false when the caret is not inside a call's arguments.
     **/
    bool callContextAtCaret(QString& calleeName, int& argIndex) const;

    //!< Shows / hides the signature-help tooltip for the caret's call. It yields to the
    //!< completer while that is open and returns when the completer closes.
    void updateSignatureHelp();

    // ---- slots ------------------------------------------------------------
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
    bool                    mAllowRaw;      //!< Unresolved fragments become raw nodes.
    bool                    mRebuildPending;//!< Coalesces deferred rebuilds.
    int                     mMinLines;      //!< Smallest field height, in text lines.
    int                     mMaxLines;      //!< Largest field height, in text lines (then it scrolls).
    bool                    mAutoHeight;    //!< True while the field sizes itself to its content.
    bool                    mSuppressAnalyze;//!< True while programmatically setting text.
    bool                    mAutoCommit;    //!< Commit on focus-out (false in the pop-out editor).

    SMGuardHighlighter*     mHighlighter;   //!< Owner colors + diagnostic underlines.
    SMRefCompleter*         mCompleter;     //!< The top-level reference completer.
    int                     mCompleterDismissedAt;//!< The sigil pos dismissed by Esc, or -1.
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

    bool                    mRawBindMode;   //!< True while the multi-kind raw-bind picker is open.
    QList<int>              mRawBindPath;    //!< The Raw node path the open picker binds on accept.
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
