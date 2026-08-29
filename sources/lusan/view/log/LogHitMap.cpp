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
 *  \file        lusan/view/log/LogHitMap.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the marks drawn on the scrollbar of a log table.
 *
 ************************************************************************/

#include "lusan/view/log/LogHitMap.hpp"

#include "areg/logging/LoggingDefs.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include <QEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QTableView>
#include <QTimer>

namespace
{
    constexpr int   MarkHeight  { 2 };      //!< The height of one mark, so a single row stays visible.
    constexpr qreal ProblemPart { 0.55 };   //!< The share of the width the severity marks take.
}

LogHitMap::LogHitMap(QTableView* table)
    : QWidget   (table != nullptr ? table->verticalScrollBar() : nullptr)

    , mTable    (table)
    , mModel    (nullptr)
    , mFilter   (nullptr)
    , mRebuild  (nullptr)
    , mLines    ( )
    , mHits     ( )
    , mCurrent  (-1)
{
    setAttribute(Qt::WidgetAttribute::WA_TransparentForMouseEvents);
    setAttribute(Qt::WidgetAttribute::WA_NoSystemBackground);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    mRebuild = new QTimer(this);
    mRebuild->setSingleShot(true);
    mRebuild->setInterval(LogHitMap::RebuildDelay);
    connect(mRebuild, &QTimer::timeout, this, [this]() { _build(); update(); });

    QScrollBar* bar{ _scrollBar() };
    if (bar != nullptr)
    {
        bar->installEventFilter(this);
        _followScrollBar();
        raise();
    }
}

void LogHitMap::setSource(LoggingModelBase* model, LogViewerFilter* filter)
{
    mModel  = model;
    mFilter = filter;

    if (mFilter != nullptr)
    {
        connect(mFilter, &QAbstractItemModel::rowsInserted  , this, [this]() { refresh(); });
        connect(mFilter, &QAbstractItemModel::rowsRemoved   , this, [this]() { refresh(); });
        connect(mFilter, &QAbstractItemModel::modelReset    , this, [this]() { refresh(); });
        connect(mFilter, &QAbstractItemModel::layoutChanged , this, [this]() { refresh(); });
    }

    refresh();
}

void LogHitMap::setHits(const QList<uint32_t>& hits, int current)
{
    mHits    = hits;
    mCurrent = current;
    _build();
    update();
}

void LogHitMap::clearHits(void)
{
    if (mHits.isEmpty() && (mCurrent < 0))
        return;

    mHits.clear();
    mCurrent = -1;
    _build();
    update();
}

void LogHitMap::refresh(void)
{
    if (mRebuild->isActive() == false)
    {
        mRebuild->start();
    }
}

QScrollBar* LogHitMap::_scrollBar(void) const
{
    return mTable != nullptr ? mTable->verticalScrollBar() : nullptr;
}

void LogHitMap::_followScrollBar(void)
{
    QScrollBar* bar{ _scrollBar() };
    if (bar == nullptr)
        return;

    QStyleOptionSlider opt;
    opt.initFrom(bar);
    opt.subControls     = QStyle::SubControl::SC_All;
    opt.orientation     = Qt::Orientation::Vertical;
    opt.minimum         = bar->minimum();
    opt.maximum         = bar->maximum();
    opt.sliderPosition  = bar->sliderPosition();
    opt.sliderValue     = bar->value();
    opt.singleStep      = bar->singleStep();
    opt.pageStep        = bar->pageStep();
    opt.upsideDown      = bar->invertedAppearance();

    QRect groove{ bar->style()->subControlRect(QStyle::ComplexControl::CC_ScrollBar, &opt
                                              , QStyle::SubControl::SC_ScrollBarGroove, bar) };
    if (groove.width() > LogHitMap::MapWidth)
    {
        groove.setLeft(groove.right() - LogHitMap::MapWidth + 1);
    }

    if (geometry() != groove)
    {
        setGeometry(groove);
        _build();
    }
}

