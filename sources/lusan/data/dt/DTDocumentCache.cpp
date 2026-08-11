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
 *  \file        lusan/data/dt/DTDocumentCache.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared read-only store of parsed `.dtml` documents.
 *
 ************************************************************************/

#include "lusan/data/dt/DTDocumentCache.hpp"

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/dt/DataTypeDocumentData.hpp"

#include <QFileInfo>

DTDocumentCache& DTDocumentCache::getInstance()
{
    static DTDocumentCache _cache;
    return _cache;
}

QString DTDocumentCache::spaceOf(const QString& absolutePath)
{
    return (absolutePath.isEmpty() ? QString() : QFileInfo(absolutePath).completeBaseName());
}

std::shared_ptr<const DataTypeDocumentData> DTDocumentCache::document(const QString& absolutePath)
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
        auto parsed = std::make_shared<DataTypeDocumentData>();
        // A half-parsed document is worse than none: it would let a reader declare against types
        // the file does not actually contain.
        if (parsed->readFromFile(absolutePath) && parsed->openSucceeded())
        {
            // Stamped once, here, while the document is still this function's own. From now on
            // every reader sees it as `const` and its types answer to `Space::Name` only. The
            // namespace is the name the document declares; the file name only fills in for a
            // document that declares none.
            const QString declared = parsed->getOverviewData().getName();
            const QString space = declared.isEmpty() ? spaceOf(absolutePath) : declared;
            for (DataTypeCustom* dataType : parsed->getDataTypeData().getCustomDataTypes())
            {
                if (dataType != nullptr)
                {
                    dataType->setImportSpace(space);
                }
            }

            entry.document = std::move(parsed);
        }
    }

    mEntries.insert(absolutePath, entry);
    return entry.document;
}

void DTDocumentCache::invalidate(const QString& absolutePath)
{
    QMutexLocker locker(&mLock);
    mEntries.remove(absolutePath);
}

void DTDocumentCache::clear()
{
    QMutexLocker locker(&mLock);
    mEntries.clear();
}
