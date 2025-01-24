#ifndef LUSAN_MODEL_COMMON_DATATYPESMODEL_HPP
#define LUSAN_MODEL_COMMON_DATATYPESMODEL_HPP
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
 *  \file        lusan/model/common/DataTypesModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Types Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QAbstractListModel>
#include "lusan/data/common/DataTypeBase.hpp"

#include "lusan/data/common/DataTypeEmpty.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeBase;
class DataTypeCustom;
class SIDataTypeData;

/**
 * \class   DataTypesModel
 * \brief   Model to manage and display data types in a QComboBox.
 **/
class DataTypesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The instance of SIDataTypeData.
     * \param   parent          The parent object.
     **/
    DataTypesModel(SIDataTypeData& dataTypeData, bool hasEmpty, QObject* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The instance of SIDataTypeData.
     * \param   excludes        The list of data types to exclude.
     * \param   parent          The parent object.
     **/
    DataTypesModel(SIDataTypeData& dataTypeData, const QStringList &excludes, bool hasEmpty, QObject* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The instance of SIDataTypeData.
     * \param   excludes        The list of data types to exclude.
     * \param   parent          The parent object.
     **/
    DataTypesModel(SIDataTypeData& dataTypeData, const QList<DataTypeBase*> &excludes, bool hasEmpty, QObject* parent = nullptr);

    /**
     * \brief   Sets the list of data type objects when need to display data type elements.
     * \param   excludes        The list of data type names to exclude.
     **/
    void setFilter(const QStringList& excludes);

    /**
     * \brief   Sets the list of data type objects when need to display data type elements.
     * \param   excludes        The list of data types to exclude.
     **/
    void setFilter(const QList<DataTypeBase*>& excludes);

    /**
     * \brief   Sets the list of data type objects when need to display data type elements.
     * \param   excludes     The list of data type categories to exclude.
     **/
    void setFilter(const QList<DataTypeBase::eCategory> & excludes);

    /**
     * \brief   Marks all data types excluded, except the given names to include.
     * \param   inclusive   The list of data type names to include.
     **/
    void setInclusiveFilter(const QStringList& inclusive);

    /**
     * \brief   Marks all data types excluded, except the given data types to include.
     * \param   inclusive   The list of data types to include.
     **/
    void setInclusiveFilter(const QList<DataTypeBase*>& inclusive);

    /**
     * \brief   Marks all data types excluded, except the given categories to include.
     * \param   inclusive   The list of data type categories to include.
     **/
    void setInclusiveFilter(const QList<DataTypeBase::eCategory>& inclusive);

    /**
     * \brief   Adds the data type to the filter list to exclude.
     * \param   dataType    The data type to add to the filter list.
     **/
    void addToFilter(const DataTypeBase* dataType);

    /**
     * \brief   Removes the data type from the filter list.
     * \param   dataType    The data type to remove from the filter list.
     **/
    void removeFromFilter(const DataTypeBase* dataType);
    
    /**
     * \brief   Clears the filter list.
     **/
    void clearFilter();

    /**
     * \brief   Returns the number of rows in the model.
     * \param   parent  The parent index.
     * \return  The number of rows.
     **/
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * \brief   Returns the data for the given role and section in the model.
     * \param   index   The index of the item.
     * \param   role    The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * \brief   Triggered when new data type is created.
     * \param   dataType    New created data type object.
     * \return  Returns true if new created data type is in the list. Otherwise, returns false.
     **/
    bool dataTypeCreated(DataTypeCustom* dataType);

    /**
     * \brief   Triggered when the data type is converted.
     * \param   oldType     The old data type object.
     * \param   newType     The new data type object.
     * \return  Returns true if the old data type is converted to the new data type. Otherwise, returns false.
     **/
    bool dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    /**
     * \brief   Triggered when the data type is deleted and invalidated.
     * \param   dataType    The data type object to be deleted.
     * \return  Returns true if the data type is removed from the list. Otherwise, returns false.
     **/
    bool dataTypeDeleted(DataTypeCustom* dataType);

    /**
     * \brief   Triggered when the data type is updated.
     * \param   dataType    The data type object to update.
     * \return  Returns true if the data type is updated. Otherwise, returns false.
     **/
    bool dataTypeUpdated(DataTypeCustom* dataType);

    /**
     * \brief   Updates the list of data types.
     *          Uses filter list to exclude data types.
     **/
    void updateDataTypeLists(void);

    /**
     * \brief   Searches for the data type in the list. If find, removes and includes in the filter.
     * \param   typeName    The name of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    bool removeDataType(DataTypeCustom* dataType);

    /**
     * \brief   Searches the field entry in the data type fields list
     *          and removes it if found.
     * \param   dataType    The data type object to check the field list.
     * \param   fieldId     The ID of the field to remove.
     * \return  Returns true if successfully removed the field.
     **/
    bool removeField(DataTypeCustom* dataType, uint32_t fieldId);

    /**
     * \brief   Adds the data type to the list.
     * \param   dataType    The data type object to add.
     * \return  Returns true if the data type is added. Otherwise, returns false.
     **/
    bool addDataType(DataTypeCustom* dataType);

    /**
     * \brief   Returns flag, indicating whether the list can have an empty entry.
     **/
    inline bool hasEmptyEntry(void) const;

    inline void addEmptyEntry(void);

    inline void removeEmptyEntry(void);

//////////////////////////////////////////////////////////////////////////
// Hidden calls.
//////////////////////////////////////////////////////////////////////////
private:

    inline void _sort(bool sortPredefined = true);

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
private:
    SIDataTypeData&         mDataTypeData;  //!< Reference to the SIDataTypeData instance.
    QList<DataTypeBase*>    mExcludeList;   //!< Filtered list of data types.
    QList<DataTypeBase*>    mDataTypeList;  //!< The list of all data types.
    int                     mCountPredef;   //!< The number of predefined entries, which are set at the beginning of mDataTypeList;
    const bool              mHasEmpty;      //!< Flag, indicating whether there can be an empty entry in the list.
    static DataTypeEmpty    _emptyType;     //!< The empty data type object.
};

//////////////////////////////////////////////////////////////////////////
// DataTypesModel class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool DataTypesModel::hasEmptyEntry(void) const
{
    return mHasEmpty;
}

inline void DataTypesModel::addEmptyEntry(void)
{
    if (mHasEmpty && (mDataTypeList.indexOf(&_emptyType) < 0))
    {
        ++ mCountPredef;
        beginInsertRows(QModelIndex(), 0, 0);
        mDataTypeList.insert(0, &_emptyType);
        endInsertRows();
    }
}

inline void DataTypesModel::removeEmptyEntry(void)
{
    if (mHasEmpty && (mDataTypeList.indexOf(&_emptyType) >= 0))
    {
        --mCountPredef;
        beginRemoveRows(QModelIndex(), 0, 0);
        mDataTypeList.removeAll(&_emptyType);
        endRemoveRows();
    }
}

#endif  // LUSAN_MODEL_COMMON_DATATYPESMODEL_HPP
