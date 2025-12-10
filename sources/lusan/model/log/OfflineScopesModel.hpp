#ifndef LUSAN_MODEL_LOG_OFFLINESCOPESMODEL_HPP
#define LUSAN_MODEL_LOG_OFFLINESCOPESMODEL_HPP
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
 *  \file        lusan/model/log/OfflineScopesModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Scopes model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LoggingScopesModelBase.hpp"
#include <QList>
#include <QMap>

#include "areg/base/GEGlobal.h"
#include "areg/base/TEArrayList.hpp"
#include "areg/base/TEHashMap.hpp"
#include "areg/base/TEMap.hpp"
#include "aregextend/db/LogSqliteDatabase.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class ScopeNodeBase;
class ScopeRoot;
class OfflineLogsModel;

/**
 * \brief   Offline log scope model to visualize scopes in the scope navigation windows for offline mode.
 *          This model reads scope information from OfflineLogsModel and builds a tree structure
 *          using ScopeNodes for navigation in offline mode.
 **/
class OfflineScopesModel : public LoggingScopesModelBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
    using sScopeFilter      = LogSqliteDatabase::sScopeFilter;
    using ListScopeFilter   = TEArrayList<sScopeFilter>;
    using ScopeFilters      = TEHashMap<uint32_t, uint32_t>;
    using MapScopeFilter    = TEMap<ITEM_ID, ScopeFilters>;

    static constexpr uint32_t INIT_LOG_PRIO { static_cast<uint32_t>(NELogging::eLogPriority::PrioScopeLogs) };

    static constexpr uint32_t LOG_PRIO_MASK { INIT_LOG_PRIO };

//////////////////////////////////////////////////////////////////////////
// Constructor, operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Initializes the offline scope model object.
     * @param   parent  The pointer to the parent object.
     */
    OfflineScopesModel(QObject* parent = nullptr);

    virtual ~OfflineScopesModel(void);

//////////////////////////////////////////////////////////////////////////
// LoggingScopesModelBase overrides
//////////////////////////////////////////////////////////////////////////
public:
/************************************************************************
 * LoggingScopesModelBase overrides
 ************************************************************************/

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

    /**
     * \brief   Sets the logging model object used to retrieve logging scopes data.
     * \param   model   The logging model to set. This can be `nullptr` to reset the model.
     *                  If can be either live logging model or offline logging model.
     **/
    virtual void setLoggingModel(LoggingModelBase* model) override;

//////////////////////////////////////////////////////////////////////////
// Internal overrides
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Build scopes from a single scope path
     **/
    virtual void buildScope(ScopeRoot& root, QString& scopePath, uint32_t scopePrio, uint32_t scopeId) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Builds the scope tree from offline model data.
     **/
    void _buildScopeTree(void);

    //!< Returns the combined allowed log priority for the given priority.
    uint32_t _logFilterPrio(uint32_t prio) const;
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MapScopeFilter  mMapScopeFilter;    //<!< The map of scope filters, where key is the instance ID and values is the pair of scope ID and message prio.
};

//////////////////////////////////////////////////////////////////////////
// OfflineScopesModel class inline methods
//////////////////////////////////////////////////////////////////////////

#endif  // LUSAN_MODEL_LOG_OFFLINESCOPESMODEL_HPP
