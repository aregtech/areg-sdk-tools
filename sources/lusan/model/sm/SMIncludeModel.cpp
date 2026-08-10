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
 *  \file        lusan/model/sm/SMIncludeModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMIncludeModel.hpp"

#include "lusan/data/sm/SMDocumentCache.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/model/sm/SMRenameCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include <QDir>
#include <QFileInfo>
#include <QObject>

namespace
{
    //!< The host document's own extension; what makes a `.fsml` include an imported machine.
    const QString FsmExtension{ QStringLiteral("fsml") };
}

SMIncludeModel::SMIncludeModel(StateMachineModel& facade)
    : IncludeModel  (facade)
    , mFacade       (facade)
{
}

const IncludeEntry* SMIncludeModel::findByAlias(const QString& alias) const
{
    return mFacade.getData().findImportByAlias(alias);
}

eIncludeKind SMIncludeModel::kindOf(uint32_t id) const
{
    const IncludeEntry* entry = findInclude(id);
    return includeKindOf(entry != nullptr ? entry->getLocation() : QString(), FsmExtension);
}

QStringList SMIncludeModel::getAliases(void) const
{
    QStringList result;
    for (const IncludeEntry* entry : mFacade.getData().machineImports())
    {
        if (entry->getAlias().isEmpty() == false)
        {
            result.append(entry->getAlias());
        }
    }

    return result;
}

bool SMIncludeModel::isReadOnly(void) const
{
    return mFacade.isReadOnly();
}

SMImportResolver::Resolution SMIncludeModel::resolutionOf(uint32_t id) const
{
    const IncludeEntry* entry = findInclude(id);
    return (entry != nullptr ? SMImportResolver::resolve(mFacade.getData(), *entry) : SMImportResolver::Resolution{});
}

QString SMIncludeModel::storableLocation(const QString& absoluteFilePath) const
{
    return SMImportResolver::storableLocation(mFacade.getData(), absoluteFilePath);
}

QString SMIncludeModel::absolutePathOf(const QString& location) const
{
    return SMImportResolver::absolutePath(mFacade.getData(), location);
}

QList<SMReferences::Use> SMIncludeModel::whereUsed(uint32_t id) const
{
    const IncludeEntry* entry = findInclude(id);
    return ((entry != nullptr) && (entry->getAlias().isEmpty() == false)
            ? SMReferences::whereUsed(mFacade.getData(), SMReferences::eTarget::Import, entry->getAlias(), id)
            : QList<SMReferences::Use>());
}

SMIncludeModel::eImportRefusal SMIncludeModel::canImport(const QString& absoluteFilePath, QStringList& chain) const
{
    chain.clear();
    const StateMachineData& host = mFacade.getData();
    if (host.getFilePath().isEmpty())
    {
        // Nothing can point back at a document that has no identity on disk, so the cycle check
        // would pass on any file; and a relative location cannot be formed either.
        return eImportRefusal::HostNotSaved;
    }

    const QString candidate = QDir::cleanPath(QFileInfo(absoluteFilePath).absoluteFilePath());
    if (candidate.compare(QDir::cleanPath(QFileInfo(host.getFilePath()).absoluteFilePath()), Qt::CaseInsensitive) == 0)
    {
        return eImportRefusal::SelfImport;
    }

    IncludeEntry probe(0u, SMImportResolver::storableLocation(host, candidate), nullptr);
    if (SMImportResolver::findCycle(host, probe, chain))
    {
        return eImportRefusal::Cycle;
    }

    if (SMImportResolver::importDepth(candidate, SMImportResolver::MAX_IMPORT_DEPTH, chain) > SMImportResolver::MAX_IMPORT_DEPTH)
    {
        return eImportRefusal::TooDeep;
    }

    chain.clear();
    // A broken import must never cost the user their own document: the file may be repaired
    // later, so this is reported and the entry is still created.
    return (SMDocumentCache::getInstance().document(candidate) != nullptr
            ? eImportRefusal::None
            : eImportRefusal::Unreadable);
}

QString SMIncludeModel::uniqueAlias(const QString& baseName) const
{
    QString seed;
    for (const QChar& ch : baseName)
    {
        seed.append(ch.isLetterOrNumber() || (ch == QLatin1Char('_')) ? ch : QLatin1Char('_'));
    }

    if (seed.isEmpty() || seed.at(0).isDigit())
    {
        seed.prepend(QStringLiteral("Machine"));
    }

    QString candidate = seed;
    for (int suffix = 2; findByAlias(candidate) != nullptr; ++suffix)
    {
        candidate = seed + QString::number(suffix);
    }

    return candidate;
}

void SMIncludeModel::prepareNewEntry(IncludeEntry& entry) const
{
    if (includeKindOf(entry.getLocation(), FsmExtension) != eIncludeKind::Document)
        return;

    // One registration per file, hosted by as many states as the user likes. The alias is what
    // those states name, so it is filled in here rather than left to a follow-up edit.
    entry.setAlias(uniqueAlias(QFileInfo(entry.getLocation()).completeBaseName()));
    const SMImportResolver::Resolution resolution = SMImportResolver::resolve(mFacade.getData(), entry);
    if (resolution.isResolved())
    {
        entry.setVersion(resolution.actualVersion);
    }
}

void SMIncludeModel::setAlias(uint32_t id, const QString& alias)
{
    IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || alias.isEmpty() || (alias == entry->getAlias()) || (findByAlias(alias) != nullptr))
        return;

    const QString oldAlias{ entry->getAlias() };
    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { IncludeEntry* e = facade->getData().getIncludes().findElement(id); return (e != nullptr ? e->getAlias() : QString()); };
    auto setter = [facade, id](const QString& value) { IncludeEntry* e = facade->getData().getIncludes().findElement(id); if (e != nullptr) e->setAlias(value); };

    SMCompositeCommand* composite = new SMCompositeCommand(mFacade.getData(), getNotifier(), QObject::tr("Rename submachine"));
    new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Import, getter, setter, alias, QObject::tr("Rename submachine"), composite);
    new SMRewriteReferencesCommand(mFacade.getData(), getNotifier(), SMReferences::eTarget::Import, id, oldAlias, alias, QObject::tr("Rename submachine"), composite);
    mFacade.getUndoStack().push(composite);
}

bool SMIncludeModel::updateVersion(uint32_t id)
{
    const IncludeEntry* entry = findInclude(id);
    if ((entry == nullptr) || (kindOf(id) != eIncludeKind::Document))
        return false;

    const SMImportResolver::Resolution resolution = SMImportResolver::resolve(mFacade.getData(), *entry);
    if ((resolution.isResolved() == false) || (resolution.actualVersion == entry->getVersion()))
        return false;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> VersionNumber { IncludeEntry* e = facade->getData().getIncludes().findElement(id); return (e != nullptr ? e->getVersion() : VersionNumber()); };
    auto setter = [facade, id](const VersionNumber& value) { IncludeEntry* e = facade->getData().getIncludes().findElement(id); if (e != nullptr) e->setVersion(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<VersionNumber>(getNotifier(), id, eDocElementKind::Import, getter, setter, resolution.actualVersion, QObject::tr("Update import version")));
    return true;
}

eDocElementKind SMIncludeModel::kindOfLocation(const QString& location) const
{
    return (includeKindOf(location, FsmExtension) == eIncludeKind::Document
            ? eDocElementKind::Import : eDocElementKind::Include);
}
