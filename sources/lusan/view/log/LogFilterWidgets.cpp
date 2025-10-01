/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/log/LogFilterWidgets.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header filter widgets.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/log/LogFilterWidgets.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>

//////////////////////////////////////////////////////////////////////////
// LogFilterBase class implementation
//////////////////////////////////////////////////////////////////////////
LogFilterBase::LogFilterBase(QWidget* parent)
    : QFrame(parent)
    , mWidget(nullptr)
{
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::Box);
    setFocusPolicy(Qt::NoFocus);
}

void LogFilterBase::setWidget(QWidget* widget)
{
    Q_ASSERT(mWidget == nullptr);
    Q_ASSERT(widget != nullptr);

    mWidget = widget;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mWidget);
}

void LogFilterBase::clearFilter(void)
{
    if (mWidget != nullptr)
    {
        mWidget->setVisible(false);
        mWidget->clearFocus();
    }
}

void LogFilterBase::showFilter(void)
{
    mWidget->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
    mWidget->activateWindow();
    show();
}

//////////////////////////////////////////////////////////////////////////
// LogComboFilterBase class implementation
//////////////////////////////////////////////////////////////////////////
LogComboFilterBase::LogComboFilterBase(QWidget* parent)
    : LogFilterBase(parent)
{
    setWidget(new QListWidget(this));
    QListWidget* widget = listWidget();
    widget->setSelectionMode(QAbstractItemView::NoSelection);
    widget->setFocusPolicy(Qt::NoFocus);

    connect(widget, &QListWidget::itemChanged, this, [this](QListWidgetItem* /*item*/) {
        emit signalFiltersChanged(this);
        });
}

void LogComboFilterBase::setDataString(const QString& data)
{
}

void LogComboFilterBase::setDataList(const std::vector<NELusanCommon::FilterData>& data)
{
    QListWidget* widget = listWidget();
    widget->blockSignals(true);
    widget->clear();
    mData.clear();
    for (const auto& entry : data)
    {
        QListWidgetItem* item = new QListWidgetItem(entry.text, widget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(entry.active ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        widget->addItem(item);
        mData.push_back(entry);
    }

    widget->blockSignals(false);
}

QList<NELusanCommon::FilterData> LogComboFilterBase::getSelectedData() const
{
    QList<NELusanCommon::FilterData> checked;
    QListWidget* widget = listWidget();
    int count = widget->count();
    for (int i = 0; i < count; ++i)
    {
        QListWidgetItem* item = widget->item(i);
        if (item->checkState() == Qt::Checked)
        {
            const NELusanCommon::FilterData & data = mData[i];
            checked.push_back(NELusanCommon::FilterData{ data.text, data.data, true });
        }
    }

    return checked;
}

void LogComboFilterBase::clearFilter(void)
{
    QListWidget* widget = listWidget();
    Q_ASSERT(widget != nullptr);
    for (int i = 0; i < widget->count(); ++i)
    {
        widget->item(i)->setCheckState(Qt::CheckState::Unchecked);
    }

    LogFilterBase::clearFilter();
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
        QList<SearchLineEdit::eToolButton> tools{ SearchLineEdit::eToolButton::ToolButtonMatchCase
                                                , SearchLineEdit::eToolButton::ToolButtonMatchWord
                                                , SearchLineEdit::eToolButton::ToolButtonWildCard};
        setWidget(new SearchLineEdit(tools, QSize(20, 20), this));
        SearchLineEdit* widget = static_cast<SearchLineEdit*>(editWidget());
        std::function<void(bool)> func = [this](bool checked) {
            SearchLineEdit* w = static_cast<SearchLineEdit*>(editWidget());
            _doSignalFilterChanged(w->text(), w->isMatchCaseChecked(), w->isMatchWordChecked(), w->isWildCardChecked());
            };

        connect(widget, &SearchLineEdit::signalFilterText, this, [this](const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
            _doSignalFilterChanged(text, isCaseSensitive, isWholeWord, isWildCard);
            });
        connect(widget, &SearchLineEdit::signalButtonSearchMatchCaseClicked   , this, func);
        connect(widget, &SearchLineEdit::signalButtonSearchMatchWordClicked   , this, func);
        connect(widget, &SearchLineEdit::signalButtonSearchWildCardClicked    , this, func);
    }
    else
    {
        setWidget(new QLineEdit(this));
        QLineEdit* widget = editWidget();
        connect(widget, &QLineEdit::textChanged, this, [this](const QString& text) {
            _doSignalFilterChanged(text, false, false, false);
            });
    }
}

void LogTextFilterBase::setDataString(const QString& data)
{
    editWidget()->setText(data);
}

void LogTextFilterBase::setDataList(const std::vector<NELusanCommon::FilterData>& data)
{
}

inline void LogTextFilterBase::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
}

QList<NELusanCommon::FilterData> LogTextFilterBase::getSelectedData() const
{
    return mData;
}

void LogTextFilterBase::clearFilter(void)
{
    editWidget()->setText(QString());
    LogFilterBase::clearFilter();
}

void LogTextFilterBase::showFilter(void)
{
    QLineEdit* widget = editWidget();
    Q_ASSERT(widget != nullptr);

    if (widget->text().isEmpty() == false)
        widget->selectAll();

    LogFilterBase::showFilter();
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
        NELusanCommon::FilterString data{ text, isCaseSensitive, isWholeWord, isWildCard };
        if (mData.empty())
        {
            mData.push_back(NELusanCommon::FilterData{ text, std::make_any< NELusanCommon::FilterString >(data), true });
        }
        else
        {
            mData[0] = NELusanCommon::FilterData{ text, std::make_any< NELusanCommon::FilterString >(data), true };
        }
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

void LogPrioComboFilter::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    QListWidget* widget = listWidget();
    std::vector<NELusanCommon::FilterData> filter;
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString& text = items[i];
        uint16_t prio = std::any_cast<uint16_t>(data[i]);
        QList<QListWidgetItem*> f = widget->findItems(text, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        bool active = (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked);
        filter.push_back(NELusanCommon::FilterData{ text, data[i], active });
    }

    setDataList(filter);
}

