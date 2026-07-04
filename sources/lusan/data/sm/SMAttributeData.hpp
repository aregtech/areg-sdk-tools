#ifndef LUSAN_DATA_SM_SMATTRIBUTEDATA_HPP
#define LUSAN_DATA_SM_SMATTRIBUTEDATA_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMAttributeData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM machine attributes registry (spec 6.10).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/ParamBase.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

/**
 * \class   SMAttributeEntry
 * \brief   One machine attribute (spec 6.10): a typed internal variable with a default
 *          value. Deliberately derives ParamBase (name/type/value/description) rather
 *          than the Service-Interface AttributeEntry, because the FSM model must not
 *          carry SI pub-sub notification state.
 **/
class SMAttributeEntry : public ParamBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMAttributeEntry(ElementBase* parent = nullptr);

    SMAttributeEntry(  uint32_t id
                     , const QString& name
                     , const QString& type    = "bool"
                     , const QString& value   = QString()
                     , ElementBase* parent    = nullptr);

    SMAttributeEntry(const SMAttributeEntry& src);
    SMAttributeEntry(SMAttributeEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    SMAttributeEntry& operator = (const SMAttributeEntry& other);
    SMAttributeEntry& operator = (SMAttributeEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getValue(void) const;
    inline void setValue(const QString& value);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString mValue;     //!< The default value of the attribute.
};

//////////////////////////////////////////////////////////////////////////
// SMAttributeData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMAttributeData
 * \brief   The `AttributeList` registry: an ordered container of SMAttributeEntry.
 **/
class SMAttributeData : public TEDataContainer<SMAttributeEntry, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMAttributeData(ElementBase* parent = nullptr);
    SMAttributeData(const QList<SMAttributeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates a new attribute appended at the end of the list.
     * \param   name    The unique name of the new attribute.
     * \return  Pointer to the created attribute, or nullptr if the name already exists.
     **/
    SMAttributeEntry* createAttribute(const QString& name);
};

//////////////////////////////////////////////////////////////////////////
// SMAttributeEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMAttributeEntry::getValue(void) const
{
    return mValue;
}

inline void SMAttributeEntry::setValue(const QString& value)
{
    mValue = value;
}

#endif  // LUSAN_DATA_SM_SMATTRIBUTEDATA_HPP
