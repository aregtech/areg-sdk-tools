#ifndef LUSAN_VIEW_LOG_LOGVIEWER_HPP
#define LUSAN_VIEW_LOG_LOGVIEWER_HPP
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
 *  \file        lusan/view/log/LogViewer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view widget.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiChild.hpp"

class QHeaderView;
class QTableView;
class QWidget;
class LogViewerModel;

namespace Ui {
    class LogViewer;
}

class LogViewer : public MdiChild
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);

private:
    QTableView* getTable(void);
    
    QHeaderView* getHeader(void);
    
private:

private:
    Ui::LogViewer*      ui;
    LogViewerModel*     mLogModel;
    QWidget*            mMdiWindow;
};

#endif // LUSAN_VIEW_LOG_LOGVIEWER_HPP
