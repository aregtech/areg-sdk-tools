#ifndef LUSAN_DATA_DT_DATATYPEIMPORTRESOLVER_HPP
#define LUSAN_DATA_DT_DATATYPEIMPORTRESOLVER_HPP
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
 *  \file        lusan/data/dt/DataTypeImportResolver.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, resolution of the data type documents a document includes.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QString>
#include <QStringList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeDataSection;
class IncludeDataSection;

/**
 * \namespace   DataTypeImportResolver
 * \brief   Turns the `.dtml` rows of a document's include list into the types they contribute.
 *          A row names a file; the file's base name is the namespace, and the types it declares
 *          reach the reading document as `Space::Name`.
 *
 *          It lives in the data layer because the code generator resolves the same rows, and
 *          because keeping "what does this include bring in" in one place is what stops the Data
 *          Types page, the validator and the type pickers from each answering it differently.
 *          Nothing is stored on the include row: the file can change under the editor, so the
 *          answer is recomputed and \ref DTDocumentCache absorbs the cost.
 **/
namespace DataTypeImportResolver
{
    /**
     * \brief   The absolute path a stored location denotes. A relative location is measured from
     *          the host document's own directory; an absolute one is returned cleaned. An unsaved
     *          host has no directory, so a relative location yields an empty string there.
     * \param   hostFilePath    The file the reading document lives in, empty when unsaved.
     * \param   location        The location as the document stores it.
     **/
    QString absolutePath(const QString& hostFilePath, const QString& location);

    /**
     * \brief   The location to store for a file the author picked. Relative to the host's own
     *          directory whenever that keeps the path inside one tree, absolute otherwise, so
     *          moving a project directory does not break every include in it.
     **/
    QString storableLocation(const QString& hostFilePath, const QString& absoluteFilePath);

    /**
     * \brief   Rebuilds what the included data type documents contribute to \p types.
     *
     *          Every `.dtml` row of \p includes becomes one group, in include order, whether it
     *          resolves or not: an unresolved row still has to be reported, and it has to be
     *          reported against the row the author can see. A namespace an earlier row already
     *          claimed is refused rather than merged -- both would generate into one namespace.
     * \param   types           The reading document's data type section, updated in place.
     * \param   hostFilePath    The file the reading document lives in, empty when unsaved.
     * \param   includes        The reading document's include list.
     * \return  True when the resolved set differs from the one \p types held before, so a caller
     *          can stay quiet when a rebuild changed nothing.
     **/
    bool refresh(DataTypeDataSection& types, const QString& hostFilePath, const IncludeDataSection& includes);

    /**
     * \brief   The files \p types currently resolves against, absolute, without duplicates. What
     *          an editor has to watch to notice a data type document changing under it.
     **/
    QStringList resolvedPaths(const DataTypeDataSection& types);
}

#endif  // LUSAN_DATA_DT_DATATYPEIMPORTRESOLVER_HPP
