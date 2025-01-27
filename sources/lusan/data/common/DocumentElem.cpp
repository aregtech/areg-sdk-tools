/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/DocumentElem.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Document element object to save and read data from document.
 *
 ************************************************************************/

#include "lusan/data/common/DocumentElem.hpp"

DocumentElem::DocumentElem(ElementBase* parent)
    : ElementBase(parent)
{
}

DocumentElem::DocumentElem(unsigned int id, ElementBase* parent)
    : ElementBase(id, parent)
{
}
