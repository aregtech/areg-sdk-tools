#ifndef LUSAN_DATA_SI_SIMETHODRESPONSE_HPP
#define LUSAN_DATA_SI_SIMETHODRESPONSE_HPP

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
 *  \file        lusan/data/si/SIMethodResponse.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Response.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodBase.hpp"
#include "lusan/common/XmlSI.hpp"
#include <QMetaMethod>

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
     **/
    SIMethodResponse(uint32_t id, const QString& name, ElementBase* parent = nullptr);
    
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
    virtual ~SIMethodResponse();

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
    bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    QIcon getIcon(ElementBase::eDisplay display) const override;

    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    QString getString(ElementBase::eDisplay display) const override;
};

#endif // LUSAN_DATA_SI_SIMETHODRESPONSE_HPP
