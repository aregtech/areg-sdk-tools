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
 *  \file        lusan/view/log/LogPriorityBar.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the scope priority bar.
 *
 ************************************************************************/
#ifndef LUSAN_VIEW_LOG_LOGPRIORITYBAR_HPP
#define LUSAN_VIEW_LOG_LOGPRIORITYBAR_HPP

/************************************************************************
 * Include files.
 ************************************************************************/
#include <QWidget>

#include <QRect>
#include <QString>

class QPainter;

/**
 * \brief   The priority bar of the scope panel.
 *
 *          One joined control instead of five toggles. The four severity cells form a
 *          ladder: choosing a level fills every cell up to it, because the target sends
 *          that level and everything above it. Scope lines are a separate flag and sit
 *          after a visible break, so the shell reads as a group holding two controls
 *          rather than as one control with five positions.
 *
 *          On a bar that changes the target the left cell sets level zero, which silences
 *          the scope. It carries no colour, because silence is not a priority. On a bar that
 *          narrows a view the left cell brings every row back; it stands behind a break of
 *          its own, so it is never read as the zero of the ladder.
 **/
class LogPriorityBar : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   What the bar acts on. It changes how the bar is drawn and what its leading
     *          cell means, never how the ladder works.
     **/
    enum class eBarRole : int
    {
          RoleTarget = 0    //!< Sets what the target produces. The leading cell silences it.
        , RoleView          //!< Narrows what this window draws. The leading cell keeps every row.
    };

    /**
     * \brief   The verbosity of a scope, as one position on a ladder.
     **/
    enum class eLogLevel : int
    {
          LevelOff      = 0     //!< Nothing is generated, not even Fatal
        , LevelError            //!< Fatal and Error
        , LevelWarning          //!< and Warning
        , LevelInformation      //!< and Information
        , LevelDebug            //!< everything
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogPriorityBar(QWidget* parent = nullptr);

    virtual ~LogPriorityBar(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Sets what the bar acts on. Call it before the bar is first shown.
     **/
    void setRole(eBarRole role);

    //!< Returns what the bar acts on.
    inline eBarRole role(void) const;

    /**
     * \brief   Returns the highest level the bar is showing.
     **/
    inline eLogLevel level(void) const;

    /**
     * \brief   Sets the level the bar shows, without emitting a change. Every cell up to
     *          it is drawn filled.
     **/
    void setLevel(eLogLevel newLevel);

    /**
     * \brief   Sets the span of levels the selected scopes produce, without emitting a
     *          change. Cells up to the lowest level are filled, because every scope
     *          produces them. Cells the highest level reaches but the lowest does not
     *          carry the rail alone, because only some scopes produce them.
     * \param   levelLow    The lowest level any selected scope produces.
     * \param   levelHigh   The highest level any selected scope produces.
     **/
    void setLevelRange(eLogLevel levelLow, eLogLevel levelHigh);

    /**
     * \brief   Returns true if the scope flag is on for every selected scope.
     **/
    inline bool isScopeEnabled(void) const;

    /**
     * \brief   Sets the scope flag, without emitting a change.
     **/
    void setScopeEnabled(bool enabled);

    /**
     * \brief   Sets the scope flag from what the selected scopes carry, without emitting
     *          a change. The cell is filled when they all write scope lines, and carries
     *          the rail alone when only some of them do.
     * \param   linesSome   True if at least one selected scope writes scope lines.
     * \param   linesAll    True if every selected scope writes scope lines.
     **/
    void setScopeRange(bool linesSome, bool linesAll);

    /**
     * \brief   Returns true if the bar shows no level at all.
     **/
    inline bool isIdle(void) const;

    /**
     * \brief   Draws the bar with no level and no scope flag, for the case where there is
     *          nothing to read a level from. Choosing a cell clears it.
     **/
    void setIdle(bool idle);

    /**
     * \brief   Sets the width one severity cell takes. The leading cell keeps its share of
     *          it and the scope cell matches it, so the whole bar follows.
     * \param   cellWidth   The width of one cell, at least one pixel.
     * \note    Give it the width of a tool button when the bar sits in a tool row, so the
     *          cells and the buttons beside them read as one grid.
     **/
    void setCellWidth(int cellWidth);

    /**
     * \brief   Returns the size the bar needs. The height is fixed at the toolbar row.
     **/
    virtual QSize sizeHint(void) const override;

    virtual QSize minimumSizeHint(void) const override;

signals:
    /**
     * \brief   Emitted when the user chooses a level.
     **/
    void signalLevelChanged(LogPriorityBar::eLogLevel newLevel);

    /**
     * \brief   Emitted when the user switches the scope flag.
     **/
    void signalScopeToggled(bool enabled);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void paintEvent(QPaintEvent* event) override;

    virtual void mousePressEvent(QMouseEvent* event) override;

    virtual void mouseMoveEvent(QMouseEvent* event) override;

    virtual void leaveEvent(QEvent* event) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

    //!< Shows the tool tip of the cell under the cursor instead of the tool tip of the bar.
    virtual bool event(QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members and methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< The number of cells the bar draws: the off cell, four levels, and the scope flag.
    static constexpr int    CellCount   { 6 };

    //!< The index of the scope flag among the cells.
    static constexpr int    ScopeCell   { 5 };

    //!< The air the leading mark of a view bar keeps on either side of itself.
    static constexpr int    AllMarkAir  { 4 };

    /**
     * \brief   Returns the width of the leading cell.
     **/
    int _offWidth(void) const;

    /**
     * \brief   Returns how many breaks the bar draws. A view bar breaks twice: its leading
     *          cell is a control of its own, not the zero of the ladder.
     **/
    int _notchCount(void) const;

    /**
     * \brief   Returns the width the breaks before the cell with the given index take.
     **/
    int _notchBefore(int cell) const;

    /**
     * \brief   Draws the mark of the leading cell of a view bar: the ladder in miniature,
     *          four rows in the colours of the four priorities.
     **/
    void _paintAllMark(QPainter& painter, const QRect& cell) const;

    /**
     * \brief   Returns the tool tip of the cell with the given index, or an empty string
     *          when there is no cell at that index.
     **/
    QString _cellTip(int cell) const;

    /**
     * \brief   Returns the running width of the cells before the one with the given index.
     *          Passing CellCount gives the width of all of them.
     **/
    int _cellStop(int cell) const;

    /**
     * \brief   Returns the rectangle of the cell with the given index.
     **/
    QRect _cellRect(int cell) const;

    /**
     * \brief   Returns the index of the cell under the given point, or -1.
     **/
    int _cellAt(const QPoint& pos) const;

    /**
     * \brief   Applies the choice the given cell stands for and emits the change.
     **/
    void _activateCell(int cell);

    eBarRole    mRole;          //!< What the bar acts on.
    eLogLevel   mLevelLow;      //!< The lowest level the selected scopes produce.
    eLogLevel   mLevelHigh;     //!< The highest level the selected scopes produce.
    bool        mScopesSome;    //!< At least one selected scope writes scope lines.
    bool        mScopesAll;     //!< Every selected scope writes scope lines.
    int         mCellWidth;     //!< The width one severity cell takes.
    bool        mIdle;          //!< There is nothing to read a level from.
    int         mHovered;       //!< The cell under the cursor, or -1.
};

//////////////////////////////////////////////////////////////////////////
// LogPriorityBar inline methods
//////////////////////////////////////////////////////////////////////////

inline LogPriorityBar::eBarRole LogPriorityBar::role(void) const
{
    return mRole;
}

inline LogPriorityBar::eLogLevel LogPriorityBar::level(void) const
{
    return mLevelHigh;
}

inline bool LogPriorityBar::isScopeEnabled(void) const
{
    return mScopesAll;
}

inline bool LogPriorityBar::isIdle(void) const
{
    return mIdle;
}

#endif  // LUSAN_VIEW_LOG_LOGPRIORITYBAR_HPP
