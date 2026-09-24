#ifndef LUSAN_DATA_COMMON_WORKSPACEWATCHER_HPP
#define LUSAN_DATA_COMMON_WORKSPACEWATCHER_HPP
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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/WorkspaceWatcher.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, watches the workspace directories for changes.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <QTimer>

#ifndef Q_OS_WIN
    #include <QFileSystemWatcher>
#endif // Q_OS_WIN

/**
 * \class   WorkspaceWatcher
 * \brief   Reports the files and directories created, removed, renamed or written under the
 *          workspace root directories. The changes are collected and reported in batches.
 *          On Windows every root is watched with its whole subtree and no directory under it is
 *          locked. Elsewhere only the root directories and the paths given by the clients are
 *          watched.
 **/
class WorkspaceWatcher : public QObject
{
    Q_OBJECT

#ifdef Q_OS_WIN
public:
    //!< The native watch of one root directory.
    struct sRootWatch;
#endif // Q_OS_WIN

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< The time in milliseconds the changes are collected before they are reported.
    static constexpr int    BatchIntervalMs { 200 };

//////////////////////////////////////////////////////////////////////////
// Constructors / destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit WorkspaceWatcher(QObject* parent = nullptr);

    virtual ~WorkspaceWatcher();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns true if the platform watches a root directory with its whole subtree.
     *          When false, the clients name the paths below the roots to watch.
     **/
    static bool watchesSubtree();

    /**
     * \brief   Checks whether the path is the directory or is located under it. The letter case
     *          is ignored.
     * \param   path    The absolute path to check, with '/' as separator.
     * \param   dir     The absolute path of the directory, with '/' as separator.
     **/
    static bool isSameOrUnder(const QString& path, const QString& dir);

    /**
     * \brief   Returns the absolute and clean form of the path with '/' as separator.
     **/
    static QString normalizePath(const QString& path);

    /**
     * \brief   Sets the root directories to watch. A root inside another root is dropped.
     * \param   roots   The absolute paths of the root directories.
     **/
    void setRoots(const QStringList& roots);

    /**
     * \brief   Returns the root directories that are watched.
     **/
    inline const QStringList& getRoots() const;

    /**
     * \brief   Checks whether the path is one of the roots or is located under one of them.
     * \param   path    The absolute path to check.
     **/
    bool isUnderRoot(const QString& path) const;

    /**
     * \brief   Sets the files and directories a client needs watched on the platforms that do
     *          not watch a whole subtree. Replaces the paths the client set before.
     * \param   client  The name of the client.
     * \param   paths   The absolute paths to watch.
     **/
    void setClientPaths(const QString& client, const QStringList& paths);

    /**
     * \brief   Reports the collected changes now, without waiting for the batch interval.
     **/
    void flush();

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Triggered with the paths that changed since the last report. A path is either
     *          an entry that was created, removed, renamed or written, or a directory whose
     *          list of entries changed.
     * \param   paths   The absolute paths with '/' as separator.
     **/
    void signalPathsChanged(const QStringList& paths);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Adds a changed path to the batch and starts the batch interval.
     **/
    void addChanged(const QString& path);

    /**
     * \brief   Starts watching the roots, dropping the watches set before.
     **/
    void restartWatch();

    /**
     * \brief   Stops watching all paths.
     **/
    void stopWatch();

#ifdef Q_OS_WIN
    /**
     * \brief   Reads the notifications of the root watch and arms it again.
     **/
    void onRootNotified(sRootWatch* watch);
#else
    /**
     * \brief   Watches the roots and the paths of the clients.
     **/
    void updatePathWatch();
#endif // Q_OS_WIN

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    QStringList                     mRoots;         //!< The watched root directories.
    QHash<QString, QStringList>     mClientPaths;   //!< The paths each client needs watched.
    QSet<QString>                   mPending;       //!< The changed paths not reported yet.
    QTimer                          mBatchTimer;    //!< Reports the pending paths when it fires.
#ifdef Q_OS_WIN
    QList<sRootWatch*>              mWatches;       //!< One native watch per root.
#else
    QFileSystemWatcher              mWatcher;       //!< Watches the roots and the client paths.
#endif // Q_OS_WIN
};

//////////////////////////////////////////////////////////////////////////
// WorkspaceWatcher class inline methods
//////////////////////////////////////////////////////////////////////////

inline const QStringList& WorkspaceWatcher::getRoots() const
{
    return mRoots;
}

#endif // LUSAN_DATA_COMMON_WORKSPACEWATCHER_HPP
