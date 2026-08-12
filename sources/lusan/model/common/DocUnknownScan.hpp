#ifndef LUSAN_MODEL_COMMON_DOCUNKNOWNSCAN_HPP
#define LUSAN_MODEL_COMMON_DOCUNKNOWNSCAN_HPP
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
 *  \file        lusan/model/common/DocUnknownScan.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the elements of a document the format does not define.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/DocElementTable.hpp"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <cstdint>

/**
 * \struct  DocUnknownElement
 * \brief   One element the format does not define, where it was found, and the text of it.
 *
 *          The text is kept so a save puts the block back exactly as it was written. An
 *          editor that cannot show an element must still not destroy it: the author's way out
 *          of a refusal is to open the document in a build that understands it, and that only
 *          works while the block survives every save in between.
 **/
struct DocUnknownElement
{
    QString     name;       //!< The element name as written.
    QString     parent;     //!< The name of the element it sits in.
    int         line;       //!< The line it starts on, so the author can find the first one.
    uint32_t    ownerId;    //!< The ID of the nearest enclosing element that carries one, 0 at the root.
    QStringList wrappers;   //!< The element names between that owner and this block.
    QString     text;       //!< The block itself, verbatim.
};

/**
 * \namespace   DocUnknownScan
 * \brief   Reads a document once more, as text, and reports every element the format does not
 *          place. The model readers cannot answer this: an element they do not recognize is
 *          simply not built, so by the time the model exists the name and the line are gone.
 **/
namespace DocUnknownScan
{
    /**
     * \brief   Every element of \p xml that the format of \p doc does not define, or defines
     *          somewhere else, in document order.
     **/
    QList<DocUnknownElement> scan(DocElementTable::eDocument doc, const QByteArray& xml);

    /**
     * \brief   Puts the blocks of \p unknown back into \p written, each inside the element it
     *          was found in. A block whose element no longer exists is dropped, because the
     *          author deleted what held it.
     * \return  \p written unchanged when \p unknown is empty.
     **/
    QByteArray restore(DocElementTable::eDocument doc, const QByteArray& written
                     , const QList<DocUnknownElement>& unknown);
}

#endif  // LUSAN_MODEL_COMMON_DOCUNKNOWNSCAN_HPP
