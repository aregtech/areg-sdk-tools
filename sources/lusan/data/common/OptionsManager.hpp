#ifndef LUSAN_MODEL_COMMON_OPTIONSMANAGER_HPP
#define LUSAN_MODEL_COMMON_OPTIONSMANAGER_HPP
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
 *  \file        lusan/data/common/OptionsManager.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include <list>
#include "lusan/data/common/WorkspaceEntry.hpp"

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * \class   OptionsManager
 * \brief   Manages application options, including workspace entries.
 **/
class OptionsManager    : public QObject
{
    using Workspaces = std::vector<WorkspaceEntry>;

    Q_OBJECT
public:
    /**
     * \brief   Constructor.
     **/
    OptionsManager(void);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   The signal is triggered when the project directories are changed.
     * \param   workspace           The workspace entry.
     * \param   isActiveWorkspace   Indicates if the workspace is the active one.
     **/
    void signalWorkspaceDirectoriesChanged(const WorkspaceEntry& workspace, bool isActiveWorkspace);

public:
    /**
     * \brief   Gets the active workspace key.
     * \return  The active workspace key.
     **/
    inline uint64_t getActiveKey(void) const;

    /**
     * \brief   Gets the list of workspace entries.
     * \return  The list of workspace entries.
     **/
    inline const std::vector<WorkspaceEntry>& getWorkspaceList(void) const;

    /**
     * \brief   Adds a new workspace entry.
     * \param   root        The root directory of the workspace.
     * \param   description The description of the workspace.
     * \return  The added workspace entry.
     **/
    WorkspaceEntry addWorkspace(const QString& root, const QString& description);
    
    /**
     * \brief   Adds an existing workspace entry.
     * \param   workspace   The workspace entry to add.
     **/
    void addWorkspace(const WorkspaceEntry & workspace);

    /**
     * \brief   Updates an entry of the workspace.
     * \param   workspace   The workspace entry to update.
     * \return  True if the workspace was successfully updated, false otherwise.
     **/
    bool updateWorkspace(const WorkspaceEntry & workspace);

    /**
     * \brief   Removes a workspace entry by key.
     * \param   key         The key of the workspace to remove.
     * \return  The removed workspace entry.
     **/
    WorkspaceEntry removeWorkspace(uint64_t key);

    /**
     * \brief   Activates a workspace by key and description.
     * \param   key         The key of the workspace to activate.
     * \param   description The description of the workspace.
     * \return  The activated workspace entry.
     **/
    WorkspaceEntry activateWorkspace(uint64_t key, const QString& description);

    /**
     * \brief   Activates a workspace by root directory and description.
     * \param   root        The root directory of the workspace.
     * \param   description The description of the workspace.
     * \return  The activated workspace entry.
     **/
    WorkspaceEntry activateWorkspace(const QString& root, const QString& description);

    /**
     * \brief   Gets the active workspace entry.
     * \return  The active workspace entry.
     **/
    WorkspaceEntry getActiveWorkspace(void) const;

    /**
     * \brief   Checks if a workspace exists by root directory.
     * \param   root        The root directory of the workspace.
     * \return  True if the workspace exists, false otherwise.
     **/
    bool existsWorkspace(const QString& root) const;

    /**
     * \brief   Reads options from the options file.
     * \return  True if the options were successfully read, false otherwise.
     **/
    bool readOptions(void);

    /**
     * \brief   Writes options to the options file.
     **/
    void writeOptions(void);

    /**
     * \brief   Returns true if there is a default workspace set.
     **/
    bool hasDefaultWorkspace(void) const;

    /**
     * \brief   Checks if the workspace ID is the default workspace.
     * \param   workspaceId The workspace ID to check.
     * \return  True if the workspace ID is the default workspace, false otherwise.
     **/
    bool isDefaultWorkspace(uint64_t workspaceId) const;

