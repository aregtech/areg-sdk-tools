#ifndef LUSAN_COMMON_DOCSCHEMAREADER_HPP
#define LUSAN_COMMON_DOCSCHEMAREADER_HPP
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
 *  \file        lusan/common/DocSchemaReader.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, reads a document schema into the elements it declares.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

/**
 * \namespace   DocSchemaReader
 * \brief   Turns a schema file into the one question the editor asks of it: which elements
 *          exist and where each may appear.
 *
 *          This is not a validating parser. Qt 6 has none, and the editor does not need one:
 *          everything a schema states beyond element placement -- occurrence counts, attribute
 *          types, keys -- is either checked by the rule catalogue or is not the editor's
 *          business. What is read is the element graph, which is exactly what the readers
 *          cannot answer once a document is loaded.
 **/
namespace DocSchemaReader
{
    /**
     * \struct  Element
     * \brief   One element a schema declares.
     **/
    struct Element
    {
        QString     name;       //!< The XML tag.
        QStringList parents;    //!< The tags it may sit in; empty for the document element.
    };

    /**
     * \brief   The directory a delivered schema is looked for in: `schema` beside the executable,
     *          which is where the build puts the copy that travels with the Areg SDK.
     **/
    QString deliveryDirectory();

    /**
     * \brief   Every element \p schema declares, sorted by name. Empty when the text is not a
     *          schema this reader understands.
     **/
    QList<Element> read(const QByteArray& schema);

    /**
     * \brief   Reads the schema at \p path, which may be a file or a resource. Empty when the
     *          file is absent or unreadable.
     **/
    QList<Element> readFile(const QString& path);
}

#endif  // LUSAN_COMMON_DOCSCHEMAREADER_HPP
