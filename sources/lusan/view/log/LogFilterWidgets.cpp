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
 *  \file        lusan/view/log/LogFilterWidgets.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header filter widgets.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/log/LogFilterWidgets.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QScreen>
#include <QSet>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    //! The air the panel keeps between its border and its content.
    constexpr int   _panelPad       { 6 };

    //! The air between two rows of the panel.
    constexpr int   _panelGap       { 4 };

    //! The rounding of the panel corners.
    constexpr qreal _panelRound     { 6.0 };

    //! The narrowest and the widest a panel is opened at.
    constexpr int   _panelMinWidth  { 220 };
    constexpr int   _panelMaxWidth  { 640 };

    //! The tallest the list of values grows before it starts to scroll.
    constexpr int   _listMaxHeight  { 320 };

    //! The room a list row keeps for its check mark and its air.
    constexpr int   _listRowLead    { 44 };

    //! The number of entries a list carries before it gets a find box.
    constexpr int   _searchFrom     { 8 };

    //! The entries measured to work out how wide the panel opens.
    constexpr int   _measureUpTo    { 200 };

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
}

//////////////////////////////////////////////////////////////////////////
// LogFilterBase class implementation
//////////////////////////////////////////////////////////////////////////
LogFilterBase::LogFilterBase(QWidget* parent)
    : QFrame    (parent)
    , mWidget   (nullptr)
    , mData     ( )
    , mLayout   (nullptr)
{
    setWindowFlags(Qt::WindowType::Popup);
    setFrameShape(QFrame::Shape::NoFrame);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);
    setAttribute(Qt::WidgetAttribute::WA_NoSystemBackground, true);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(_panelPad, _panelPad, _panelPad, _panelPad);
    mLayout->setSpacing(_panelGap);
}

void LogFilterBase::setWidget(QWidget* widget)
{
    Q_ASSERT(mWidget == nullptr);
    Q_ASSERT(widget != nullptr);

    mWidget = widget;
    mLayout->addWidget(mWidget);
}

void LogFilterBase::setDataFilter(const NELusanCommon::FilterString& filter)
{
    setDataString(filter.text);
}

void LogFilterBase::clearFilter()
{
    // The panel is a window of its own, so closing it is hiding the panel. Hiding the control
    // inside it instead leaves an empty box behind for every following open.
    hide();
}

void LogFilterBase::showFilter()
{
    show();
    raise();
    if (mWidget != nullptr)
    {
        mWidget->show();
        mWidget->setFocus(Qt::FocusReason::PopupFocusReason);
    }
}

void LogFilterBase::showFilterAt(const QRect& anchor)
{
    ensurePolished();
    mLayout->activate();

    const QSize size{ popupSize(anchor.width()) };
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
            // There is no room under the section, so the panel opens over the header instead.
            const int above{ anchor.top() - 1 - size.height() };
            place.moveTop(above >= area.top() ? above : qMax(area.top(), area.bottom() - size.height()));
        }
    }

    setGeometry(place);
    showFilter();
}

QSize LogFilterBase::popupSize(int anchorWidth) const
{
    const QSize hint{ sizeHint() };
    const int width{ qBound(_panelMinWidth, qMax(anchorWidth, hint.width()), _panelMaxWidth) };
    return QSize(width, hint.height());
}

void LogFilterBase::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    painter.setPen(QPen(panelBorder(palette()), 1.0));
    painter.setBrush(palette().color(QPalette::ColorRole::Base));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), _panelRound, _panelRound);

    QFrame::paintEvent(event);
}

void LogFilterBase::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key::Key_Escape) || (event->key() == Qt::Key::Key_Return) || (event->key() == Qt::Key::Key_Enter))
    {
        hide();
        event->accept();
        return;
    }

    QFrame::keyPressEvent(event);
}

