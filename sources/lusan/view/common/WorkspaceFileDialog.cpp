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
 *  \file        lusan/view/common/WorkspaceFileDialog.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the custom File Dialog to display only workspace specific file system.
 *
 ************************************************************************/
#include "lusan/view/common/WorkspaceFileDialog.hpp"

#include "lusan/common/NELusanCommon.hpp"

QString WorkspaceFileDialog::relativeToRoots(const QString& absoluteFilePath, const QStringList& roots)
{
    return NELusanCommon::relativeToRoots(absoluteFilePath, roots);
}

WorkspaceFileDialog::WorkspaceFileDialog(bool openFile, bool openDir, const QString& caption, QWidget* parent /*= nullptr*/)
    : QFileDialog       (parent, caption)
    , mRootDirectories  ()
    , mFileFilters      ()
{
    // Set up the file dialog
    setOption(QFileDialog::DontUseNativeDialog, true);
    setViewMode(QFileDialog::ViewMode::List);
    setFilter(QDir::Filter::AllDirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    if (openDir)
    {
        setOptions(QFileDialog::Option::ShowDirsOnly | QFileDialog::Option::ReadOnly | QFileDialog::Option::DontUseNativeDialog);
        setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
        setFileMode(QFileDialog::FileMode::Directory);
    }
    else if (openFile)
    {
        setOptions(QFileDialog::Option::ReadOnly | QFileDialog::Option::DontUseNativeDialog);
        setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
        setFileMode(QFileDialog::FileMode::ExistingFile);
    }
    else
    {
        setOptions(QFileDialog::Option::DontUseNativeDialog);
        setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
        setFileMode(QFileDialog::AnyFile);
    }
}

WorkspaceFileDialog::WorkspaceFileDialog(bool openFile, bool openDir, const QStringList& roots, const QStringList& filters, const QString& caption, QWidget* parent)
    : QFileDialog       (parent, caption)
    , mRootDirectories  (roots)
    , mFileFilters      (filters)
{
    // Set up the file dialog
    setOption(QFileDialog::DontUseNativeDialog, true);
    setViewMode(QFileDialog::ViewMode::List);
    setFilter(QDir::Filter::AllDirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    
    if (openFile)
    {
        setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
        setFileMode(QFileDialog::FileMode::ExistingFile);
    }
    else if (openDir)
    {
        setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
        setFileMode(QFileDialog::FileMode::Directory);
    }
    else
    {
        setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
        setFileMode(QFileDialog::AnyFile);
    }
    
    setRootDirectories(roots);
    setFileFilters(filters);
}

void WorkspaceFileDialog::setRootDirectories(const QStringList& roots)
{
    mRootDirectories = roots;
    if (!roots.isEmpty())
    {
        QList<QUrl> urls;
        for (const QString & root : roots)
        {
            urls << QUrl::fromLocalFile(root);
        }
        
        setSidebarUrls(urls);
        QString rootPath = roots.at(0);
        setDirectory(rootPath);
        setDirectoryUrl(urls.at(0));
    }
    
    setHistory(QStringList());
}

void WorkspaceFileDialog::setFileFilters(const QStringList& filters)
{
    QString all(tr("All Files"));
    mFileFilters = filters;
    bool allFiles {false};
    for (const QString& entry : filters)
    {
        if (entry.startsWith(all))
        {
            allFiles = true;
            break;
        }
    }
    
    if (allFiles == false)
    {
        all += " (*.*)";
        mFileFilters.append(all);
    }
    
    if (!mFileFilters.isEmpty())
    {
        setNameFilters(mFileFilters);
    }
}

QString WorkspaceFileDialog::getSelectedFileRelativePath() const
{
    // Measured against the root, not against the folder the user navigated into: the document
    // stores this string, and two documents must spell one file the same way whatever route
    // their authors took through the tree.
    return WorkspaceFileDialog::relativeToRoots(getSelectedFilePath(), mRootDirectories);
}

QString WorkspaceFileDialog::getSelectedFilePath() const
{
    const QStringList selected{ selectedFiles() };
    return (selected.isEmpty() ? QString() : selected.first());
}

void WorkspaceFileDialog::clearHistory()
{
    setHistory(QStringList());
}
