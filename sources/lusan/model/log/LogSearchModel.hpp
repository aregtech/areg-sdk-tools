#ifndef LUSAN_MODEL_LOG_LOGSEARCHMODEL_HPP
#define LUSAN_MODEL_LOG_LOGSEARCHMODEL_HPP
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
 *  \file        lusan/model/log/LogSearchModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Search Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include "lusan/common/NELusanCommon.hpp"
#include "areg/base/GEGlobal.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class QAbstractItemModel;

/**
 * \brief   LogSearchModel class provides functionality to search for text in log messages.
 *          It can search in both live and offline logging models.
 *          The search can be case-sensitive, whole word only, and can use wildcards.
 **/
class LogSearchModel    : public QObject
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    static constexpr   uint32_t    InvalidPos  { 0xFFFFFFFFu };    //!< Invalid position constants
    static constexpr   uint32_t    NotFound    { 0xFFFFFFFFu };    //!< Not found position constants
    static constexpr   uint32_t    ReachedEnd  { 0xFFFFFFFEu };    //!< Reached end position constants

    /**
     * \brief   Structure to hold the found position and related information.
     *          It contains the row, column, start and end positions of the found text.
     **/
    struct sFoundPos
    {
        //!< The row index where the text was found
        uint32_t rowFound   { InvalidPos };
        //!< The column index where the text was found
        int32_t  colFound   { static_cast<int32_t>(InvalidPos) };
        //!< The start position of search phrase that was found in the text of specified row and column or log data
        int32_t  posStart   { static_cast<int32_t>(InvalidPos) };
        //!< The end position of search phrase that was found in the text of specified row and column or log data
        int32_t  posEnd     { static_cast<int32_t>(InvalidPos) };
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    LogSearchModel(QAbstractItemModel* logModel = nullptr);

    virtual ~LogSearchModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the logging model used for searching.
     *          It can be either live or offline logging model.
     **/
    inline QAbstractItemModel* getLogModel(void) const;

    /**
     * \brief   Sets the logging model to use for searching.
     *          It can be either live or offline logging model.
     * \param   logModel    The logging model to set.
     **/
    inline void setLogModel(QAbstractItemModel* lodModel);

    /**
     * \brief   Checks if the end of the log has been reached.
     *          It returns true if the position is equal to ReachedEnd.
     * \param   pos The position to check.
     * \return  True if the end is reached, false otherwise.
     **/
    inline bool isReachedEnd(uint32_t pos) const;

    /**
     * \brief   Returns the row index where the search started.
     *          It is used to indicate the starting point of the search.
     * \return  The row index where the search started.
     **/
    inline uint32_t getRowBegin(void) const;

    /**
     * \brief   Returns the row index where the search found a match.
     *          It is used to indicate the row where the search phrase was found.
     * \return  The row index where the search found a match.
     **/
    inline uint32_t getFoundRow(void) const;

    /**
     * \brief   Return the index of the column where the search found a match.
     **/
    inline uint32_t getFoundColumn(void) const;

    /**
     * \brief   Returns the start positions of the found search phrase in the string.
     **/
    inline int32_t getFoundStartPosition(void) const;

    /**
     * \brief   Returns the end positions of the found search phrase in the string.
     **/
    inline int32_t getFoundEndPosition(void) const;

    /**
     * \brief   Returns the search phrase used for searching.
     *          It is the text that was searched in the log messages.
     **/
    inline const  QString& getSearchPhrase(void) const;

    /**
     * \brief   Returns true if the next search can be performed.
     **/
    inline bool canSearchNext(void) const;

    /**
     * \brief   Returns true if the search has found a match.
     *          It is used to indicate whether the search was successful or not.
     **/
    inline bool hasFound(void) const;

    /**
     * \brief   Returns true if specified position is valid.
     **/
    bool isValidPosition(uint32_t pos) const;

    /**
     * \brief   Returns true if specified position structure is valid.
     **/
    bool isValidPosition(const LogSearchModel::sFoundPos& pos) const;

    /**
     * \brief   Returns the search phrase that was found in the log messages.
     *          It is used to display the found phrase in the UI.
     **/
    QString getFoundPhrase(void) const;

    /**
     * \brief   Resets the search parameters and clears the search results.
     **/
    void resetSearch(void);

    /**
     * \brief   Start searching for the specified phrase in the log messages.
     * \param   searchPhrase    The phrase to search for in the log messages. May contain wildcards.
     * \param   startAt         The row index to start searching from.
     * \param   isMatchCase     Flag indicating whether the search is case-sensitive.
     * \param   isMatchWord     Flag indicating whether the search should match whole words only.
     * \param   isWildcard      Flag indicating whether the search phrase contains wildcards.
     * \param   isBackward      Flag indicating whether the search is backward.
     * \return  Returns the found position structure containing the row, column, start and end positions of the found text.
     **/
    LogSearchModel::sFoundPos startSearch(const QString& searchPhrase, uint32_t startAt, bool isMatchCase, bool isMatchWord, bool isWildcard, bool isBackward);

    /**
     * \brief   Continues searching for the next occurrence of the search phrase in the log messages.
     *          It starts searching from the last found position.
     * \param   lastFound   The last found position to continue searching from.
     * \return  Returns the found position structure containing the row, column, start and end positions of the found text.
     **/
    LogSearchModel::sFoundPos nextSearch(uint32_t lastFound);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Checks if the text matches the search phrase using wildcards.
     * \param   text        The text to match against the search phrase.
     * \param   posStart    The start position in the text to begin matching.
     * \param   posEnd      The end position in the text to stop matching.
     * \return  Returns true if the text matches the search phrase, false otherwise.
     **/
    bool wildcardMatch(const QString& text, int posStart, int posEnd);

    /**
     * \brief   Checks if the text matches the search phrase using regular expressions.
     * \param   text        The text to match against the search phrase.
     * \param   regex       The regular expression to use for matching.
     * \param   posStart    The start position in the text to begin matching.
     * \param   posEnd      The end position in the text to stop matching.
     * \return  Returns true if the text matches the search phrase, false otherwise.
     **/
    bool wildcardMatch(const QString& text, const QRegularExpression & regex, int posStart, int posEnd);

    /**
     * \brief   Creates the regular expression for searching based on the search phrase and options.
     *          It handles case sensitivity, whole word matching, and wildcard characters.
     * \return  Returns the configured QRegularExpression object for searching.
     **/
    QRegularExpression createRegex(void);

    /**
     * \brief   Checks if the text matches the search phrase as a simple string.
     * \param   text        The text to match against the search phrase.
     * \param   posStart    The start position in the text to begin matching.
     * \param   posEnd      The end position in the text to stop matching.
     * \return  Returns true if the text matches the search phrase, false otherwise.
     **/
    bool stringMatch(const QString& text, int posStart, int posEnd);

    /**
     * \brief   Searches for the start position of the search phrase in the text.
     *          It returns the position where the search phrase starts.
     * \param   text        The text to search in.
     * \param   posStart    The start position in the text to begin searching.
     * \param   posEnd      The end position in the text to stop searching.
     * \return  Returns the start position of the search phrase, or InvalidPos if not found.
     **/
    int positionStartSearch(const QString& text, int posStart, int posEnd) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    QAbstractItemModel* mLogModel;      //!< Pointer to the logging model used for searching
    QString             mSearchPhrase;  //!< The search phrase to find in the log messages
    QString             mCurrentText;   //!< The current text being searched in the log messages
    bool                mIsMatchCase;   //!< Flag indicating if the search is case-sensitive
    bool                mIsMatchWord;   //!< Flag indicating if the search should match whole words only
    bool                mIsWildcard;    //!< Flag indicating if the search phrase contains wildcards
    bool                mIsBackward;    //!< Flag indicating if the search is backward
    uint32_t            mRowBegin;      //!< The row index where the search started
    uint32_t            mRowFound;      //!< The row index where the search found a match
    int32_t             mColFound;      //!< The column index where the search found a match
    int32_t             mPosStart;      //!< The start position of the found search phrase in the text
    int32_t             mPosEnd;        //!< The end position of the found search phrase in the text

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(LogSearchModel);
};

