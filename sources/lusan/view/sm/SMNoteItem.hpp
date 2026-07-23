#ifndef LUSAN_VIEW_SM_SMNOTEITEM_HPP
#define LUSAN_VIEW_SM_SMNOTEITEM_HPP
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
 *  \file        lusan/view/sm/SMNoteItem.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas note item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/SMCanvasItem.hpp"

#include <QSizeF>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QGraphicsProxyWidget;
class SMScene;

/**
 * \class   SMNoteItem
 * \brief   A diagram-only text annotation on the design canvas: a colored rounded box with
 *          wrapped text, movable and resizable via a single corner handle, with inline text
 *          editing. Notes reference no model element and are semantically inert;
 *          the element ID is the item's single link to its `Note` layout entry, and every
 *          edit it produces goes through an undo command.
 **/
class SMNoteItem : public SMCanvasItem
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the box item of a note element.
     * \param   noteId  The note's document element ID.
     * \param   parent  The parent graphics item.
     **/
    explicit SMNoteItem(uint32_t noteId, QGraphicsItem* parent = nullptr);
    virtual ~SMNoteItem();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The box geometry (position and size) in scene coordinates.
     **/
    QRectF getBoxGeometry() const;

    /**
     * \brief   Opens the in-place multi-line text editor. Commit pushes an undoable text
     *          change on focus-out; Esc cancels and restores the previous text.
     **/
    void startInlineEdit();

    /**
     * \brief   True while the in-place text editor is open.
     **/
    inline bool isEditActive() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /**
     * \brief   Re-reads the note's layout entry: position, size, color, and text.
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
     * \brief   The bottom-right resize handle rectangle in item coordinates.
     **/
    QRectF handleRect() const;

    /**
     * \brief   True when a position lies within the resize handle (only while selected).
     **/
    bool hitHandle(const QPointF& position) const;

    /**
     * \brief   Applies an interactive resize drag to the given scene position.
     **/
    void applyResizeDrag(const QPointF& scenePos);

    /**
     * \brief   Pushes the finished resize gesture as one undo command.
     **/
    void commitResize();

    /**
     * \brief   Pushes the text-change command when the committed text differs.
     **/
    void commitText(const QString& text);

    /**
     * \brief   Closes and destroys the in-place text editor.
     **/
    void closeEditEditor();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QSizeF                  mSize;          //!< The box size from the Note layout entry.
    QString                 mText;          //!< The note text.
    QString                 mColorName;     //!< The persisted note color (empty = default).
    bool                    mResizing;      //!< A corner-handle resize drag is in progress.
    QRectF                  mResizeStart;   //!< The box scene geometry at resize start.
    QGraphicsProxyWidget*   mEditProxy;     //!< The open in-place text editor, or nullptr.
    bool                    mClosingEdit;   //!< Guards re-entrant editor teardown.
};

//////////////////////////////////////////////////////////////////////////
// SMNoteItem inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMNoteItem::isEditActive() const
{
    return (mEditProxy != nullptr);
}

#endif  // LUSAN_VIEW_SM_SMNOTEITEM_HPP
