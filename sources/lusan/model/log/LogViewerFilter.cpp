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

void LogViewerFilter::setComboFilter(int logicalColumn, const QList<LogComboFilter::sComboItem>& items)
{
    if (items.isEmpty())
    {
        mComboFilters.remove(logicalColumn);
    }
    else
    {
        mComboFilters[logicalColumn] = items;
    }

    invalidateFilter();
}

void LogViewerFilter::setTextFilter(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard)
{
    if (text.isEmpty())
    {
        mTextFilters.remove(logicalColumn);
    }
    else
    {
        mTextFilters[logicalColumn] = sStringFilter{text, isCaseSensitive, isWholeWord, isWildCard};
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
    eMatchType comboMatch = matchesComboFilters(index);
    eMatchType textMatch = matchesTextFilters(index);

    return  ((comboMatch != eMatchType::NoMatch)    && (textMatch != eMatchType::NoMatch)) &&
            ((comboMatch == eMatchType::ExactMatch) || (textMatch == eMatchType::ExactMatch));
}

bool LogViewerFilter::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    QModelIndex index = sourceModel() != nullptr ? sourceModel()->index(row, 0, parent) : QModelIndex();

    // Check if row matches all active filters
    return  (matchesComboFilters(index) != eMatchType::NoMatch) && 
            (matchesTextFilters(index)  != eMatchType::NoMatch);
}

LogViewerFilter::eMatchType LogViewerFilter::matchesComboFilters(const QModelIndex& index) const
{
    eMatchType matchType = eMatchType::PartialMatch;
    if (index.isValid() == false)
        return eMatchType::NoMatch;
    
    QAbstractItemModel* model = sourceModel();
    if (model == nullptr)
        return matchType;
    
    int row = index.row();    
    // Check each active combo filter
    for (auto it = mComboFilters.constBegin(); it != mComboFilters.constEnd(); ++it)
    {
        int column = it.key();
        const QList<LogComboFilter::sComboItem>& filterItems = it.value();

        if (filterItems.isEmpty())
            continue;
        
        // Get the data for this column and row
        QModelIndex idxCol = model->index(row, column);
        if (idxCol.isValid() == false)
            continue;
        
        const NELogging::sLogMessage* msg = model->data(idxCol, Qt::UserRole).toString();
        // Check if the cell data matches any of the selected filter items
        bool matches = false;
        for (const QString& filterItem : filterItems)
        {
            if (cellData == filterItem)
            {
                matches     = true;
                matchType   = eMatchType::ExactMatch;
                break;
            }
        }
        
        // If this column doesn't match, the row is filtered out
        if (!matches)
            return eMatchType::NoMatch;
    }

    return matchType;
}

LogViewerFilter::eMatchType LogViewerFilter::matchesTextFilters(const QModelIndex& index) const
{
    eMatchType matchType = eMatchType::PartialMatch;
    if (index.isValid() == false)
        return eMatchType::NoMatch;
    
    QAbstractItemModel* model = sourceModel();
    if (model == nullptr)
        return matchType;
    
    const int row{ index.row() };
    // Check each active text filter
    for (auto it = mTextFilters.constBegin(); it != mTextFilters.constEnd(); ++it)
    {
        int column = it.key();
        const sStringFilter& filterText = it.value();

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
                return eMatchType::NoMatch;
        }
        else
        {
            // Check if the cell data contains the filter text (case-insensitive)
            if (filterText.isWildCard || filterText.isWholeWord)
            {
                if (wildcardMatch(cellData, filterText.text, filterText.isCaseSensitive, filterText.isWholeWord) == false)
                    return eMatchType::NoMatch;
    
                matchType = eMatchType::ExactMatch;
            }
            else if (cellData.contains(filterText.text, filterText.isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) == false)
            {
                return eMatchType::NoMatch;
            }
            else
            {
                matchType = eMatchType::ExactMatch;
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