void LogHitMap::_build(void)
{
    const int lines{ height() };
    mLines.assign(lines > 0 ? lines : 0, static_cast<uint8_t>(LogHitMap::eMark::MarkNone));

    if ((lines <= 0) || (mModel == nullptr) || (mFilter == nullptr))
        return;

    const int shown{ mFilter->rowCount(QModelIndex()) };
    if (shown <= 0)
        return;

    // The scrollbar spans the rows the table draws, so the map is built over those and a hit
    // a filter keeps out has no line to sit on.
    for (int row = 0; row < shown; ++row)
    {
        const QModelIndex source{ mFilter->mapToSource(mFilter->index(row, 0)) };
        const areg::LogEntry* entry{ mModel->getLogData(source.row()) };
        if (LoggingModelBase::isProblemEntry(entry) == false)
            continue;

        const uint16_t prio{ static_cast<uint16_t>(entry->logMessagePrio) };
        uint8_t mark{ static_cast<uint8_t>(LogHitMap::eMark::MarkNone) };
        if ((prio & (static_cast<uint16_t>(areg::LogPriority::PrioError)
                   | static_cast<uint16_t>(areg::LogPriority::PrioFatal))) != 0)
        {
            mark = static_cast<uint8_t>(mark | LogHitMap::eMark::MarkError);
        }
        else if ((prio & static_cast<uint16_t>(areg::LogPriority::PrioWarning)) != 0)
        {
            mark = static_cast<uint8_t>(mark | LogHitMap::eMark::MarkWarning);
        }

        if (mark != LogHitMap::eMark::MarkNone)
        {
            mLines[(row * lines) / shown] = static_cast<uint8_t>(mLines[(row * lines) / shown] | mark);
        }
    }

    for (uint32_t hit : mHits)
    {
        const QModelIndex shownAt{ mFilter->mapFromSource(mModel->index(static_cast<int>(hit), 0)) };
        if (shownAt.isValid() == false)
            continue;

        const int line{ (shownAt.row() * lines) / shown };
        const uint8_t mark{ static_cast<uint8_t>((mCurrent >= 0) && (hit == static_cast<uint32_t>(mCurrent))
                                                 ? LogHitMap::eMark::MarkCurrent
                                                 : LogHitMap::eMark::MarkHit) };
        mLines[line] = static_cast<uint8_t>(mLines[line] | mark);
    }
}

void LogHitMap::paintEvent(QPaintEvent* /*event*/)
{
    if (mLines.isEmpty())
        return;

    QPainter painter(this);
    const int   full   { width() };
    const int   partial{ static_cast<int>(full * ProblemPart) };
    const QColor error { NELogPalette::railColor(NELogPalette::eLogColorRole::RoleError) };
    const QColor warn  { NELogPalette::railColor(NELogPalette::eLogColorRole::RoleWarning) };
    const QColor hit   { palette().color(QPalette::ColorRole::Highlight) };

    for (int line = 0; line < mLines.size(); ++line)
    {
        const uint8_t mark{ mLines[line] };
        if (mark == LogHitMap::eMark::MarkNone)
            continue;

        // The severity marks keep the left of the strip and the search marks the right, so one
        // never hides the other.
        if ((mark & LogHitMap::eMark::MarkError) != 0)
        {
            painter.fillRect(0, line, partial, MarkHeight, error);
        }
        else if ((mark & LogHitMap::eMark::MarkWarning) != 0)
        {
            painter.fillRect(0, line, partial, MarkHeight, warn);
        }

        if ((mark & LogHitMap::eMark::MarkCurrent) != 0)
        {
            painter.fillRect(0, line, full, MarkHeight, hit.darker(130));
        }
        else if ((mark & LogHitMap::eMark::MarkHit) != 0)
        {
            painter.fillRect(partial, line, full - partial, MarkHeight, hit);
        }
    }
}

bool LogHitMap::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _scrollBar())
    {
        const QEvent::Type type{ event->type() };
        if ((type == QEvent::Type::Resize) || (type == QEvent::Type::Show) || (type == QEvent::Type::StyleChange))
        {
            _followScrollBar();
            raise();
        }
    }

    return QWidget::eventFilter(watched, event);
}
