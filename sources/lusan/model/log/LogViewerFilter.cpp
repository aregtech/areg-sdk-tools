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
#include <QRegularExpression>

LogViewerFilter::LogViewerFilter(LoggingModelBase* model)
    : QSortFilterProxyModel (model)
    , mComboFilters         ( )
    , mTextFilters          ( )
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
        mComboFilters.remove(logicalColumn);
    }
    else
    {
        mComboFilters[logicalColumn] = filters;
    }

    invalidateFilter();
}

void LogViewerFilter::setTextFilter(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard)
{
    setTextFilter(logicalColumn, NELusanCommon::FilterString{ text, isCaseSensitive, isWholeWord, isWildCard });
}

void LogViewerFilter::setTextFilter(int logicalColumn, const NELusanCommon::FilterString& filter)
{
    if (filter.text.isEmpty())
    {
        mTextFilters.remove(logicalColumn);
    }
    else
    {
        mTextFilters[logicalColumn] = filter;
    }

    invalidateFilter();
}


void LogViewerFilter::clearFilters()
{
    _clearData();
    invalidateFilter();
}

bool LogViewerFilter::filterExactMatch(const QModelIndex& index) const
{
    NELusanCommon::eMatchType comboMatch = matchesComboFilters(index);
    NELusanCommon::eMatchType textMatch = matchesTextFilters(index);

    return  ((comboMatch != NELusanCommon::eMatchType::NoMatch)    && (textMatch != NELusanCommon::eMatchType::NoMatch)) &&
            ((comboMatch == NELusanCommon::eMatchType::ExactMatch) || (textMatch == NELusanCommon::eMatchType::ExactMatch));
}

bool LogViewerFilter::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    QModelIndex index = sourceModel() != nullptr ? sourceModel()->index(row, 0, parent) : QModelIndex();

    // Check if row matches all active filters
    return  (matchesComboFilters(index) != NELusanCommon::eMatchType::NoMatch) && 
            (matchesTextFilters(index)  != NELusanCommon::eMatchType::NoMatch);
}

NELusanCommon::eMatchType LogViewerFilter::matchesComboFilters(const QModelIndex& index) const
{
    NELusanCommon::eMatchType matchType = NELusanCommon::eMatchType::PartialMatch;
    if (index.isValid() == false)
        return NELusanCommon::eMatchType::NoMatch;
    
    LoggingModelBase* model = static_cast<LoggingModelBase *>(sourceModel());
    if (model == nullptr)
        return matchType;
    
    int row = index.row();    
    // Check each active combo filter
    for (auto it = mComboFilters.constBegin(); (matchType != NELusanCommon::eMatchType::NoMatch) && (it != mComboFilters.constEnd()); ++it)
    {
        int column = it.key();
        const NELusanCommon::FilterList& filters = it.value();

        if (filters.isEmpty())
            continue;
        
        int len = static_cast<int>(filters.size());
        // Get the data for this column and row
        QModelIndex idxCol = model->index(row, column);
        if (idxCol.isValid() == false)
            continue;
        
        const NELogging::sLogMessage* msg = model->getLogData(row);
        LoggingModelBase::eColumn ecol = model->fromIndexToColumn(column);
        // Check if the cell data matches any of the selected filter items
        bool matches = false;
        for (int i = 0; (matches == false) && (i < len); ++i)
        {
            const NELusanCommon::sFilterData & f = filters[i];
            switch (ecol)
            {
            case LoggingModelBase::eColumn::LogColumnPriority:
            {
                matches = (std::any_cast<uint16_t>(f.data) & static_cast<uint16_t>(msg->logMessagePrio));
            }
            break;

            case LoggingModelBase::eColumn::LogColumnSource:
            case LoggingModelBase::eColumn::LogColumnSourceId:
            {
                matches = std::any_cast<ITEM_ID>(f.data) == msg->logCookie;
            }
            break;
            
            case LoggingModelBase::eColumn::LogColumnThreadId:
            case LoggingModelBase::eColumn::LogColumnThread:
            {
                matches = std::any_cast<ITEM_ID>(f.data) == msg->logThreadId;
            }
            break;

            case LoggingModelBase::eColumn::LogColumnScopeId:
            {
                matches = std::any_cast<uint32_t>(f.data) == msg->logScopeId;
            }
            break;

            default:
                break;
            }
        }
        
        // If this column doesn't match, the row is filtered out
        matchType = (matches ? NELusanCommon::eMatchType::ExactMatch : NELusanCommon::eMatchType::NoMatch);
    }
    
    return matchType;
}

NELusanCommon::eMatchType LogViewerFilter::matchesTextFilters(const QModelIndex& index) const
{
    NELusanCommon::eMatchType matchType = NELusanCommon::eMatchType::PartialMatch;
    if (index.isValid() == false)
        return NELusanCommon::eMatchType::NoMatch;
    
    QAbstractItemModel* model = sourceModel();
    if (model == nullptr)
        return matchType;
    
    const int row{ index.row() };
    // Check each active text filter
    for (auto it = mTextFilters.constBegin(); it != mTextFilters.constEnd(); ++it)
    {
        int column = it.key();
        const NELusanCommon::sStringFilter& filterText = it.value();

        if (filterText.text.isEmpty())
            continue;

        // Get the data for this column and row
        QModelIndex idxCol = sourceModel()->index(row, column);
        if (idxCol.isValid() == false)
            continue;
        
        QString cellData = model->data(idxCol, Qt::DisplayRole).toString();
        if (model->headerData(column, Qt::Orientation::Horizontal, static_cast<int>(Qt::ItemDataRole::UserRole)).toInt() == static_cast<int>(LoggingModelBase::eColumn::LogColumnTimeDuration))
        {
            uint32_t rowDuration = cellData.toUInt();
            uint32_t filDuration = filterText.text.toUInt();
            if  (rowDuration < filDuration)
                return NELusanCommon::eMatchType::NoMatch;
        }
        else
        {
            // Check if the cell data contains the filter text (case-insensitive)
            if (filterText.isWildCard || filterText.isWholeWord)
            {
                if (wildcardMatch(cellData, filterText.text, filterText.isCaseSensitive, filterText.isWholeWord) == false)
                    return NELusanCommon::eMatchType::NoMatch;
    
                matchType = NELusanCommon::eMatchType::ExactMatch;
            }
            else if (cellData.contains(filterText.text, filterText.isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) == false)
            {
                return NELusanCommon::eMatchType::NoMatch;
            }
            else
            {
                matchType = NELusanCommon::eMatchType::ExactMatch;
            }
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

inline void LogViewerFilter::_clearData(void)
{
    mComboFilters.clear();
    mTextFilters.clear();
}
