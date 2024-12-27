#ifndef LUSAN_DATA_COMMON_DATATYPEENUM_HPP
#define LUSAN_DATA_COMMON_DATATYPEENUM_HPP
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
 *  \file        lusan/data/common/DataTypeEnum.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Enum Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include <QList>

/**
 * \class   DataTypeEnum
 * \brief   Represents an enumeration data type in the Lusan application.
 **/
class DataTypeEnum : public DataTypeCustom
{
    static constexpr const char* const  DEFAULT_VALUES  { "default" };

public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeEnum(void);

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeEnum(const QString& name);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeEnum(const DataTypeEnum& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeEnum(DataTypeEnum&& src) noexcept;

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeEnum& operator = (const DataTypeEnum& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeEnum& operator = (DataTypeEnum&& other) noexcept;

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
     * \brief   Gets the list of enum entries.
     * \return  The list of enum entries.
     **/
    const QList<EnumEntry>& getFieldList() const;

    /**
     * \brief   Sets the list of enum entries.
     * \param   fieldList   The list of enum entries.
     **/
    void setFieldList(const QList<EnumEntry>& fieldList);

private:
    QList<EnumEntry>        mFieldList; //!< The list of enum entries.
    DataTypePrimitiveInt    mDerived;   //!< The type of values. If name is empty, default is used.
};

#endif // LUSAN_DATA_COMMON_DATATYPEENUM_HPP
