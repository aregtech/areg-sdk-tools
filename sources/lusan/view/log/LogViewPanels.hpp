#ifndef LUSAN_VIEW_LOG_LOGVIEWPANELS_HPP
#define LUSAN_VIEW_LOG_LOGVIEWPANELS_HPP
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
 *  \file        lusan/view/log/LogViewPanels.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the panels a log window drops from its bar.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QFrame>

#include "lusan/model/log/LoggingModelBase.hpp"

#include <QList>
#include <QRect>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QListWidget;
class QListWidgetItem;
class QToolButton;
class QVBoxLayout;

//////////////////////////////////////////////////////////////////////////
// LogPopoverBase class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   A panel that drops under a control of the log window and closes when the
 *          reader clicks past it. It stays open while its own content is used, so a
 *          task of several steps costs one opening.
 **/
class LogPopoverBase : public QFrame
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogPopoverBase(const QString& title, QWidget* parent = nullptr);

    virtual ~LogPopoverBase(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Opens the panel under the given rectangle, keeping it on the screen.
     * \param   anchor  The rectangle of the control that opened it, in screen coordinates.
     **/
    void showAt(const QRect& anchor);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Draws the ground and the border of the panel.
     **/
    void paintEvent(QPaintEvent* event) override;

    /**
     * \brief   Closes the panel on Escape.
     **/
    void keyPressEvent(QKeyEvent* event) override;

    //!< Returns the layout the panel stacks its content in.
    inline QVBoxLayout* panelLayout(void) const;

    //!< Adds a thin line across the panel.
    void addSeparator(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVBoxLayout*    mLayout;    //!< The layout the panel stacks its content in.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    Q_DISABLE_COPY_MOVE(LogPopoverBase)
};

//////////////////////////////////////////////////////////////////////////
// LogColumnPicker class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The panel that chooses which columns the log table shows and in which order.
 *
 *          Every change reaches the table at once and the panel stays open, so a column
 *          set is built in one opening instead of one opening per column.
 **/
class LogColumnPicker : public LogPopoverBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    using ListColumns = QList<LoggingModelBase::eColumn>;

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogColumnPicker(QWidget* parent = nullptr);

    virtual ~LogColumnPicker(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Fills the list from the columns the table shows now. The shown ones come
     *          first, in their own order, and the rest follow.
     * \param   active  The columns the table shows, in the order it shows them.
     **/
    void setColumns(const LogColumnPicker::ListColumns& active);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when the reader changes which columns are shown or their order.
     * \param   columns The columns to show, in the order to show them.
     **/
    void signalColumnsChanged(const LogColumnPicker::ListColumns& columns);

    /**
     * \brief   Emitted when the reader asks for the columns the application starts with.
     **/
    void signalColumnsReset(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Returns the checked columns, in the order the list holds them.
    LogColumnPicker::ListColumns _chosen(void) const;

    //!< Reports what the list holds now.
    void _report(void);

    //!< Adds one row for the given column.
    void _addRow(LoggingModelBase::eColumn column, bool shown);

    /**
     * \brief   Moves the row of a column the reader just checked to the place the table gives
     *          that column, so the list shows where the column went. An unchecked row falls
     *          below the ones that are still shown.
     * \param   column  The column whose row moved in or out of the shown ones.
     **/
    void _placeChecked(int column);

    //!< Builds the row of the sets that are reached in one click.
    void _addPresets(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QListWidget*    mList;      //!< The columns, checked when the table shows them.
    bool            mFilling;   //!< True while the list is built, so it reports nothing.
    bool            mPending;   //!< True while a report is waiting for the event loop.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    Q_DISABLE_COPY_MOVE(LogColumnPicker)
};

//////////////////////////////////////////////////////////////////////////
// LogFilterPanel class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The panel that names every column a log window can be narrowed by, and what
 *          each one keeps at the moment.
 *
 *          It answers the question the header cannot: a header speaks only for the
 *          columns the table shows, this one also reaches the columns it hides.
 **/
class LogFilterPanel : public LogPopoverBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   One column of the panel, as the window hands it over.
     **/
    struct sEntry
    {
        LoggingModelBase::eColumn   column  { LoggingModelBase::eColumn::LogColumnInvalid };
        QString                     state   { };        //!< What the column keeps, or nothing.
        bool                        shown   { false };  //!< True when the table shows the column.
    };

    using ListEntries = QList<LogFilterPanel::sEntry>;

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogFilterPanel(QWidget* parent = nullptr);

    virtual ~LogFilterPanel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Fills the panel with the columns that can be narrowed and their state.
     * \param   entries The columns to list, in the order to list them.
     **/
    void setEntries(const LogFilterPanel::ListEntries& entries);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when the reader picks a column to narrow.
     * \param   column  The column, as a LoggingModelBase::eColumn value.
     * \param   anchor  The row that was picked, in screen coordinates, to open the panel of
     *                  the column under it.
     **/
    void signalOpenFilter(int column, const QRect& anchor);

    /**
     * \brief   Emitted when every filter is asked to go.
     **/
    void signalClearFilters(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QListWidget*    mList;      //!< The columns that can be narrowed.
    QToolButton*    mClearAll;  //!< Drops every filter the window has on.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    Q_DISABLE_COPY_MOVE(LogFilterPanel)
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline QVBoxLayout* LogPopoverBase::panelLayout(void) const
{
    return mLayout;
}

#endif  // LUSAN_VIEW_LOG_LOGVIEWPANELS_HPP