    /**
     * \brief   Checks if the workspace root is the default workspace.
     * \param   workspaceRoot   The workspace root to check.
     * \return  True if the workspace root is the default workspace, false otherwise.
     **/
    bool isDefaultWorkspace(const QString& workspaceRoot) const;

    /**
     * \brief   Returns the default workspace ID. Returns 0 if no default workspace is set.
     **/
    uint64_t getDefaultWorkspaceId(void) const;

    /**
     * \brief   Returns the default workspace root directory. Returns an empty string if no default workspace is set.
     **/
    const QString& getDefaultWorkspaceRoot(void) const;

    /**
     * \brief   Returns the default workspace entry. Returns an invalid workspace entry if no default workspace is set.
     **/
    const WorkspaceEntry & getDefaultWorkspace(void) const;

    /**
     * \brief   Activates the default workspace entry, if there is any, and returns the activation key.
     *          Returns 0 if no default workspace entry is set.
     **/
    uint64_t activateDefaultWorkspace(void);

    /**
     * \brief   Sets the default workspace by ID.
     * \param   defWorkspaceId  The ID of the default workspace to set.
     *                          Unsets the default workspace if the ID is 0 or does not exist.
     * \return  True if the default workspace was successfully set, false otherwise.
     **/
    bool setDefaultWorkspace(uint64_t defWorkspaceId);

    /**
     * \brief   Sets the default workspace by root directory.
     * \param   defWorkspaceRoot The root directory of the default workspace to set.
     *                           Unsets the default workspace if the root is empty or does not exist.
     * \return  True if the default workspace was successfully set, false otherwise.
     **/
    bool setDefaultWorkspace(const QString defWorkspaceRoot);

    /**
     * \brief   Checks if the given ID is the ID of active workspace.
     **/
    bool isActiveWorkspace(uint32_t id) const;

private:
    /**
     * \brief   Reads the option list from an XML stream.
     * \param   xml         The XML stream reader.
     **/
    void _readOptionList(QXmlStreamReader& xml);

    /**
     * \brief   Reads an option from an XML stream.
     * \param   xml         The XML stream reader.
     **/
    void _readOption(QXmlStreamReader& xml);

    /**
     * \brief   Reads the workspace list from an XML stream.
     * \param   xml         The XML stream reader.
     **/
    void _readWorkspaceList(QXmlStreamReader& xml);

    /**
     * \brief   Finds a workspace by root directory.
     * \param   root        The root directory of the workspace.
     * \return  The found workspace entry.
     **/
    const WorkspaceEntry& _findWorkspace(const QString& root) const;

    /**
     * \brief   Finds a workspace by ID.
     * \param   id          The ID of the workspace.
     * \return  The found workspace entry.
     **/
    const WorkspaceEntry& _findWorkspace(uint64_t id) const;

    /**
     * \brief   Sorts the workspace entries.
     **/
    inline void _sort();

    /**
     * \brief   Checks if a workspace ID exists in the list.
     * \param   workspaceId The workspace ID to check.
     * \return  True if the workspace ID exists, false otherwise.
     **/
    inline bool _existWorkspaceId(uint64_t workspaceId) const;

private:
    uint64_t    mActiveKey;     //!< The active workspace key.
    uint32_t    mDefWorkspace;  //!< The ID of default workspace.
    Workspaces  mWorkspaces;    //!< The list of workspace entries.
    uint32_t    mCurId;         //!< The current workspace ID.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint64_t OptionsManager::getActiveKey(void) const
{
    return mActiveKey;
}

inline const std::vector<WorkspaceEntry>& OptionsManager::getWorkspaceList(void) const
{
    return mWorkspaces;
}

inline void OptionsManager::_sort()
{
    std::sort(mWorkspaces.begin(), mWorkspaces.end(), std::greater<WorkspaceEntry>());
}

#endif // LUSAN_MODEL_COMMON_OPTIONSMANAGER_HPP
