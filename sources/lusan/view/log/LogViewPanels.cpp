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
 *  \file        lusan/view/log/LogViewPanels.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the panels a log window drops from its bar.
 *
 ************************************************************************/

#include "lusan/view/log/LogViewPanels.hpp"

#include "lusan/common/NELogPalette.hpp"

#include <QApplication>
#include <QCursor>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    //! The air the panel keeps around its content.
    constexpr int   _panelPad       { 6 };

    //! The gap between two blocks of the panel.
    constexpr int   _panelGap       { 4 };

    //! The rounding of the panel corners.
    constexpr qreal _panelRound     { 6.0 };

    //! The narrowest and the widest a panel opens at.
    constexpr int   _panelMinWidth  { 240 };
    constexpr int   _panelMaxWidth  { 460 };

    //! The tallest a list grows before it starts to scroll.
    constexpr int   _listMaxHeight  { 340 };

    //! Where a list row keeps the column it stands for.
    constexpr int   _columnRole     { Qt::ItemDataRole::UserRole + 1 };

    //!< Mixes @p over into @p base, where @p amount is how much of @p over is taken.
    QColor blend(const QColor& base, const QColor& over, qreal amount)
    {
        const qreal keep{ 1.0 - amount };
        return QColor( qRound((base.red()   * keep) + (over.red()   * amount))
                     , qRound((base.green() * keep) + (over.green() * amount))
                     , qRound((base.blue()  * keep) + (over.blue()  * amount)));
    }

    //!< The line the panel is drawn around with, in the theme in use.
    QColor panelBorder(const QPalette& palette)
    {
        const QColor ground{ palette.color(QPalette::ColorRole::Base) };
        return NELogPalette::isDarkTheme() ? blend(ground, QColor(Qt::GlobalColor::white), 0.24)
                                           : blend(ground, QColor(Qt::GlobalColor::black), 0.22);
    }

    //!< The name the reader sees for a column.
    QString columnName(LoggingModelBase::eColumn column)
    {
        const int index{ static_cast<int>(column) };
        const QStringList& names{ LoggingModelBase::getHeaderList() };
        return ((index >= 0) && (index < names.size())) ? names.at(index) : QString();
    }

    //!< Gives the list the height of the rows it holds, so the panel carries no empty box.
    void fitToRows(QListWidget* list)
    {
        int height{ 2 * list->frameWidth() };
        for (int i = 0; i < list->count(); ++i)
        {
            height += list->sizeHintForRow(i);
        }

        list->setFixedHeight(qMin(height + 2, _listMaxHeight));
    }

    //!< Builds a small flat button that carries text alone.
    QToolButton* makeLink(const QString& text, const QString& hint, QWidget* parent)
    {
        QToolButton* button = new QToolButton(parent);
        button->setText(text);
        button->setToolTip(hint);
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
        button->setAutoRaise(true);
        button->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        return button;
    }
}

//////////////////////////////////////////////////////////////////////////
// LogPopoverBase class implementation
//////////////////////////////////////////////////////////////////////////

LogPopoverBase::LogPopoverBase(const QString& title, QWidget* parent /*= nullptr*/)
    : QFrame  (parent)
    , mLayout (nullptr)
{
    setWindowFlags(Qt::WindowType::Popup);
    setFrameShape(QFrame::Shape::NoFrame);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);
    setAttribute(Qt::WidgetAttribute::WA_NoSystemBackground, true);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(_panelPad, _panelPad, _panelPad, _panelPad);
    mLayout->setSpacing(_panelGap);

    QLabel* caption = new QLabel(title, this);
    QFont face{ caption->font() };
    face.setBold(true);
    caption->setFont(face);
    mLayout->addWidget(caption);
}

void LogPopoverBase::addSeparator(void)
{
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::Shape::HLine);
    line->setFrameShadow(QFrame::Shadow::Plain);
    line->setLineWidth(1);
    mLayout->addWidget(line);
}

