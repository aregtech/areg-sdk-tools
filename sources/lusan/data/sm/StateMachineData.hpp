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

/**
 * \class   StateMachineData
 * \brief   The in-memory root of an `.fsml` document, the FSM sibling of
 *          ServiceInterfaceData. It owns one member per XML section, is the parent of the
 *          ElementBase chain and therefore owns the document-wide, monotonically
 *          increasing ID counter. It carries the editor-owned
 *          `FormatVersion` separately from the user's `Overview@Version`.
 **/
class StateMachineData : public ElementBase
{
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

private:
    static constexpr const uint32_t     MINIMUM_ID          { 50u };    //!< Reserved-low-ID floor (as in .siml).

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    StateMachineData(void);
    virtual ~StateMachineData(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getFilePath(void) const;
    inline void setFilePath(const QString& filePath);

    inline const VersionNumber& getFormatVersion(void) const;
    inline void setFormatVersion(const VersionNumber& version);

    inline bool openSucceeded(void) const;
    inline void setOpenSucceeded(bool succeeded);

    inline const SMOverviewData& getOverview(void) const;
    inline SMOverviewData& getOverview(void);

    inline const SMDataTypeData& getDataTypes(void) const;
    inline SMDataTypeData& getDataTypes(void);

    inline const SMAttributeData& getAttributes(void) const;
    inline SMAttributeData& getAttributes(void);

    inline const SMEventData& getEvents(void) const;
    inline SMEventData& getEvents(void);

    inline const SMTimerData& getTimers(void) const;
    inline SMTimerData& getTimers(void);

    inline const SMMethodData& getMethods(void) const;
    inline SMMethodData& getMethods(void);

    inline const SMConstantData& getConstants(void) const;
    inline SMConstantData& getConstants(void);

    inline const SMIncludeData& getIncludes(void) const;
    inline SMIncludeData& getIncludes(void);

    inline const SMImportData& getImports(void) const;
    inline SMImportData& getImports(void);

    inline const SMStateData& getStates(void) const;
    inline SMStateData& getStates(void);

    inline const SMLayoutData& getLayout(void) const;
    inline SMLayoutData& getLayout(void);

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
    int getStateCount(void) const;

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
    bool            mOpenSuccess;   //!< Whether the document opened successfully.
};

//////////////////////////////////////////////////////////////////////////
// StateMachineData inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& StateMachineData::getFilePath(void) const
{
    return mFilePath;
}

inline void StateMachineData::setFilePath(const QString& filePath)
{
    mFilePath = filePath;
}

inline const VersionNumber& StateMachineData::getFormatVersion(void) const
{
    return mFormatVersion;
}

inline void StateMachineData::setFormatVersion(const VersionNumber& version)
{
    mFormatVersion = version;
}

inline bool StateMachineData::openSucceeded(void) const
{
    return mOpenSuccess;
}

inline void StateMachineData::setOpenSucceeded(bool succeeded)
{
    mOpenSuccess = succeeded;
}

inline const SMOverviewData& StateMachineData::getOverview(void) const
{
    return mOverview;
}

inline SMOverviewData& StateMachineData::getOverview(void)
{
    return mOverview;
}

inline const SMDataTypeData& StateMachineData::getDataTypes(void) const
{
    return mDataTypes;
}

inline SMDataTypeData& StateMachineData::getDataTypes(void)
{
    return mDataTypes;
}

inline const SMAttributeData& StateMachineData::getAttributes(void) const
{
    return mAttributes;
}

inline SMAttributeData& StateMachineData::getAttributes(void)
{
    return mAttributes;
}

inline const SMEventData& StateMachineData::getEvents(void) const
{
    return mEvents;
}

inline SMEventData& StateMachineData::getEvents(void)
{
    return mEvents;
}

inline const SMTimerData& StateMachineData::getTimers(void) const
{
    return mTimers;
}

inline SMTimerData& StateMachineData::getTimers(void)
{
    return mTimers;
}

inline const SMMethodData& StateMachineData::getMethods(void) const
{
    return mMethods;
}

inline SMMethodData& StateMachineData::getMethods(void)
{
    return mMethods;
}

inline const SMConstantData& StateMachineData::getConstants(void) const
{
    return mConstants;
}

inline SMConstantData& StateMachineData::getConstants(void)
{
    return mConstants;
}

inline const SMIncludeData& StateMachineData::getIncludes(void) const
{
    return mIncludes;
}

inline SMIncludeData& StateMachineData::getIncludes(void)
{
    return mIncludes;
}

inline const SMImportData& StateMachineData::getImports(void) const
{
    return mImports;
}

inline SMImportData& StateMachineData::getImports(void)
{
    return mImports;
}

inline const SMStateData& StateMachineData::getStates(void) const
{
    return mStates;
}

inline SMStateData& StateMachineData::getStates(void)
{
    return mStates;
}

inline const SMLayoutData& StateMachineData::getLayout(void) const
{
    return mLayout;
}

inline SMLayoutData& StateMachineData::getLayout(void)
{
    return mLayout;
}

#endif  // LUSAN_DATA_SM_STATEMACHINEDATA_HPP
