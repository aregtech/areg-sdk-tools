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
#include "lusan/model/common/IncludeModel.hpp"

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

/**
 * \class   SMIncludeModel
 * \brief   The FSM Includes page model: the shared \ref IncludeModel plus what only a state
 *          machine can say about a row. A `.fsml` include is an imported machine, so it carries
 *          an alias -- the name a state's `Submachine` attribute uses -- and the version pinned
 *          when it was registered. Alias lookup is an explicit scan, never findInclude(): the
 *          container matches on the location.
 *
 *          Resolution (does the file exist, does it parse, what version is in it) is asked of
 *          the shared resolver on demand and never cached: the file lives outside the document
 *          and can change while the editor is open.
 **/
class SMIncludeModel : public IncludeModel
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

    virtual ~SMIncludeModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Reads
//////////////////////////////////////////////////////////////////////////
public:
    //!< The machine include registered under \p alias, or nullptr.
    const IncludeEntry* findByAlias(const QString& alias) const;

    //!< What the include at \p id points at, derived from its file extension.
    eIncludeKind kindOf(uint32_t id) const;

    //!< The aliases of every registered machine, in document order (the submachine picker's list).
    QStringList getAliases(void) const;

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

    //!< A unique, identifier-safe alias for a newly registered file, derived from its base name.
    QString uniqueAlias(const QString& baseName) const;

//////////////////////////////////////////////////////////////////////////
// Mutations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Renames the alias and rewrites every hosting state's `Submachine` in the same undo step.
    void setAlias(uint32_t id, const QString& alias);

    /**
     * \brief   Re-pins the recorded version to the imported document's current one (the Update
     *          button). Does nothing when the import does not resolve or already matches.
     * \return  True when a new pin was pushed.
     **/
    bool updateVersion(uint32_t id);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isReadOnly(void) const override;

protected:
    /**
     * \brief   A machine import announces itself as Import, everything else as Include. The kind
     *          follows the row, not the container that now holds both.
     **/
    virtual eDocElementKind kindOfLocation(const QString& location) const override;

    /**
     * \brief   A new `.fsml` row gets a default alias derived from the file name and, when the
     *          file resolves, the version pinned from it -- the one automatic pin; every later
     *          change of the pin is the user's explicit Update.
     **/
    virtual void prepareNewEntry(IncludeEntry& entry) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mFacade;
};

#endif  // LUSAN_MODEL_SM_SMINCLUDEMODEL_HPP
