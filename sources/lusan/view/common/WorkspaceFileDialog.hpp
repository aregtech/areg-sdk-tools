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
     * \brief   Returns the selected file relative path.
     **/
    QString getSelectedFileRelativePath() const;

    /**
     * \brief   Returns the selected file path.
     **/
    QString getSelectedFilePath() const;

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
