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

/************************************************************************
 * Includes
 ************************************************************************/
#include <QAbstractListModel>

/************************************************************************
 * Dependencies
 ************************************************************************/
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

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   data            The instance of SIMethodData.
     * \param   parent          The parent object.
     **/
    ReplyMethodModel(SIMethodData & data, QObject* parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~ReplyMethodModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
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

    /**
     * \brief   Called when a new method is created. This updates the model.
     * \param   method  The new created method object.
     **/
    void methodCreated(SIMethodBase* method);

    /**
     * \brief   Called when a method is converted. This call replaces oldMethod
     *          by newMethod in the model and triggers update event.
     * \param   oldMethod   The old method object.
     * \param   newMethod   The new method object.
     **/
    void methodConverted(SIMethodBase* oldMethod, SIMethodBase* newMethod);

    /**
     * \brief   Called when a method is removed. This removes the method from the model.
     * \param   method  The method object to remove.
     **/
    void methodRemoved(SIMethodBase* method);

    /**
     * \brief   Called when a method is updated. This triggers the update event.
     * \param   method  The updated method object.
     **/
    void methodUpdated(SIMethodBase* method);

    /**
     * \brief   Updates the list of SIMethodResponse objects.
     *          The data is taken from the SIMethodData object.
     **/
    void updateList(void);

    /**
     * \brief   Finds the SIMethodResponse object by name.
     * \param   name    The name of the SIMethodResponse object to find.
     * \return  Returns the found SIMethodResponse object if exists. Otherwise, returns nullptr.
     **/
    SIMethodResponse* findResponse(const QString& name) const;

    /**
     * \brief   Finds the SIMethodResponse object by ID.
     * \param   id  The ID of the SIMethodResponse object to find.
     * \return  Returns the found SIMethodResponse object if exists. Otherwise, returns nullptr.
     **/
    SIMethodResponse* findResponse(uint32_t id) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    SIMethodData &              mData;      //!< Instance of method data object
    QList<SIMethodResponse*>    mMethods;   //!< List of SIMethodResponse objects.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    ReplyMethodModel(void) = delete;
    ReplyMethodModel(const ReplyMethodModel& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// ReplyMethodModel class inline methods
//////////////////////////////////////////////////////////////////////////

#endif  // LUSAN_MODEL_COMMON_REPLYMETHODMODEL_HPP
