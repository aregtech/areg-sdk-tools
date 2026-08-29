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
 *  \file        lusan/view/log/LogSessionBar.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the session bar of a log window.
 *
 ************************************************************************/

#include "lusan/view/log/LogSessionBar.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/NaviToolbarWindow.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"
#include "lusan/view/log/LogFilterChips.hpp"

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    //! The air the bar keeps around its rows.
    constexpr int   _barMargin  { 3 };

    //! The gap between two entries of a row.
    constexpr int   _barGap     { 2 };

    //! The air a row keeps above and below its tallest control.
    constexpr int   _barAir     { 1 };

    //! The mark of the live state is drawn on this box and scaled down by the button.
    constexpr int   _dotBox     { 32 };
}

LogSessionBar::LogSessionBar(LogSessionBar::eSessionMode mode, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)

    , mMode         (mode)
    , mMainLayout   (nullptr)
    , mIdentity     (nullptr)
    , mIdentityMenu (nullptr)
    , mLeadSeparator(nullptr)
    , mPause        (nullptr)
    , mStop         (nullptr)
    , mClear        (nullptr)
    , mReload       (nullptr)
    , mClose        (nullptr)
    , mSpan         (nullptr)
    , mSearch       (nullptr)
    , mFilterMatches(nullptr)
    , mSearchScope  (nullptr)
    , mCounters     (nullptr)
    , mMoveTop      (nullptr)
    , mMoveBottom   (nullptr)
    , mChips        (nullptr)
    , mNoticeRow    (nullptr)
    , mNoticeLine   { }
    , mNoticeText   { }
    , mNoticeLink   { }
    , mButtons      ( )
    , mShrinkable   ( )
    , mAddress      ( )
    , mFullPath     ( )
    , mStateHint    ( )
    , mShown        (0)
    , mTotal        (0)
    , mDensity      (0)
    , mInDensity    (false)
{
    QVBoxLayout* rows = new QVBoxLayout(this);
    rows->setContentsMargins(0, 0, 0, 0);
    rows->setSpacing(0);

    QWidget* mainRow = new QWidget(this);
    mMainLayout = new QHBoxLayout(mainRow);
    mMainLayout->setContentsMargins(_barMargin, _barAir, _barMargin, _barAir);
    mMainLayout->setSpacing(_barGap);
    rows->addWidget(mainRow);

    _buildMainRow();

    mChips = new LogFilterChips(this);
    rows->addWidget(mChips);

    _buildNoticeRow();
    rows->addWidget(mNoticeRow);

    for (QToolButton* button : mButtons)
    {
        button->setIconSize(QSize(NaviToolbarWindow::NAVI_TOOL_ICON, NaviToolbarWindow::NAVI_TOOL_ICON));
    }

    // The row is as tall as the tallest control it carries. A line edit needs more height than
    // a flat tool button, and a row sized to the button clips the text it holds.
    mainRow->setFixedHeight(qMax(NaviToolbarWindow::toolRowHeight(), mSearch->sizeHint().height() + (_barAir * 2)));

    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    _drawCounters();
}

