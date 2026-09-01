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
 *  \file        lusan/view/log/LogHeaderItem.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/
#include "lusan/view/log/LogHeaderItem.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/log/LogFilterWidgets.hpp"

/////////////////////////////////////////////////////////////
// LogHeaderItem class implementation
/////////////////////////////////////////////////////////////

LogHeaderItem::LogHeaderItem(LogTableHeader& header, int index)
    : QObject(&header)
    , mColumn(static_cast<LoggingModelBase::eColumn>(index))
    , mType (None)
    , mActive(false)
    , mHeader(header)
    , mWidget(nullptr)
{
    switch (mColumn)
    {
    case LoggingModelBase::eColumn::LogColumnPriority:
        mType   = eType::Combo;
        mWidget = new LogPrioComboFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnSource:
        mType   = eType::Combo;
        mWidget = new LogSourceComboFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnSourceId:
        mType   = eType::Combo;
        mWidget = new LogSourceIdComboFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnThread:
        mType   = eType::Combo;
        mWidget = new LogThreadComboFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnThreadId:
        mType   = eType::Combo;
        mWidget = new LogThreadIdComboFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnTimeDuration:
        mType   = eType::Text;
        mWidget = new LogDurationEditFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnMessage:
        mType   = eType::Text;
        mWidget = new LogMessageEditFilter(&mHeader);
        break;

    case LoggingModelBase::eColumn::LogColumnScopeId:
    case LoggingModelBase::eColumn::LogColumnTimestamp:
    case LoggingModelBase::eColumn::LogColumnTimeReceived:
    default:
        break;
    }

    if (mType == eType::Combo)
    {
        connect(mWidget, &LogFilterBase::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
                const QList<NELusanCommon::FilterData> picked{ widget->getSelectedData() };
                markFiltered(picked.isEmpty() == false);
                emit mHeader.signalComboFilterChanged(fromColumnToIndex(), picked);
            });
    }
    else if (mType == eType::Text)
    {
        connect(mWidget, &LogFilterBase::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
                const QList<NELusanCommon::FilterData>& data{ widget->getData() };
                const NELusanCommon::FilterString* phrase{ data.isEmpty() ? nullptr
                                                         : std::any_cast<NELusanCommon::FilterString>(&data[0].data) };
                if ((phrase == nullptr) || phrase->text.isEmpty())
                {
                    markFiltered(false);
                    emit mHeader.signalTextFilterChanged(fromColumnToIndex(), QString(), false, false, false);
                }
                else
                {
                    markFiltered(true);
                    emit mHeader.signalTextFilterChanged(fromColumnToIndex(), phrase->text, phrase->isCaseSensitive, phrase->isWholeWord, phrase->isWildCard);
                }
            });
    }
}

inline int LogHeaderItem::fromColumnToIndex() const
{
    return mHeader.mModel->fromColumnToIndex(mColumn);
}

inline LoggingModelBase::eColumn LogHeaderItem::fromIndexToColumn(int logicalIndex) const
{
    return mHeader.mModel->fromIndexToColumn(logicalIndex);
}

void LogHeaderItem::markFiltered(bool active)
{
    if (mActive != active)
    {
        mActive = active;
        mHeader.refreshColumn(mColumn);
    }
}

void LogHeaderItem::showFilters()
{
    if ((mType == eType::None) || (mWidget == nullptr))
        return;

    const int index{ fromColumnToIndex() };
    if (index < 0)
        return;

    const QPoint corner{ mHeader.mapToGlobal(QPoint(mHeader.sectionViewportPosition(index), 0)) };
    mWidget->showFilterAt(QRect(corner, QSize(mHeader.sectionSize(index), mHeader.height())));
}

void LogHeaderItem::showFiltersAt(const QRect& anchor)
{
    if ((mType == eType::None) || (mWidget == nullptr))
        return;

    mWidget->showFilterAt(anchor);
}

NELusanCommon::AnyData LogHeaderItem::valueOf(const areg::LogEntry& entry) const
{
    switch (mColumn)
    {
    case LoggingModelBase::eColumn::LogColumnPriority:
        return std::make_any<uint16_t>(static_cast<uint16_t>(entry.logMessagePrio));

    case LoggingModelBase::eColumn::LogColumnSource:
    case LoggingModelBase::eColumn::LogColumnSourceId:
        return std::make_any<ITEM_ID>(entry.logCookie);

    case LoggingModelBase::eColumn::LogColumnThread:
    case LoggingModelBase::eColumn::LogColumnThreadId:
        return std::make_any<ITEM_ID>(entry.logThreadId);

    default:
        return NELusanCommon::AnyData();
    }
}

bool LogHeaderItem::pickValue(const areg::LogEntry& entry, bool exclude)
{
    if (mWidget == nullptr)
        return false;

    if (mType == eType::Combo)
    {
        LogComboFilterBase* combo{ static_cast<LogComboFilterBase*>(mWidget) };
        return combo->pickValue(valueOf(entry), exclude);
    }

    // A phrase keeps what it matches. There is nothing to write into it that would keep
    // everything else, so the reader is offered no exclude on such a column.
    if (exclude || (mType != eType::Text))
        return false;

    QString text;
    if (mColumn == LoggingModelBase::eColumn::LogColumnMessage)
    {
        text = QString::fromUtf8(entry.logMessage);
    }
    else if ((mColumn == LoggingModelBase::eColumn::LogColumnTimeDuration) && (entry.logDuration != 0))
    {
        // The panel of this column keeps the rows that last at least the given time, in
        // the microseconds the framework measures in.
        text = QString::number(entry.logDuration);
    }

    if (text.isEmpty())
        return false;

    mWidget->setDataFilter(NELusanCommon::FilterString{ text, true, false, false });
    return true;
}

void LogHeaderItem::setFilterData(const QString& data)
{
    if ((mType == eType::Text) && (mWidget != nullptr))
    {
        mWidget->setDataString(data);
    }
}

void LogHeaderItem::setFilterData(const NELusanCommon::FilterString& filter)
{
    if ((mType == eType::Text) && (mWidget != nullptr))
    {
        mWidget->setDataFilter(filter);
    }
}

void LogHeaderItem::setFilterData(const std::vector<QString>& data, const NELusanCommon::AnyList& list)
{
    if ((mType == eType::Combo) && (mWidget != nullptr))
    {
        QStringList items;
        items.reserve(static_cast<int>(data.size()));
        for (const QString& entry : data)
        {
            items << entry;
        }

        mWidget->setDataItems(items, list);
    }
}

void LogHeaderItem::setFilterData(const std::vector<areg::String>& data, const NELusanCommon::AnyList& list)
{
    if ((mType == eType::Combo) && (mWidget != nullptr))
    {
        QStringList items;
        items.reserve(static_cast<int>(data.size()));
        for (const areg::String& entry : data)
        {
            items << QString::fromStdString(entry.data());
        }

        mWidget->setDataItems(items, list);
    }
}

void LogHeaderItem::resetFilter()
{
    if (mWidget != nullptr)
    {
        mWidget->clearFilter();
    }
}

QList<NELusanCommon::FilterData> LogHeaderItem::getFilterData() const
{
    return (mWidget != nullptr ? mWidget->getSelectedData() : QList<NELusanCommon::FilterData>());
}
