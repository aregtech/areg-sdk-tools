#ifndef LUSAN_VIEW_COMMON_IEDATATYPECONSUMER_HPP
#define LUSAN_VIEW_COMMON_IEDATATYPECONSUMER_HPP
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
 *  \file        lusan/view/common/IEDataTypeConsumer.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type consumer object.
 *
 ************************************************************************/

/**
 * \class   IEDataTypeConsumer
 * \brief   A page that offers the document's data types, or declares its elements with them,
 *          and so has to follow what the Data Types page stores.
 *
 *          The page is told that the set changed, not what changed in it, and re-reads the
 *          document. A delta would not survive an undo: it names objects that a later redo
 *          replaces, and the page cannot tell an added type from a converted one by pointer.
 **/
class IEDataTypeConsumer
{
protected:
    IEDataTypeConsumer() = default;
    virtual ~IEDataTypeConsumer() = default;

public:

    /**
     * \brief   Triggered after any change to the document's data types: one was added, removed,
     *          renamed, converted, reordered, or the whole document was reloaded. The page
     *          re-reads the current set and re-resolves whatever it declared with it.
     **/
    virtual void dataTypesChanged();
};

#endif // LUSAN_VIEW_COMMON_IEDATATYPECONSUMER_HPP
