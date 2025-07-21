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
 *  \file        lusan/model/log/LogSearchModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Search Model.
 *
 ************************************************************************/

#include "lusan/model/log/LogSearchModel.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"

#include <QRegularExpression>

LogSearchModel::LogSearchModel(LoggingModelBase* logModel)
    : QObject       ( )
    , mLogModel     (logModel)
    , mSearchPhrase ( )
    , mCurrentText  ( )
    , mRowBegin     (InvalidPos)
    , mRowFound     (InvalidPos)
    , mColFound     (static_cast<int>(InvalidPos))
    , mPosStart     (static_cast<int>(InvalidPos))
    , mPosEnd       (static_cast<int>(InvalidPos))
    , mIsMatchCase  (false)
    , mIsMatchWord  (false)
    , mIsWildcard   (false)
    , mIsBackward   (false)
{
}

bool LogSearchModel::isValidPosition(uint32_t pos) const
{
    return  (pos != InvalidPos) && (pos != ReachedEnd) &&
            ((mLogModel != nullptr) && (mLogModel->rowCount() > static_cast<int>(pos)));
}

bool LogSearchModel::isValidPosition(const LogSearchModel::sFoundPos& pos) const
{
    return isValidPosition(pos.rowFound);
}

QString LogSearchModel::getFoundPhrase(void) const
{
    QString result;
    if ((mCurrentText.isEmpty() == false) && (mPosStart != static_cast<int>(InvalidPos)))
    {
        int count = (mPosEnd != static_cast<int>(InvalidPos)) && (mPosEnd >= mPosStart) ? mPosEnd - mPosStart : -1;
        result = mCurrentText.mid(mPosStart, count);
    }
    
    return result;
}

void LogSearchModel::resetSearch(void)
{
    mSearchPhrase.clear();
    mCurrentText.clear();
    mRowBegin       = InvalidPos;
    mRowFound       = InvalidPos;
    mColFound       = static_cast<int>(InvalidPos);
    mPosStart       = static_cast<int>(InvalidPos);
    mPosEnd         = static_cast<int>(InvalidPos);
    mIsMatchCase    = false;
    mIsMatchWord    = false;
    mIsWildcard     = false;
    mIsBackward     = false;
}

LogSearchModel::sFoundPos LogSearchModel::startSearch(const QString& searchPhrase, uint32_t startAt, bool isMatchCase, bool isMatchWord, bool isWildcard, bool isBackward)
{
    LogSearchModel::sFoundPos result{};
    
    resetSearch();
    if (searchPhrase.isEmpty() || (mLogModel == nullptr))
        return LogSearchModel::sFoundPos{};

    mSearchPhrase= searchPhrase;
    mRowBegin    = isValidPosition(startAt) ? startAt : 0u;
    mIsMatchCase = isMatchCase;
    mIsMatchWord = isMatchWord;
    mIsWildcard  = isWildcard;
    mIsBackward  = isBackward;
    
    return nextSearch(mRowBegin);
}

LogSearchModel::sFoundPos LogSearchModel::nextSearch(uint32_t lastFound)
{
    LogSearchModel::sFoundPos result{};
    uint32_t startAt {lastFound};
    if (isValidPosition(startAt) == false)
        return result;
    
    int32_t posStart{ static_cast<int32_t>(mPosStart) };
    int32_t posEnd  { static_cast<int32_t>(mPosEnd)   };
    bool    doSearch{ true };
    QRegularExpression regex(createRegex());
    
    do
    {
        const NELogging::sLogMessage* log{ mLogModel->getLogData(startAt) };
        if (log == nullptr)
            return result;
        
        mCurrentText = QString::fromUtf8(log->logMessage);
        if (wildcardMatch(mCurrentText, regex, posStart, posEnd))
        {
            mRowFound       = startAt;
            doSearch        = false;
            result.colFound = static_cast<int>(LoggingModelBase::eColumn::LogColumnMessage);
            result.posEnd   = mPosEnd;
            result.posStart = mPosStart;
            result.rowFound = mRowFound;
        }
        else
        {
            mCurrentText.clear();
            uint32_t rowCount = static_cast<uint32_t>(mLogModel->rowCount(QModelIndex()));
            if (mIsBackward == false)
            {
                startAt = (startAt < (rowCount - 1)) ? startAt + 1 : 0;
            }
            else
            {
                startAt = (startAt == 0) ? rowCount - 1 : startAt - 1;
            }
            
            posStart = InvalidPos;
            posEnd   = InvalidPos;
            if (startAt == mRowBegin)
            {
                doSearch = false;
                mRowFound = InvalidPos;
            }
        }
        
    } while (doSearch);
    
    return result;
}

bool LogSearchModel::wildcardMatch(const QString& text, const QRegularExpression & regex, int posStart, int posEnd)
{
    if (text.isEmpty())
        return false;
    
    int searchStart = 0;
    int foundStart = -1;
    int foundEnd = -1;
    
    if (mIsBackward == false)
    {
        if (posEnd == static_cast<int32_t>(InvalidPos))
        {
            searchStart = 0;
        }
        else if (posEnd + 1 < text.length())
        {
            searchStart = mPosEnd + 1;
        }
        else
        {
            mPosStart = static_cast<int32_t>(InvalidPos);
            mPosEnd = static_cast<int32_t>(InvalidPos);
            return false;
        }
        
        // Forward search
        QRegularExpressionMatchIterator it = regex.globalMatch(text, searchStart);
        if (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            foundStart = match.capturedStart();
            foundEnd = match.capturedEnd();
        }
    }
    else
    {
        if (posStart == static_cast<int32_t>(InvalidPos))
        {
            searchStart = text.length() - 1;
        }
        else if (posStart >= 0)
        {
            searchStart = posStart - 1;
        }
        else
        {
            mPosStart = static_cast<int32_t>(InvalidPos);
            mPosEnd = static_cast<int32_t>(InvalidPos);
            return false;
        }
        
        // Backward search: find all matches up to searchStart, pick the last one before searchStart
        QRegularExpressionMatchIterator it = regex.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            int s = match.capturedStart();
            int e = match.capturedEnd();
            if (e <= searchStart)
            {
                foundStart = s;
                foundEnd = e;
            }
            else
            {
                break;
            }
        }
    }
    
    if (foundStart != -1 && foundEnd != -1)
    {
        mPosStart = foundStart;
        mPosEnd   = foundEnd;
        return true;
    }
    else
    {
        mPosStart = static_cast<int32_t>(InvalidPos);
        mPosEnd   = static_cast<int32_t>(InvalidPos);
        return false;
    }
}

QRegularExpression LogSearchModel::createRegex(void)
{
    // Escape regex special characters except * and ?
    QString regexPattern = QRegularExpression::escape(mSearchPhrase);
    regexPattern.replace("\\*", ".*");
    regexPattern.replace("\\?", ".");
    
    // For whole word, use word boundaries, but treat '_' as a word boundary as well
    if (mIsMatchWord)
    {
        // Custom boundaries: start of string or non-word char (including '_'), and end of string or non-word char (including '_')
        // \b does not treat '_' as a boundary, so we use lookarounds
        regexPattern = QStringLiteral("(?:(?<=^)|(?<=[^\\w]|_))") + regexPattern + QStringLiteral("(?:(?=$)|(?=[^\\w]|_))");
    }
    
    QRegularExpression::PatternOptions options = mIsMatchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
    return QRegularExpression(regexPattern, options);
}


bool LogSearchModel::wildcardMatch(const QString& text, int posStart, int posEnd)
{
    return wildcardMatch(text, createRegex(), posStart, posEnd);
}
