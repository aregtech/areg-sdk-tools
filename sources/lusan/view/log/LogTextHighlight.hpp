#ifndef LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
#define LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
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
 *  \file        lusan/view/log/LogTextHighlight.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Styling class to highlight search elements.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QStyledItemDelegate>
#include "lusan/model/log/LogSearchModel.hpp"
#include "areg/base/areg_global.h"

#include <QIcon>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QFontMetrics;
class QPainter;
class QStyleOptionViewItem;
class QModelIndex;

/**
 * \brief   LogTextHighlight class is a custom item delegate that highlights the search results in the log viewer.
 **/
class LogTextHighlight : public QStyledItemDelegate
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    //!< The zone the delegate owns at the leading end of a row. It is the width of the rail
    //!< column, so the rail and the scope marks never reach a cell that carries text.
    static constexpr int    GutterWidth { 18 };

    //!< Where the scope mark starts inside the gutter, clear of the rail.
    static constexpr int    MarkLeft    {  6 };

    //!< The largest a scope mark is drawn. A short row draws it smaller.
    static constexpr int    MarkSide    { 11 };

    //!< The width of the edge that marks a row the Scope Analyzer currently holds.
    static constexpr int    AnalyzedEdge{ 2 };

    LogTextHighlight(const LogSearchModel::sFoundPos& foundPos, QObject* parent = nullptr);
    virtual ~LogTextHighlight() = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Names the columns whose text is cut at its left end when the column is too
     *          narrow. A clock reading loses its meaning from the left, not from the right.
     * \param   mask    One bit per logical column of the table.
     **/
    inline void setElideLeftColumns(quint32 mask);

    /**
     * \brief   Says whether a long message is broken over several lines, and how many lines
     *          it may take before the rest of it is cut.
     * \param   wrap        True to break a long message over several lines.
     * \param   maxLines    The most lines one row may take.
     **/
    inline void setWordWrap(bool wrap, int maxLines);

    /**
     * \brief   Returns true when a long message is broken over several lines.
     **/
    inline bool isWordWrap(void) const;

    /**
     * \brief   Returns the height a cell needs to hold its text, in the width it is given.
     * \param   text        The text of the cell.
     * \param   metrics     The metrics of the face the table draws in.
     * \param   width       The width the cell has for its text.
     * \param   maxLines    The most lines the answer may count.
     * \return  The height in pixels, the air of one row included.
     **/
    static int wrappedHeight(const QString& text, const QFontMetrics& metrics, int width, int maxLines);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * \brief   Fills the drawing options of one cell, and cuts the columns that were named
     *          at their left end.
     **/
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

private:

    /**
     * brief   Draws the priority rail on the leading edge of the row.
     **/
    void _paintPriorityRail(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    /**
     * brief   Draws the background and the gutter mark of a row the filters hide and the
     *         search brought back.
     **/
    void _paintRevealed(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    /**
     * brief   Draws the scope enter or exit mark of the row in the gutter, beside the rail.
     **/
    void _paintScopeMark(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    /**
     * brief   Draws the tint and the leading edge of a row that belongs to the call the
     *         Scope Analyzer currently holds.
     **/
    void _paintAnalyzed(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    /**
     * brief   Draws the line that opens a calendar day, along the top edge of the row.
     **/
    void _paintDayChange(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const LogSearchModel::sFoundPos& mFoundPos;
    QIcon                            mMarkEnter;    //!< The mark of a row that enters a scope.
    QIcon                            mMarkExit;     //!< The mark of a row that leaves a scope.
    quint32                          mElideLeft;    //!< The columns whose text is cut at its left end.
    bool                             mWordWrap;     //!< True while a long message is broken over several lines.
    int                              mMaxLines;     //!< The most lines one row may take while wrapping.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
    AREG_NOCOPY_NOMOVE(LogTextHighlight);
};

//////////////////////////////////////////////////////////////////////////
// LogTextHighlight inline methods
//////////////////////////////////////////////////////////////////////////

inline void LogTextHighlight::setElideLeftColumns(quint32 mask)
{
    mElideLeft = mask;
}

inline void LogTextHighlight::setWordWrap(bool wrap, int maxLines)
{
    mWordWrap = wrap;
    mMaxLines = qMax(1, maxLines);
}

inline bool LogTextHighlight::isWordWrap(void) const
{
    return mWordWrap;
}

#endif  // LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