QList<NELusanCommon::FilterData> LogPrioComboFilter::getSelectedData() const
{
    uint16_t prio = static_cast<uint16_t>(NELogging::eLogPriority::PrioInvalid);
    QString text{};

    QListWidget* widget = listWidget();
    int count = widget->count();
    for (int i = 0; i < count; ++i)
    {
        QListWidgetItem* item = widget->item(i);
        if (item->checkState() == Qt::Checked)
        {
            const NELusanCommon::FilterData f = mData[i];
            prio |= std::any_cast<uint16_t>(f.data);
            text += text.isEmpty() ? f.text : " | " + f.text;
        }
    }

    return (prio != 0 ? QList<NELusanCommon::FilterData>{NELusanCommon::FilterData{ text, std::make_any<uint16_t>(prio), true }} : QList<NELusanCommon::FilterData>());
}

//////////////////////////////////////////////////////////////////////////
// LogSourceComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogSourceComboFilter::LogSourceComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

void LogSourceComboFilter::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    QListWidget* widget = listWidget();
    std::vector<NELusanCommon::FilterData> filter;
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString& text = items[i];
        ITEM_ID srcId = std::any_cast<ITEM_ID>(data[i]);
        QTextStream view;
        QString txt;
        view << text << " (" << srcId << ")" >> txt;
        QList<QListWidgetItem*> f = widget->findItems(txt, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        bool active = (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked);
        filter.push_back(NELusanCommon::FilterData{ text, data[i], active });
    }

    setDataList(filter);
}

//////////////////////////////////////////////////////////////////////////
// LogSourceIdComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogSourceIdComboFilter::LogSourceIdComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

void LogSourceIdComboFilter::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    QListWidget* widget = listWidget();
    std::vector<NELusanCommon::FilterData> filter;
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString& text = items[i];
        ITEM_ID srcId = std::any_cast<ITEM_ID>(data[i]);
        QTextStream view;
        QString txt;
        view << srcId << " (" << text << ")" >> txt;

        QList<QListWidgetItem*> f = widget->findItems(txt, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        bool active = (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked);
        filter.push_back(NELusanCommon::FilterData{ text, data[i], active });
    }

    setDataList(filter);
}

//////////////////////////////////////////////////////////////////////////
// LogThreadComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogThreadComboFilter::LogThreadComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

void LogThreadComboFilter::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    QListWidget* widget = listWidget();
    std::vector<NELusanCommon::FilterData> filter;
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString& text = items[i];
        ITEM_ID trdId = std::any_cast<ITEM_ID>(data[i]);
        QTextStream view;
        QString txt;
        view << text << " (" << trdId << ")" >> txt;

        QList<QListWidgetItem*> f = widget->findItems(txt, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        bool active = (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked);
        filter.push_back(NELusanCommon::FilterData{ text, data[i], active });
    }

    setDataList(filter);
}

//////////////////////////////////////////////////////////////////////////
// LogThreadIdComboFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogThreadIdComboFilter::LogThreadIdComboFilter(QWidget* parent)
    : LogComboFilterBase(parent)
{
}

void LogThreadIdComboFilter::setDataItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    QListWidget* widget = listWidget();
    std::vector<NELusanCommon::FilterData> filter;
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString& text = items[i];
        ITEM_ID trdId = std::any_cast<ITEM_ID>(data[i]);
        QTextStream view;
        QString txt;
        view << trdId << " (" << text << ")" >> txt;

        QList<QListWidgetItem*> f = widget->findItems(txt, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        bool active = (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked);
        filter.push_back(NELusanCommon::FilterData{ text, data[i], active });
    }

    setDataList(filter);
}

//////////////////////////////////////////////////////////////////////////
// LogDurationEditFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogDurationEditFilter::LogDurationEditFilter(QWidget* parent)
    : LogTextFilterBase(false, parent)
{
}

QList<NELusanCommon::FilterData> LogDurationEditFilter::getSelectedData() const
{
    QList<NELusanCommon::FilterData> checked;
    QLineEdit* widget = editWidget();
    QString text = widget->text();
    if (text.isEmpty())
    {
        uint32_t duration = text.toUInt();
        checked.push_back(NELusanCommon::FilterData{ text, duration, true});
    }

    return checked;
}

//////////////////////////////////////////////////////////////////////////
// LogMessageEditFilter class implementation
//////////////////////////////////////////////////////////////////////////
LogMessageEditFilter::LogMessageEditFilter(QWidget* parent)
    : LogTextFilterBase(true, parent)
{
}

QList<NELusanCommon::FilterData> LogMessageEditFilter::getSelectedData() const
{
    QList<NELusanCommon::FilterData> checked;
    SearchLineEdit* widget = static_cast<SearchLineEdit *>(editWidget());
    QString text = widget->text();
    if (text.isEmpty())
    {
        NELusanCommon::FilterString f{ text, widget->isMatchCaseChecked(), widget->isMatchWordChecked(), widget->isWildCardChecked()};
        checked.push_back(NELusanCommon::FilterData{ text, f, true });
    }

    return checked;
}
