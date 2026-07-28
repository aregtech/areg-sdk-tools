#ifndef LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
#define LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
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
 *  \file        lusan/model/sm/SMIncludeModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMIncludeData.hpp"
#include "lusan/data/sm/SMImportResolver.hpp"
#include "lusan/data/sm/SMReferences.hpp"

#include <QList>
#include <QString>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class DocModelNotifier;
enum class eDocElementKind;

/**
 * \class   SMIncludeModel
 * \brief   The Includes page model and the single owner of the `IncludeList` section: header
 *          files, data type documents and imported state machines all live in it. Every edit
 *          goes through an undo command, so the page never mutates an `IncludeEntry` directly.
 *
 *          An include is identified by its location, which is also its unique name. An imported
 *          machine additionally carries an alias -- the name a state's `Submachine` attribute
 *          uses -- and the version pinned when it was registered. Alias lookup is an explicit
 *          scan, never findElement(): the container matches on the location.
 *
 *          Resolution (does the file exist, does it parse, what version is in it) is asked of
 *          the shared resolver on demand and never cached: the file lives outside the document
 *          and can change while the editor is open.
 **/
class SMIncludeModel
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eImportRefusal
     * \brief   Why a candidate machine cannot be registered.
     **/
    enum class eImportRefusal
    {
          None          //!< The file can be imported.
        , HostNotSaved  //!< This document has never been saved, so it has no folder to resolve against.
        , SelfImport    //!< The candidate is the host document itself.
        , Cycle         //!< Importing it would close a cycle back to the host.
        , TooDeep       //!< The candidate's own imports already reach MAX_IMPORT_DEPTH.
        , Unreadable    //!< The file cannot be parsed as a state machine.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMIncludeModel(StateMachineModel& facade);

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    const QList<IncludeEntry>& getIncludes() const;
    int getIncludeCount() const;

    IncludeEntry* findInclude(const QString& location) const;
    IncludeEntry* findInclude(uint32_t id) const;
    int findIndex(uint32_t id) const;

    //!< The machine include registered under \p alias, or nullptr.
    const IncludeEntry* findByAlias(const QString& alias) const;

    //!< What the include at \p id points at, derived from its file extension.
    eIncludeKind kindOf(uint32_t id) const;

    //!< The aliases of every registered machine, in document order (the submachine picker's list).
    QStringList getAliases() const;

    DocModelNotifier& getNotifier() const;

    //!< True while the document refuses every change (a read-only import view).
    bool isReadOnly() const;

    /**
     * \brief   What the imported machine currently points at: the resolved file, its parsed
     *          content and its actual version.
     **/
    SMImportResolver::Resolution resolutionOf(uint32_t id) const;

    /**
     * \brief   The location to store for a file the user picked in a browse dialog, relative to
     *          the host document when that is possible.
     **/
    QString storableLocation(const QString& absoluteFilePath) const;

    /**
     * \brief   The absolute path a stored location denotes, resolved against the host document's
     *          own directory. Empty when it cannot be resolved (an unsaved host, no location).
     **/
    QString absolutePathOf(const QString& location) const;

    /**
     * \brief   The states that host the given machine import, for the delete confirmation and
     *          the where-used popup.
     **/
    QList<SMReferences::Use> whereUsed(uint32_t id) const;

    /**
     * \brief   Tests a candidate machine file before it is registered. \p chain receives the
     *          offending document chain for Cycle and TooDeep, for display.
     **/
    eImportRefusal canImport(const QString& absoluteFilePath, QStringList& chain) const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Registers an include. A `.fsml` location also gets a default alias derived from
     *          the file name and, when the file resolves, the version pinned from it -- the one
     *          automatic pin; every later change of the pin is the user's explicit Update.
     * \return  The created entry, or nullptr when the location is already registered.
     **/
    IncludeEntry* createInclude(const QString& location);
    IncludeEntry* insertInclude(int position, const QString& location);
    void deleteInclude(uint32_t id);
    void swapIncludes(uint32_t firstId, uint32_t secondId);

    //!< The location is the include's unique name, so this is both the rename and the path edit.
    void setLocation(uint32_t id, const QString& location);
    void setDescription(uint32_t id, const QString& text);
    void setDeprecated(uint32_t id, bool deprecated);
    void setDeprecateHint(uint32_t id, const QString& hint);

    //!< Renames the alias and rewrites every hosting state's `Submachine` in the same undo step.
    void setAlias(uint32_t id, const QString& alias);

    /**
     * \brief   Re-pins the recorded version to the imported document's current one (the Update
     *          button). Does nothing when the import does not resolve or already matches.
     * \return  True when a new pin was pushed.
     **/
    bool updateVersion(uint32_t id);

    //!< A unique, identifier-safe alias for a newly registered file, derived from its base name.
    QString uniqueAlias(const QString& baseName) const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    const SMIncludeData& includes() const;
    SMIncludeData& includes();

    //!< The notifier kind of one row: a machine import announces itself as Import, everything
    //!< else as Include. The kind follows the row, not the container that now holds both.
    static eDocElementKind kindFor(const QString& location);
    eDocElementKind kindFor(uint32_t id) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
