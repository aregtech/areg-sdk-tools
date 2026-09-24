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
 *  \file        lusan/data/common/WorkspaceWatcher.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, watches the workspace directories for changes.
 *
 ************************************************************************/

#include "lusan/data/common/WorkspaceWatcher.hpp"

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <utility>

#ifdef Q_OS_WIN
    #include <QWinEventNotifier>

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif // WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #include <vector>
#endif // Q_OS_WIN

namespace
{
    inline QString dirPrefix(const QString& dir)
    {
        return (dir.endsWith(QLatin1Char('/')) ? dir : dir + QLatin1Char('/'));
    }
}

#ifdef Q_OS_WIN

//////////////////////////////////////////////////////////////////////////
// WorkspaceWatcher::sRootWatch structure
//////////////////////////////////////////////////////////////////////////

//!< The native watch of one root directory and its whole subtree.
struct WorkspaceWatcher::sRootWatch
{
    QString             root;                           //!< The watched root directory.
    HANDLE              dir     { INVALID_HANDLE_VALUE };//!< The handle of the root directory.
    OVERLAPPED          overlap { };                    //!< The pending read of the notifications.
    bool                armed   { false };              //!< A read of the notifications is pending.
    QWinEventNotifier*  notifier{ nullptr };            //!< Signals the completed read in the event loop.
    std::vector<DWORD>  buffer  { };                    //!< Receives the notifications, DWORD aligned.
};

namespace
{
    //!< The size of the notification buffer in DWORDs. 64 KB is the most a network share accepts.
    constexpr std::size_t NotifyBufferDwords{ 64 * 1024 / sizeof(DWORD) };

    //!< The changes of the subtree the watch reports.
    constexpr DWORD NotifyFilter{ FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE };

    bool armWatch(WorkspaceWatcher::sRootWatch& watch);

    void releaseWatch(WorkspaceWatcher::sRootWatch* watch)
    {
        if (watch == nullptr)
            return;

        if (watch->notifier != nullptr)
        {
            watch->notifier->setEnabled(false);
            delete watch->notifier;
            watch->notifier = nullptr;
        }

        if (watch->dir != INVALID_HANDLE_VALUE)
        {
            if (watch->armed)
            {
                // The buffer must stay alive until the kernel has finished with the cancelled read.
                DWORD bytes{ 0 };
                ::CancelIoEx(watch->dir, &watch->overlap);
                ::GetOverlappedResult(watch->dir, &watch->overlap, &bytes, TRUE);
                watch->armed = false;
            }

            ::CloseHandle(watch->dir);
            watch->dir = INVALID_HANDLE_VALUE;
        }

        if (watch->overlap.hEvent != nullptr)
        {
            ::CloseHandle(watch->overlap.hEvent);
            watch->overlap.hEvent = nullptr;
        }

        delete watch;
    }
}

#endif // Q_OS_WIN

//////////////////////////////////////////////////////////////////////////
// WorkspaceWatcher class implementation
//////////////////////////////////////////////////////////////////////////

WorkspaceWatcher::WorkspaceWatcher(QObject* parent /*= nullptr*/)
    : QObject       (parent)
    , mRoots        ( )
    , mClientPaths  ( )
    , mPending      ( )
    , mBatchTimer   ( )
#ifdef Q_OS_WIN
    , mWatches      ( )
#else
    , mWatcher      ( )
#endif // Q_OS_WIN
{
    mBatchTimer.setSingleShot(true);
    mBatchTimer.setInterval(BatchIntervalMs);
    connect(&mBatchTimer, &QTimer::timeout, this, &WorkspaceWatcher::flush);

#ifndef Q_OS_WIN
    connect(&mWatcher, &QFileSystemWatcher::directoryChanged, this, &WorkspaceWatcher::addChanged);
    connect(&mWatcher, &QFileSystemWatcher::fileChanged     , this, &WorkspaceWatcher::addChanged);
#endif // Q_OS_WIN
}

WorkspaceWatcher::~WorkspaceWatcher()
{
    stopWatch();
}

bool WorkspaceWatcher::watchesSubtree()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif // Q_OS_WIN
}

bool WorkspaceWatcher::isSameOrUnder(const QString& path, const QString& dir)
{
    if (path.isEmpty() || dir.isEmpty())
        return false;

    return (path.compare(dir, Qt::CaseSensitivity::CaseInsensitive) == 0) || path.startsWith(dirPrefix(dir), Qt::CaseSensitivity::CaseInsensitive);
}

QString WorkspaceWatcher::normalizePath(const QString& path)
{
    return (path.isEmpty() ? QString() : QDir::cleanPath(QFileInfo(QDir::fromNativeSeparators(path)).absoluteFilePath()));
}

void WorkspaceWatcher::setRoots(const QStringList& roots)
{
    QStringList candidates;
    for (const QString& root : roots)
    {
        const QString path{ normalizePath(root) };
        if (path.isEmpty() == false)
        {
            candidates.append(path);
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const QString& left, const QString& right) { return (left.size() < right.size()); });

    QStringList result;
    for (const QString& path : candidates)
    {
        const bool covered = std::any_of(result.cbegin(), result.cend(), [&path](const QString& root) { return isSameOrUnder(path, root); });
        if (covered == false)
        {
            result.append(path);
        }
    }

    if (result == mRoots)
        return;

    mRoots = result;
    restartWatch();
}

bool WorkspaceWatcher::isUnderRoot(const QString& path) const
{
    return std::any_of(mRoots.cbegin(), mRoots.cend(), [&path](const QString& root) { return isSameOrUnder(path, root); });
}

