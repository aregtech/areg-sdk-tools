#ifndef LUSAN_MODEL_SM_SMIMPORTMODEL_HPP
#define LUSAN_MODEL_SM_SMIMPORTMODEL_HPP
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
 *  \file        lusan/model/sm/SMImportModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM submachine imports page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMImportData.hpp"
#include "lusan/data/sm/SMImportResolver.hpp"
#include "lusan/data/sm/SMReferences.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DocModelNotifier;
class StateMachineModel;

/**
 * \class   SMImportModel
 * \brief   The Imports section of the Includes page. It reads the live `ImportList` through the
 *          facade and routes every edit through an undo command, like the sibling registry
 *          models, and adds the two things an import has and a plain registry entry does not:
 *          the file it points at, and the version pinned when it was registered.
 *
 *          Resolution (does the file exist, does it parse, what version is in it) is queried
 *          from the shared resolver on demand rather than cached here -- the file lives outside
 *          the document and can change while the editor is open.
 **/
class SMImportModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMImportModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<SMImportEntry>& getImports() const;
    int getImportCount() const;

    SMImportEntry* findImport(const QString& alias) const;
    SMImportEntry* findImport(uint32_t id) const;
    int findIndex(uint32_t id) const;

    //!< The aliases of every registered import, in document order (the submachine picker's list).
    QStringList getAliases() const;

    DocModelNotifier& getNotifier() const;

    //!< True while the document refuses every change (a read-only import view).
    bool isReadOnly() const;

    /**
     * \brief   What the import currently points at: the resolved file, its parsed content and
     *          its actual version.
     **/
    SMImportResolver::Resolution resolutionOf(uint32_t id) const;

    /**
     * \brief   The location to store for a file the user picked in a browse dialog, relative to
     *          the host document when that is possible.
     **/
    QString storableLocation(const QString& absoluteFilePath) const;

    /**
     * \brief   The states that host the given import, for the delete confirmation and the
     *          where-used popup.
     **/
    QList<SMReferences::Use> whereUsed(uint32_t id) const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Registers an import. The version is pinned from the imported document when it
     *          resolves -- the one automatic pinning; every later change of the pin is the
     *          user's explicit Update.
     * \param   alias       The unique alias the states will name.
     * \param   location    The stored location of the `.fsml` document.
     * \return  The created entry, or nullptr when the alias is taken.
     **/
    SMImportEntry* createImport(const QString& alias, const QString& location);
    void deleteImport(uint32_t id);
    void swapImports(uint32_t firstId, uint32_t secondId);

    //!< Renames the alias and rewrites every hosting state's `Submachine` in the same undo step.
    void setName(uint32_t id, const QString& alias);
    void setLocation(uint32_t id, const QString& location);
    void setDescription(uint32_t id, const QString& text);

    /**
     * \brief   Re-pins the recorded version to the imported document's current one (the Update
     *          button). Does nothing when the import does not resolve or already matches.
     * \return  True when a new pin was pushed.
     **/
    bool updateVersion(uint32_t id);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMImportData& imports() const;
    SMImportData& imports();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMIMPORTMODEL_HPP
