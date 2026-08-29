#ifndef LUSAN_VIEW_LOG_LOGSESSIONBAR_HPP
#define LUSAN_VIEW_LOG_LOGSESSIONBAR_HPP
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
 *  \file        lusan/view/log/LogSessionBar.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the session bar of a log window.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "areg/base/areg_global.h"

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogFilterChips;
class LogPriorityBar;
class SearchLineEdit;

class QHBoxLayout;
class QLabel;
class QMenu;
class QResizeEvent;
class QToolButton;

//////////////////////////////////////////////////////////////////////////
// LogSessionBar class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The bar at the top of a log window. It says what the window is looking at,
 *          how many rows it shows, and whether the table follows the newest row, and it
 *          carries the controls of the mode the window is in.
 *
 *          Every log window owns its own bar, so two open windows never share a counter
 *          or a follow state.
 **/
class LogSessionBar : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The window the bar is built for.
     **/
    enum class eSessionMode : int
    {
          ModeLive = 0  //!< Logs arriving from a log collector.
        , ModeOffline   //!< Logs read from an archive file.
    };

    /**
     * \brief   What the live window is doing with the logs it receives.
     **/
    enum class eLiveState : int
    {
          StateDisconnected = 0 //!< No log collector.
        , StateConnected        //!< Connected and recording.
        , StatePaused           //!< Connected, recording held; the database stays open.
        , StateStopped          //!< Recording ended and the database closed.
    };

    /**
     * \brief   The lines the notice row can carry. Each one appears and goes on its own.
     **/
    enum class eNotice : int
    {
          NoticeClockSkew = 0   //!< Two sources disagree about the wall clock.
        , NoticeRevealed        //!< A row a filter hides is drawn because the search found it.
        , NoticeCount           //!< Number of lines.
    };

    //!< The narrowest the text area of the search field may become. The tool buttons sit
    //!< inside the field and are paid for by its right text margin, so the width of the
    //!< control is this plus that margin, never this alone.
    static constexpr int    SEARCH_TEXT_MIN     { 130 };

    //!< The text area the search field asks for when there is room.
    static constexpr int    SEARCH_TEXT_MAX     { 460 };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the bar for the given window mode.
     * \param   mode    The window the bar belongs to.
     * \param   parent  The parent widget.
     **/
    explicit LogSessionBar(LogSessionBar::eSessionMode mode, QWidget* parent = nullptr);

    virtual ~LogSessionBar(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    //!< Returns the search field of the bar.
    inline SearchLineEdit* ctrlSearch(void) const;

    //!< Returns the button that turns the typed phrase into a filter on the message column.
    inline QToolButton* ctrlFilterMatches(void) const;

    //!< Returns the button that lists every row the phrase matches.
    inline QToolButton* ctrlHitList(void) const;

    /**
     * \brief   Returns the control that says which rows the search walks. While it is
     *          checked the search walks every row the window holds, filtered out or not.
     **/
    inline QToolButton* ctrlSearchScope(void) const;

    //!< Returns true if the search walks every row the window holds.
    bool isSearchingAllLogs(void) const;

    //!< Returns the row that names the filters the window has on.
    inline LogFilterChips* ctrlChips(void) const;

    //!< Returns the ladder that narrows the priorities the table draws.
    inline LogPriorityBar* ctrlPriority(void) const;

    /**
     * \brief   Returns the priorities the table draws, as a bit mask of areg::LogPriority
     *          values. Zero means every priority is drawn.
     **/
    uint16_t viewPriorityMask(void) const;

    /**
     * \brief   Returns the words that name the priority filter, as a chip shows them, for
     *          example "at least Warning". Empty when no priority is filtered out.
     **/
    QString priorityFilterName(void) const;

    //!< Returns the button that moves the table to its first row.
    inline QToolButton* ctrlMoveTop(void) const;

    /**
     * \brief   Returns the button that moves the table to its last row. In the live window it
     *          is a toggle: while it is checked the table keeps the newest log in sight.
     **/
    inline QToolButton* ctrlMoveBottom(void) const;

    //!< Returns the pause / resume recording button. Live mode only.
    inline QToolButton* ctrlPause(void) const;

    //!< Returns the stop / restart recording button. Live mode only.
    inline QToolButton* ctrlStop(void) const;

    //!< Returns the clear window button. Live mode only.
    inline QToolButton* ctrlClear(void) const;

    //!< Returns the reload archive button. Offline mode only.
    inline QToolButton* ctrlReload(void) const;

    //!< Returns the close archive button. Offline mode only.
    inline QToolButton* ctrlClose(void) const;

    /**
     * \brief   Returns true if the table should keep the newest row in sight. It is the
     *          checked state of the move to bottom button, and always false in the offline
     *          window, where the archive does not grow.
     **/
    bool isFollowing(void) const;

    /**
     * \brief   Sets the follow state without emitting the toggle signal.
     * \param   follow  True to keep the newest row in sight.
     **/
    void setFollowing(bool follow);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Draws the live state in the identity control.
     * \param   state   The state to show.
     * \param   address The host name of the log collector, empty when there is none.
     * \param   port    The TCP port of the log collector.
     **/
    void setLiveState(LogSessionBar::eLiveState state, const QString& address, uint16_t port);

    /**
     * \brief   Draws the opened archive in the identity control.
     * \param   fileName    The file name shown, empty when no archive is open.
     * \param   fullPath    The full path of the archive.
     * \param   inWorkspace True if the archive belongs to the current workspace.
     **/
    void setArchive(const QString& fileName, const QString& fullPath, bool inWorkspace);

    /**
     * \brief   Names the file the window writes its logs into. Live mode only.
     * \param   path    The full path of the database, empty when there is none.
     **/
    void setDatabasePath(const QString& path);

    /**
     * \brief   Draws every priority again and emits the change. The ladder returns to its
     *          leading cell and the scope lines come back.
     **/
    void resetPriorityFilter(void);

    /**
     * \brief   Draws the time the archive spans. Offline mode only.
     * \param   firstUs The timestamp of the first log, in microseconds since the epoch.
     * \param   lastUs  The timestamp of the last log, in microseconds since the epoch.
     **/
    void setSpan(TIME64 firstUs, TIME64 lastUs);

    /**
     * \brief   Draws how many rows the table shows out of how many it holds.
     * \param   shown   The rows the filters let through.
     * \param   total   The rows the model holds.
     **/
    void setCounters(int shown, int total);

    /**
     * \brief   Shows one line of the notice row. The row does not exist while every line
     *          is silent, so the table starts one line higher.
     * \param   which       The line to fill.
     * \param   text        The words to show.
     * \param   actionText  The label of the link beside the words, empty for no link.
     **/
    void showNotice(LogSessionBar::eNotice which, const QString& text, const QString& actionText = QString());

    /**
     * \brief   Hides one line of the notice row.
     * \param   which   The line to silence.
     **/
    void hideNotice(LogSessionBar::eNotice which);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Emitted when the identity menu asks to leave the log collector.
     **/
    void signalDisconnectRequested(void);

    /**
     * \brief   Emitted when the link of a notice line is pressed.
     * \param   which   The line the link belongs to.
     **/
    void signalNoticeAction(LogSessionBar::eNotice which);

    /**
     * \brief   Emitted when the reader changes the priorities the table draws.
     * \param   mask    The priorities to draw, as a bit mask of areg::LogPriority values.
     *                  Zero lets every priority through.
     **/
    void signalViewPriorityChanged(uint16_t mask);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void resizeEvent(QResizeEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Builds the row that is always present.
    void _buildMainRow(void);

    //!< Builds the row that carries the notice lines, hidden until one has words.
    void _buildNoticeRow(void);

    //!< Builds one line of the notice row.
    QWidget* _buildNoticeLine(LogSessionBar::eNotice which);

    //!< Shows the notice row while at least one of its lines has words.
    void _updateNoticeRow(void);

    //!< Fills the identity menu from the state the controls are in.
    void _fillIdentityMenu(void);

    //!< Appends a tool button to the main row.
    QToolButton* _addButton(const QIcon& icon, const QString& toolTip);

    //!< Appends a vertical separator to the main row.
    QWidget* _addSeparator(void);

    //!< Draws the counters in the form the current width allows.
    void _drawCounters(void);

    //!< Draws the tool tip of the identity control from the state and the file it names.
    void _drawIdentityHint(void);

    //!< Appends the entries that act on the file the window is reading or writing.
    void _addFileActions(void);

    //!< Returns the width the main row needs at the density it is in.
    int _requiredWidth(void) const;

    /**
     * \brief   Applies one density: 0 keeps everything, and each further step drops the
     *          least useful thing left. What a step drops stays reachable in the
     *          identity menu, which is why the bar needs no overflow control of its own.
     * \param   level   The density to apply.
     **/
    void _applyDensity(int level);

    //!< Picks the widest density the current width can hold.
    void _updateDensity(void);

    //!< Returns a round mark in the given colour, used as the live state icon.
    static QIcon _stateIcon(const QColor& color);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const eSessionMode  mMode;          //!< The window the bar was built for.
    QHBoxLayout*        mMainLayout;    //!< The layout of the row that is always present.
    QToolButton*        mIdentity;      //!< What the window is looking at, and its menu.
    QMenu*              mIdentityMenu;  //!< The menu of the identity control.
    QWidget*            mLeadSeparator; //!< The separator after the mode controls.
    QToolButton*        mPause;         //!< Pause or resume recording. Live mode.
    QToolButton*        mStop;          //!< Stop recording or record into a new file. Live mode.
    QToolButton*        mClear;         //!< Drop every row of this window. Live mode.
    QToolButton*        mReload;        //!< Read the archive again. Offline mode.
    QToolButton*        mClose;         //!< Close the archive. Offline mode.
    QLabel*             mSpan;          //!< The time the archive covers. Offline mode.
    SearchLineEdit*     mSearch;        //!< The search field.
    QToolButton*        mFilterMatches; //!< Turns the typed phrase into a filter on the message column.
    QToolButton*        mHitList;       //!< Lists every row the phrase matches.
    QToolButton*        mSearchScope;   //!< Says whether the search walks the shown rows or every row.
    QLabel*             mCounters;      //!< How many rows are shown out of how many there are.
    QToolButton*        mMoveTop;       //!< Move the table to its first row.
    QToolButton*        mMoveBottom;    //!< Move the table to its last row, and keep it there while checked.
    LogPriorityBar*     mPriority;      //!< The ladder that narrows the priorities the table draws.
    LogFilterChips*     mChips;         //!< The row that names the filters the window has on.
    QWidget*            mNoticeRow;     //!< The row that carries the notice lines.
    QWidget*            mNoticeLine[static_cast<int>(LogSessionBar::eNotice::NoticeCount)];  //!< One line per notice.
    QLabel*             mNoticeText[static_cast<int>(LogSessionBar::eNotice::NoticeCount)];  //!< The words of each line.
    QToolButton*        mNoticeLink[static_cast<int>(LogSessionBar::eNotice::NoticeCount)];  //!< The link of each line.
    QList<QToolButton*> mButtons;       //!< Every tool button of the main row, to size their icons at once.
    QList<QWidget*>     mShrinkable;    //!< The main row entries that give way, in the order they go.
    QString             mAddress;       //!< The log collector address shown in the identity control.
    QString             mFullPath;      //!< The full path of the file the window reads or writes.
    QString             mStateHint;     //!< The line the identity tool tip starts with.
    int                 mShown;         //!< The rows the filters let through.
    int                 mTotal;         //!< The rows the model holds.
    int                 mDensity;       //!< The density currently applied.
    bool                mInDensity;     //!< True while a density is being applied.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    LogSessionBar(void) = delete;
    Q_DISABLE_COPY_MOVE(LogSessionBar)
};

//////////////////////////////////////////////////////////////////////////
// LogSessionBar class inline methods
//////////////////////////////////////////////////////////////////////////

inline SearchLineEdit* LogSessionBar::ctrlSearch(void) const
{
    return mSearch;
}

inline QToolButton* LogSessionBar::ctrlFilterMatches(void) const
{
    return mFilterMatches;
}

inline QToolButton* LogSessionBar::ctrlSearchScope(void) const
{
    return mSearchScope;
}

inline QToolButton* LogSessionBar::ctrlHitList(void) const
{
    return mHitList;
}

inline LogFilterChips* LogSessionBar::ctrlChips(void) const
{
    return mChips;
}

inline LogPriorityBar* LogSessionBar::ctrlPriority(void) const
{
    return mPriority;
}

inline QToolButton* LogSessionBar::ctrlMoveTop(void) const
{
    return mMoveTop;
}

inline QToolButton* LogSessionBar::ctrlMoveBottom(void) const
{
    return mMoveBottom;
}

inline QToolButton* LogSessionBar::ctrlPause(void) const
{
    return mPause;
}

inline QToolButton* LogSessionBar::ctrlStop(void) const
{
    return mStop;
}

inline QToolButton* LogSessionBar::ctrlClear(void) const
{
    return mClear;
}

inline QToolButton* LogSessionBar::ctrlReload(void) const
{
    return mReload;
}

inline QToolButton* LogSessionBar::ctrlClose(void) const
{
    return mClose;
}

#endif  // LUSAN_VIEW_LOG_LOGSESSIONBAR_HPP
