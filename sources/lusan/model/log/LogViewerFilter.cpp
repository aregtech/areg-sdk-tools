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
 *  \file        lusan/model/log/LogViewerFilter.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Filter Proxy Model.
 *
 ************************************************************************/
#include "lusan/model/log/LogViewerFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include <QModelIndex>

LogViewerFilter::LogViewerFilter(LoggingModelBase* model)
    : QSortFilterProxyModel (model)
    , mComboFilters         ( )
    , mTextFilters          ( )
    , mRePattern            ( )
    , mReExpression         ( )
{
    setSourceModel(model);
}

LogViewerFilter::~LogViewerFilter(void)
{
    setSourceModel(nullptr);
    _clearData();
}

void LogViewerFilter::setComboFilter(int logicalColumn, const NELusanCommon::FilterList& filters)
{
    if (filters.isEmpty())
    {
        if (mComboFilters.contains(logicalColumn))
        {
            mComboFilters.remove(logicalColumn);
            invalidateFilter();
        }
    }
    else
    {
        mComboFilters[logicalColumn] = filters;
        invalidateFilter();
    }
}

void LogViewerFilter::setTextFilter(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard)
{
    setTextFilter(logicalColumn, NELusanCommon::FilterString{ text, isCaseSensitive, isWholeWord, isWildCard });
}

void LogViewerFilter::setTextFilter(int logicalColumn, const NELusanCommon::FilterString& filter)
{
    if (filter.text.isEmpty())
    {
        if (mTextFilters.contains(logicalColumn))
        {
            mTextFilters.remove(logicalColumn);
            LoggingModelBase* model = static_cast<LoggingModelBase*>(sourceModel());
            if ((model != nullptr) && (model->fromIndexToColumn(logicalColumn) == LoggingModelBase::eColumn::LogColumnMessage))
            {
                // If the filter is removed, we need to invalidate the filter
                // to ensure that the model updates correctly.
                prepareReExpression(filter.text, false, false, false);
            }

            invalidateFilter();
        }
    }
    else
    {
        LoggingModelBase* model = static_cast<LoggingModelBase*>(sourceModel());
        LoggingModelBase::eColumn ecol = model != nullptr ? model->fromIndexToColumn(logicalColumn) : LoggingModelBase::eColumn::LogColumnInvalid;
        switch (ecol)
        {
        case LoggingModelBase::eColumn::LogColumnInvalid:
            mTextFilters.clear();
            break;
            
        case LoggingModelBase::eColumn::LogColumnTimeDuration:
        {   
            uint32_t duration = filter.text.toUInt();
            mTextFilters[logicalColumn] = NELusanCommon::FilterList{ NELusanCommon::FilterData{filter.text, std::make_any<uint32_t>(duration), true} };
        }
        break;
        
        case LoggingModelBase::eColumn::LogColumnMessage:
            mTextFilters[logicalColumn] = NELusanCommon::FilterList{ NELusanCommon::FilterData{filter.text, std::make_any<NELusanCommon::FilterString>(filter), true} };
            // If the filter is set, prepare regex
            // to ensure that the model updates correctly.
            prepareReExpression(filter.text, filter.isCaseSensitive, filter.isWholeWord, filter.isWildCard);
            break;
        
        default:
            mTextFilters[logicalColumn] = NELusanCommon::FilterList{ NELusanCommon::FilterData{filter.text, std::make_any<NELusanCommon::FilterString>(filter), true} };
            break;
        }

        invalidateFilter();
    }
}


void LogViewerFilter::clearFilters()
{
    _clearData();
    invalidateFilter();
}

bool LogViewerFilter::filterExactMatch(const QModelIndex& index) const
{
    LoggingModelBase* model = static_cast<LoggingModelBase*>(sourceModel());
    if (index.isValid() == false)
        return false;
    else if (model == nullptr)
        return true;

    const NELogging::sLogMessage* msg = model->getLogData(index.row());
    NELusanCommon::eMatchType comboMatch = matchesComboFilters(model, msg);
    if (comboMatch != NELusanCommon::eMatchType::NoMatch)
    {
        NELusanCommon::eMatchType textMatch = matchesTextFilters(model, msg);
        if (textMatch != NELusanCommon::eMatchType::NoMatch)
            return (comboMatch == NELusanCommon::eMatchType::ExactMatch) || (textMatch == NELusanCommon::eMatchType::ExactMatch);
    }

    return false;
}

bool LogViewerFilter::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    LoggingModelBase* model = static_cast<LoggingModelBase*>(sourceModel());
    QModelIndex index = sourceModel() != nullptr ? sourceModel()->index(row, 0, parent) : QModelIndex();
    if (index.isValid() == false)
        return false;
    else if (model == nullptr)
        return true;

    const NELogging::sLogMessage* msg = model->getLogData(index.row());
    // Check if row matches all active filters
    return  (matchesComboFilters(model, msg) != NELusanCommon::eMatchType::NoMatch) &&
            (matchesTextFilters(model, msg)  != NELusanCommon::eMatchType::NoMatch);
}

