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
 *  with this distribution or contact us at info[at]areg.tech.
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
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

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
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Adds new fields to the structure object.
     * \param   name    The unique name of the field to add. By default, new field has type of 'bool'.
     * \return  If operations succeeds, i.e. the name is unique, returns valid pointer to the new added field.
     *          Otherwise, returns nullptr.
     **/
    FieldEntry* addField(const QString& name);

    /**
     * \brief   Inserts new fields to the structure object at the given position.
     * \param   position    The position to add new field.
     * \param   name        The unique name of the field to add. By default, new field has type of 'bool'.
     * \return  If operations succeeds, i.e. the name is unique, returns valid pointer to the new added field.
     *          Otherwise, returns nullptr.
     **/
    FieldEntry* insertField(int position, const QString& name);

    /**
     * \brief   Removes the field from the structure object by given name.
     * \param   name    The name of the field to remove.
     **/
    void removeField(const QString& name);

    /**
     * \brief   Removes the field from the structure object by given ID.
     * \param   id  The ID of the field to remove.
     **/
    void removeField(uint32_t id);

    /**
     * \brief   Returns the data type of the field by given name.
     * \param   name    The name of the field to get data type.
     * \return  Returns the data type of the field by given name. If field does not exist, returns nullptr.
     **/
    DataTypeBase* getFieldType(const QString& name) const;

    /**
     * \brief   Returns the data type of the field by given ID.
     * \param   id  The ID of the field to get data type.
     * \return  Returns the data type of the field by given ID. If field does not exist, returns nullptr.
     **/
    DataTypeBase* getFieldType(uint32_t id) const;

    /**
     * \brief   Validates the structure data type.
     * \param   customTypes The list of custom data types to validate.
     * \return  Returns true if the structure is valid, false otherwise.
     **/
    bool validate(const QList<DataTypeCustom*>& customTypes);

    /**
     * \brief   Invalidates the structure data type.
     **/
    void invalidate(void);
};

#endif // LUSAN_DATA_COMMON_DATATYPESTRUCTURE_HPP