//////////////////////////////////////////////////////////////////////////
// LogComboFilterBase class implementation
//////////////////////////////////////////////////////////////////////////
LogComboFilterBase::LogComboFilterBase(QWidget* parent)
    : LogFilterBase (parent)
    , mSearch       (nullptr)
    , mSummary      (nullptr)
    , mBtnAll       (nullptr)
    , mBtnNone      (nullptr)
{
    mSearch = new QLineEdit(this);
    mSearch->setPlaceholderText(tr("Find"));
    mSearch->setClearButtonEnabled(true);
    mSearch->setFixedHeight(NELusanCommon::inputRowHeight(*mSearch));
    mSearch->hide();
    panelLayout()->addWidget(mSearch);
    connect(mSearch, &QLineEdit::textChanged, this, [this](const QString& phrase) { applySearch(phrase); });

    setWidget(new QListWidget(this));
    QListWidget* list{ listWidget() };
    list->setFrameShape(QFrame::Shape::NoFrame);
    list->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    list->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    list->setUniformItemSizes(true);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    connect(list, &QListWidget::itemChanged, this, [this](QListWidgetItem*) {
            updateSummary();
            emit signalFiltersChanged(this);
        });

    QFrame* rule = new QFrame(this);
    rule->setFrameShape(QFrame::Shape::HLine);
    rule->setFixedHeight(1);
    panelLayout()->addWidget(rule);

    mSummary = new QLabel(this);
    mSummary->setEnabled(false);

    mBtnAll  = new QToolButton(this);
    mBtnAll->setText(tr("All"));
    mBtnNone = new QToolButton(this);
    mBtnNone->setText(tr("None"));
    for (QToolButton* button : { mBtnAll, mBtnNone })
    {
        button->setAutoRaise(true);
        button->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
    }

    mBtnAll->setToolTip(tr("Pick every value the list shows"));
    mBtnNone->setToolTip(tr("Drop the filter of this column"));
    connect(mBtnAll , &QToolButton::clicked, this, [this]() { setAllChecked(true);  });
    connect(mBtnNone, &QToolButton::clicked, this, [this]() { setAllChecked(false); });

    QHBoxLayout* foot = new QHBoxLayout();
    foot->setContentsMargins(0, 0, 0, 0);
    foot->setSpacing(_panelGap);
    foot->addWidget(mSummary);
    foot->addStretch(1);
    foot->addWidget(mBtnAll);
    foot->addWidget(mBtnNone);
    panelLayout()->addLayout(foot);

    updateSummary();
}

void LogComboFilterBase::setDataString(const QString& /*data*/)
{
}

QString LogComboFilterBase::itemLabel(const QString& name, const NELusanCommon::AnyData& /*data*/) const
{
    return name;
}

QSet<QString> LogComboFilterBase::checkedLabels() const
{
    QSet<QString> picked;
    const QListWidget* list{ listWidget() };
    for (int i = 0; i < list->count(); ++i)
    {
        const QListWidgetItem* item{ list->item(i) };
        if (item->checkState() == Qt::CheckState::Checked)
        {
            picked.insert(item->text());
        }
    }

    return picked;
}

void LogComboFilterBase::setDataList(const std::vector<NELusanCommon::FilterData>& data)
{
    QListWidget* list{ listWidget() };
    const QSignalBlocker blocker(list);

    list->clear();
    mData.clear();
    mData.reserve(static_cast<int>(data.size()));
    for (const NELusanCommon::FilterData& entry : data)
    {
        QListWidgetItem* item = new QListWidgetItem(entry.text, list);
        item->setFlags(item->flags() | Qt::ItemFlag::ItemIsUserCheckable);
        item->setCheckState(entry.active ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        mData.push_back(entry);
    }

    // A short list is read at a glance, a long one is not, so only the long one gets a find box.
    mSearch->setVisible(list->count() > _searchFrom);
    if (mSearch->isHidden())
    {
        mSearch->clear();
    }
    else
    {
        applySearch(mSearch->text());
    }

    updateSummary();
}

void LogComboFilterBase::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    const int count{ static_cast<int>(qMin(static_cast<qsizetype>(items.size()), static_cast<qsizetype>(data.size()))) };
    const QSet<QString> before{ checkedLabels() };

    std::vector<NELusanCommon::FilterData> entries;
    entries.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i)
    {
        const QString label{ itemLabel(items.at(i), data[static_cast<size_t>(i)]) };
        entries.push_back(NELusanCommon::FilterData{ label, data[static_cast<size_t>(i)], before.contains(label) });
    }

    setDataList(entries);

    // A value the reader had picked can be gone from the list now, and the column then
    // filters by something else than it did a moment ago.
    if (checkedLabels() != before)
    {
        emit signalFiltersChanged(this);
    }
}

