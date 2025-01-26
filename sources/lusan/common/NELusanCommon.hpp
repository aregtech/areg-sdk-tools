#ifndef LUSAN_COMMON_NELUSANCOMMON_HPP
#define LUSAN_COMMON_NELUSANCOMMON_HPP
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
 *  \file        lusan/common/NELusanCommon.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include <QList>
#include <QPoint>
#include <QString>
#include <QStringList>

#include <algorithm>

/**
 * \namespace NELusanCommon
 * \brief     Contains common definitions and utility functions for the Lusan application.
 **/
namespace NELusanCommon
{
    /**
     * \brief   The list of file filters.
     **/
    extern const QStringList FILTERS;

    /**
     * \brief   The application name.
     **/
    extern const QString    APPLICATION;

    /**
     * \brief   The organization name.
     **/
    extern const QString    ORGANIZATION;

    /**
     * \brief   The application version.
     **/
    extern const QString    VERSION;

    /**
     * \brief   The options file name.
     **/
    extern const QString    OPTIONS;

    // constexpr const QPoint  DescPos {120, 235};

    /**
     * \brief   Gets the options file path.
     * \return  The options file path.
     **/
    QString getOptionsFile(void);

    /**
     * \brief   Gets the user profile file path.
     * \param   fileName    The name of the file.
     * \return  The user profile file path.
     **/
    QString getUserProfileFile(const QString& fileName);

    /**
     * \brief   Generates a unique ID.
     * \return  A unique ID.
     **/
    uint32_t getId(void);

    /**
     * \brief   Gets the current timestamp.
     * \return  The current timestamp.
     **/
    uint64_t getTimestamp(void);
    
    /**
     * \brief   XML workspace version.
     **/
    const QString xmlWorkspaceVersion                       {"1.0.0"};

    /**
     * \brief   XML element names and attributes.
     **/
    constexpr const char * const xmlElementOptionList       { "OptionList" };
    constexpr const char * const xmlElementOption           { "Option" };
    constexpr const char * const xmlElementWorkspaceList    { "WorspaceList" };
    constexpr const char * const xmlElementWorkspace        { "Workspace" };
    constexpr const char * const xmlElementSettings         { "Settings" };
    constexpr const char * const xmlElementDirectories      { "Directories" };

    constexpr const char * const xmlElementWorspaceRoot     { "WorkspaceRoot" };
    constexpr const char * const xmlElementDescription      { "Description" };
    constexpr const char * const xmlElementSources          { "Sources" };
    constexpr const char * const xmlElementIncludes         { "Includes" };
    constexpr const char * const xmlElementDelivery         { "Delivery" };

    constexpr const char * const xmlElementProject          { "Project" };

    constexpr const char * const xmlAttributeLastAccessed   { "Accessed" };
    constexpr const char * const xmlAttributeId             { "id" };
    constexpr const char * const xmlAttributeName           { "Name" };
    constexpr const char * const xmlAttributeVersion        { "Version" };

    constexpr const char * const xmlElementRecentFiles      { "RecentFiles" };
    constexpr const char * const xmlElementFile             { "File" };

    enum class eSorting : uint8_t
    {
          SortingAscending      = 1     //!< bits: 0000 0001, Sort elements ascending
        , SortingDescending     = 2     //!< bits: 0000 0010, Sort elements descending
    };

    enum class eOrdering : uint8_t
    {
          OrderById             = 4     //!< bits: 0000 0100, Order elements by ID
        , OrderByName           = 8     //!< bits: 0000 1000, Order elements by name
    };

    enum class eSortingType : uint8_t
    {
          NoSorting         = 0     //!< bits: 0000 0000, No sorting
        , SortByIdAsc       = 5     //!< bits: 0000 0101, Sorting by ID ascending
        , SortByIdDesc      = 6     //!< bits: 0000 0110, Sorting by ID descending
        , SortByNameAsc     = 9     //!< bits: 0000 1001, Sorting by Name ascending
        , SortByNameDesc    = 10    //!< bits: 0000 1010, Sorting by Name descending
    };
        