void LogSessionBar::_buildMainRow(void)
{
    QWidget* owner{ mMainLayout->parentWidget() };

    mIdentityMenu = new QMenu(owner);
    connect(mIdentityMenu, &QMenu::aboutToShow, this, &LogSessionBar::_fillIdentityMenu);

    mIdentity = new QToolButton(owner);
    mIdentity->setAutoRaise(true);
    mIdentity->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    mIdentity->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
    mIdentity->setMenu(mIdentityMenu);
    mIdentity->setMaximumWidth(260);
    mMainLayout->addWidget(mIdentity);
    mButtons.append(mIdentity);

    mLeadSeparator = _addSeparator();

    if (mMode == eSessionMode::ModeLive)
    {
        mPause = _addButton(NELusanCommon::iconPause(NELusanCommon::SizeBig), tr("Pause logging"));
        mPause->setCheckable(true);
        mStop  = _addButton(NELusanCommon::iconStop(NELusanCommon::SizeBig), tr("Stop logging"));
        mStop->setCheckable(true);
        mClear = _addButton(NELusanCommon::iconClear(NELusanCommon::SizeBig), tr("Clear this window"));

        mShrinkable << mClear << mStop << mPause;
    }
    else
    {
        mReload = _addButton(NELusanCommon::iconRefresh(NELusanCommon::SizeBig), tr("Read the archive again and drop every filter"));
        mClose  = _addButton(NELusanCommon::iconClose(NELusanCommon::SizeBig), tr("Close the archive"));

        mSpan = new QLabel(owner);
        mSpan->setObjectName(QStringLiteral("logSessionSpan"));
        mSpan->setToolTip(tr("The first and the last log of this archive"));
        mSpan->setVisible(false);
        mMainLayout->addWidget(mSpan);

        mShrinkable << mSpan << mClose << mReload;
    }

    _addSeparator();

    QList<SearchLineEdit::eToolButton> tools;
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonMatchCase);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonMatchWord);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonWildCard);
    tools.push_back(SearchLineEdit::eToolButton::ToolButtonBackward);

    mSearch = new SearchLineEdit(owner);
    mSearch->initialize(tools);
    mSearch->setPlaceholderText(tr("search a phrase in logs"));
    mSearch->setToolTip(tr("Moves to the next row that carries the phrase. It removes no row."));
    // The field spends part of its width on the marks it draws inside itself; the rest is the
    // room to type in, and that is the number worth naming.
    const int chrome{ mSearch->chromeWidth() };
    mSearch->setMinimumWidth(chrome + LogSessionBar::SEARCH_TEXT_MIN);
    mSearch->setMaximumWidth(chrome + LogSessionBar::SEARCH_TEXT_MAX);
    mSearch->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    // The field carries the stretch and the slack does not, so spare width grows the field up
    // to its maximum and only what is left over goes to the gap before the counters.
    mMainLayout->addWidget(mSearch, 1);

    // The bridge from searching to filtering. Searching moves the cursor and keeps every row;
    // pressing this keeps only the rows that carry the phrase, and says so with a chip.
    mFilterMatches = _addButton(NELusanCommon::iconFilter(NELusanCommon::SizeBig)
                               , tr("Keep only the rows that carry this phrase"));
    mFilterMatches->setEnabled(false);
    mFilterMatches->setWhatsThis(tr("Turns the typed phrase into a filter on the message column. The filter appears as a chip and one click drops it again."));

    // Which rows the search walks. The label names the state it is in, and the pressed look
    // marks the one that is not the default, so a glance answers both.
    mSearchScope = new QToolButton(owner);
    mSearchScope->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    mSearchScope->setAutoRaise(true);
    mSearchScope->setCheckable(true);
    mSearchScope->setText(tr("Visible"));
    mSearchScope->setToolTip(tr("The search walks the rows the table shows. Press to walk every row this window holds."));
    mSearchScope->setWhatsThis(tr("In All logs a hit that a filter hides is drawn in place and marked, with a line naming what hid it."));
    mMainLayout->addWidget(mSearchScope);
    connect(mSearchScope, &QToolButton::toggled, this, [this](bool checked) {
            mSearchScope->setText(checked ? tr("All logs") : tr("Visible"));
            mSearchScope->setToolTip(checked
                ? tr("The search walks every row this window holds. A hit a filter hides is drawn in place and marked.")
                : tr("The search walks the rows the table shows. Press to walk every row this window holds."));
        });

    mMainLayout->addStretch(0);

    mCounters = new QLabel(owner);
    mCounters->setObjectName(QStringLiteral("logSessionCounters"));
    mMainLayout->addWidget(mCounters);

    _addSeparator();

    mMoveTop    = _addButton(NELusanCommon::iconScrollTop(NELusanCommon::SizeBig)
                            , tr("Move to top"));
    mMoveTop->setWhatsThis(tr("Moves the table to its first row, where the logging started."));

    mMoveBottom = _addButton(NELusanCommon::iconScrollBottom(NELusanCommon::SizeBig)
                            , mMode == eSessionMode::ModeLive
                              ? tr("Move to bottom and keep following the new logs")
                              : tr("Move to bottom"));

    if (mMode == eSessionMode::ModeLive)
    {
        // The two are one operation: going to the last row and staying on it. While the
        // button is checked every arriving log keeps the table at its end.
        mMoveBottom->setCheckable(true);
        mMoveBottom->setChecked(true);
        mMoveBottom->setWhatsThis(tr("Moves the table to its last row and holds it there, so the newest log stays in sight. Scrolling away releases it."));
    }
    else
    {
        mMoveBottom->setWhatsThis(tr("Moves the table to the last row of the archive."));
    }
}

