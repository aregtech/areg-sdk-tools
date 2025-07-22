#ifndef LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
#define LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
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
 *  \file        lusan/view/log/LogTextHighlight.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Styling class to highlight search elements.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QStyledItemDelegate>
#include "lusan/model/log/LogSearchModel.hpp"
#include "areg/base/GEGlobal.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class QPainter;
class QStyleOptionViewItem;
class QModelIndex;

/**
 * \brief   LogTextHighlight class is a custom item delegate that highlights the search results in the log viewer.
 **/
class LogTextHighlight : public QStyledItemDelegate
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    LogTextHighlight(const LogSearchModel::sFoundPos& foundPos, QObject* parent = nullptr);
    virtual ~LogTextHighlight(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const LogSearchModel::sFoundPos& mFoundPos;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
    DECLARE_NOCOPY_NOMOVE(LogTextHighlight);
};

#endif  // LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
