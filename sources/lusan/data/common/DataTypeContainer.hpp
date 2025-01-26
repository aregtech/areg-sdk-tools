#ifndef LUSAN_DATA_COMMON_DATATYPECONTAINER_HPP
#define LUSAN_DATA_COMMON_DATATYPECONTAINER_HPP
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
 *  \file        lusan/data/common/DataTypeContainer.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Container Data Type.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/ParamType.hpp"

/**
 * \class   DataTypeContainer
 * \brief   Represents a Container data type in the Lusan application.
 **/
class DataTypeContainer : public DataTypeCustom
{
//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    DataTypeContainer(ElementBase* parent = nullptr);

    /**
     * \brief   Constructor with name initialization.
     * \param   name    The name of the data type.
     **/
    DataTypeContainer(const QString& name, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    DataTypeContainer(const DataTypeContainer& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    DataTypeContainer(DataTypeContainer&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    DataTypeContainer& operator = (const DataTypeContainer& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    DataTypeContainer& operator = (DataTypeContainer&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes, operations and overrides
//////////////////////////////////////////////////////////////////////////
public:
    
    QString toString(void);

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
    
    inline void setKey(const QString& key, const QList<DataTypeCustom *>& customTypes);
    
    inline DataTypeBase* getKeyDataType(void) const;
    
    inline void setKeyDataType(DataTypeBase *dataType);
    
    inline const QString& getValue(void) const;

    inline void setValue(const QString& value);
    
    inline void setValue(const QString& value, const QList<DataTypeCustom*>& customTypes);

    inline DataTypeBase* getValueDataType(void) const;
    
    inline void setValueDataType(DataTypeBase *dataType);
    
    bool validate(const QList<DataTypeCustom *>& customTypes);

    void invalidate(void);
    
private:
    QString     mContainer; //!< The container type.
    ParamType   mValueType; //!< The base type value.
    ParamType   mKeyType;   //!< The base type key (optional).
};

//////////////////////////////////////////////////////////////////////////
// DataTypeContainer class inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& DataTypeContainer::getContainer(void) const
{
    return mContainer;
}

inline void DataTypeContainer::setContainer(const QString& valContainer)
{
    mContainer = valContainer;
}

inline bool DataTypeContainer::hasKey(void) const
{
    return mKeyType.isEmpty() == false;
}

inline const QString& DataTypeContainer::getKey(void) const
{
    return mKeyType.getName();
}

inline void DataTypeContainer::setKey(const QString& key)
{
    mKeyType.setName(canHaveKey() ? key : QString());
}

inline void DataTypeContainer::setKey(const QString& key, const QList<DataTypeCustom *>& customTypes)
{
    mKeyType.setName(key, customTypes);
}

inline DataTypeBase* DataTypeContainer::getKeyDataType(void) const
{
    return const_cast<ParamType &>(mKeyType).getDataType();
}

inline void DataTypeContainer::setKeyDataType(DataTypeBase* dataType)
{
    mKeyType.setDataType(dataType);
}

inline const QString& DataTypeContainer::getValue(void) const
{
    return mValueType.getName();
}

inline void DataTypeContainer::setValue(const QString& value)
{
    mValueType.setName(value);
}

inline void DataTypeContainer::setValue(const QString& value, const QList<DataTypeCustom*>& customTypes)
{
    mValueType.setName(value, customTypes);
}

inline DataTypeBase* DataTypeContainer::getValueDataType(void) const
{
    return const_cast<ParamType &>(mValueType).getDataType();
}

inline void DataTypeContainer::setValueDataType(DataTypeBase* dataType)
{
    mValueType.setDataType(dataType);
}

inline bool DataTypeContainer::validate(const QList<DataTypeCustom*>& customTypes)
{
    if (canHaveKey())
    {
        return mValueType.validate(customTypes) && mKeyType.validate(customTypes);
    }
    else
    {
        mKeyType.invalidate();
        return mValueType.validate(customTypes);
    }
}

inline void DataTypeContainer::invalidate(void)
{
    mValueType.invalidate();
    mKeyType.invalidate();
}

#endif // LUSAN_DATA_COMMON_DATATYPECONTAINER_HPP
