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
 *  \file        lusan/model/log/OfflineScopesModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Offline Log Scopes model.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/

#include "lusan/model/log/OfflineScopesModel.hpp"
#include "lusan/model/log/LogIconFactory.hpp"
#include "lusan/model/log/OfflineLogsModel.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "areg/base/NECommon.hpp"
#include "aregextend/db/LogSqliteDatabase.hpp"
#include "aregextend/db/SqliteStatement.hpp"

#include <QIcon>

namespace {

constexpr std::string_view _sqlCreateTempScopes
    {
        "CREATE TEMP TABLE filter_rules ("
        "   scope_id      INTEGER NOT NULL DEFAULT 0,"
        "   target_id     INTEGER NOT NULL DEFAULT 0,"
        "   log_mask      INTEGER NOT NULL DEFAULT 1008"
        "); "
    };

constexpr std::string_view _sqlInitTempScopes
    {
        "INSERT INTO filter_rules(scope_id, target_id, log_mask) SELECT scope_id, cookie_id, 1008 FROM scopes;"
    };

constexpr std::string_view _sqlCreateTempFilter
    {
        "CREATE TEMP TABLE filter_masks ("
        "   scope_id      INTEGER NOT NULL DEFAULT 0,"
        "   log_mask      INTEGER NOT NULL DEFAULT 1008"
        ");"
    };


constexpr std::string_view _sqlInsertTempFilter
    {
        "INSERT INTO filter_masks(scope_id, log_mask) VALUES(?, ?);"
    };

constexpr std::string_view _sqlFilterScopeLogsCountAll
    {
        "SELECT COUNT(l.id)"
        "   FROM logs AS l"
        "   JOIN filter_rules AS r"
        "       ON l.scope_id = r.scope_id AND l.cookie_id = r.target_id"
        "   WHERE ((l.msg_prio & r.log_mask) != 0)"
    };

constexpr std::string_view _sqlFilterScopeLogsCount
    {
        "SELECT COUNT(l.id)"
        "   FROM logs AS l"
        "   JOIN filter_rules AS r"
        "       ON l.scope_id = r.scope_id AND l.cookie_id = r.target_id"
        "   WHERE (l.cookie_id = ?) AND ((l.msg_prio & r.log_mask) != 0)"
    };

constexpr std::string_view _sqlFilterScopeLogsAll
    {
        "SELECT l.msg_type, l.msg_prio, l.cookie_id, l.msg_module_id, l.msg_thread_id, l.time_created, l.time_received, l.time_duration, l.scope_id, l.session_id, l.msg_log, l.msg_thread, l.msg_module"
        "   FROM logs AS l"
        "   JOIN filter_rules AS r"
        "       ON l.scope_id = r.scope_id AND l.cookie_id = r.target_id"
        "   WHERE ((l.msg_prio & r.log_mask) != 0)"
        "   ORDER BY time_created;"
    };

constexpr std::string_view _sqlFilterScopeLogsInst
    {
        "SELECT l.msg_type, l.msg_prio, l.cookie_id, l.msg_module_id, l.msg_thread_id, l.time_created, l.time_received, l.time_duration, l.scope_id, l.session_id, l.msg_log, l.msg_thread, l.msg_module"
        "   FROM logs AS l"
        "   JOIN filter_rules AS r"
        "       ON l.scope_id = r.scope_id AND l.cookie_id = r.target_id"
        "   WHERE (l.cookie_id = ?) AND ((l.msg_prio & r.log_mask) != 0)"
        "   ORDER BY time_created;"
    };

constexpr std::string_view _sqlUpdateFilterScopes
    {
        "UPDATE filter_rules SET log_mask = ("
        "       SELECT m.log_mask"
        "       FROM filter_masks m"
        "   WHERE m.scope_id = filter_rules.scope_id)"
        "   WHERE target_id = ?"
        "   AND scope_id IN (SELECT scope_id FROM filter_masks);"
    };

constexpr std::string_view _sqlResetFilterScopes
    {
        "UPDATE filter_rules SET log_mask = 1008 WHERE target_id = ?;"
    };

constexpr std::string_view _sqlResetFilterScopesAll
    {
        "UPDATE filter_rules SET log_mask = 1008;"
    };

constexpr std::string_view _sqlDisableFilterScopes
    {
        "UPDATE filter_rules SET log_mask = 1008 WHERE target_id = ?;"
    };

constexpr std::string_view _sqlDisableFilterScopesAll
    {
        "UPDATE filter_rules SET log_mask = 1008;"
    };

constexpr std::string_view _sqlDropTable
    {
        "DROP TABLE IF EXISTS %s;"
    };

}

