#ifndef LUSAN_COMMON_IEIDGENERATOR_HPP
#define LUSAN_COMMON_IEIDGENERATOR_HPP
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
 *  \file        lusan/common/IEIdGenerator.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ID Generator interface.
 *
 ************************************************************************/

class IEIdGenerator
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    IEIdGenerator(void) = default;
    virtual ~IEIdGenerator(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the next available ID.
     **/
    virtual unsigned int getNextId(void) const = 0;

    /**
     * \brief   Sets the maximum ID. It checks before setting the maximum ID.
     *          If the passed ID is less than the actual saved, it is ignored
     * \param   id  The maximum ID to set.
     **/
    virtual void setMaxId(unsigned int id) const = 0;
};

#endif  // LUSAN_COMMON_IEIDGENERATOR_HPP
