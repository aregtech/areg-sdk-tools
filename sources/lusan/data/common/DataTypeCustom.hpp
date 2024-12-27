#ifndef LUSAN_DATA_COMMON_DATATYPECUSTOM_HPP
#define LUSAN_DATA_COMMON_DATATYPECUSTOM_HPP
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
 *  \file        lusan/data/common/DataTypeCustom.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeBase.hpp"

/**
 * \class   DataTypeCustom
 * \brief   Represents a custom data type in the Lusan application.
 **/
class DataTypeCustom : public DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Constructor with initialization of category.
     * \param   category    The category of the data type.
     **/
    DataTypeCustom(DataTypeBase::eCategory category);
    
    /**
     * \brief   Constructor with initialization.
     * \param   category    The category of the data type.
     * \param   id          The ID of the data type.
     * \param   name        The name of the data type.
     **/
    DataTypeCustom(DataTypeBase::eCategory category, uint32_t id, const QString& name);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeCustom(const DataTypeCustom& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeCustom(DataTypeCustom&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeCustom& operator=(const DataTypeCustom& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeCustom& operator=(DataTypeCustom&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the ID of the data type.
     * \return  The ID of the data type.
     **/
    uint32_t getId() const;

    /**
     * \brief   Sets the ID of the data type.
     * \param   id  The ID of the data type.
     **/
    void setId(uint32_t id);

    /**
     * \brief   Checks if the data type is valid.
     * \return  True if the data type is valid, false otherwise.
     **/
    virtual bool isValid(void) const override;

    /**
     * \brief   Gets the type of the data type.
     * \return  The type of the data type.
     **/
    virtual QString getType() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    uint32_t    mId;    //!< The ID of the data type.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
public:
    DataTypeCustom(void) = delete;
};

#endif // LUSAN_DATA_COMMON_DATATYPECUSTOM_HPP