OfflineScopesModel::OfflineScopesModel(QObject* parent)
    : LoggingScopesModelBase(parent)
    , mMapScopeFilter       ( )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

OfflineScopesModel::~OfflineScopesModel(void)
{
}

bool OfflineScopesModel::setLogPriority(const QModelIndex& index, uint32_t prio)
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if ((root == nullptr) || (mLoggingModel == nullptr) || (mLoggingModel->isOperable() == false))
        return false;

    ITEM_ID instId{ static_cast<ScopeRoot*>(root)->getRootId() };
    auto pos = mMapScopeFilter.addIfUnique(instId, ScopeFilters{}, false).first;
    ASSERT(mMapScopeFilter.isValidPosition(pos));
    ScopeFilters& filterPrio = mMapScopeFilter.valueAtPosition(pos);
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;

    uint32_t scopePrio = _logFilterPrio(prio);
    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->getScopeId();
        if (scopeId == NELogging::LOG_SCOPE_ID_NONE)
            continue;

        filterPrio[scopeId] = scopePrio;
        filter.add({ scopeId, scopePrio });
    }

    mLoggingModel->applyFilters(instId, filter);
    // _applyFilters(instId, filter);
    mLoggingModel->filterLogsAsynchronous();

    return true;
}

bool OfflineScopesModel::addLogPriority(const QModelIndex& index, uint32_t prio)
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if ((root == nullptr) || (mLoggingModel == nullptr) || (mLoggingModel->isOperable() == false))
        return false;
    
    ITEM_ID instId{ static_cast<ScopeRoot*>(root)->getRootId() };
    auto pos = mMapScopeFilter.addIfUnique(instId, ScopeFilters{}, false).first;
    ASSERT(mMapScopeFilter.isValidPosition(pos));
    ScopeFilters& filterPrio = mMapScopeFilter.valueAtPosition(pos);
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;

    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->getPriority();
        uint32_t scopePrio = filterPrio.contains(scopeId) ? filterPrio[scopeId] | prio : static_cast<uint32_t>(NELogging::eLogPriority::PrioScopeLogs);
        filterPrio[scopeId] = scopePrio;
        filter.add({ scopeId, scopePrio });
    }
    
    mLoggingModel->applyFilters(instId, filter);
    mLoggingModel->filterLogsAsynchronous();
    
    return true;
}

bool OfflineScopesModel::removeLogPriority(const QModelIndex& index, uint32_t prio)
{
    ScopeNodeBase* node = index.isValid() ? static_cast<ScopeNodeBase*>(index.internalPointer()) : nullptr;
    ScopeNodeBase* root = node != nullptr ? node->getTreeRoot() : nullptr;
    if ((root == nullptr) || (mLoggingModel == nullptr) || (mLoggingModel->isOperable() == false))
        return false;
    
    ITEM_ID instId{ static_cast<ScopeRoot*>(root)->getRootId() };
    auto pos = mMapScopeFilter.addIfUnique(instId, ScopeFilters{}, false).first;
    ASSERT(mMapScopeFilter.isValidPosition(pos));
    ScopeFilters& filterPrio = mMapScopeFilter.valueAtPosition(pos);
    ListScopeFilter filter;
    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else if (node->extractNodeLeafs(leafs) == 0u)
        return false;
    
    uint32_t prioRemove = ~prio;
    for (const auto& leaf : leafs)
    {
        uint32_t scopeId = static_cast<ScopeLeaf*>(leaf)->getPriority();
        uint32_t scopePrio = filterPrio.contains(scopeId) ? filterPrio[scopeId] & prioRemove : static_cast<uint32_t>(NELogging::eLogPriority::PrioScopeLogs) & prioRemove;
        filterPrio[scopeId] = scopePrio;
        filter.add({ scopeId, scopePrio });
    }
    
    mLoggingModel->applyFilters(instId, filter);
    // _applyFilters(instId, filter);
    mLoggingModel->filterLogsAsynchronous();
    
    return true;
}