    template<typename T>
    struct get_id
    {
        uint32_t operator()(const T & entry){return entry.getId();};
    };
    
    template<typename T>
    struct get_id<T*>
    {
        uint32_t operator()(const T* entry){return entry->getId();};
    };
    
    template<typename T>
    struct get_name
    {
        const QString& operator()(const T & entry){return entry.getName();};
    };
    
    template<typename T>
    struct get_name<T*>
    {
        const QString& operator()(const T* entry){return entry->getName();};
    };

    template<typename T>
    struct get_ref
    {
        T& operator()(T& entry) { return entry; };
        const T& operator() (const T& entry) { return entry; };
    };


    template<typename T>
    struct get_ref<T*>
    {
        T& operator()(T* entry) { return (*entry); };
        const T& operator() (const T* entry) { return (*entry); };
    };

    template <class Type, class Iter>
    void sortByName(Iter first, Iter last, bool ascending)
    {
        std::sort(first, last, [ascending](Type lhs, Type rhs) -> bool
                  {
                      int res = get_name<Type>{}(lhs).compare(get_name<Type>{}(rhs), Qt::CaseInsensitive);
                      return ascending ? res < 0 : res > 0;
                  });
    }
    
    template <class Type>
    void sortByName(QList<Type>& list, bool ascending)
    {
        NELusanCommon::sortByName<const Type &>(list.begin(), list.end(), ascending);
    }

    template <class Type>
    void sortByName(QList<Type *>& list, bool ascending)
    {
        NELusanCommon::sortByName<const Type*>(list.begin(), list.end(), ascending);
    }
    
    template<class Type, typename Iter>
    void sortById( Iter first, Iter last, bool ascending)
    {
        std::sort(first, last, [ascending](Type lhs, Type rhs) -> bool
            {
                return ascending ? get_id<Type>{}(lhs) < get_id<Type>{}(rhs) : get_id<Type>{}(lhs) > get_id<Type>{}(rhs);
            });
    }
    
    template <class Type>
    void sortById(QList<Type>& list, bool ascending)
    {
        NELusanCommon::sortById<const Type &>(list.begin(), list.end(), ascending);
    }

    template <class Type>
    void sortById(QList<Type *>& list, bool ascending)
    {
        NELusanCommon::sortById<const Type *>(list.begin(), list.end(), ascending);
    }

    template <class Type>
    void moveUp(QList<Type>& list, int index)
    {
        if ((index > 0) && (index < list.size()))
        {
            Type& one = list.at(index);
            Type& two = list.at(index - 1);
            uint32_t id = one.getId();
            one.setId(two.getId());
            two.setId(id);

            list.at(index) = two;
            list.at(index - 1) = one;
        }
    }

    template <class Type>
    void moveUp(QList<Type *>& list, int index)
    {
        if ((index > 0) && (index < list.size()))
        {
            Type* one = list.at(index);
            Type* two = list.at(index - 1);
            uint32_t id = one->getId();
            one->setId(two->getId());
            two->setId(id);

            list.at(index) = two;
            list.at(index - 1) = one;
        }
    }

    template <class Type>
    void moveDown(QList<Type>& list, int index)
    {
        if ((index >= 0) && (index < list.size() - 1))
        {
            Type& one = list.at(index);
            Type& two = list.at(index + 1);
            uint32_t id = one.getId();
            one.setId(two.getId());
            two.setId(id);
            list.at(index) = two;
            list.at(index + 1) = one;
        }
    }

    template <class Type>
    void moveDown(QList<Type*>& list, int index)
    {
        if ((index >= 0) && (index < list.size() - 1))
        {
            Type* one = list.at(index);
            Type* two = list.at(index + 1);
            uint32_t id = one->getId();
            one->setId(two->getId());
            two->setId(id);
            list.at(index) = two;
            list.at(index + 1) = one;
        }
    }
}

#endif  // LUSAN_COMMON_NELUSANCOMMON_HPP
