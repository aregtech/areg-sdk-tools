﻿#ifndef LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
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
#include <QStringList>
#include <QMap>

#include "areg/base/GEGlobal.h"

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
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:

    //!< Structure to hold text filter parameters
    struct sStringFilter
    {
        QString text            {};         //!< The text to filter by
        bool    isCaseSensitive {false};    //!< Indicates if the filter is case-sensitive
        bool    isWholeWord     {false};    //!< Indicates if the filter matches whole words only
        bool    isWildCard      {false};    //!< Indicates if the filter uses wildcards
    };

protected:

    /**
     * \brief   The type of match for the filter.
     *          NoMatch - no match found,
     *          PartialMatch - partial match found,
     *          ExactMatch - exact match found.
     **/
    enum eMatchType : int
    {
          NoMatch       = 0 //!< Has not match of filters
        , PartialMatch  = 1 //!< Has partial match of filters
        , ExactMatch    = 2 //!< Has exact match of filters
    };

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
     * \param   items          The list of selected items to filter by.
     **/
    void setComboFilter(int logicalColumn, const QStringList& items);

    /**
     * \brief   Sets text filter for a specific column.
     * \param   logicalColumn   The logical column index to filter.
     * \param   text           The text to filter by.
     **/
    void setTextFilter(int logicalColumn, const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWildCard);

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
    
//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Returns true if the given source row should be included in the model.
     * \param   row      The row index in the source model.
     * \param   parent   The parent index in the source model.
     * \return  True if the row should be included, false otherwise.
     **/
    virtual bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Helper method to check if a row matches the combo filters.
     * \param   row         The row index in the source model.
     * \return  True if the row matches all combo filters.
     **/
    LogViewerFilter::eMatchType matchesComboFilters(const QModelIndex& index) const;

    /**
     * \brief   Helper method to check if a row matches the text filters.
     * \param   row  The row index in the source model.
     * \return  True if the row matches all text filters.
     **/
    LogViewerFilter::eMatchType matchesTextFilters(const QModelIndex& index) const;

    /**
     * \brief   Helper method to perform wildcard matching.
     * \param   text            The text to match against the wildcard pattern.
     * \param   wildcardPattern The wildcard pattern to match against.
     * \param   isCaseSensitive Flag indicating if the match is case-sensitive.
     * \param   isWholeWord     Flag indicating if the match is for whole words only.
     * \return  True if the text matches the wildcard pattern, false otherwise.
     **/
    bool wildcardMatch(const QString& text, const QString& wildcardPattern, bool isCaseSensitive, bool isWholeWord) const;
    
private:

    //!< Clear filter data/
    inline void _clearData(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QMap<int, QStringList>      mComboFilters;  //!< Map of column index to selected filter items
    QMap<int, sStringFilter>    mTextFilters;   //!< Map of column index to filter text

//////////////////////////////////////////////////////////////////////////
// Forbidden call
//////////////////////////////////////////////////////////////////////////
private:
    LogViewerFilter(void) = delete;
    DECLARE_NOCOPY_NOMOVE(LogViewerFilter);
};

#endif // LUSAN_MODEL_LOG_LOGVIEWERFILTER_HPP
