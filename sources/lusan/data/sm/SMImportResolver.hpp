#ifndef LUSAN_DATA_SM_SMIMPORTRESOLVER_HPP
#define LUSAN_DATA_SM_SMIMPORTRESOLVER_HPP
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
 *  \file        lusan/data/sm/SMImportResolver.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, submachine import resolution and cycle detection.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/VersionNumber.hpp"

#include <QString>
#include <QStringList>

#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMImportEntry;
class StateMachineData;

/**
 * \namespace   SMImportResolver
 * \brief   Turns a `MachineImport` registration into the document it names: path resolution
 *          against the host, the parsed content (through \ref SMDocumentCache), the imported
 *          document's actual version, and the cycle check.
 *
 *          It lives in the data layer because the code generator resolves imports too, and
 *          because keeping "what does this alias point at" in one place is what stops the
 *          canvas, the validator and the Includes page from each answering it differently.
 *          Resolution is never stored on the entry -- the file can change under the editor,
 *          so the answer is computed where it is needed and the cache absorbs the cost.
 **/
namespace SMImportResolver
{
    /**
     * \enum    eState
     * \brief   The outcome of resolving one import.
     **/
    enum class eState
    {
          Resolved      //!< The file exists and parsed.
        , NoLocation    //!< The registration carries no location at all.
        , NotFound      //!< The location does not resolve to an existing file.
        , ParseFailed   //!< The file exists but is not a readable `.fsml` document.
    };

    /**
     * \struct  Resolution
     * \brief   What one import alias currently points at.
     **/
    struct Resolution
    {
        eState                                  state { eState::NoLocation };
        QString                                 absolutePath;   //!< Empty unless the location could be made absolute.
        std::shared_ptr<const StateMachineData> document;       //!< Null unless \a state is Resolved.
        VersionNumber                           actualVersion;  //!< The imported `Overview@Version`; invalid unless resolved.

        inline bool isResolved() const { return (state == eState::Resolved) && (document != nullptr); }
    };

    /**
     * \brief   The absolute path a stored location denotes. Relative locations are resolved
     *          against the host document's own directory; an absolute location is returned
     *          cleaned. An unsaved host has no directory to resolve against, so a relative
     *          location yields an empty string there.
     **/
    QString absolutePath(const StateMachineData& host, const QString& location);

    /**
     * \brief   The location to store for a file the user picked. Relative to the host's own
     *          directory whenever that keeps the path inside a shared tree, absolute otherwise
     *          (a different drive, or an unsaved host), so moving a project directory does not
     *          break every import in it.
     **/
    QString storableLocation(const StateMachineData& host, const QString& absoluteFilePath);

    /**
     * \brief   Resolves one import registration of \p host.
     **/
    Resolution resolve(const StateMachineData& host, const SMImportEntry& entry);

    /**
     * \brief   Detects an import cycle reachable through \p entry: the host importing itself,
     *          directly or through any chain of imports.
     * \param   host    The importing document.
     * \param   entry   The import registration to walk from.
     * \param   chain   Receives the document names along the cycle, host first, when one is
     *                  found; untouched otherwise.
     * \return  True when following the import leads back to the host.
     **/
    bool findCycle(const StateMachineData& host, const SMImportEntry& entry, QStringList& chain);
}

#endif  // LUSAN_DATA_SM_SMIMPORTRESOLVER_HPP