QList<NELusanCommon::FilterData> LogComboFilterBase::getSelectedData() const
{
    QList<NELusanCommon::FilterData> checked;
    const QListWidget* list{ listWidget() };
    const int count{ qMin(list->count(), static_cast<int>(mData.size())) };
    for (int i = 0; i < count; ++i)
    {
        if (list->item(i)->checkState() == Qt::CheckState::Checked)
        {
            const NELusanCommon::FilterData& data{ mData[i] };
            checked.push_back(NELusanCommon::FilterData{ data.text, data.data, true });
        }
    }

    return checked;
}

bool LogComboFilterBase::pickValue(const NELusanCommon::AnyData& value, bool exclude)
{
    if (value.has_value() == false)
        return false;

    // The list carries an identifier for a source or a thread, and a priority bit for the
    // priorities. Anything else is not a value a row can be matched by.
    const auto same = [&value](const NELusanCommon::AnyData& entry) -> bool
        {
            if ((entry.has_value() == false) || (entry.type() != value.type()))
                return false;
            else if (entry.type() == typeid(ITEM_ID))
                return std::any_cast<ITEM_ID>(entry) == std::any_cast<ITEM_ID>(value);
            else if (entry.type() == typeid(uint16_t))
                return std::any_cast<uint16_t>(entry) == std::any_cast<uint16_t>(value);

            return false;
        };

    QListWidget* list{ listWidget() };
    const int count{ qMin(list->count(), static_cast<int>(mData.size())) };
    int found{ -1 };
    for (int i = 0; (found < 0) && (i < count); ++i)
    {
        if (same(mData[i].data))
        {
            found = i;
        }
    }

    if (found < 0)
        return false;

    {
        const QSignalBlocker blocker(list);
        for (int i = 0; i < count; ++i)
        {
            list->item(i)->setCheckState(((i == found) != exclude) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        }
    }

    updateSummary();
    emit signalFiltersChanged(this);
    return true;
}

void LogComboFilterBase::clearFilter()
{
    QListWidget* list{ listWidget() };
    bool changed{ false };

    {
        // The entries are unpicked behind blocked signals, so the listeners re-filter once
        // instead of once per entry.
        const QSignalBlocker blocker(list);
        for (int i = 0; i < list->count(); ++i)
        {
            QListWidgetItem* item{ list->item(i) };
            if (item->checkState() != Qt::CheckState::Unchecked)
            {
                item->setCheckState(Qt::CheckState::Unchecked);
                changed = true;
            }
        }
    }

    updateSummary();
    LogFilterBase::clearFilter();
    if (changed)
    {
        emit signalFiltersChanged(this);
    }
}

void LogComboFilterBase::setAllChecked(bool checked)
{
    QListWidget* list{ listWidget() };
    const Qt::CheckState state{ checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked };
    bool changed{ false };

    {
        const QSignalBlocker blocker(list);
        for (int i = 0; i < list->count(); ++i)
        {
            QListWidgetItem* item{ list->item(i) };
            // The find box leaves part of the list out of sight, and the shortcut acts on
            // what the reader can see.
            if (item->isHidden() || (item->checkState() == state))
                continue;

            item->setCheckState(state);
            changed = true;
        }
    }

    if (changed)
    {
        updateSummary();
        emit signalFiltersChanged(this);
    }
}

void LogComboFilterBase::applySearch(const QString& phrase)
{
    QListWidget* list{ listWidget() };
    const bool showAll{ phrase.isEmpty() };
    for (int i = 0; i < list->count(); ++i)
    {
        QListWidgetItem* item{ list->item(i) };
        item->setHidden(showAll == false && item->text().contains(phrase, Qt::CaseSensitivity::CaseInsensitive) == false);
    }
}

void LogComboFilterBase::updateSummary()
{
    if (mSummary == nullptr)
        return;

    const QListWidget* list{ listWidget() };
    const int total{ list->count() };
    int picked{ 0 };
    for (int i = 0; i < total; ++i)
    {
        picked += (list->item(i)->checkState() == Qt::CheckState::Checked) ? 1 : 0;
    }

    mSummary->setText(picked == 0 ? tr("Every value shown") : tr("%1 of %2 picked").arg(picked).arg(total));
    if (mBtnNone != nullptr)
    {
        mBtnNone->setEnabled(picked != 0);
    }
}

QSize LogComboFilterBase::popupSize(int anchorWidth) const
{
    const QListWidget* list{ listWidget() };
    const QFontMetrics metrics{ list->fontMetrics() };
    const int rows{ list->count() };
    const int rowHeight{ rows > 0 ? list->sizeHintForRow(0) : qMax(metrics.height() + 8, 20) };

    int widest{ 0 };
    const int measured{ qMin(rows, _measureUpTo) };
    for (int i = 0; i < measured; ++i)
    {
        widest = qMax(widest, metrics.horizontalAdvance(list->item(i)->text()));
    }

    const int chrome{ (_panelPad * 2) + 2 };
    const int width{ qBound(_panelMinWidth, qMax(anchorWidth, widest + _listRowLead + chrome), _panelMaxWidth) };

    int height{ chrome + qBound(rowHeight, (rows * rowHeight) + 4, _listMaxHeight) };
    height += _panelGap + 1;
    height += _panelGap + qMax(mSummary->sizeHint().height(), mBtnAll->sizeHint().height());
    if (mSearch->isHidden() == false)
    {
        height += _panelGap + mSearch->minimumHeight();
    }

    return QSize(width, height);
}

QListWidget* LogComboFilterBase::listWidget() const
{
    return static_cast<QListWidget*>(mWidget);
}

//////////////////////////////////////////////////////////////////////////
// LogTextFilterBase class implementation
//////////////////////////////////////////////////////////////////////////
LogTextFilterBase::LogTextFilterBase(bool extend, QWidget* parent)
    : LogFilterBase(parent)
{
    if (extend)
    {
        const QList<SearchLineEdit::eToolButton> tools{ SearchLineEdit::eToolButton::ToolButtonMatchCase
                                                      , SearchLineEdit::eToolButton::ToolButtonMatchWord
                                                      , SearchLineEdit::eToolButton::ToolButtonWildCard};
        setWidget(new SearchLineEdit(tools, this));
        SearchLineEdit* widget{ static_cast<SearchLineEdit*>(editWidget()) };
        const std::function<void(bool)> retell = [this](bool /*checked*/) {
                SearchLineEdit* box{ static_cast<SearchLineEdit*>(editWidget()) };
                _doSignalFilterChanged(box->text(), box->isMatchCaseChecked(), box->isMatchWordChecked(), box->isWildCardChecked());
            };

        connect(widget, &SearchLineEdit::signalFilterText, this, [this](const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
                _doSignalFilterChanged(text, isCaseSensitive, isWholeWord, isWildCard);
            });
        connect(widget, &SearchLineEdit::signalSearchText, this, [this](const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard, bool /*isBackward*/) {
                _doSignalFilterChanged(text, isCaseSensitive, isWholeWord, isWildCard);
                hide();
            });
        connect(widget, &SearchLineEdit::signalButtonSearchMatchCaseClicked   , this, retell);
        connect(widget, &SearchLineEdit::signalButtonSearchMatchWordClicked   , this, retell);
        connect(widget, &SearchLineEdit::signalButtonSearchWildCardClicked    , this, retell);
    }
    else
    {
        setWidget(new QLineEdit(this));
        QLineEdit* widget{ editWidget() };
        widget->setClearButtonEnabled(true);
        widget->setFixedHeight(NELusanCommon::inputRowHeight(*widget));
        connect(widget, &QLineEdit::textChanged, this, [this](const QString& text) {
                _doSignalFilterChanged(text, false, false, false);
            });
        connect(widget, &QLineEdit::returnPressed, this, [this]() { hide(); });
    }
}

void LogTextFilterBase::setDataString(const QString& data)
{
    editWidget()->setText(data);
}

void LogTextFilterBase::setDataList(const std::vector<NELusanCommon::FilterData>& /*data*/)
{
}

void LogTextFilterBase::setDataItems(const QStringList& /*items*/, const NELusanCommon::AnyList& /*data*/)
{
}

QList<NELusanCommon::FilterData> LogTextFilterBase::getSelectedData() const
{
    return mData;
}

void LogTextFilterBase::setDataFilter(const NELusanCommon::FilterString& filter)
{
    SearchLineEdit* widget{ qobject_cast<SearchLineEdit *>(mWidget) };
    if (widget != nullptr)
    {
        // The options are set first, so the text change carries the final ones with it.
        const QSignalBlocker blocker(widget);
        if (widget->buttonMatchCase() != nullptr)
            widget->buttonMatchCase()->setChecked(filter.isCaseSensitive);
        if (widget->buttonMatchWord() != nullptr)
            widget->buttonMatchWord()->setChecked(filter.isWholeWord);
        if (widget->buttonWildCard() != nullptr)
            widget->buttonWildCard()->setChecked(filter.isWildCard);
    }

    // Setting the same text emits nothing, so the changed options are pushed by hand.
    if (editWidget()->text() == filter.text)
        _doSignalFilterChanged(filter.text, filter.isCaseSensitive, filter.isWholeWord, filter.isWildCard);
    else
        setDataString(filter.text);
}

void LogTextFilterBase::clearFilter()
{
    SearchLineEdit* widget{ qobject_cast<SearchLineEdit *>(mWidget) };
    if (widget != nullptr)
    {
        // A dropped filter takes its match options with it. They are reset behind blocked
        // signals, and the text change below reports the whole thing once.
        const QSignalBlocker blocker(widget);
        if (widget->buttonMatchCase() != nullptr)
            widget->buttonMatchCase()->setChecked(false);
        if (widget->buttonMatchWord() != nullptr)
            widget->buttonMatchWord()->setChecked(false);
        if (widget->buttonWildCard() != nullptr)
            widget->buttonWildCard()->setChecked(false);
    }

    if (editWidget()->text().isEmpty() == false)
        editWidget()->setText(QString());
    else if (mData.isEmpty() == false)
        _doSignalFilterChanged(QString(), false, false, false);

    LogFilterBase::clearFilter();
}

void LogTextFilterBase::showFilter()
{
    QLineEdit* widget{ editWidget() };
    Q_ASSERT(widget != nullptr);

    LogFilterBase::showFilter();
    if (widget->text().isEmpty() == false)
    {
        widget->selectAll();
    }
}

QLineEdit* LogTextFilterBase::editWidget() const
{
    return static_cast<QLineEdit *>(mWidget);
}

inline void LogTextFilterBase::_doSignalFilterChanged(const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard)
{
    if (text.isEmpty())
    {
        mData.clear();
    }
    else
    {
        const NELusanCommon::FilterString data{ text, isCaseSensitive, isWholeWord, isWildCard };
        const NELusanCommon::FilterData entry{ text, std::make_any<NELusanCommon::FilterString>(data), true };
        if (mData.isEmpty())
            mData.push_back(entry);
        else
            mData[0] = entry;
    }

    emit signalFiltersChanged(this);
}

//////////////////////////////////////////////////////////////////////////
// LogPrioComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogPrioComboFilter::LogPrioComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

QList<NELusanCommon::FilterData> LogPrioComboFilter::getSelectedData() const
{
    uint16_t prio{ static_cast<uint16_t>(areg::LogPriority::PrioInvalid) };
    QString text;

    const QListWidget* list{ listWidget() };
    const int count{ qMin(list->count(), static_cast<int>(mData.size())) };
    for (int i = 0; i < count; ++i)
    {
        if (list->item(i)->checkState() != Qt::CheckState::Checked)
            continue;

        const NELusanCommon::FilterData& entry{ mData[i] };
        if (const uint16_t* value = std::any_cast<uint16_t>(&entry.data); value != nullptr)
        {
            prio |= *value;
            text += text.isEmpty() ? entry.text : QStringLiteral(" | ") + entry.text;
        }
    }

    return (prio != 0 ? QList<NELusanCommon::FilterData>{ NELusanCommon::FilterData{ text, std::make_any<uint16_t>(prio), true } }
                      : QList<NELusanCommon::FilterData>());
}

//////////////////////////////////////////////////////////////////////////
// LogSourceComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogSourceComboFilter::LogSourceComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

QString LogSourceComboFilter::itemLabel(const QString& name, const NELusanCommon::AnyData& data) const
{
    const ITEM_ID* id{ std::any_cast<ITEM_ID>(&data) };
    return (id != nullptr ? QStringLiteral("%1 (%2)").arg(name).arg(static_cast<qulonglong>(*id)) : name);
}

//////////////////////////////////////////////////////////////////////////
// LogSourceIdComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogSourceIdComboFilter::LogSourceIdComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

QString LogSourceIdComboFilter::itemLabel(const QString& name, const NELusanCommon::AnyData& data) const
{
    const ITEM_ID* id{ std::any_cast<ITEM_ID>(&data) };
    return (id != nullptr ? QString::number(static_cast<qulonglong>(*id)) : name);
}

//////////////////////////////////////////////////////////////////////////
// LogThreadComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogThreadComboFilter::LogThreadComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

QString LogThreadComboFilter::itemLabel(const QString& name, const NELusanCommon::AnyData& data) const
{
    const ITEM_ID* id{ std::any_cast<ITEM_ID>(&data) };
    return (id != nullptr ? QStringLiteral("%1 (%2)").arg(name).arg(static_cast<qulonglong>(*id)) : name);
}

//////////////////////////////////////////////////////////////////////////
// LogThreadIdComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogThreadIdComboFilter::LogThreadIdComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

QString LogThreadIdComboFilter::itemLabel(const QString& name, const NELusanCommon::AnyData& data) const
{
    const ITEM_ID* id{ std::any_cast<ITEM_ID>(&data) };
    return (id != nullptr ? QString::number(static_cast<qulonglong>(*id)) : name);
}

//////////////////////////////////////////////////////////////////////////
// LogDurationEditFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogDurationEditFilter::LogDurationEditFilter(QWidget* parent)
    : LogTextFilterBase(false, parent)
{
    editWidget()->setPlaceholderText(tr("At least, µs"));
}

QList<NELusanCommon::FilterData> LogDurationEditFilter::getSelectedData() const
{
    QList<NELusanCommon::FilterData> checked;
    const QString text{ editWidget()->text() };
    if (text.isEmpty() == false)
    {
        checked.push_back(NELusanCommon::FilterData{ text, std::make_any<uint32_t>(text.toUInt()), true });
    }

    return checked;
}

//////////////////////////////////////////////////////////////////////////
// LogMessageEditFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogMessageEditFilter::LogMessageEditFilter(QWidget* parent)
    : LogTextFilterBase(true, parent)
{
    editWidget()->setPlaceholderText(tr("Keep the rows carrying"));
}

QList<NELusanCommon::FilterData> LogMessageEditFilter::getSelectedData() const
{
    QList<NELusanCommon::FilterData> checked;
    const SearchLineEdit* widget{ static_cast<const SearchLineEdit *>(editWidget()) };
    const QString text{ widget->text() };
    if (text.isEmpty() == false)
    {
        const NELusanCommon::FilterString filter{ text, widget->isMatchCaseChecked(), widget->isMatchWordChecked(), widget->isWildCardChecked() };
        checked.push_back(NELusanCommon::FilterData{ text, std::make_any<NELusanCommon::FilterString>(filter), true });
    }

    return checked;
}
