#ifndef LUSAN_MODEL_COMMON_OPTIONSMANAGER_HPP
#define LUSAN_MODEL_COMMON_OPTIONSMANAGER_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/OptionsManager.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include <algorithm>
#include <list>
#include <cstdint>
#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NETimeUnits.hpp"
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
    //!< The height of one log table row when the user has not chosen one. A dense table
    //!< shows about a third more rows than a comfortable one, and how many rows are on
    //!< screen is what makes a burst or a gap visible without scrolling.
    static constexpr int    LogRowHeightDefault { 21 };
    //!< The range a chosen row height is clamped into.
    static constexpr int    LogRowHeightMin     { 16 };
    static constexpr int    LogRowHeightMax     { 48 };

    enum class eAppTheme : uint8_t
    {
          SystemDefault = 0
        , ModernLight
        , ModernDark
        , MidnightBlue
        , Nord
    };

    /**
     * \brief   Constructor.
     **/
    OptionsManager();

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
    inline uint64_t getActiveKey() const;

    /**
     * \brief   Gets the list of workspace entries.
     * \return  The list of workspace entries.
     **/
    inline const std::vector<WorkspaceEntry>& getWorkspaceList() const;

    /**
     * \brief   Adds a new workspace entry.
     * \param   root        The root directory of the workspace.
     * \param   name        The short name of the workspace. Empty takes the root directory name.
     * \param   description The description of the workspace.
     * \return  The added workspace entry.
     **/
    WorkspaceEntry addWorkspace(const QString& root, const QString& name, const QString& description);

    /**
     * \brief   Checks whether a workspace with the given name already exists.
     * \param   name        The name to check. The comparison ignores the letter case.
     * \param   exceptId    The identifier of a workspace to skip, so an entry does not clash
     *                      with itself while it is edited. Zero checks every entry.
     * \return  True if another workspace already carries the name.
     **/
    bool existsWorkspaceName(const QString& name, uint32_t exceptId = 0) const;
    
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
    WorkspaceEntry getActiveWorkspace() const;

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
    bool readOptions();

    /**
     * \brief   Writes options to the options file.
     **/
    void writeOptions();

    /**
     * \brief   Returns true if there is a default workspace set.
     **/
    bool hasDefaultWorkspace() const;

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
    uint64_t getDefaultWorkspaceId() const;

    /**
     * \brief   Returns the default workspace root directory. Returns an empty string if no default workspace is set.
     **/
    const QString& getDefaultWorkspaceRoot() const;

    /**
     * \brief   Returns the default workspace entry. Returns an invalid workspace entry if no default workspace is set.
     **/
    const WorkspaceEntry & getDefaultWorkspace() const;

    /**
     * \brief   Activates the default workspace entry, if there is any, and returns the activation key.
     *          Returns 0 if no default workspace entry is set.
     **/
    uint64_t activateDefaultWorkspace();

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
    bool setDefaultWorkspace(const QString& defWorkspaceRoot);

    /**
     * \brief   Checks if the given ID is the ID of active workspace.
     **/
    bool isActiveWorkspace(uint32_t id) const;

    /**
     * \brief   Returns currently configured application theme.
     **/
    inline eAppTheme getTheme() const;

    /**
     * \brief   Sets currently configured application theme.
     **/
    inline void setTheme(eAppTheme theme);

    /**
     * \brief   Returns the unit the measured times are shown in.
     **/
    inline NETimeUnits::eTimeUnit getTimeUnit() const;

    /**
     * \brief   Sets the unit the measured times are shown in.
     **/
    inline void setTimeUnit(NETimeUnits::eTimeUnit unit);

    /**
     * \brief   Returns the colour set the log rows are drawn with.
     **/
    inline NELogPalette::eLogPalette getLogPalette() const;

    /**
     * \brief   Sets the colour set the log rows are drawn with.
     **/
    inline void setLogPalette(NELogPalette::eLogPalette palette);

    /**
     * \brief   Returns the height of one row of a log table, in device independent pixels.
     **/
    inline int getLogRowHeight() const;

    /**
     * \brief   Sets the height of one row of a log table. A value outside the allowed
     *          range is clamped into it.
     **/
    inline void setLogRowHeight(int height);

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
     * \brief   Reads the application theme setting from XML.
     **/
    void _readTheme(QXmlStreamReader& xml);

    /**
     * \brief   Reads the time unit setting from XML.
     **/
    void _readTimeUnit(QXmlStreamReader& xml);

    /**
     * \brief   Reads the log colour set setting from XML.
     **/
    void _readLogPalette(QXmlStreamReader& xml);

    /**
     * \brief   Reads the log row height setting from XML.
     **/
    void _readLogRowHeight(QXmlStreamReader& xml);

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
    eAppTheme   mTheme;         //!< Configured application theme.
    NETimeUnits::eTimeUnit mTimeUnit;   //!< The unit the measured times are shown in.
    NELogPalette::eLogPalette mLogPalette;   //!< The colour set the log rows are drawn with.
    int         mLogRowHeight;  //!< The height of one row of a log table.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint64_t OptionsManager::getActiveKey() const
{
    return mActiveKey;
}

inline const std::vector<WorkspaceEntry>& OptionsManager::getWorkspaceList() const
{
    return mWorkspaces;
}

inline void OptionsManager::_sort()
{
    std::sort(mWorkspaces.begin(), mWorkspaces.end(), std::greater<WorkspaceEntry>());
}

inline OptionsManager::eAppTheme OptionsManager::getTheme() const
{
    return mTheme;
}

inline void OptionsManager::setTheme(eAppTheme theme)
{
    mTheme = theme;
}

inline NETimeUnits::eTimeUnit OptionsManager::getTimeUnit() const
{
    return mTimeUnit;
}

inline NELogPalette::eLogPalette OptionsManager::getLogPalette() const
{
    return mLogPalette;
}

inline void OptionsManager::setLogPalette(NELogPalette::eLogPalette palette)
{
    mLogPalette = palette;
}

inline int OptionsManager::getLogRowHeight() const
{
    return mLogRowHeight;
}

inline void OptionsManager::setLogRowHeight(int height)
{
    mLogRowHeight = std::clamp(height, OptionsManager::LogRowHeightMin, OptionsManager::LogRowHeightMax);
}

inline void OptionsManager::setTimeUnit(NETimeUnits::eTimeUnit unit)
{
    mTimeUnit = unit;
}

#endif // LUSAN_MODEL_COMMON_OPTIONSMANAGER_HPP
