#ifndef LUSAN_VIEW_LOG_LOGFILTERCHIPS_HPP
#define LUSAN_VIEW_LOG_LOGFILTERCHIPS_HPP
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
 *  \file        lusan/view/log/LogFilterChips.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the row of chips naming the filters a log window has on.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/common/NELusanCommon.hpp"

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QHBoxLayout;
class QToolButton;

//////////////////////////////////////////////////////////////////////////
// LogFilterChips class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The row that names every filter a log window has on, one chip each. A chip says
 *          what it keeps out and drops it in one click, so a row that is missing from the
 *          table is always one gesture from coming back.
 *
 *          The row does not exist while no filter is on, so the table starts one line higher.
 **/
class LogFilterChips : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   What a chip stands for. It decides who drops it and how it is drawn.
     **/
    enum class eChipKind : int
    {
          ChipColumn = 0    //!< A filter set on one column of the table.
        , ChipScopes        //!< The scopes the navigation tree is hiding.
        , ChipIsolate       //!< A single row picked out of the table.
        , ChipPriority      //!< The priorities the view filter lets through.
    };

    /**
     * \brief   One chip, as the window hands it over.
     **/
    struct sChip
    {
        LogFilterChips::eChipKind   kind    { LogFilterChips::eChipKind::ChipColumn };
        int                         column  { -1 };     //!< The column it acts on, as a LoggingModelBase::eColumn value.
        QString                     label   { };        //!< What the chip says.
        QString                     hint    { };        //!< The tool tip of the chip.
        NELusanCommon::FilterString phrase  { };        //!< The phrase and options a search can take over.
    };

    using ListChips = QList<LogFilterChips::sChip>;

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogFilterChips(QWidget* parent = nullptr);

    virtual ~LogFilterChips(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Draws the given chips, replacing the ones on the row. The row hides itself
     *          when the list is empty.
     * \param   chips   The filters that are on.
     **/
    void setChips(const LogFilterChips::ListChips& chips);

    /**
     * \brief   Returns the chips the row draws.
     **/
    inline const LogFilterChips::ListChips& chips(void) const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Emitted when one chip is dropped.
     * \param   chip    The filter to switch off.
     **/
    void signalChipDropped(const LogFilterChips::sChip& chip);

    /**
     * \brief   Emitted when a chip is asked to move into the search box instead.
     * \param   chip    The filter whose phrase and options the search takes over.
     **/
    void signalSearchInstead(const LogFilterChips::sChip& chip);

    /**
     * \brief   Emitted when every filter is asked to go.
     **/
    void signalClearAll(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Builds one chip and appends it to the row.
    void _addChip(const LogFilterChips::sChip& chip);

    //!< Removes every widget the row holds.
    void _clearRow(void);

    //!< Returns the colour a chip of the given kind is drawn in.
    static QColor _chipColor(LogFilterChips::eChipKind kind, const QPalette& palette);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QHBoxLayout*                mLayout;    //!< The layout the chips sit in.
    QToolButton*                mClearAll;  //!< Drops every filter at once.
    LogFilterChips::ListChips   mChips;     //!< The chips the row draws.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    Q_DISABLE_COPY_MOVE(LogFilterChips)
};

//////////////////////////////////////////////////////////////////////////
// LogFilterChips class inline methods
//////////////////////////////////////////////////////////////////////////

inline const LogFilterChips::ListChips& LogFilterChips::chips(void) const
{
    return mChips;
}

#endif  // LUSAN_VIEW_LOG_LOGFILTERCHIPS_HPP
