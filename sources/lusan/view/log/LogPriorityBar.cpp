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

#include <QHelpEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

namespace
{
    constexpr int   BarHeight   { 24 };  //!< The toolbar row height.
    constexpr int   CellWidth   { 31 };  //!< One severity cell until the owner sets another.
    constexpr int   NotchWidth  {  5 };  //!< The break between the ladder and the scope flag.
    constexpr int   RailHeight  {  3 };  //!< The mark that says a cell is chosen.
    constexpr qreal Radius      { 5.0 }; //!< The corner of the shell.

    //! The leading cell is narrower than a severity cell: silence is not a priority, and the
    //! dash it carries is the narrowest mark in the row.
    constexpr int   OffNumer    { 22 };
    constexpr int   OffDenom    { 31 };

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

    //! The ground of a bar that narrows a view. It sits halfway between the input ground of a
    //! bar that changes the target and the toolbar behind it, so the two never read alike.
    QColor viewGround(const QColor& base, const QColor& window)
    {
        return QColor( (base.red()   + window.red())   / 2
                     , (base.green() + window.green()) / 2
                     , (base.blue()  + window.blue())  / 2 );
    }
}

LogPriorityBar::LogPriorityBar(QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mRole         (LogPriorityBar::eBarRole::RoleTarget)
    , mLevelLow     (LogPriorityBar::eLogLevel::LevelOff)
    , mLevelHigh    (LogPriorityBar::eLogLevel::LevelOff)
    , mScopesSome   (false)
    , mScopesAll    (false)
    , mCellWidth    (CellWidth)
    , mIdle         (false)
    , mHovered      (-1)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setFixedHeight(BarHeight);
}

QSize LogPriorityBar::sizeHint(void) const
{
    return QSize(_cellStop(LogPriorityBar::CellCount) + (NotchWidth * _notchCount()) + 2, BarHeight);
}

QSize LogPriorityBar::minimumSizeHint(void) const
{
    return QSize(((_cellStop(LogPriorityBar::CellCount) * 2) / 3) + (NotchWidth * _notchCount()) + 2, BarHeight);
}

void LogPriorityBar::setCellWidth(int cellWidth)
{
    const int width{ qMax(cellWidth, 1) };
    if (mCellWidth != width)
    {
        mCellWidth = width;
        updateGeometry();
        update();
    }
}

void LogPriorityBar::setRole(LogPriorityBar::eBarRole role)
{
    if (mRole == role)
        return;

    mRole = role;
    if (mRole == LogPriorityBar::eBarRole::RoleView)
    {
        setToolTip(tr("Draws only the rows of these priorities. The target keeps producing every one of them."));
    }

    updateGeometry();
    update();
}

void LogPriorityBar::setLevel(LogPriorityBar::eLogLevel newLevel)
{
    setLevelRange(newLevel, newLevel);
}

void LogPriorityBar::setLevelRange(LogPriorityBar::eLogLevel levelLow, LogPriorityBar::eLogLevel levelHigh)
{
    const eLogLevel high{ levelHigh };
    const eLogLevel low { levelLow < levelHigh ? levelLow : levelHigh };

    if ((mLevelLow != low) || (mLevelHigh != high))
    {
        mLevelLow  = low;
        mLevelHigh = high;
        update();
    }
}

void LogPriorityBar::setScopeEnabled(bool enabled)
{
    setScopeRange(enabled, enabled);
}

