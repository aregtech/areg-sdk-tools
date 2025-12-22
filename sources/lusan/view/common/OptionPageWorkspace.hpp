#ifndef LUSAN_VIEW_COMMON_OPTIONPAGEWORKSPACE_HPP
#define LUSAN_VIEW_COMMON_OPTIONPAGEWORKSPACE_HPP
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
 *  \file        lusan/view/common/OptionPageWorkspace.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, workspace manager widget.
 *
 ************************************************************************/
#include "lusan/view/common/OptionPageBase.hpp"

#include <QString>
#include <unordered_map>
#include <cstdint>
#include <optional>
#include <memory>

namespace Ui {
class OptionPageWorkspace;
}
class WorkspaceEntry;
class QDialog;
class QLineEdit;

/**
 * \brief   This class is managing the workspace settings.
 **/
class OptionPageWorkspace : public OptionPageBase
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
    explicit OptionPageWorkspace(QDialog *parent);
    virtual ~OptionPageWorkspace();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Call when the option should apply the changes.
     **/
    virtual void applyChanges(void) override;
    
    /**
     * \brief   Called when the workspace directories in option pages are updated.
     * \param   sources    The sources directory.
     * \param   includes   The includes directory.
     * \param   delivery   The delivery directory.
     * \param   logs       The logs directory.
     **/
    virtual void updateWorkspaceDirectories(const sWorkspaceDir& sources, const sWorkspaceDir& includes, const sWorkspaceDir& delivery, const sWorkspaceDir& logs) override;
    
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

    /**
     * \brief   Triggered when default workspace check-box is checked or unchecked.
     * \param   checked     True is checked. False, otherwise.
     **/
    void onDefaultChecked(bool checked);

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
    void connectSignalHandlers();

    /**
     * \brief   Initializes the paths with selected workspace data.
     * \param   workspaceId     The workspace ID to initialize the paths.
     **/
    void initializePathsWithSelectedWorkspaceData(uint32_t workspaceId) const;

    /**
     * \brief   Populates the list of workspaces.
     **/
    void populateListOfWorkspaces();

    /**
     * \brief   Sets up the user interface.
     **/
    void setupUi();

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
    
    //!< Returns root path edit object
    inline QLineEdit* ctrlRoot(void) const;

    //!< Returns sources path edit object
    inline QLineEdit* ctrlSources(void) const;

    //!< Returns includes path edit object
    inline QLineEdit* ctrlIncludes(void) const;

    //!< Returns delivery path edit object
    inline QLineEdit* ctrlDelivery(void) const;

    //!< Returns logs path edit object
    inline QLineEdit* ctrlLogs(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    std::unique_ptr<Ui::OptionPageWorkspace>    mUi;                    //!< The user interface object.
    MapModifiedWorkspaces                       mModifiedWorkspaces;    //!< The map of modified workspaces.
    QString                                     mSources;               //!< The sources directory of active workspace.
    QString                                     mIncludes;              //!< The includes directory of active workspace.
    QString                                     mDelivery;              //!< The delivery directory of active workspace.
    QString                                     mLogs;                  //!< The logs directory of active workspace.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OptionPageWorkspace(void) = delete;
    DECLARE_NOCOPY_NOMOVE(OptionPageWorkspace);
};

#endif // LUSAN_VIEW_COMMON_OPTIONPAGEWORKSPACE_HPP
