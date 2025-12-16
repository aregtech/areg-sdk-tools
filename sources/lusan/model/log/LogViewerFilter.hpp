#ifndef LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
#define LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
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
 *  \file        lusan/model/log/LogViewerFilter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log Viewer Filter Proxy Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QSortFilterProxyModel>

#include "lusan/common/NELusanCommon.hpp"
#include "areg/logging/NELogging.hpp"
#include <QMap>
#include <QString>
#include <QRegularExpression>


class LoggingModelBase;

/**
 * \brief   Filter proxy model for the log viewer to enable filtering of log messages.
 *          This proxy model filters the LiveLogsModel based on user-selected criteria
 *          from the header filters (combo boxes and text filters).
 **/
class LogViewerFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with parent object.
     * \param   model   The logging data model object.
     **/
    explicit LogViewerFilter(LoggingModelBase* model = nullptr);

    virtual ~LogViewerFilter(void);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
public slots:
    /**
     * \brief   Sets combo box filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   filters         The list of selected items to filter by.
     **/
    void setComboFilter(int logicalColumn, const NELusanCommon::FilterList& filters);

    /**
     * \brief   Sets text filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   text           The text to filter by.
     **/
    void setTextFilter(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard);
    void setTextFilter(int logicalColumn, const NELusanCommon::FilterString& filter);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Clears all filters.
     **/
    virtual void clearFilters(void);

    /**
     * \brief   Returns true if the given source row has exact match of the filters.
     *          The method returns false if source model is not set or there are no filters.
     *          The method returns true if filters passed and at least one hat exact match.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row has exact match of the filter.
     **/
    virtual bool filterExactMatch(const QModelIndex& index) const;

    /**
     * \brief   Returns true if the given source row should be included in the model.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row should be included, false otherwise.
     **/
    virtual bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Helper method to check if a row matches the combo filters.
     * \param   model  The logging model to use for filtering.
     * \param   msg    The log message to check against the filters.
     * \return  True if the row matches all combo filters.
     **/
    NELusanCommon::eMatchType matchesComboFilters(LoggingModelBase* model, const NELogging::sLogMessage* msg) const;

    /**
     * \brief   Helper method to check if a row matches the text filters.
     * \param   model  The logging model to use for filtering.
     * \param   msg    The log message to check against the filters.
     * \return  True if the row matches all text filters.
     **/
    NELusanCommon::eMatchType matchesTextFilters(LoggingModelBase* model, const NELogging::sLogMessage* msg) const;

    /**
     * \brief   Helper method to perform wildcard matching.
     * \param   text            The text to match against the wildcard pattern.
     * \param   wildcardPattern The wildcard pattern to match against.
     * \param   isCaseSensitive Flag indicating if the match is case-sensitive.
     * \param   isWholeWord     Flag indicating if the match is for whole words only.
     * \return  True if the text matches the wildcard pattern, false otherwise.
     **/
    bool wildcardMatch(const QString& text, const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord) const;

    /**
     * \brief   Checks if the log message matches the priority filters.
     * \param   msg     The log message to check.
     * \param   filters The list of priority filters to match against.
     * \return  True if the log message matches any of the priority filters, false otherwise.
     **/
    inline bool matchPrio(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the source filters.
     * \param   msg     The log message to check.
     * \param   filters The list of source filters to match against.
     * \return  True if the log message matches any of the source filters, false otherwise.
     **/
    inline bool matchSources(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the thread filters.
     * \param   msg     The log message to check.
     * \param   filters The list of thread filters to match against.
     * \return  True if the log message matches any of the thread filters, false otherwise.
     **/
    inline bool matchThreads(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the duration filters.
     * \param   msg     The log message to check.
     * \param   filters The list of duration filters to match against.
     * \return  True if the log message matches any of the duration filters, false otherwise.
     **/
    inline bool matchDuration(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Checks if the log message matches the message text filters.
     * \param   msg     The log message to check.
     * \param   filters The list of message text filters to match against.
     * \return  True if the log message matches any of the message text filters, false otherwise.
     **/
    inline bool matchMessage(const NELogging::sLogMessage* msg, const NELusanCommon::FilterList& filters) const;

    /**
     * \brief   Prepares the regular expression for wildcard matching.
     * \param   wildcardPattern The wildcard pattern to convert to a regular expression.
     * \param   isCaseSensitive Flag indicating if the match is case-sensitive.
     * \param   isWholeWord     Flag indicating if the match is for whole words only.
     * \param   isWildCard      Flag indicating if the pattern is a wildcard.
     **/
    inline void prepareReExpression(const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord, bool isWildCard);

    //!< Clear filter data/
    inline void _clearData(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QMap<int, NELusanCommon::FilterList>    mComboFilters;  //!< Map of column index to selected filter items
    QMap<int, NELusanCommon::FilterList>    mTextFilters;   //!< Map of column index to filter text
    QString                                 mRePattern;     //!< Regular expression pattern for wildcard matching
    QRegularExpression                      mReExpression;  //!< Regular expression for wildcard matching

//////////////////////////////////////////////////////////////////////////
// Forbidden call
//////////////////////////////////////////////////////////////////////////
private:
    LogViewerFilter(void) = delete;
    DECLARE_NOCOPY_NOMOVE(LogViewerFilter);
};

#endif // LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