void LogPopoverBase::showAt(const QRect& anchor)
{
    ensurePolished();
    mLayout->activate();

    const QSize hint{ sizeHint() };
    const QSize size{ qBound(_panelMinWidth, qMax(anchor.width(), hint.width()), _panelMaxWidth), hint.height() };
    QRect place{ QPoint(anchor.left(), anchor.bottom() + 1), size };

    const QScreen* screen{ QGuiApplication::screenAt(anchor.center()) };
    const QRect area{ screen != nullptr ? screen->availableGeometry() : QRect() };
    if (area.isValid())
    {
        if (place.right() > area.right())
            place.moveRight(area.right());
        if (place.left() < area.left())
            place.moveLeft(area.left());

        if (place.bottom() > area.bottom())
        {
            // There is no room under the control, so the panel opens over it instead.
            const int above{ anchor.top() - 1 - size.height() };
            place.moveTop(above >= area.top() ? above : qMax(area.top(), area.bottom() - size.height()));
        }
    }

    setGeometry(place);
    show();
    raise();
    setFocus(Qt::FocusReason::PopupFocusReason);
}

QRect LogPopoverBase::handOverRect(void) const
{
    // The panel that follows opens under the rectangle it is given, so a one pixel line right
    // above this panel lands it on the same corner.
    const QRect place{ geometry() };
    return QRect(place.left(), place.top() - 1, place.width(), 1);
}

void LogPopoverBase::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter.setPen(QPen(panelBorder(palette()), 1.0));
    painter.setBrush(palette().color(QPalette::ColorRole::Base));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), _panelRound, _panelRound);

    QFrame::paintEvent(event);
}

void LogPopoverBase::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key::Key_Escape)
    {
        hide();
        event->accept();
        return;
    }

    QFrame::keyPressEvent(event);
}

//////////////////////////////////////////////////////////////////////////
// LogColumnPicker class implementation
//////////////////////////////////////////////////////////////////////////

LogColumnPicker::LogColumnPicker(QWidget* parent /*= nullptr*/)
    : LogPopoverBase(tr("Columns"), parent)
    , mList         (nullptr)
    , mFilling      (false)
    , mPending      (false)
{
    _addPresets();

    mList = new QListWidget(this);
    mList->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
    mList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    mList->setUniformItemSizes(true);
    mList->setToolTip(tr("Check a column to show it in the table."));
    panelLayout()->addWidget(mList, 1);

    QLabel* hint = new QLabel(tr("Drag a row to change the order of the columns."), this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    panelLayout()->addWidget(hint);

    addSeparator();

    QToolButton* reset{ makeLink(tr("Reset columns"), tr("Brings back the columns the application starts with"), this) };
    QHBoxLayout* footer = new QHBoxLayout();
    footer->setContentsMargins(0, 0, 0, 0);
    footer->addStretch(1);
    footer->addWidget(reset);
    panelLayout()->addLayout(footer);

    connect(reset, &QToolButton::clicked, this, [this]() {
            hide();
            emit signalColumnsReset();
        });

    connect(mList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
            if (mFilling || (item == nullptr))
                return;

            if (mPending)
                return;

            // The list is changed from inside the signal that says it changed, so the move
            // waits for the event loop, and the report goes out with it in one turn.
            mPending = true;
            const int column{ item->data(_columnRole).toInt() };
            QTimer::singleShot(0, this, [this, column]() {
                    _placeChecked(column);
                    mPending = false;
                    if (mFilling == false)
                    {
                        emit signalColumnsChanged(_chosen());
                    }
                });
        });

    // A moved row reaches the model in either of two shapes: one row move, or a removal
    // followed by an insertion. All three signals are watched so the shape does not matter,
    // and the report is deferred, so the table is rebuilt once on the finished order rather
    // than once per half of a move.
    connect(mList->model(), &QAbstractItemModel::rowsInserted, this
            , [this](const QModelIndex&, int, int) { _report(); });
    connect(mList->model(), &QAbstractItemModel::rowsRemoved, this
            , [this](const QModelIndex&, int, int) { _report(); });
    connect(mList->model(), &QAbstractItemModel::rowsMoved, this
            , [this](const QModelIndex&, int, int, const QModelIndex&, int) { _report(); });
}

