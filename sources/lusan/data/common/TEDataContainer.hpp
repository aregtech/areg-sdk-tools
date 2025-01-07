// File: lusan/data/common/TEDataContainer.hpp

#ifndef LUSAN_DATA_COMMON_TEDATACONTAINER_HPP
#define LUSAN_DATA_COMMON_TEDATACONTAINER_HPP
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
 *  \file        lusan/data/common/TEDataContainer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Template Data Container.
 *
 ************************************************************************/

#include <QList>
#include "lusan/common/ElementBase.hpp"

//////////////////////////////////////////////////////////////////////////
// TEDataContainer class template declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   TEDataContainer
 * \brief   Template class for managing a list of data elements.
 **/
template<class Data, class ElemBase>
class TEDataContainer : public ElemBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    TEDataContainer(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id      The ID of the container.
     * \param   parent  The parent element.
     **/
    TEDataContainer(unsigned int id, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    TEDataContainer(const TEDataContainer& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    TEDataContainer(TEDataContainer&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    TEDataContainer& operator = (const TEDataContainer& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    TEDataContainer& operator = (TEDataContainer&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list of data elements.
     * \return  The list of data elements.
     **/
    const QList<Data>& getElements(void) const;

    /**
     * \brief   Sets the list of data elements.
     * \param   elements  The list of data elements to set.
     **/
    void setElements(const QList<Data>& elements);

    /**
     * \brief   Adds a new data element.
     * \param   element   The data element to add.
     * \param   unique    If true, ensures the element is unique.
     * \return  True if the element was added, false otherwise.
     **/
    bool addElement(Data&& element, bool unique = true);

    /**
     * \brief   Adds a new data element.
     * \param   element   The data element to add.
     * \param   unique    If true, ensures the element is unique.
     * \return  True if the element was added, false otherwise.
     **/
    bool addElement(const Data& element, bool unique = true);

    /**
     * \brief   Replaces a data element.
     * \param   oldElement    The data element to replace.
     * \param   newElement    The new data element.
     * \param   unique        If true, ensures the element is unique.
     * \return  True if the element was replaced, false otherwise.
     **/
    bool replaceElement(const Data& oldElement, Data&& newElement, bool unique = true);

    /**
     * \brief   Replaces a data element.
     * \param   oldElement    The data element to replace.
     * \param   newElement    The new data element.
     * \param   unique        If true, ensures the element is unique.
     * \return  True if the element was replaced, false otherwise.
     **/
    bool replaceElement(const Data& oldElement, const Data& newElement, bool unique = true);

    /**
     * \brief   Replaces a data element by name or ID.
     * \param   id          The ID of the element to replace.
     * \param   name        The name of the element to replace.
     * \param   newElement  The new data element.
     * \param   unique      If true, ensures the element is unique.
     * \return  True if the element was replaced, false otherwise.
     **/
    bool replaceElement(uint32_t id, Data&& newElement, bool unique = true);

    /**
     * \brief   Replaces a data element by name or ID.
     * \param   id          The ID of the element to replace.
     * \param   name        The name of the element to replace.
     * \param   newElement  The new data element.
     * \param   unique      If true, ensures the element is unique.
     * \return  True if the element was replaced, false otherwise.
     **/
    bool replaceElement(uint32_t id, const Data& newElement, bool unique = true);

    /**
     * \brief   Replaces a data element by name or ID.
     * \param   name        The name of the element to replace.
     * \param   newElement  The new data element.
     * \param   unique      If true, ensures the element is unique.
     * \return  True if the element was replaced, false otherwise.
     **/
    bool replaceElement(const QString& name, Data&& newElement, bool unique = true);

    /**
     * \brief   Replaces a data element by name or ID.
     * \param   name        The name of the element to replace.
     * \param   newElement  The new data element.
     * \param   unique      If true, ensures the element is unique.
     * \return  True if the element was replaced, false otherwise.
     **/
    bool replaceElement(const QString& name, const Data& newElement, bool unique = true);

    /**
     * \brief   Inserts a data element at a specific position.
     * \param   index   The position to insert the element.
     * \param   element The data element to insert.
     * \param   unique  If true, ensures the element is unique.
     * \return  True if the element was inserted, false otherwise.
     **/
    bool insertElement(int index, Data&& element, bool unique = true);

    /**
     * \brief   Inserts a data element at a specific position.
     * \param   index   The position to insert the element.
     * \param   element The data element to insert.
     * \param   unique  If true, ensures the element is unique.
     * \return  True if the element was inserted, false otherwise.
     **/
    bool insertElement(int index, const Data& element, bool unique = true);

    /**
     * \brief   Removes a data element by name.
     * \param   name    The name of the element to remove.
     * \return  True if the element was removed, false otherwise.
     **/
    bool removeElement(const QString& name);

    /**
     * \brief   Removes a data element by ID.
     * \param   id  The ID of the element to remove.
     * \return  True if the element was removed, false otherwise.
     **/
    bool removeElement(uint32_t id);

    /**
     * \brief   Removes all data elements.
     **/
    void removeAllElements();

    /**
     * \brief   Finds a data element by name.
     * \param   uniqueName   The unique name of the element to find.
     * \return  The element if found, nullptr otherwise.
     **/
    Data* findElement(const QString& uniqueName) const;

    /**
     * \brief   Finds a data element by ID.
     * \param   id  The ID of the element to find.
     * \return  The element if found, nullptr otherwise.
     **/
    Data* findElement(uint32_t id) const;

    /**
     * \brief   Finds the index of a data element by name.
     * \param   name    The name of the element to find.
     * \return  The index of the element if found, -1 otherwise.
     **/
    int findIndex(const QString& name) const;

    /**
     * \brief   Finds the index of a data element by ID.
     * \param   id  The ID of the element to find.
     * \return  The index of the element if found, -1 otherwise.
     **/
    int findIndex(uint32_t id) const;

    /**
     * \brief   Checks if a data element exists by name.
     * \param   name    The name of the element to check.
     * \return  True if the element exists, false otherwise.
     **/
    bool hasElement(const QString& name) const;

    /**
     * \brief   Checks if a data element exists by ID.
     * \param   id  The ID of the element to check.
     * \return  True if the element exists, false otherwise.
     **/
    bool hasElement(uint32_t id) const;

    /**
     * \brief   Checks if a data element exists by name.
     * \param   element The data element to check.
     * \return  True if the element exists, false otherwise.
     **/
    bool hasElement(const Data& element) const;

    /**
     * \brief   Sorts the elements by name.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     * \return  True if the elements were sorted, false otherwise.
     **/
    bool sortElementsByName(bool ascending = true);

//////////////////////////////////////////////////////////////////////////
// Protected methods
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Checks if the elements are unique.
     * \return  True if the elements are unique, false otherwise.
     **/
    bool checkUniqueness() const;

    /**
     * \brief   Checks if an element is unique.
     * \param   element The element to check.
     * \param   unique  If true, ensures the element is unique.
     * \return  Pointer to the element if found, nullptr otherwise.
     **/
    Data* checkElement(const Data& element, bool unique) const;

    /**
     * \brief   Checks if an element is updated.
     * \param   element The element to check.
     * \param   unique  If true, ensures the element is unique.
     * \return  True if the element is updated, false otherwise.
     **/
    bool checkUpdated(const Data& element, bool unique) const;

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:
    QList<Data> mElementList; //!< The list of data elements.
};

//////////////////////////////////////////////////////////////////////////
// TEDataContainer class template implementation
//////////////////////////////////////////////////////////////////////////

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(ElementBase* parent)
    : ElemBase(parent)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(unsigned int id, ElementBase* parent)
    : ElemBase(id, parent)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(const TEDataContainer& src)
    : ElemBase( src)
    , mElementList(src.mElementList)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(TEDataContainer&& src) noexcept
    : ElemBase(std::move(src))
    , mElementList(std::move(src.mElementList))
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>& TEDataContainer<Data, ElemBase>::operator = (const TEDataContainer& other)
{
    if (this != &other)
    {
        ElemBase::operator=(other);
        mElementList = other.mElementList;
    }

    return *this;
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>& TEDataContainer<Data, ElemBase>::operator = (TEDataContainer&& other) noexcept
{
    if (this != &other)
    {
        ElemBase::operator=(std::move(other));
        mElementList = std::move(other.mElementList);
    }

    return *this;
}

template<class Data, class ElemBase>
const QList<Data>& TEDataContainer<Data, ElemBase>::getElements(void) const
{
    return mElementList;
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::setElements(const QList<Data>& elements)
{
    mElementList = elements;
    for (auto& entry : mElementList)
    {
        if (entry.getParent() != this)
        {
            entry.setParent(this);
            ElemBase::setMaxId(entry.getId());
        }
    }
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::addElement(Data&& element, bool unique)
{
    if (checkUpdated(element, unique))
    {
        mElementList.append(std::move(element));
        Q_ASSERT(element.getId() != 0);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::addElement(const Data& element, bool unique)
{
    if (checkUpdated(element, unique))
    {
        mElementList.append(element);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(const Data& oldElement, Data&& newElement, bool unique)
{
    Data* element{ checkElement(oldElement, unique) };
    if (element != nullptr)
    {
        newElement.setParent(element->getParent());
        newElement.setId(element->getId());
        *element = std::move(newElement);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(const Data& oldElement, const Data& newElement, bool unique)
{
    Data* element{ checkElement(oldElement, unique) };
    if (element != nullptr)
    {
        newElement.setParent(element->getParent());
        newElement.setId(element->getId());
        *element = newElement;
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(uint32_t id, Data&& newElement, bool unique)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getId() == id)
        {
            newElement.setParent(mElementList[i].getParent());
            newElement.setId(mElementList[i].getId());
            mElementList[i] = std::move(newElement);
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(uint32_t id, const Data& newElement, bool unique)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getId() == id)
        {
            newElement.setParent(mElementList[i].getParent());
            newElement.setId(mElementList[i].getId());
            mElementList[i] = newElement;
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(const QString& name, Data&& newElement, bool unique)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getName() == name)
        {
            newElement.setParent(mElementList[i].getParent());
            newElement.setId(mElementList[i].getId());
            mElementList[i] = std::move(newElement);
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(const QString& name, const Data& newElement, bool unique)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getName() == name)
        {
            newElement.setParent(mElementList[i].getParent());
            newElement.setId(mElementList[i].getId());
            mElementList[i] = newElement;
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::insertElement(int index, Data&& element, bool unique)
{
    if (index < 0 || index > mElementList.size())
    {
        return false;
    }

    if (checkUpdated(element, unique))
    {
        mElementList.insert(index, std::move(element));
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::insertElement(int index, const Data& element, bool unique)
{
    if (index < 0 || index > mElementList.size())
    {
        return false;
    }

    if (checkUpdated(element, unique))
    {
        mElementList.insert(index, element);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::removeElement(uint32_t id)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getId() == id)
        {
            mElementList.removeAt(i);
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::removeElement(const QString& name)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getName() == name)
        {
            mElementList.removeAt(i);
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::removeAllElements()
{
    mElementList.clear();
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::findElement(const QString& uniqueName) const
{
    for (const Data& element : mElementList)
    {
        if (element.getName() == uniqueName)
        {
            return const_cast<Data *>(&element);
        }
    }

    return nullptr;
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::findElement(uint32_t id) const
{
    for (const Data& element : mElementList)
    {
        if (element.getId() == id)
        {
            return const_cast<Data *>(&element);
        }
    }

    return nullptr;
}

template<class Data, class ElemBase>
int TEDataContainer<Data, ElemBase>::findIndex(const QString& name) const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getName() == name)
        {
            return i;
        }
    }

    return -1;
}

template<class Data, class ElemBase>
int TEDataContainer<Data, ElemBase>::findIndex(uint32_t id) const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        if (mElementList[i].getId() == id)
        {
            return i;
        }
    }

    return -1;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::hasElement(const QString& name) const
{
    return findElement(name) != nullptr;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::hasElement(uint32_t id) const
{
    return findElement(id) != nullptr;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::hasElement(const Data& element) const
{
    return (element.getId() != 0 ? hasElement(element.getId()) : hasElement(element.getName()));
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::sortElementsByName(bool ascending)
{
    std::sort(mElementList.begin(), mElementList.end(), [ascending](const Data& left, const Data& right)
        {
            return (ascending ? left.getName() < right.getName() : left.getName() > right.getName());
        });

    return true;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::checkUniqueness() const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        const QString& name{ mElementList[i].getName() };
        for (int j = i + 1; j < mElementList.size(); ++j)
        {
            if (name == mElementList[j].getName())
            {
                return false;
            }
        }
    }

    return true;
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::checkElement(const Data& element, bool unique) const
{
    Data* found{ nullptr };

    if (element.getParent() != this)
    {
        element.setParent(this);
        element.setId(ElemBase::getNextId());
    }

    if (unique)
    {
        if (element.getId() != 0)
        {
            found = findElement(element.getId());
        }
        else
        {
            found = findElement(element.getName());
        }
    }

    return found;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::checkUpdated(const Data& element, bool unique) const
{
    return (checkElement(element, unique) == nullptr ? (element.getParent() == this) && (element.getId() != 0) : false);
}

#endif // LUSAN_DATA_COMMON_TEDATACONTAINER_HPP
