#ifndef LUSAN_DATA_COMMON_DATATYPEDEFINED_HPP
#define LUSAN_DATA_COMMON_DATATYPEDEFINED_HPP
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
 *  \file        lusan/data/common/DataTypeDefined.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Defined Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeCustom.hpp"

/**
 * \class   DataTypeDefined
 * \brief   Represents a defined data type in the Lusan application.
 **/
class DataTypeDefined : public DataTypeCustom
{
//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeDefined(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeDefined(const QString& name, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeDefined(const DataTypeDefined& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeDefined(DataTypeDefined&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeDefined& operator = (const DataTypeDefined& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeDefined& operator = (DataTypeDefined&& other) noexcept;

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

    bool canHaveKey(void) const;

    inline const QString& getContainer(void) const;

    inline void setContainer(const QString& valContainer);

    inline bool hasKey(void) const;


    inline const QString& getKey(void) const;

    inline void setKey(const QString& key);

    inline const QString& getValue(void) const;

    inline void setValue(const QString& value);

private:
    QString mContainer; //!< The container type.
    QString mValue;     //!< The base type value.
    QString mKey;       //!< The base type key (optional).
};

//////////////////////////////////////////////////////////////////////////
// DataTypeDefined class inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& DataTypeDefined::getContainer(void) const
{
    return mContainer;
}

inline void DataTypeDefined::setContainer(const QString& valContainer)
{
    mContainer = valContainer;
}

inline bool DataTypeDefined::hasKey(void) const
{
    return mKey.isEmpty() == false;
}

inline const QString& DataTypeDefined::getKey(void) const
{
    return mKey;
}

inline void DataTypeDefined::setKey(const QString& key)
{
    mKey = key;
}

inline const QString& DataTypeDefined::getValue(void) const
{
    return mValue;
}

inline void DataTypeDefined::setValue(const QString& value)
{
    mValue = value;
}

#endif // LUSAN_DATA_COMMON_DATATYPEDEFINED_HPP
