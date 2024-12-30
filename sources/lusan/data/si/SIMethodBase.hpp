#ifndef LUSAN_DATA_SI_SIMETHODBASE_HPP
#define LUSAN_DATA_SI_SIMETHODBASE_HPP
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
 *  \file        lusan/data/si/SIMethodBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Method Base.
 *
 ************************************************************************/

#include "lusan/data/common/MethodBase.hpp"
#include <QString>

/**
 * \class   SIMethodBase
 * \brief   Represents a service interface method base in the Lusan application.
 **/
class SIMethodBase : public MethodBase
{
//////////////////////////////////////////////////////////////////////////
// Enumerations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eMethodType
     * \brief   Represents the type of the method.
     **/
    enum class eMethodType
    {
          MethodUnknown     //!< Unknown method type
        , MethodRequest     //!< Request method type
        , MethodResponse    //!< Response method type
        , MethodBroadcast   //!< Broadcast method type
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Default constructor.
     **/
    SIMethodBase(void);
    
    SIMethodBase(eMethodType methodType);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the method.
     * \param   name            The name of the method.
     * \param   description     The description of the method.
     * \param   methodType      The type of the method.
     **/
    SIMethodBase(uint32_t id, const QString& name, const QString& description, eMethodType methodType);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    SIMethodBase(const SIMethodBase& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    SIMethodBase(SIMethodBase&& src) noexcept;

    /**
     * \brief   Destructor.
     **/
    virtual ~SIMethodBase(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    SIMethodBase& operator=(const SIMethodBase& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    SIMethodBase& operator=(SIMethodBase&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the type of the method.
     * \return  The type of the method.
     **/
    eMethodType getMethodType() const;

    /**
     * \brief   Sets the type of the method.
     * \param   methodType  The type of the method.
     **/
    void setMethodType(eMethodType methodType);

    /**
     * \brief   Return the type of method as a string.
     **/
    QString getType(void) const;

    /**
     * \brief   Marks the method as deprecated.
     * \param   isDeprecated    Flag, indicating whether method is deprecated or not.
     * \param   hint            The hint, why method is deprecated.
     **/
    void markDeprecated(bool isDeprecated, const QString & hint);

    /**
     * \brief   Sets the method as deprecated.
     * \param   isDeprecated    Flag, indicating whether method is deprecated or not.
     **/
    void setDeprecated(bool isDeprecated);

    /**
     * \brief   Checks if the method is deprecated.
     * \return  True if the method is deprecated, false otherwise.
     **/
    bool isDeprecated(void) const;

    /**
     * \brief   Sets the hint, why method is deprecated.
     * \param   hint    The hint, why method is deprecated.
     **/
    void setDeprecateHint(const QString& hint);

    /**
     * \brief   Returns the hint, why method is deprecated.
     * \return  The hint, why method is deprecated.
     **/
    const QString& getDeprecateHint(void) const;

    /**
     * \brief   Converts the method type to a string value.
     * \param   methodType  The method type to convert.
     * \return  The string representation of the method type.
     **/
    static QString toString(eMethodType methodType);

    /**
     * \brief   Gets the method type from a string value.
     * \param   methodTypeStr   The string representation of the method type.
     * \return  The method type.
     **/
    static eMethodType fromString(const QString& methodTypeStr);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    eMethodType mMethodType;    //!< The type of the method.
    bool        mIsDeprecated;  //!< Flag, indicating whether method is deprecated or not.
    QString     mDeprecateHint; //!< The hint, why method is deprecated.
};

#endif // LUSAN_DATA_SI_SIMETHODBASE_HPP
