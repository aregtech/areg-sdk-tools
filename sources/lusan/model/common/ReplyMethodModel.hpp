#ifndef LUSAN_MODEL_COMMON_REPLYMETHODMODEL_HPP
#define LUSAN_MODEL_COMMON_REPLYMETHODMODEL_HPP
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
 *  \file        lusan/model/common/ReplyMethodModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Reply Method Model.
 *
 ************************************************************************/

#include <QAbstractListModel>

class SIMethodBase;
class SIMethodData;
class SIMethodResponse;

/**
 * \class   ReplyMethodModel
 * \brief   Model to manage and display SIMethodResponse objects in a QComboBox.
 **/
class ReplyMethodModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * \brief   Constructor with initialization.
     * \param   parent          The parent object.
     **/
    ReplyMethodModel(SIMethodData & data, QObject* parent = nullptr);

    /**
     * \brief   Adds a SIMethodResponse object to the model.
     * \param   methodResponse  The SIMethodResponse object to add.
     **/
    void addMethodResponse(SIMethodResponse* methodResponse);

    /**
     * \brief   Removes a SIMethodResponse object from the model.
     * \param   methodResponse  The SIMethodResponse object to remove.
     **/
    void removeMethodResponse(SIMethodResponse* methodResponse);

    /**
     * \brief   Sorts the SIMethodResponse objects by name.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortByName(bool ascending);

    /**
     * \brief   Sorts the SIMethodResponse objects by ID.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortById(bool ascending);

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

    void methodCreated(SIMethodBase* method);

    void methodConverted(SIMethodBase* oldMethod, SIMethodBase* newMethod);

    void methodRemoved(SIMethodBase* method);

    void methodUpdated(SIMethodBase* method);

    void updateList(void);

    SIMethodResponse* findResponse(const QString& name) const;

    SIMethodResponse* findResponse(uint32_t id) const;

private:
    SIMethodData &              mData;      //!< Instance of method data object
    QList<SIMethodResponse*>    mMethods;   //!< List of SIMethodResponse objects.
};

#endif  // LUSAN_MODEL_COMMON_REPLYMETHODMODEL_HPP
