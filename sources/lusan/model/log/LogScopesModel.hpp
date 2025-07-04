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
#include "lusan/model/log/LoggingScopesModelBase.hpp"

#include <QList>
#include <QMap>

#include "areg/component/NEService.hpp"
#include "areg/logging/NELogging.hpp"
#include "areglogger/client/LogObserverApi.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class ScopeNodeBase;
class ScopeRoot;
class LogViewerModel;

/**
 * \brief   Log scope model to visualize scopes in the scope navigation windows.
 **/
class LogScopesModel : public LoggingScopesModelBase
{
    Q_OBJECT

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
     * \brief   Initializes the model, sets the signals to receive messages from LogViewerModel.
     * \param   logViewerModel  The LogViewerModel that will forward scope-related signals.
     * \return  True if the model is initialized, false otherwise.
     **/
    bool initialize(LogViewerModel* logViewerModel);

    /**
     * \brief   Releases the model, disconnects the signals.
     **/
    virtual void release(void) override;
    
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

    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the specified node has scope priority, it will not be changed.
     * \param   index   The index of the log scope to change priority.
     * \param   prio    The new priority to set for the log scope on target.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    bool setLogPriority(const QModelIndex& index, NELogging::eLogPriority prio);

    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the log scope already has this priority, it will not be added again.
     * \param   index   The index of the log scope to add priority.
     * \param   prio    The log priority to add to the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    bool addLogPriority(const QModelIndex& index, NELogging::eLogPriority prio);

    /**
     * \brief   Removes the specified log priority from the log scope at the given index.
     *          The request to remove the log priority is sent to the target module.
     *          If the log scope does not have this priority, it will not be removed.
     * \param   index   The index of the log scope to remove priority.
     * \param   prio    The log priority to remove from the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    bool removeLogPriority(const QModelIndex& index, NELogging::eLogPriority prio);

    /**
     * \brief   Saves the log scope priority for the given target index.
     *          If the target index is invalid, it saves the log scope priority for all instances.
     * \param   target  The target index to save log scope priority. If invalid, saves for root index.
     * \return  True if succeeded to save log scope priority, false otherwise.
     **/
    bool saveLogScopePriority(const QModelIndex& target = QModelIndex()) const;
    
signals:

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

    /**
     * \brief   Requests the log priority for the given node.
     * \param   root    The root of the scope.
     * \param   node    The node to request priority.
     * \return  True if succeeded to request the log priority, false otherwise.
     **/
    bool _requestNodePriority(const ScopeRoot& root, const ScopeNodeBase& node);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    LogViewerModel*     mLogViewerModel;// The LogViewerModel that provides scope data
};

#endif  // LUSAN_MODEL_LOG_LOGSCOPESMODEL_HPP
