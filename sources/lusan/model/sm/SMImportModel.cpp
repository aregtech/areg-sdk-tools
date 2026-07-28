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
 *  \file        lusan/model/sm/SMImportModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM submachine imports page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMImportModel.hpp"

#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/model/sm/SMRenameCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

SMImportModel::SMImportModel(StateMachineModel& facade)
    : mFacade(facade)
{
}

const QList<SMImportEntry>& SMImportModel::getImports() const
{
    return imports().getElements();
}

int SMImportModel::getImportCount() const
{
    return imports().getElementCount();
}

SMImportEntry* SMImportModel::findImport(const QString& alias) const
{
    return imports().findElement(alias);
}

SMImportEntry* SMImportModel::findImport(uint32_t id) const
{
    return imports().findElement(id);
}

int SMImportModel::findIndex(uint32_t id) const
{
    return imports().findIndex(id);
}

QStringList SMImportModel::getAliases() const
{
    QStringList result;
    for (const SMImportEntry& entry : imports().getElements())
    {
        result.append(entry.getName());
    }

    return result;
}

DocModelNotifier& SMImportModel::getNotifier() const
{
    return mFacade.getNotifier();
}

bool SMImportModel::isReadOnly() const
{
    return mFacade.isReadOnly();
}

SMImportResolver::Resolution SMImportModel::resolutionOf(uint32_t id) const
{
    const SMImportEntry* entry = findImport(id);
    return (entry != nullptr ? SMImportResolver::resolve(mFacade.getData(), *entry) : SMImportResolver::Resolution{});
}

QString SMImportModel::storableLocation(const QString& absoluteFilePath) const
{
    return SMImportResolver::storableLocation(mFacade.getData(), absoluteFilePath);
}

QList<SMReferences::Use> SMImportModel::whereUsed(uint32_t id) const
{
    const SMImportEntry* entry = findImport(id);
    return (entry != nullptr
            ? SMReferences::whereUsed(mFacade.getData(), SMReferences::eTarget::Import, entry->getName(), id)
            : QList<SMReferences::Use>());
}

SMImportEntry* SMImportModel::createImport(const QString& alias, const QString& location)
{
    if (alias.isEmpty() || (findImport(alias) != nullptr))
        return nullptr;

    SMImportEntry entry(0, alias, location, VersionNumber(), nullptr);
    // Pinning at registration time is the only automatic pin: the user picked this file now, so
    // this is the version they designed against. Every later change of the pin is an explicit
    // Update, because silently accepting a changed import is how a machine starts lying.
    const SMImportResolver::Resolution resolution = SMImportResolver::resolve(mFacade.getData(), entry);
    if (resolution.isResolved())
    {
        entry.setVersion(resolution.actualVersion);
    }

    mFacade.getUndoStack().push(new TDocAddCommand<SMImportEntry, DocumentElem>(getNotifier(), imports(), std::move(entry), eDocElementKind::Import, QObject::tr("Add import")));
    return findImport(alias);
}

void SMImportModel::deleteImport(uint32_t id)
{
    mFacade.getUndoStack().push(new TDocRemoveCommand<SMImportEntry, DocumentElem>(getNotifier(), imports(), id, eDocElementKind::Import, QObject::tr("Delete import")));
}

void SMImportModel::swapImports(uint32_t firstId, uint32_t secondId)
{
    const int index1 = imports().findIndex(firstId);
    const int index2 = imports().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mFacade.getUndoStack().push(new TDocReorderCommand<SMImportEntry, DocumentElem>(getNotifier(), imports(), index1, index2, 0u, eDocElementKind::Import, QObject::tr("Reorder imports")));
}

void SMImportModel::setName(uint32_t id, const QString& alias)
{
    SMImportEntry* entry = findImport(id);
    if ((entry == nullptr) || alias.isEmpty() || (alias == entry->getName()) || (findImport(alias) != nullptr))
        return;

    const QString oldName{ entry->getName() };
    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMImportEntry* e = facade->getData().getImports().findElement(id); return (e != nullptr ? e->getName() : QString()); };
    auto setter = [facade, id](const QString& value) { SMImportEntry* e = facade->getData().getImports().findElement(id); if (e != nullptr) e->setName(value); };

    SMCompositeCommand* composite = new SMCompositeCommand(mFacade.getData(), getNotifier(), QObject::tr("Rename import"));
    new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Import, getter, setter, alias, QObject::tr("Rename import"), composite);
    new SMRewriteReferencesCommand(mFacade.getData(), getNotifier(), SMReferences::eTarget::Import, id, oldName, alias, QObject::tr("Rename import"), composite);
    mFacade.getUndoStack().push(composite);
}

void SMImportModel::setLocation(uint32_t id, const QString& location)
{
    SMImportEntry* entry = findImport(id);
    if ((entry == nullptr) || (location == entry->getLocation()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMImportEntry* e = facade->getData().getImports().findElement(id); return (e != nullptr ? e->getLocation() : QString()); };
    auto setter = [facade, id](const QString& value) { SMImportEntry* e = facade->getData().getImports().findElement(id); if (e != nullptr) e->setLocation(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Import, getter, setter, location, QObject::tr("Set import location")));
}

void SMImportModel::setDescription(uint32_t id, const QString& text)
{
    SMImportEntry* entry = findImport(id);
    if ((entry == nullptr) || (text == entry->getDescription()))
        return;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> QString { SMImportEntry* e = facade->getData().getImports().findElement(id); return (e != nullptr ? e->getDescription() : QString()); };
    auto setter = [facade, id](const QString& value) { SMImportEntry* e = facade->getData().getImports().findElement(id); if (e != nullptr) e->setDescription(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::Import, getter, setter, text, QObject::tr("Set description")));
}

bool SMImportModel::updateVersion(uint32_t id)
{
    const SMImportEntry* entry = findImport(id);
    if (entry == nullptr)
        return false;

    const SMImportResolver::Resolution resolution = SMImportResolver::resolve(mFacade.getData(), *entry);
    if ((resolution.isResolved() == false) || (resolution.actualVersion == entry->getVersion()))
        return false;

    StateMachineModel* facade = &mFacade;
    auto getter = [facade, id]() -> VersionNumber { SMImportEntry* e = facade->getData().getImports().findElement(id); return (e != nullptr ? e->getVersion() : VersionNumber()); };
    auto setter = [facade, id](const VersionNumber& value) { SMImportEntry* e = facade->getData().getImports().findElement(id); if (e != nullptr) e->setVersion(value); };
    mFacade.getUndoStack().push(new TDocSetPropertyCommand<VersionNumber>(getNotifier(), id, eDocElementKind::Import, getter, setter, resolution.actualVersion, QObject::tr("Update import version")));
    return true;
}

const SMImportData& SMImportModel::imports() const
{
    return mFacade.getData().getImports();
}

SMImportData& SMImportModel::imports()
{
    return mFacade.getData().getImports();
}
