/************************************************************************
 * This file is part of the Lusan project, an official component of the Areg SDK.
 * Lusan is a graphical user interface (GUI) tool designed to support the development,
 * debugging, and testing of applications built with the Areg Framework.
 *
 * Lusan is available as free and open-source software under the Apache version 2.0 License,
 * providing essential features for developers.
 *
 * For detailed licensing terms, please refer to the LICENSE file included
 * with this distribution or contact us at info[at]areg.tech.
 *
 * \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 * \file        lusan/view/common/WorkspaceFileDialog.hpp
 * \ingroup     Lusan - GUI Tool for Areg SDK
 * \author      Artak Avetyan
 * \brief       Lusan application, the custom File Dialog to display only workspace specific file system.
 *
 ************************************************************************/
#ifndef LUSAN_VIEW_COMMON_WORKSPACEFILEDIALOG_HPP
#define LUSAN_VIEW_COMMON_WORKSPACEFILEDIALOG_HPP

/************************************************************************
 * Include files.
 ************************************************************************/
#include <QFileDialog>
#include <QStringList>

//////////////////////////////////////////////////////////////////////////
// WorkspaceFileDialog class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The custom File Dialog to display only workspace specific file system.
 **/
class WorkspaceFileDialog : public QFileDialog
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor.
     * \param   openFile    If true, the dialog is open file.
     * \param   openDir     If true, the dialog is open directory.
     * \param   caption     The caption of the dialog.
     * \param   parent      The parent widget.
     **/
    WorkspaceFileDialog(bool openFile, bool openDir, const QString& caption, QWidget* parent = nullptr);

    /**
     * \brief   Constructor.
     * \param   openFile    If true, the dialog is open file.
     * \param   openDir     If true, the dialog is open directory.
     * \param   roots       The list of root directories.
     * \param   filters     The list of file filters.
     * \param   caption     The caption of the dialog.
     * \param   parent      The parent widget.
     **/
    WorkspaceFileDialog(  bool openFile
                        , bool openDir
                        , const QStringList& roots
                        , const QStringList& filters
                        , const QString& caption
                        , QWidget* parent);

    virtual ~WorkspaceFileDialog() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the root directories.
     * \param   roots   The list of root directories.
     **/
    void setRootDirectories(const QStringList& roots);

    /**
     * \brief   Sets the file filters.
     * \param   filters The list of file filters.
     **/
    void setFileFilters(const QStringList& filters);

    /**
     * \brief   Returns the path of the selected file relative to the root directory it belongs to.
     *          Returns an empty string when nothing is selected.
     *          A file under none of the roots keeps its absolute path.
     **/
    QString getSelectedFileRelativePath() const;

    /**
     * \brief   Returns the absolute path of the selected file, or an empty string when nothing
     *          is selected.
     **/
    QString getSelectedFilePath() const;

    /**
     * \brief   Measures a file against a list of root directories and returns the path relative
     *          to the first root that contains it. The list is in priority order: the workspace
     *          root comes first, so a file under it is spelled the same way from every document.
     *          Returns the cleaned absolute path when no root contains the file.
     * \param   absoluteFilePath    The file to measure.
     * \param   roots               The root directories, most preferred first.
     **/
    static QString relativeToRoots(const QString& absoluteFilePath, const QStringList& roots);

    /**
     * \brief   Clears the history.
     **/
    void clearHistory();

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    QStringList mRootDirectories;
    QStringList mFileFilters;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    WorkspaceFileDialog() = delete;
    WorkspaceFileDialog(const WorkspaceFileDialog& src) = delete;
    WorkspaceFileDialog(WorkspaceFileDialog&& src) = delete;
    WorkspaceFileDialog& operator = (const WorkspaceFileDialog& src) = delete;
    WorkspaceFileDialog& operator = (WorkspaceFileDialog&& src) = delete;
};

#endif // LUSAN_VIEW_COMMON_WORKSPACEFILEDIALOG_HPP
