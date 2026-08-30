#ifndef LUSAN_COMMON_NELUSANCOMMON_HPP
#define LUSAN_COMMON_NELUSANCOMMON_HPP
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
 *  \file        lusan/common/NELusanCommon.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "areg/base/areg_global.h"
#include <QIcon>
#include <QList>
#include <QPoint>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <any>
#include <filesystem>
#include <vector>

class QKeySequence;
class QMenu;
class QObject;
class QToolButton;
class QValidator;
class QWidget;

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
     * \brief   The default (preferred) width of the navigation window.
     **/
    constexpr const uint32_t  MIN_NAVI_WIDTH    { 340 };

    /**
     * \brief   The absolute minimum width the navigation window can be shrunk to.
     **/
    constexpr const uint32_t  MIN_NAVI_WIDTH_ABS { 64 };

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
    QString getOptionsFile();

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
    uint32_t getId();

    /**
     * \brief   Gets the current timestamp.
     * \return  The current timestamp.
     **/
    uint64_t getTimestamp();

    /**
     * \brief   Stylesheet for QToolButton when it is checked.
     **/
    constexpr std::string_view  StyleToolbuttonChecked      {"QToolButton::checked{background-color: rgba(163, 183, 204, 80); border-radius: 6px; border: none;}"};
    
    /**
     * \brief   Returns QToolButton stylesheet when it is checked.
     **/
    const QString& getStyleToolbutton();
    
    /**
     * \brief   XML workspace version.
     **/
    const QString xmlWorkspaceVersion                       {"1.0.0"};

    /**
     * \brief   XML element names and attributes.
     **/
    constexpr QLatin1StringView xmlElementOptionList       { "OptionList" };
    constexpr QLatin1StringView xmlElementOption           { "Option" };
    constexpr QLatin1StringView xmlElementTheme            { "Theme" };
    constexpr QLatin1StringView xmlElementWorkspaceList    { "WorspaceList" };
    constexpr QLatin1StringView xmlElementWorkspace        { "Workspace" };
    constexpr QLatin1StringView xmlElementSettings         { "Settings" };
    constexpr QLatin1StringView xmlElementDirectories      { "Directories" };

    constexpr QLatin1StringView xmlElementWorspaceRoot     { "WorkspaceRoot" };
    constexpr QLatin1StringView xmlElementName             { "Name" };
    constexpr QLatin1StringView xmlElementDescription      { "Description" };
    constexpr QLatin1StringView xmlElementSources          { "Sources" };
    constexpr QLatin1StringView xmlElementIncludes         { "Includes" };
    constexpr QLatin1StringView xmlElementDelivery         { "Delivery" };
    constexpr QLatin1StringView xmlElementLogs             { "Logs" };

    constexpr QLatin1StringView xmlElementProject          { "Project" };

    constexpr QLatin1StringView xmlAttributeDefault        { "hasDefault" };
    constexpr QLatin1StringView xmlAttributeLastAccessed   { "Accessed" };
    constexpr QLatin1StringView xmlAttributeId             { "id" };
    constexpr QLatin1StringView xmlAttributeName           { "Name" };
    constexpr QLatin1StringView xmlAttributeVersion        { "Version" };

    constexpr QLatin1StringView xmlElementRecentFiles      { "RecentFiles" };
    constexpr QLatin1StringView xmlElementFile             { "File" };
    
    constexpr char const    SCOPE_SEPRATOR                  { '_' };    //!< Node separator in scope path
    constexpr char const    SCOPE_LEAF_SEPRATOR             { '.' };    //!< Leaf separator in scope path (node.leaf_name)

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

    /**
     * \brief   Sets the directories a relative include location is measured from, most preferred
     *          first. The application sets them from the active workspace; the data layer reads
     *          them to turn a stored location back into a file.
     **/
    void setSearchRoots(const QStringList& roots);

    /**
     * \brief   Returns the directories set by setSearchRoots().
     **/
    const QStringList& getSearchRoots(void);

    /**
     * \brief   Turns a stored include location into an absolute file path.
     *
     *          A location is stored relative to one of the search roots. Older documents spell it
     *          relative to the document that holds it, so both are tried and the first candidate
     *          that exists on disk wins. When none exists, the preferred candidate is returned, so
     *          that a "not found" message names the path the author most likely meant.
     *
     * \param   hostDirectory   Directory of the document holding the location; may be empty when
     *                          the document is unsaved.
     * \param   location        The location as the document spells it.
     **/
    QString resolveLocation(const QString& hostDirectory, const QString& location);

    /**
     * \brief   Returns the path of a file relative to the first of \p roots that contains it, or
     *          the cleaned absolute path when no root does. The list is in priority order.
     **/
    QString relativeToRoots(const QString& absoluteFilePath, const QStringList& roots);

    /**
     * \brief   Returns the form of a file path a document stores: relative to the search root that
     *          contains it, so that every document spells one file the same way. A file under no
     *          root keeps its absolute path, which is not portable but is at least resolvable.
     **/
    QString toStorableLocation(const QString& absoluteFilePath);


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

    //! The air an input row keeps around one line of text.
    constexpr int   InputAir    { 5 };

    /**
     * \brief   Returns the height of a one line input control: a filter box, a find box, a
     *          search field or a selector. Every such control in the application takes it, so
     *          two boxes on the same panel are never a pixel apart.
     * \param   owner   The control the height is for. Its font sets the value.
     **/
    int inputRowHeight(const QWidget& owner);

    //! Empty icon
    const QIcon     EmptyIcon{};

    //! Default small size of icons
    const QSize     SizeSmall   { 16, 16 };

    //! Default middle size of icons
    const QSize     SizeMiddle  { 24, 24 };

    //! Size of the icons on the main window toolbars and on the navigation rail
    const QSize     SizeToolbar { 20, 20 };

    //! Default big size of icons
    const QSize     SizeBig     { 32, 32 };

    /**
     * \brief   Loads an icon from the specified file.
     *          When dark theme icons are enabled, dark strokes are lightened.
     * \param   fileName    The name of the file to load the icon from.
     * \param   size        The size of the icon to load.
     * \return  The loaded icon.
     **/
    QIcon loadIcon(const QString & fileName, const QSize & size = QSize{32, 32});

    /**
     * \brief   Enables or disables icon adaptation for dark themes.
     * \param   isDark  True if the active theme has a dark background.
     **/
    void setIconsForDarkTheme(bool isDark);

    /**
     * \brief   Returns true if icons are adapted for dark themes.
     **/
    bool iconsForDarkTheme();

    //!< Loads new workspace icon and sets the specified size
    inline QIcon iconNewWorkspace(const QSize & size = QSize{ 32, 32 });

    //!< Loads the icon that marks a single workspace and sets the specified size
    inline QIcon iconWorkspace(const QSize & size = QSize{ 32, 32 });

    //!< Loads the manage workspaces icon and sets the specified size
    inline QIcon iconManageWorkspaces(const QSize & size = QSize{ 32, 32 });

    //<! Loads service interface icon and sets the specified size
    inline QIcon iconServiceInterface(const QSize & size = QSize{ 32, 32 });

    //<! Loads state-machine icon and sets the specified size
    inline QIcon iconStateMachine(const QSize & size = QSize{ 32, 32 });

    //<! Loads data type document icon and sets the specified size
    inline QIcon iconDataTypeDocument(const QSize & size = QSize{ 32, 32 });

    //<! Loads the padlock icon, which marks a row this document shows but does not own
    inline QIcon iconLocked(const QSize & size = QSize{ 32, 32 });

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

    //<! Loads FSM design toolbar navigation tab icon and sets the specified size
    inline QIcon iconViewFsmDesign(const QSize & size = QSize{ 32, 32 });

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

    //<! Loads state-machine event icon and sets the specified size
    inline QIcon iconEvent(const QSize & size = QSize{ 32, 32 });

    //<! Loads state-machine timer icon and sets the specified size
    inline QIcon iconTimer(const QSize & size = QSize{ 32, 32 });

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

    //<! Loads FSM trigger method icon and sets the specified size
    inline QIcon iconMethodTrigger(const QSize & size = QSize{ 32, 32 });

    //<! Loads FSM action method icon and sets the specified size
    inline QIcon iconMethodAction(const QSize & size = QSize{ 32, 32 });

    //<! Loads FSM condition method icon and sets the specified size
    inline QIcon iconMethodCondition(const QSize & size = QSize{ 32, 32 });

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

    //<! Loads the icon of the toolbar entries that do not fit the row
    inline QIcon iconToolbarMore(const QSize & size = QSize{ 32, 32 });

    //<! Loads open workspace icon and sets the specified size
    inline QIcon iconWorkspaceOpen(const QSize & size = QSize{ 32, 32 });

    //!< Loads search button icon and sets the specified size
    inline QIcon iconSearch(const QSize & size = QSize{ 32, 32 });

    //!< Loads the funnel icon of the controls that narrow a list, and sets the specified size
    inline QIcon iconFilter(const QSize & size = QSize{ 32, 32 });

    //!< Loads the find usages icon and sets the specified size
    inline QIcon iconSearchUsages(const QSize & size = QSize{ 32, 32 });

    //<! Loads search by match case button icon and sets the specified size
    inline QIcon iconSearchMatchCase(const QSize & size = QSize{ 32, 32 });

    //<! Loads search by match word button icon and sets the specified size
    inline QIcon iconSearchMatchWord(const QSize & size = QSize{ 32, 32 });

    //<! Loads search by wild card button icon and sets the specified size
    inline QIcon iconSearchWildCard(const QSize & size = QSize{ 32, 32 });

    //!< Loads the go-to-declaration (jump) icon and sets the specified size
    inline QIcon iconGotoDefinition(const QSize & size = QSize{ 32, 32 });

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

    //!< Loads the delete entry icon and sets the specified size
    inline QIcon iconDelete(const QSize & size = QSize{ 32, 32 });

    //!< Loads the read again icon and sets the specified size
    inline QIcon iconRefresh(const QSize & size = QSize{ 32, 32 });

    //!< Loads the create file icon and sets the specified size
    inline QIcon iconNewFile(const QSize & size = QSize{ 32, 32 });

    //!< Loads the create folder icon and sets the specified size
    inline QIcon iconNewFolder(const QSize & size = QSize{ 32, 32 });

    //!< Loads the show every entry icon and sets the specified size
    inline QIcon iconShowAll(const QSize & size = QSize{ 32, 32 });

    //!< Loads the browse the machine icon and sets the specified size
    inline QIcon iconComputer(const QSize & size = QSize{ 32, 32 });

    //!< Loads the rename entry icon and sets the specified size
    inline QIcon iconRename(const QSize & size = QSize{ 32, 32 });

    //!< Loads the close icon and sets the specified size
    inline QIcon iconClose(const QSize & size = QSize{ 32, 32 });

    //!< Loads the jump to the first row icon and sets the specified size
    inline QIcon iconScrollTop(const QSize & size = QSize{ 32, 32 });

    //!< Loads the jump to the last row icon and sets the specified size
    inline QIcon iconScrollBottom(const QSize & size = QSize{ 32, 32 });

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

    //<! Loads the icon that shows only the selected scopes
    inline QIcon iconScopeSolo(const QSize & size = QSize{ 32, 32 });

    //<! Loads the icon that hides the selected scopes
    inline QIcon iconScopeMute(const QSize & size = QSize{ 32, 32 });

    //<! Loads the icon that brings every hidden scope back
    inline QIcon iconScopeRestoreAll(const QSize & size = QSize{ 32, 32 });

    //<! Loads the icon of the scope enter and exit lines
    inline QIcon iconScopeLines(const QSize & size = QSize{ 32, 32 });

    //<! Loads the icon that stops a target producing its logs
    inline QIcon iconTargetStop(const QSize & size = QSize{ 32, 32 });

    //<! Loads the icon that holds the logs a target sends
    inline QIcon iconTargetPause(const QSize & size = QSize{ 32, 32 });

    //<! Loads the icon that lets a target produce and send its logs again
    inline QIcon iconTargetResume(const QSize & size = QSize{ 32, 32 });

    //<! Loads service interface tab icon and sets the specified size
    inline QIcon iconServiceInterfaceTab(const QSize & size = QSize{ 32, 32 });
    
    //!< Loads Undo icon
    inline QIcon iconEditUndo(const QSize& size = QSize{32, 32});
    
    //!< Loads Undo icon
    inline QIcon iconEditRedo(const QSize& size = QSize{32, 32});
    
    /**
     * \brief   The side of the square cell every list page toolbar button occupies. One value for
     *          all of them, so the icon column starts at the same place on every page.
     **/
    constexpr int TOOLBUTTON_CELL { 24 };

    //!< The icon size a list page toolbar button carries, the same for plain and split buttons.
    constexpr int TOOLBUTTON_ICON { 25 };

    /**
     * \brief   Object name every split (drop-down) toolbar button carries. The theme sheet matches
     *          on it to reserve the drop-down zone as right padding, which is what keeps the icon
     *          of a split button in the same cell as the plain buttons beside it. The matching
     *          rule lives in `res/styles/theme-template.qss`.
     **/
    extern const QString SPLIT_TOOLBUTTON_NAME;

    //!< Crate a tool button object.
    QToolButton* createToolButton(QWidget* parent, const QString& iconName, const QString& toolTip, const QKeySequence& shortcut);

    //!< Applies the shared look to a plain toolbar button (without a split drop-down menu).
    void decorateToolButton(QToolButton* button);

    /**
     * \brief   Attaches the drop-down menu to an Add split button and sizes it so its icon keeps
     *          the same cell as a plain toolbar button, with the arrow zone appended on the right.
     *          The button re-sizes itself whenever the application style changes.
     **/
    void decorateToolButton(QToolButton* button, QMenu* menu);

    /**
     * \brief   The canonical C++ identifier pattern used across all editor name fields:
     *          a letter or underscore followed by letters, digits or underscores. Anchored
     *          full matches are Acceptable, prefixes (including the empty string) Intermediate,
     *          so a QRegularExpressionValidator built from it rejects invalid keystrokes while
     *          still allowing the field to be cleared or start with an underscore/letter.
     **/
    const QString& identifierPattern();

    /**
     * \brief   The longest name any editor field accepts. The generated code carries the name
     *          as written, so a name no compiler would take is refused where it is typed.
     **/
    constexpr int MAX_IDENTIFIER_LENGTH { 0xFF };

    /**
     * \brief   Returns true if the given text is a valid, complete C++ identifier of at most
     *          \a MAX_IDENTIFIER_LENGTH characters. Empty text is not valid. This is the one
     *          answer every document editor and every validation engine asks.
     **/
    bool isValidIdentifier(const QString& name);

    /**
     * \brief   Turns the base name of a file into the name a document may carry into generated
     *          code: spaces drop out, every character C++ cannot spell becomes '_', and a leading
     *          digit becomes 'N'. "Some File" answers "SomeFile", "123What Ever #1" answers
     *          "NNNWhatEver_1". Answers an empty string for a base name that leaves nothing.
     * \param   fileBaseName    The file name without its extension.
     **/
    QString toDocumentName(const QString& fileBaseName);

    /**
     * \brief   Creates a validator that filters keystrokes to valid C++ identifier characters.
     *          Use for every type/attribute/method/constant/field/parameter/event/timer name
     *          field so invalid characters can never be typed in the first place.
     * \param   parent  The owner of the returned validator (manages its lifetime).
     **/
    QValidator* createIdentifierValidator(QObject* parent);

    /**
     * \brief   Creates a validator that filters keystrokes to characters valid in an include
     *          path (letters, digits, '_', '.', '/', '\\', ':', '-', space). Use for the
     *          Includes location field.
     * \param   parent  The owner of the returned validator (manages its lifetime).
     **/
    QValidator* createPathValidator(QObject* parent);

    /**
     * \brief   Creates a validator that filters keystrokes to a valid C++ qualified name:
     *          one or more identifiers joined by '::' (letters, digits, '_' and '::' only).
     *          Use for the imported-type Namespace field so only valid characters can be typed.
     * \param   parent  The owner of the returned validator (manages its lifetime).
     **/
    QValidator* createQualifiedNameValidator(QObject* parent);

    /**
     * \brief   Paints a crisp, theme-aware chevron icon (a stable replacement for the
     *          platform-dependent QStyle::SP_Arrow* pixmaps). Points down when expanded,
     *          right when collapsed.
     * \param   expanded    True for the open (down) chevron, false for the closed (right) one.
     * \param   color       The stroke color; pass a palette color so it follows the theme.
     * \param   size        The icon size in pixels.
     **/
    QIcon chevronIcon(bool expanded, const QColor& color, const QSize& size = QSize{ 16, 16 });
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
    return loadIcon(":/icons/log-scope-enter", size);
}

