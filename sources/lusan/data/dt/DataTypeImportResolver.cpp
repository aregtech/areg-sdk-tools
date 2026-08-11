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
 *  \file        lusan/data/dt/DataTypeImportResolver.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, resolution of the data type documents a document includes.
 *
 ************************************************************************/

#include "lusan/data/dt/DataTypeImportResolver.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/data/common/IncludeDataSection.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/dt/DTDocumentCache.hpp"
#include "lusan/data/dt/DataTypeDocumentData.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>

namespace
{
    using ImportedTypes = DataTypeDataSection::ImportedTypes;
    using eImportState  = DataTypeDataSection::eImportState;

    //!< The directory a host's relative locations are measured from; empty when the host is unsaved.
    QString hostDirectory(const QString& hostFilePath)
    {
        return (hostFilePath.isEmpty() ? QString() : QFileInfo(hostFilePath).absolutePath());
    }

    //!< Two rebuilds that produce the same groups leave every reader alone, so the comparison is
    //!< on what a reader can see: which row, which file, which state, and which types.
    bool sameGroups(const QList<ImportedTypes>& before, const QList<ImportedTypes>& after)
    {
        if (before.size() != after.size())
            return false;

        for (qsizetype i = 0; i < before.size(); ++i)
        {
            const ImportedTypes& a = before.at(i);
            const ImportedTypes& b = after.at(i);
            if ((a.id != b.id) || (a.state != b.state) || (a.space != b.space)
                || (a.absolutePath != b.absolutePath) || (a.types != b.types))
            {
                return false;
            }
        }

        return true;
    }
}

QString DataTypeImportResolver::absolutePath(const QString& hostFilePath, const QString& location)
{
    return NELusanCommon::resolveLocation(hostDirectory(hostFilePath), location);
}

QString DataTypeImportResolver::storableLocation(const QString& /*hostFilePath*/, const QString& absoluteFilePath)
{
    // Measured from a workspace root and not from the host document, so that two documents in
    // different folders importing one file write the same location and the generator places its
    // output once. A relative location also survives moving or checking out the tree elsewhere.
    return NELusanCommon::toStorableLocation(absoluteFilePath);
}

bool DataTypeImportResolver::refresh(DataTypeDataSection& types, const QString& hostFilePath
                                    , const IncludeDataSection& includes)
{
    QList<ImportedTypes> groups;
    QSet<QString>        claimed;

    for (const IncludeEntry& include : includes.getElements())
    {
        const QString location = include.getLocation();
        if (includeKindOf(location, QString()) != eIncludeKind::DataType)
        {
            continue;
        }

        ImportedTypes group;
        group.id           = include.getId();
        group.location     = location;
        group.absolutePath = absolutePath(hostFilePath, location);
        // Until the file is read, the best guess at the namespace is what it is called on disk.
        group.space        = DTDocumentCache::spaceOf(group.absolutePath.isEmpty() ? location : group.absolutePath);

        if (group.absolutePath.isEmpty() || (QFileInfo(group.absolutePath).isFile() == false))
        {
            group.state = eImportState::NotFound;
            groups.append(std::move(group));
            continue;
        }

        group.document = DTDocumentCache::getInstance().document(group.absolutePath);
        if (group.document == nullptr)
        {
            group.state = eImportState::ParseFailed;
            groups.append(std::move(group));
            continue;
        }

        // The namespace is the name the document declares, which is also what the generated header
        // and source are called. The file may be called something else.
        const QString declared = group.document->getOverviewData().getName();
        if (declared.isEmpty() == false)
        {
            group.space = declared;
        }

        if (claimed.contains(group.space))
        {
            // Both would generate into one namespace, so the second one contributes nothing and
            // is reported instead.
            group.state = eImportState::DuplicateSpace;
            group.document.reset();
            groups.append(std::move(group));
            continue;
        }

        group.state = eImportState::Resolved;
        group.types = group.document->getDataTypeData().getCustomDataTypes();
        claimed.insert(group.space);
        groups.append(std::move(group));
    }

    if (sameGroups(types.getImports(), groups))
    {
        return false;
    }

    types.setImports(std::move(groups));
    return true;
}

QStringList DataTypeImportResolver::resolvedPaths(const DataTypeDataSection& types)
{
    QStringList result;
    for (const ImportedTypes& group : types.getImports())
    {
        if ((group.absolutePath.isEmpty() == false) && (result.contains(group.absolutePath) == false))
        {
            result.append(group.absolutePath);
        }
    }

    return result;
}
