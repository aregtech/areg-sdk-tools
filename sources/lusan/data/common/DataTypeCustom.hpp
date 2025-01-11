#ifndef LUSAN_DATA_COMMON_DATATYPECUSTOM_HPP
#define LUSAN_DATA_COMMON_DATATYPECUSTOM_HPP
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
 *  \file        lusan/data/common/DataTypeCustom.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeBase.hpp"

/**
 * \class   DataTypeCustom
 * \brief   Represents a custom data type in the Lusan application.
 **/
class DataTypeCustom : public DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Static members and types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the type of the data type.
     * \return  The type of the data type.
     **/
    static QString getType(DataTypeBase::eCategory category);

    /**
     * \brief   Converts the data type category to string.
     * \param   category    The category of the data type.
     * \return  The string representation of the data type category.
     **/
    static DataTypeBase::eCategory fromTypeString(const QString & type);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    DataTypeCustom(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id      The ID of the data type.
     * \param   parent  The parent element.
     **/
    DataTypeCustom(uint32_t id, ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with initialization of category.
     * \param   category    The category of the data type.
     * \param   parent      The parent element.
     **/
    DataTypeCustom(DataTypeBase::eCategory category, ElementBase* parent = nullptr);
    
    /**
     * \brief   Constructor with initialization.
     * \param   category    The category of the data type.
     * \param   id          The ID of the data type.
     * \param   name        The name of the data type.
     * \param   parent      The parent element.
     **/
    DataTypeCustom(DataTypeBase::eCategory category, uint32_t id, const QString& name, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeCustom(const DataTypeCustom& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeCustom(DataTypeCustom&& src) noexcept;
    
public:
    
    virtual ~DataTypeCustom(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeCustom& operator = (const DataTypeCustom& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeCustom& operator = (DataTypeCustom&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns description string of the custom data type.
     **/
    const QString& getDescription(void) const;

    /**
     * \brief   Sets the description of the data type.
     * \param   description The description of the data type.
     **/
    void setDescription(const QString& description);

    bool getIsDeprecated(void) const;

    void setDeprecated(bool isDeprecated);

    const QString& getDeprecateHint(void) const;

    void setDeprecateHint(const QString& hint);

    void setDeprecated(bool isDeprecated, const QString& reason);

    /**
     * \brief   Returns the name of the custom data type.
     **/
    QString getType(void) const;

    /**
     * \brief   Checks if the data type is valid.
     * \return  True if the data type is valid, false otherwise.
     **/
    virtual bool isValid(void) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QString     mDescription;   //!< The description of the data type.
    QString     mDeprecateHint; //!< The data type deprication reason.
    bool        mIsDeprecated;  //!< Flag, indicating the data type is deprecated.
};

#endif // LUSAN_DATA_COMMON_DATATYPECUSTOM_HPP

