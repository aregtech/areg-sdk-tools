#ifndef LUSAN_VIEW_SM_SMSTATEITEM_HPP
#define LUSAN_VIEW_SM_SMSTATEITEM_HPP
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
 *  \file        lusan/view/sm/SMStateItem.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas state box item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/SMCanvasItem.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMReferences.hpp"
#include "lusan/view/sm/SMKindGlyph.hpp"
#include "lusan/view/sm/SMNoteEditor.hpp"

#include <QColor>
#include <QList>
#include <QSizeF>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QGraphicsProxyWidget;
class SMScene;

/**
 * \class   SMStateItem
 * \brief   The state box on the design canvas: a rounded rectangle split into a header
 *          (name, badges, Start/Final markers) and a body listing the state's behavior
 *          (entry operations, timers, event sends, internal transitions, exit operations).
 *          The item caches only painting data; the element ID is its single model link,
 *          and every edit it produces goes through an undo command.
 **/
class SMStateItem : public SMCanvasItem
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eRowZone
     * \brief   Where a behavior row is anchored inside the state box. Enter runs from the top of
     *          the body down, Exit is anchored to the bottom, and everything that happens WHILE
     *          in the state (Do operations, internal transitions) sits in the middle -- so the box
     *          reads in the order the state actually executes.
     **/
    enum class eRowZone
    {
          Enter
        , Middle
        , Exit
    };

    /**
     * \struct  BodyRow
     * \brief   One behavior row of the state body.
     **/
    struct BodyRow
    {
        SMKindGlyph::eGlyph         icon;           //!< The row's mark: its band, or its own kind.
        QString                     text;           //!< The row text.
        eRowZone                    zone;           //!< Where in the box the row is anchored.
        bool                        firstInGroup;   //!< First row of its Enter/Do/Exit group (carries the band mark).
        bool                        continues;      //!< Another row of the same group follows (draws a ` \` cue).
        QList<SMReferences::Ref>    refs;           //!< Declarations this row references (empty = not a navigable link).

        //!< A SECOND mark, drawn after \ref icon, or \c None. Exactly one row uses it: the
        //!< `on <stimulus>` header of an internal transition, which states two independent facts --
        //!< that an internal transition lives here (the band mark) and what kind of stimulus fires
        //!< it (this one). An operation row never carries two, which is the #543 rule.
        SMKindGlyph::eGlyph         kindIcon { SMKindGlyph::eGlyph::None };

        //!< The transition this row IS (an `on <stimulus>` header), or 0. A row that carries one
        //!< opens that transition for editing instead of navigating to its stimulus declaration.
        uint32_t                    transitionId { 0u };
    };

