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
 *  \file        lusan/view/common/WorkspaceFileDialog.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the custom File Dialog to display only workspace specific file system.
 *
 ************************************************************************/
#include "lusan/view/common/WorkspaceFileDialog.hpp"

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

QString WorkspaceFileDialog::getSelectedFileRelativePath(void) const
{
    QStringList selected{ selectedFiles() };
    QString result = selected.first();
    QDir rootDir(directoryUrl().path());
    
    if (result.startsWith(rootDir.absolutePath()))
    {
        result = rootDir.relativeFilePath(result);
    }
    else
    {
        for (const QString& root : mRootDirectories)
        {
            if (result.startsWith(root))
            {
                result = QDir(root).relativeFilePath(result);
                break;
            }
        }
    }
    
    return result;
}

QString WorkspaceFileDialog::getSelectedFilePath(void) const
{
    QStringList selected{ selectedFiles() };
    return selected.first();
}

void WorkspaceFileDialog::clearHistory()
{
    setHistory(QStringList());
}