NELusanCommon::eMatchType LogViewerFilter::matchesComboFilters(LoggingModelBase* model, const NELogging::sLogMessage* msg) const
{
    NELusanCommon::eMatchType matchType = NELusanCommon::eMatchType::PartialMatch;
    // Check each active combo filter
    for (auto it = mComboFilters.constBegin(); (matchType != NELusanCommon::eMatchType::NoMatch) && (it != mComboFilters.constEnd()); ++it)
    {
        const NELusanCommon::FilterList& filters = it.value();
        if (filters.isEmpty())
            continue;
        
        LoggingModelBase::eColumn ecol = model->fromIndexToColumn(static_cast<int>(it.key()));
        switch (ecol)
        {
        case LoggingModelBase::eColumn::LogColumnPriority:
            matchType = matchPrio(msg, filters) ? NELusanCommon::eMatchType::ExactMatch : NELusanCommon::eMatchType::NoMatch;
            break;

        case LoggingModelBase::eColumn::LogColumnSource:
        case LoggingModelBase::eColumn::LogColumnSourceId:
            matchType = matchSources(msg, filters) ? NELusanCommon::eMatchType::ExactMatch : NELusanCommon::eMatchType::NoMatch;
            break;

        case LoggingModelBase::eColumn::LogColumnThreadId:
        case LoggingModelBase::eColumn::LogColumnThread:
            matchType = matchThreads(msg, filters) ? NELusanCommon::eMatchType::ExactMatch : NELusanCommon::eMatchType::NoMatch;
            break;

        default:
            break;
        }
    }
    
    return matchType;
}

NELusanCommon::eMatchType LogViewerFilter::matchesTextFilters(LoggingModelBase* model, const NELogging::sLogMessage* msg) const
{
    NELusanCommon::eMatchType matchType = NELusanCommon::eMatchType::PartialMatch;
    // Check each active text filter
    for (auto it = mTextFilters.constBegin(); (matchType != NELusanCommon::eMatchType::NoMatch) && (it != mTextFilters.constEnd()); ++it)
    {
        const NELusanCommon::FilterList& filters = it.value();
        if (filters.isEmpty())
            continue;

        LoggingModelBase::eColumn ecol = model->fromIndexToColumn(static_cast<int>(it.key()));
        switch (ecol)
        {
        case LoggingModelBase::eColumn::LogColumnTimeDuration:
            matchType = matchDuration(msg, filters) ? NELusanCommon::eMatchType::ExactMatch : NELusanCommon::eMatchType::NoMatch;
            break;

        case LoggingModelBase::eColumn::LogColumnMessage:
            matchType = matchMessage(msg, filters) ? NELusanCommon::eMatchType::ExactMatch : NELusanCommon::eMatchType::NoMatch;
            break;

        default:
            break;
        }
    }

    return matchType;
}

bool LogViewerFilter::wildcardMatch(const QString& text, const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord) const
{
    // Escape regex special characters except * and ?
    QString regexPattern = QRegularExpression::escape(wildcardPattern);
    regexPattern.replace("\\*", ".*");
    regexPattern.replace("\\?", ".");

    // For whole word, use word boundaries, but treat '_' as a word boundary as well
    if (isWholeWord)
    {
        // Custom boundaries: start of string or non-word char (including '_'), and end of string or non-word char (including '_')
        // \b does not treat '_' as a boundary, so we use lookarounds
        regexPattern = QStringLiteral("(?:(?<=^)|(?<=[^\\w]|_))") + regexPattern + QStringLiteral("(?:(?=$)|(?=[^\\w]|_))");
    }

    QRegularExpression::PatternOptions options = isCaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
    QRegularExpression re(regexPattern, options);
    return text.contains(re);
}

inline bool LogViewerFilter::matchPrio(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const
{
    return (std::any_cast<uint16_t>(filters[0].data) & static_cast<uint16_t>(msg->logMessagePrio)) != 0;
}

inline bool LogViewerFilter::matchSources(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const
{
    for (const auto& f : filters)
    {
        if (std::any_cast<ITEM_ID>(f.data) == msg->logCookie)
        {
            return true;
        }
    }

    return false;
}

inline bool LogViewerFilter::matchThreads(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const
{
    for (const auto& f : filters)
    {
        if (std::any_cast<ITEM_ID>(f.data) == msg->logThreadId)
        {
            return true;
        }
    }
    return false;
}

inline bool LogViewerFilter::matchDuration(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const
{
    return (msg->logDuration >= std::any_cast<uint32_t>(filters[0].data));
}

inline bool LogViewerFilter::matchMessage(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const
{
    NELusanCommon::FilterString filterText = std::any_cast<NELusanCommon::FilterString>(filters[0].data);
    // Check if the cell data contains the filter text (case-insensitive)
    if (filterText.isWildCard || filterText.isWholeWord)
    {
        // return wildcardMatch(QString::fromUtf8(msg->logMessage, msg->logMessageLen), filterText.text, filterText.isCaseSensitive, filterText.isWholeWord);
        Q_ASSERT(mRePattern.isEmpty() == false);
        return QString::fromUtf8(msg->logMessage, msg->logMessageLen).contains(mReExpression);
    }
    else
    {
        return QString::fromUtf8(msg->logMessage, msg->logMessageLen).contains(filterText.text, filterText.isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    }
}

inline void LogViewerFilter::prepareReExpression(const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord, bool isWildCard)
{
    if ((isWildCard || isWholeWord) && !wildcardPattern.isEmpty())
    {
        // Escape regex special characters except * and ?
        mRePattern = QRegularExpression::escape(wildcardPattern);
        mRePattern.replace("\\*", ".*");
        mRePattern.replace("\\?", ".");

        // For whole word, use word boundaries, but treat '_' as a word boundary as well
        if (isWholeWord)
        {
            // Custom boundaries: start of string or non-word char (including '_'), and end of string or non-word char (including '_')
            // \b does not treat '_' as a boundary, so we use lookarounds
            mRePattern = QStringLiteral("(?:(?<=^)|(?<=[^\\w]|_))") + mRePattern + QStringLiteral("(?:(?=$)|(?=[^\\w]|_))");
        }

        QRegularExpression::PatternOptions options = isCaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
        mReExpression = QRegularExpression(mRePattern, options);
    }
    else
    {
        mRePattern.clear();
        mReExpression = QRegularExpression();
    }
}

inline void LogViewerFilter::_clearData(void)
{
    mComboFilters.clear();
    mTextFilters.clear();
}
