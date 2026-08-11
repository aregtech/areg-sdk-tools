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
 *  \file        lusan/data/sm/SMImportResolver.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, submachine import resolution and cycle detection.
 *
 ************************************************************************/

#include "lusan/data/sm/SMImportResolver.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/sm/SMDocumentCache.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>

namespace
{
    //!< The directory a host document's relative locations are measured from; empty when unsaved.
    QString hostDirectory(const StateMachineData& host)
    {
        const QString& path = host.getFilePath();
        return (path.isEmpty() ? QString() : QFileInfo(path).absolutePath());
    }

    //!< How a document names itself in a message: its machine name, or the file name if unnamed.
    QString documentLabel(const StateMachineData& doc)
    {
        const QString& name = doc.getOverview().getName();
        return (name.isEmpty() == false ? name : QFileInfo(doc.getFilePath()).fileName());
    }

    //!< The recursion behind importDepth: \p onPath holds the documents already on the current
    //!< branch, so a cycle terminates instead of recursing forever.
    int depthOf(const QString& absoluteFilePath, int limit, QSet<QString>& onPath, QStringList& chain);

    //!< Absolute, cleaned path of a stored location. A location is written against a workspace
    //!< root; documents written before that spell it against their own folder, and both resolve.
    QString makeAbsolute(const QString& directory, const QString& location)
    {
        return NELusanCommon::resolveLocation(directory, location);
    }

    int depthOf(const QString& absoluteFilePath, int limit, QSet<QString>& onPath, QStringList& chain)
    {
        if (absoluteFilePath.isEmpty() || onPath.contains(absoluteFilePath))
        {
            return 0;
        }

        std::shared_ptr<const StateMachineData> doc = SMDocumentCache::getInstance().document(absoluteFilePath);
        if (doc == nullptr)
        {
            return 0;
        }

        chain.append(documentLabel(*doc));
        if (limit <= 0)
        {
            return 1;
        }

        onPath.insert(absoluteFilePath);
        int best = 0;
        QStringList bestChain;
        for (const IncludeEntry* nested : doc->machineImports())
        {
            QStringList branch;
            const int depth = depthOf(makeAbsolute(QFileInfo(absoluteFilePath).absolutePath(), nested->getLocation())
                                      , limit - 1, onPath, branch);
            if (depth > best)
            {
                best = depth;
                bestChain = branch;
                if (best >= limit)
                {
                    break;
                }
            }
        }

        onPath.remove(absoluteFilePath);
        chain.append(bestChain);
        return (1 + best);
    }
}

QString SMImportResolver::absolutePath(const StateMachineData& host, const QString& location)
{
    return makeAbsolute(hostDirectory(host), location);
}

QString SMImportResolver::storableLocation(const StateMachineData& /*host*/, const QString& absoluteFilePath)
{
    // Measured from a workspace root and not from the host document, so that two machines in
    // different folders importing one file write the same location and the generator places its
    // output once. A relative location also survives moving or checking out the tree elsewhere.
    return NELusanCommon::toStorableLocation(absoluteFilePath);
}

SMImportResolver::Resolution SMImportResolver::resolve(const StateMachineData& host, const IncludeEntry& entry)
{
    Resolution result;
    if (entry.getLocation().isEmpty())
    {
        return result;
    }

    result.absolutePath = absolutePath(host, entry.getLocation());
    if (result.absolutePath.isEmpty() || (QFileInfo(result.absolutePath).isFile() == false))
    {
        result.state = eState::NotFound;
        return result;
    }

    result.document = SMDocumentCache::getInstance().document(result.absolutePath);
    if (result.document == nullptr)
    {
        result.state = eState::ParseFailed;
        return result;
    }

    result.state         = eState::Resolved;
    result.actualVersion = result.document->getOverview().getVersion();
    return result;
}

bool SMImportResolver::findCycle(const StateMachineData& host, const IncludeEntry& entry, QStringList& chain)
{
    const QString hostPath = QDir::cleanPath(QFileInfo(host.getFilePath()).absoluteFilePath());
    if (host.getFilePath().isEmpty())
    {
        // An unsaved document has no identity on disk, so nothing can point back at it.
        return false;
    }

    struct Step
    {
        QString     path;
        QStringList trail;
    };

    const QString first = absolutePath(host, entry.getLocation());
    if (first.isEmpty())
    {
        return false;
    }

    QList<Step>   pending{ Step{ first, QStringList{ documentLabel(host) } } };
    QSet<QString> seen{ hostPath };

    while (pending.isEmpty() == false)
    {
        const Step step = pending.takeFirst();
        if (step.path == hostPath)
        {
            chain = step.trail;
            chain.append(documentLabel(host));
            return true;
        }

        if (seen.contains(step.path))
        {
            // A cycle that does not run through the host belongs to that other document's own
            // findings, not to this one.
            continue;
        }

        seen.insert(step.path);
        std::shared_ptr<const StateMachineData> doc = SMDocumentCache::getInstance().document(step.path);
        if (doc == nullptr)
        {
            continue;
        }

        QStringList trail = step.trail;
        trail.append(documentLabel(*doc));
        for (const IncludeEntry* nested : doc->machineImports())
        {
            const QString next = absolutePath(*doc, nested->getLocation());
            if (next.isEmpty() == false)
            {
                pending.append(Step{ next, trail });
            }
        }
    }

    return false;
}

int SMImportResolver::importDepth(const QString& absoluteFilePath, int limit, QStringList& chain)
{
    QSet<QString> onPath;
    chain.clear();
    return depthOf(absoluteFilePath, limit, onPath, chain);
}
