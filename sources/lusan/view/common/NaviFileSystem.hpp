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

#include <QList>
#include <QString>
#include <QWidget>

class FileSystemModel;
class FileSystemFilter;
class MdiMainWindow;
class QTreeView;
class QToolButton;

namespace Ui {
    class NaviFileSystem;
}

class NaviFileSystem : public QWidget
{
private:
    static constexpr uint32_t  MIN_WIDTH{280};
    static constexpr uint32_t  MIN_HEIGHT{280};
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NaviFileSystem(MdiMainWindow* mainFrame, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// public methods
//////////////////////////////////////////////////////////////////////////
public:

    QTreeView* ctrlFileSystem(void) const;

    QToolButton* ctrlToolRefresh(void) const;

    QToolButton* ctrlToolShowAll(void) const;

    QToolButton* ctrlToolCollapse(void) const;

    QToolButton* ctrlToolExpand(void) const;

    QToolButton* ctrlToolNewFolder(void) const;

    QToolButton* ctrlToolNewFile(void) const;

    QToolButton* ctrlToolOpen(void) const;

    QToolButton* ctrlToolDelete(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onToolRefreshClicked(bool checked);
    
    void onToolShowAllToggled(bool checked);
    
    void onToolCollapseAllClicked(bool checked);
    
    void onToolExpandAllClicked(bool checked);
    
    void onToolNewFolderClicked(bool checked);
    
    void onToolNewFileClicked(bool checked);
    
    void onToolOpenSelectedClicked(bool checked);
    
    void onToolDeleteSelectedClicked(bool checked);
    
    void onTreeViewCollapsed(const QModelIndex &index);
    
    void onTreeViewExpanded(const QModelIndex &index);
    
    void onTreeViewDoubleClicked(const QModelIndex &index);
    
    void onTreeViewCctivated(const QModelIndex &index);
    
private:
    void updateData(void);
    
    void setupWidgets(void);
    
    void setupSignals(void);
    
//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    MdiMainWindow*      mMainFrame;
    FileSystemModel*    mNaviModel;
    Ui::NaviFileSystem* ui;
};

#endif  // LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
