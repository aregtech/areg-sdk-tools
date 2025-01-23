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
#include "lusan/data/common/DataTypePrimitive.hpp"
#include "lusan/data/common/DataTypeBasic.hpp"
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

    /**
     * \brief Returns instance of data type data object.
     */
    inline SIDataTypeData& getDataTypeData(void);
    
    /**
     * \brief   Returns the list of basic container data types objects.
     **/
    const QList<DataTypeBasicContainer*>& getContainerDatTypes(void) const;

    /**
     * \brief   Returns the list of custom data types objects.
     **/
    const QList<DataTypeCustom *>& getCustomDataTypes(void) const;

    /**
     * \brief   Gets list of data types of specified categories and returns number of data types copied.
     * \param   result          On output, this contains the list of data types.
     * \param   categories      The list of data type categories to include.
     * \param   makeSorting     If true, the list of data types will be sorted.
     * \return  Returns the number of data types copied to the 'result' list.
     **/
    int getDataTypes(QList<DataTypeBase *> & result, const QList<DataTypeBase::eCategory> & categories, bool makeSorting = false);

    /**
     * \brief   Creates new data type object with specified name and category.
     * \param   name        The name of data type.
     * \param   category    The category of data type.
     * \return  Returns the pointer to created data type object.
     **/
    DataTypeCustom * createDataType(const QString & name, DataTypeBase::eCategory category);

    /**
     * \brief   Deletes data type object from the list by specified ID.
     * \param   id  The ID of data type to delete.
     * \return  Returns true if data type object was deleted. Otherwise, returns false.
     **/
    bool deleteDataType(uint32_t id);

    /**
     * \brief   Deletes data type object from the list.
     * \param   dataType    The pointer to data type object to delete.
     * \return  Returns true if data type object was deleted. Otherwise, returns false.
     **/
    bool deleteDataType(const DataTypeCustom* dataType);

    /**
     * \brief   Converts data type object to new category.
     * \param   dataType    The pointer to data type object to convert.
     * \param   category    The new category of data type.
     * \return  Returns the pointer to converted data type object.
     **/
    DataTypeCustom* convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category);

    /**
     * \brief   Searches for data type object by specified name.
     * \param   name    The name of data type to search.
     * \return  Returns the pointer to data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeCustom* findDataType(const QString& name);
    const DataTypeCustom* findDataType(const QString& name) const;

    /**
     * \brief   Searches for data type object by specified ID.
     * \param   id  The ID of data type to search.
     * \return  Returns the pointer to data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeCustom* findDataType(uint32_t id);
    const DataTypeCustom* findDataType(uint32_t id) const;

    /**
     * \brief   Sorts the list of custom data types by name.
     * \param   ascending   Flag, indicating whether the list of custom data types
     *                      should be sorted ascending or descending.
     *                      If true, the sorting is ascending.
     **/
    void sortByName(bool ascending);

    /**
     * \brief   Sorts the list of custom data types by ID.
     * \param   ascending   Flag, indicating whether the list of custom data types
     *                      should be sorted ascending or descending.
     *                      If true, the sorting is ascending.
     **/
    void sortById(bool ascending);

    /**
     * \brief   Returns the list of custom data types.
     **/
    const QList<DataTypeCustom*>& getDataTypes(void) const;

    /**
     * \brief   Returns the size of custom data type objects.
     **/
    int getDataTypeCount(void) const;

    /**
     * \brief   Creates a new child field in the specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   name        The name of the child field of the custom data type.
     *                      The name should be unique.
     * \return  Returns the pointer to created child data type object.
     **/
    ElementBase* ceateDataTypeChild(DataTypeCustom* dataType, const QString& name);

    /**
     * \brief   Deletes the child field from the specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   childId     The ID of the child field to delete.
     **/
    void deleteDataTypeChild(DataTypeCustom* dataType, uint32_t childId);

    /**
     * \brief   Deletes the child field from the specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   child       The child field to delete.
     **/
    void deleteDataTypeChild(DataTypeCustom* dataType, const ElementBase& child);

    /**
     * \brief   Finds the child field in the specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   childId     The ID of the child field to find.
     * \return  Returns the pointer to found child field object. If not found, returns nullptr.
     **/
    ElementBase* findDataTypeChild(DataTypeCustom* dataType, uint32_t childId);
    const ElementBase* findDataTypeChild(DataTypeCustom* dataType, uint32_t childId) const;

    /**
     * \brief   Returns the list of specified custom data type object fields.
     * \param   dataType    The structure data type object.
     **/
    const QList<FieldEntry>& getStructChildren(DataTypeCustom* dataType) const;

    /**
     * \brief   Returns the list of specified custom data type object fields.
     * \param   dataType    The enumeration data type object.
     **/
    const QList<EnumEntry>& getEnumChildren(DataTypeCustom* dataType) const;

    /**
     * \brief   Sorts the data type fields entries by name.
     * \param   dataType    The custom data type object to sort the fields.
     * \param   ascending   If true, the sorting is ascending.
     **/
    void sortDataTypeChildren(DataTypeCustom* dataType, bool ascending);

    /**
     * \brief   Returns true if data type has child fields.
     * \param   dataType    The custom data type object to sort the fields.
     **/
    bool hasChildren(const DataTypeCustom* dataType) const;

    /**
     * \brief   Returns true if data type can have child fields,
     *          i.e. data type is structure or enumeration
     * \param   dataType    The custom data type object to sort the fields.
     **/
    bool canHaveChildren(const DataTypeCustom* dataType) const;

    /**
     * \brief   Returns the number of child fields in the specified custom data type object.
     * \param   dataType    The custom data type object.
     **/
    int getChildCount(const DataTypeCustom* dataType) const;

    /**
     * \brief   Returns the index of the specified custom data type ID object in the list.
     * \param   id  The ID of the custom data type object.
     * \return  Returns valid index of the custom data type object in the list. Otherwise, returns -1.
     **/
    int findIndex(uint32_t id) const;

    /**
     * \brief   Returns the index of the specified custom data type object in the list.
     * \param   dataType    The custom data type object.
     * \return  Returns valid index of the custom data type object in the list. Otherwise, returns -1.
     **/
    int findIndex(const DataTypeCustom* dataType) const;

    /**
     * \brief   Returns the index of the child field of specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   childId     The ID of the child field.
     * \return  Returns valid index of the custom data type object in the list. Otherwise, returns -1.
     **/
    int findChildIndex(const DataTypeCustom* dataType, uint32_t childId) const;

    /**
     * \brief   Returns the index of the child field of specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   child       The child field object.
     * \return  Returns valid index of the custom data type object in the list. Otherwise, returns -1.
     **/
    int findChildIndex(const DataTypeCustom* dataType, const ElementBase& child) const;

    /**
     * \brief   Returns the index of the child field of specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   childName   The name of the child field.
     * \return  Returns valid index of the custom data type object in the list. Otherwise, returns -1.
     **/
    int findChildIndex(const DataTypeCustom* dataType, const QString& childName) const;

    /**
     * \brief   Returns the pointer of the child field of specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   childId     The ID of the child field.
     * \return  Returns valid pointer to child field object. If not found, returns nullptr.
     **/
    ElementBase* findChild(const DataTypeCustom* dataType, uint32_t childId) const;

    /**
     * \brief   Returns the pointer of the child field of specified custom data type object.
     * \param   dataType    The custom data type object.
     * \param   child       The child field object.
     * \return  Returns valid pointer to child field object. If not found, returns nullptr.
     **/
    ElementBase* findChild(const DataTypeCustom* dataType, const QString& childName) const;

    /**
     * \brief   Updates the data type object.
     * \param   dataType    The data type object to update.
     * \param   newName     The new name of the data type.
     **/
    void updateDataType(DataTypeCustom* dataType, const QString& newName);

    /**
     * \brief   Updates the data type object.
     * \param   id          The ID of the data type object to update.
     * \param   newName     The new name of the data type.
     **/
    void updateDataType(uint32_t id, const QString& newName);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIDataTypeData& mData; //!< The data object.
};

//////////////////////////////////////////////////////////////////////////
// SIDataTypeModel class inline methods
//////////////////////////////////////////////////////////////////////////

inline SIDataTypeData& SIDataTypeModel::getDataTypeData(void)
{
    return mData;
}

#endif // LUSAN_MODEL_SI_SIDATATYPEMODEL_HPP
