#ifndef LUSAN_DATA_SI_SIMETHODRESPONSE_HPP
#define LUSAN_DATA_SI_SIMETHODRESPONSE_HPP

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
 *  \file        lusan/data/si/SIMethodResponse.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Response.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodBase.hpp"
#include "lusan/common/XmlSI.hpp"

/**
 * \class   SIMethodResponse
 * \brief   Represents a service interface method response in the Lusan application.
 **/
class SIMethodResponse : public SIMethodBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIMethodResponse(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the method.
     * \param   name            The name of the method.
     * \param   description     The description of the method.
     **/
    SIMethodResponse(uint32_t id, const QString& name, const QString& description, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    SIMethodResponse(const SIMethodResponse& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    SIMethodResponse(SIMethodResponse&& src) noexcept;

    /**
     * \brief   Destructor.
     **/
    virtual ~SIMethodResponse(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    SIMethodResponse& operator=(const SIMethodResponse& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    SIMethodResponse& operator=(SIMethodResponse&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Reads data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;
};

#endif // LUSAN_DATA_SI_SIMETHODRESPONSE_HPP
