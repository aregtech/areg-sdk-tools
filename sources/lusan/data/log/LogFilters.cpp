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
 *  \file        lusan/data/log/LogFilters.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log viewer filter elements
 *
 ************************************************************************/
/************************************************************************
 * Include files.
 ************************************************************************/
#include "lusan/data/log/LogFilters.hpp"

////////////////////////////////////////////////////////////////////////
// LogFilterPriorities class implementation
////////////////////////////////////////////////////////////////////////

LogFilterPriorities::LogFilterPriorities(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterPriority, LogFilterBase::eVisualType::VisualList)
    , mMask     ( 0x0000u )
    , mData     ( )
    , mPrios    ( )
{
    mData.push_back("");
    mPrios.push_back(static_cast<uint16_t>(NELogging::eLogPriority::PrioAny));

    mData.push_back("DEBUG");
    mPrios.push_back(static_cast<uint16_t>(NELogging::eLogPriority::PrioDebug));

    mData.push_back("INFO");
    mPrios.push_back(static_cast<uint16_t>(NELogging::eLogPriority::PrioInfo));

    mData.push_back("WARN");
    mPrios.push_back(static_cast<uint16_t>(NELogging::eLogPriority::PrioWarning));

    mData.push_back("ERROR");
    mPrios.push_back(static_cast<uint16_t>(NELogging::eLogPriority::PrioError));

    mData.push_back("FATAL");
    mPrios.push_back(static_cast<uint16_t>(NELogging::eLogPriority::PrioFatal));
}

