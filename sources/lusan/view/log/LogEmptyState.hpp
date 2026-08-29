#ifndef LUSAN_VIEW_LOG_LOGEMPTYSTATE_HPP
#define LUSAN_VIEW_LOG_LOGEMPTYSTATE_HPP
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
 *  \file        lusan/view/log/LogEmptyState.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, what an empty log table says.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QPushButton;

//////////////////////////////////////////////////////////////////////////
// LogEmptyState class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The panel a log table shows when it has no row to draw. It names the reason and,
 *          when there is one, offers the action that ends it.
 *
 *          It is placed over the table viewport, so the column headers stay reachable and a
 *          filter can be reopened from the header while the panel is up.
 **/
class LogEmptyState : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The reason the table has nothing to draw.
     **/
    enum class eEmptyReason : int
    {
          ReasonNone = 0        //!< The table has rows. The panel is hidden.
        , ReasonNotConnected    //!< The live window has no log collector.
        , ReasonNoArchive       //!< The offline window has no file open.
        , ReasonNoLiveLogs      //!< The collector is there and no target has produced a log yet.
        , ReasonEmptyArchive    //!< The archive is open and holds no log.
        , ReasonFiltered        //!< Every row the window holds is kept out by a filter.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the panel, hidden.
     * \param   parent  The widget the panel covers.
     **/
    explicit LogEmptyState(QWidget* parent = nullptr);

    virtual ~LogEmptyState(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Draws the given reason, or hides the panel when there is none.
     * \param   reason  The reason the table is empty.
     * \param   held    The rows the filters keep out. Used by ReasonFiltered only.
     * \param   scopes  True if the scope tree is hiding scopes. ReasonFiltered only.
     * \param   columns True if a column filter is set. ReasonFiltered only. The action button
     *                  is offered when a column filter or a hidden scope is what holds the rows.
     **/
    void setReason(LogEmptyState::eEmptyReason reason, int held, bool scopes, bool columns);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Emitted when the reader asks to drop every column filter.
     **/
    void signalClearFilters(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLabel*         mMark;      //!< The glyph of the reason.
    QLabel*         mHeadline;  //!< The reason in one line.
    QLabel*         mDetails;   //!< What to do about it.
    QPushButton*    mAction;    //!< Drops every column filter. Shown for ReasonFiltered only.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    Q_DISABLE_COPY_MOVE(LogEmptyState)
};

#endif  // LUSAN_VIEW_LOG_LOGEMPTYSTATE_HPP
