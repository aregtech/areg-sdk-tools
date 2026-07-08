#ifndef LUSAN_DATA_SM_STATEMACHINEDATA_HPP
#define LUSAN_DATA_SM_STATEMACHINEDATA_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/StateMachineData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSML document root.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/ElementBase.hpp"
#include "lusan/common/VersionNumber.hpp"

#include "lusan/data/sm/SMOverviewData.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMIncludeData.hpp"
#include "lusan/data/sm/SMImportData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"

#include <QString>
#include <QVector>
#include <memory>

/**
 * \class   StateMachineData
 * \brief   The in-memory root of an `.fsml` document, the FSM sibling of
 *          ServiceInterfaceData. It owns one member per XML section, is the parent of the
 *          ElementBase chain and therefore owns the document-wide, monotonically
 *          increasing ID counter. It carries the editor-owned
 *          `FormatVersion` separately from the user's `Overview@Version`.
 **/
class StateMachineData  : protected QObject
                        , public    ElementBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eStimulusType
     * \brief   The kind of a stimulus found in the shared stimulus name space.
     **/
    enum class eStimulusType
    {
          None      //!< No stimulus of that name exists.
        , Trigger   //!< A trigger method.
        , Event     //!< An internal event.
        , Timer     //!< A timer.
    };

    /**
     * \struct  StimulusRef
     * \brief   A resolved stimulus: its kind and the referenced element (or nullptr).
     **/
    struct StimulusRef
    {
        eStimulusType   type    { eStimulusType::None };
        DocumentElem*   element { nullptr };
    };

    static constexpr const char* const  XML_FORMAT_100      { "1.0.0" };
    static constexpr const char* const  XML_FORMAT_DEFAULT  { XML_FORMAT_100 };

    /**
     * \struct  UnknownAttribute
     * \brief   One unknown root-level attribute preserved for newer minor/patch formats.
     **/
    struct UnknownAttribute
    {
        QString name;
        QString value;
    };

    /**
     * \struct  UnknownElement
     * \brief   One unknown root-level subtree and the insertion bucket around known sections.
     **/
    struct UnknownElement
    {
        int     bucket { 0 };
        QString xml;
    };

private:
    static constexpr const uint32_t     MINIMUM_ID          { 50u };    //!< Reserved-low-ID floor (as in .siml).

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    StateMachineData();
    virtual ~StateMachineData() = default;

    /**
     * \brief   Creates the spec-14 skeleton for a new document:
     *          Overview with machine name, one root Start state and default layout entries.
     * \param   machineName  The machine name to set in Overview.
     * \return  A newly initialized document root.
     **/
    static std::unique_ptr<StateMachineData> createNewDocument(const QString& machineName);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getFilePath() const;
    inline void setFilePath(const QString& filePath);

    inline const VersionNumber& getFormatVersion() const;
    inline void setFormatVersion(const VersionNumber& version);

    inline bool openSucceeded() const;
    inline void setOpenSucceeded(bool succeeded);

    inline const SMOverviewData& getOverview() const;
    inline SMOverviewData& getOverview();

    inline const SMDataTypeData& getDataTypes() const;
    inline SMDataTypeData& getDataTypes();

    inline const SMAttributeData& getAttributes() const;
    inline SMAttributeData& getAttributes();

    inline const SMEventData& getEvents() const;
    inline SMEventData& getEvents();

    inline const SMTimerData& getTimers() const;
    inline SMTimerData& getTimers();

    inline const SMMethodData& getMethods() const;
    inline SMMethodData& getMethods();

    inline const SMConstantData& getConstants() const;
    inline SMConstantData& getConstants();

    inline const SMIncludeData& getIncludes() const;
    inline SMIncludeData& getIncludes();

    inline const SMImportData& getImports() const;
    inline SMImportData& getImports();

    inline const SMStateData& getStates() const;
    inline SMStateData& getStates();

    inline const SMLayoutData& getLayout() const;
    inline SMLayoutData& getLayout();

//////////////////////////////////////////////////////////////////////////
// XML persistence
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Reads the whole `.fsml` document from a file, replacing the model content.
     *          On any parse error the document is marked not-opened (openSucceeded() ==
     *          false) so the window can refuse it and never overwrite the original.
     * \param   filePath    The path of the `.fsml` document to read.
     * \return  True if the document parsed without error, false otherwise.
     **/
    bool readFromFile(const QString& filePath);

    /**
     * \brief   Writes the whole `.fsml` document to a file (deterministic, UTF-8,
     *          4-space indentation). When \p filePath is empty the last read/written path
     *          is used.
     * \param   filePath    The destination path, or empty to reuse the document's path.
     * \return  True on success, false otherwise.
     **/
    bool writeToFile(const QString& filePath = QString());

    /**
     * \brief   Writes autosave content without changing the document's current file path.
     * \param   autosavePath The autosave destination path.
     * \return  True on success, false otherwise.
     **/
    bool writeToAutosaveFile(const QString& autosavePath) const;

    /**
     * \brief   Returns the autosave sibling path for a document path.
     **/
    static QString autosavePathForDocument(const QString& documentPath);

    /**
     * \brief   True if a recoverable autosave exists for \p documentPath.
     *          Recovery is offered when autosave exists and is newer than the document.
     **/
    static bool hasRecoverableAutosave(const QString& documentPath, QString* autosavePath = nullptr);

    /**
     * \brief   Removes the autosave sibling file if it exists.
     * \return  True if no autosave exists or it was removed.
     **/
    static bool removeAutosave(const QString& documentPath);

    /**
     * \brief   Reads the `StateMachine` root element and every section from the stream.
     * \return  True if the root element was recognized, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes the `StateMachine` root element and every non-empty section in the
     *          fixed spec-7.7 order.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

//////////////////////////////////////////////////////////////////////////
// Cross-registry lookups
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Resolves a name in the shared stimulus name space — trigger methods,
     *          events and timers. One function so the "one name space" rule
     *          has a single implementation used by validation, pickers and rename.
     * \param   name    The stimulus name to resolve.
     * \return  The stimulus kind and the referenced element.
     **/
    StimulusRef findStimulus(const QString& name) const;

    /**
     * \brief   True if the name is used by any stimulus (trigger/event/timer).
     **/
    bool isStimulusName(const QString& name) const;

    /**
     * \brief   Finds a state by name anywhere in the document (state names are document-unique).
     **/
    SMStateEntry* findState(const QString& name) const;

    /**
     * \brief   The total number of states across every machine level.
     **/
    int getStateCount() const;