void LogSessionBar::_buildNoticeRow(void)
{
    mNoticeRow = new QWidget(this);
    mNoticeRow->setObjectName(QStringLiteral("logSessionNotice"));
    QVBoxLayout* lines = new QVBoxLayout(mNoticeRow);
    lines->setContentsMargins(0, 0, 0, 0);
    lines->setSpacing(0);

    for (int i = 0; i < static_cast<int>(LogSessionBar::eNotice::NoticeCount); ++i)
    {
        lines->addWidget(_buildNoticeLine(static_cast<LogSessionBar::eNotice>(i)));
    }

    mNoticeRow->setVisible(false);
}

QWidget* LogSessionBar::_buildNoticeLine(LogSessionBar::eNotice which)
{
    const int slot{ static_cast<int>(which) };

    QWidget* line = new QWidget(mNoticeRow);
    QHBoxLayout* layout = new QHBoxLayout(line);
    layout->setContentsMargins(_barMargin, 1, _barMargin, 1);
    layout->setSpacing(6);

    QLabel* mark = new QLabel(line);
    mark->setPixmap(NELusanCommon::iconWarning(NELusanCommon::SizeBig).pixmap(NaviToolbarWindow::NAVI_TOOL_ICON, NaviToolbarWindow::NAVI_TOOL_ICON));
    layout->addWidget(mark);

    mNoticeText[slot] = new QLabel(line);
    mNoticeText[slot]->setWordWrap(false);
    mNoticeText[slot]->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    layout->addWidget(mNoticeText[slot], 1);

    mNoticeLink[slot] = new QToolButton(line);
    mNoticeLink[slot]->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    mNoticeLink[slot]->setAutoRaise(true);
    mNoticeLink[slot]->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    mNoticeLink[slot]->setVisible(false);
    layout->addWidget(mNoticeLink[slot]);
    connect(mNoticeLink[slot], &QToolButton::clicked, this, [this, which]() { emit signalNoticeAction(which); });

    QToolButton* dismiss = new QToolButton(line);
    dismiss->setIcon(NELusanCommon::iconClose(NELusanCommon::SizeBig));
    dismiss->setIconSize(QSize(NaviToolbarWindow::NAVI_TOOL_ICON, NaviToolbarWindow::NAVI_TOOL_ICON));
    dismiss->setAutoRaise(true);
    dismiss->setToolTip(tr("Hide this notice"));
    layout->addWidget(dismiss);
    connect(dismiss, &QToolButton::clicked, this, [this, which]() { hideNotice(which); });

    line->setVisible(false);
    mNoticeLine[slot] = line;
    return line;
}

void LogSessionBar::_updateNoticeRow(void)
{
    bool any{ false };
    for (int i = 0; i < static_cast<int>(LogSessionBar::eNotice::NoticeCount); ++i)
    {
        any = any || mNoticeLine[i]->isVisibleTo(mNoticeRow);
    }

    mNoticeRow->setVisible(any);
}

QToolButton* LogSessionBar::_addButton(const QIcon& icon, const QString& toolTip)
{
    QToolButton* button = new QToolButton(mMainLayout->parentWidget());
    button->setIcon(icon);
    button->setToolTip(toolTip);
    button->setStatusTip(toolTip);
    button->setAutoRaise(true);
    mMainLayout->addWidget(button);
    mButtons.append(button);
    return button;
}

QWidget* LogSessionBar::_addSeparator(void)
{
    QFrame* line = new QFrame(mMainLayout->parentWidget());
    line->setFrameShape(QFrame::Shape::VLine);
    line->setFrameShadow(QFrame::Shadow::Sunken);
    line->setFixedWidth(1);
    mMainLayout->addWidget(line);
    return line;
}

