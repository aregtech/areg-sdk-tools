#ifndef LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
#define LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
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
 *  \file        lusan/view/log/LogTableHeader.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/log/LogHeaderItem.hpp"

#include <QHeaderView>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LiveLogViewer;
class LoggingModelBase;
class QTableView;

//////////////////////////////////////////////////////////////////////////
// LogTableHeader class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The header row of a log table. Every column it can narrow carries a funnel at
 *          its right end, and the panel of that column opens under it.
 **/
class LogTableHeader : public QHeaderView
{
    friend class LogHeaderItem;

    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// LogTableHeader attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    explicit LogTableHeader(QTableView* parent, LoggingModelBase* model, Qt::Orientation orientation = Qt::Horizontal);

//////////////////////////////////////////////////////////////////////////
// LogTableHeader attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Resets filter data of all columns.
     **/
    void resetFilters();

    /**
     * \brief   Returns the logical index of the column.
     **/
    int getColumnIndex(LoggingModelBase::eColumn column) const;

    /**
     * \brief   Returns the column set by specified logical index.
     **/
    LoggingModelBase::eColumn getColumn(int logicalIndex) const;

    /**
     * \brief   Returns the header item of the specified column.
     * \param   column  The column the header item belongs to.
     * \return  The header item of the column, or nullptr if the column is unknown.
     *          An item exists for every column, also for the ones the table does not show.
     **/
    LogHeaderItem* getHeaderItem(LoggingModelBase::eColumn column) const;

signals:
/************************************************************************
 * Signals
 ************************************************************************/

    /**
     * \brief   The signal is triggered when a combo-box filter is changed.
     * \param   logicalColumn  The logical column index of the filter.
     * \param   items           The list of items selected in the combo-box filter.
     **/
    void signalComboFilterChanged(int logicalColumn, const QList<NELusanCommon::FilterData>& items);

    /**
     * \brief   The signal is triggered when a text filter is changed.
     * \param   logicalColumn  The logical column index of the filter.
     * \param   text                The text entered in the text filter.
     * \param   isCaseSensitive     True if the text filter is case-sensitive.
     * \param   isWholeWord         True if the text filter is for whole words only.
     * \param   isWildCard          True if the text filter is a wildcard search.
     **/
    void signalTextFilterChanged(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard);

protected:
/************************************************************************
 * Overrides
 ************************************************************************/

    /**
     * \brief   Returns the size of the header. The height holds one line of the title
     *          and the funnel, and nothing more.
     **/
    QSize sizeHint() const override;

    /**
     * \brief   Triggered when the section is painted.
     **/
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

    /**
     * \brief   Triggered when the mouse is pressed on the header section.
     **/
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * \brief   Triggered when the mouse is moved.
     **/
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * \brief   Triggered when the mouse leaves the header.
     **/
    void leaveEvent(QEvent* event) override;

/************************************************************************
 * Hidden methods
 ************************************************************************/
private:

    /**
     * \brief   Returns the square the filter funnel of a section is drawn in. It sits at the
     *          right end of the section, so every column title starts at the same offset.
     * \param   rect    The rectangle of the section.
     **/
    inline QRect filterRect(const QRect& rect) const;

    /**
     * \brief   Draws the filter funnel of a section.
     * \param   painter The painter of the header.
     * \param   rect    The square to draw the funnel in.
     * \param   isOn    True when the column carries a filter.
     * \param   isHot   True when the pointer stands on the funnel itself.
     **/
    void drawFunnel(QPainter& painter, const QRect& rect, bool isOn, bool isHot) const;

    /**
     * \brief   Returns the rectangle of the section based on the logical index.
     *          The rectangle is empty when the index names no section.
     * \param   logicalIndex    The logical index of the section.
     **/
    inline QRect sectionRect(int logicalIndex) const;

    /**
     * \brief   Returns true when the given point stands on the funnel of a column that
     *          can be narrowed.
     * \param   pos             The point, in the coordinates of the header.
     * \param   logicalIndex    The section the point falls in.
     **/
    bool isOnFunnel(const QPoint& pos, int logicalIndex) const;

    /**
     * \brief   Fills the panel of the given column with the values the log carries now.
     * \param   column  The column to fill the panel of.
     **/
    void fillFilterData(LoggingModelBase::eColumn column);

    /**
     * \brief   Draws the section of the given column again. Called when the filter of a
     *          column goes on or off.
     * \param   column  The column to draw again.
     **/
    void refreshColumn(LoggingModelBase::eColumn column);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    LoggingModelBase*       mModel;     //!< The model for the log viewer, handling the data and its representation.
    QList<LogHeaderItem*>   mHeaders;   //!< List of header items, each representing a column in the log viewer.
    int                     mHovered;   //!< The logical index of the section under the mouse, -1 when there is none.
    bool                    mOnFunnel;  //!< True while the mouse stands on the funnel of the hovered section.
};

#endif // LUSAN_VIEW_LOG_LOGTABLEHEADER_HPP
