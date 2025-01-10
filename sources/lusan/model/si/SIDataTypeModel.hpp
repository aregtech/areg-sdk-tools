#ifndef LUSAN_MODEL_SI_SIDATATYPEMODEL_HPP
#define LUSAN_MODEL_SI_SIDATATYPEMODEL_HPP
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
 *  \file        lusan/model/si/SIDataTypeModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type Model.
 *
 ************************************************************************/

#include "lusan/data/common/DataTypeBase.hpp"
#include <QList>

class DataTypeCustom;
class ElementBase;
class EnumEntry;
class FieldEntry;
class SIDataTypeData;

/**
 * \class SIDataTypeModel
 * \brief Model to manage custom data types for the table view.
 */
class SIDataTypeModel
{
public:
    /**
     * \brief Constructor with initialization.
     * \param parent The parent object.
     */
    SIDataTypeModel(SIDataTypeData & data);

    inline SIDataTypeData& getDataTypeData(void);

    DataTypeCustom * createDataType(const QString & name, DataTypeBase::eCategory category);
    
    bool deleteDataType(uint32_t id);
    
    bool deleteDataType(const DataTypeCustom* dataType);
    
    DataTypeCustom* convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category);

    DataTypeCustom* findDataType(const QString& name);
    const DataTypeCustom* findDataType(const QString& name) const;
    
    DataTypeCustom* findDataType(uint32_t id);
    const DataTypeCustom* findDataType(uint32_t id) const;

    void sortDataTypes(bool ascending);

    const QList<DataTypeCustom*>& getDataTypes(void) const;

    int getDataTypeCount(void) const;

    ElementBase* ceateDataTypeChild(DataTypeCustom* dataType, const QString& name);
    
    void deleteDataTypeChild(DataTypeCustom* dataType, uint32_t childId);
    void deleteDataTypeChild(DataTypeCustom* dataType, const ElementBase& child);

    ElementBase* findDataTypeChild(DataTypeCustom* dataType, uint32_t childId);
    const ElementBase* findDataTypeChild(DataTypeCustom* dataType, uint32_t childId) const;

    QList<FieldEntry> getStructChildren(DataTypeCustom* dataType) const;

    QList<EnumEntry> getEnumChildren(DataTypeCustom* dataType) const;

    int getDataTypeChildCount(const DataTypeCustom* dataType) const;

    void sortDataTypeChildren(DataTypeCustom* dataType, bool ascending);

    bool hasChildren(const DataTypeCustom* dataType) const;

    bool canHaveChildren(const DataTypeCustom* dataType) const;

    int getChildCount(const DataTypeCustom* dataType) const;

    int findIndex(uint32_t id) const;

    int findIndex(const DataTypeCustom* dataType) const;

    int findChildIndex(const DataTypeCustom* dataType, uint32_t childId) const;

    int findChildIndex(const DataTypeCustom* dataType, const ElementBase& child) const;

    int findChildIndex(const DataTypeCustom* dataType, const QString& childName) const;

    ElementBase* findChild(const DataTypeCustom* dataType, uint32_t childId) const;

    ElementBase* findChild(const DataTypeCustom* dataType, const QString& childName) const;

private:
    SIDataTypeData& mData; //!< The data object.
};

inline SIDataTypeData& SIDataTypeModel::getDataTypeData(void)
{
    return mData;
}

#endif // LUSAN_MODEL_SI_SIDATATYPEMODEL_HPP
