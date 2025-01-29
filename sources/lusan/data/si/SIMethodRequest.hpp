#ifndef LUSAN_DATA_SI_SIMETHODREQUEST_HPP
#define LUSAN_DATA_SI_SIMETHODREQUEST_HPP
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
 *  \file        lusan/data/si/SIMethodRequest.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Request.
 *
 ************************************************************************/

#include "lusan/data/si/SIMethodBase.hpp"
#include "lusan/data/si/SIResponseLink.hpp"
#include "lusan/common/XmlSI.hpp"


/**
 * \class   SIMethodRequest
 * \brief   Represents a service interface method request in the Lusan application.
 **/
class SIMethodRequest : public SIMethodBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIMethodRequest(ElementBase * parent = nullptr);
    
    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the method.
     * \param   name            The name of the method.
     **/
    SIMethodRequest(uint32_t id, const QString& name, ElementBase* parent = nullptr);
    
    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the method.
     * \param   name            The name of the method.
     * \param   description     The description of the method.
     **/
    SIMethodRequest(uint32_t id, const QString& name, const QString& description, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    SIMethodRequest(const SIMethodRequest& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    SIMethodRequest(SIMethodRequest&& src) noexcept;

    /**
     * \brief   Destructor.
     **/
    virtual ~SIMethodRequest(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    SIMethodRequest& operator=(const SIMethodRequest& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    SIMethodRequest& operator=(SIMethodRequest&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Overrides
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

    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QIcon getIcon(ElementBase::eDisplay display) const override;

    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QString getString(ElementBase::eDisplay display) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    void normalize(const QList<SIMethodResponse*>& listResponses);

    void connectResponse(SIMethodResponse* respMethod);

    /**
     * \brief   Returns the connected response name.
     * \return  The connected response name.
     **/
    const QString& getConectedResponseName(void) const;

    /**
     * \brief   Returns the connected response object.
     * \return  The connected response object.
     **/
    const SIMethodResponse* getConectedResponse(void) const;

    /**
     * \brief   Clears the connected response name.
     **/
    void clearResponse(void);

private:
    SIResponseLink mResponse;
};

#endif // LUSAN_DATA_SI_SIMETHODREQUEST_HPP