void LogPriorityBar::setScopeRange(bool linesSome, bool linesAll)
{
    const bool some{ linesSome || linesAll };

    if ((mScopesSome != some) || (mScopesAll != linesAll))
    {
        mScopesSome = some;
        mScopesAll  = linesAll;
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

int LogPriorityBar::_offWidth(void) const
{
    return (mCellWidth * OffNumer) / OffDenom;
}

int LogPriorityBar::_cellStop(int cell) const
{
    // Running total over the six cells: the off cell, four levels, and the scope flag.
    return cell <= 0 ? 0 : _offWidth() + (mCellWidth * (cell - 1));
}

int LogPriorityBar::_notchCount(void) const
{
    return mRole == LogPriorityBar::eBarRole::RoleView ? 2 : 1;
}

int LogPriorityBar::_notchBefore(int cell) const
{
    int notches{ 0 };
    if ((mRole == LogPriorityBar::eBarRole::RoleView) && (cell >= 1))
    {
        ++notches;
    }

    if (cell >= LogPriorityBar::ScopeCell)
    {
        ++notches;
    }

    return notches * NotchWidth;
}

QRect LogPriorityBar::_cellRect(int cell) const
{
    if ((cell < 0) || (cell >= LogPriorityBar::CellCount))
        return QRect();

    const int stopMax{ _cellStop(LogPriorityBar::CellCount) };
    // The cells keep their proportions at any width, so the bar fits a narrow dock
    // without the letters leaving their cells.
    const int span { qMax(width() - (NotchWidth * _notchCount()) - 2, (stopMax * 2) / 3) };
    const int notch{ _notchBefore(cell) };
    const int left { 1 + notch + ((_cellStop(cell)     * span) / stopMax) };
    const int right{ 1 + notch + ((_cellStop(cell + 1) * span) / stopMax) };

    return QRect(left, 1, right - left, height() - 2);
}

void LogPriorityBar::_paintAllMark(QPainter& painter, const QRect& cell) const
{
    // Four rows in the colours of the four priorities: the ladder in miniature, which says
    // every row is drawn without borrowing a letter from the ladder beside it.
    constexpr int rowCount{ 4 };
    constexpr int rowHeight{ 2 };
    constexpr int rowGap   { 1 };

    const int markWidth { qMax(cell.width() - (AllMarkAir * 2), 6) };
    const int markHeight{ (rowHeight * rowCount) + (rowGap * (rowCount - 1)) };
    const int left      { cell.left() + ((cell.width()  - markWidth ) / 2) };
    const bool lit      { isEnabled() };
    const QColor muted  { palette().color(QPalette::Disabled, QPalette::WindowText) };

    int top{ cell.top() + ((cell.height() - markHeight) / 2) };
    for (int index = 0; index < rowCount; ++index)
    {
        painter.fillRect( QRect(left, top, markWidth, rowHeight)
                        , lit ? NELogPalette::railColor(_roles[index]) : muted);
        top += rowHeight + rowGap;
    }
}

QString LogPriorityBar::_cellTip(int cell) const
{
    const bool view{ mRole == LogPriorityBar::eBarRole::RoleView };
    switch (cell)
    {
    case 0:
        return view ? tr("Draw the rows of every priority again.") : tr("Generate nothing, not even Fatal.");

    case 1:
        return view ? tr("Draw only Fatal and Error.") : tr("Generate Fatal and Error.");

    case 2:
        return view ? tr("Draw Warning and above.") : tr("Generate Warning and above.");

    case 3:
        return view ? tr("Draw Information and above.") : tr("Generate Information and above.");

    case 4:
        return view ? tr("Draw every priority.") : tr("Generate every priority.");

    case LogPriorityBar::ScopeCell:
        return view ? tr("Draw the scope enter and exit lines.") : tr("Write the scope enter and exit lines.");

    default:
        return QString();
    }
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
        // Only a cell every selected scope already carries switches off. A cell some of
        // them carry switches the rest on.
        const bool enabled{ mIdle ? true : (mScopesAll == false) };
        mScopesSome = enabled;
        mScopesAll  = enabled;
        mIdle       = false;
        update();
        emit signalScopeToggled(enabled);
    }
    else if ((cell >= 0) && (cell <= 4))
    {
        const eLogLevel chosen{ static_cast<eLogLevel>(cell) };
        if ((chosen != mLevelHigh) || (mLevelLow != mLevelHigh) || mIdle)
        {
            mLevelLow  = chosen;
            mLevelHigh = chosen;
            mIdle      = false;
            update();
            emit signalLevelChanged(chosen);
        }
    }
}

void LogPriorityBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPalette& pal{ palette() };
    const bool   view  { mRole == LogPriorityBar::eBarRole::RoleView };
    const QColor base  { view ? viewGround(pal.color(QPalette::Base), pal.color(QPalette::Window))
                              : pal.color(QPalette::Base) };
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

    const int  lowValue { mIdle ? -1 : static_cast<int>(mLevelLow) };
    const int  highValue{ mIdle ? -1 : static_cast<int>(mLevelHigh) };
    const bool scopesAny{ (mIdle == false) && mScopesSome };
    const bool scopesAll{ (mIdle == false) && mScopesAll };

    // The leading cell. On a target bar it is the zero of the ladder, drawn as a dash and
    // never lit: silence is not a priority. On a view bar it is a control of its own that
    // brings every row back, so it carries the four priorities in miniature and stands
    // behind the same break the scope flag stands behind.
    {
        const QRect cell{ _cellRect(0) };
        if (view)
        {
            const QRect ladder{ _cellRect(1) };
            painter.fillRect(QRect(ladder.left() - NotchWidth, 0, NotchWidth, height()), pal.color(QPalette::Window));
            painter.setPen(border);
            painter.drawLine(ladder.left(), 0, ladder.left(), height());
            _paintAllMark(painter, cell);
        }
        else
        {
            if (highValue == 0)
            {
                painter.fillRect(QRect(cell.left(), cell.bottom() - RailHeight + 1, cell.width(), RailHeight), muted);
            }

            painter.setPen(highValue == 0 ? text : muted);
            painter.drawText(cell, Qt::AlignCenter, QStringLiteral("-"));
        }
    }

    // The four severity cells. A cell every selected scope reaches is filled and railed.
    // A cell only some of them reach keeps the rail alone.
    for (int index = 0; index < 4; ++index)
    {
        const QRect cell{ _cellRect(index + 1) };
        const bool  lit { highValue >= (index + 1) };
        const bool  full{ lowValue  >= (index + 1) };
        const QColor hue{ NELogPalette::railColor(_roles[index]) };
        const QColor ink{ NELogPalette::textColor(_roles[index]) };

        if (full)
        {
            painter.fillRect(cell, tintOf(hue));
        }

        if (lit)
        {
            painter.fillRect(QRect(cell.left(), cell.bottom() - RailHeight + 1, cell.width(), RailHeight), hue);
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
        if (scopesAll)
        {
            painter.fillRect(cell.adjusted(1, 0, 0, 0), tintOf(hue));
        }

        if (scopesAny)
        {
            painter.fillRect(QRect(cell.left() + 1, cell.bottom() - RailHeight + 1, cell.width() - 1, RailHeight), hue);
        }

        painter.setPen(scopesAny ? ink : muted);
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

bool LogPriorityBar::event(QEvent* event)
{
    if (event->type() == QEvent::Type::ToolTip)
    {
        QHelpEvent* help{ static_cast<QHelpEvent*>(event) };
        const QString tip{ _cellTip(_cellAt(help->pos())) };
        if (tip.isEmpty() == false)
        {
            QToolTip::showText(help->globalPos(), tip, this);
            event->accept();
            return true;
        }
    }

    return QWidget::event(event);
}

void LogPriorityBar::keyPressEvent(QKeyEvent* event)
{
    const int levelValue{ mIdle ? -1 : static_cast<int>(mLevelHigh) };

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