bool OfflineScopesModel::saveLogScopePriority(const QModelIndex& target) const
{
    return false;
}

void OfflineScopesModel::setLoggingModel(LoggingModelBase* model)
{
    LoggingScopesModelBase::setLoggingModel(model);
    if ((model != nullptr) && model->isOperable())
    {
        if (model->rootCount() == 0)
        {
            const std::vector<NEService::sServiceConnectedInstance>& instances = model->getLogInstances();
            slotInstancesAvailable(instances);
            for (const auto& inst : instances)
            {
                const std::vector<NELogging::sScopeInfo>& scopes = model->getLogInstScopes(inst.ciCookie);
                slotScopesAvailable(inst.ciCookie, scopes);
            }
        }
        else
        {
            beginResetModel();
            endResetModel();
        }
    }
    else
    {
        beginResetModel();
        endResetModel();
    }
}

//////////////////////////////////////////////////////////////////////////
// Private helper methods
//////////////////////////////////////////////////////////////////////////

void OfflineScopesModel::_buildScopeTree(void)
{
    if (mLoggingModel == nullptr)
        return;

    beginResetModel();
    const std::vector<NEService::sServiceConnectedInstance>& instances = mLoggingModel->getLogInstances();
    slotInstancesAvailable(instances);

    for (const auto& inst : instances)
    {
        const std::vector<NELogging::sScopeInfo> & scopes = mLoggingModel->getLogInstScopes(inst.ciCookie);
        slotScopesAvailable(inst.ciCookie, scopes);
    }

    endResetModel();
}

uint32_t OfflineScopesModel::_logFilterPrio(uint32_t prio) const
{
    std::function funcPrio = [](NELogging::eLogPriority prio) -> uint32_t {
            uint32_t result = 0u;
            switch (prio)
            {
            case NELogging::eLogPriority::PrioDebug:
                result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug); // fall through
            case NELogging::eLogPriority::PrioInfo:
                result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo); // fall through
            case NELogging::eLogPriority::PrioWarning:
                result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning); // fall through
            case NELogging::eLogPriority::PrioError:
                result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioError); // fall through
            case NELogging::eLogPriority::PrioFatal:
                result |= static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal); // fall through
                break;

            default:
                break;
            }

            return result;
    };

    uint32_t result{ (prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioScope)) != 0 ? static_cast<uint32_t>(NELogging::eLogPriority::PrioScope) : 0u };
    if ((prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioDebug)) != 0)
    {
        result |= funcPrio(NELogging::eLogPriority::PrioDebug);
    }
    else if ((prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioInfo)) != 0)
    {
        result |= funcPrio(NELogging::eLogPriority::PrioInfo);
    }
    else if ((prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioWarning)) != 0)
    {
        result |= funcPrio(NELogging::eLogPriority::PrioWarning);
    }
    else if ((prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioError)) != 0)
    {
        result |= funcPrio(NELogging::eLogPriority::PrioError);
    }
    else if ((prio & static_cast<uint32_t>(NELogging::eLogPriority::PrioFatal)) != 0)
    {
        result |= funcPrio(NELogging::eLogPriority::PrioFatal);
    }

    return result;
}

