/************************************************************************
 * This file is part of the Lusan project, an official component of the AREG SDK.
 * Lusan is a graphical user interface (GUI) tool designed to support the development,
 * debugging, and testing of applications built with the AREG Framework.
 *
 * Lusan is available as free and open-source software under the MIT License,
 * providing essential features for developers.
 *
 * For detailed licensing terms, please refer to the LICENSE.txt file included
 * with this distribution or contact us at info[at]aregtech.com.
 *
 * \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 * \file        lusan/view/common/WorkspaceSetupDialog.hpp
 * \ingroup     Lusan - GUI Tool for AREG SDK
 * \author      Artak Avetyan
 * \brief       Lusan application, the new workspace setup dialog.
 *
 ************************************************************************/
#ifndef LUSAN_VIEW_COMMON_WORKSPACESETUPDIALOG_HPP
#define LUSAN_VIEW_COMMON_WORKSPACESETUPDIALOG_HPP

/************************************************************************
 * Include files.
 ************************************************************************/
#include <QDialog>
#include <memory>

/************************************************************************
 * Dependencies 
 ************************************************************************/
class OptionPageProjectDirs;
namespace Ui {
    class WorkspaceSetupDialog;
}

/**
 * \brief   Dialog to setup directories of the new workspace.
 **/
class WorkspaceSetupDialog : public QDialog
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    WorkspaceSetupDialog(void);

    virtual ~WorkspaceSetupDialog(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////

    /**
     * \brief   Applies the directories set in the workspace setup dialog.
     **/
    void applyDirectories(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::WorkspaceSetupDialog*   mUi;                //!< The user interface object.
    OptionPageProjectDirs*      mOptionProjectDirs; //!< The directory settings.
};

#endif  // LUSAN_VIEW_COMMON_WORKSPACESETUPDIALOG_HPP
