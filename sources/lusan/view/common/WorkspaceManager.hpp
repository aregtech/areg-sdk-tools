﻿#ifndef LUSAN_VIEW_COMMON_WORKSPACEMANAGER_HPP
#define LUSAN_VIEW_COMMON_WORKSPACEMANAGER_HPP
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
 *  \file        lusan/view/common/WorkspaceManager.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, workspace manager widget.
 *
 ************************************************************************/
#include <QWidget>
#include <QString>
#include <unordered_map>
#include <cstdint>
#include <optional>

namespace Ui {
class workspaceManager;
}
class WorkspaceEntry;

/**
 * \brief   This class is managing the workspace settings.
 **/
class WorkspaceManager : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   The structure to store the workspace change data.
     **/
    struct WorkspaceChangeData
    {
        bool                    hasDeleted{ false };    //!< Flag, indicating that the workspace is deleted.
        std::optional<QString>  newDescription{};       //!< Description of the workspace
    };

    //!< Workspace ID
    using WorkspaceId = uint32_t;
    //!< The map of modified workspaces.
    using MapModifiedWorkspaces = std::unordered_map<WorkspaceId, WorkspaceChangeData>;

//////////////////////////////////////////////////////////////////////////
// constructors / destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit WorkspaceManager(QWidget *parent = nullptr);
    virtual ~WorkspaceManager();

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Applies the changes made in the workspace settings.
     **/
    void applyChanges();

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Triggered when delete button is clicked.
     **/
    void onDeleteButtonClicked();

    /**
     * \brief   Triggered when workspace list selection has been changed.
     **/
    void onWorkspaceSelectionChanged() const;

    /**
     * \brief   Triggered when workspace description has been changed.
     **/
    void onWorkspaceDescChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns the workspace entry by given workspace ID.
     * \param   workspaceId     The workspace ID to search.
     * \return  Returns the workspace entry if found, otherwise returns empty optional.
     **/
    static std::optional<WorkspaceEntry> getWorkspace(uint32_t workspaceId);

    /**
     * \brief   Connects signals.
     **/
    void connectSignalHandlers() const;

    /**
     * \brief   Initializes the paths with selected workspace data.
     * \param   workspaceId     The workspace ID to initialize the paths.
     **/
    void initializePathsWithSelectedWorkspaceData(uint32_t workspaceId) const;

    /**
     * \brief   Populates the list of workspaces.
     **/
    void populateListOfWorkspaces() const;

    /**
     * \brief   Sets up the user interface.
     **/
    void setupUi() const;

    /**
     * \brief   Returns the selected workspace ID.
     **/
    std::optional<uint32_t> getSelectedWorkspaceId() const;

    /**
     * \brief   Sets selected the workspace item by given index.
     **/
    void selectWorkspace(int index) const;

    /**
     * \brief   Deletes the selected workspace item.
     **/
    void deleteSelectedWorkspaceItem() const;


//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    Ui::workspaceManager* ui{}; //!< The user interface object.
    MapModifiedWorkspaces mModifiedWorkspaces; //!< The map of modified workspaces.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    WorkspaceManager(WorkspaceManager const&) = delete;
    WorkspaceManager& operator=(WorkspaceManager const&) = delete;
};

#endif // LUSAN_VIEW_COMMON_WORKSPACEMANAGER_HPP
