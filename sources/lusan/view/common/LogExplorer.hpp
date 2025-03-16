#ifndef LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
#define LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
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
 *  \file        lusan/view/common/LogExplorer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the log explorer.
 *
 ************************************************************************/

#include <QList>
#include <QString>
#include <QWidget>

class MdiMainWindow;

namespace Ui {
    class LogExplorer;
}

//////////////////////////////////////////////////////////////////////////
// LogExplorer class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The LogExplorer class is a view of the logging sources and logging scopes.
 **/
class LogExplorer : public    QWidget
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    LogExplorer(MdiMainWindow* mainFrame, QWidget* parent = nullptr);

    
//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    MdiMainWindow*          mMainFrame;     //!< The main frame of the application.
    Ui::LogExplorer*        ui;             //!< The user interface object.
};

#endif  // LUSAN_VIEW_COMMON_LOGEXPLORER_HPP
