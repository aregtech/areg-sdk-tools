#ifndef LUSAN_MODEL_LOG_LIVESCOPESMODEL_HPP
#define LUSAN_MODEL_LOG_LIVESCOPESMODEL_HPP
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
 *  \file        lusan/model/log/LiveScopesModel.hpp
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
class LiveLogsModel;

/**
 * \brief   Log scope model to visualize scopes in the scope navigation windows.
 **/
class LiveScopesModel : public LoggingScopesModelBase
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
    LiveScopesModel(QObject * parent = nullptr);

    virtual ~LiveScopesModel(void);

public:
/************************************************************************
 * LoggingScopesModelBase overrides
 ************************************************************************/

    /**
     * \brief   Sets the logging model object used to retrieve logging scopes data.
     *          If valid pointer, sets up the signals and makes initialization.
     *          If `nullptr`, resets the model and clears the data.
     * \param   model   The logging model to set.
     **/
    virtual void setLoggingModel(LoggingModelBase* model) override;
        
    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the specified node has scope priority, it will not be changed.
     * \param   index   The index of the log scope to change priority.
     * \param   prio    The new priority to set for the log scope on target.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    virtual bool setLogPriority(const QModelIndex& index, uint32_t prio) override;

    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the log scope already has this priority, it will not be added again.
     * \param   index   The index of the log scope to add priority.
     * \param   prio    The log priority to add to the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    virtual bool addLogPriority(const QModelIndex& index, uint32_t prio) override;

    /**
     * \brief   Removes the specified log priority from the log scope at the given index.
     *          The request to remove the log priority is sent to the target module.
     *          If the log scope does not have this priority, it will not be removed.
     * \param   index   The index of the log scope to remove priority.
     * \param   prio    The log priority to remove from the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    virtual bool removeLogPriority(const QModelIndex& index, uint32_t prio) override;

    /**
     * \brief   Saves the log scope priority for the given target index.
     *          If the target index is invalid, it saves the log scope priority for all instances.
     * \param   target  The target index to save log scope priority. If invalid, saves for root index.
     * \return  True if succeeded to save log scope priority, false otherwise.
     **/
    virtual bool saveLogScopePriority(const QModelIndex& target = QModelIndex()) const override;
    
protected:
    
    /**
     * \brief   Signal emitted when one or more instances are available.
     *          This can be either instances connected to the log collector service in live mode
     *          of instances read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instances   The list of instances available.
     * \return  Returns true if the instance was added to the root element.
     **/
    virtual bool slotInstancesAvailable(const std::vector<NEService::sServiceConnectedInstance> & instances) override;
    
private:

    /**
     * \brief   Requests the log priority for the given node.
     * \param   root    The root of the scope.
     * \param   node    The node to request priority.
     * \return  True if succeeded to request the log priority, false otherwise.
     **/
    bool _requestNodePriority(const ScopeRoot& root, const ScopeNodeBase& node);
};

#endif  // LUSAN_MODEL_LOG_LIVESCOPESMODEL_HPP