void WorkspaceWatcher::setClientPaths(const QString& client, const QStringList& paths)
{
    QStringList normalized;
    normalized.reserve(paths.size());
    for (const QString& path : paths)
    {
        const QString entry{ normalizePath(path) };
        if ((entry.isEmpty() == false) && (normalized.contains(entry) == false))
        {
            normalized.append(entry);
        }
    }

    QStringList& current{ mClientPaths[client] };
    if (current == normalized)
        return;

    current = normalized;
#ifndef Q_OS_WIN
    updatePathWatch();
#endif // Q_OS_WIN
}

void WorkspaceWatcher::flush()
{
    mBatchTimer.stop();
    if (mPending.isEmpty())
        return;

    const QStringList paths{ mPending.values() };
    mPending.clear();
    emit signalPathsChanged(paths);
}

void WorkspaceWatcher::addChanged(const QString& path)
{
    mPending.insert(path);
    if (mBatchTimer.isActive() == false)
    {
        mBatchTimer.start();
    }
}

#ifdef Q_OS_WIN

namespace
{
    bool armWatch(WorkspaceWatcher::sRootWatch& watch)
    {
        ::ResetEvent(watch.overlap.hEvent);
        const DWORD bytes{ static_cast<DWORD>(watch.buffer.size() * sizeof(DWORD)) };
        watch.armed = (::ReadDirectoryChangesW(watch.dir, watch.buffer.data(), bytes, TRUE, NotifyFilter, nullptr, &watch.overlap, nullptr) != FALSE);
        return watch.armed;
    }
}

void WorkspaceWatcher::restartWatch()
{
    stopWatch();

    for (const QString& root : mRoots)
    {
        sRootWatch* watch = new sRootWatch();
        watch->root = root;
        watch->buffer.resize(NotifyBufferDwords);
        // Opened with every sharing flag, so the folders under the root stay renamable and deletable.
        watch->dir  = ::CreateFileW( reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(root).utf16())
                                   , FILE_LIST_DIRECTORY
                                   , FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE
                                   , nullptr
                                   , OPEN_EXISTING
                                   , FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED
                                   , nullptr);
        watch->overlap.hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if ((watch->dir == INVALID_HANDLE_VALUE) || (watch->overlap.hEvent == nullptr) || (armWatch(*watch) == false))
        {
            releaseWatch(watch);
            continue;
        }

        watch->notifier = new QWinEventNotifier(watch->overlap.hEvent, this);
        connect(watch->notifier, &QWinEventNotifier::activated, this, [this, watch]() { onRootNotified(watch); });
        mWatches.append(watch);
    }
}

void WorkspaceWatcher::stopWatch()
{
    for (sRootWatch* watch : mWatches)
    {
        releaseWatch(watch);
    }

    mWatches.clear();
}

void WorkspaceWatcher::onRootNotified(sRootWatch* watch)
{
    if (mWatches.contains(watch) == false)
        return;

    DWORD bytes{ 0 };
    const bool completed{ ::GetOverlappedResult(watch->dir, &watch->overlap, &bytes, FALSE) != FALSE };
    watch->armed = false;
    if (completed && (bytes > 0))
    {
        const QString prefix{ dirPrefix(watch->root) };
        const char* data{ reinterpret_cast<const char*>(watch->buffer.data()) };
        for (;;)
        {
            const FILE_NOTIFY_INFORMATION* info{ reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(data) };
            const QString name{ QString::fromWCharArray(info->FileName, static_cast<int>(info->FileNameLength / sizeof(WCHAR))) };
            addChanged(prefix + QDir::fromNativeSeparators(name));
            if (info->NextEntryOffset == 0)
                break;

            data += info->NextEntryOffset;
        }
    }
    else
    {
        // The buffer overflowed or the root is gone: the whole root is reported as changed.
        addChanged(watch->root);
    }

    if (completed && armWatch(*watch))
        return;

    // The root cannot be read any more. The watch is released once the notifier has returned.
    watch->notifier->setEnabled(false);
    QMetaObject::invokeMethod(this, [this, watch]()
        {
            if (mWatches.removeOne(watch))
            {
                releaseWatch(watch);
            }
        }, Qt::QueuedConnection);
}

#else   // Q_OS_WIN

void WorkspaceWatcher::restartWatch()
{
    updatePathWatch();
}

void WorkspaceWatcher::stopWatch()
{
    const QStringList watched{ mWatcher.files() + mWatcher.directories() };
    if (watched.isEmpty() == false)
    {
        mWatcher.removePaths(watched);
    }
}

void WorkspaceWatcher::updatePathWatch()
{
    QSet<QString> wanted;
    for (const QString& root : mRoots)
    {
        wanted.insert(root);
    }

    for (auto it = mClientPaths.constBegin(); it != mClientPaths.constEnd(); ++it)
    {
        for (const QString& path : it.value())
        {
            wanted.insert(path);
        }
    }

    const QStringList watched{ mWatcher.files() + mWatcher.directories() };
    QStringList remove;
    for (const QString& path : watched)
    {
        if (wanted.remove(path) == false)
        {
            remove.append(path);
        }
    }

    if (remove.isEmpty() == false)
    {
        mWatcher.removePaths(remove);
    }

    QStringList add;
    for (const QString& path : std::as_const(wanted))
    {
        if (QFileInfo::exists(path))
        {
            add.append(path);
        }
    }

    if (add.isEmpty() == false)
    {
        mWatcher.addPaths(add);
    }
}

#endif  // Q_OS_WIN
