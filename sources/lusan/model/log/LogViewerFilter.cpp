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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/log/LogViewerFilter.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Filter Proxy Model.
 *
 ************************************************************************/
#include "lusan/model/log/LogViewerFilter.hpp"

#include "areg/logging/LoggingDefs.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include <QModelIndex>

LogViewerFilter::LogViewerFilter(LoggingModelBase* model)
    : QSortFilterProxyModel (model)
    , mComboFilters         ( )
    , mTextFilters          ( )
    , mRePattern            ( )
    , mReExpression         ( )
    , mRevealed             ( )
    , mIsolation            ( )
    , mViewPriority         (0)
{
    setSourceModel(model);
    if (model != nullptr)
    {
        connect(model, &LoggingModelBase::signalRefusedScopesChanged, this, [this]() { invalidateRowsFilter(); });
    }
}

LogViewerFilter::~LogViewerFilter()
{
    setSourceModel(nullptr);
    _clearData();
}

void LogViewerFilter::setComboFilter(LoggingModelBase::eColumn column, const NELusanCommon::FilterList& filters)
{
    if (column == LoggingModelBase::eColumn::LogColumnInvalid)
        return;

    int columnKey = static_cast<int>(column);
    if (filters.isEmpty())
    {
        if (mComboFilters.contains(columnKey))
        {
            mComboFilters.remove(columnKey);
            invalidateRowFilter();
        }
    }
    else
    {
        mComboFilters[columnKey] = filters;
        invalidateRowFilter();
    }
}

void LogViewerFilter::setTextFilter(LoggingModelBase::eColumn column, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard)
{
    setTextFilter(column, NELusanCommon::FilterString{ text, isCaseSensitive, isWholeWord, isWildCard });
}

void LogViewerFilter::setTextFilter(LoggingModelBase::eColumn column, const NELusanCommon::FilterString& filter)
{
    if (column == LoggingModelBase::eColumn::LogColumnInvalid)
        return;

    int columnKey = static_cast<int>(column);
    if (filter.text.isEmpty())
    {
        if (mTextFilters.contains(columnKey))
        {
            mTextFilters.remove(columnKey);
            if (column == LoggingModelBase::eColumn::LogColumnMessage)
            {
                // If the filter is removed, we need to invalidate the filter
                // to ensure that the model updates correctly.
                prepareReExpression(filter.text, false, false, false);
            }

            invalidateRowFilter();
        }
    }
    else
    {
        switch (column)
        {
        case LoggingModelBase::eColumn::LogColumnTimeDuration:
        {   
            uint32_t duration = filter.text.toUInt();
            mTextFilters[columnKey] = NELusanCommon::FilterList{ NELusanCommon::FilterData{filter.text, std::make_any<uint32_t>(duration), true} };
        }
        break;
        
        case LoggingModelBase::eColumn::LogColumnMessage:
            mTextFilters[columnKey] = NELusanCommon::FilterList{ NELusanCommon::FilterData{filter.text, std::make_any<NELusanCommon::FilterString>(filter), true} };
            // If the filter is set, prepare regex
            // to ensure that the model updates correctly.
            prepareReExpression(filter.text, filter.isCaseSensitive, filter.isWholeWord, filter.isWildCard);
            break;
        
        default:
            mTextFilters[columnKey] = NELusanCommon::FilterList{ NELusanCommon::FilterData{filter.text, std::make_any<NELusanCommon::FilterString>(filter), true} };
            break;
        }

        invalidateRowFilter();
    }
}


void LogViewerFilter::clearFilters()
{
    _clearData();
    invalidateRowFilter();
}

bool LogViewerFilter::hasColumnFilters(void) const
{
    for (auto it = mComboFilters.cbegin(); it != mComboFilters.cend(); ++it)
    {
        if (it.value().isEmpty() == false)
            return true;
    }

    for (auto it = mTextFilters.cbegin(); it != mTextFilters.cend(); ++it)
    {
        if (it.value().isEmpty() == false)
            return true;
    }

    return false;
}

void LogViewerFilter::setIsolation(const LogViewerFilter::sIsolation& isolation)
{
    mIsolation = isolation;
    mRevealed.clear();
    invalidateRowFilter();
}

