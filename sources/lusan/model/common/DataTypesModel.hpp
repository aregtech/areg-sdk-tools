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
    DataTypesModel(SIDataTypeData& dataTypeData, QObject* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The instance of SIDataTypeData.
     * \param   excludes        The list of data types to exclude.
     * \param   parent          The parent object.
     **/
    DataTypesModel(SIDataTypeData& dataTypeData, const QStringList &excludes, QObject* parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   dataTypeData    The instance of SIDataTypeData.
     * \param   excludes        The list of data types to exclude.
     * \param   parent          The parent object.
     **/
    DataTypesModel(SIDataTypeData& dataTypeData, const QList<DataTypeBase*> &excludes, QObject* parent = nullptr);

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

    bool dataTypeCreated(DataTypeCustom* dataType);

    bool dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    bool dataTypeRemoved(DataTypeCustom* dataType);

    bool dataTypeUpdated(DataTypeCustom* dataType);

    void updateDataTypeLists(void);

    bool removeDataType(DataTypeCustom* dataType);

    bool removeField(DataTypeCustom* dataType, uint32_t fieldId);

    bool addDataType(DataTypeCustom* dataType);

private:

    inline void _sort(bool sortPredefined = true);

private:
    SIDataTypeData&         mDataTypeData;  //!< Reference to the SIDataTypeData instance.
    QList<DataTypeBase*>    mExcludeList;   //!< Filtered list of data types.
    QList<DataTypeBase*>    mDataTypeList;  //!< The list of all data types.
    int                     mCountPredef;   //!< The number of predefined entries, which are set at the beginning of mDataTypeList;
};

#endif  // LUSAN_MODEL_COMMON_DATATYPESMODEL_HPP
