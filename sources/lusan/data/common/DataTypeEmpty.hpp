#ifndef LUSAN_DATA_COMMON_DATATYPEEMPTY_HPP
#define LUSAN_DATA_COMMON_DATATYPEEMPTY_HPP
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
 *  \file        lusan/data/common/DataTypeEmpty.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Empty Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeBase.hpp"


 /**
  * \class   DataTypeEmpty
  * \brief   Base class for data types in the Lusan application.
  **/
class DataTypeEmpty : public DataTypeBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    DataTypeEmpty(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
protected:

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief Gets the ID of the element.
     * \return The ID of the element.
     */
    inline unsigned int getId(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Checks if the data type is valid.
     * \return  True if the data type is valid, false otherwise.
     **/
    virtual bool isValid(void) const override;

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
    
protected:
    /**
     * \brief Gets the next available ID.
     * \return The next available ID.
     */
    virtual unsigned int getNextId(void) const override;
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Sets the name of the data type.
     * \param   name    The name of the data type.
     **/
    inline void setName(const QString& name);

    /**
     * \brief Sets the ID of the element.
     * \param id The ID to set.
     */
    inline void setId(unsigned int id) const;

    /**
     * \brief Sets the parent element.
     * \param parent Pointer to the parent element.
     */
    inline void setParent(ElementBase* parent) const;

    /**
     * \brief Sets the parent element.
     * \param parent Pointer to the parent element (const).
     */
    inline void setParent(const ElementBase* parent) const;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeEmpty(const DataTypeEmpty& /*other*/) = delete;
    DataTypeEmpty(DataTypeEmpty&& /*other*/) noexcept = delete;
    DataTypeEmpty& operator = (const DataTypeEmpty& /*other*/) = delete;
    DataTypeEmpty& operator = (DataTypeEmpty&& /*other*/) noexcept = delete;

};

//////////////////////////////////////////////////////////////////////////
// DataTypeEmpty inline methods
//////////////////////////////////////////////////////////////////////////
inline void DataTypeEmpty::setName(const QString& /*name*/)
{
}

inline void DataTypeEmpty::setId(unsigned int /*id*/) const
{
}

inline void DataTypeEmpty::setParent(ElementBase* /*parent*/) const
{
}

inline void DataTypeEmpty::setParent(const ElementBase* /*parent*/) const
{
}

#endif  // LUSAN_DATA_COMMON_DATATYPEEMPTY_HPP
