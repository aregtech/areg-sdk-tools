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

#include <QStyledItemDelegate>

#include "lusan/model/log/LogSearchModel.hpp"
#include <QPainter>
#include <QTextLayout>
#include <QTextLine>
#include <QList>

class LogTextHighlight : public QStyledItemDelegate
{
public:
    LogTextHighlight(QObject* parent = nullptr);

    void setFoundPos(const LogSearchModel::sFoundPos& foundPos);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    LogSearchModel::sFoundPos mFoundPos;
};

#endif  // LUSAN_VIEW_LOG_LOGTEXTHIGHLIGHT_HPP
