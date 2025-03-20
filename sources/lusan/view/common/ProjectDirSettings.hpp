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

namespace Ui {
class projectDirSettingsDlg;
}

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
    explicit ProjectDirSettings(QWidget *parent = nullptr);
    virtual ~ProjectDirSettings() override;

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the root directory.
     **/
    QString getRootDirectory() const;

    /**
     * \brief   Returns the source directory.
     **/
    QString getSourceDirectory() const;

    /**
     * \brief   Returns the include directory.
     **/
    QString getIncludeDirectory() const;

    /**
     * \brief   Returns the delivery directory.
     **/
    QString getDeliveryDirectory() const;

    /**
     * \brief   Returns the log directory.
     **/
    QString getLogDirectory() const;

    /**
     * \brief   Returns the workspace description.
     **/
    QString getWorkspaceDescription() const;

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
    void connectSignalHandlers() const;
    void initialisePathsWithCurrentWorkspaceData() const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    Ui::projectDirSettingsDlg* ui;

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
