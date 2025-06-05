#ifndef LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
#define LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
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
 *  \file        lusan/view/common/NaviFileSystem.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the workspace related file system.
 *
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QList>
#include <QString>
#include <QWidget>

class FileSystemModel;
class FileSystemFilter;
class GeneralFileSystemModel;
class MdiMainWindow;
class TableCell;
class QFileInfo;
class QItemSelection;
class QTreeView;
class QToolButton;
class WorkspaceEntry;

namespace Ui {
    class NaviFileSystem;
}

//////////////////////////////////////////////////////////////////////////
// NaviFileSystem class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The NaviFileSystem class is a view of the workspace related file system.
 *          The class is used to display the file system in the workspace.
 **/
class NaviFileSystem    : public    NavigationWindow
                        , protected IETableHelper
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:
    using RootPaths = QMap<QString, QString>;
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NaviFileSystem(MdiMainWindow* wndMain, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// public methods
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the file system tree view control.
     **/
    QTreeView* ctrlFileSystem(void) const;

    /**
     * \brief   Returns the refresh tool button control.
     **/
    QToolButton* ctrlToolRefresh(void) const;

    /**
     * \brief   Returns the show all tool button control.
     **/
    QToolButton* ctrlToolShowAll(void) const;

    /**
     * \brief   Returns the collapse all tool button control.
     **/
    QToolButton* ctrlToolCollapse(void) const;

    /**
     * \brief   Returns the new folder tool button control.
     **/
    QToolButton* ctrlToolNewFolder(void) const;

    /**
     * \brief   Returns the new file tool button control.
     **/
    QToolButton* ctrlToolNewFile(void) const;

    /**
     * \brief   Returns the open tool button control.
     **/
    QToolButton* ctrlToolOpen(void) const;

    /**
     * \brief   Returns the edit tool button control.
     **/
    QToolButton* ctrlToolEdit(void) const;

    /**
     * \brief   Returns the delete tool button control.
     **/
    QToolButton* ctrlToolDelete(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Returns the number of columns in the table.
     **/
    virtual int getColumnCount(void) const override;

    /**
     * \brief   Returns the text of the cell.
     * \param   cell    The index of the cell.
     **/
    virtual QString getCellText(const QModelIndex& cell) const override;

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
     * \brief   Triggered when the tree view is double clicked.
     * \param   index   The index of the tree view.
     **/
    void onTreeViewDoubleClicked(const QModelIndex &index);

    /**
     * \brief   Triggered when the tree view is activated.
     * \param   index   The index of the tree view.
     **/
    void onTreeViewActivated(const QModelIndex &index);

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

    /**
     * \brief   Updates the data of the file system.
     **/
    void updateData(void);

    /**
     * \brief   Initializes the widgets.
     **/
    void setupWidgets(void);

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);

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

private slots:

    /**
     * \brief   Triggered when the workspace directories are changed.
     * \param   workspace   The workspace entry, which directories have being changed.
     *                      Normally, it is the directory of the active workspace.
     **/
    void onWorkspaceDirectoriesChanged(const WorkspaceEntry& workspace);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    FileSystemModel*        mNaviModel;     //!< The model of the file system.
    GeneralFileSystemModel* mGenModel;      //!< The general model of the file system.
    FileSystemFilter*       mFileFilter;    //!< The file filter object.
    Ui::NaviFileSystem*     ui;             //!< The user interface object.
    RootPaths               mRootPaths;     //!< The list of root paths.
    TableCell*              mTableCell;     //!< The table cell object.
};

#endif  // LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
