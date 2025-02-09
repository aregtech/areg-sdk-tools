#ifndef LUSAN_DATA_COMMON_PARAMBASE_HPP
#define LUSAN_DATA_COMMON_PARAMBASE_HPP
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
 *  \file        lusan/data/common/ParamBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Parameter Base.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/ParamType.hpp"

#include <QString>
#include <memory>
#include "lusan/data/common/DataTypeBase.hpp"

/**
 * \class   ParamBase
 * \brief   Represents a parameter base in the Lusan application.
 **/
class ParamBase : public DocumentElem
{
    
    static const QString DefaultType;
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Default constructor.
     * \param   parent          The parent element.
     **/
    ParamBase(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the parameter.
     * \param   name            The name of the parameter.
     * \param   type            The data type name of the parameter.
     * \param   parent          The parent element.
     **/
    ParamBase(uint32_t id, const QString& name, const QString& type, ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the parameter.
     * \param   name            The name of the parameter.
     * \param   type            The data type name of the parameter.
     * \param   isDeprecated    The deprecated flag of the parameter.
     * \param   description     The description of the parameter.
     * \param   deprecateHint   The deprecation hint of the parameter.
     * \param   parent          The parent element.
     **/
    ParamBase(uint32_t id, const QString& name, const QString & type, bool isDeprecated, const QString& description, const QString& deprecateHint, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    ParamBase(const ParamBase& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    ParamBase(ParamBase&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    ParamBase& operator=(const ParamBase& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    ParamBase& operator=(ParamBase&& other) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other   The other object to compare with.
     * \return  True if the parameters are equal, false otherwise.
     **/
    bool operator==(const ParamBase& other) const;

    /**
     * \brief   Inequality operator.
     * \param   other   The other object to compare with.
     * \return  True if the parameters are not equal, false otherwise.
     **/
    bool operator!=(const ParamBase& other) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks if the parameter is valid.
     * \return  True if the parameter is valid, false otherwise.
     **/
    virtual bool isValid() const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Validates the parameter type object.
     * \param   customTypes The list of custom data types to validate.
     * \return  Returns true if the parameter type is valid, false otherwise.
     **/
    bool validate(const QList<DataTypeCustom*>& customTypes);

    /**
     * \brief   Invalidates the parameter type object.
     **/
    void invalidate(void);

    /**
     * \brief   Gets the name of the parameter.
     * \return  The name of the parameter.
     **/
    const QString& getName() const;

    /**
     * \brief   Sets the name of the parameter.
     * \param   name    The name of the parameter.
     **/
    void setName(const QString& name);

    /**
     * \brief   Gets the type of the parameter.
     * \return  The data type name of the parameter.
     **/
    const QString& getType() const;

    /**
     * \brief   Sets the type of the parameter.
     * \param   type    The data type name of the parameter.
     **/
    void setType(const QString & type);

    /**
     * \brief   Sets the type of the parameter.
     * \param   type            The data type name of the parameter.
     * \param   customTypes     The list of custom data types to validate.
     **/
    void setType(const QString& type, const QList<DataTypeCustom*>& customTypes);

    /**
     * \brief   Sets the parameter data type object.
     * \param   dataType    The data type object to set.
     **/
    void setParamType(DataTypeBase* dataType);

    /**
     * \brief   Returns the parameter data type object.
     **/
    DataTypeBase* getParamType(void) const;

    /**
     * \brief   Returns the deprecated flag of the parameter.
     **/
    bool getIsDeprecated() const;

    /**
     * \brief   Sets the deprecated flag of the parameter.
     * \param   isDeprecated    The deprecated flag of the parameter.
     **/
    void setIsDeprecated(bool isDeprecated);

    /**
     * \brief   Returns the description of the parameter.
     **/
    const QString& getDescription() const;

    /**
     * \brief   Sets the description of the parameter.
     * \param   description     The description of the parameter.
     **/
    void setDescription(const QString& description);

    /**
     * \brief   Returns the deprecation hint of the parameter.
     **/
    const QString& getDeprecateHint() const;

    /**
     * \brief   Sets the deprecation hint of the parameter.
     * \param   deprecateHint   The deprecation hint of the parameter.
     **/
    void setDeprecateHint(const QString& deprecateHint);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QString     mName;          //!< The name of the parameter.
    ParamType   mParamType;     //!< The name of data type of the parameter.
    bool        mIsDeprecated;  //!< The deprecated flag of the parameter.
    QString     mDescription;   //!< The description of the parameter.
    QString     mDeprecateHint; //!< The deprecation hint of the parameter.
};

#endif // LUSAN_DATA_COMMON_PARAMBASE_HPP