private:
    /**
     * \enum    eHandle
     * \brief   The resize handle under the cursor.
     **/
    enum class eHandle
    {
          None
        , TopLeft
        , Top
        , TopRight
        , Right
        , BottomRight
        , Bottom
        , BottomLeft
        , Left
    };

    /**
     * \struct  RowSlot
     * \brief   One drawn body row after band packing: its index into \ref mRows, its top Y in item
     *          coordinates, and whether it is the truncation ("...") slot. The single source of the
     *          body layout, shared by painting and Ctrl+Shift link hit-testing so they never drift.
     **/
    struct RowSlot
    {
        int     index;      //!< The row's index into mRows.
        double  y;          //!< The row's top Y in item coordinates.
        bool    truncated;  //!< True when this slot draws the "..." overflow marker (not a link).
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the box item of a state element.
     * \param   stateId The state's document element ID.
     * \param   parent  The parent graphics item.
     **/
    explicit SMStateItem(uint32_t stateId, QGraphicsItem* parent = nullptr);
    virtual ~SMStateItem();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The behavior rows of the body, in display order, exactly as painted. Read-only: the
    //!< canvas tests assert the rows the user reads, not the pixels they are drawn as.
    inline const QList<BodyRow>& getBodyRows() const;

    /**
     * \brief   The actionable body row drawn at \p pos (item coordinates), or \c nullptr. A row is
     *          actionable when it references a declaration or when it is an internal transition's
     *          header. The canvas context menu asks this so its entries name what is under the
     *          pointer, the same rows the Ctrl+Shift link gesture acts on.
     **/
    const BodyRow* bodyRowAtPos(const QPointF& pos) const;

    /**
     * \brief   Opens the in-place name editor over the header. Commit pushes an undoable
     *          rename; invalid or duplicate names are rejected inline; Esc cancels.
     **/
    void startInlineRename();

    /**
     * \brief   True while the in-place name editor is open.
     **/
    inline bool isRenameActive() const;

    /**
     * \brief   Sets the painted name to a transient preview value (no model change), used to
     *          mirror live typing in the Properties panel name field onto the canvas box in
     *          real time. The next updateFromModel() (on commit) or a restore preview (on
     *          reject / cancel) makes the box authoritative again.
     **/
    void setNamePreview(const QString& name);

    /**
     * \brief   Opens the in-place note editor over the box for the note bound to this state;
     *          commit (focus-out) pushes an undoable text change, then collapses back to the
     *          corner note badge. No-op when the state has no bound note.
     **/
    void startNoteEdit();

    /**
     * \brief   True when a note is bound to this state (a note badge is shown).
     **/
    inline bool hasNote() const;

    /**
     * \brief   Toggles the body collapse through an undo command.
     **/
    void toggleExpanded();

    /**
     * \brief   True while the body is shown; false when only the header is left.
     **/
    inline bool isExpanded() const;

    /**
     * \brief   The history mark drawn in the header (None = no badge). It sits in the header,
     *          so collapsing the body never takes it away.
     **/
    inline SMStateEntry::eHistory getHistoryBadge() const;

    /**
     * \brief   The box geometry (position and size) in scene coordinates.
     **/
    QRectF getBoxGeometry() const;

    /**
     * \brief   True when a scene position lies in the border band of the box (and not on a
     *          resize handle, the chevron, or the rename editor): the zone from which a
     *          transition drag starts with the Select tool.
     **/
    bool isBorderDragZone(const QPointF& scenePos) const;

    /**
     * \brief   The drawn corner radius of the box: the pill radius for Start / Final
     *          marker boxes, the standard state corner radius otherwise. Edge anchors
     *          use it to sit exactly on the drawn border.
     **/
    double boxCornerRadius() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /**
     * \brief   Re-reads the state element and its Node layout entry: name, kind, badges,
     *          colors, behavior rows, geometry, and the collapsed flag.
     **/
    virtual void updateFromModel() override;

    /**
     * \brief   Ends the rename and the bound-note editor, whichever is open.
     **/
    virtual void finishInlineEdit() override;

protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns the owning canvas scene, or nullptr.
     **/
    SMScene* getCanvas() const;

    /**
     * \brief   Returns the rendered state element, or nullptr.
     **/
    const SMStateEntry* getState() const;

    /**
     * \brief   The height the box is painted with (header only when collapsed).
     **/
    double visibleHeight() const;

    /**
     * \brief   Rebuilds the cached behavior rows from the state element.
     **/
    void rebuildRows(const SMStateEntry& state);

    /**
     * \brief   The drawn body rows after band packing (entry at top, exit at bottom, middle
     *          centered, overflow truncated), each with its item-coordinate top Y. The shared
     *          layout backing both paintBodyRows and Ctrl+Shift link hit-testing. Empty when the
     *          box is collapsed, a marker, or has no body content.
     **/
    QList<RowSlot> bodyRowLayout() const;

    /**
     * \brief   The index (into mRows) of the navigable body row under an item-coordinate point, or
     *          -1 when the point is over no row, over the truncation slot, or over a row that
     *          references nothing. Used to underline and to route the Ctrl+Shift link click.
     **/
    int bodyRowAt(const QPointF& pos) const;

    /**
     * \brief   Paints the header: markers, name, badges, and the collapse chevron.
     **/
    void paintHeaderContent(QPainter* painter, const QRectF& box, const QColor& headerColor);

    /**
     * \brief   Paints the behavior rows into the body area.
     **/
    void paintBodyRows(QPainter* painter, const QRectF& box, const QColor& bodyColor);

    /**
     * \brief   The bottom-right body corner the submachine hint occupies, in item coordinates, and
     *          the hot zone of the Ctrl+Alt quick view. Null when nothing is drawn there: the state
     *          has no real submachine, is collapsed, or is too short to give the corner away.
     **/
    QRectF miniatureRect() const;

    /**
     * \brief   Paints the submachine hint of a composite state into the bottom-right body corner:
     *          a FIXED symbol (a Start marker and two states), not the substates themselves --
     *          see the note in the definition.
     **/
    void paintMiniature(QPainter* painter, const QRectF& box, const QColor& bodyColor);

    /**
     * \brief   Paints the resize handles of the selected box.
     **/
    void paintHandles(QPainter* painter, const QPalette& palette);

    /**
     * \brief   The resize handle at an item-local position (None when not selected).
     **/
    eHandle hitHandle(const QPointF& position) const;

    /**
     * \brief   The rectangle of one resize handle in item coordinates.
     **/
    QRectF handleRect(eHandle handle) const;

    /**
     * \brief   False for the vertical handles of a collapsed box (its stored height
     *          must stay untouched while only the header is visible).
     **/
    bool isHandleEnabled(eHandle handle) const;

    /**
     * \brief   The collapse/expand chevron rectangle in the header.
     **/
    QRectF chevronRect() const;

    /**
     * \brief   The note-badge rectangle in the top-right corner (valid only when the state
     *          has a bound note); used for painting and click-to-edit hit testing.
     **/
    QRectF noteBadgeRect() const;

    /**
     * \brief   Paints the note badge (a small folded-page glyph) in the top-right corner.
     **/
    void paintNoteBadge(QPainter* painter, const QColor& color);

    /**
     * \brief   True when the body has rows to show (the chevron is useful).
     **/
    inline bool hasBodyContent() const;

    /**
     * \brief   True for the compact Start / Final marker boxes (pill rendering,
     *          no header band, no body rows).
     **/
    inline bool isMarker() const;

    /**
     * \brief   Paints the compact Start / Final marker box: a pill with the kind's
     *          fill color, a glyph, and the centered name (Final adds an inner ring).
     **/
    void paintMarker(QPainter* painter, const QRectF& box, const QPalette& palette);

    /**
     * \brief   Applies an interactive resize drag to the given scene position.
     **/
    void applyResizeDrag(const QPointF& scenePos);

    /**
     * \brief   Pushes the finished resize gesture as one undo command.
     **/
    void commitResize();

    /**
     * \brief   Validates a candidate name: identifier syntax and document-wide
     *          uniqueness. Returns an empty string when valid, the reason otherwise.
     **/
    QString validateName(const QString& name) const;

    /**
     * \brief   Pushes the rename command when the committed name differs.
     **/
    void commitRename(const QString& name);

    /**
     * \brief   Ends an open rename: commits a valid name, restores the committed one otherwise.
     *          QLineEdit withholds `editingFinished` on focus-out while its validator rejects
     *          the text, so the editor cannot rely on that signal alone to end an edit.
     **/
    void finishRename(bool immediate = false);

    /**
     * \brief   Closes and destroys the in-place name editor.
     * \param   immediate   Destroys the proxy now instead of deferring it. The deferred path is
     *                      required when closing from inside the editor's own signal, but a
     *                      proxy still alive after a tool armed keeps its cursor on the viewport.
     **/
    void closeRenameEditor(bool immediate = false);


    /**
     * \brief   Re-anchors the transitions connected to this box after it moved or resized.
     **/
    void notifyEdgesOfGeometry();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QSizeF                      mSize;          //!< The box size from the Node layout entry.
    QString                     mName;          //!< The state name.
    SMStateEntry::eStateKind    mKind;          //!< The state kind.
    SMStateEntry::eHistory      mHistory;       //!< The history badge mode.
    bool                        mComposite;     //!< The state owns painted substates.
    bool                        mImported;      //!< The state hosts an imported submachine.
    QString                     mSubmachine;    //!< The hosted import alias, shown on the badge.
    bool                        mExpanded;      //!< The body is expanded.
    int                         mActionSeverity;//!< Entry/exit mapping severity (NEGuardStyle), or -1 (clean).
    QString                     mColorName;     //!< The persisted body color (empty = theme).
    QString                     mHeaderColorName; //!< The persisted header color (empty = derived).
    QList<BodyRow>              mRows;          //!< The behavior rows, in display order.
    bool                        mHasNote;       //!< A note is bound to this state (badge shown).
    eHandle                     mResizeHandle;  //!< The handle grabbed by the resize drag.
    QRectF                      mResizeStart;   //!< The box scene geometry at resize start.
    QGraphicsProxyWidget*       mRenameProxy;   //!< The open in-place name editor, or nullptr.
    bool                        mClosingRename; //!< Guards re-entrant editor teardown.
    int                         mHoverRow;      //!< Body row underlined as a link under Ctrl+Shift hover, or -1.

    //!< True between the press and the release of a Ctrl+Shift link click. The press is consumed
    //!< without calling the base, so the base never records the press -- and Qt then treats the
    //!< release as a click on nothing and CLEARS the scene selection, which put the Properties
    //!< panel back to "No selection" right after the link had filled it. The release is consumed too.
    bool                        mLinkClick;
    SMNoteEditor                mNoteEditor;    //!< The open in-place note editor (if any).
};

//////////////////////////////////////////////////////////////////////////
// SMStateItem inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMStateItem::isRenameActive() const
{
    return (mRenameProxy != nullptr);
}

inline bool SMStateItem::hasNote() const
{
    return mHasNote;
}

inline bool SMStateItem::hasBodyContent() const
{
    return (mRows.isEmpty() == false) && (isMarker() == false);
}

inline bool SMStateItem::isMarker() const
{
    return (mKind != SMStateEntry::eStateKind::Normal);
}

inline const QList<SMStateItem::BodyRow>& SMStateItem::getBodyRows() const
{
    return mRows;
}

inline bool SMStateItem::isExpanded() const
{
    return mExpanded;
}

inline SMStateEntry::eHistory SMStateItem::getHistoryBadge() const
{
    return mHistory;
}

#endif  // LUSAN_VIEW_SM_SMSTATEITEM_HPP
