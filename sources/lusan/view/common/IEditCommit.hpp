#ifndef LUSAN_VIEW_COMMON_IEDITCOMMIT_HPP
#define LUSAN_VIEW_COMMON_IEDITCOMMIT_HPP
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
 *  \file        lusan/view/common/IEditCommit.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, contract of an editor page holding an uncommitted edit.
 *
 ************************************************************************/

/**
 * \class   IEditCommit
 * \brief   Implemented by a page whose multi-line fields hand their text to the document only
 *          when the field loses the keyboard focus. Saving from the keyboard moves no focus, so
 *          the host asks every page it owns to hand over what it holds before it writes the file.
 **/
class IEditCommit
{
protected:
    IEditCommit(void) = default;
    ~IEditCommit(void) = default;

public:
    /**
     * \brief   Applies the text the fields of this page are still holding. Applying an unchanged
     *          value does nothing, so this is safe to call at any moment and as often as needed.
     **/
    virtual void commitPendingEdits(void) = 0;
};

#endif  // LUSAN_VIEW_COMMON_IEDITCOMMIT_HPP
