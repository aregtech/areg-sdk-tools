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
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/common/NELusanCommon.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "areg/base/GEGlobal.h"
#include <QIcon>
#include <QList>
#include <QPoint>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <any>
#include <filesystem>
#include <vector>

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

    /**
     * \brief   The application profile initialization file name.
     **/
    extern const QString    INIT_FILE;

    /**
     * \brief   The minimal width of navigation window.
     **/
    constexpr const uint32_t  MIN_NAVI_WIDTH    { 280 };
    
    /**
     * \brief   The minimal height of navigation window.
     **/
    constexpr const uint32_t  MIN_NAVI_HEIGHT   { 280 };

    /**
     * \brief   The minimal height of output window.
     **/
    constexpr const uint32_t  MIN_OUTPUT_HEIGHT { 80  };

    /**
     * \brief   The minimal width of output window.
     **/
    constexpr const uint32_t  MIN_OUTPUT_WIDTH  { 320 };

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
     * \brief   Stylesheet for QToolButton when it is checked.
     **/
    constexpr std::string_view  StyleToolbuttonChecked      {"QToolButton::checked{background-color: rgba(163, 183, 204, 80); border-radius: 6px; border: none;}"};
    
    /**
     * \brief   Returns QToolButton stylesheet when it is checked.
     **/
    const QString& getStyleToolbutton(void);
    
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
    constexpr const char * const xmlElementLogs             { "Logs" };

    constexpr const char * const xmlElementProject          { "Project" };
    
    constexpr const char * const xmlAttributeDefault        { "hasDefault" };
    constexpr const char * const xmlAttributeLastAccessed   { "Accessed" };
    constexpr const char * const xmlAttributeId             { "id" };
    constexpr const char * const xmlAttributeName           { "Name" };
    constexpr const char * const xmlAttributeVersion        { "Version" };

    constexpr const char * const xmlElementRecentFiles      { "RecentFiles" };
    constexpr const char * const xmlElementFile             { "File" };
    
    constexpr char const    SCOPE_SEPRATOR                  { '_' };
    
    constexpr char const    SCOPE_ALL                       { '*' };

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
    
    template<typename T>
    struct get_ptr
    {
        T* operator()(T& entry) { return &entry; };
        const T* operator() (const T& entry) { return &entry; };
    };
    
    
    template<typename T>
    struct get_ptr<T*>
    {
        T* operator()(T* entry) { return entry; };
        const T* operator() (const T* entry) { return entry; };
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

    /**
     * \brief   Fix path to make it absolute and OS-independent.
     * \param   path    The path to fix.
     **/
    QString fixPath(const QString& path);
    
    using AnyData = std::any;
    using AnyList = std::vector<AnyData>;
    
    //!< Structure to hold text filter parameters
    struct sStringFilter
    {
        QString text            {     };    //!< The text to filter by
        bool    isCaseSensitive {false};    //!< Indicates if the filter is case-sensitive
        bool    isWholeWord     {false};    //!< Indicates if the filter matches whole words only
        bool    isWildCard      {false};    //!< Indicates if the filter uses wildcards
    };
    
    
    /**
     * \brief   The type of match for the filter.
     *          NoMatch - no match found,
     *          PartialMatch - partial match found,
     *          ExactMatch - exact match found.
     **/
    enum eMatchType : int
    {
          NoMatch       = 0 //!< Has not match of filters
        , PartialMatch  = 1 //!< Has partial match of filters
        , PartialOutput = 2 //!< Has partial match of filters to output, but not exact
        , ExactMatch    = 4 //!< Has exact match of filters
    };

    /**
     * \brief   The filter data structure.
     **/
    struct sFilterData
    {
        QString text    { };        //!< The text to filter by
        AnyData data    { };        //!< The data associated with the filter, can be any type
        bool    active  { false };  //!< Indicates if the filter is active (checked)
    };
    
    using FilterList    = QList<sFilterData>;
    using FilterString  = sStringFilter;
    using FilterData    = sFilterData;

    /**
     * \brief   Merges two icons into one, scaling them according to the specified factors.
     * \param   icon1   First icon to merge
     * \param   scale1  Scaling factor for the first icon (0.0 to 1.0)
     * \param   icon2   Second icon to merge
     * \param   scale2  Scaling factor for the second icon (0.0 to 1.0)
     * \param   size    The size in pixels of the resulting icon
     * \return  Instance of merged icons.
     **/
    QIcon mergeIcons(const QIcon& icon1, double scale1, const QIcon& icon2, double scale2, const QSize& size);

    //! Empty icon
    const QIcon     EmptyIcon{};

    //! Default small size of icons
    const QSize     SizeSmall   { 16, 16 };

    //! Default middle size of icons
    const QSize     SizeMiddle  { 24, 24 };

    //! Default big size of icons
    const QSize     SizeBig     { 32, 32 };

    /**
     * \brief   Loads an icon from the specified file.
     * \param   fileName    The name of the file to load the icon from.
     * \param   size        The size of the icon to load.
     * \return  The loaded icon.
     **/
    inline QIcon loadIcon(const QString & fileName, const QSize & size = QSize{32, 32});

    //!< Loads new workspace icon and sets the specified size
    inline QIcon iconNewWorkspace(const QSize & size = QSize{ 32, 32 });

    //<! Loads service interface icon and sets the specified size
    inline QIcon iconServiceInterface(const QSize & size = QSize{ 32, 32 });

    //<! Loads live logging connected icon and sets the specified size
    inline QIcon iconLiveLogConnected(const QSize & size = QSize{ 32, 32 });

    //!< Loads live logging disconnected icon and sets the specified size
    inline QIcon iconLiveLogDisconnected(const QSize & size = QSize{ 32, 32 });

    //<! Loads new live logging icon and sets the specified size
    inline QIcon iconNewLiveLogs(const QSize & size = QSize{ 32, 32 });

    //!< Loads new offline logging icon and sets the specified size
    inline QIcon iconNewOfflineLogs(const QSize & size = QSize{ 32, 32 });

    //!< Loads new live logging window icon and sets the specified size
    inline QIcon iconLiveLogWindow(const QSize & size = QSize{ 32, 32 });

    //!< Loads offline logging window icon and sets the specified size
    inline QIcon iconOfflineLogWindow(const QSize & size = QSize{ 32, 32 });

    //<! Loads open document icon and sets the specified size
    inline QIcon iconOpenDocument(const QSize & size = QSize{ 32, 32 });
    
    inline QIcon iconOpenFile(const QSize & size = QSize{ 32, 32 });

    //<! Loads save document icon and sets the specified size
    inline QIcon iconSaveDocument(const QSize & size = QSize{ 32, 32 });

    //<! Loads save as document icon and sets the specified size
    inline QIcon iconSaveAsDocument(const QSize & size = QSize{ 32, 32 });

    //<! Loads exit application icon and sets the specified size
    inline QIcon iconApplicationExit(const QSize & size = QSize{ 32, 32 });

    //<! Loads cut text icon and sets the specified size
    inline QIcon iconCut(const QSize & size = QSize{ 32, 32 });

    //<! Loads copy text icon and sets the specified size
    inline QIcon iconCopy(const QSize & size = QSize{ 32, 32 });

    //<! Loads paste text icon and sets the specified size
    inline QIcon iconPaste(const QSize & size = QSize{ 32, 32 });

    //<! Loads navigation window icon and sets the specified size
    inline QIcon iconViewNavigationWindow(const QSize & size = QSize{ 32, 32 });

    //<! Loads workspace navigation tab icon and sets the specified size
    inline QIcon iconViewWorkspace(const QSize & size = QSize{ 32, 32 });

    //<! Loads live logs navigation tab icon and sets the specified size
    inline QIcon iconViewLiveLogs(const QSize & size = QSize{ 32, 32 });

    //<! Loads offline logs navigation tab icon and sets the specified size
    inline QIcon iconViewOfflineLogs(const QSize & size = QSize{ 32, 32 });

    //<! Loads output / status window tab icon and sets the specified size
    inline QIcon iconViewOutputWindow(const QSize & size = QSize{ 32, 32 });

    //<! Loads application options / settings icon and sets the specified size
    inline QIcon iconSettings(const QSize & size = QSize{ 32, 32 });

    //<! Loads data type warning icon and sets the specified size
    inline QIcon iconTypeWarning(const QSize & size = QSize{ 32, 32 });

    //<! Loads warning icon and sets the specified size
    inline QIcon iconWarning(const QSize & size = QSize{ 32, 32 });

    //<! Loads attribute icon and sets the specified size
    inline QIcon iconAttribute(const QSize & size = QSize{ 32, 32 });

    //<! Loads constant icon and sets the specified size
    inline QIcon iconConstant(const QSize & size = QSize{ 32, 32 });

    //<! Loads container icon and sets the specified size
    inline QIcon iconContainer(const QSize & size = QSize{ 32, 32 });

    //<! Loads enum icon and sets the specified size
    inline QIcon iconEnum(const QSize & size = QSize{ 32, 32 });

    //<! Loads imported icon and sets the specified size
    inline QIcon iconImported(const QSize & size = QSize{ 32, 32 });

    //<! Loads structure icon and sets the specified size
    inline QIcon iconStructure(const QSize & size = QSize{ 32, 32 });

    //<! Loads enum field icon and sets the specified size
    inline QIcon iconEnumField(const QSize & size = QSize{ 32, 32 });

    //<! Loads struct field icon and sets the specified size
    inline QIcon iconStructField(const QSize & size = QSize{ 32, 32 });

    //<! Loads include icon and sets the specified size
    inline QIcon iconInclude(const QSize & size = QSize{ 32, 32 });

    //<! Loads request type method icon and sets the specified size
    inline QIcon iconMethodRequest(const QSize & size = QSize{ 32, 32 });

    //<! Loads response type method icon and sets the specified size
    inline QIcon iconMethodResponse(const QSize & size = QSize{ 32, 32 });

    //<! Loads broadcast type method icon and sets the specified size
    inline QIcon iconMethodBroadcast(const QSize & size = QSize{ 32, 32 });

    //<! Loads method parameter icon and sets the specified size
    inline QIcon iconMethodParam(const QSize & size = QSize{ 32, 32 });

    //<! Loads set data default value icon and sets the specified size
    inline QIcon iconDefaultValue(const QSize & size = QSize{ 32, 32 });

    //<! Loads log selected icon and sets the specified size
    inline QIcon iconLogSelected(const QSize & size = QSize{ 32, 32 });

    //<! Loads node collapsed icon and sets the specified size
    inline QIcon iconNodeCollapsed(const QSize & size = QSize{ 32, 32 });

    //<! Loads node expanded icon and sets the specified size
    inline QIcon iconNodeExpanded(const QSize & size = QSize{ 32, 32 });

    //<! Loads open workspace icon and sets the specified size
    inline QIcon iconWorkspaceOpen(const QSize & size = QSize{ 32, 32 });

    //!< Loads search button icon and sets the specified size
    inline QIcon iconSearch(const QSize & size = QSize{ 32, 32 });

    //<! Loads search by match case button icon and sets the specified size
    inline QIcon iconSearchMatchCase(const QSize & size = QSize{ 32, 32 });

    //<! Loads search by match word button icon and sets the specified size
    inline QIcon iconSearchMatchWord(const QSize & size = QSize{ 32, 32 });

    //<! Loads search by wild card button icon and sets the specified size
    inline QIcon iconSearchWildCard(const QSize & size = QSize{ 32, 32 });

    //<! Loads go up icon and sets the specified size
    inline QIcon iconGoUp(const QSize & size = QSize{ 32, 32 });

    //!< Loads pause button icon and sets the specified size
    inline QIcon iconPause(const QSize & size = QSize{ 32, 32 });

    //!< Loads stop button icon and sets the specified size
    inline QIcon iconStop(const QSize & size = QSize{ 32, 32 });

    //!< Loads play button icon and sets the specified size
    inline QIcon iconPlay(const QSize & size = QSize{ 32, 32 });

    //!< Loads record button icon and sets the specified size
    inline QIcon iconRecord(const QSize & size = QSize{ 32, 32 });

    //<! Loads clear button icon and sets the specified size
    inline QIcon iconClear(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio scope log icon and sets the specified size
    inline QIcon iconLogScope(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio debug log icon and sets the specified size
    inline QIcon iconLogDebug(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio info log icon and sets the specified size
    inline QIcon iconLogInfo(const QSize & size = QSize{ 32, 32 });

    //!< Loads prio warning log icon and sets the specified size
    inline QIcon iconLogWarning(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio error log icon and sets the specified size
    inline QIcon iconLogError(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio fatal log icon and sets the specified size
    inline QIcon iconLogFatal(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio scope enter log icon and sets the specified size
    inline QIcon iconScopeEnter(const QSize & size = QSize{ 32, 32 });

    //<! Loads prio scope exit log icon and sets the specified size
    inline QIcon iconScopeExit(const QSize & size = QSize{ 32, 32 });

    //<! Loads service interface tab icon and sets the specified size
    inline QIcon iconServiceInterfaceTab(const QSize & size = QSize{ 32, 32 });
}

inline QIcon NELusanCommon::iconLogDebug(const QSize & size)
{
    return loadIcon(":/icons/log-prio-debug", size);
}

inline QIcon NELusanCommon::iconLogInfo(const QSize & size)
{
    return loadIcon(":/icons/log-prio-info", size);
}

inline QIcon NELusanCommon::iconLogWarning(const QSize & size)
{
    return loadIcon(":/icons/log-prio-warning", size);
}

inline QIcon NELusanCommon::iconLogError(const QSize & size)
{
    return loadIcon(":/icons/log-prio-error", size);
}

inline QIcon NELusanCommon::iconLogFatal(const QSize & size)
{
    return loadIcon(":/icons/log-prio-fatal", size);
}

inline QIcon NELusanCommon::iconScopeEnter(const QSize & size)
{
    return loadIcon(":/icons/log-prio-scope-enter", size);
}

inline QIcon NELusanCommon::iconScopeExit(const QSize & size)
{
    return loadIcon(":/icons/log-prio-scope-exit", size);
}

inline QIcon NELusanCommon::iconServiceInterfaceTab(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconSearchMatchCase(const QSize & size)
{
    return loadIcon(":/icons/search-match-case", size);
}

inline QIcon NELusanCommon::iconSearchMatchWord(const QSize & size)
{
    return loadIcon(":/icons/search-match-word", size);
}

inline QIcon NELusanCommon::iconSearchWildCard(const QSize & size)
{
    return loadIcon(":/icons/search-wild-card", size);
}

inline QIcon NELusanCommon::iconGoUp(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::GoUp) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconPause(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("media-playback-pause") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconStop(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("media-playback-stop") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconPlay(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("media-playback-start") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconRecord(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("media-record") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconClear(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::EditClear) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconLogScope(const QSize & size)
{
    return loadIcon(":/icons/log-prio-scope", size);
}

inline QIcon NELusanCommon::iconLogSelected(const QSize & size)
{
    return loadIcon(":/icons/right-arrow", size);
}

inline QIcon NELusanCommon::iconNodeCollapsed(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("list-add")};
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconNodeExpanded(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("list-remove")};
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconWorkspaceOpen(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconSearch(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::EditFind) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconMethodRequest(const QSize & size)
{
    return loadIcon(":/icons/data method request", size);
}

inline QIcon NELusanCommon::iconMethodResponse(const QSize & size)
{
    return loadIcon(":/icons/data method response", size);
}

inline QIcon NELusanCommon::iconMethodBroadcast(const QSize & size)
{
    return loadIcon(":/icons/data method broadcast", size);
}

inline QIcon NELusanCommon::iconDefaultValue(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::ToolsCheckSpelling) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconMethodParam(const QSize & size)
{
    return loadIcon(":/icons/data method param", size);
}

inline QIcon NELusanCommon::iconInclude(const QSize & size)
{
    return loadIcon(":/icons/data-include", size);
}

inline QIcon NELusanCommon::iconStructField(const QSize & size)
{
    return loadIcon(":/icons/data type struct field", size);
}

inline QIcon NELusanCommon::iconEnumField(const QSize & size)
{
    return loadIcon(":/icons/data type enum field", size);
}

inline QIcon NELusanCommon::iconStructure(const QSize & size)
{
    return loadIcon(":/icons/data type structure", size);
}

inline QIcon NELusanCommon::iconImported(const QSize & size)
{
    return loadIcon(":/icons/data type import", size);
}

inline QIcon NELusanCommon::iconEnum(const QSize & size)
{
    return loadIcon(":/icons/data type enum", size);
}

inline QIcon NELusanCommon::iconContainer(const QSize & size)
{
    return loadIcon(":/icons/data type container", size);
}

inline QIcon NELusanCommon::iconConstant(const QSize & size)
{
    return loadIcon(":/icons/data-constant", size);
}

inline QIcon NELusanCommon::iconAttribute(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/data-attribute", size);
}

inline QIcon NELusanCommon::iconWarning(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconTypeWarning(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconSettings(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("applications-development") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconViewOutputWindow(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/view-status", size);
}

inline QIcon NELusanCommon::iconViewOfflineLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/view-offline-logs", size);
}

inline QIcon NELusanCommon::iconViewLiveLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/view-live-logs", size);
}

inline QIcon NELusanCommon::iconViewWorkspace(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/workspace-explorer", size);
}

inline QIcon NELusanCommon::iconViewNavigationWindow(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/view-navigation", size);
}

inline QIcon NELusanCommon::iconPaste(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png")) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconCopy(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png")) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconCut(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png")) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconApplicationExit(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("application-exit") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconSaveAsDocument(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("document-save-as") };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconSaveDocument(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("document-save", QIcon(":/images/save.png")) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconOpenDocument(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("document-open", QIcon(":/images/open.png")) };
    icon.actualSize(size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconOpenFile(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/open-file", size);
}

inline QIcon NELusanCommon::iconNewOfflineLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/new-offline-logs", size);
}

inline QIcon NELusanCommon::iconLiveLogWindow(const QSize & size /*= QSize{ 32, 32 }*/)
{
    return loadIcon(":/icons/view-live-logs", size);
}

inline QIcon NELusanCommon::iconOfflineLogWindow(const QSize & size /*= QSize{ 32, 32 }*/)
{
    return loadIcon(":/icons/view-offline-logs", size);
}

inline QIcon NELusanCommon::iconNewLiveLogs(const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon{ QIcon::fromTheme("network-wireless")};
    icon.actualSize(QSize{32, 32}, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconServiceInterface(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/new-service", size);
}

inline QIcon NELusanCommon::iconLiveLogConnected(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("network-wireless")};
    icon.actualSize(QSize{ 32, 32 }, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconLiveLogDisconnected(const QSize & size)
{
    QIcon icon{ QIcon::fromTheme("network-offline")};
    icon.actualSize(QSize{ 32, 32 }, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

inline QIcon NELusanCommon::iconNewWorkspace(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/new-workspace", size);
}
    
inline QIcon NELusanCommon::loadIcon(const QString & fileName, const QSize & size /*= QSize{32, 32}*/)
{
    QIcon icon;
    icon.addFile(fileName, size, QIcon::Mode::Normal, QIcon::State::On);
    return icon;
}

#endif  // LUSAN_COMMON_NELUSANCOMMON_HPP
