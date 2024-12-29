#ifndef LUSAN_DATA_SI_SIDATATYPEDATA_HPP
#define LUSAN_DATA_SI_SIDATATYPEDATA_HPP
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
 *  \file        lusan/data/si/SIDataTypeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data Type Data.
 *
 ************************************************************************/

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"

#include <memory>

class DataTypeCustom;

 /**
  * \class   SIDataTypeData
  * \brief   Manages data type data for service interfaces.
  **/
class SIDataTypeData
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIDataTypeData(void);

    SIDataTypeData(const SIDataTypeData& src) = default;

    /**
     * \brief   Constructor with initialization.
     * \param   entries     The list of data types.
     **/
    SIDataTypeData(QList<DataTypeCustom *>&& entries) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the list of data types.
     * \return  The list of data types.
     **/
    const QList<DataTypeCustom *>& getDataTypes(void) const;

    /**
     * \brief   Sets the list of data types.
     * \param   entries     The list of data types.
     **/
    void setDataTypes(QList<DataTypeCustom *>&& entries);

    /**
     * \brief   Searches for a data type in the list.
     * \param   entry   The data type to search for.
     * \return  The index of the data type, or -1 if not found.
     **/
    int findDataType(const DataTypeCustom& entry) const;

    /**
     * \brief   Adds a data type to the list.
     * \param   entry   The data type to add.
     **/
    void addDataType(DataTypeCustom* entry);

    /**
     * \brief   Removes a data type from the list.
     * \param   entry   The data type to remove.
     * \return  True if the data type was removed, false otherwise.
     **/
    bool removeDataType(const DataTypeCustom& entry);

    /**
     * \brief   Replaces a data type in the list.
     * \param   oldEntry    The data type to replace.
     * \param   newEntry    The new data type.
     * \return  True if the data type was replaced, false otherwise.
     **/
    bool replaceDataType(const DataTypeCustom& oldEntry, DataTypeCustom * newEntry);

    /**
     * \brief   Reads data type data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the data type data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes data type data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   remove all entries and frees resources.
     **/
    void removeAll(void);

    //////////////////////////////////////////////////////////////////////////
    // Hidden member variables.
    //////////////////////////////////////////////////////////////////////////
private:
    QList<DataTypeCustom *> mDataTypes; //!< The list of data types.
};

#endif  // LUSAN_DATA_SI_SIDATATYPEDATA_HPP
