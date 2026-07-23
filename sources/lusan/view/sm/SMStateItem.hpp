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
     * \enum    eRowIcon
     * \brief   The glyph kind of one behavior row in the state body.
     **/
    enum class eRowIcon
    {
          Entry     //!< A generic entry operation: an arrow running INTO a bar, `->|`.
        , Exit      //!< A generic exit operation: an arrow running away from a bar, `<-|`.
        , ExitAlt   //!< The alternative exit glyph, `|<-` -- see \c ExitRowIcon in the .cpp.
        , TimerStart//!< A timer start (clock + play).
        , TimerStop //!< A timer stop (clock + square).
        , Event     //!< An event send/trigger.
        , Internal  //!< An internal transition, and a generic Do operation (a self-loop).
    };

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
     * \struct  BodyRow
     * \brief   One behavior row of the state body.
     **/
    struct BodyRow
    {
        eRowIcon    icon;   //!< The glyph kind.
        QString     text;   //!< The row text.
        eRowZone    zone;   //!< Where in the box the row is anchored.
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
     * \brief   Paints the header: markers, name, badges, and the collapse chevron.
     **/
    void paintHeaderContent(QPainter* painter, const QRectF& box, const QColor& headerColor);

    /**
     * \brief   Paints the behavior rows into the body area.
     **/
    void paintBodyRows(QPainter* painter, const QRectF& box, const QColor& bodyColor);

    /**
     * \brief   Paints the submachine miniature hint of a composite state into the
     *          bottom-right body corner: the nested level's node boxes, scaled to fit.
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
     * \brief   Closes and destroys the in-place name editor.
     **/
    void closeRenameEditor();

    /**
     * \brief   Toggles the body collapse through an undo command.
     **/
    void toggleExpanded();

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
    bool                        mExpanded;      //!< The body is expanded.
    int                         mActionSeverity;//!< Entry/exit mapping severity (NEGuardStyle), or -1 (clean).
    QString                     mColorName;     //!< The persisted body color (empty = theme).
    QString                     mHeaderColorName; //!< The persisted header color (empty = derived).
    QList<BodyRow>              mRows;          //!< The behavior rows, in display order.
    QList<QRectF>               mMiniature;     //!< The nested level's node boxes (scene units).
    bool                        mHasNote;       //!< A note is bound to this state (badge shown).
    eHandle                     mResizeHandle;  //!< The handle grabbed by the resize drag.
    QRectF                      mResizeStart;   //!< The box scene geometry at resize start.
    QGraphicsProxyWidget*       mRenameProxy;   //!< The open in-place name editor, or nullptr.
    bool                        mClosingRename; //!< Guards re-entrant editor teardown.
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

#endif  // LUSAN_VIEW_SM_SMSTATEITEM_HPP
