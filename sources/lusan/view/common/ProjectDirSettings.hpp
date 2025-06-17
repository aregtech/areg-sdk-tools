#ifndef LUSAN_VIEW_COMMON_PROJECTDIRSETTINGS_HPP
#define LUSAN_VIEW_COMMON_PROJECTDIRSETTINGS_HPP
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
 *  \file        lusan/view/common/ProjectDirSettings.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, project settings widget.
 *
 ************************************************************************/

#include <QWidget>
#include <QString>
#include <memory>

namespace Ui {
class projectDirSettingsDlg;
}

class ProjectSettings;

//////////////////////////////////////////////////////////////////////////
// ProjectDirSettings class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The ProjectDirSettings class is a widget to set the project directory settings.
 **/
class ProjectDirSettings : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit ProjectDirSettings(ProjectSettings *parent);
    virtual ~ProjectDirSettings();

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Applies the changes made in the project settings.
     **/
    void applyChanges() const;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   Slot triggered when the root directory browse button is clicked.
     **/
    void onRootDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the source directory browse button is clicked.
     **/
    void onSourceDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the include directory browse button is clicked.
     **/
    void onIncludeDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the delivery directory browse button is clicked.
     **/
    void onDeliveryDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the log directory browse button is clicked.
     **/
    void onLogDirBrowseBtnClicked();

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Connects the signal handlers.
     **/
    void connectSignalHandlers() const;
 
    /**
     * \brief   Initializes the paths with the current workspace data.
     **/
    void initializePathsWithCurrentWorkspaceData() const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    std::unique_ptr<Ui::projectDirSettingsDlg> mUi;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    ProjectDirSettings(const ProjectDirSettings & /*src*/) = delete;
    ProjectDirSettings& operator = (const ProjectDirSettings & /*src*/) = delete;
    ProjectDirSettings(ProjectDirSettings && /*src*/) noexcept = delete;
    ProjectDirSettings& operator = (ProjectDirSettings && /*src*/) noexcept = delete;
};

#endif // LUSAN_VIEW_COMMON_PROJECTDIRSETTINGS_HPP
