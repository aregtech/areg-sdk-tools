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

#include <QCoreApplication>
#include <QFile>
#include <QLoggingCategory>

#include <algorithm>

namespace
{
    using eDocument = DocElementTable::eDocument;
    using eSource   = DocElementTable::eSource;
    using Row       = DocElementTable::Row;

    constexpr int DOCUMENT_COUNT { 3 };

    //!< The schema file of each document, the same name in both places it is looked for.
    constexpr const char* const SCHEMA_FILE[DOCUMENT_COUNT]
    {
          "fsml.xsd"
        , "siml.xsd"
        , "dtml.xsd"
    };

    //!< One document's vocabulary, and which copy of the description it came from.
    struct Table
    {
        QList<Row>  rows;                           //!< Sorted by name.
        eSource     source { eSource::BuiltIn };
        QString     path;
        QString     documentElement;
    };

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

        table.rows.reserve(elements.size());
        for (const DocSchemaReader::Element& element : elements)
        {
            Row row;
            row.name    = element.name;
            row.parents = element.parents;
            table.rows.append(row);

            if (element.parents.isEmpty())
            {
                table.documentElement = element.name;
            }
        }

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
    if (row == nullptr)
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

bool DocElementTable::documentOfSuffix(QStringView suffix, eDocument& doc)
{
    const QStringView bare = suffix.startsWith(QLatin1Char('.')) ? suffix.mid(1) : suffix;
    for (int i = 0; i < DOCUMENT_COUNT; ++i)
    {
        const QLatin1StringView schema{ SCHEMA_FILE[i] };
        if (bare.compare(schema.left(schema.size() - 4), Qt::CaseInsensitive) == 0)
        {
            doc = static_cast<eDocument>(i);
            return true;
        }
    }

    return false;
}

QString DocElementTable::sourceSummary(eDocument doc)
{
    const Table& table = tableOf(doc);
    const QString name = QString::fromLatin1(SCHEMA_FILE[static_cast<int>(doc)]);
    if (table.rows.isEmpty())
    {
        return QCoreApplication::translate("DocElementTable", "%1: not readable").arg(name);
    }

    return (table.source == eSource::Delivered)
                ? QCoreApplication::translate("DocElementTable", "%1: delivered").arg(name)
                : QCoreApplication::translate("DocElementTable", "%1: built-in").arg(name);
}

QStringList DocElementTable::sourceReport()
{
    QStringList report;
    for (int i = 0; i < DOCUMENT_COUNT; ++i)
    {
        const eDocument doc = static_cast<eDocument>(i);
        report.append(QCoreApplication::translate("DocElementTable", "%1 -- %2")
                        .arg(DocElementTable::sourceSummary(doc), DocElementTable::sourcePath(doc)));
    }

    return report;
}

void DocElementTable::reload()
{
    tables().load();
}
