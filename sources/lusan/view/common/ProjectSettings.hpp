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
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/ProjectSettings.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, options dialog.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include <QDialog>
#include <QStackedWidget>
#include <QStringListModel>
#include <memory>

/************************************************************************
 * Dependencies 
 ************************************************************************/
class QAbstractButton;
class OptionPageProjectDirs;
class OptionPageLogging;
class OptionPageWorkspace;
class MdiMainWindow;

namespace Ui {
    class ProjectSettingsDlg;
}

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
    inline OptionPageProjectDirs * getSettingProjectDirs(void);

    /**
     * \brief   Returns the pointer to workspace settings widget.
     **/
    inline OptionPageWorkspace* getSettingWorkspace(void);

    /**
     * \brief   Returns the pointer to log settings widget.
     **/
    inline OptionPageLogging* getSettingLog(void);

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
    void onSettingsListSelectionChanged(QModelIndex const&);

    /**
     * \brief   Slot triggered when the user clicked on the button.
     * \param   button  The clicked button.
     **/
    void onButtonClicked(QAbstractButton*);

    /**
     * \brief   Slot, triggered when OK button in the button box is clicked.
     **/
    void onAcceptClicked(void);
    
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

    /**
     * \brief   Activates certain option page by given index.
     **/
    void selectPage(int index) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    std::unique_ptr<Ui::ProjectSettingsDlg> mUi;            //!< The user interface object.
    std::unique_ptr<QStackedWidget> mSettingsStackedWidget; //!< The stacked widget to show the settings.
    MdiMainWindow*          mMainWindow;                    //!< The main window of the application.
    QStringListModel        mModel;                         //!< The model of the settings list.
    OptionPageProjectDirs*  mOptionProjectDirs;             //!< The directory settings.
    OptionPageWorkspace*    mOptionPageWorkspace;           //!< The workspace settings.
    OptionPageLogging*      mOptionPageLogging;             //!< The log settings.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(ProjectSettings);
};

//////////////////////////////////////////////////////////////////////////
// ProjectSettings inline methods
//////////////////////////////////////////////////////////////////////////

inline OptionPageProjectDirs * ProjectSettings::getSettingProjectDirs(void)
{
    return mOptionProjectDirs;
}

inline OptionPageWorkspace * ProjectSettings::getSettingWorkspace(void)
{
    return mOptionPageWorkspace;
}

inline OptionPageLogging * ProjectSettings::getSettingLog(void)
{
    return mOptionPageLogging;
}

#endif // LUSAN_VIEW_COMMON_PROJECTSETTINGS_HPP
