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
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMOverviewData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview data (name, user version, threading mode).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/OverviewDataSection.hpp"

/**
 * \class   SMOverviewData
 * \brief   The `Overview` element of an `.fsml` document: the shared name, version, description
 *          and deprecation mark, plus the threading mode, which only a state machine declares.
 **/
class SMOverviewData : public OverviewDataSection
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
protected:
    /**
     * \brief   Reads the threading mode.
     **/
    virtual void readOwnAttributes(const QXmlStreamAttributes& attributes) override;

    /**
     * \brief   Writes the threading mode.
     **/
    virtual void writeOwnAttributes(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eThreading getThreading(void) const;
    inline void setThreading(eThreading threading);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eThreading      mThreading;     //!< The threading mode.
};

//////////////////////////////////////////////////////////////////////////
// SMOverviewData inline methods
//////////////////////////////////////////////////////////////////////////

inline SMOverviewData::eThreading SMOverviewData::getThreading(void) const
{
    return mThreading;
}

inline void SMOverviewData::setThreading(SMOverviewData::eThreading threading)
{
    mThreading = threading;
}

#endif  // LUSAN_DATA_SM_SMOVERVIEWDATA_HPP
