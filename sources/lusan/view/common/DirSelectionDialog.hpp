#ifndef LUSAN_VIEW_COMMON_DIRSELECTIONDIALOG_HPP
#define LUSAN_VIEW_COMMON_DIRSELECTIONDIALOG_HPP
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
 *  \file        lusan/view/common/DirSelectionDialog.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include <QDialog>
#include <QDir>

class QTreeView;
class QFileSystemModel;
class QLineEdit;
class QPushButton;

class DirSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    DirSelectionDialog(QWidget* parent = nullptr);
        
    DirSelectionDialog(const QString & curDir, QWidget* parent = nullptr);
    
public:
    QDir getDirectory() const;

protected:
    virtual void onCurrentDirChanged();
    
private:
    
    void _initialize(const QString & curDir);
    
private:
    QTreeView* mTreeViewDirs;
    QFileSystemModel* mModel;
    QLineEdit* mDirName;
    QPushButton* mButtonOK;
}; 

#endif  // LUSAN_VIEW_COMMON_DIRSELECTIONDIALOG_HPP