QIcon LogSessionBar::_stateIcon(const QColor& color)
{
    QPixmap pixmap(_dotBox, _dotBox);
    pixmap.fill(Qt::GlobalColor::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(NELogPalette::withOpacity(color, NELogPalette::eLogOpacity::OpacityTint));
    painter.drawEllipse(QPointF(_dotBox / 2.0, _dotBox / 2.0), _dotBox / 2.0, _dotBox / 2.0);
    painter.setBrush(color);
    painter.drawEllipse(QPointF(_dotBox / 2.0, _dotBox / 2.0), _dotBox / 4.0, _dotBox / 4.0);
    painter.end();

    return QIcon(pixmap);
}

void LogSessionBar::setLiveState(LogSessionBar::eLiveState state, const QString& address, uint16_t port)
{
    if (mMode != eSessionMode::ModeLive)
        return;

    mAddress = address.isEmpty() ? QString() : QString("%1:%2").arg(address).arg(port);

    QColor  color{ NELogPalette::textColor(NELogPalette::eLogColorRole::RoleNotset) };
    QString word;
    QString hint;

    switch (state)
    {
    case eLiveState::StateConnected:
        color = NELogPalette::stateColor(NELogPalette::eLogStateRole::StateSaved);
        word  = tr("Connected");
        hint  = tr("Recording the logs of %1").arg(mAddress);
        break;

    case eLiveState::StatePaused:
        color = NELogPalette::stateColor(NELogPalette::eLogStateRole::StatePending);
        word  = tr("Paused");
        hint  = tr("Connected to %1. The database stays open and nothing is written while paused.").arg(mAddress);
        break;

    case eLiveState::StateStopped:
        // Stopping closes the database. It does not leave the collector, so the mark is the
        // colour of a connection that is not recording, never the grey of no connection.
        color = NELogPalette::stateColor(NELogPalette::eLogStateRole::StatePending);
        word  = tr("Stopped");
        hint  = tr("Connected to %1. The database is closed and nothing is written.").arg(mAddress);
        break;

    case eLiveState::StateDisconnected:
    default:
        word = tr("Not connected");
        hint = tr("Connect to a log collector in the Live Logs navigation panel.");
        break;
    }

    mIdentity->setIcon(_stateIcon(color));
    mIdentity->setText(mAddress.isEmpty() ? word : QString("%1  %2").arg(word, mAddress));
    mStateHint = hint;
    _drawIdentityHint();
    _updateDensity();
}

void LogSessionBar::setDatabasePath(const QString& path)
{
    if (mMode != eSessionMode::ModeLive)
        return;

    mFullPath = path;
    _drawIdentityHint();
}

void LogSessionBar::_drawIdentityHint(void)
{
    const QString hint{ mFullPath.isEmpty() ? mStateHint : QString("%1\n%2").arg(mStateHint, mFullPath) };
    mIdentity->setToolTip(hint);
    mIdentity->setStatusTip(mStateHint);
}

void LogSessionBar::setArchive(const QString& fileName, const QString& fullPath, bool inWorkspace)
{
    if (mMode != eSessionMode::ModeOffline)
        return;

    mFullPath = fullPath;
    if (fileName.isEmpty())
    {
        mIdentity->setIcon(NELusanCommon::iconOfflineLogWindow(NELusanCommon::SizeBig));
        mIdentity->setText(tr("No archive open"));
        mIdentity->setToolTip(tr("Open a log archive to read logs collected earlier."));
    }
    else
    {
        mIdentity->setIcon(inWorkspace ? NELusanCommon::iconOfflineLogWindow(NELusanCommon::SizeBig)
                                       : NELusanCommon::iconWarning(NELusanCommon::SizeBig));
        mIdentity->setText(fileName);
        mIdentity->setToolTip(inWorkspace ? fullPath
                                          : tr("%1\n\nThis archive is not in the current workspace.").arg(fullPath));
    }

    mIdentity->setStatusTip(mIdentity->toolTip());
    _updateDensity();
}

void LogSessionBar::setSpan(TIME64 firstUs, TIME64 lastUs)
{
    if ((mSpan == nullptr) || (firstUs == 0) || (lastUs < firstUs))
    {
        if (mSpan != nullptr)
        {
            mSpan->setVisible(false);
        }

        return;
    }

    const QDateTime first{ QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(firstUs / 1000u)) };
    const QDateTime last { QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(lastUs  / 1000u)) };
    const qint64    secs { first.secsTo(last) };

    QString span;
    if (secs >= 3600)
    {
        span = tr("%1h %2m").arg(secs / 3600).arg((secs % 3600) / 60);
    }
    else if (secs >= 60)
    {
        span = tr("%1m %2s").arg(secs / 60).arg(secs % 60);
    }
    else
    {
        span = tr("%1s").arg(secs);
    }

    const QString text{ QString("%1 - %2  (%3)").arg(first.toString("hh:mm:ss"), last.toString("hh:mm:ss"), span) };
    mSpan->setText(text);
    mSpan->setToolTip(tr("First log %1, last log %2").arg(first.toString(Qt::DateFormat::ISODate), last.toString(Qt::DateFormat::ISODate)));
    _updateDensity();
}

