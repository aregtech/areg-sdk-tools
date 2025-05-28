#ifndef LUSAN_MODEL_LOG_LOGSCOPESMODEL_HPP
#define LUSAN_MODEL_LOG_LOGSCOPESMODEL_HPP
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
 *  \file        lusan/model/log/LogScopesModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Log scopes model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QStandardItemModel>

#include <QList>
#include <QMap>

#include "areg/component/NEService.hpp"
#include "areglogger/client/LogObserverApi.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class ScopeRoot;

/**
 * \brief   Log scope model to visualize scopes in the scope navigation windows.
 **/
class LogScopesModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    using RootList = QList< ScopeRoot *>;

//////////////////////////////////////////////////////////////////////////
// Constructor, operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Initializes the scope model object.
     * @param   parent  The pointer to the parent object.
     */
    LogScopesModel(QObject * parent = nullptr);

    virtual ~LogScopesModel(void);

    /**
     * \brief   Initializes the model, sets the signals to receive messages.
     * \return  True if the model is initialized, false otherwise.
     **/
    bool initialize(void);

    /**
     * \brief   Releases the model, disconnects the signals.
     **/
    void release(void);
    
    /**
     * \brief   Checks if the given index is valid.
     * \param   index   The index to check.
     * \return  True if the index is valid, false otherwise.
     **/
    inline bool isValidIndex(const QModelIndex& index) const;

    /**
     * \brief   Returns root index.
     **/
    inline const QModelIndex& getRootIndex(void) const;
    
signals:

/************************************************************************
 * Signals
 ************************************************************************/

    /**
     * \brief   Signal emitted when the root of the model is updated.
     * \param   root    The index of the root that is updated.
     **/
    void signalRootUpdated(const QModelIndex& root);

    /**
     * \brief   Signal emitted when the scopes of an instance are inserted.
     * \param   parent  The index of the parent instance item where scopes are inserted.
     **/
    void signalScopesInserted(const QModelIndex& parent);

    /**
     * \brief   Signal emitted when the scopes of an instance are updated.
     * \param   parent  The index of the parent instance item that is updated.
     **/
    void signalScopesUpdated(const QModelIndex& parent);
    
//////////////////////////////////////////////////////////////////////////
// QAbstractItemModel overrides
//////////////////////////////////////////////////////////////////////////
public:
/************************************************************************
 * QAbstractItemModel overrides
 ************************************************************************/

    /**
     * \brief   Returns the index of the item in the model specified by the given row, column and parent index.
     * \param   row     The row of the item.
     * \param   column  The column of the item.
     * \param   parent  The parent index.
     * \return  The index of the item.
     **/
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    /**
     * \brief   Returns the parent index of the given child index.
     * \param   child   The child index.
     * \return  The parent index.
     **/
    virtual QModelIndex parent(const QModelIndex& child) const override;

    /**
     * \brief   Returns the number of rows under the given parent.
     * \param   parent  The parent index.
     * \return  The number of rows.
     **/
    virtual int rowCount(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the number of columns for the children of the given parent.
     * \param   parent  The parent index.
     * \return  The number of columns.
     **/
    virtual int columnCount(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the data stored under the given role for the item referred to by the index.
     * \param   index   The index of the item.
     * \param   role    The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    virtual QVariant data(const QModelIndex& index, int role) const override;

    /**
     * \brief   Returns the data for the given role and section in the header with the specified orientation.
     * \param   section     The section of the header.
     * \param   orientation The orientation of the header.
     * \param   role        The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * \brief   Returns the flags for the item at the given index.
     * \param   index   The index of the item.
     * \return  The flags of the item.
     **/
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    
private:
    /**
     * \brief   The callback of the event triggered when receive the list of connected instances that make logs.
     * \param   instances   The list of the connected instances.
     **/
    void slotLogInstancesConnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   The callback of the event triggered when receive the list of disconnected instances that make logs.
     * \param   instances   The list of IDs of the disconnected instances.
     * \param   count       The number of entries in the list.
     **/
    void slotLogInstancesDisconnect(const QList< NEService::sServiceConnectedInstance >& instances);

    /**
     * \brief   The callback of the event triggered when connection with the log collector service is lost.
     * \param   instances   The list of disconnected instances.
     **/
    void slotLogServiceDisconnected(const QMap<ITEM_ID, NEService::sServiceConnectedInstance>& instances);

    /**
     * \brief   The callback of the event triggered when receive the list of the scopes registered in an application.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of the scopes registered in the application. Each entry contains the ID of the scope, message priority and the full name.
     **/
    void slotLogRegisterScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes);

    /**
     * \brief   The callback of the event triggered when receive the list of previously registered scopes with new priorities.
     * \param   cookie  The cookie ID of the connected instance / application. Same as sLogInstance::liCookie
     * \param   scopes  The list of previously registered scopes. Each entry contains the ID of the scope, message priority and the full name.
     * \param   count   The number of scope entries in the list.
     **/
    void slotLogUpdateScopes(ITEM_ID cookie, const QList<sLogScope*>& scopes);

private:

    /**
     * \brief   Clears the model and deletes all nodes.
     **/
    inline void _clear(void);

    /**
     * \brief   Checks if the root with the given ID exists in the model.
     * \param   rootId  The ID of the root to check.
     * \return  True if the root exists, false otherwise.
     **/
    inline bool _exists(ITEM_ID rootId) const;

    /**
     * \brief   Appends the root to the model.
     * \param   root    The root to append.
     * \param   unique  If true, checks if the root is unique before appending.
     * \return  True if the root is appended, false otherwise.
     **/
    inline bool _appendRoot(ScopeRoot* root, bool unique = true);

    /**
     * \brief   Finds the root with the given ID in the model.
     * \param   rootId  The ID of the root to find.
     * \return  The position of the root in the list, or NECommon::INVALID_INDEX if not found.
     **/
    inline int _findRoot(ITEM_ID rootId) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    RootList        mRootList;      // The list of root nodes
    QModelIndex     mRootIndex;     // The root index of the model
    QStandardItem*  mRootNode;      // The root item.
};

//////////////////////////////////////////////////////////////////////////
// LogScopesModel class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LogScopesModel::isValidIndex(const QModelIndex& index) const
{
    return (index.isValid() && (index.row() >= 0) && (index.column() == 0) && (index.model() == this));
}

inline const QModelIndex& LogScopesModel::getRootIndex(void) const
{
    return mRootIndex;
}


#endif  // LUSAN_MODEL_LOG_LOGSCOPESMODEL_HPP
