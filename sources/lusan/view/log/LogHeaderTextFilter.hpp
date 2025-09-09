#ifndef LUSAN_VIEW_LOG_LOGHEADERTEXTFILTER_HPP
#define LUSAN_VIEW_LOG_LOGHEADERTEXTFILTER_HPP
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
 *  \file        lusan/view/log/LogHeaderTextFilter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/log/LogFilterBase.hpp"
#include <QFrame>

class LoggingModelBase;
class QLineEdit;

class LogHeaderTextFilter : public QFrame
{
    Q_OBJECT

public:
    /**
     * \brief   Creates the simple or extended text filter object.
     *          The extended text filter is an instance of SearchLineEdit and contains additional tool-buttons
     * \param   isExtended  If true, the text filter is extended and contains additional tool-buttons for match case, match word, wild card, and backward search.
     * \param   parent  The parent widget of the text filter.
     **/
    LogHeaderTextFilter(LoggingModelBase* model, bool isExtended, QWidget* parent = nullptr);

public:
    inline void setModel(LoggingModelBase* model);

    inline LoggingModelBase* getModel(void) const;

    LogFilterBase::sFilterData getFilterData(void) const;

    void clearFilter(void);

    void showFilter(void);

    void hideFilter(void);

signals:

    void signalFilterChanged(const LogFilterBase::sFilterData& data);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QLineEdit*          mLineEdit;  //!< Line edit control.
    LoggingModelBase*   mModel;     //!< The logging model base class to use a filtering data base class.
    bool                mExtended;  //!< Flag, indicating whether the line edit is extended and contains tool-buttons or not.
};

inline void LogHeaderTextFilter::setModel(LoggingModelBase* model)
{
    mModel = model;
}

inline LoggingModelBase* LogHeaderTextFilter::getModel(void) const
{
    return mModel;
}

#endif  // LUSAN_VIEW_LOG_LOGHEADERTEXTFILTER_HPP
