#ifndef LUSAN_COMMON_DOCELEMENTTABLE_HPP
#define LUSAN_COMMON_DOCELEMENTTABLE_HPP
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
 *  \file        lusan/common/DocElementTable.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the elements a document may contain and where each may sit.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QLatin1StringView>
#include <QStringView>

/************************************************************************
 * Dependencies
 ************************************************************************/
class VersionNumber;

/**
 * \namespace   DocElementTable
 * \brief   Every element the state machine format defines, where each one may appear, and
 *          whether it still counts. One table, one lookup: a reader that meets an element
 *          asks here instead of carrying its own list, so a tag is added or retired in a
 *          single row.
 *
 *          This is the built-in copy. A delivered description of the format wins over it when
 *          one is present beside the executable; with none, this is what validates, so it is
 *          kept complete rather than treated as a stand-in. The row shape is the delivered
 *          one's -- name, permitted parents, status -- so replacing the source touches no
 *          call site.
 **/
namespace DocElementTable
{
    /**
     * \enum    eStatus
     * \brief   What the format currently says about an element.
     **/
    enum class eStatus
    {
          Valid         //!< Part of the format; read, written and edited.
        , Deprecated    //!< Still read and written, but the replacement is preferred.
        , Removed       //!< No longer part of the format; a document carrying it does not generate.
    };

    /**
     * \struct  Row
     * \brief   One element of the format.
     **/
    struct Row
    {
        QLatin1StringView   name;           //!< The element name.
        QLatin1StringView   parents;        //!< The elements it may sit in, '|' separated; empty for the document element.
        eStatus             status;         //!< Whether the element still counts.
        QLatin1StringView   replacement;    //!< What to use instead, for a deprecated or removed element.
    };

    /**
     * \brief   The newest format this build reads. A document declaring a newer one is refused,
     *          because an element this build has never heard of cannot be edited safely.
     **/
    const VersionNumber& maxFormatVersion();

    /**
     * \brief   The row for \p name, or nullptr when the format defines no such element.
     **/
    const Row* find(QStringView name);

    /**
     * \brief   True when \p name is an element the format defines and \p parent is one of the
     *          elements it may sit in.
     * \param   name    The element name as written in the document.
     * \param   parent  The name of the element that contains it, empty at the document level.
     **/
    bool accepts(QStringView name, QStringView parent);
}

#endif  // LUSAN_COMMON_DOCELEMENTTABLE_HPP