void LogColumnPicker::_addPresets(void)
{
    const struct { const char* name; const char* hint; LogColumnPicker::ListColumns columns; } _presets[]
    {
          { QT_TR_NOOP("Minimal") , QT_TR_NOOP("The time, the priority and the message")
          , { LoggingModelBase::eColumn::LogColumnTimestamp
            , LoggingModelBase::eColumn::LogColumnPriority
            , LoggingModelBase::eColumn::LogColumnMessage } }

        , { QT_TR_NOOP("Identity"), QT_TR_NOOP("Adds the process and the thread that wrote the row")
          , { LoggingModelBase::eColumn::LogColumnTimestamp
            , LoggingModelBase::eColumn::LogColumnPriority
            , LoggingModelBase::eColumn::LogColumnSource
            , LoggingModelBase::eColumn::LogColumnThread
            , LoggingModelBase::eColumn::LogColumnMessage } }

        , { QT_TR_NOOP("Timing")  , QT_TR_NOOP("Adds the elapsed time of a call and the time the collector received the row")
          , { LoggingModelBase::eColumn::LogColumnTimestamp
            , LoggingModelBase::eColumn::LogColumnTimeReceived
            , LoggingModelBase::eColumn::LogColumnPriority
            , LoggingModelBase::eColumn::LogColumnTimeDuration
            , LoggingModelBase::eColumn::LogColumnMessage } }
    };

    QHBoxLayout* row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(_panelGap);
    for (const auto& preset : _presets)
    {
        QToolButton* button{ makeLink(tr(preset.name), tr(preset.hint), this) };
        const LogColumnPicker::ListColumns columns{ preset.columns };
        connect(button, &QToolButton::clicked, this, [this, columns]() {
                setColumns(columns);
                emit signalColumnsChanged(columns);
            });

        row->addWidget(button);
    }

    row->addStretch(1);
    panelLayout()->addLayout(row);
}

void LogColumnPicker::setColumns(const LogColumnPicker::ListColumns& active)
{
    mFilling = true;
    mList->clear();

    for (LoggingModelBase::eColumn column : active)
    {
        if (column != LoggingModelBase::eColumn::LogColumnRail)
        {
            _addRow(column, true);
        }
    }

    for (int i = 0; i < static_cast<int>(LoggingModelBase::eColumn::LogColumnCount); ++i)
    {
        const LoggingModelBase::eColumn column{ static_cast<LoggingModelBase::eColumn>(i) };
        if ((column != LoggingModelBase::eColumn::LogColumnRail) && (active.contains(column) == false))
        {
            _addRow(column, false);
        }
    }

    fitToRows(mList);
    mFilling = false;
}

void LogColumnPicker::_addRow(LoggingModelBase::eColumn column, bool shown)
{
    QListWidgetItem* item = new QListWidgetItem(columnName(column), mList);
    item->setData(_columnRole, static_cast<int>(column));
    item->setCheckState(shown ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    if (column == LoggingModelBase::eColumn::LogColumnMessage)
    {
        // The message is what the table is for. It stays, and the row says so rather than
        // leaving the reader to wonder where it went.
        item->setFlags(Qt::ItemFlag::ItemIsEnabled);
        item->setCheckState(Qt::CheckState::Checked);
        item->setToolTip(tr("The message is always shown."));
    }
    else
    {
        item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable
                     | Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsDragEnabled);
    }
}

LogColumnPicker::ListColumns LogColumnPicker::_chosen(void) const
{
    LogColumnPicker::ListColumns columns;
    for (int i = 0; i < mList->count(); ++i)
    {
        const QListWidgetItem* item{ mList->item(i) };
        if (item->checkState() == Qt::CheckState::Checked)
        {
            columns.append(static_cast<LoggingModelBase::eColumn>(item->data(_columnRole).toInt()));
        }
    }

    return columns;
}

void LogColumnPicker::_placeChecked(int column)
{
    int from{ -1 };
    for (int i = 0; (from < 0) && (i < mList->count()); ++i)
    {
        if (mList->item(i)->data(_columnRole).toInt() == column)
        {
            from = i;
        }
    }

    if (from < 0)
        return;

    // The table places a column against the ones it already shows, and the rail opens that
    // list. The same call answers here, so the panel and the row menu agree on the place.
    LoggingModelBase::ListColumns shown{ LoggingModelBase::eColumn::LogColumnRail };
    QList<int> rows{ -1 };
    for (int i = 0; i < mList->count(); ++i)
    {
        if ((i != from) && (mList->item(i)->checkState() == Qt::CheckState::Checked))
        {
            shown.append(static_cast<LoggingModelBase::eColumn>(mList->item(i)->data(_columnRole).toInt()));
            rows.append(i);
        }
    }

    int to{ rows.last() + 1 };
    if (mList->item(from)->checkState() == Qt::CheckState::Checked)
    {
        const int place{ LoggingModelBase::placeOfColumn(shown, static_cast<LoggingModelBase::eColumn>(column)) };
        to = (place < rows.size()) ? rows.at(place) : (rows.last() + 1);
    }

    if (to == from)
        return;

    mFilling = true;
    QListWidgetItem* moved{ mList->takeItem(from) };
    mList->insertItem(to > from ? to - 1 : to, moved);
    mList->setCurrentItem(moved);
    mFilling = false;
}