void LogSessionBar::setCounters(int shown, int total)
{
    if ((shown == mShown) && (total == mTotal))
        return;

    mShown = shown;
    mTotal = total;
    _drawCounters();
}

void LogSessionBar::_drawCounters(void)
{
    const QLocale locale{ QLocale::system() };
    const QString shown{ locale.toString(mShown) };
    const QString total{ locale.toString(mTotal) };
    const bool    brief{ mDensity >= 1 };

    QString text;
    if (mTotal == 0)
    {
        text = brief ? QString("0") : tr("no rows");
    }
    else if (mShown == mTotal)
    {
        text = brief ? total : tr("%1 rows").arg(total);
    }
    else
    {
        text = brief ? QString("%1/%2").arg(shown, total) : tr("%1 of %2 rows").arg(shown, total);
    }

    mCounters->setText(text);
    mCounters->setToolTip(mShown == mTotal ? tr("Every row this window holds is shown")
                                           : tr("%1 rows are kept out by the filters that are on").arg(locale.toString(mTotal - mShown)));
}

void LogSessionBar::showNotice(LogSessionBar::eNotice which, const QString& text, const QString& actionText /*= QString()*/)
{
    const int slot{ static_cast<int>(which) };
    if ((slot < 0) || (slot >= static_cast<int>(LogSessionBar::eNotice::NoticeCount)))
        return;

    mNoticeText[slot]->setText(text);
    mNoticeText[slot]->setToolTip(text);
    mNoticeLink[slot]->setText(actionText);
    mNoticeLink[slot]->setVisible(actionText.isEmpty() == false);
    mNoticeLine[slot]->setVisible(true);
    _updateNoticeRow();
}

void LogSessionBar::hideNotice(LogSessionBar::eNotice which)
{
    const int slot{ static_cast<int>(which) };
    if ((slot < 0) || (slot >= static_cast<int>(LogSessionBar::eNotice::NoticeCount)))
        return;

    mNoticeLine[slot]->setVisible(false);
    _updateNoticeRow();
}

bool LogSessionBar::isSearchingAllLogs(void) const
{
    return mSearchScope->isChecked();
}

bool LogSessionBar::isFollowing(void) const
{
    return mMoveBottom->isCheckable() && mMoveBottom->isChecked();
}

void LogSessionBar::setFollowing(bool follow)
{
    if (mMoveBottom->isCheckable() && (mMoveBottom->isChecked() != follow))
    {
        const QSignalBlocker blocker(mMoveBottom);
        mMoveBottom->setChecked(follow);
    }
}