LogFilterBase::eMatchResult LogFilterPriorities::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return (mMask == 0u) || ((static_cast<uint16_t>(logMessage.logMessagePrio) & mMask) != 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterPriorities::deactivateFilter(void)
{
    LogFilterBase::deactivateFilter();
    mMask = 0x0000u;
}

void LogFilterPriorities::activateFilter(const LogFilterBase::FilterList& filters)
{
    LogFilterBase::activateFilter(filters);
    mMask = 0x0000u;
    for (auto & filter : filters)
    {
        const QString& prio = filter.data;
        for (int i = 0; i < static_cast<int>(mData.size()); ++i)
        {
            if (mData[i] == prio)
            {
                mMask |= mPrios[i];
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////
// LogFilterScopes class implementation
////////////////////////////////////////////////////////////////////////

LogFilterScopes::LogFilterScopes(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterScope, LogFilterBase::eVisualType::VisualList)
    , mScopes   ( )
    , mData     ( )
{
}

LogFilterBase::eMatchResult LogFilterScopes::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    if (mFilters.empty())
        return LogFilterBase::eMatchResult::MatchExact; // filter is not set, accept all
    
    auto it = mScopes.find(logMessage.logCookie);
    if (it == mScopes.end())
        return LogFilterBase::eMatchResult::NoMatch;
    
    const ListScopes & scopes = it->second;
    for (auto id : scopes)
    {
        if (id == logMessage.logScopeId)
            return LogFilterBase::eMatchResult::MatchExact;
    }
        
    return LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterScopes::deactivateFilter(void)
{
    LogFilterBase::deactivateFilter();
    mScopes.clear();
}

void LogFilterScopes::activateFilter(const LogFilterBase::FilterList& filters)
{
    LogFilterBase::activateFilter(filters);
    mScopes.clear();
    if (filters.isEmpty())
        return;

    for (const auto& filter : filters)
    {
        ListScopes& scopes = mScopes[filter.source];
        scopes.push_back(filter.value.digit);
    }
}

////////////////////////////////////////////////////////////////////////
// LogFilterInstances  class declaration
////////////////////////////////////////////////////////////////////////

LogFilterInstances::LogFilterInstances(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterInstance, LogFilterBase::eVisualType::VisualList)
    , mNames    ()
    , mIds      ()
    , mDispNames()
    , mDispIds  ()
{
}

LogFilterBase::eMatchResult LogFilterInstances::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    if (mFilters.isEmpty())
        return LogFilterBase::eMatchResult::MatchExact;
    
    for (auto filter : mFilters)
    {
        if (filter.value.digit == static_cast<uint64_t>(logMessage.logCookie))
            return LogFilterBase::eMatchResult::MatchExact;
    }
    
    return LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterInstances::updateInstanceData(const NEService::sServiceConnectedInstance& instance)
{
    static constexpr char formatName[] { "%s (%lu)" };
    static constexpr char formatId[]   { "%lu (%s)" };
    
    char text[256] {};

    mIds.push_back(QString::number(instance.ciCookie));
    mNames.push_back(QString::fromStdString(instance.ciInstance));
    
    String::formatString(text, 256, formatName, instance.ciInstance, instance.ciCookie);
    mDispNames.push_back(QString::fromUtf8(text));
    
    String::formatString(text, 256, formatId, instance.ciCookie, instance.ciInstance);
    mDispIds.push_back(QString::fromUtf8(text));
}

////////////////////////////////////////////////////////////////////////
// LogFilterTimeCreated class implementation
////////////////////////////////////////////////////////////////////////

LogFilterTimeCreated::LogFilterTimeCreated(void)
    : LogFilterBase (LogFilterBase::eFilterType::FilterTimeCreated, LogFilterBase::eVisualType::VisualUndefined)
{
}

LogFilterBase::eMatchResult LogFilterTimeCreated::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    if (mFilters.isEmpty())
        return LogFilterBase::eMatchResult::MatchExact;
    
    if ((mFilters[MIN_TIME].value.digit <= logMessage.logTimestamp) && (mFilters[MAX_TIME].value.digit >= logMessage.logTimestamp))
    {
        return LogFilterBase::eMatchResult::MatchExact;
    }
    else
    {
        return LogFilterBase::eMatchResult::NoMatch;
    }
}

////////////////////////////////////////////////////////////////////////
// LogFilterTimeReceived class implementation
////////////////////////////////////////////////////////////////////////

LogFilterTimeReceived::LogFilterTimeReceived(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterTimeReceived, LogFilterBase::eVisualType::VisualUndefined)
{
}

LogFilterBase::eMatchResult LogFilterTimeReceived::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    if (mFilters.isEmpty())
        return LogFilterBase::eMatchResult::MatchExact;
    
    if ((mFilters[MIN_TIME].value.digit <= logMessage.logReceived) && (mFilters[MAX_TIME].value.digit >= logMessage.logReceived))
    {
        return LogFilterBase::eMatchResult::MatchExact;
    }
    else
    {
        return LogFilterBase::eMatchResult::NoMatch;
    }
}

////////////////////////////////////////////////////////////////////////
// LogFilterDuration class declaration
////////////////////////////////////////////////////////////////////////

LogFilterDuration::LogFilterDuration(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterDuration, LogFilterBase::eVisualType::VisualText)
    , mDuration ( 0 )
    , mData     ( )
{
}

LogFilterBase::eMatchResult LogFilterDuration::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    return (mDuration <= logMessage.logDuration) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
}

void LogFilterDuration::deactivateFilter(void)
{
    LogFilterBase::deactivateFilter();
    mDuration = 0;
}

void LogFilterDuration::activateFilter(const LogFilterBase::FilterList& filters)
{
    LogFilterBase::activateFilter(filters);
    mDuration = filters.isEmpty() ? 0 : static_cast<uint32_t>(filters[0].value.digit);
    if (mData.isEmpty())
        mData.push_back(QString::number(mDuration));
    else
        mData[0] = QString::number(mDuration);
}

////////////////////////////////////////////////////////////////////////
// LogFilterThread class implementation
////////////////////////////////////////////////////////////////////////

LogFilterThread::LogFilterThread(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterThread, LogFilterBase::eVisualType::VisualList)
    , mThreads      ( )
    , mDataThreads  ( )
    , mDispNames    ( )
    , mDispIds      ( )
{
}

void LogFilterThread::updateThreadData(const NELogging::sLogMessage& logMessage)
{
    static constexpr char formatNames[] { "%s (%ul:%ul)" };
    static constexpr char formatIds[]   { "%ul (%ul)" };
    char text[256] {};
    
    ListThreads& threads = mDataThreads[logMessage.logCookie];
    for (const auto& thread : threads)
    {
        if (thread.value.digit == static_cast<uint64_t>(logMessage.logThreadId))
            return;
    }
    
    sFilterData data{};
    data.source = logMessage.logCookie;
    data.data   = QString::fromUtf8(logMessage.logThread);
    data.value.digit = static_cast<uint64_t>(logMessage.logThreadId);
    threads.push_back(data);
    
    String::formatString(text, 256, formatNames, logMessage.logThread, logMessage.logCookie, logMessage.logThreadId);
    mDispNames.push_back(QString::fromUtf8(text));
    
    String::formatString(text, 256, formatIds, logMessage.logThreadId, logMessage.logCookie);
    mDispIds.push_back(QString::fromUtf8(text));
}

LogFilterBase::eMatchResult LogFilterThread::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    if (mThreads.empty())
        return LogFilterBase::eMatchResult::MatchExact; // filter is not set, accept all
    
    auto it = mThreads.find(logMessage.logCookie);
    if (it == mThreads.end())
        return LogFilterBase::eMatchResult::NoMatch; // no matching thread found, reject
    
    const auto & threads = *it;
    for (const auto& thread : threads)
    {
        if (thread.value.digit == static_cast<uint64_t>(logMessage.logThreadId))
            return LogFilterBase::eMatchResult::MatchExact; // found a matching thread, accept
    }

    return LogFilterBase::eMatchResult::NoMatch; // no matching thread found, reject
}

void LogFilterThread::deactivateFilter(void)
{
    LogFilterBase::deactivateFilter();
    mThreads.clear();
}

void LogFilterThread::activateFilter(const LogFilterBase::FilterList& filters)
{
    LogFilterBase::activateFilter(filters);
    mThreads.clear();
    for (auto& entry : filters)
    {
        ListThreads & threads = mThreads[entry.source];
        threads.push_back(entry);
    }
}

////////////////////////////////////////////////////////////////////////
// LogFilterText class declaration
////////////////////////////////////////////////////////////////////////

LogFilterText::LogFilterText(void)
    : LogFilterBase(LogFilterBase::eFilterType::FilterMessage, LogFilterBase::eVisualType::VisualText)
    , mSearch       ( )
    , mIsSensitive  (false)
    , mIsWholeWord  (false)
    , mIsRegularEx  (false)
    , mRegularEx    ( )
{
}

LogFilterBase::eMatchResult LogFilterText::isLogMessageAccepted(const NELogging::sLogMessage& logMessage) const
{
    const char* text= logMessage.logMessage;
    if (mFilters.isEmpty())
    {
        return LogFilterBase::eMatchResult::MatchExact; // no filter set, accept all
    }
    else if (mIsRegularEx)
    {
        return (containsWildCard(QString::fromUtf8(text)) >= 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
    }
    else if (mIsWholeWord)
    {
        return (containsWord(text) >= 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
    }
    else if (mIsSensitive)
    {
        return (containsExact(text) >= 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
    }
    else
    {
        return (containsInsensitive(text) >= 0) ? LogFilterBase::eMatchResult::MatchExact : LogFilterBase::eMatchResult::NoMatch;
    }
}

void LogFilterText::deactivateFilter(void)
{
    LogFilterBase::deactivateFilter();

    mSearch.clear();
    mIsSensitive= false;
    mIsWholeWord= false;
    mIsRegularEx= false;
    mRegularEx  = QRegularExpression();
}

void LogFilterText::activateFilter(const LogFilterBase::FilterList& filters)
{
    LogFilterBase::activateFilter(filters);

    mSearch.clear();
    mIsSensitive= false;
    mIsWholeWord= false;
    mIsRegularEx= false;
    mRegularEx      = QRegularExpression();
    
    if (filters.isEmpty() == false)
    {
        mSearch = filters[0].data.toStdString();
        mIsSensitive = filters[0].value.srch.isSensitive;
        mIsWholeWord = filters[0].value.srch.isWholeWord;
        mIsRegularEx = filters[0].value.srch.isRegularEx;
        
        if (mIsRegularEx)
        {
            // Escape regex special characters except * and ?
            QString regexPattern = QRegularExpression::escape(filters[0].data);
            regexPattern.replace("\\*", ".*");
            regexPattern.replace("\\?", ".");
            
            // For whole word, use word boundaries, but treat '_' as a word boundary as well
            if (mIsWholeWord)
            {
                // Custom boundaries: start of string or non-word char (including '_'), and end of string or non-word char (including '_')
                // \b does not treat '_' as a boundary, so we use lookarounds
                regexPattern = QStringLiteral("(?:(?<=^)|(?<=[^\\w]|_))") + regexPattern + QStringLiteral("(?:(?=$)|(?=[^\\w]|_))");
            }
            
            QRegularExpression::PatternOptions options = mIsSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
            mRegularEx = QRegularExpression(regexPattern, options);
        }
    }
}

inline int LogFilterText::containsExact(const char* text) const
{
    Q_ASSERT((mFilters.isEmpty() == false) && (text != nullptr));

    const char* phrase= mSearch.c_str();
    const char* found = strstr(text, phrase);
    return (found != nullptr ? static_cast<int>(found - text) : -1);
}

inline int LogFilterText::containsInsensitive(const char* text) const
{
    Q_ASSERT((mFilters.isEmpty() == false) && (text != nullptr));

    const char* phrase  = mSearch.c_str();
    const char* s_start = text;
    const char* p_start = phrase;
    while ((*s_start != '\0') && (*p_start != '\0'))
    {
        if ((*s_start == *p_start) || (tolower(*s_start) == tolower(*p_start)))
        {
            ++s_start;
            ++p_start;
        }
        else
        {
            s_start = ++text; // reset to the next character in str
            p_start = phrase; // reset to the start of phrase
        }
    }

    return (*p_start == '\0' ? static_cast<int>(s_start - text) : -1);
}

int LogFilterText::containsWord(const char* text) const
{
    Q_ASSERT((mFilters.isEmpty() == false) && (text != nullptr));

    int result = -1;
    const int phraseLen = static_cast<int>(mSearch.length());

    do
    {
        result = mIsSensitive ? containsExact(text) : containsInsensitive(text);
        if (result < 0)
            break;

        // Check for word boundaries
        if ((result == 0 || !isalnum(text[result - 1])) && (text[result + phraseLen] == '\0' || !isalnum(text[result + phraseLen])))
            return result;

        text += result + 1; // Move to the next character after the found phrase
    } while (*text != '\0');

    return -1; // No whole word match found
}

inline int LogFilterText::containsWildCard(const QString& text) const
{
    Q_ASSERT((mFilters.isEmpty() == false) && (text.isEmpty() == false) && (mRegularEx.isValid()));
    return text.indexOf(mRegularEx);
}

