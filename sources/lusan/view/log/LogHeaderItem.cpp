/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/log/LogHeaderItem.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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
    , mHeader(header)
    , mWidget(nullptr)
{
    switch (mColumn)
    {
    case LoggingModelBase::eColumn::LogColumnPriority:
        mType = eType::Combo;
        mWidget = new LogPrioComboFilter(&mHeader);
        connect(mWidget, &LogPrioComboFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), widget->getSelectedData());
        });
        break;

    case LoggingModelBase::eColumn::LogColumnSource:
        mType = eType::Combo;
        mWidget = new LogSourceComboFilter(&mHeader);
        connect(mWidget, &LogSourceComboFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), widget->getSelectedData());
        });
        break;

    case LoggingModelBase::eColumn::LogColumnSourceId:
        mType = eType::Combo;
        mWidget = new LogSourceIdComboFilter(&mHeader);
        connect(mWidget, &LogSourceIdComboFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), widget->getSelectedData());
        });
        break;

    case LoggingModelBase::eColumn::LogColumnThread:
        mType = eType::Combo;
        mWidget = new LogThreadComboFilter(&mHeader);
        connect(mWidget, &LogThreadComboFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), widget->getSelectedData());
        });
        break;

    case LoggingModelBase::eColumn::LogColumnThreadId:
        mType = eType::Combo;
        mWidget = new LogThreadIdComboFilter(&mHeader);
        connect(mWidget, &LogThreadIdComboFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), widget->getSelectedData());
        });
        break;

    case LoggingModelBase::eColumn::LogColumnTimeDuration:
        mType = eType::Text;
        mWidget = new LogDurationEditFilter(&mHeader);
        connect(mWidget, &LogDurationEditFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            const QList<NELusanCommon::FilterData>& data = widget->getData();
            emit mHeader.signalTextFilterChanged(fromColumnToIndex(), data.isEmpty() ? QString() : data[0].text, false, false, false);
        });
        break;

    case LoggingModelBase::eColumn::LogColumnMessage:
        mType = eType::Text;
        mWidget = new LogMessageEditFilter(&mHeader);
        connect(mWidget, &LogMessageEditFilter::signalFiltersChanged, &mHeader, [this](LogFilterBase* widget) {
            const QList<NELusanCommon::FilterData>& data = widget->getData();
            if (data.isEmpty() || data[0].text.isEmpty())
            {
                emit mHeader.signalTextFilterChanged(fromColumnToIndex(), QString(), false, false, false);
            }
            else
            {
                NELusanCommon::FilterString fs = std::any_cast<NELusanCommon::FilterString>(data[0].data);
                Q_ASSERT(fs.text.isEmpty() == false);
                emit mHeader.signalTextFilterChanged(fromColumnToIndex(), fs.text, fs.isCaseSensitive, fs.isWholeWord, fs.isWildCard);
            }
        });
        break;

    case LoggingModelBase::eColumn::LogColumnScopeId:
    case LoggingModelBase::eColumn::LogColumnTimestamp:
    case LoggingModelBase::eColumn::LogColumnTimeReceived:
    default:
        break;
    }
}

inline int LogHeaderItem::fromColumnToIndex(void) const
{
    return mHeader.mModel->fromColumnToIndex(mColumn);
}

inline LoggingModelBase::eColumn LogHeaderItem::fromIndexToColumn(int logicalIndex) const
{
    return mHeader.mModel->fromIndexToColumn(logicalIndex);
}

void LogHeaderItem::showFilters(void)
{
    if (mType == eType::None)
        return;

    Q_ASSERT(mWidget != nullptr);
    int index   = fromColumnToIndex();
    int pos     = mHeader.sectionViewportPosition(index);
    int height  = mHeader.size().height();
    QPoint pt   = mHeader.mapToGlobal(QPoint(pos, height));

    if (mType == eType::Text)
    {
        Q_ASSERT(mWidget != nullptr);
        QSize sz = mWidget->size();
        sz.setWidth(mHeader.sectionSize(index));
        mWidget->setMinimumSize(sz);
    }

    mWidget->move(pt);
    mWidget->showFilter();
}

void LogHeaderItem::setFilterData(const QString& data)
{
    if ((mType == eType::Text) && (mWidget != nullptr))
    {
        mWidget->setDataString(data);
    }
}

void LogHeaderItem::setFilterData(const std::vector<QString>& data, const NELusanCommon::AnyList& list)
{
    if ((mType == eType::Combo) && (mWidget != nullptr))
    {
        QStringList items;
        items.reserve(static_cast<int>(data.size()));
        for (const auto& entry : data)
        {
            items << entry;
        }

        mWidget->setDataItems(items, list);
    }
}

void LogHeaderItem::setFilterData(const std::vector<String>& data, const NELusanCommon::AnyList& list)
{
    if ((mType == eType::Combo) && (mWidget != nullptr))
    {
        QStringList items;
        items.reserve(static_cast<int>(data.size()));
        for (const auto& entry : data)
        {
            items << QString::fromStdString(entry.getData());
        }

        mWidget->setDataItems(items, list);
    }
}

void LogHeaderItem::setFilterData(const std::vector<ITEM_ID>& data, const NELusanCommon::AnyList& list)
{
    if ((mType == eType::Combo) && (mWidget != nullptr))
    {
        QStringList items;
        for (const auto& entry : data)
        {
            items << QString::number(static_cast<uint64_t>(entry));
        }

        mWidget->setDataItems(items, list);
    }
}

void LogHeaderItem::resetFilter(void)
{
    if (mWidget != nullptr)
        mWidget->clearFilter();
}

QList<NELusanCommon::FilterData> LogHeaderItem::getFilterData(void) const
{
    return (mWidget != nullptr ? mWidget->getSelectedData() : QList<NELusanCommon::FilterData>());
}