//////////////////////////////////////////////////////////////////////////
// LogSearchModel inline methods implementation
//////////////////////////////////////////////////////////////////////////

inline QAbstractItemModel* LogSearchModel::getLogModel(void) const
{
    return mLogModel;
}

inline void LogSearchModel::setLogModel(QAbstractItemModel* logModel)
{
    mLogModel = logModel;
}

inline bool LogSearchModel::isReachedEnd(uint32_t pos) const
{
    return (pos == ReachedEnd);
}

inline uint32_t LogSearchModel::getRowBegin(void) const
{
    return mRowBegin;
}

inline uint32_t LogSearchModel::getFoundRow(void) const
{
    return mRowFound;
}

inline uint32_t LogSearchModel::getFoundColumn(void) const
{
    return mColFound;
}

inline int32_t LogSearchModel::getFoundStartPosition(void) const
{
    return mPosStart;
}

inline int32_t LogSearchModel::getFoundEndPosition(void) const
{
    return mPosEnd;
}

inline const QString& LogSearchModel::getSearchPhrase(void) const
{
    return mSearchPhrase;
}

inline bool LogSearchModel::canSearchNext(void) const
{
    return (mSearchPhrase.isEmpty() == false) && isValidPosition(mRowBegin) && isValidPosition(mRowFound);
}

inline bool LogSearchModel::hasFound(void) const
{
    return (mRowFound != InvalidPos) && (mRowFound != ReachedEnd);
}

#endif  // LUSAN_MODEL_LOG_LOGSEARCHMODEL_HPP