void LogColumnPicker::_report(void)
{
    if (mFilling || mPending)
        return;

    mPending = true;
    QTimer::singleShot(0, this, [this]() {
            mPending = false;
            if (mFilling == false)
            {
                emit signalColumnsChanged(_chosen());
            }
        });
}

//////////////////////////////////////////////////////////////////////////
// LogFilterPanel class implementation
//////////////////////////////////////////////////////////////////////////

LogFilterPanel::LogFilterPanel(QWidget* parent /*= nullptr*/)
    : LogPopoverBase(tr("Filters"), parent)
    , mList         (nullptr)
    , mClearAll     (nullptr)
{
    mList = new QListWidget(this);
    mList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    panelLayout()->addWidget(mList, 1);

    QLabel* hint = new QLabel(tr("Pick a column to choose what it keeps."), this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    panelLayout()->addWidget(hint);

    addSeparator();

    mClearAll = makeLink(tr("Clear filters"), tr("Drops every filter this window has on, and brings every row back"), this);
    QHBoxLayout* footer = new QHBoxLayout();
    footer->setContentsMargins(0, 0, 0, 0);
    footer->addStretch(1);
    footer->addWidget(mClearAll);
    panelLayout()->addLayout(footer);

    connect(mClearAll, &QToolButton::clicked, this, [this]() {
            hide();
            emit signalClearFilters();
        });

    connect(mList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
            if ((item == nullptr) || (item->data(_columnRole).isValid() == false))
                return;

            emit signalOpenFilter(item->data(_columnRole).toInt(), handOverRect());
        });
}

void LogFilterPanel::setEntries(const LogFilterPanel::ListEntries& entries)
{
    mList->clear();

    bool anyFilter{ false };
    for (const LogFilterPanel::sEntry& entry : entries)
    {
        anyFilter = anyFilter || (entry.state.isEmpty() == false);
        if (entry.shown)
        {
            _addRow(entry);
        }
    }

    // The columns out of the table are named once, under a line of their own, instead of
    // every one of their rows carrying the same mark.
    bool divided{ false };
    for (const LogFilterPanel::sEntry& entry : entries)
    {
        if (entry.shown)
            continue;

        if (divided == false)
        {
            _addDivider();
            divided = true;
        }

        _addRow(entry);
    }

    fitToRows(mList);
    mClearAll->setEnabled(anyFilter);
}

void LogFilterPanel::_addRow(const LogFilterPanel::sEntry& entry)
{
    const bool narrowed{ entry.state.isEmpty() == false };

    QListWidgetItem* item = new QListWidgetItem(mList);
    item->setData(_columnRole, static_cast<int>(entry.column));
    item->setText(narrowed ? tr("%1: %2").arg(columnName(entry.column)).arg(entry.state)
                           : columnName(entry.column));

    // The weight marks the columns that hold something back. They are the few, and they are
    // what the panel is opened for.
    QFont face{ item->font() };
    face.setBold(narrowed);
    item->setFont(face);

    QString hint{ narrowed ? tr("Keeps: %1").arg(entry.state) : tr("Every value passes.") };
    if (entry.shown == false)
    {
        hint += QString("\n") + tr("The table does not show this column.");
        item->setForeground(QApplication::palette().color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::Text));
    }

    item->setToolTip(hint);
}

void LogFilterPanel::_addDivider(void)
{
    QListWidgetItem* item = new QListWidgetItem(tr("Not shown in the table"), mList);
    item->setFlags(Qt::ItemFlag::NoItemFlags);
    item->setForeground(QApplication::palette().color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::Text));

    QFont face{ item->font() };
    face.setItalic(true);
    item->setFont(face);
}
