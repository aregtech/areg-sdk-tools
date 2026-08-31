#ifndef LUSAN_MODEL_LOG_LIVESCOPESMODEL_HPP
#define LUSAN_MODEL_LOG_LIVESCOPESMODEL_HPP
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
 *  \file        lusan/model/log/LiveScopesModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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

#include "areg/component/ServiceDefs.hpp"
#include "areg/logging/areg_log.h"
#include "areglogger/client/LogObserverApi.h"

#include <QHash>

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

    virtual ~LiveScopesModel();

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
    void setLoggingModel(LoggingModelBase* model) override;
        
    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the specified node has scope priority, it will not be changed.
     * \param   index   The index of the log scope to change priority.
     * \param   prio    The new priority to set for the log scope on target.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    bool setLogPriority(const QModelIndex& index, uint32_t prio) override;

    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the log scope already has this priority, it will not be added again.
     * \param   index   The index of the log scope to add priority.
     * \param   prio    The log priority to add to the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    bool addLogPriority(const QModelIndex& index, uint32_t prio) override;

    /**
     * \brief   Removes the specified log priority from the log scope at the given index.
     *          The request to remove the log priority is sent to the target module.
     *          If the log scope does not have this priority, it will not be removed.
     * \param   index   The index of the log scope to remove priority.
     * \param   prio    The log priority to remove from the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    bool removLogPriority(const QModelIndex& index, uint32_t prio) override;

    /**
     * \brief   Saves the log scope priority for the given target index.
     *          If the target index is invalid, it saves the log scope priority for all instances.
     * \param   target  The target index to save log scope priority. If invalid, saves for root index.
     * \return  True if succeeded to save log scope priority, false otherwise.
     **/
    bool saveLogScopePriority(const QModelIndex& target = QModelIndex()) override;

    /**
     * \brief   Asks the target of the given tree entry to take a state. Active produces the logs
     *          and sends them, Paused produces them and drops them, Stopped produces none because
     *          every scope priority is turned off. Leaving Stopped puts the priorities back.
     * \param   node    Any entry of the target. An invalid index reaches every target.
     * \param   state   The state the target should take.
     * \return  True if the request was sent.
     **/
    bool setSourceState(const QModelIndex& node, areg::LogSourceState state) override;

    /**
     * \brief   Asks the target of the given tree entry to apply the scope priorities it has
     *          saved. A target that was never configured applies its built-in defaults.
     * \param   node    Any entry of the target. An invalid index reaches every target.
     * \return  True if the request was sent.
     **/
    bool restoreConfiguration(const QModelIndex& node) override;

    /**
     * \brief   Returns true. The model follows running targets, so a process that goes is marked.
     **/
    bool isLiveSession() const override;

protected:
    
    /**
     * \brief   Signal emitted when one or more instances are available.
     *          This can be either instances connected to the log collector service in live mode
     *          of instances read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instances   The list of instances available.
     * \return  Returns true if the instance was added to the root element.
     **/
    bool slotInstancesAvailable(const std::vector<areg::ConnectedInstance> & instances) override;

    /**
     * \brief   Applies the priorities a revived root remembered and sends them to the target,
     *          so a process that comes back generates what it generated before it went.
     * \param   root    The root whose scopes have just been rebuilt.
     * \return  The number of scopes the priority was applied to.
     **/
    int applyRememberedPriorities(ScopeRoot & root) override;

private:

    /**
     * \brief   Marks every process as waiting for the answer of a state request.
     * \param   wanted  The state every target was asked to take.
     **/
    void _markAllSourceRequests(areg::LogSourceState wanted);

    /**
     * \brief   Connects to or disconnects from the log observer notifications this model reads.
     * \param   doSetup  True to connect, false to disconnect.
     **/
    void _setupObserverSignals(bool doSetup);

    /**
     * \brief   Holds what the collector said about the sending state of a target and draws it.
     * \param   cookie      The ID of the target.
     * \param   state       The state of the target, as `areg::LogSourceState` holds it.
     * \param   byObserver  The ID of the observer that asked for it, zero when the collector did.
     **/
    void _onSourceState(ITEM_ID cookie, uint8_t state, ITEM_ID byObserver);

    /**
     * \brief   Requests the log priority for the given node.
     * \param   root    The root of the scope.
     * \param   node    The node to request priority.
     * \return  True if succeeded to request the log priority, false otherwise.
     **/
    bool _requestNodePriority(const ScopeRoot& root, const ScopeNodeBase& node);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    //!< The state of every target the collector named, with the observer that asked for it.
    QHash<ITEM_ID, QPair<areg::LogSourceState, ITEM_ID> > mPausedSources;
    //!< The connection to the sending state notification.
    QMetaObject::Connection mConSourceState;
    //!< The connection to the configuration restored notification.
    QMetaObject::Connection mConConfigRestored;
};

#endif  // LUSAN_MODEL_LOG_LIVESCOPESMODEL_HPP
