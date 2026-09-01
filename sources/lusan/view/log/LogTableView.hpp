#ifndef LUSAN_VIEW_LOG_LOGTABLEVIEW_HPP
#define LUSAN_VIEW_LOG_LOGTABLEVIEW_HPP
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
 *  \file        lusan/view/log/LogTableView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the table that draws the log rows.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QTableView>

//////////////////////////////////////////////////////////////////////////
// LogTableView class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The table of a log window. It behaves like a plain table except that it never
 *          scrolls sideways on its own: the reader moves the horizontal bar, nothing else.
 *
 *          The leading column carries the rail, which opens every row with its priority
 *          colour and its scope mark. Revealing a cell of a column that stands past the
 *          right edge would take the rail off the left edge, so a row is revealed by its
 *          height alone and the horizontal position is left where the reader put it.
 **/
class LogTableView : public QTableView
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogTableView(QWidget* parent = nullptr);

    virtual ~LogTableView(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Brings the row of the given index into view without moving the table sideways.
     * \param   index   The index to reveal.
     * \param   hint    Where the row should stand once it is visible.
     **/
    virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    LogTableView(const LogTableView& /*src*/) = delete;
    LogTableView& operator = (const LogTableView& /*src*/) = delete;
};

#endif  // LUSAN_VIEW_LOG_LOGTABLEVIEW_HPP
