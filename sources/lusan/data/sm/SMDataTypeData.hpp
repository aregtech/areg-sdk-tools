#ifndef LUSAN_DATA_SM_SMDATATYPEDATA_HPP
#define LUSAN_DATA_SM_SMDATATYPEDATA_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMDataTypeData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM data types registry. Reuses DataTypeCustom.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/common/DataTypeBase.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeCustom;

/**
 * \class   SMDataTypeData
 * \brief   The `DataTypeList` registry. Holds the document's custom data
 *          types (enumerations, structures, imported types) with the same declaration
 *          semantics as `.siml`. It owns the DataTypeCustom pointers and deletes them
 *          on destruction, mirroring SIDataTypeData without the UI-facing QObject
 *          signals (the data layer stays headless; change notification lives in model/sm).
 **/
class SMDataTypeData : public TEDataContainer<DataTypeCustom*, DocumentElem>
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMDataTypeData(ElementBase* parent = nullptr);
    virtual ~SMDataTypeData();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list of custom data types (document order).
     **/
    const QList<DataTypeCustom*>& getCustomDataTypes() const;

    /**
     * \brief   Creates a custom data type of the given category and appends it.
     * \param   name        The unique name of the new data type.
     * \param   category    The category (Enumeration, Structure, Imported, ...).
     * \return  Pointer to the created data type, or nullptr if the name already exists.
     **/
    DataTypeCustom* addCustomDataType(const QString& name, DataTypeBase::eCategory category);

    /**
     * \brief   Convenience creators for the FSM-relevant custom categories.
     **/
    DataTypeCustom* addEnum(const QString& name);

    DataTypeCustom* addStructure(const QString& name);
    
    DataTypeCustom* addImported(const QString& name);

    /**
     * \brief   Finds a custom data type by name.
     **/
    DataTypeCustom* findCustomDataType(const QString& name) const;

    /**
     * \brief   Finds a custom data type by ID.
     **/
    DataTypeCustom* findCustomDataType(uint32_t id) const;

    /**
     * \brief   Deletes and removes all custom data types.
     **/
    void removeAll();

    /**
     * \brief   Resolves every structure field's and container key/value's declared type
     *          against the registry, so a freshly loaded document reflects the same
     *          resolved state as one edited interactively (a parsed entry only has the
     *          type name until resolved).
     * \param   dataTypes   The document's data type registry (itself, passed explicitly to
     *                      mirror `SIDataTypeData::validate`).
     **/
    void validate(const SMDataTypeData& dataTypes);

private:

    //!< Resolves one custom type's nested type references (structure fields, container
    //!< key/value) against the registry; no-op for enumeration and imported types.
    void normalizeType(DataTypeCustom* dataType) const;
};

#endif  // LUSAN_DATA_SM_SMDATATYPEDATA_HPP
