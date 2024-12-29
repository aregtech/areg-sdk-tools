#ifndef LUSAN_DATA_COMMON_DATATYPEIMPORTED_HPP
#define LUSAN_DATA_COMMON_DATATYPEIMPORTED_HPP
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
 *  \file        lusan/data/common/DataTypeImported.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Imported Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeCustom.hpp"

/**
 * \class   DataTypeImported
 * \brief   Represents an imported data type in the Lusan application.
 **/
class DataTypeImported : public DataTypeCustom
{
//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeImported(void);

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeImported(const QString& name);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeImported(const DataTypeImported& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeImported(DataTypeImported&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeImported& operator = (const DataTypeImported& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeImported& operator = (DataTypeImported&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes, operations and overrides
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

private:
    QString mNamespace; //!< The namespace of the imported data type.
    QString mLocation;  //!< The location of the imported data type.
};

#endif // LUSAN_DATA_COMMON_DATATYPEIMPORTED_HPP

