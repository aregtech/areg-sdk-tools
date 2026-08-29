#ifndef LUSAN_DATA_DT_DTDOCUMENTCACHE_HPP
#define LUSAN_DATA_DT_DTDOCUMENTCACHE_HPP
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
 *  \file        lusan/data/dt/DTDocumentCache.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared read-only store of parsed `.dtml` documents.
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
class DataTypeDocumentData;

/**
 * \class   DTDocumentCache
 * \brief   Application-wide store of included `.dtml` documents, keyed by absolute path and
 *          revalidated by file timestamp. One shared file is normally included by several
 *          interfaces and machines, all of them open at once, and the type registry, the Data
 *          Types page and the validator of each of them need its content -- so it is parsed once
 *          and shared instead of once per reader.
 *
 *          Every type of a cached document carries the document's namespace before it is handed
 *          out, so a reader resolves `Space::Name` and never the bare name: a bare name belongs
 *          to the document that reads it. That is done here because it is the one place a parsed
 *          document and the file it came from are both in hand.
 *
 *          Entries are immutable once parsed: a reader only ever sees a `const` document.
 **/
class DTDocumentCache
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    struct Entry
    {
        std::shared_ptr<const DataTypeDocumentData> document;    //!< Null when the file failed to parse.
        QDateTime                                   modified;    //!< The file timestamp the entry was built from.
        qint64                                      size { 0 };  //!< The file size at parse time.
    };

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    static DTDocumentCache& getInstance();

    /**
     * \brief   The parsed document at the given absolute path, or nullptr when the file is
     *          missing or does not parse. A stale entry (file touched since the last read) is
     *          re-parsed; a failed parse is remembered too, so a broken include is not retried
     *          on every validation pass.
     **/
    std::shared_ptr<const DataTypeDocumentData> document(const QString& absolutePath);

    /**
     * \brief   The fallback namespace of a file at the given path: its base name. Answered
     *          without reading the file, so a caller can name a document that does not parse.
     *          A document that reads contributes the name it declares instead.
     **/
    static QString spaceOf(const QString& absolutePath);

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
    DTDocumentCache() = default;

    DTDocumentCache(const DTDocumentCache&) = delete;
    DTDocumentCache& operator = (const DTDocumentCache&) = delete;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QHash<QString, Entry>   mEntries;
    QMutex                  mLock;
};

#endif  // LUSAN_DATA_DT_DTDOCUMENTCACHE_HPP
