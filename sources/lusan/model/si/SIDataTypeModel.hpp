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

#include <QAbstractItemModel>
#include "lusan/data/common/DataTypeBase.hpp"

class SIDataTypeData;
class DataTypeCustom;

/**
 * \class SIDataTypeModel
 * \brief Model to manage custom data types for the table view.
 */
class SIDataTypeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /**
     * \brief Constructor with initialization.
     * \param parent The parent object.
     */
    SIDataTypeModel(SIDataTypeData & data, QObject* parent = nullptr);

    /**
     * \brief Returns the number of rows in the model.
     * \param parent The parent index.
     * \return The number of rows.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * \brief Returns the number of columns in the model.
     * \param parent The parent index.
     * \return The number of columns.
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * \brief Returns the data for the given role and section in the model.
     * \param index The index of the item.
     * \param role The role for which data is requested.
     * \return The data for the given role and section.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * \brief Returns the header data for the given role and section in the model.
     * \param section The section of the header.
     * \param orientation The orientation of the header.
     * \param role The role for which data is requested.
     * \return The header data for the given role and section.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * \brief Returns the index of the item in the model specified by the given row, column, and parent index.
     * \param row The row of the item.
     * \param column The column of the item.
     * \param parent The parent index.
     * \return The index of the item.
     */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    /**
     * \brief Returns the parent index of the given index.
     * \param index The index of the item.
     * \return The parent index of the item.
     */
    QModelIndex parent(const QModelIndex& index) const override;

    DataTypeCustom * addDataType(const QString & name, DataTypeBase::eCategory category);

    DataTypeCustom* convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category);

private:
    SIDataTypeData& mData; //!< The data object.
};

#endif // LUSAN_MODEL_SI_SIDATATYPEMODEL_HPP
