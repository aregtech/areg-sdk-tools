#ifndef LUSAN_DATA_SM_SMDOCUMENTCACHE_HPP
#define LUSAN_DATA_SM_SMDOCUMENTCACHE_HPP
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
 *  \file        lusan/data/sm/SMDocumentCache.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared read-only store of parsed `.fsml` documents.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QString>

#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \class   SMDocumentCache
 * \brief   Application-wide store of imported `.fsml` documents, keyed by absolute path and
 *          revalidated by file timestamp. The same import is normally instantiated by several
 *          states and often by several open documents, and the canvas badges, the validator,
 *          the version-pinning check and the interpreter all need the parsed content -- so it
 *          is parsed once and shared, instead of once per consumer with no guarantee that the
 *          copies agree.
 *
 *          Entries are immutable once parsed: a consumer only ever sees a `const` document.
 *          The cache is reachable from the validator, which runs on a worker thread, so every
 *          access is serialized.
 **/
class SMDocumentCache
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    struct Entry
    {
        std::shared_ptr<const StateMachineData> document;    //!< Null when the file failed to parse.
        QDateTime                               modified;    //!< The file timestamp the entry was built from.
        qint64                                  size { 0 };  //!< The file size at parse time.
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    static SMDocumentCache& getInstance();

    /**
     * \brief   The parsed document at the given absolute path, or nullptr when the file is
     *          missing or does not parse. A stale entry (file touched since the last read) is
     *          re-parsed; a failed parse is remembered too, so a broken import is not retried
     *          on every validation pass.
     **/
    std::shared_ptr<const StateMachineData> document(const QString& absolutePath);

    /**
     * \brief   Drops the entry of one document, so the next request re-reads it.
     **/
    void invalidate(const QString& absolutePath);

    /**
     * \brief   Drops every entry.
     **/
    void clear();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    SMDocumentCache() = default;

    SMDocumentCache(const SMDocumentCache&) = delete;
    SMDocumentCache& operator = (const SMDocumentCache&) = delete;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QHash<QString, Entry>   mEntries;
    QMutex                  mLock;
};

#endif  // LUSAN_DATA_SM_SMDOCUMENTCACHE_HPP
