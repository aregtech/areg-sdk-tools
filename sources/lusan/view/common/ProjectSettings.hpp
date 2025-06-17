#ifndef LUSAN_VIEW_COMMON_PROJECTSETTINGS_HPP
#define LUSAN_VIEW_COMMON_PROJECTSETTINGS_HPP
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
 *  \file        lusan/view/common/ProjectSettings.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, options dialog.
 *
 ************************************************************************/

#include <QDialog>
#include <QStackedWidget>
#include <QStringListModel>
#include <memory>

/************************************************************************
 * Dependencies 
 ************************************************************************/
namespace Ui {
class ProjectSettingsDlg;
}

class QAbstractButton;
class ProjectDirSettings;
class LogSettings;
class WorkspaceManager;
class MdiMainWindow;

//////////////////////////////////////////////////////////////////////////
// ProjectSettings class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The ProjectSettings class is a dialog to set the project settings.
 **/
class ProjectSettings : public QDialog
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Internal constants and types
//////////////////////////////////////////////////////////////////////////
public:

    //!< The enumeration of the option pages.
    enum class eOptionPage : int
    {
          PageUndefined     = -1//!< Undefined page, used for error checking
        , PageProjectDirs   = 0 //!< Page for project directories settings
        , PageWorkspace         //!< Page for workspace settings
        , PageLogging           //!< Page for logging settings
        
        , PageCount             //!< Total number of pages
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit ProjectSettings(MdiMainWindow *parent);
    virtual ~ProjectSettings(void);
    
//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the pointer to project settings widget.
     **/
    inline ProjectDirSettings * getSettingProjectDirs(void);

    /**
     * \brief   Returns the pointer to workspace settings widget.
     **/
    inline WorkspaceManager* getSettingWorkspace(void);

    /**
     * \brief   Returns the pointer to log settings widget.
     **/
    inline LogSettings* getSettingLog(void);

    /**
     * \brief   Activates the page in the settings dialog.
     * \param   page    The page to activate.
     **/
    void activatePage(eOptionPage page);
    
//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Slot triggered when the selection of the settings list changed.
     * \param   index   The index of the selected item.
     **/
    void settingsListSelectionChanged(QModelIndex const&);

    /**
     * \brief   Slot triggered when the user clicked on the button.
     * \param   button  The clicked button.
     **/
    void buttonClicked(QAbstractButton*) const;

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Initializes the dialog.
     **/
    void setupDialog();
    /**
     * \brief   Initializes the signals.
     **/
    void connectSignals() const;
    /**
     * \brief   Add settings data.
     **/
    void addSettings();
    /**
     * \brief   Select the setting.
     * \param   index   The index of the setting.
     **/
    void selectSetting(int index) const;

    void selectPage(int index) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    std::unique_ptr<Ui::ProjectSettingsDlg> mUi;            //!< The user interface object.
    std::unique_ptr<QStackedWidget> mSettingsStackedWidget; //!< The stacked widget to show the settings.
    MdiMainWindow*      mMainWindow;                        //!< The main window of the application.
    QStringListModel    mModel;                             //!< The model of the settings list.
    ProjectDirSettings* mDirSettings;                       //!< The directory settings.
    WorkspaceManager*   mWorkspaceManager;                  //!< The workspace settings.
    LogSettings*        mLogSettings;                       //!< The log settings.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    ProjectSettings(const ProjectSettings & /*src*/) = delete;
    ProjectSettings& operator = (const ProjectSettings & /*src*/) = delete;
    ProjectSettings(ProjectSettings && /*src*/) noexcept = delete;
    ProjectSettings& operator = (ProjectSettings && /*src*/) noexcept = delete;
};

//////////////////////////////////////////////////////////////////////////
// ProjectSettings inline methods
//////////////////////////////////////////////////////////////////////////

inline ProjectDirSettings * ProjectSettings::getSettingProjectDirs(void)
{
    return mDirSettings;
}

inline WorkspaceManager * ProjectSettings::getSettingWorkspace(void)
{
    return mWorkspaceManager;
}

inline LogSettings * ProjectSettings::getSettingLog(void)
{
    return mLogSettings;
}

#endif // LUSAN_VIEW_COMMON_PROJECTSETTINGS_HPP
