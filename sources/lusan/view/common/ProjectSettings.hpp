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

#include "lusan/view/common/ProjectDirSettings.hpp"
#include <QDialog>
#include <QStackedWidget>
#include <QStringListModel>

namespace Ui {
class ProjectSettingsDlg;
}

class QAbstractButton;

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
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit ProjectSettings(QWidget *parent = nullptr);
    virtual ~ProjectSettings(void);
    
//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    
    void signalSettingsChanged(ProjectSettings & dlgSettings);

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

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::ProjectSettingsDlg* ui;                 //!< The user interface object.
    QStackedWidget*     mSettingsStackedWidget; //!< The stacked widget to show the settings.
    ProjectDirSettings* mDirSettings;           //!< The directory settings.
    QStringListModel    mModel;                 //!< The model of the settings list.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    ProjectSettings(const ProjectSettings & /*src*/) = delete;
    ProjectSettings& operator = (const ProjectSettings & /*src*/) = delete;
    ProjectSettings(ProjectSettings && /*src*/) noexcept = delete;
    ProjectSettings& operator = (ProjectSettings && /*src*/) noexcept = delete;
};

#endif // LUSAN_VIEW_COMMON_PROJECTSETTINGS_HPP