void LogViewerFilter::clearIsolation(void)
{
    if (mIsolation.kind == LogViewerFilter::eIsolation::IsolationNone)
        return;

    mIsolation = LogViewerFilter::sIsolation{ };
    invalidateRowFilter();
}

void LogViewerFilter::setViewPriority(uint16_t mask)
{
    if (mViewPriority == mask)
        return;

    mViewPriority = mask;
    invalidateRowFilter();
}

bool LogViewerFilter::hasWindowFilters(void) const
{
    return hasColumnFilters() || hasIsolation() || (mViewPriority != 0);
}

void LogViewerFilter::revealRow(int sourceRow)
{
    if ((sourceRow < 0) || mRevealed.contains(sourceRow))
        return;

    mRevealed.insert(sourceRow);
    invalidateRowFilter();
}

void LogViewerFilter::clearRevealedRows(void)
{
    if (mRevealed.isEmpty())
        return;

    mRevealed.clear();
    invalidateRowFilter();
}

QVariant LogViewerFilter::data(const QModelIndex& index, int role) const
{
    if (role == LogViewerFilter::RevealedRole)
    {
        const QModelIndex source{ mapToSource(index) };
        return QVariant(source.isValid() && mRevealed.contains(source.row()));
    }

    return QSortFilterProxyModel::data(index, role);
}

LogViewerFilter::ListActiveFilters LogViewerFilter::activeFilters(void) const
{
    LogViewerFilter::ListActiveFilters result;

    for (auto it = mComboFilters.cbegin(); it != mComboFilters.cend(); ++it)
    {
        const NELusanCommon::FilterList& filters{ it.value() };
        if (filters.isEmpty())
            continue;

        QStringList names;
        names.reserve(filters.size());
        for (const NELusanCommon::FilterData& entry : filters)
        {
            names.append(entry.text);
        }

        result.append(LogViewerFilter::sActiveFilter{ it.key(), false, names.join(QStringLiteral(", ")), { } });
    }

    for (auto it = mTextFilters.cbegin(); it != mTextFilters.cend(); ++it)
    {
        const NELusanCommon::FilterList& filters{ it.value() };
        if (filters.isEmpty())
            continue;

        const NELusanCommon::FilterData& entry{ filters.first() };
        NELusanCommon::FilterString phrase{ entry.text, false, false, false };
        if (entry.data.has_value() && (entry.data.type() == typeid(NELusanCommon::FilterString)))
        {
            phrase = std::any_cast<NELusanCommon::FilterString>(entry.data);
        }

        result.append(LogViewerFilter::sActiveFilter{ it.key(), true, entry.text, phrase });
    }

    return result;
}

bool LogViewerFilter::filterExactMatch(const QModelIndex& index) const
{
    LoggingModelBase* model = static_cast<LoggingModelBase*>(sourceModel());
    if (index.isValid() == false)
        return false;
    else if (model == nullptr)
        return true;

    const areg::LogEntry* msg = model->getLogData(index.row());
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

    // A row the search asked for is drawn whatever the filters say, and marked apart.
    if (mRevealed.contains(row))
        return true;

    const areg::LogEntry* msg = model->getLogData(index.row());
    // The scope tree refuses rows from the moment it was unchecked, so this is asked first:
    // it is a lookup, while the column filters walk their lists.
    if (model->isEntryRefused(msg))
        return false;

    if (matchIsolation(msg) == false)
        return false;

    if ((mViewPriority != 0) && (msg != nullptr)
        && ((mViewPriority & static_cast<uint16_t>(msg->logMessagePrio)) == 0))
        return false;

    // Check if row matches all active filters
    return  (matchesComboFilters(model, msg) != NELusanCommon::eMatchType::NoMatch) &&
            (matchesTextFilters(model, msg)  != NELusanCommon::eMatchType::NoMatch);
}

