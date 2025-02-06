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
#include "lusan/common/NELusanCommon.hpp"
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
    using ref = NELusanCommon::get_ref<Data>;
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

    TEDataContainer(const QList<Data>& entries, ElementBase* parent);

    TEDataContainer(QList<Data>&& entries, ElementBase* parent) noexcept;

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
    inline const QList<Data>& getElements(void) const;
    inline QList<Data>& getElements(void);

    /**
     * \brief   Checks if the container is empty.
     **/
    inline bool isEmpty(void) const;

    /**
     * \brief   Sets the list of data elements.
     * \param   elements  The list of data elements to set.
     **/
    void setElements(const QList<Data>& elements);
    void setElements(QList<Data>&& elements) noexcept;

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
    bool removeElement(const QString& name, Data* elemRemoved = nullptr);

    /**
     * \brief   Removes a data element by ID.
     * \param   id  The ID of the element to remove.
     * \return  True if the element was removed, false otherwise.
     **/
    bool removeElement(uint32_t id, Data* elemRemoved = nullptr);

    /**
     * \brief   Removes all data elements.
     **/
    inline void removeAllElements();

    /**
     * \brief   Finds a data element by name.
     * \param   uniqueName   The unique name of the element to find.
     * \return  The element if found, nullptr otherwise.
     **/
    Data* findElement(const QString& uniqueName) const;
    Data* findElement(const QString& uniqueName);

    /**
     * \brief   Finds a data element by ID.
     * \param   id  The ID of the element to find.
     * \return  The element if found, nullptr otherwise.
     **/
    Data* findElement(uint32_t id) const;
    Data* findElement(uint32_t id);

    /**
     * \brief   Finds the index of a data element by name.
     * \param   name    The name of the element to find.
     * \return  The index of the element if found, -1 otherwise.
     **/
    inline int findIndex(const Data& field) const;

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
    inline bool hasElement(const QString& name) const;

    /**
     * \brief   Checks if a data element exists by ID.
     * \param   id  The ID of the element to check.
     * \return  True if the element exists, false otherwise.
     **/
    inline bool hasElement(uint32_t id) const;

    /**
     * \brief   Checks if a data element exists by name.
     * \param   element The data element to check.
     * \return  True if the element exists, false otherwise.
     **/
    inline bool hasElement(const Data& element) const;

    /**
     * \brief   Sorts the elements by name.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortElementsByName(bool ascending = true);

    /**
     * \brief   Sorts the elements by ID.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortElementsById(bool ascending = true);

    /**
     * \brief   Checks whether the container has list of elements.
     * \return  True if the container object has elements, false otherwise.
     **/
    inline bool hasElements(void) const;

    /**
     * \brief   Returns the number of elements in the container.
     * \return  The number of elements in the container.
     **/
    inline int getElementCount(void) const;

    /**
     * \brief   Swaps 2 elements by given index.
     *          It will keep the element IDs in the list in the same order.
     **/
    void swapElements(int index1, int index2);

    /**
     * \brief   Swaps 2 elements by given values.
     *          It will keep the element IDs in the list in the same order.
     **/
    void swapElements(const Data& elem1, const Data& elem2);

    /**
     * \brief   Swaps 2 elements by given IDs.
     *          It will keep the IDs in the list in the same order.
     **/
    void swapElements(uint32_t elem1Id, uint32_t elem2Id);

    /**
     * \brief   Orders the elements by given list of IDs.
     * \param   orderedIds  The list of ordered IDs.
     * \return  True if the elements are ordered, false otherwise.
     **/
    bool orderElements(const QList<uint32_t>& orderedIds);

    /**
     * \brief   Set the ordered list IDs for elements in container,
     *          i.e. sets the IDs without changing the content of container.
     *          The number of entries in the list and in container should be same.
     *          The IDs in the order list should be unique.
     * \param   orderedIds  The list of ordered IDs.
     **/
    void setOrderedIds(const QList<uint32_t>& orderIds);

    /**
     * \brief   Sorts the elements by the given sorting type.
     * \param   sortingType The sorting type.
     **/
    void sortElements(NELusanCommon::eSortingType sortingType);

    /**
     * \brief   Resets the sorting state to stop sorting.
     **/
    inline void noSortElements(void);

    /**
     * \brief   Checks if the elements in the list are sorted.
     **/
    inline bool isSorted(void) const;

    /**
     * \brief   Checks if the elements are sorted by ID.
     **/
    inline bool isSortedById(void) const;

    /**
     * \brief   Checks if the elements are sorted by name.
     **/
    inline bool isSortedByName(void) const;

    /**
     * \brief   Checks if the elements are sorted by ID in ascending order.
     **/
    inline bool isSortedByIdAscending(void) const;

    /**
     * \brief   Checks if the elements are sorted by ID in descending order.
     **/
    inline bool isSortedByIdDescending(void) const;

