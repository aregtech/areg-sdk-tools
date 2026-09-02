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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/log/LogTableView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the table that draws the log rows.
 *
 ************************************************************************/

#include "lusan/view/log/LogTableView.hpp"

#include <QScrollBar>

LogTableView::LogTableView(QWidget* parent /*= nullptr*/)
    : QTableView(parent)
{
}

void LogTableView::scrollTo(const QModelIndex& index, ScrollHint hint /*= EnsureVisible*/)
{
    QScrollBar* bar{ horizontalScrollBar() };
    const int position{ bar != nullptr ? bar->value() : 0 };
    QTableView::scrollTo(index, hint);
    if ((bar != nullptr) && (bar->value() != position))
    {
        bar->setValue(position);
    }
}