NELusanCommon::eMatchType LogViewerFilter::matchesComboFilters(LoggingModelBase* model, const areg::LogEntry* msg) const
{
    NELusanCommon::eMatchType matchType = NELusanCommon::eMatchType::PartialMatch;
    // Check each active combo filter
    for (auto it = mComboFilters.constBegin(); (matchType != NELusanCommon::eMatchType::NoMatch) && (it != mComboFilters.constEnd()); ++it)
    {
        const NELusanCommon::FilterList& filters = it.value();
        if (filters.isEmpty())
            continue;
        
        LoggingModelBase::eColumn ecol = static_cast<LoggingModelBase::eColumn>(it.key());
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

NELusanCommon::eMatchType LogViewerFilter::matchesTextFilters(LoggingModelBase* model, const areg::LogEntry* msg) const
{
    NELusanCommon::eMatchType matchType = NELusanCommon::eMatchType::PartialMatch;
    // Check each active text filter
    for (auto it = mTextFilters.constBegin(); (matchType != NELusanCommon::eMatchType::NoMatch) && (it != mTextFilters.constEnd()); ++it)
    {
        const NELusanCommon::FilterList& filters = it.value();
        if (filters.isEmpty())
            continue;

        LoggingModelBase::eColumn ecol = static_cast<LoggingModelBase::eColumn>(it.key());
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

inline bool LogViewerFilter::matchPrio(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const
{
    if (filters.isEmpty())
        return false;

    const uint16_t* prio = std::any_cast<uint16_t>(&filters[0].data);
    return (prio != nullptr) && ((*prio & static_cast<uint16_t>(msg->logMessagePrio)) != 0);
}

inline bool LogViewerFilter::matchSources(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const
{
    for (const auto& f : filters)
    {
        if (const ITEM_ID* value = std::any_cast<ITEM_ID>(&f.data); (value != nullptr) && (*value == msg->logCookie))
        {
            return true;
        }
    }

    return false;
}

inline bool LogViewerFilter::matchThreads(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const
{
    for (const auto& f : filters)
    {
        if (const ITEM_ID* value = std::any_cast<ITEM_ID>(&f.data); (value != nullptr) && (*value == msg->logThreadId))
        {
            return true;
        }
    }
    return false;
}

inline bool LogViewerFilter::matchDuration(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const
{
    if (filters.isEmpty())
        return false;

    const uint32_t* duration = std::any_cast<uint32_t>(&filters[0].data);
    return (duration != nullptr) && (msg->logDuration >= *duration);
}

inline bool LogViewerFilter::matchMessage(const areg::LogEntry* msg, const NELusanCommon::FilterList& filters) const
{
    if (filters.isEmpty())
        return false;

    const NELusanCommon::FilterString* filterText = std::any_cast<NELusanCommon::FilterString>(&filters[0].data);
    if (filterText == nullptr)
        return false;

    // logMessageLen is the length before the message was cut and can exceed the buffer,
    // so the stored length is what may be read.
    const QString message{ QString::fromUtf8(msg->logMessage, static_cast<int>(areg::log_message_size(*msg))) };

    // Check if the cell data contains the filter text (case-insensitive)
    if (filterText->isWildCard || filterText->isWholeWord)
    {
        Q_ASSERT(mRePattern.isEmpty() == false);
        return message.contains(mReExpression);
    }
    else
    {
        return message.contains(filterText->text, filterText->isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
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

bool LogViewerFilter::matchesIsolation(const LogViewerFilter::sIsolation& isolation, const areg::LogEntry* entry)
{
    if (isolation.kind == LogViewerFilter::eIsolation::IsolationNone)
        return true;
    else if (entry == nullptr)
        return false;
    else if (entry->logCookie != isolation.cookie)
        return false;

    switch (isolation.kind)
    {
    case LogViewerFilter::eIsolation::IsolationThread:
        return (entry->logThreadId == isolation.thread);

    case LogViewerFilter::eIsolation::IsolationScope:
        return (entry->logScopeId == isolation.scopeId);

    case LogViewerFilter::eIsolation::IsolationCall:
        return (entry->logScopeId == isolation.scopeId) && (entry->logSessionId == isolation.sessionId);

    case LogViewerFilter::eIsolation::IsolationProcess:
    default:
        return true;
    }
}

inline bool LogViewerFilter::matchIsolation(const areg::LogEntry* msg) const
{
    return LogViewerFilter::matchesIsolation(mIsolation, msg);
}

inline void LogViewerFilter::_clearData()
{
    mComboFilters.clear();
    mTextFilters.clear();
    mRevealed.clear();
    mIsolation    = LogViewerFilter::sIsolation{ };
    mViewPriority = 0;
}
