#ifndef LUSAN_DATA_COMMON_DATATYPESTRUCTURE_HPP
#define LUSAN_DATA_COMMON_DATATYPESTRUCTURE_HPP
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
 *  \file        lusan/data/common/DataTypeStructure.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Structure Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/TEDataTypeContainer.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include <QList>

/**
 * \class   DataTypeStructure
 * \brief   Represents a structure data type in the Lusan application.
 **/
class DataTypeStructure : public TEDataTypeContainer<FieldEntry>
{

    static constexpr const char* const  DefName{ "field" };

public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeStructure(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeStructure(const QString& name, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeStructure(const DataTypeStructure& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeStructure(DataTypeStructure&& src) noexcept;

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeStructure& operator = (const DataTypeStructure& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeStructure& operator = (DataTypeStructure&& other) noexcept;

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

    FieldEntry* addField(const QString& name);
};

#endif // LUSAN_DATA_COMMON_DATATYPESTRUCTURE_HPP

