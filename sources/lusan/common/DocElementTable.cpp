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
 *  \file        lusan/common/DocElementTable.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the elements a document may contain and where each may sit.
 *
 ************************************************************************/

#include "lusan/common/DocElementTable.hpp"

#include "lusan/common/DocSchemaReader.hpp"
#include "lusan/common/VersionNumber.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QFile>
#include <QLoggingCategory>

#include <algorithm>

namespace
{
    using eDocument = DocElementTable::eDocument;
    using eSource   = DocElementTable::eSource;
    using eStatus   = DocElementTable::eStatus;
    using Row       = DocElementTable::Row;

    constexpr int DOCUMENT_COUNT { 3 };

    //!< The schema file of each document, the same name in both places it is looked for.
    constexpr const char* const SCHEMA_FILE[DOCUMENT_COUNT]
    {
          "fsml.xsd"
        , "siml.xsd"
        , "dtml.xsd"
    };

    /**
     * \struct  Retired
     * \brief   An element the format once defined. A schema states what it accepts, so what it
     *          no longer accepts has to be named here or a document carrying one is reported as
     *          a plain misspelling with nothing useful to say about it.
     **/
    struct Retired
    {
        eDocument   document;
        const char* name;
        const char* parents;        //!< Where it used to sit, '|' separated.
        const char* replacement;
    };

    constexpr Retired RETIRED[]
    {
          { eDocument::StateMachine, "DoList", "State"
          , "a Timer started on entry and stopped on exit" }
        , { eDocument::StateMachine, "InlineCode", "EntryList|ExitList|OperationList"
          , "an Action that carries the code" }
    };

    //!< One document's vocabulary, and which copy of the description it came from.
    struct Table
    {
        QList<Row>  rows;                           //!< Sorted by name.
        eSource     source { eSource::BuiltIn };
        QString     path;
        QString     documentElement;
    };

    void appendRetired(Table& table, eDocument doc)
    {
        for (const Retired& retired : RETIRED)
        {
            if (retired.document != doc)
            {
                continue;
            }

            const QString name = QString::fromLatin1(retired.name);
            const auto known = std::find_if(table.rows.cbegin(), table.rows.cend()
                                          , [&name](const Row& row) { return row.name == name; });
            if (known != table.rows.cend())
            {
                continue;
            }

            Row row;
            row.name        = name;
            row.parents     = QString::fromLatin1(retired.parents).split(QLatin1Char('|'));
            row.status      = eStatus::Removed;
            row.replacement = QString::fromLatin1(retired.replacement);
            table.rows.append(row);
        }
    }

    Table buildTable(eDocument doc)
    {
        const QString fileName = QString::fromLatin1(SCHEMA_FILE[static_cast<int>(doc)]);
        const QString directory = DocSchemaReader::deliveryDirectory();
        const QString delivered = directory.isEmpty() ? QString() : (directory + QLatin1Char('/') + fileName);
        const QString builtIn   = QStringLiteral(":/schema/") + fileName;

        Table table;
        QList<DocSchemaReader::Element> elements;
        if ((delivered.isEmpty() == false) && QFile::exists(delivered))
        {
            elements = DocSchemaReader::readFile(delivered);
            if (elements.isEmpty() == false)
            {
                table.source = eSource::Delivered;
                table.path   = delivered;
            }
        }

        if (elements.isEmpty())
        {
            elements     = DocSchemaReader::readFile(builtIn);
            table.source = eSource::BuiltIn;
            table.path   = builtIn;
        }

        table.rows.reserve(elements.size() + 2);
        for (const DocSchemaReader::Element& element : elements)
        {
            Row row;
            row.name    = element.name;
            row.parents = element.parents;
            row.status  = eStatus::Valid;
            table.rows.append(row);

            if (element.parents.isEmpty())
            {
                table.documentElement = element.name;
            }
        }

        appendRetired(table, doc);
        std::sort(table.rows.begin(), table.rows.end()
                , [](const Row& left, const Row& right) { return left.name < right.name; });
        return table;
    }

    //!< True when the delivered schema says something the compiled-in one does not. The two travel
    //!< through git and through a copy step, so their line endings differ routinely and mean nothing.
    bool differsFromBuiltIn(const QString& delivered, const char* fileName)
    {
        QFile shipped(delivered);
        QFile compiled(QStringLiteral(":/schema/") + QString::fromLatin1(fileName));
        if ((shipped.open(QIODevice::ReadOnly) == false) || (compiled.open(QIODevice::ReadOnly) == false))
        {
            return false;
        }

        return shipped.readAll().replace('\r', "") != compiled.readAll().replace('\r', "");
    }

    //!< The three tables, built once. A missing delivered schema costs nothing but one line in
    //!< the log: the built-in copy is the same description, not a reduced one.
    struct Tables
    {
        Table entry[DOCUMENT_COUNT];

        Tables()
        {
            load();
        }

        void load()
        {
            for (int i = 0; i < DOCUMENT_COUNT; ++i)
            {
                entry[i] = buildTable(static_cast<eDocument>(i));
                if (entry[i].rows.isEmpty())
                {
                    qWarning("Lusan: no description of the %s format could be read; "
                             "every element of such a document will be reported as unknown."
                           , SCHEMA_FILE[i]);
                }
                else if (entry[i].source == eSource::BuiltIn)
                {
                    qInfo("Lusan: no %s delivered beside the executable, using the built-in copy."
                        , SCHEMA_FILE[i]);
                }
                else if (differsFromBuiltIn(entry[i].path, SCHEMA_FILE[i]))
                {
                    // The two copies drifting apart is what nobody notices until a document
                    // validates in one build and not in another, so say it once.
                    qInfo("Lusan: the delivered %s differs from the built-in copy; the delivered one is used."
                        , SCHEMA_FILE[i]);
                }
            }
        }
    };

    Tables& tables()
    {
        static Tables instance;
        return instance;
    }

    const Table& tableOf(eDocument doc)
    {
        return tables().entry[static_cast<int>(doc)];
    }
}

const VersionNumber& DocElementTable::maxFormatVersion()
{
    static const VersionNumber highest(StateMachineData::XML_FORMAT_DEFAULT);
    return highest;
}

const DocElementTable::Row* DocElementTable::find(eDocument doc, QStringView name)
{
    const QList<Row>& rows = tableOf(doc).rows;
    const auto found = std::lower_bound(rows.cbegin(), rows.cend(), name
                                      , [](const Row& row, QStringView key) { return row.name < key; });
    return ((found != rows.cend()) && (found->name == name)) ? &(*found) : nullptr;
}

bool DocElementTable::accepts(eDocument doc, QStringView name, QStringView parent)
{
    const Row* row = find(doc, name);
    if ((row == nullptr) || (row->status == eStatus::Removed))
    {
        return false;
    }

    if (parent.isEmpty())
    {
        return row->parents.isEmpty();
    }

    for (const QString& candidate : row->parents)
    {
        if (candidate == parent)
        {
            return true;
        }
    }

    return false;
}

QString DocElementTable::documentElement(eDocument doc)
{
    return tableOf(doc).documentElement;
}

DocElementTable::eSource DocElementTable::source(eDocument doc)
{
    return tableOf(doc).source;
}

QString DocElementTable::sourcePath(eDocument doc)
{
    return tableOf(doc).path;
}

void DocElementTable::reload()
{
    tables().load();
}
