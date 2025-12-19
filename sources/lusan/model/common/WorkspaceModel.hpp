#ifndef LUSAN_MODEL_COMMON_WORKSPACEMODEL_HPP
#define LUSAN_MODEL_COMMON_WORKSPACEMODEL_HPP
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
 *  \file        lusan/model/common/WorkspaceModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include <QAbstractListModel>
#include <vector>
#include "lusan/data/common/WorkspaceEntry.hpp"

class OptionsManager;

/**
 * \class   WorkspaceModel
 * \brief   Represents a model for workspace items to be used in a QComboBox.
 **/
class WorkspaceModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    /**
     * \brief   Constructor.
     * \param   options The options manager.
     * \param   parent  The parent QObject.
     **/
    explicit WorkspaceModel(OptionsManager & options, QObject* parent = nullptr);
    
    /**
     * \brief   Destructor.
     **/
    virtual ~WorkspaceModel(void);

    /**
     * \brief   Adds a workspace item to the model.
     * \param   item    The workspace item to add.
     **/
    void addWorkspaceEntry(const WorkspaceEntry& item);
    
    /**
     * \brief   Adds a workspace item to the model.
     * \param   root        The root directory of the workspace.
     * \param   describe    The description of the workspace.
     * \return  The added workspace entry.
     **/
    WorkspaceEntry addWorkspaceEntry(const QString& root, const QString& describe);
    
    /**
     * \brief   Finds a workspace entry by root directory.
     * \param   root    The root directory of the workspace.
     * \return  The found workspace entry.
     **/
    const WorkspaceEntry & findWorkspaceEntry(const QString& root) const;

    /**
     * \brief   Removes the entry with specified unique workspace directory.
     * \param   root    The root directory of the workspace
     **/
    void removeWorkspaceEntry(const QString& root);

    /**
     * \brief   Finds the index of a workspace entry by root directory.
     * \param   root    The root directory of the workspace.
     * \return  The index of the workspace entry.
     **/
    int find(const QString& root) const;
    
    /**
     * \brief   Finds the index of a workspace entry by key.
     * \param   key     The key of the workspace.
     * \return  The index of the workspace entry.
     **/
    int find(const uint64_t key) const;

    /**
     * \brief   Returns the number of rows in the model.
     * \param   parent  The parent QModelIndex.
     * \return  The number of rows.
     **/
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * \brief   Returns the data for a given index and role.
     * \param   index   The index of the item.
     * \param   role    The role for which data is requested.
     * \return  The data for the given index and role.
     **/
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * \brief   Returns the role names for the model.
     * \return  A hash of role names.
     **/
    virtual QHash<int, QByteArray> roleNames() const override;
    
    /**
     * \brief   Gets the list of workspace entries.
     * \return  The list of workspace entries.
     **/
    inline const std::vector<WorkspaceEntry>& getEntries(void) const;
    
    /**
     * \brief   Gets the workspace entry at the specified row.
     * \param   row     The row index.
     * \return  The workspace entry at the specified row.
     **/
    const WorkspaceEntry& getData(uint32_t row) const;
    
    /**
     * \brief   Activates the workspace entry at the specified row.
     * \param   row     The row index.
     * \return  The updated last accessed timestamp.
     **/
    uint64_t activate(uint32_t row);
    
    /**
     * \brief   Activates the workspace entry by root directory.
     * \param   root    The root directory of the workspace.
     * \return  The updated last accessed timestamp.
     **/
    uint64_t activate(const QString & root);

    /**
     * \brief   Returns true if the workspace has new entry.
     **/
    inline bool hasNewWorkspace(void) const;

    /**
     * \brief   Returns the directory of the new workspace entry.
     *          Returns empty string if there is no new workspace entry.
     **/
    inline const WorkspaceEntry& getNewWorkspace(void) const;

    /**
     * \brief   Checks if the workspace root is the default workspace.
     * \param   root    The workspace root to check.
     * \return  True if the workspace root is the default workspace, false otherwise.
     **/
    bool isDefaultWorkspace(const QString& root) const;

    /**
     * \brief   Checks if the workspace entry row is the default workspace.
     * \param   row     The workspace entry in the row to check.
     * \return  True if the workspace entry row is the default workspace, false otherwise.
     **/
    bool isDefaultWorkspace(uint32_t row) const;

    /**
     * \brief   Returns true if there is a default workspace set.
     **/
    bool hasDefaultWorkspace(void) const;

    /**
     * \brief   Activates the default workspace entry, if there is any, and returns the activation key.
     *          Returns 0 if no default workspace entry is set.
     **/
    uint64_t activateDefaultWorkspace(void);

    /**
     * \brief   Returns the default workspace entry. Returns an invalid workspace entry if no default workspace is set.
     **/
    const WorkspaceEntry& getDefaultWorkspace(void) const;
    
    /**
     * \brief   Sets the default workspace by root directory.
     * \param   root    The root directory of the default workspace to set.
     *                  Unsets the default workspace if the root is empty or does not exist.
     * \return  True if the default workspace was successfully set, false otherwise.
     **/
    bool setDefaultWorkspace(const QString & root);
        
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Finds a workspace entry by ID.
     * \param   id  The ID of the workspace.
     * \return  Returns valid index if the workspace entry found. Otherwise, returns -1.
     **/
    inline int _findById(uint32_t id) const;
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    std::vector<WorkspaceEntry> mItems;         //!< The list of workspace items.
    WorkspaceEntry              mNewItem;       //!< The new workspace item;
    uint32_t                    mDefWorkspace;  //!< The default workspace ID;
};

//////////////////////////////////////////////////////////////////////////
// WorkspaceModel inline methods
//////////////////////////////////////////////////////////////////////////

inline const std::vector<WorkspaceEntry>& WorkspaceModel::getEntries(void) const
{
    return mItems;
}

inline bool WorkspaceModel::hasNewWorkspace(void) const
{
    return mNewItem.isValid();
}

inline const WorkspaceEntry& WorkspaceModel::getNewWorkspace(void) const
{
    return mNewItem;
}

#endif  // LUSAN_MODEL_COMMON_WORKSPACEMODEL_HPP
