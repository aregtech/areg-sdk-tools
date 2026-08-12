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
#include <QString>
#include <QStringList>
#include <QStringView>

/************************************************************************
 * Dependencies
 ************************************************************************/
class VersionNumber;

/**
 * \namespace   DocElementTable
 * \brief   Every element the three document formats define, where each one may appear, and
 *          whether it still counts. One table per document, one lookup: a reader that meets an
 *          element asks here instead of carrying its own list.
 *
 *          The table is read from the schema of the document, and there are two of those. The
 *          one delivered with the Areg SDK sits in `schema` beside the executable and wins
 *          whenever it is there. The one compiled into the application answers when it is not,
 *          so a build with no SDK schemas beside it validates exactly as well -- the copies are
 *          the same file, and the delivered one exists so a format change reaches an installed
 *          Lusan without rebuilding it.
 **/
namespace DocElementTable
{
    /**
     * \enum    eDocument
     * \brief   The document formats Lusan edits, each with its own element vocabulary.
     **/
    enum class eDocument
    {
          StateMachine      //!< `.fsml`
        , ServiceInterface  //!< `.siml`
        , DataType          //!< `.dtml`
    };

    /**
     * \enum    eSource
     * \brief   Which of the two copies of the format description answered.
     **/
    enum class eSource
    {
          BuiltIn       //!< Compiled into the application.
        , Delivered     //!< Found beside the executable, delivered with the Areg SDK.
    };

    /**
     * \struct  Row
     * \brief   One element of a format.
     **/
    struct Row
    {
        QString     name;           //!< The element name.
        QStringList parents;        //!< The elements it may sit in; empty for the document element.
    };

    /**
     * \brief   The newest state machine format this build reads. A document declaring a newer
     *          one is refused, because an element this build has never heard of cannot be
     *          edited safely.
     **/
    const VersionNumber& maxFormatVersion();

    /**
     * \brief   The row \p doc has for \p name, or nullptr when that format defines no such
     *          element.
     **/
    const Row* find(eDocument doc, QStringView name);

    /**
     * \brief   True when \p name is an element \p doc defines and \p parent is one of the
     *          elements it may sit in.
     * \param   doc     The document format being read.
     * \param   name    The element name as written in the document.
     * \param   parent  The name of the element that contains it, empty at the document level.
     **/
    bool accepts(eDocument doc, QStringView name, QStringView parent);

    /**
     * \brief   The document element of \p doc, which is the tag its files open with.
     **/
    QString documentElement(eDocument doc);

    /**
     * \brief   Which copy of the format description \p doc was read from.
     **/
    eSource source(eDocument doc);

    /**
     * \brief   The file the format description of \p doc was read from, for a status line.
     **/
    QString sourcePath(eDocument doc);

    /**
     * \brief   Reads every format description again. The tables are built once on first use;
     *          this exists so a test can put a schema beside the executable and see it taken.
     **/
    void reload();
}

#endif  // LUSAN_COMMON_DOCELEMENTTABLE_HPP
