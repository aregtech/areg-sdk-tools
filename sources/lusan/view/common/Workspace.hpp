#ifndef LUSAN_VIEW_COMMON_WORKSPACE_HPP
#define LUSAN_VIEW_COMMON_WORKSPACE_HPP
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
 *  \file        lusan/view/common/Workspace.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Workspace setup dialog.
 *
 ************************************************************************/
#include <QDialog>
#include "lusan/model/common/WorkspaceModel.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"

class OptionsManager;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

/**
 * \class   Workspace
 * \brief   Represents the workspace setup dialog in the Lusan application.
 **/
class Workspace : public QDialog
{
    Q_OBJECT
public:
    /**
     * \brief   Constructor.
     * \param   options The options manager.
     * \param   parent  The parent widget.
     **/
    Workspace(OptionsManager & options, QWidget * parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~Workspace();

    /**
     * \brief   Returns true if the workspace has new entry.
     **/
    inline bool hasNewWorkspaceEntry() const;
    
protected slots:
    /**
     * \brief   Slot called when the dialog is accepted.
     **/
    void onAccept();
    
    /**
     * \brief   Slot called when the dialog is rejected.
     **/
    void onReject();
    
    /**
     * \brief   Slot called when the workspace path is changed.
     * \param   newText The new workspace path.
     **/
    void onWorskpacePathChanged(const QString & newText);
    
    /**
     * \brief   Slot called when the browse button is clicked.
     * \param   checked Indicates whether the button is checked.
     **/
    void onBrowseClicked(bool checked = true);
    
    /**
     * \brief   Slot called when the workspace index is changed.
     * \param   index   The new workspace index.
     **/
    void onWorskpaceIndexChanged(int index);
    
    /**
     * \brief   Slot called when the path selection is changed.
     * \param   topLeft     The top-left index of the selection.
     * \param   bottomRight The bottom-right index of the selection.
     * \param   roles       The roles of the selection.
     **/
    void onPathSelectionChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);
    
    /**
     * \brief   Triggered when default workspace check-box is checked or unchecked.
     * \param   checked     True is checked. False, otherwise.
     **/
    void onDefaultChecked(bool checked);

    /**
     * \brief   Slot called when the workspace name is edited.
     * \param   newText The new workspace name.
     **/
    void onWorkspaceNameChanged(const QString& newText);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Creates the controls of the dialog.
     **/
    void setupDialog();

    /**
     * \brief   Enables the OK button when the entered workspace can be used, and reports on the
     *          hint line what is still missing.
     **/
    void validateInput();

    /**
     * \brief   Shows the name of the given workspace, or the name derived from the root when the
     *          entry carries none, without reporting the change.
     **/
    void showWorkspaceName(const WorkspaceEntry& entry);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    OptionsManager &        mOptions;       //!< The options manager.
    WorkspaceModel          mModel;         //!< The model for workspace entries.
    QComboBox*              mRoot;          //!< The workspace root directory selector.
    QPushButton*            mBrowse;        //!< The button to browse for a workspace directory.
    QLineEdit*              mName;          //!< The short name of the workspace.
    QPlainTextEdit*         mDescription;   //!< The optional description of the workspace.
    QCheckBox*              mDefault;       //!< The flag to open this workspace without asking again.
    QLabel*                 mHint;          //!< The line reporting why the dialog cannot be accepted.
    QDialogButtonBox*       mButtons;       //!< The OK and Cancel buttons.
    bool                    mNameEdited;    //!< True when the name was typed and must not be overwritten.
};

//////////////////////////////////////////////////////////////////////////
// Workspace inline functions implementation
//////////////////////////////////////////////////////////////////////////
inline bool Workspace::hasNewWorkspaceEntry() const
{
    return mModel.hasNewWorkspace();
}

#endif // LUSAN_VIEW_COMMON_WORKSPACE_HPP
