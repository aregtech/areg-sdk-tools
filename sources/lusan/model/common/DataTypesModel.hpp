#ifndef LUSAN_MODEL_COMMON_DATATYPESMODEL_HPP
#define LUSAN_MODEL_COMMON_DATATYPESMODEL_HPP
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
 *  \file        lusan/model/common/DataTypesModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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
class DataTypeDataSection;

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
     * \param   dataTypeData    The document's data types section.
     * \param   parent          The parent object.
     **/
    DataTypesModel(DataTypeDataSection& dataTypeData, bool hasEmpty, QObject* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The document's data types section.
     * \param   excludes        The list of data types to exclude.
     * \param   parent          The parent object.
     **/
    DataTypesModel(DataTypeDataSection& dataTypeData, const QStringList &excludes, bool hasEmpty, QObject* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The document's data types section.
     * \param   excludes        The list of data types to exclude.
     * \param   parent          The parent object.
     **/
    DataTypesModel(DataTypeDataSection& dataTypeData, const QList<DataTypeBase*> &excludes, bool hasEmpty, QObject* parent = nullptr);

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
     * \brief   Rebuilds the list of data types from the document, honoring the filter. Called
     *          whenever the document's data types changed: the model keeps no deltas, because a
     *          pointer-level delta cannot survive an undo that puts a different object back.
     **/
    void updateDataTypeLists();

    /**
     * \brief   Returns flag, indicating whether the list can have an empty entry.
     **/
    inline bool hasEmptyEntry() const;

    /**
     * \brief   Adds an empty entry to the list. The empty type has no data type.
     **/
    inline void addEmptyEntry();

    /**
     * \brief   Removes the empty entry from the list.
     **/
    inline void removeEmptyEntry();

    /**
     * \brief   Returns the data type object by the given name.
     * \param   name    The name of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeBase* findDataType(const QString& name) const;

    /**
     * \brief   Returns the data type object by the given ID.
     * \param   id  The ID of the data type to search.
     * \return  Returns the data type object if found. Otherwise, returns nullptr.
     **/
    DataTypeBase* findDataType(uint32_t id) const;

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeDataSection&    mDataTypeData;  //!< The document section the list is read from.
    QList<DataTypeBase*>    mExcludeList;   //!< Filtered list of data types.
    QList<DataTypeBase*>    mDataTypeList;  //!< The list of all data types.
    int                     mCountPredef;   //!< The number of predefined entries, which are set at the beginning of mDataTypeList;
    const bool              mHasEmpty;      //!< Flag, indicating whether there can be an empty entry in the list.
    static DataTypeEmpty    _emptyType;     //!< The empty data type object.
};

//////////////////////////////////////////////////////////////////////////
// DataTypesModel class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool DataTypesModel::hasEmptyEntry() const
{
    return mHasEmpty;
}

inline void DataTypesModel::addEmptyEntry()
{
    if (mHasEmpty && (mDataTypeList.indexOf(&_emptyType) < 0))
    {
        ++ mCountPredef;
        beginInsertRows(QModelIndex(), 0, 0);
        mDataTypeList.insert(0, &_emptyType);
        endInsertRows();
    }
}

inline void DataTypesModel::removeEmptyEntry()
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
