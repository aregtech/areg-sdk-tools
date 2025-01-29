#ifndef LUSAN_DATA_COMMON_METHODPARAMETER_HPP
#define LUSAN_DATA_COMMON_METHODPARAMETER_HPP
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
 *  \file        lusan/data/common/MethodParameter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Method Parameter.
 *
 ************************************************************************/

#include "lusan/data/common/ParamBase.hpp"

/**
 * \class   MethodParameter
 * \brief   Represents a method parameter in the Lusan application.
 **/
class MethodParameter : public ParamBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    MethodParameter(ElementBase * parent = nullptr);
    
    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the parameter.
     * \param   name            The name of the parameter.
     * \param   type            The data type name of the parameter.
     * \param   value           The value of the parameter.
     * \param   isDefault       The default flag of the parameter.
     **/
    MethodParameter(  uint32_t id
                    , const QString& name
                    , bool isDefault        = false
                    , ElementBase* parent   = nullptr);
    
    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the parameter.
     * \param   name            The name of the parameter.
     * \param   type            The data type name of the parameter.
     * \param   value           The value of the parameter.
     * \param   isDefault       The default flag of the parameter.
     **/
    MethodParameter(  uint32_t id
                    , const QString& name
                    , const QString& type   = "bool"
                    , const QString& value  = ""
                    , bool isDefault        = false
                    , ElementBase* parent   = nullptr);
    
    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the parameter.
     * \param   name            The name of the parameter.
     * \param   type            The data type name of the parameter.
     * \param   isDeprecated    The deprecated flag of the parameter.
     * \param   description     The description of the parameter.
     * \param   deprecateHint   The deprecation hint of the parameter.
     * \param   value           The value of the parameter.
     * \param   isDefault       The default flag of the parameter.
     **/
    MethodParameter(  uint32_t id
                    , const QString& name
                    , const QString& type
                    , bool isDeprecated
                    , const QString& description
                    , const QString& deprecateHint
                    , const QString& value
                    , bool isDefault
                    , ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    MethodParameter(const MethodParameter& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    MethodParameter(MethodParameter&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    MethodParameter& operator = (const MethodParameter& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    MethodParameter& operator = (MethodParameter&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the value of the parameter.
     * \return  The value of the parameter.
     **/
    const QString& getValue() const;

    /**
     * \brief   Sets the value of the parameter.
     * \param   value   The value of the parameter.
     **/
    void setValue(const QString& value);

    /**
     * \brief   Gets the default flag of the parameter.
     * \return  The default flag of the parameter.
     **/
    bool hasDefault() const;

    /**
     * \brief   Sets the default flag of the parameter.
     * \param   isDefault   The default flag of the parameter.
     **/
    void setDefault(bool isDefault);

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
    virtual QIcon getIcon(ElementBase::eDisplay display) const;

    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QString getString(ElementBase::eDisplay display) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString mValue;    //!< The value of the parameter.
    bool    mIsDefault; //!< The default flag of the parameter.
};

#endif // LUSAN_DATA_COMMON_METHODPARAMETER_HPP
