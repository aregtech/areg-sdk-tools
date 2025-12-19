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
 *  with this distribution or contact us at info[at]areg.tech.
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
    static constexpr const char * const DefaultContaier {"Array"};  //!< The default basic container type.
    static constexpr const char * const DefaultKeys     {"bool"};   //!< The default basic container key type.
    static constexpr const char * const DefaultValues   {"bool"};   //!< The default basic container value type.

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
// Overrides
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

    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QIcon getIcon(ElementBase::eDisplay display) const override;
    
    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QString getString(ElementBase::eDisplay display) const override;
    
//////////////////////////////////////////////////////////////////////////
// Attributes, operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns true if the container can have a key.
     **/
    bool canHaveKey(void) const;

    /**
     * \brief   Returns the name of basic container type.
     **/
    inline const QString& getContainer(void) const;

    /**
     * \brief   Sets the name of basic container type.
     * \param   valContainer    The name of basic container type.
     **/
    inline void setContainer(const QString& valContainer);

    /**
     * \brief   Returns true if the container has a key.
     **/
    inline bool hasKey(void) const;

    /**
     * \brief   Returns the name of basic container key type.
     **/
    inline const QString& getKey(void) const;

    /**
     * \brief   Sets the name of basic container key type.
     * \param   key     The name of basic container key type.
     **/
    inline void setKey(const QString& key);

    /**
     * \brief   Sets the name of basic container key type.
     * \param   key         The name of basic container key type.
     * \param   customTypes The list of custom data types to validate.
     **/
    inline void setKey(const QString& key, const QList<DataTypeCustom *>& customTypes);

    /**
     * \brief   Returns the data type of basic container key.
     **/
    inline DataTypeBase* getKeyDataType(void) const;

    /**
     * \brief   Sets the data type of basic container key.
     * \param   dataType    The data type of basic container key.
     **/
    inline void setKeyDataType(DataTypeBase *dataType);

    /**
     * \brief   Returns the name of basic container value type.
     **/
    inline const QString& getValue(void) const;

    /**
     * \brief   Sets the name of basic container value type.
     * \param   value   The name of basic container value type.
     **/
    inline void setValue(const QString& value);

    /**
     * \brief   Sets the name of basic container value type.
     * \param   value       The name of basic container value type.
     * \param   customTypes The list of custom data types to validate.
     **/
    inline void setValue(const QString& value, const QList<DataTypeCustom*>& customTypes);

    /**
     * \brief   Returns the data type of basic container value.
     **/
    inline DataTypeBase* getValueDataType(void) const;

    /**
     * \brief   Sets the data type of basic container value.
     * \param   dataType    The data type of basic container value.
     **/
    inline void setValueDataType(DataTypeBase *dataType);

    /**
     * \brief   Validates the data type.
     * \param   customTypes The list of custom data types to validate.
     * \return  Returns true if the data type validation succeeded.
     **/
    bool validate(const QList<DataTypeCustom *>& customTypes);

    /**
     * \brief   Invalidates the data type.
     **/
    void invalidate(void);
    
private:

    /**
     * \brief   Returns the string to display as basic container type.
     **/
    QString toTypeString(void) const;
        
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
    if (canHaveKey())
    {
        if (mKeyType.isEmpty())
            mKeyType = DefaultKeys;
    }
    else if (mKeyType.isEmpty() == false)
    {
        mKeyType.invalidate();
    }
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
    return const_cast<ParamType &>(mKeyType).getType();
}

inline void DataTypeContainer::setKeyDataType(DataTypeBase* dataType)
{
    mKeyType.setType(dataType);
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
    return const_cast<ParamType &>(mValueType).getType();
}

inline void DataTypeContainer::setValueDataType(DataTypeBase* dataType)
{
    mValueType.setType(dataType);
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
