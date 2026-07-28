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
 *  \file        lusan/data/sm/SMDocumentCache.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared read-only store of parsed `.fsml` documents.
 *
 ************************************************************************/

#include "lusan/data/sm/SMDocumentCache.hpp"

#include "lusan/data/sm/StateMachineData.hpp"

#include <QFileInfo>

SMDocumentCache& SMDocumentCache::getInstance()
{
    static SMDocumentCache _cache;
    return _cache;
}

std::shared_ptr<const StateMachineData> SMDocumentCache::document(const QString& absolutePath)
{
    if (absolutePath.isEmpty())
    {
        return nullptr;
    }

    const QFileInfo info(absolutePath);
    const QDateTime modified = info.lastModified();
    const qint64    size     = info.size();

    QMutexLocker locker(&mLock);
    const auto found = mEntries.constFind(absolutePath);
    if ((found != mEntries.constEnd()) && (found->modified == modified) && (found->size == size))
    {
        return found->document;
    }

    Entry entry;
    entry.modified = modified;
    entry.size     = size;
    if (info.isFile())
    {
        auto parsed = std::make_shared<StateMachineData>();
        // A half-parsed document is worse than none: it would let a host validate against
        // content the file does not actually contain.
        if (parsed->readFromFile(absolutePath) && parsed->openSucceeded())
        {
            entry.document = std::move(parsed);
        }
    }

    mEntries.insert(absolutePath, entry);
    return entry.document;
}

void SMDocumentCache::invalidate(const QString& absolutePath)
{
    QMutexLocker locker(&mLock);
    mEntries.remove(absolutePath);
}

void SMDocumentCache::clear()
{
    QMutexLocker locker(&mLock);
    mEntries.clear();
}
