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
 *  \file        lusan/view/log/LogPriorityBar.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the scope priority bar.
 *
 ************************************************************************/

#include "lusan/view/log/LogPriorityBar.hpp"

#include "lusan/common/NELogPalette.hpp"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace
{
    constexpr int   BarHeight   { 24 };  //!< The toolbar row height.
    constexpr int   OffWidth    { 22 };  //!< The leading cell that silences the scope.
    constexpr int   CellWidth   { 31 };  //!< One severity cell.
    constexpr int   NotchWidth  {  6 };  //!< The break between the ladder and the scope flag.
    constexpr int   ScopeWidth  { 31 };  //!< The scope flag.
    constexpr int   RailHeight  {  3 };  //!< The mark that says a cell is chosen.
    constexpr qreal Radius      { 5.0 }; //!< The corner of the shell.

    //! The share of the width each cell takes, as running totals over the six cells.
    constexpr int   _cellStops[7]{ 0, OffWidth
                                 , OffWidth + CellWidth
                                 , OffWidth + (CellWidth * 2)
                                 , OffWidth + (CellWidth * 3)
                                 , OffWidth + (CellWidth * 4)
                                 , OffWidth + (CellWidth * 4) + ScopeWidth };

    //! The sum of the cell shares.
    constexpr int   CellStopMax { _cellStops[6] };

    //! The narrowest the six cells may be squeezed to before the letters stop reading.
    constexpr int   MinCellSpan { (CellStopMax * 2) / 3 };

    //! The letters of the four severity cells, in ladder order.
    const char* const _letters[4]{ "E", "W", "I", "D" };

    //! The palette role of the four severity cells, in ladder order.
    const NELogPalette::eLogColorRole _roles[4]
    {
          NELogPalette::eLogColorRole::RoleError
        , NELogPalette::eLogColorRole::RoleWarning
        , NELogPalette::eLogColorRole::RoleInformation
        , NELogPalette::eLogColorRole::RoleDebug
    };

    //! A cell is tinted rather than filled: the letter stays the strongest text in the row,
    //! which a white letter on saturated amber does not.
    QColor tintOf(const QColor& hue)
    {
        return NELogPalette::withOpacity(hue, NELogPalette::eLogOpacity::OpacityTint);
    }
}

LogPriorityBar::LogPriorityBar(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mLevel    (LogPriorityBar::eLogLevel::LevelOff)
    , mScopes   (false)
    , mMixed    (false)
    , mIdle     (false)
    , mHovered  (-1)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setFixedHeight(BarHeight);
}

QSize LogPriorityBar::sizeHint(void) const
{
    return QSize(OffWidth + (CellWidth * 4) + NotchWidth + ScopeWidth + 2, BarHeight);
}

QSize LogPriorityBar::minimumSizeHint(void) const
{
    return QSize(MinCellSpan + NotchWidth + 2, BarHeight);
}

void LogPriorityBar::setLevel(LogPriorityBar::eLogLevel newLevel)
{
    if (mLevel != newLevel)
    {
        mLevel = newLevel;
        update();
    }
}

void LogPriorityBar::setScopeEnabled(bool enabled)
{
    if (mScopes != enabled)
    {
        mScopes = enabled;
        update();
    }
}

void LogPriorityBar::setMixed(bool mixed)
{
    if (mMixed != mixed)
    {
        mMixed = mixed;
        update();
    }
}

void LogPriorityBar::setIdle(bool idle)
{
    if (mIdle != idle)
    {
        mIdle = idle;
        update();
    }
}

QRect LogPriorityBar::_cellRect(int cell) const
{
    if ((cell < 0) || (cell >= LogPriorityBar::CellCount))
        return QRect();

    // The cells keep their proportions at any width, so the bar fits a narrow dock
    // without the letters leaving their cells.
    const int span { qMax(width() - NotchWidth - 2, MinCellSpan) };
    const int notch{ cell == LogPriorityBar::ScopeCell ? NotchWidth : 0 };
    const int left { 1 + notch + ((_cellStops[cell]     * span) / CellStopMax) };
    const int right{ 1 + notch + ((_cellStops[cell + 1] * span) / CellStopMax) };

    return QRect(left, 1, right - left, height() - 2);
}

int LogPriorityBar::_cellAt(const QPoint& pos) const
{
    for (int cell = 0; cell < LogPriorityBar::CellCount; ++cell)
    {
        if (_cellRect(cell).contains(pos))
            return cell;
    }

    return -1;
}

void LogPriorityBar::_activateCell(int cell)
{
    if (cell == LogPriorityBar::ScopeCell)
    {
        mScopes = mIdle ? true : (mScopes == false);
        mIdle   = false;
        update();
        emit signalScopeToggled(mScopes);
    }
    else if ((cell >= 0) && (cell <= 4))
    {
        const eLogLevel chosen{ static_cast<eLogLevel>(cell) };
        if ((chosen != mLevel) || mMixed || mIdle)
        {
            mLevel = chosen;
            mMixed = false;
            mIdle  = false;
            update();
            emit signalLevelChanged(mLevel);
        }
    }
}

void LogPriorityBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPalette& pal{ palette() };
    const QColor base  { pal.color(QPalette::Base) };
    const QColor border{ pal.color(QPalette::Mid) };
    const QColor muted { pal.color(QPalette::Disabled, QPalette::WindowText) };
    const QColor text  { pal.color(isEnabled() ? QPalette::Active : QPalette::Disabled, QPalette::WindowText) };

    // The shell, one outline around every cell.
    const QRectF shell{ QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5) };
    QPainterPath shape;
    shape.addRoundedRect(shell, Radius, Radius);
    painter.fillPath(shape, base);

    painter.setClipPath(shape);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const int  levelValue{ mIdle ? -1 : static_cast<int>(mLevel) };
    const bool scopes    { (mIdle == false) && mScopes };

    // The leading cell. It is selected, never lit: silence is not a priority, so it
    // takes a colourless rail and no fill.
    {
        const QRect cell{ _cellRect(0) };
        if (levelValue == 0)
        {
            painter.fillRect(QRect(cell.left(), cell.bottom() - RailHeight + 1, cell.width(), RailHeight), muted);
        }

        painter.setPen(levelValue == 0 ? text : muted);
        painter.drawText(cell, Qt::AlignCenter, QStringLiteral("-"));
    }

    // The four severity cells, filled up to the chosen level.
    for (int index = 0; index < 4; ++index)
    {
        const QRect cell{ _cellRect(index + 1) };
        const bool  lit { levelValue >= (index + 1) };
        const QColor hue{ NELogPalette::railColor(_roles[index]) };
        const QColor ink{ NELogPalette::textColor(_roles[index]) };

        if (lit && (mMixed == false))
        {
            painter.fillRect(cell, tintOf(hue));
            painter.fillRect(QRect(cell.left(), cell.bottom() - RailHeight + 1, cell.width(), RailHeight), hue);
        }
        else if (lit)
        {
            // The selected scopes disagree: say so with an outline instead of a fill,
            // because dimming already means the process is gone.
            painter.setPen(hue);
            painter.drawRect(cell.adjusted(1, 1, -2, -2));
        }

        painter.setPen(lit ? ink : muted);
        QFont letter{ font() };
        letter.setBold(true);
        painter.setFont(letter);
        painter.drawText(cell, Qt::AlignCenter, QString::fromLatin1(_letters[index]));
    }

    // The break, then the scope flag. The break is what lets one shell hold two controls.
    {
        const QRect cell{ _cellRect(LogPriorityBar::ScopeCell) };
        painter.fillRect(QRect(cell.left() - NotchWidth, 0, NotchWidth, height()), pal.color(QPalette::Window));
        painter.setPen(border);
        painter.drawLine(cell.left(), 0, cell.left(), height());

        const QColor hue{ NELogPalette::railColor(NELogPalette::eLogColorRole::RoleScope) };
        const QColor ink{ NELogPalette::textColor(NELogPalette::eLogColorRole::RoleScope) };
        if (scopes)
        {
            painter.fillRect(cell.adjusted(1, 0, 0, 0), tintOf(hue));
            painter.fillRect(QRect(cell.left() + 1, cell.bottom() - RailHeight + 1, cell.width() - 1, RailHeight), hue);
        }

        painter.setPen(scopes ? ink : muted);
        QFont letter{ font() };
        letter.setBold(true);
        painter.setFont(letter);
        painter.drawText(cell, Qt::AlignCenter, QStringLiteral("S"));
    }

    // The cell under the cursor, drawn last so it sits over the fills.
    if ((mHovered >= 0) && isEnabled())
    {
        const QColor hover{ NELogPalette::withOpacity(pal.color(QPalette::Highlight), NELogPalette::eLogOpacity::OpacityHover) };
        painter.fillRect(_cellRect(mHovered), hover);
    }

    painter.setClipping(false);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(border);
    painter.drawPath(shape);
}

void LogPriorityBar::mousePressEvent(QMouseEvent* event)
{
    if (isEnabled() && (event->button() == Qt::LeftButton))
    {
        const int cell{ _cellAt(event->pos()) };
        if (cell >= 0)
        {
            _activateCell(cell);
            setFocus(Qt::MouseFocusReason);
            event->accept();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void LogPriorityBar::mouseMoveEvent(QMouseEvent* event)
{
    const int cell{ _cellAt(event->pos()) };
    if (cell != mHovered)
    {
        mHovered = cell;
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void LogPriorityBar::leaveEvent(QEvent* event)
{
    if (mHovered != -1)
    {
        mHovered = -1;
        update();
    }

    QWidget::leaveEvent(event);
}

void LogPriorityBar::keyPressEvent(QKeyEvent* event)
{
    const int levelValue{ mIdle ? -1 : static_cast<int>(mLevel) };

    // The arrows walk the ladder and stop at Debug. They never reach the scope flag,
    // because that would put two different axes on one key.
    if (event->key() == Qt::Key_Left)
    {
        if (levelValue > 0)
        {
            _activateCell(levelValue - 1);
        }

        event->accept();
    }
    else if (event->key() == Qt::Key_Right)
    {
        if (levelValue < 4)
        {
            _activateCell(levelValue + 1);
        }

        event->accept();
    }
    else if ((event->key() >= Qt::Key_0) && (event->key() <= Qt::Key_4))
    {
        _activateCell(event->key() - Qt::Key_0);
        event->accept();
    }
    else if (event->key() == Qt::Key_5)
    {
        _activateCell(LogPriorityBar::ScopeCell);
        event->accept();
    }
    else if ((event->key() == Qt::Key_Space) || (event->key() == Qt::Key_Return))
    {
        _activateCell(LogPriorityBar::ScopeCell);
        event->accept();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}
