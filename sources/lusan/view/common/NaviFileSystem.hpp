#ifndef LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
#define LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
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
 *  \file        lusan/view/common/NaviFileSystem.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the workspace related file system.
 *
 ************************************************************************/

#include "lusan/view/common/NaviToolbarWindow.hpp"
#include "lusan/view/common/TableCell.hpp"

#include "lusan/model/common/FileSystemEntry.hpp"

#include <QList>
#include <QString>
#include <QWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class FileSystemModel;
class FileSystemFilter;
class GeneralFileSystemModel;
class MdiMainWindow;
class TableCell;
class QComboBox;
class QFileInfo;
class QItemSelection;
class QTreeView;
class QToolButton;
class WorkspaceEntry;

//////////////////////////////////////////////////////////////////////////
// NaviFileSystem class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The NaviFileSystem class is a view of the workspace related file system.
 *          The class is used to display the file system in the workspace.
 **/
class NaviFileSystem    : public    NaviToolbarWindow
                        , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /** \brief   Constructor.
     * \param   wndMain     The main window of the application.
     * \param   parent      The parent widget.
     **/
    NaviFileSystem(MdiMainWindow* wndMain, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Returns the number of columns in the table.
     **/
    int getColumnCount() const override;

    /**
     * \brief   Returns the text of the cell.
     * \param   cell    The index of the cell.
     **/
    QString getCellText(const QModelIndex& cell) const override;

    /**
     * \brief   This method is called when the apply button in options dialog is pressed.
     *          It can be used to apply changes made in the options dialog.
     **/
    void optionApplied() override;

    /**
     * \brief   This method is called when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    void optionClosed(bool OKpressed) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Triggered when the refresh tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolRefreshClicked(bool checked);

    /**
     * \brief   Triggered when the show all tool button is toggled.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolShowAllToggled(bool checked);

    /**
     * \brief   Triggered when the collapse all tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolCollapseAllClicked(bool checked);

    /**
     * \brief   Triggered when the new folder tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolNewFolderClicked(bool checked);

    /**
     * \brief   Triggered when the new file tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolNewFileClicked(bool checked);

    /**
     * \brief   Triggered when the open selected tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolOpenSelectedClicked(bool checked);

    /**
     * \brief   Triggered when the edit selected tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolEditSelectedClicked(bool checked);

    /**
     * \brief   Triggered when the delete selected tool button is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolDeleteSelectedClicked(bool checked);

    /**
     * \brief   Triggered when the root path is clicked.
     * \param   checked     The flag indicating if the tool button is checked.
     **/
    void onToolNaviRootClicked(bool checked);
    
    /**
     * \brief   Triggered when the user asks to open the item under the cursor -- by
     *          double-clicking it or by pressing Enter on it with the keyboard. Both gestures
     *          arrive through QAbstractItemView::activated, which is the view's own "open this"
     *          signal; connecting `doubleClicked` instead is what used to leave the tree
     *          mouse-only (issue: no keyboard open).
     *
     *          Directories are left alone: Right and Left already unfold and fold them, and the
     *          view expands a double-clicked directory by itself.
     * \param   index   The index of the item to open.
     **/
    void onTreeViewOpenRequested(const QModelIndex &index);

    /**
     * \brief   Triggered when the tree view selection is changed.
     * \param   current     The current index of the tree view.
     * \param   previous    The previous index of the tree view.
     **/
    void onTreeSelectinoRowChanged(const QModelIndex &current, const QModelIndex &previous);
    
    /**
     * \brief   Triggered when the cell editor data is changed.
     * \param   index       The index of the cell.
     * \param   newValue    The new value of the cell.
     **/
    void onEditorDataChanged(const QModelIndex& index, const QString& newValue);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< Returns the refresh tool button control.
    inline QToolButton* ctrlToolRefresh() const;

    //!< Returns the show all tool button control.
    inline QToolButton* ctrlToolShowAll() const;

    //!< Returns the navigate from root (machine) tool button control.
    inline QToolButton* ctrlToolNaviRoot() const;

    //!< Returns the collapse all tool button control.
    inline QToolButton* ctrlToolCollapse() const;

    //!< Returns the new folder tool button control.
    inline QToolButton* ctrlToolNewFolder() const;

    //!< Returns the new file tool button control.
    inline QToolButton* ctrlToolNewFile() const;

    //!< Returns the open tool button control.
    inline QToolButton* ctrlToolOpen() const;

    //!< Returns the edit tool button control.
    inline QToolButton* ctrlToolEdit() const;

    //!< Returns the delete tool button control.
    inline QToolButton* ctrlToolDelete() const;

    /**
     * \brief   Enables or disables the toolbar buttons that act on one item, according to what
     *          the given index is. Driven both by the selection moving and by the mouse passing
     *          over a row.
     * \param   index   The index the buttons should describe.
     **/
    void updateToolButtons(const QModelIndex &index);

    /**
     * \brief   Updates the data of the file system.
     **/
    void updateData();

    /**
     * \brief   Creates the tool buttons of the workspace explorer.
     **/
    void setupToolbar();

    /**
     * \brief   Creates the workspace selector shown above the tool buttons.
     **/
    void setupWorkspaceSelector();

    /**
     * \brief   Fills the workspace selector with the known workspaces and marks the active one.
     **/
    void populateWorkspaces();

    /**
     * \brief   Moves the workspace selector back to the active workspace without switching.
     **/
    void restoreWorkspaceSelection();

    /**
     * \brief   Initializes the widgets.
     **/
    void setupWidgets();

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals();

    /**
     * \brief   Blocks the basic signals.
     * \param   block   If true, blocks the signals. Otherwise, unblocks the signals.
     **/
    void blockBasicSignals(bool block);

    /**
     * \brief   Returns the file information for the given index.
     * \param   index   The index of the item.
     * \return  The file information.
     **/
    QFileInfo getFileInfo(const QModelIndex & index) const;

    /**
     * \brief   Sets the root paths for the file system model.
     * \param   workspace   The workspace entry object that contains information of paths.
     * \return  The map of root paths.
     **/
    WorkspaceElem setupRootPaths(const WorkspaceEntry& workspace);

private slots:

    /**
     * \brief   Triggered when the workspace directories are changed.
     * \param   workspace           The workspace entry, which directories have being changed.
     *                              Normally, it is the directory of the active workspace.
     * \param   isActiveWorkspace   The flag indicating whether the given workspace is currently active or not.
     **/
    void onWorkspaceDirectoriesChanged(const WorkspaceEntry& workspace, bool isActiveWorkspace);

    /**
     * \brief   Triggered when the user picks an entry of the workspace selector.
     * \param   index   The index of the picked entry.
     **/
    void onWorkspaceSelected(int index);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    FileSystemModel*        mNaviModel;     //!< The model of the file system.
    GeneralFileSystemModel* mGenModel;      //!< The general model of the file system.
    FileSystemFilter*       mFileFilter;    //!< The file filter object.
    WorkspaceElem           mRootPaths;     //!< The list of root paths.
    TableCell*              mTableCell;     //!< The table cell object.
    QComboBox*              mWorkspaces;    //!< The selector of the active workspace.
    QToolButton*            mToolRefresh;   //!< The tool button to reload the file system tree.
    QToolButton*            mToolShowAll;   //!< The tool button to show every file or only the known ones.
    QToolButton*            mToolCollapse;  //!< The tool button to collapse all folders.
    QToolButton*            mToolNaviRoot;  //!< The tool button to switch to the file system of the machine.
    QToolButton*            mToolOpen;      //!< The tool button to open the selected file.
    QToolButton*            mToolNewFolder; //!< The tool button to create a new folder.
    QToolButton*            mToolNewFile;   //!< The tool button to create a new file.
    QToolButton*            mToolEdit;      //!< The tool button to rename the selected entry.
    QToolButton*            mToolDelete;    //!< The tool button to delete the selected entry.
};

//////////////////////////////////////////////////////////////////////////
// NaviFileSystem class inline methods
//////////////////////////////////////////////////////////////////////////

inline QToolButton* NaviFileSystem::ctrlToolRefresh() const
{
    return mToolRefresh;
}

inline QToolButton* NaviFileSystem::ctrlToolShowAll() const
{
    return mToolShowAll;
}

inline QToolButton* NaviFileSystem::ctrlToolNaviRoot() const
{
    return mToolNaviRoot;
}

inline QToolButton* NaviFileSystem::ctrlToolCollapse() const
{
    return mToolCollapse;
}

inline QToolButton* NaviFileSystem::ctrlToolNewFolder() const
{
    return mToolNewFolder;
}

inline QToolButton* NaviFileSystem::ctrlToolNewFile() const
{
    return mToolNewFile;
}

inline QToolButton* NaviFileSystem::ctrlToolOpen() const
{
    return mToolOpen;
}

inline QToolButton* NaviFileSystem::ctrlToolEdit() const
{
    return mToolEdit;
}

inline QToolButton* NaviFileSystem::ctrlToolDelete() const
{
    return mToolDelete;
}

#endif  // LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
