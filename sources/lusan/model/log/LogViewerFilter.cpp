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
    , mLogModel             (model)
{
    setSourceModel(model);
}

LogViewerFilter::~LogViewerFilter(void)
{
    setSourceModel(nullptr);
    clearFilters();
    mLogModel = nullptr;
}

void LogViewerFilter::setComboFilter(int logicalColumn, const QStringList& items)
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
    mComboFilters.clear();
    mTextFilters.clear();
    invalidateFilter();
}

bool LogViewerFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_UNUSED(source_parent);

    if (mLogModel == nullptr)
        return true;

    // Check if row matches all active filters
    return matchesComboFilters(source_row) && matchesTextFilters(source_row);
}

bool LogViewerFilter::matchesComboFilters(int source_row) const
{
    if (mLogModel == nullptr)
        return true;

    // Check each active combo filter
    for (auto it = mComboFilters.constBegin(); it != mComboFilters.constEnd(); ++it)
    {
        int column = it.key();
        const QStringList& filterItems = it.value();

        if (filterItems.isEmpty())
            continue;

        // Get the data for this column and row
        QModelIndex index = mLogModel->index(source_row, column);
        if (index.isValid() == false)
            continue;

        QString cellData = mLogModel->data(index, Qt::DisplayRole).toString();
        // Check if the cell data matches any of the selected filter items
        bool matches = false;
        for (const QString& filterItem : filterItems)
        {
            if (cellData == filterItem)
            {
                matches = true;
                break;
            }
        }

        // If this column doesn't match, the row is filtered out
        if (!matches)
            return false;
    }

    return true;
}

bool LogViewerFilter::matchesTextFilters(int source_row) const
{
    if (mLogModel == nullptr)
        return true;

    // Check each active text filter
    for (auto it = mTextFilters.constBegin(); it != mTextFilters.constEnd(); ++it)
    {
        int column = it.key();
        const sStringFilter& filterText = it.value();

        if (filterText.text.isEmpty())
            continue;

        // Get the data for this column and row
        QModelIndex index = mLogModel->index(source_row, column);
        if (index.isValid() == false)
            continue;

        QString cellData = mLogModel->data(index, Qt::DisplayRole).toString();

        // Check if the cell data contains the filter text (case-insensitive)
        if (filterText.isWildCard || filterText.isWholeWord)
        {
            if (wildcardMatch(cellData, filterText.text, filterText.isCaseSensitive, filterText.isWholeWord) == false)
                return false;
        }
        else if (cellData.contains(filterText.text, filterText.isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) == false)
        {
            return false;
        }
    }

    return true;
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

inline void LogViewerFilter::clearFilter(sScopeFilter& filter)
{
    filter.sessionIds.clear();
    filter.prioMask = 0;
    filter.moduleId = 0;
    filter.filterMask = static_cast<uint32_t>(eScopeMask::NothingValid);
}

void LogViewerFilter::setScopeFilter(uint32_t scopeId, uint32_t moduleId, const std::vector<uint32_t>& sessionIds, uint32_t prioMask)
{
    sScopeFilter& filter = mScopeFilter[scopeId];
    filter.sessionIds   = sessionIds;
    filter.moduleId     = moduleId;
    filter.prioMask     = prioMask;
    filter.filterMask   = static_cast<uint32_t>(eScopeMask::NothingValid);

    if (moduleId != 0)
    {
        filter.filterMask |= static_cast<uint32_t>(eScopeMask::ModuleValid);
    }

    if (sessionIds.empty() == false)
    {
        filter.filterMask |= static_cast<uint32_t>(eScopeMask::SessionValid);
    }

    if (prioMask != 0)
    {
        filter.filterMask |= static_cast<uint32_t>(eScopeMask::PrioValid);
    }
}

void LogViewerFilter::addScopeFilter(uint32_t scopeId, uint32_t moduleId, const std::vector<uint32_t>& sessionIds, uint32_t prioMask)
{
    sScopeFilter& filter = mScopeFilter[scopeId];


    if (sessionIds.empty() == false)
    {
        filter.filterMask |= static_cast<uint32_t>(eScopeMask::SessionValid);
        for (const auto& sessionId : sessionIds)
        {
            if (std::find(filter.sessionIds.begin(), filter.sessionIds.end(), sessionId) == filter.sessionIds.end())
            {
                filter.sessionIds.push_back(sessionId);
            }
        }
    }
}
