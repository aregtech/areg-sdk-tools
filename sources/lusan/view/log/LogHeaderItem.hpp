#ifndef LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
#define LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/log/LogHeaderItem.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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
    void showFilters(void);

    /**
     * \brief   Sets the filter string for line edit filter control.
     **/
    void setFilterData(const QString & data);

    /**
     * \brief   Sets the list of strings in the combo-box filter control
     **/
    void setFilterData(const std::vector<QString> & data, const NELusanCommon::AnyList& list);
    
    /**
     * \brief   Sets the list of strings in the combo-box filter control
     **/
    void setFilterData(const std::vector<String> & data, const NELusanCommon::AnyList& list);

    /**
     * \brief   Sets the list of integers in the combo-box filter control
     **/
    void setFilterData(const std::vector<ITEM_ID> & data, const NELusanCommon::AnyList& list);

    /**
     * \brief   Returns true if header object can be visualized in the pop-up widget.
     **/
    inline bool canPopupFilter(void) const;

    /**
     * \brief   Resets filter data.
     **/
    void resetFilter(void);

    /**
     * \brief   Returns the filter data.
     **/
    QList<NELusanCommon::FilterData> getFilterData(void) const;

private:
/************************************************************************
 * Hidden methods
 ************************************************************************/

    //!< Returns the logical index of the column.
    //!< Returns `-1` if the column is not active.
    inline int fromColumnToIndex(void) const;

    //!< Returns the column from the index.
    //!< Return LogColumnInvalid value if index is invalid.
    inline LoggingModelBase::eColumn fromIndexToColumn(int logicalIndex) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    LoggingModelBase::eColumn mColumn;  //!< The index of the header item.
    eType           mType;      //!< Type of the header item.
    LogTableHeader& mHeader;    //!< The header object, which contains this item.
    LogFilterBase * mWidget;    //<!< The filter widget, which is displayed in the header item.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LogHeaderItem::canPopupFilter(void) const
{
    return (mType != None);    
}

#endif  // LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
