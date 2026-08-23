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
 *          The text is kept while the document is open, so a finding can quote the block. A
 *          save does not write it back.
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
 * \struct  DocUnknownAttribute
 * \brief   One attribute the format does not define, and the element it was written on.
 **/
struct DocUnknownAttribute
{
    QString element;    //!< The element the attribute sits on.
    QString name;       //!< The attribute name as written.
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
}

#endif  // LUSAN_MODEL_COMMON_DOCUNKNOWNSCAN_HPP
