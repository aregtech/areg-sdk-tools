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
#include <QList>

//////////////////////////////////////////////////////////////////////////
// TEDataTypeContainer class template declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   TEDataTypeContainer
 * \brief   Template class for managing a list of data type fields.
 **/
template<class FieldType>
class TEDataTypeContainer : public DataTypeCustom
{
//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Initializes by category.
     * \param   category    The category of the data type.
     **/
    TEDataTypeContainer(DataTypeBase::eCategory category);

    /**
     * \brief   Constructor with name initialization.
     * \param   category    The category of the data type.
     * \param   name        The name of the data type.
     * \param   id          The ID of the data type.
     **/ 
    TEDataTypeContainer(DataTypeBase::eCategory category, const QString& name, uint32_t id);

public:
    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    TEDataTypeContainer(const TEDataTypeContainer& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    TEDataTypeContainer(TEDataTypeContainer&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    TEDataTypeContainer& operator = (const TEDataTypeContainer& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    TEDataTypeContainer& operator = (TEDataTypeContainer&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    inline const QList<FieldType>& getFields(void) const;

    inline void setFields(const QList<FieldType>& fields);

    /**
     * \brief   Adds a new field.
     * \param   field   The field to add.
     * \param   unique  If true, ensures the field is unique.
     * \return  True if the field was added, false otherwise.
     **/
    bool addField(const FieldType& field, bool unique = true);

    /**
     * \brief   Inserts an field at a specific position.
     * \param   index   The position to insert the field.
     * \param   field   The field to insert.
     * \param   unique  If true, ensures the field is unique.
     * \return  True if the field was inserted, false otherwise.
     **/
    bool insertField(int index, const FieldType& field, bool unique = true);

    /**
     * \brief   Removes an field by name.
     * \param   name    The name of the field to remove.
     * \return  True if the field was removed, false otherwise.
     **/
    bool removeField(const QString& name);

    /**
     * \brief   Removes all fields.
     **/
    void removeAllFields();

    /**
     * \brief   Finds an field by name.
     * \param   name    The name of the field to find.
     * \return  The field if found, nullptr otherwise.
     **/
    const FieldType* findField(const QString& name) const;

    /**
     * \brief   Finds the index of an field by name.
     * \param   name    The name of the field to find.
     * \return  The index of the field if found, -1 otherwise.
     **/
    int findIndex(const QString& name) const;

    /**
     * \brief   Checks if an field exists by name.
     * \param   name    The name of the field to check.
     * \return  True if the field exists, false otherwise.
     **/
    bool hasField(const QString& name) const;

    /**
     * \brief   Checks if the object is valid.
     * \return  True if the object is valid, false otherwise.
     **/
    bool isValid() const override;

    /**
     * \brief   Invalidates the object.
     **/
    void invalidate();

//////////////////////////////////////////////////////////////////////////
// Protected methods
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Checks if the fields are unique.
     * \return  True if the fields are unique, false otherwise.
     **/
    bool checkUniqueness(void) const;

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:
    QList<FieldType> mFieldList; //!< The list of fields.
};

//////////////////////////////////////////////////////////////////////////
// TEDataTypeContainer class template implementation
//////////////////////////////////////////////////////////////////////////

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(DataTypeBase::eCategory category)
    : DataTypeCustom(category)
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(DataTypeBase::eCategory category, const QString& name, uint32_t id)
    : DataTypeCustom(category, id, name)
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(const TEDataTypeContainer& src)
    : DataTypeCustom(src)
    , mFieldList(src.mFieldList)
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>::TEDataTypeContainer(TEDataTypeContainer&& src) noexcept
    : DataTypeCustom(std::move(src))
    , mFieldList(std::move(src.mFieldList))
{
}

template<class FieldType>
TEDataTypeContainer<FieldType>& TEDataTypeContainer<FieldType>::operator=(const TEDataTypeContainer& other)
{
    if (this != &other)
    {
        DataTypeBase::operator=(other);
        mFieldList = other.mFieldList;
    }
    return *this;
}

template<class FieldType>
TEDataTypeContainer<FieldType>& TEDataTypeContainer<FieldType>::operator=(TEDataTypeContainer&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator=(std::move(other));
        mFieldList = std::move(other.mFieldList);
    }
    return *this;
}

template<class FieldType>
inline const QList<FieldType>& TEDataTypeContainer<FieldType>::getFields(void) const
{
    return mFieldList;
}

template<class FieldType>
inline void TEDataTypeContainer<FieldType>::setFields(const QList<FieldType>& fields)
{
    mFieldList = fields;
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::addField(const FieldType& field, bool unique)
{
    if (unique)
    {
        const QString& name{ field.getName() };
        for (const FieldType& existingField : mFieldList)
        {
            if (existingField.getName() == name)
            {
                return false;
            }
        }
    }

    mFieldList.append(field);
    return true;
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::insertField(int index, const FieldType& field, bool unique)
{
    if (unique)
    {
        for (const FieldType& existingField : mFieldList)
        {
            if (existingField.getName() == field.getName())
            {
                return false;
            }
        }
    }

    if (index < 0 || index > mFieldList.size())
    {
        return false;
    }
    
    mFieldList.insert(index, field);
    return true;
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::removeField(const QString& name)
{
    for (int i = 0; i < mFieldList.size(); ++i)
    {
        if (mFieldList[i].getName() == name)
        {
            mFieldList.removeAt(i);
            return true;
        }
    }

    return false;
}

template<class FieldType>
void TEDataTypeContainer<FieldType>::removeAllFields()
{
    mFieldList.clear();
}

template<class FieldType>
const FieldType* TEDataTypeContainer<FieldType>::findField(const QString& name) const
{
    for (const FieldType& field : mFieldList)
    {
        if (field.getName() == name)
        {
            return &field;
        }
    }

    return nullptr;
}

template<class FieldType>
int TEDataTypeContainer<FieldType>::findIndex(const QString& name) const
{
    for (int i = 0; i < mFieldList.size(); ++i)
    {
        if (mFieldList[i].getName() == name)
        {
            return i;
        }
    }

    return -1;
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::hasField(const QString& name) const
{
    return findField(name) != nullptr;
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::isValid() const
{
    return !mFieldList.isEmpty();
}

template<class FieldType>
void TEDataTypeContainer<FieldType>::invalidate()
{
    mFieldList.clear();
}

template<class FieldType>
bool TEDataTypeContainer<FieldType>::checkUniqueness(void) const
{
    for (int i = 0; i < mFieldList.size(); ++i)
    {
        const QString& name{ mFieldList[i].getName() };
        for (int j = i + 1; j < mFieldList.size(); ++j)
        {
            if (name == mFieldList[j].getName())
            {
                return false;
            }
        }
    }

    return true;
}

#endif // LUSAN_DATA_COMMON_TEDATATYPECONTAINER_HPP