void LogSessionBar::_fillIdentityMenu(void)
{
    mIdentityMenu->clear();
    if (mMode == eSessionMode::ModeLive)
    {
        QAction* disconnect = mIdentityMenu->addAction(tr("Disconnect"));
        disconnect->setEnabled(mAddress.isEmpty() == false);
        connect(disconnect, &QAction::triggered, this, [this]() { emit signalDisconnectRequested(); });

        QAction* copy = mIdentityMenu->addAction(tr("Copy the collector address"));
        copy->setEnabled(mAddress.isEmpty() == false);
        connect(copy, &QAction::triggered, this, [this]() { QApplication::clipboard()->setText(mAddress); });

        mIdentityMenu->addSeparator();

        QAction* pause = mIdentityMenu->addAction(mPause->isChecked() ? tr("Resume in the same database")
                                                                     : tr("Pause, keeping the database open"));
        pause->setEnabled(mPause->isEnabled());
        connect(pause, &QAction::triggered, this, [this]() { mPause->click(); });

        QAction* stop = mIdentityMenu->addAction(mStop->isChecked() ? tr("Resume in a new database")
                                                                   : tr("Stop and close the database"));
        stop->setEnabled(mStop->isEnabled());
        connect(stop, &QAction::triggered, this, [this]() { mStop->click(); });

        QAction* clear = mIdentityMenu->addAction(tr("Clear this window"));
        clear->setEnabled(mClear->isEnabled());
        connect(clear, &QAction::triggered, this, [this]() { mClear->click(); });

        if (mFullPath.isEmpty() == false)
        {
            mIdentityMenu->addSeparator();
            _addFileActions();
        }
    }
    else
    {
        QAction* reload = mIdentityMenu->addAction(tr("Reload"));
        reload->setEnabled(mReload->isEnabled());
        connect(reload, &QAction::triggered, this, [this]() { mReload->click(); });

        QAction* close = mIdentityMenu->addAction(tr("Close"));
        close->setEnabled(mClose->isEnabled());
        connect(close, &QAction::triggered, this, [this]() { mClose->click(); });

        mIdentityMenu->addSeparator();
        _addFileActions();
    }
}

void LogSessionBar::_addFileActions(void)
{
    QAction* folder = mIdentityMenu->addAction(tr("Open the containing folder"));
    folder->setEnabled(mFullPath.isEmpty() == false);
    connect(folder, &QAction::triggered, this, [this]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(mFullPath).absolutePath()));
        });

    QAction* copy = mIdentityMenu->addAction(tr("Copy the full path"));
    copy->setEnabled(mFullPath.isEmpty() == false);
    connect(copy, &QAction::triggered, this, [this]() { QApplication::clipboard()->setText(mFullPath); });
}

int LogSessionBar::_requiredWidth(void) const
{
    const QMargins margins{ mMainLayout->contentsMargins() };
    int width{ margins.left() + margins.right() };
    int items{ 0 };

    for (int i = 0; i < mMainLayout->count(); ++i)
    {
        QWidget* widget{ mMainLayout->itemAt(i)->widget() };
        if ((widget == nullptr) || widget->isHidden())
            continue;

        ++items;
        width += (widget == mSearch) ? mSearch->minimumWidth() : widget->sizeHint().width();
    }

    return width + ((items > 1) ? ((items - 1) * mMainLayout->spacing()) : 0);
}

void LogSessionBar::_applyDensity(int level)
{
    mDensity = level;
    mIdentity->setToolButtonStyle(level < 2 ? Qt::ToolButtonStyle::ToolButtonTextBesideIcon
                                            : Qt::ToolButtonStyle::ToolButtonIconOnly);

    bool anyLead{ false };
    for (int i = 0; i < mShrinkable.size(); ++i)
    {
        // The span label stays hidden until it has a value to show.
        const bool empty{ (mShrinkable[i] == mSpan) && (mSpan->text().isEmpty()) };
        const bool show { (level < (3 + i)) && (empty == false) };
        mShrinkable[i]->setVisible(show);
        anyLead = anyLead || show;
    }

    mLeadSeparator->setVisible(anyLead);
    _drawCounters();
}

void LogSessionBar::_updateDensity(void)
{
    if (mInDensity)
        return;

    mInDensity = true;
    const int maxLevel{ 3 + static_cast<int>(mShrinkable.size()) };
    for (int level = 0; level <= maxLevel; ++level)
    {
        _applyDensity(level);
        if ((level == maxLevel) || (_requiredWidth() <= width()))
            break;
    }

    mInDensity = false;
}

void LogSessionBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    _updateDensity();
}