void OfflineScopesModel::_applyFilters(uint32_t instId, const TEArrayList<LogSqliteDatabase::sScopeFilter>& filter)
{
    if (mLoggingModel == nullptr)
        return;

    LogSqliteDatabase& db = mLoggingModel->getDatabase();
    if (db.isOperable() == false)
        return;
    if (db.tableExists("sqlite_master", "scopes") == false)
        return;

    if (db.tableExists("sqlite_temp_master", "filter_rules") == false)
    {
        SqliteStatement stmtTemp(db.getDatabase(), _sqlCreateTempScopes);
        if (stmtTemp.execute() == false)
        {
            db.commit(false);
            return;
        }
        
        stmtTemp.reset();
        if (stmtTemp.prepare(_sqlInitTempScopes) == false)
            return;
        
        stmtTemp.execute();
        db.commit(true);
    }

    if (filter.isEmpty())
    {
        SqliteStatement stmt(db.getDatabase());
        if (instId == NEService::TARGET_ALL)
        {
            if (stmt.prepare(_sqlResetFilterScopesAll) == false)
                return;
        }
        else
        {
            if (stmt.prepare(_sqlResetFilterScopes) == false)
                return;

            stmt.bindUint64(0, instId);
        }

        stmt.execute();
    }
    else
    {
        do
        {
            SqliteStatement stmt(db.getDatabase(), _sqlCreateTempFilter);
            if (stmt.execute() == false)
                return;
        } while (false);

        do
        {
            SqliteStatement stmt(db.getDatabase(), _sqlInsertTempFilter);
            for (const auto& scope : filter.getData())
            {
                stmt.bindUint32(0, scope.scopeId);
                stmt.bindUint32(1, scope.scopePrio);
                if (stmt.execute() == false)
                    return;

                stmt.reset();
                stmt.clearBindings();
            }
        } while (false);

        do
        {
            SqliteStatement stmt(db.getDatabase(), _sqlUpdateFilterScopes);
            stmt.bindInt64(0, instId);
            if (stmt.execute() == false)
                return;
        } while (false);

        _dropTable("filter_masks");
    }

    uint32_t count = _countFilteredLogs(instId);
    if (count != 0)
    {
        SqliteStatement stmt(db.getDatabase());
        if (instId == NEService::TARGET_ALL)
        {
            stmt.prepare(_sqlFilterScopeLogsAll);
        }
        else if (stmt.prepare(_sqlFilterScopeLogsInst))
        {
            stmt.bindInt64(0, instId);
        }
    }
}

bool OfflineScopesModel::_tableExists(const char* master, const char* table)
{
    LogSqliteDatabase& db = mLoggingModel->getDatabase();
    return db.tableExists(master, table);
}

void OfflineScopesModel::_dropTable(const char* table)
{
    LogSqliteDatabase& db = mLoggingModel->getDatabase();
    if (NEString::isEmpty<char>(table))
        return;
    else if (db.isOperable() == false)
        return;

    String sql;
    sql.format(_sqlDropTable.data(), table);
    SqliteStatement stmt(db.getDatabase(), sql);
    stmt.execute();
    stmt.finalize();
}

uint32_t OfflineScopesModel::_countFilteredLogs(ITEM_ID instId)
{
    LogSqliteDatabase& db = mLoggingModel->getDatabase();
    if (db.isOperable() == false)
        return 0;

    SqliteStatement stmt(db.getDatabase());
    if (instId == NEService::TARGET_ALL)
    {
        stmt.prepare(_sqlFilterScopeLogsCountAll);
    }
    else if (stmt.prepare(_sqlFilterScopeLogsCount))
    {
        stmt.bindInt64(0, instId);
    }

    return (stmt.next() != SqliteStatement::eQueryResult::Failed ? stmt.getUint32(0) : 0);
}
