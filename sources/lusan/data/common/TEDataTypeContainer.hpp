#ifndef LUSAN_DATA_COMMON_TEDATATYPECONTAINER_HPP
#define LUSAN_DATA_COMMON_TEDATATYPECONTAINER_HPP
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
 *  \file        lusan/data/common/TEDataTypeContainer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Template Data Type Container.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

//////////////////////////////////////////////////////////////////////////
// TEDataTypeContainer class template declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   TEDataTypeContainer
 * \brief   Template class for managing a list of data type fields.
 **/
template<class FieldType>
class TEDataTypeContainer   : public TEDataContainer< FieldType, DataTypeCustom >
{
//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Initializes by category.
     * \param   parent  The parent element.
     **/
    TEDataTypeContainer(ElementBase * parent = nullptr);
    
    /**
     * \brief   Initializes by category.
     * \param   id          The ID of the data type.
     * \param   parent  The parent element.
     **/
    explicit TEDataTypeContainer(uint32_t id, ElementBase * parent = nullptr);
    
    /**
     * \brief   Initializes by category.
     * \param   category    The category of the data type.
     * \param   parent  The parent element.
     **/
    explicit TEDataTypeContainer(DataTypeBase::eCategory category, ElementBase * parent = nullptr);
    
    /**
     * \brief   Constructor with name initialization.
     * \param   category    The category of the data type.
     * \param   name        The name of the data type.
     * \param   id          The ID of the data type.
     * \param   parent      The parent element.
     **/
    TEDataTypeContainer(DataTypeBase::eCategory category, const QString& name, uint32_t id, ElementBase* parent = nullptr);

public:
    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    TEDataTypeContainer(const TEDataTypeContainer<FieldType>& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    TEDataTypeContainer(TEDataTypeContainer<FieldType>&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    TEDataTypeContainer& operator = (const TEDataTypeContainer<FieldType>& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    TEDataTypeContainer& operator = (TEDataTypeContainer<FieldType>&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks if the object is valid.
     * \return  True if the object is valid, false otherwise.
     **/
    virtual bool isValid() const override;

    /**
     * \brief   Invalidates the object.
     **/
    void invalidate();
};

//////////////////////////////////////////////////////////////////////////
// TEDataTypeContainer class template implementation
//////////////////////////////////////////////////////////////////////////

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(ElementBase* parent)
    : TEDataContainer< FieldType, DataTypeCustom >(parent)
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(uint32_t id, ElementBase* parent)
    : TEDataContainer< FieldType, DataTypeCustom >(id, parent)
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(DataTypeBase::eCategory category, ElementBase* parent /*= nullptr*/)
    : TEDataContainer< FieldType, DataTypeCustom >(parent)
{
    DataTypeCustom::mCategory = category;
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(  DataTypeBase::eCategory category
                                                    , const QString& name
                                                    , uint32_t id
                                                    , ElementBase* parent /*= nullptr*/)
    : TEDataContainer< FieldType, DataTypeCustom >(id, parent)
{
    DataTypeCustom::mCategory   = category;
    DataTypeCustom::mName       = name;
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(const TEDataTypeContainer<FieldType>& src)
    : TEDataContainer< FieldType, DataTypeCustom >(src)
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(TEDataTypeContainer<FieldType>&& src) noexcept
    : TEDataContainer< FieldType, DataTypeCustom >(std::move(src))
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>& TEDataTypeContainer<FieldType>::operator = (const TEDataTypeContainer<FieldType>& other)
{
    using BaseClass  = TEDataContainer< FieldType, DataTypeCustom >;
    BaseClass::operator = (static_cast<const BaseClass &>(other));
    
    return *this;
}

template<class FieldType>
TEDataTypeContainer<FieldType>& TEDataTypeContainer<FieldType>::operator = (TEDataTypeContainer<FieldType>&& other) noexcept
{
    using BaseClass  = TEDataContainer< FieldType, DataTypeCustom >;
    BaseClass::operator = (std::move<BaseClass &&>(other));
    return *this;
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::isValid() const
{
    using BaseClass  = TEDataContainer< FieldType, DataTypeCustom >;
    return (BaseClass::mElementList.isEmpty() == false);
}

template<class FieldType>
void TEDataTypeContainer<FieldType>::invalidate()
{
    using BaseClass  = TEDataContainer< FieldType, DataTypeCustom >;
    return BaseClass::mElementList.clear();
}

#endif // LUSAN_DATA_COMMON_TEDATATYPECONTAINER_HPP
