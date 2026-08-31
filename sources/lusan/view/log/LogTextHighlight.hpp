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
    //!< The zone at the left of the leading column that the delegate owns. No cell text is
    //!< drawn in it, so the rail and the scope marks are never overdrawn.
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
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

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

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const LogSearchModel::sFoundPos& mFoundPos;
    QIcon                            mMarkEnter;    //!< The mark of a row that enters a scope.
    QIcon                            mMarkExit;     //!< The mark of a row that leaves a scope.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
    AREG_NOCOPY_NOMOVE(LogTextHighlight);
};

#endif  // LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
