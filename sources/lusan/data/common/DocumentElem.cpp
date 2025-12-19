/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/DocumentElem.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Document element object to save and read data from document.
 *
 ************************************************************************/

#include "lusan/data/common/DocumentElem.hpp"

#include <QXmlStreamWriter>

DocumentElem::DocumentElem(ElementBase* parent)
    : ElementBase(parent)
{
}

DocumentElem::DocumentElem(unsigned int id, ElementBase* parent)
    : ElementBase(id, parent)
{
}

DocumentElem::DocumentElem(const DocumentElem & src)
    : ElementBase(static_cast<const ElementBase &>(src))
{
}

DocumentElem::DocumentElem(DocumentElem&& src) noexcept
    : ElementBase(std::move(src))
{
}

DocumentElem& DocumentElem::operator = (const DocumentElem& src)
{
    ElementBase::operator = (src);
    return (*this);
}

DocumentElem& DocumentElem::operator = (DocumentElem&& src) noexcept
{
    ElementBase::operator = (std::move(src));
    return (*this);
}

void DocumentElem::writeTextElem(QXmlStreamWriter& xml, const char* elemName, const QString elemValue, bool skipIfEmpty) const
{
    if (elemName == nullptr)
        return;

    if (elemValue.isEmpty() == false)
    {
        xml.writeTextElement(elemName, elemValue);
    }
    else if (skipIfEmpty == false)
    {
        xml.writeEmptyElement(elemName);
    }
}
