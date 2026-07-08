/************************************************************************
 * This file is part of the Lusan project, an official component of the Areg SDK.
 * Lusan is a graphical user interface (GUI) tool designed to support the development,
 * debugging, and testing of applications built with the Areg Framework.
 *
 * Lusan is available as free and open-source software under the Apache version 2.0 License,
 * providing essential features for developers.
 *
 * For detailed licensing terms, please refer to the LICENSE file included
 * with this distribution or contact us at info[at]areg.tech.
 *
 * \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 * \file        lusan/view/common/WorkspaceSetupDialog.hpp
 * \ingroup     Lusan - GUI Tool for Areg SDK
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
    WorkspaceSetupDialog();

    virtual ~WorkspaceSetupDialog();

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////

    /**
     * \brief   Applies the directories set in the workspace setup dialog.
     **/
    void applyDirectories();

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::WorkspaceSetupDialog*   mUi;                //!< The user interface object.
    OptionPageProjectDirs*      mOptionProjectDirs; //!< The directory settings.
};

#endif  // LUSAN_VIEW_COMMON_WORKSPACESETUPDIALOG_HPP