inline QIcon NELusanCommon::iconScopeExit(const QSize & size)
{
    return loadIcon(":/icons/log-scope-exit", size);
}

inline QIcon NELusanCommon::iconScopeSolo(const QSize & size)
{
    return loadIcon(":/icons/scope-solo", size);
}

inline QIcon NELusanCommon::iconScopeMute(const QSize & size)
{
    return loadIcon(":/icons/scope-mute", size);
}

inline QIcon NELusanCommon::iconScopeRestoreAll(const QSize & size)
{
    return loadIcon(":/icons/scope-restore-all", size);
}

inline QIcon NELusanCommon::iconScopeLines(const QSize & size)
{
    return loadIcon(":/icons/scope-lines", size);
}

inline QIcon NELusanCommon::iconTargetStop(const QSize & size)
{
    return loadIcon(":/icons/target-stop", size);
}

inline QIcon NELusanCommon::iconTargetPause(const QSize & size)
{
    return loadIcon(":/icons/target-pause", size);
}

inline QIcon NELusanCommon::iconTargetResume(const QSize & size)
{
    return loadIcon(":/icons/target-resume", size);
}

inline QIcon NELusanCommon::iconServiceInterfaceTab(const QSize & size)
{
    return loadIcon(":/icons/doc-service", size);
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

inline QIcon NELusanCommon::iconGotoDefinition(const QSize & size)
{
    return loadIcon(":/icons/goto-declaration", size);
}

inline QIcon NELusanCommon::iconGoUp(const QSize & size)
{
    return loadIcon(":/icons/arrow-up", size);
}

inline QIcon NELusanCommon::iconPause(const QSize & size)
{
    return loadIcon(":/icons/media-pause", size);
}

inline QIcon NELusanCommon::iconStop(const QSize & size)
{
    return loadIcon(":/icons/media-stop", size);
}

inline QIcon NELusanCommon::iconPlay(const QSize & size)
{
    return loadIcon(":/icons/media-play", size);
}

inline QIcon NELusanCommon::iconRecord(const QSize & size)
{
    return loadIcon(":/icons/media-record", size);
}

inline QIcon NELusanCommon::iconClear(const QSize & size)
{
    return loadIcon(":/icons/trash", size);
}

inline QIcon NELusanCommon::iconDelete(const QSize & size)
{
    return loadIcon(":/icons/trash", size);
}

inline QIcon NELusanCommon::iconRefresh(const QSize & size)
{
    return loadIcon(":/icons/Update Item", size);
}

inline QIcon NELusanCommon::iconNewFile(const QSize & size)
{
    return loadIcon(":/icons/file-new", size);
}

inline QIcon NELusanCommon::iconNewFolder(const QSize & size)
{
    return loadIcon(":/icons/folder-new", size);
}

inline QIcon NELusanCommon::iconShowAll(const QSize & size)
{
    return loadIcon(":/icons/visibility", size);
}

inline QIcon NELusanCommon::iconComputer(const QSize & size)
{
    return loadIcon(":/icons/computer", size);
}

inline QIcon NELusanCommon::iconRename(const QSize & size)
{
    return loadIcon(":/icons/rename", size);
}

inline QIcon NELusanCommon::iconClose(const QSize & size)
{
    return loadIcon(":/icons/close", size);
}

inline QIcon NELusanCommon::iconScrollTop(const QSize & size)
{
    return loadIcon(":/icons/scroll-top", size);
}

inline QIcon NELusanCommon::iconScrollBottom(const QSize & size)
{
    return loadIcon(":/icons/scroll-bottom", size);
}

inline QIcon NELusanCommon::iconEditUndo(const QSize& size)
{
    return loadIcon(":/icons/edit-undo", size);
}

inline QIcon NELusanCommon::iconEditRedo(const QSize& size)
{
    return loadIcon(":/icons/edit-redo", size);
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
    return loadIcon(":/icons/tree-expand", size);
}

inline QIcon NELusanCommon::iconNodeExpanded(const QSize & size)
{
    return loadIcon(":/icons/tree-collapse", size);
}

inline QIcon NELusanCommon::iconToolbarMore(const QSize & size)
{
    return loadIcon(":/icons/toolbar-more", size);
}

inline QIcon NELusanCommon::iconWorkspaceOpen(const QSize & size)
{
    return loadIcon(":/icons/file-open", size);
}

inline QIcon NELusanCommon::iconSearch(const QSize & size)
{
    return loadIcon(":/icons/edit-find", size);
}

inline QIcon NELusanCommon::iconFilter(const QSize & size)
{
    return loadIcon(":/icons/filter", size);
}

inline QIcon NELusanCommon::iconSearchUsages(const QSize & size)
{
    return loadIcon(":/icons/edit-find-usages", size);
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

inline QIcon NELusanCommon::iconMethodTrigger(const QSize & size)
{
    return loadIcon(":/icons/sm method trigger", size);
}

inline QIcon NELusanCommon::iconMethodAction(const QSize & size)
{
    return loadIcon(":/icons/sm method action", size);
}

inline QIcon NELusanCommon::iconMethodCondition(const QSize & size)
{
    return loadIcon(":/icons/sm method condition", size);
}

inline QIcon NELusanCommon::iconDefaultValue(const QSize & size)
{
    return loadIcon(":/icons/check", size);
}

inline QIcon NELusanCommon::iconMethodParam(const QSize & size)
{
    return loadIcon(":/icons/data method param", size);
}

inline QIcon NELusanCommon::iconInclude(const QSize & size)
{
    return loadIcon(":/icons/include-file", size);
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

inline QIcon NELusanCommon::iconEvent(const QSize & size)
{
    return loadIcon(":/icons/sm-event", size);
}

inline QIcon NELusanCommon::iconTimer(const QSize & size)
{
    return loadIcon(":/icons/sm-timer", size);
}

inline QIcon NELusanCommon::iconWarning(const QSize & size)
{
    return loadIcon(":/icons/warning", size);
}

inline QIcon NELusanCommon::iconTypeWarning(const QSize & size)
{
    return loadIcon(":/icons/warning", size);
}

inline QIcon NELusanCommon::iconSettings(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/app-settings", size);
}

inline QIcon NELusanCommon::iconViewOutputWindow(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/view-output", size);
}

inline QIcon NELusanCommon::iconViewOfflineLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/nav-offline-logs", size);
}

inline QIcon NELusanCommon::iconViewFsmDesign(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/nav-design-toolbar", size);
}

inline QIcon NELusanCommon::iconViewLiveLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/nav-live-logs", size);
}

inline QIcon NELusanCommon::iconViewWorkspace(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/nav-workspace", size);
}

inline QIcon NELusanCommon::iconViewNavigationWindow(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/view-navigation", size);
}

inline QIcon NELusanCommon::iconPaste(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/edit-paste", size);
}

inline QIcon NELusanCommon::iconCopy(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/edit-copy", size);
}

inline QIcon NELusanCommon::iconCut(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/edit-cut", size);
}

inline QIcon NELusanCommon::iconApplicationExit(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/app-exit", size);
}

inline QIcon NELusanCommon::iconSaveAsDocument(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/file-save-as", size);
}

inline QIcon NELusanCommon::iconSaveDocument(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/file-save", size);
}

inline QIcon NELusanCommon::iconOpenDocument(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/file-open", size);
}

inline QIcon NELusanCommon::iconOpenFile(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/file-open", size);
}

inline QIcon NELusanCommon::iconNewOfflineLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/nav-offline-logs", size);
}

