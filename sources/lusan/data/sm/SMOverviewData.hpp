#ifndef LUSAN_DATA_SM_SMOVERVIEWDATA_HPP
#define LUSAN_DATA_SM_SMOVERVIEWDATA_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMOverviewData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview data (name, user version, threading mode).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/common/VersionNumber.hpp"

#include <QString>

/**
 * \class   SMOverviewData
 * \brief   Represents the `Overview` element of an `.fsml` document.
 *          Holds the machine name, the user-owned document version, the threading
 *          mode and a description. This is the FSM sibling of SIOverviewData; it does
 *          not carry the service-interface category.
 **/
class SMOverviewData : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eThreading
     * \brief   The FSM threading mode. `Shared` is the default because a
     *          wrong `Local` causes race conditions while a wrong `Shared` only costs
     *          performance.
     **/
    enum class eThreading
    {
          Shared    //!< The machine may be driven from several threads (locking generated).
        , Local     //!< Single-thread guarantee; lock-free generated code.
    };

    static constexpr const char* const  STR_THREADING_SHARED    { "Shared" };
    static constexpr const char* const  STR_THREADING_LOCAL     { "Local"  };

    /**
     * \brief   Converts a string to an eThreading value; unknown text yields `Shared`.
     **/
    static SMOverviewData::eThreading fromThreadingString(const QString& threading);

    /**
     * \brief   Converts an eThreading value to its string representation.
     **/
    static const char* toString(SMOverviewData::eThreading threading);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element (the document root).
     **/
    SMOverviewData(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id      The ID of the overview element.
     * \param   name    The name of the state machine.
     * \param   parent  The parent element.
     **/
    SMOverviewData(uint32_t id, const QString& name, ElementBase* parent = nullptr);

    virtual ~SMOverviewData(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Checks whether the overview is valid (has a non-empty machine name).
     **/
    virtual bool isValid(void) const override;

    /**
     * \brief   Reads data from an XML stream.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes data to an XML stream.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getName(void) const;
    inline void setName(const QString& name);

    inline const VersionNumber& getVersion(void) const;
    inline void setVersion(const VersionNumber& version);
    inline void setVersion(const QString& version);

    inline eThreading getThreading(void) const;
    inline void setThreading(eThreading threading);

    inline const QString& getDescription(void) const;
    inline void setDescription(const QString& description);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString         mName;          //!< The state machine name.
    VersionNumber   mVersion;       //!< The user-owned document version (Overview@Version).
    eThreading      mThreading;     //!< The threading mode.
    QString         mDescription;   //!< The description text.
};

//////////////////////////////////////////////////////////////////////////
// SMOverviewData inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMOverviewData::getName(void) const
{
    return mName;
}

inline void SMOverviewData::setName(const QString& name)
{
    mName = name;
}

inline const VersionNumber& SMOverviewData::getVersion(void) const
{
    return mVersion;
}

inline void SMOverviewData::setVersion(const VersionNumber& version)
{
    mVersion = version;
}

inline void SMOverviewData::setVersion(const QString& version)
{
    mVersion.fromString(version);
}

inline SMOverviewData::eThreading SMOverviewData::getThreading(void) const
{
    return mThreading;
}

inline void SMOverviewData::setThreading(SMOverviewData::eThreading threading)
{
    mThreading = threading;
}

inline const QString& SMOverviewData::getDescription(void) const
{
    return mDescription;
}

inline void SMOverviewData::setDescription(const QString& description)
{
    mDescription = description;
}

#endif  // LUSAN_DATA_SM_SMOVERVIEWDATA_HPP