//////////////////////////////////////////////////////////////////////////
// Protected methods
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Checks if the elements are unique.
     * \return  True if the elements are unique, false otherwise.
     **/
    bool checkUniqueness() const;inline 
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
    inline bool checkUpdated(const Data& element, bool unique) const;

    /**
     * \brief   Extracts and saves the IDs of the elements in the list.
     * \param   out_ids     The list to save IDs.
     **/
    inline void getIds(QList<uint32_t>& out_ids) const;

    /**
     * \brief   Extracts and saves the IDs of the elements in the list in the sorted way.
     * \param   out_ids     The list to save IDs.
     * \param   ascending   If true, sorts the IDs in ascending order.
     **/
    inline void getIdsSorted(QList<uint32_t>& out_ids, bool ascending) const;

    /**
     * \brief   Fixes the entries in the list.
     **/
    inline void fixEntries(void);

    inline void reorderIds(void);

//////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////
protected:
    QList<Data>                 mElementList; //!< The list of data elements.
    NELusanCommon::eSortingType mSorting;     //!< The sorting type.
};

//////////////////////////////////////////////////////////////////////////
// TEDataContainer class template implementation
//////////////////////////////////////////////////////////////////////////

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(ElementBase* parent)
    : ElemBase(parent)
    , mElementList()
    , mSorting(NELusanCommon::eSortingType::NoSorting)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(unsigned int id, ElementBase* parent)
    : ElemBase(id, parent)
    , mElementList()
    , mSorting(NELusanCommon::eSortingType::NoSorting)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(const TEDataContainer& src)
    : ElemBase      (src)
    , mElementList  (src.mElementList)
    , mSorting      (src.mSorting)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(TEDataContainer&& src) noexcept
    : ElemBase      (std::move(src))
    , mElementList  (std::move(src.mElementList))
    , mSorting      (src.mSorting)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(const QList<Data>& entries, ElementBase* parent)
    : ElemBase      (parent)
    , mElementList  (entries)
    , mSorting      (NELusanCommon::eSortingType::NoSorting)
{
    fixEntries();
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>::TEDataContainer(QList<Data>&& entries, ElementBase* parent) noexcept
    : ElemBase      (parent)
    , mElementList  (std::move(entries))
    , mSorting      (NELusanCommon::eSortingType::NoSorting)
{
}

template<class Data, class ElemBase>
TEDataContainer<Data, ElemBase>& TEDataContainer<Data, ElemBase>::operator = (const TEDataContainer& other)
{
    if (this != &other)
    {
        ElemBase::operator=(other);
        mElementList = other.mElementList;
        mSorting     = other.mSorting;
        fixEntries();
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
        mSorting     = other.mSorting;
        fixEntries();
    }

    return *this;
}

template<class Data, class ElemBase>
inline const QList<Data>& TEDataContainer<Data, ElemBase>::getElements(void) const
{
    return mElementList;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::isEmpty(void) const
{
    return mElementList.isEmpty();
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::setElements(const QList<Data>& elements)
{
    mElementList = elements;
    fixEntries();
    sortElements(mSorting);
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::setElements(QList<Data>&& elements) noexcept
{
    mElementList = std::move(elements);
    fixEntries();
    sortElements(mSorting);
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::addElement(Data&& element, bool unique)
{
    if (checkUpdated(element, unique))
    {
        mElementList.append(std::move(element));
        sortElements(mSorting);
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
        sortElements(mSorting);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(const Data& oldElement, Data&& newElement, bool unique)
{
    if (&oldElement == &newElement)
    {
        return true;
    }

    Data* element{ checkElement(oldElement, unique) };
    if (element != nullptr)
    {
        auto& temp = ref{ }(*element);
        newElement.setParent(temp.getParent());
        newElement.setId(temp.getId());
        temp = std::move(newElement);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(const Data& oldElement, const Data& newElement, bool unique)
{
    if (&oldElement == &newElement)
    {
        return true;
    }

    Data* element{ checkElement(oldElement, unique) };
    if (element != nullptr)
    {
        auto& oldElem = ref{ }(*element);
        const auto& newElem = ref{}(newElement);
        newElem.setParent(oldElem.getParent());
        newElem.setId(oldElem.getId());
        *element = std::move(newElement);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::replaceElement(uint32_t id, Data&& newElement, bool unique)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        auto& temp = ref{ }(mElementList[i]);
        if (temp.getId() == id)
        {
            auto& newElem = ref{}(newElement);
            newElem.setParent(temp.getParent());
            newElem.setId(temp.getId());
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
        auto& temp = ref{ }(mElementList[i]);
        if (temp.getId() == id)
        {
            auto& newElem = ref{}(newElement);
            newElem.setParent(temp.getParent());
            newElem.setId(temp.getId());
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
        auto& temp = ref{ }(mElementList[i]);
        if (temp.getName() == name)
        {
            auto& newElem = ref{}(newElement);
            newElem.setParent(temp.getParent());
            newElem.setId(temp.getId());
            mElementList[i] = newElement;
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
        auto& temp = ref{ }(mElementList[i]);
        if (temp.getName() == name)
        {
            auto& newElem = ref{}(newElement);
            newElem.setParent(temp.getParent());
            newElem.setId(temp.getId());
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
        sortElements(mSorting);
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
        sortElements(mSorting);
        return true;
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::removeElement(uint32_t id, Data* elemRemoved /*= nullptr*/)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        const auto& elem = ref{}(mElementList[i]);
        if (elem.getId() == id)
        {
            if (elemRemoved != nullptr)
            {
                *elemRemoved = std::move(mElementList[i]);
            }

            mElementList.removeAt(i);
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::removeElement(const QString& name, Data* elemRemoved /*= nullptr*/)
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        const auto& elem = ref{}(mElementList[i]);
        if (elem.getName() == name)
        {
            if (elemRemoved != nullptr)
            {
                *elemRemoved = std::move(mElementList[i]);
            }

            mElementList.removeAt(i);
            return true;
        }
    }

    return false;
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::removeAllElements()
{
    mElementList.clear();
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::findElement(const QString& uniqueName) const
{
    for (const Data& element : mElementList)
    {
        const auto& elem = ref{}(element);
        if (elem.getName() == uniqueName)
        {
            return const_cast<Data *>(&element);
        }
    }

    return nullptr;
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::findElement(const QString& uniqueName)
{
    for (Data& element : mElementList)
    {
        auto& elem = ref{}(element);
        if (elem.getName() == uniqueName)
        {
            return static_cast<Data *>(&element);
        }
    }

    return nullptr;
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::findElement(uint32_t id) const
{
    for (const Data& element : mElementList)
    {
        const auto& elem = ref{}(element);
        if (elem.getId() == id)
        {
            return const_cast<Data *>(&element);
        }
    }

    return nullptr;
}

template<class Data, class ElemBase>
Data* TEDataContainer<Data, ElemBase>::findElement(uint32_t id)
{
    for (Data& element : mElementList)
    {
        auto& elem = ref{}(element);
        if (elem.getId() == id)
        {
            return static_cast<Data *>(&element);
        }
    }

    return nullptr;
}

template<class Data, class ElemBase>
int TEDataContainer<Data, ElemBase>::findIndex(const QString& name) const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        const auto& elem = ref{}(mElementList[i]);
        if (elem.getName() == name)
        {
            return i;
        }
    }

    return -1;
}

template<class Data, class ElemBase>
inline int TEDataContainer<Data, ElemBase>::findIndex(const Data& field) const
{
    return findIndex(field.getId());
}

template<class Data, class ElemBase>
int TEDataContainer<Data, ElemBase>::findIndex(uint32_t id) const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        const auto& elem = ref{}(mElementList[i]);
        if (elem.getId() == id)
        {
            return i;
        }
    }

    return -1;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::hasElement(const QString& name) const
{
    return findElement(name) != nullptr;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::hasElement(uint32_t id) const
{
    return findElement(id) != nullptr;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::hasElement(const Data& element) const
{
    return (element.getId() != 0 ? hasElement(element.getId()) : hasElement(element.getName()));
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::sortElementsByName(bool ascending)
{
    mSorting = (ascending ? NELusanCommon::eSortingType::SortByNameAsc : NELusanCommon::eSortingType::SortByNameDesc);
    QList<uint32_t> ids;
    getIdsSorted(ids, ascending);
    NELusanCommon::sortByName(mElementList, ascending);
    setOrderedIds(ids);
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::sortElementsById(bool ascending)
{
    QList<uint32_t> ids;
    getIdsSorted(ids, ascending);
    mSorting = (ascending ? NELusanCommon::eSortingType::SortByIdAsc : NELusanCommon::eSortingType::SortByIdDesc);
    NELusanCommon::sortById(mElementList, ascending);
    setOrderedIds(ids);
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::hasElements(void) const
{
    return (mElementList.isEmpty() == false);
}

template<class Data, class ElemBase>
inline int TEDataContainer<Data, ElemBase>::getElementCount(void) const
{
    return static_cast<int>(mElementList.size());
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::swapElements(int index1, int index2)
{
    auto & first = ref{}(mElementList[index1]);
    auto & second = ref{}(mElementList[index2]);
    uint32_t id1 = first.getId();
    uint32_t id2 = second.getId();
    first.setId(id2);
    second.setId(id1);
    
    Data temp = mElementList[index1];
    mElementList[index1] = mElementList[index2];
    mElementList[index2] = temp;
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::swapElements(const Data& elem1, const Data& elem2)
{
    int index1 = findIndex(elem1);
    int index2 = findIndex(elem2);
    if ((index1 >= 0) && (index2 >= 0))
    {
        swapElements(index1, index2);
    }
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::swapElements(uint32_t elem1Id, uint32_t elem2Id)
{
    int index1 = findIndex(elem1Id);
    int index2 = findIndex(elem2Id);
    if ((index1 >= 0) && (index2 >= 0))
    {
        swapElements(index1, index2);
    }
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::orderElements(const QList<uint32_t>& orderedIds)
{
    if (orderedIds.size() != mElementList.size())
    {
        return false;
    }

    int count = static_cast<int>(mElementList.size());
    for (int i = 0; i < count; ++i)
    {
        uint32_t id = orderedIds[i];
        int index = findIndex(id);
        if (index < 0)
        {
            return false;
        }
        else if (index != i)
        {
            swapElements(i, index);
        }
    }

    return true;
}

template<class Data, class ElemBase>
void TEDataContainer<Data, ElemBase>::setOrderedIds(const QList<uint32_t>& orderedIds)
{
    if (orderedIds.size() != mElementList.size())
        return;

    int count = static_cast<int>(mElementList.size());
    for (int i = 0; i < count; ++i)
    {
        auto& temp = ref{ }(mElementList[i]);
        temp.setId(orderedIds[i]);
    }
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::sortElements(NELusanCommon::eSortingType sortingType)
{
    switch (sortingType)
    {
    case NELusanCommon::eSortingType::SortByIdAsc:
        sortElementsById(true);
        break;

    case NELusanCommon::eSortingType::SortByIdDesc:
        sortElementsById(false);
        break;

    case NELusanCommon::eSortingType::SortByNameAsc:
        sortElementsByName(true);
        break;

    case NELusanCommon::eSortingType::SortByNameDesc:
        sortElementsByName(false);
        break;

    default:
        mSorting = sortingType;
        break;
    }
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::noSortElements(void)
{
    mSorting = NELusanCommon::eSortingType::NoSorting;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::isSorted(void) const
{
    return (mSorting != NELusanCommon::eSortingType::NoSorting);
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::isSortedById(void) const
{
    return (static_cast<uint8_t>(mSorting) & static_cast<uint8_t>(NELusanCommon::eOrdering::OrderById)) != 0;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::isSortedByName(void) const
{
    return (static_cast<uint8_t>(mSorting) & static_cast<uint8_t>(NELusanCommon::eOrdering::OrderByName)) != 0;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::isSortedByIdAscending(void) const
{
    return (static_cast<uint8_t>(mSorting) & static_cast<uint8_t>(NELusanCommon::eSorting::SortingAscending)) != 0;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::isSortedByIdDescending(void) const
{
    return (static_cast<uint8_t>(mSorting) & static_cast<uint8_t>(NELusanCommon::eSorting::SortingDescending)) != 0;
}

template<class Data, class ElemBase>
bool TEDataContainer<Data, ElemBase>::checkUniqueness() const
{
    for (int i = 0; i < mElementList.size(); ++i)
    {
        const auto& elem = ref{}(mElementList[i]);
        const QString& name{ elem.getName() };
        for (int j = i + 1; j < mElementList.size(); ++j)
        {
            const auto& temp = ref{}(mElementList[j]);
            if (name == temp.getName())
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

    auto& elem = ref{}(element);
    if (elem.getParent() != this)
    {
        elem.setParent(this);
        elem.setId(ElemBase::getNextId());
    }

    if (elem.getId() != 0)
    {
        found = findElement(elem.getId());
    }
    else if (unique)
    {
        found = findElement(elem.getName());
    }

    return found;
}

template<class Data, class ElemBase>
inline bool TEDataContainer<Data, ElemBase>::checkUpdated(const Data& element, bool unique) const
{
    const auto& elem = ref{}(element);
    return (checkElement(element, unique) == nullptr ? (elem.getParent() == this) && (elem.getId() != 0) : false);
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::getIds(QList<uint32_t>& out_ids) const
{
    for (const Data& element : mElementList)
    {
        const auto& elem = ref{}(element);
        out_ids.append(elem.getId());
    }
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::getIdsSorted(QList<uint32_t>& out_ids, bool ascending) const
{
    out_ids.resize(mElementList.size());
    int i{ 0 };
    for (const Data& element : mElementList)
    {
        const auto& elem = ref{}(element);
        out_ids.at(i) = elem.getId();
    }

    std::sort(out_ids.begin(), out_ids.end(), [ascending](uint32_t lhs, uint32_t rhs) -> bool
        {
            return (ascending ? lhs < rhs : lhs > rhs);
        });
}

template<class Data, class ElemBase>
inline QList<Data>& TEDataContainer<Data, ElemBase>::getElements(void)
{
    return mElementList;
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::fixEntries(void)
{
    for (Data& entry : mElementList)
    {
        auto& temp = ref{ }(entry);
        if (temp.getParent() != this)
        {
            temp.setParent(this);
            temp.setId(ElemBase::getParent()->getNextId());
        }
    }
}

template<class Data, class ElemBase>
inline void TEDataContainer<Data, ElemBase>::reorderIds(void)
{
    QList<uint32_t> ids;
    getIdsSorted(ids, true);
    setOrderedIds(ids);
}

#endif // LUSAN_DATA_COMMON_TEDATACONTAINER_HPP