inline QIcon NELusanCommon::iconLiveLogWindow(const QSize & size /*= QSize{ 32, 32 }*/)
{
    return loadIcon(":/icons/nav-live-logs", size);
}

inline QIcon NELusanCommon::iconOfflineLogWindow(const QSize & size /*= QSize{ 32, 32 }*/)
{
    return loadIcon(":/icons/nav-offline-logs", size);
}

inline QIcon NELusanCommon::iconNewLiveLogs(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/nav-live-logs", size);
}

inline QIcon NELusanCommon::iconServiceInterface(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/doc-service", size);
}

inline QIcon NELusanCommon::iconStateMachine(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/doc-state-machine", size);
}

inline QIcon NELusanCommon::iconDataTypeDocument(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/doc-data-types", size);
}

inline QIcon NELusanCommon::iconLocked(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/locked", size);
}

inline QIcon NELusanCommon::iconLiveLogConnected(const QSize & size)
{
    return loadIcon(":/icons/signal-on", size);
}

inline QIcon NELusanCommon::iconLiveLogDisconnected(const QSize & size)
{
    return loadIcon(":/icons/signal-off", size);
}

inline QIcon NELusanCommon::iconNewWorkspace(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/workspace-new", size);
}

inline QIcon NELusanCommon::iconWorkspace(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/workspace-item", size);
}

inline QIcon NELusanCommon::iconManageWorkspaces(const QSize & size /*= QSize{32, 32}*/)
{
    return loadIcon(":/icons/workspace-manage", size);
}

#endif  // LUSAN_COMMON_NELUSANCOMMON_HPP
