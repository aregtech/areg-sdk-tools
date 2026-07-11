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
 *  \file        lusan/view/sm/SMNoteItem.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design canvas note item.
 *
 ************************************************************************/

#include "lusan/view/sm/SMNoteItem.hpp"

#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMScene.hpp"

#include <QCoreApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>

#include <algorithm>
#include <functional>

namespace
{
    inline QString translate(const char* text)
    {
        return QCoreApplication::translate("SMNoteItem", text);
    }

    /**
     * \brief   The in-place multi-line text editor: Esc cancels, focus-out commits.
     *          Enter/Return inserts a newline (notes are free-form multi-line text).
     **/
    class NoteEdit : public QPlainTextEdit
    {
    public:
        std::function<void()>  mCommit;    //!< Commits the current text.
        std::function<void()>  mCancel;    //!< Abandons the edit, restoring the previous text.

        explicit NoteEdit(const QString& text)
            : QPlainTextEdit(text)
        {
        }

    protected:
        virtual void keyPressEvent(QKeyEvent* event) override
        {
            if (event->key() == Qt::Key_Escape)
            {
                event->accept();
                if (mCancel)
                {
                    mCancel();
                }

                return;
            }

            QPlainTextEdit::keyPressEvent(event);
        }

        virtual void focusOutEvent(QFocusEvent* event) override
        {
            QPlainTextEdit::focusOutEvent(event);
            if (mCommit)
            {
                mCommit();
            }
        }
    };
}

SMNoteItem::SMNoteItem(uint32_t noteId, QGraphicsItem* parent /*= nullptr*/)
    : SMCanvasItem  (noteId, parent)
    , mSize         (NESMDesign::NoteDefaultWidth, NESMDesign::NoteDefaultHeight)
    , mText         ( )
    , mColorName    ( )
    , mResizing     (false)
    , mResizeStart  ( )
    , mEditProxy    (nullptr)
    , mClosingEdit  (false)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
    setZValue(-0.5);   // Above the grid, but under state boxes and edges.
}

SMNoteItem::~SMNoteItem()
{
}

QRectF SMNoteItem::getBoxGeometry() const
{
    return QRectF(pos(), mSize);
}

QRectF SMNoteItem::boundingRect() const
{
    const double margin = NESMDesign::NoteHandleSize;
    return QRectF(-margin, -margin, mSize.width() + 2.0 * margin, mSize.height() + 2.0 * margin);
}

QPainterPath SMNoteItem::shape() const
{
    QPainterPath path;
    path.addRoundedRect(QRectF(0.0, 0.0, mSize.width(), mSize.height()), NESMDesign::NoteCornerRadius, NESMDesign::NoteCornerRadius);
    return path;
}

void SMNoteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* widget)
{
    const QPalette palette{ (widget != nullptr) ? widget->palette() : QPalette() };
    const QRectF   box{ 0.0, 0.0, mSize.width(), mSize.height() };

    QColor fill{ mColorName };
    if (fill.isValid() == false)
    {
        fill = NESMDesign::noteColor(palette);
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addRoundedRect(box, NESMDesign::NoteCornerRadius, NESMDesign::NoteCornerRadius);
    painter->fillPath(path, fill);
    painter->setPen(QPen(NESMDesign::stateBorderColor(palette), 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    if (isEditActive() == false)
    {
        painter->setPen(NESMDesign::contrastTextColor(fill));
        const QRectF textRect = box.adjusted(  NESMDesign::NotePadding, NESMDesign::NotePadding
                                             , -NESMDesign::NotePadding, -NESMDesign::NotePadding);
        painter->drawText(textRect, Qt::AlignLeft | Qt::TextWordWrap, mText);
    }

    if (isSelected())
    {
        NESMDesign::paintSelectionFrame(painter, box.adjusted(-2.0, -2.0, 2.0, 2.0), palette, hasFocus());
        painter->setPen(QPen(palette.color(QPalette::Base), 1.0));
        painter->setBrush(NESMDesign::selectionColor(palette));
        painter->drawRect(handleRect());
    }
}

QRectF SMNoteItem::handleRect() const
{
    const double size = NESMDesign::NoteHandleSize;
    return QRectF(mSize.width() - size / 2.0, mSize.height() - size / 2.0, size, size);
}

bool SMNoteItem::hitHandle(const QPointF& position) const
{
    return isSelected() && handleRect().adjusted(-1.5, -1.5, 1.5, 1.5).contains(position);
}

SMScene* SMNoteItem::getCanvas() const
{
    return qobject_cast<SMScene*>(scene());
}

void SMNoteItem::updateFromModel()
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    const SMLayoutNote* note = canvas->getModel().getData().getLayout().findNote(getElementId());
    if (note == nullptr)
    {
        return;
    }

    prepareGeometryChange();
    mText      = note->text;
    mColorName = note->color;
    mSize      = QSizeF(  std::max(note->width, NESMDesign::NoteMinWidth)
                        , std::max(note->height, NESMDesign::NoteMinHeight));

    const QPointF position{ note->x, note->y };
    if (pos() != position)
    {
        setPos(position);
    }

    update();
}

void SMNoteItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(hitHandle(event->pos()) ? Qt::SizeFDiagCursor : Qt::ArrowCursor);
    SMCanvasItem::hoverMoveEvent(event);
}

void SMNoteItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    unsetCursor();
    SMCanvasItem::hoverLeaveEvent(event);
}

void SMNoteItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if ((event->button() == Qt::LeftButton) && hitHandle(event->pos()))
    {
        mResizing    = true;
        mResizeStart = getBoxGeometry();
        event->accept();
        return;
    }

    SMCanvasItem::mousePressEvent(event);
}

void SMNoteItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (mResizing)
    {
        applyResizeDrag(event->scenePos());
        event->accept();
        return;
    }

    SMCanvasItem::mouseMoveEvent(event);
}

void SMNoteItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (mResizing)
    {
        if (event->button() == Qt::LeftButton)
        {
            commitResize();
            mResizing = false;
        }

        event->accept();
        return;
    }

    SMCanvasItem::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton)
    {
        SMScene* canvas = getCanvas();
        if (canvas != nullptr)
        {
            canvas->commitSelectionMove(translate("Move selection"));
        }
    }
}

void SMNoteItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (isEditActive() == false)
        {
            startInlineEdit();
        }

        event->accept();
        return;
    }

    SMCanvasItem::mouseDoubleClickEvent(event);
}

QVariant SMNoteItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        prepareGeometryChange();
        if ((value.toBool() == false) && isEditActive())
        {
            closeEditEditor();
        }
    }

    return SMCanvasItem::itemChange(change, value);
}

void SMNoteItem::applyResizeDrag(const QPointF& scenePos)
{
    SMScene* canvas = getCanvas();
    if (canvas == nullptr)
    {
        return;
    }

    QPointF point{ scenePos };
    if (canvas->isSnapToGrid())
    {
        point = NESMDesign::snapPoint(point, canvas->getGridSize());
    }

    QRectF rect{ mResizeStart };
    rect.setRight(std::max(point.x(), rect.left() + NESMDesign::NoteMinWidth));
    rect.setBottom(std::max(point.y(), rect.top() + NESMDesign::NoteMinHeight));

    prepareGeometryChange();
    mSize = rect.size();
    update();
}

void SMNoteItem::commitResize()
{
    SMScene* canvas = getCanvas();
    const QRectF geometry = getBoxGeometry();
    if ((canvas == nullptr) || (geometry == mResizeStart))
    {
        return;
    }

    StateMachineModel& model = canvas->getModel();
    model.getUndoStack().push(new SMMoveNoteCommand(  model.getData(), model.getNotifier()
                                                    , getElementId(), SMMoveNodeCommand::takeNextGesture()
                                                    , geometry.x(), geometry.y(), geometry.width(), geometry.height()
                                                    , translate("Resize note")));
}

void SMNoteItem::startInlineEdit()
{
    if (mEditProxy != nullptr)
    {
        return;
    }

    NoteEdit* edit = new NoteEdit(mText);
    edit->mCommit = [this, edit]() {
        if (mEditProxy == nullptr)
        {
            return;
        }

        const QString text = edit->toPlainText();
        closeEditEditor();
        commitText(text);
    };
    edit->mCancel = [this]() { closeEditEditor(); };

    mEditProxy = new QGraphicsProxyWidget(this);
    mEditProxy->setWidget(edit);
    mEditProxy->setGeometry(QRectF(  NESMDesign::NotePadding, NESMDesign::NotePadding
                                  , mSize.width() - 2.0 * NESMDesign::NotePadding
                                  , mSize.height() - 2.0 * NESMDesign::NotePadding));
    mEditProxy->setZValue(1.0);

    edit->selectAll();
    edit->setFocus();
}

void SMNoteItem::commitText(const QString& text)
{
    SMScene* canvas = getCanvas();
    if ((canvas == nullptr) || (text == mText))
    {
        return;
    }

    StateMachineModel& model = canvas->getModel();
    model.getUndoStack().push(new SMSetNoteTextCommand(  model.getData(), model.getNotifier()
                                                       , getElementId(), text
                                                       , translate("Edit note")));
}

void SMNoteItem::closeEditEditor()
{
    if ((mEditProxy == nullptr) || mClosingEdit)
    {
        return;
    }

    mClosingEdit = true;
    QGraphicsProxyWidget* proxy = mEditProxy;
    mEditProxy = nullptr;
    proxy->deleteLater();
    mClosingEdit = false;
    update();
    setFocus();
}