//////////////////////////////////////////////////////////////////////////
// hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    bool writeToPathAtomic(const QString& path, bool updateFilePath);
    
    bool writeToPathAtomicConst(const QString& path) const;
    
    bool migrateFromVersion(const VersionNumber& sourceVersion);
    
    bool migrateTo100(const VersionNumber& sourceVersion);

    void clearUnknownContent();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    QString         mFilePath;      //!< The document file path.
    VersionNumber   mFormatVersion; //!< The editor-owned file format version.
    SMOverviewData  mOverview;      //!< The Overview section.
    SMDataTypeData  mDataTypes;     //!< The DataTypeList section.
    SMAttributeData mAttributes;    //!< The AttributeList section.
    SMEventData     mEvents;        //!< The EventList section.
    SMTimerData     mTimers;        //!< The TimerList section.
    SMMethodData    mMethods;       //!< The MethodList section.
    SMConstantData  mConstants;     //!< The ConstantList section.
    SMIncludeData   mIncludes;      //!< The IncludeList section.
    SMImportData    mImports;       //!< The ImportList section.
    SMStateData     mStates;        //!< The root StateList (level 0).
    SMLayoutData    mLayout;        //!< The Layout section.
    QVector<UnknownAttribute> mUnknownRootAttributes; //!< Unknown root attributes preserved on round-trip.
    QVector<UnknownElement>   mUnknownRootElements;   //!< Unknown root elements preserved on round-trip.
    bool            mOpenSuccess;   //!< Whether the document opened successfully.
};

//////////////////////////////////////////////////////////////////////////
// StateMachineData inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& StateMachineData::getFilePath() const
{
    return mFilePath;
}

inline void StateMachineData::setFilePath(const QString& filePath)
{
    mFilePath = filePath;
}

inline const VersionNumber& StateMachineData::getFormatVersion() const
{
    return mFormatVersion;
}

inline void StateMachineData::setFormatVersion(const VersionNumber& version)
{
    mFormatVersion = version;
}

inline bool StateMachineData::openSucceeded() const
{
    return mOpenSuccess;
}

inline void StateMachineData::setOpenSucceeded(bool succeeded)
{
    mOpenSuccess = succeeded;
}

inline const SMOverviewData& StateMachineData::getOverview() const
{
    return mOverview;
}

inline SMOverviewData& StateMachineData::getOverview()
{
    return mOverview;
}

inline const SMDataTypeData& StateMachineData::getDataTypes() const
{
    return mDataTypes;
}

inline SMDataTypeData& StateMachineData::getDataTypes()
{
    return mDataTypes;
}

inline const SMAttributeData& StateMachineData::getAttributes() const
{
    return mAttributes;
}

inline SMAttributeData& StateMachineData::getAttributes()
{
    return mAttributes;
}

inline const SMEventData& StateMachineData::getEvents() const
{
    return mEvents;
}

inline SMEventData& StateMachineData::getEvents()
{
    return mEvents;
}

inline const SMTimerData& StateMachineData::getTimers() const
{
    return mTimers;
}

inline SMTimerData& StateMachineData::getTimers()
{
    return mTimers;
}

inline const SMMethodData& StateMachineData::getMethods() const
{
    return mMethods;
}

inline SMMethodData& StateMachineData::getMethods()
{
    return mMethods;
}

inline const SMConstantData& StateMachineData::getConstants() const
{
    return mConstants;
}

inline SMConstantData& StateMachineData::getConstants()
{
    return mConstants;
}

inline const SMIncludeData& StateMachineData::getIncludes() const
{
    return mIncludes;
}

inline SMIncludeData& StateMachineData::getIncludes()
{
    return mIncludes;
}

inline const SMImportData& StateMachineData::getImports() const
{
    return mImports;
}

inline SMImportData& StateMachineData::getImports()
{
    return mImports;
}

inline const SMStateData& StateMachineData::getStates() const
{
    return mStates;
}

inline SMStateData& StateMachineData::getStates()
{
    return mStates;
}

inline const SMLayoutData& StateMachineData::getLayout() const
{
    return mLayout;
}

inline SMLayoutData& StateMachineData::getLayout()
{
    return mLayout;
}

#endif  // LUSAN_DATA_SM_STATEMACHINEDATA_HPP
