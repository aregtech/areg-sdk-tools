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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
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
          Entry     //!< An entry operation (into-the-state arrow).
        , Timer     //!< A timer start/stop.
        , Event     //!< An event send.
        , Internal  //!< An internal transition.
        , Exit      //!< An exit operation (out-of-the-state arrow).
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
     * \brief   The box geometry (position and size) in scene coordinates.
     **/
    QRectF getBoxGeometry() const;

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
     * \brief   True when the body has rows to show (the chevron is useful).
     **/
    inline bool hasBodyContent() const;

    /**
     * \brief   Applies an interactive resize drag to the given scene position.
     **/
    void applyResizeDrag(const QPointF& scenePos);

    /**
     * \brief   Pushes the finished resize gesture as one undo command.
     **/
    void commitResize();

    /**
     * \brief   Pushes one undo step moving every selected state box whose item position
     *          differs from its Node layout entry (the finished drag gesture).
     **/
    void commitMoveGesture();

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
    QString                     mColorName;     //!< The persisted body color (empty = theme).
    QString                     mHeaderColorName; //!< The persisted header color (empty = derived).
    QList<BodyRow>              mRows;          //!< The behavior rows, in display order.
    eHandle                     mResizeHandle;  //!< The handle grabbed by the resize drag.
    QRectF                      mResizeStart;   //!< The box scene geometry at resize start.
    QGraphicsProxyWidget*       mRenameProxy;   //!< The open in-place name editor, or nullptr.
    bool                        mClosingRename; //!< Guards re-entrant editor teardown.
};

//////////////////////////////////////////////////////////////////////////
// SMStateItem inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMStateItem::isRenameActive() const
{
    return (mRenameProxy != nullptr);
}

inline bool SMStateItem::hasBodyContent() const
{
    return (mRows.isEmpty() == false);
}

#endif  // LUSAN_VIEW_SM_SMSTATEITEM_HPP
