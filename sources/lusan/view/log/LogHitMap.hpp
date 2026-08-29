#ifndef LUSAN_VIEW_LOG_LOGHITMAP_HPP
#define LUSAN_VIEW_LOG_LOGHITMAP_HPP
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
 *  \file        lusan/view/log/LogHitMap.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the marks drawn on the scrollbar of a log table.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QList>
#include <QVector>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogViewerFilter;
class LoggingModelBase;

class QScrollBar;
class QTableView;
class QTimer;

//////////////////////////////////////////////////////////////////////////
// LogHitMap class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The marks drawn over the vertical scrollbar of a log table. They say where the
 *          search hits and the rows above a severity sit in the whole table, so a long file
 *          shows its trouble spots without being scrolled.
 *
 *          The widget sits on the scrollbar and lets every mouse event through to it, so the
 *          scrollbar keeps working as it did.
 **/
class LogHitMap : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< What a mark on one line of the map stands for. One line may carry several.
    enum eMark : uint8_t
    {
          MarkNone      = 0x00  //!< Nothing on this line.
        , MarkWarning   = 0x01  //!< A row of warning priority.
        , MarkError     = 0x02  //!< A row of error or fatal priority.
        , MarkHit       = 0x04  //!< A row the search phrase matches.
        , MarkCurrent   = 0x08  //!< The hit the search is standing on.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the map over the vertical scrollbar of the given table.
     * \param   table   The table whose scrollbar the map is drawn on.
     **/
    explicit LogHitMap(QTableView* table);

    virtual ~LogHitMap(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Names the model and the filter the map reads the rows from.
     * \param   model   The model that holds the log entries.
     * \param   filter  The proxy that decides which of them the table draws.
     **/
    void setSource(LoggingModelBase* model, LogViewerFilter* filter);

    /**
     * \brief   Names the rows the search phrase matches.
     * \param   hits    The rows of the model, ascending.
     * \param   current The row the search is standing on, or -1.
     **/
    void setHits(const QList<uint32_t>& hits, int current);

    /**
     * \brief   Drops every mark and redraws the map empty.
     **/
    void clearHits(void);

    /**
     * \brief   Asks for the marks to be built again. Several calls in a row cost one build.
     **/
    void refresh(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void paintEvent(QPaintEvent* event) override;

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members and methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< The delay that collects a run of row changes into one build.
    static constexpr int    RebuildDelay    { 200 };

    //!< The widest the map may become, so it never hides the scrollbar it sits on.
    static constexpr int    MapWidth        { 14 };

    /**
     * \brief   Returns the scrollbar the map is drawn on.
     **/
    QScrollBar* _scrollBar(void) const;

    /**
     * \brief   Puts the map over the groove of the scrollbar, between its arrows.
     **/
    void _followScrollBar(void);

    /**
     * \brief   Reads the rows again and fills one mark set per line of the map.
     **/
    void _build(void);

    QTableView*         mTable;     //!< The table the map belongs to.
    LoggingModelBase*   mModel;     //!< The model that holds the log entries.
    LogViewerFilter*    mFilter;    //!< The proxy that decides which rows the table draws.
    QTimer*             mRebuild;   //!< Collects a run of row changes into one build.
    QVector<uint8_t>    mLines;     //!< One mark set per line of the map, top to bottom.
    QList<uint32_t>     mHits;      //!< The rows the search phrase matches, ascending.
    int                 mCurrent;   //!< The row the search is standing on, or -1.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    LogHitMap(void) = delete;
    Q_DISABLE_COPY_MOVE(LogHitMap)
};

#endif  // LUSAN_VIEW_LOG_LOGHITMAP_HPP
