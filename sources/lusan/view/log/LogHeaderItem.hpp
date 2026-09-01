#ifndef LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
#define LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
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
 *  \file        lusan/view/log/LogHeaderItem.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "areg/base/String.hpp"

#include <QFrame>
#include <QList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogFilterBase;
class LogTableHeader;

//////////////////////////////////////////////////////////////////////////
// LogHeaderItem class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   Header item, which contains visual elements to visualize by need.
 **/
class LogHeaderItem : public QObject
{
    Q_OBJECT

private:
/************************************************************************
 * Implemented classes
 ************************************************************************/

    //!< The type of visual object to display
    enum eType
    {
          None  //!< Nothing to display
        , Combo //!< Display combo-box
        , Text  //!< Display line-editor
    };

public:
    /**
     * \brief   Initialize the elements
     * \param   header  The header object
     * \param   logicalIndex  The logical index of the column
     */
    LogHeaderItem(LogTableHeader& header, int logicalIndex);

    /**
     * \brief   Visualize the filter widgets
     **/
    void showFilters();

    /**
     * \brief   Opens the filter panel under the given rectangle instead of under the header
     *          section, so a column the table does not show can be reached as well.
     * \param   anchor  The rectangle to open the panel under, in screen coordinates.
     **/
    void showFiltersAt(const QRect& anchor);

    /**
     * \brief   Narrows the column to the value the given row carries, or to every value
     *          but that one.
     * \param   entry   The row the value is taken from.
     * \param   exclude True to keep every value except the one the row carries.
     * \return  True when the column took the value.
     **/
    bool pickValue(const areg::LogEntry& entry, bool exclude);

    /**
     * \brief   Returns the value of the given row in this column, as the filter panel of the
     *          column holds it. Empty when the column is narrowed by a phrase.
     **/
    NELusanCommon::AnyData valueOf(const areg::LogEntry& entry) const;

    /**
     * \brief   Sets the filter string for line edit filter control.
     **/
    void setFilterData(const QString & data);

    /**
     * \brief   Sets the phrase and the match options of a text filter control.
     * \param   filter  The phrase and the options to match it with.
     **/
    void setFilterData(const NELusanCommon::FilterString & filter);

    /**
     * \brief   Sets the list of strings in the combo-box filter control
     **/
    void setFilterData(const std::vector<QString> & data, const NELusanCommon::AnyList& list);
    
    /**
     * \brief   Sets the list of strings in the combo-box filter control
     **/
    void setFilterData(const std::vector<areg::String> & data, const NELusanCommon::AnyList& list);

    /**
     * \brief   Returns true if header object can be visualized in the pop-up widget.
     **/
    inline bool canPopupFilter() const;

    /**
     * \brief   Returns true if the column carries a filter the reader set.
     **/
    inline bool isFiltered() const;

    /**
     * \brief   Resets filter data.
     **/
    void resetFilter();

    /**
     * \brief   Returns the filter data.
     **/
    QList<NELusanCommon::FilterData> getFilterData() const;

private:
/************************************************************************
 * Hidden methods
 ************************************************************************/

    //!< Returns the logical index of the column.
    //!< Returns `-1` if the column is not active.
    inline int fromColumnToIndex() const;

    //!< Returns the column from the index.
    //!< Return LogColumnInvalid value if index is invalid.
    inline LoggingModelBase::eColumn fromIndexToColumn(int logicalIndex) const;

    //!< Remembers whether the column carries a filter and draws its section again.
    void markFiltered(bool active);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    LoggingModelBase::eColumn mColumn;  //!< The index of the header item.
    eType           mType;      //!< Type of the header item.
    bool            mActive;    //!< True while the column carries a filter.
    LogTableHeader& mHeader;    //!< The header object, which contains this item.
    LogFilterBase * mWidget;    //<!< The filter widget, which is displayed in the header item.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LogHeaderItem::canPopupFilter() const
{
    return (mType != None);
}

inline bool LogHeaderItem::isFiltered() const
{
    return mActive;
}

#endif  // LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
