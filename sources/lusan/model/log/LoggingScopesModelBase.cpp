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
 *  \file        lusan/model/log/LoggingScopesModelBase.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Base class for log scopes models.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/log/LoggingScopesModelBase.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/model/log/LogIconFactory.hpp"

#include "lusan/data/log/ScopeNodes.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include "areg/base/DateTime.hpp"

#include <QTimer>

LoggingScopesModelBase::LoggingScopesModelBase(QObject* parent)
    : QAbstractItemModel( parent )
    , mRootIndex        ( )
    , mLoggingModel     ( nullptr )
    , mTargetClock      ( nullptr )
    , mTempRaises       ( )
    
    , mSignalsSetup         ( false )
    , mConSvcConnected      ( )
    , mConSvcDisconnected   ( )
    , mConInstAvailable     ( )
    , mConInstUnavailable   ( )
    , mConScopesAvailable   ( )
    , mConScopesUnavailable ( )
{
    mRootIndex = createIndex(0, 0, nullptr);
}

LoggingScopesModelBase::~LoggingScopesModelBase()
{
    _setupSignals(false);
    clearModel(false);
    mLoggingModel = nullptr;
}

namespace
{
    //! The pace the target state is moved on with, in milliseconds.
    constexpr int _targetTickMs { 120 };

    //! Returns the sentence that says what the target knows about its priorities.
    QVariant _targetStateText(ScopeRoot::eTargetState state)
    {
        switch (state)
        {
        case ScopeRoot::eTargetState::TargetPending:
            return QObject::tr("Changed here, not in effect on the target.");

        case ScopeRoot::eTargetState::TargetSent:
            return QObject::tr("Sent to the target, waiting for its answer.");

        case ScopeRoot::eTargetState::TargetSaved:
            return QObject::tr("Saved on the target. It keeps these priorities across a restart.");

        default:
            return QVariant();
        }
    }
}

void LoggingScopesModelBase::nodeExpanded(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeState(true);
        }
        else if (idxNode == mRootIndex)
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeState(true);
            }
        }
    }
}

void LoggingScopesModelBase::nodeCollapsed(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeState(false);
        }
        else if ((idxNode == mRootIndex))
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeState(false);
            }
        }
    }
}

void LoggingScopesModelBase::nodeSelected(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        mLoggingModel->setSelectedScope(idxNode);
    }
}

void LoggingScopesModelBase::nodeTreeExpanded(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeTreeExpanded();
        }
        else if ((idxNode == mRootIndex))
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeTreeExpanded();
            }
        }
    }
}

void LoggingScopesModelBase::nodeTreeCollapsed(const QModelIndex& idxNode)
{
    if (mLoggingModel != nullptr)
    {
        ScopeNodeBase* node = idxNode.isValid() ? static_cast<ScopeNodeBase*>(idxNode.internalPointer()) : nullptr;
        if (node != nullptr)
        {
            node->setNodeTreeCollapsed();
        }
        else if ((idxNode == mRootIndex))
        {
            LoggingModelBase::RootList & roots = mLoggingModel->getRootList();
            for (auto root : roots)
            {
                Q_ASSERT(root != nullptr);
                root->setNodeTreeCollapsed();
            }
        }
    }
}

void LoggingScopesModelBase::setLoggingModel(LoggingModelBase* model)
{
    if (model != nullptr)
    {
        if (mLoggingModel == model)
            return;

        if (mLoggingModel != nullptr)
        {
            // If the model is already set, disconnect from the previous one
            _setupSignals(false);
        }

        mLoggingModel = model;
        _setupSignals(true);        
        slotLogServiceConnected();
    }
    else if (mLoggingModel != nullptr)
    {
        _setupSignals(false);
        mLoggingModel = nullptr;
    }
}

QModelIndex LoggingScopesModelBase::index(int row, int column, const QModelIndex& parent) const
{
    if ((hasIndex(row, column, parent) == false) || (mLoggingModel == nullptr))
        return QModelIndex();

    const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
    ScopeNodeBase* parentNode = parent.isValid() ? static_cast<ScopeNodeBase*>(parent.internalPointer()) : nullptr;
    
    if (parentNode == nullptr)
    {
        ScopeRoot* root = (row >= 0) && (row < static_cast<int>(roots.size())) ? roots[row] : nullptr;
        return (root != nullptr ? createIndex(row, column, root) : mRootIndex);
    }
    else
    {
        ScopeNodeBase* childNode = parentNode->getChildAt(row);
        return (childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex());
    }
}

QModelIndex LoggingScopesModelBase::parent(const QModelIndex& child) const
{
    if ((child.isValid() == false) || (child == mRootIndex) || (mLoggingModel == nullptr))
        return QModelIndex();

    ScopeNodeBase* childNode = static_cast<ScopeNodeBase*>(child.internalPointer());
    Q_ASSERT(childNode != nullptr);
    if (childNode->isRoot())
        return mRootIndex;

    ScopeNodeBase* parentNode = childNode->getParent();
    Q_ASSERT(parentNode != nullptr);
    if (parentNode->isRoot())
    {
        int pos = findRoot(static_cast<ScopeRoot*>(parentNode)->getRootId());
        Q_ASSERT(pos != static_cast<int>(areg::INVALID_INDEX));
        return createIndex(pos, 0, parentNode);
    }
    else
    {
        ScopeNodeBase* grandParent = parentNode->getParent();
        int pos = grandParent->getChildPosition(parentNode->getNodeName());
        Q_ASSERT(pos != static_cast<int>(areg::INVALID_INDEX));
        return createIndex(pos, 0, parentNode);
    }
}

int LoggingScopesModelBase::rowCount(const QModelIndex& parent) const
{
    ScopeNodeBase* node = parent.isValid() ? static_cast<ScopeNodeBase*>(parent.internalPointer()) : nullptr;
    if (node == nullptr)
    {
        // Root level - return number of instances
        return (mLoggingModel != nullptr ? static_cast<int>(mLoggingModel->getRootList().size()) : 0);
    }
    else
    {
        // Child level - get from the node
        return node->getChildCount();
    }
}

int LoggingScopesModelBase::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return LoggingScopesModelBase::ColumnCount;
}

QVariant LoggingScopesModelBase::data(const QModelIndex& index, int role) const
{
    if (isValidIndex(index) == false)
        return QVariant();
    
    if (index == mRootIndex)
    {
        return (static_cast<Qt::ItemDataRole>(role) == Qt::ItemDataRole::DisplayRole ? QVariant(tr("Scopes")) : QVariant());
    }
    
    ScopeNodeBase* entry{ static_cast<ScopeNodeBase*>(index.internalPointer()) };
    if (entry == nullptr)
        return QVariant();

    if (index.column() == LoggingScopesModelBase::ColumnShow)
    {
        if (role == Qt::ItemDataRole::CheckStateRole)
            return QVariant(entry->shownState());
        else if ((role == Qt::ItemDataRole::ToolTipRole) && isGoneTarget(entry))
            return QVariant(tr("The process has stopped, so this has nothing left to act on.\nUse the column filters of the log table to narrow the rows it already produced."));

        return QVariant();
    }

    switch (static_cast<Qt::ItemDataRole>(role))
    {
    case Qt::ItemDataRole::DisplayRole:
    {
        return entry->getDisplayName();
    }
    
    case Qt::ItemDataRole::DecorationRole:
    {
        const ScopeNodeBase::sPrioRollup rollup{ entry->priorityRollup() };
        const LogIconFactory::eScopeLines lines{ rollup.linesAll  ? LogIconFactory::eScopeLines::LinesOn
                                               : rollup.linesSome ? LogIconFactory::eScopeLines::LinesPartial
                                                                  : LogIconFactory::eScopeLines::LinesOff };

        LogIconFactory::sCharger charger{ LogIconFactory::chargerOfRange(rollup.levelLow, rollup.levelHigh, lines) };
        const ScopeNodeBase* treeRoot{ entry->getTreeRoot() };
        charger.frozen = isLiveSession()
                      && (treeRoot != nullptr)
                      && (static_cast<const ScopeRoot *>(treeRoot)->isConnected() == false);

        return LogIconFactory::chargerIcon(charger);
    }
    
    case Qt::ItemDataRole::UserRole:
    {
        return QVariant::fromValue<ScopeNodeBase *>(entry);
    }

    case Qt::ItemDataRole::ToolTipRole:
    {
        return entry->isRoot() ? _targetStateText(static_cast<const ScopeRoot *>(entry)->targetState()) : QVariant();
    }
        
    default:
        break;
    }

    if (entry->isRoot())
    {
        const ScopeRoot* root{ static_cast<const ScopeRoot *>(entry) };
        if (role == LoggingScopesModelBase::RoleTargetState)
            return QVariant(static_cast<int>(root->targetState()));
        else if (role == LoggingScopesModelBase::RoleTargetFade)
            return QVariant(root->targetFade());
    }

    return QVariant();
}

QVariant LoggingScopesModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole) && (section == 0))
    {
        return QString("Scopes");
    }

    return QVariant();
}

Qt::ItemFlags LoggingScopesModelBase::flags(const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return Qt::NoItemFlags;
    }
    else
    {
        ScopeNodeBase* node = static_cast<ScopeNodeBase*>(index.internalPointer());
        Qt::ItemFlags result{ Qt::ItemIsSelectable | Qt::ItemIsEnabled };
        if ((node != nullptr) && node->isLeaf())
        {
            result |= Qt::ItemNeverHasChildren;
        }

        if ((index.column() == LoggingScopesModelBase::ColumnShow) && (isGoneTarget(node) == false))
        {
            result |= Qt::ItemIsUserCheckable;
        }

        return result;
    }
}

void LoggingScopesModelBase::_refuseScopesOf(ScopeNodeBase* node, bool refuse)
{
    if ((node == nullptr) || (mLoggingModel == nullptr))
        return;

    ScopeNodeBase* treeRoot{ node->getTreeRoot() };
    if (treeRoot == nullptr)
        return;

    std::vector<ScopeNodeBase*> leafs;
    if (node->isLeaf())
        leafs.push_back(node);
    else
        node->extractNodeLeafs(leafs);

    QSet<uint32_t> scopeIds;
    for (const ScopeNodeBase* leaf : leafs)
    {
        const uint32_t scopeId{ static_cast<const ScopeLeaf *>(leaf)->getScopeId() };
        if (scopeId != areg::LOG_SCOPE_ID_NONE)
        {
            scopeIds.insert(scopeId);
        }
    }

    // A live session refuses from this moment on. An archive has no moment left to wait for,
    // so the refusal covers the whole file.
    const TIME64 since{ isLiveSession() ? static_cast<TIME64>(areg::DateTime::now()) : 0 };
    mLoggingModel->setScopesRefused(static_cast<ScopeRoot *>(treeRoot)->getRootId(), scopeIds, refuse, since);
}

bool LoggingScopesModelBase::isGoneTarget(const ScopeNodeBase* node) const
{
    if ((node == nullptr) || (isLiveSession() == false))
        return false;

    const ScopeNodeBase* treeRoot{ node->getTreeRoot() };
    return (treeRoot != nullptr) && (static_cast<const ScopeRoot *>(treeRoot)->isConnected() == false);
}

bool LoggingScopesModelBase::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if ( (role != Qt::ItemDataRole::CheckStateRole)
      || (index.column() != LoggingScopesModelBase::ColumnShow)
      || (isValidIndex(index) == false) )
    {
        return false;
    }

    ScopeNodeBase* node = static_cast<ScopeNodeBase*>(index.internalPointer());
    if (node == nullptr)
        return false;

    Q_UNUSED(value);

    // A partially checked node shows everything below it. Only a fully shown node hides.
    const bool hide{ node->shownState() == Qt::CheckState::Checked };
    node->setShownRecursive(hide == false);
    _refuseScopesOf(node, hide);

    const QModelIndex last{ this->index(index.row(), LoggingScopesModelBase::ColumnCount - 1, index.parent()) };
    emit dataChanged(index, last);
    _notifyBranchChanged(index);
    _notifyParentsChanged(index);
    emit signalScopeVisibilityChanged();
    return true;
}

void LoggingScopesModelBase::setScopeShown(const QModelIndex& index, bool shown)
{
    if (isValidIndex(index) == false)
        return;

    ScopeNodeBase* node = static_cast<ScopeNodeBase*>(index.internalPointer());
    if (node == nullptr)
        return;

    node->setShownRecursive(shown);
    _refuseScopesOf(node, shown == false);
    const QModelIndex first{ this->index(index.row(), LoggingScopesModelBase::ColumnShow, index.parent()) };
    const QModelIndex last { this->index(index.row(), LoggingScopesModelBase::ColumnCount - 1, index.parent()) };
    emit dataChanged(first, last);
    _notifyBranchChanged(first);
    _notifyParentsChanged(first);
    emit signalScopeVisibilityChanged();
}

void LoggingScopesModelBase::showScopeAlone(const QModelIndex& index)
{
    if (isValidIndex(index) == false)
        return;

    const int roots{ rowCount(mRootIndex) };
    for (int row = 0; row < roots; ++row)
    {
        const QModelIndex idxRoot{ this->index(row, LoggingScopesModelBase::ColumnShow, mRootIndex) };
        ScopeNodeBase* root = static_cast<ScopeNodeBase*>(idxRoot.internalPointer());
        if (root != nullptr)
        {
            root->setShownRecursive(false);
            _refuseScopesOf(root, true);
        }
    }

    ScopeNodeBase* node = static_cast<ScopeNodeBase*>(index.internalPointer());
    if (node != nullptr)
    {
        node->setShownRecursive(true);
        _refuseScopesOf(node, false);
    }

    _notifyBranchChanged(mRootIndex);
    _notifyParentsChanged(this->index(index.row(), LoggingScopesModelBase::ColumnShow, index.parent()));
    emit dataChanged(this->index(0, LoggingScopesModelBase::ColumnShow, mRootIndex), this->index(roots - 1, LoggingScopesModelBase::ColumnCount - 1, mRootIndex));
    emit signalScopeVisibilityChanged();
}

void LoggingScopesModelBase::showAllScopes(void)
{
    const int roots{ rowCount(mRootIndex) };
    if (roots == 0)
        return;

    for (int row = 0; row < roots; ++row)
    {
        ScopeNodeBase* root = static_cast<ScopeNodeBase*>(this->index(row, LoggingScopesModelBase::ColumnShow, mRootIndex).internalPointer());
        if (root != nullptr)
        {
            root->setShownRecursive(true);
        }
    }

    if (mLoggingModel != nullptr)
    {
        mLoggingModel->clearRefusedScopes();
    }

    _notifyBranchChanged(mRootIndex);
    emit dataChanged(this->index(0, LoggingScopesModelBase::ColumnShow, mRootIndex), this->index(roots - 1, LoggingScopesModelBase::ColumnCount - 1, mRootIndex));
    emit signalScopeVisibilityChanged();
}

bool LoggingScopesModelBase::hasHiddenScopes(void) const
{
    QSet<uint32_t> hidden;
    return (const_cast<LoggingScopesModelBase*>(this)->collectHiddenScopes(hidden) != 0);
}

void LoggingScopesModelBase::_notifyBranchChanged(const QModelIndex& parent)
{
    const int count{ rowCount(parent) };
    if (count == 0)
        return;

    emit dataChanged(index(0, LoggingScopesModelBase::ColumnShow, parent), index(count - 1, LoggingScopesModelBase::ColumnShow, parent));
    for (int row = 0; row < count; ++row)
    {
        _notifyBranchChanged(index(row, LoggingScopesModelBase::ColumnShow, parent));
    }
}

void LoggingScopesModelBase::_notifyParentsChanged(const QModelIndex& child)
{
    QModelIndex node{ child.parent() };
    while (node.isValid() && (node != mRootIndex))
    {
        ScopeNodeBase* entry = static_cast<ScopeNodeBase*>(node.internalPointer());
        if (entry == nullptr)
            break;

        // The state of a parent is kept, not computed on every paint, so it is recomputed here.
        if (entry->isLeaf() == false)
        {
            static_cast<ScopeNode*>(entry)->refreshShownState();
        }

        emit dataChanged(node, node);
        node = node.parent();
    }
}

int LoggingScopesModelBase::collectHiddenScopes(QSet<uint32_t>& scopeIds) const
{
    int result{ 0 };
    if (mLoggingModel != nullptr)
    {
        for (const ScopeRoot* root : mLoggingModel->getRootList())
        {
            Q_ASSERT(root != nullptr);
            result += root->collectHiddenScopes(scopeIds);
        }
    }

    return result;
}

void LoggingScopesModelBase::buildScope(ScopeRoot& root, QString& scopePath, uint32_t scopePrio, uint32_t scopeId)
{
    root.addChildRecursive(scopePath, scopePrio, scopeId);
}

void LoggingScopesModelBase::clearModel(bool notify /*= false*/)
{
    if (notify)
    {
        beginResetModel();
        endResetModel();
    }
}

bool LoggingScopesModelBase::existsRoot(ITEM_ID rootId) const
{
    if (mLoggingModel != nullptr)
    {
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (auto root : roots)
        {
            Q_ASSERT(root != nullptr);
            if (root->getRootId() == rootId)
                return true;
        }
    }

    return false;
}

bool LoggingScopesModelBase::appendRoot(ScopeRoot* root, bool unique /*= true*/)
{
    if ((mLoggingModel != nullptr) && ((unique == false) || (existsRoot(root->getRootId()) == false)))
    {
        mLoggingModel->getRootList().push_back(root);
        return true;
    }

    return false;
}

int LoggingScopesModelBase::findRoot(ITEM_ID rootId) const
{
    if (mLoggingModel != nullptr)
    {
        const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (int i = 0; i < static_cast<int>(roots.size()); ++i)
        {
            if (roots[i]->getRootId() == rootId)
                return i;
        }
    }

    return static_cast<int>(areg::INVALID_INDEX);
}

int LoggingScopesModelBase::findGoneRoot(const areg::ConnectedInstance & instance) const
{
    if (mLoggingModel != nullptr)
    {
        const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (int i = 0; i < static_cast<int>(roots.size()); ++i)
        {
            if ((roots[i]->isConnected() == false) && roots[i]->isSameInstance(instance))
                return i;
        }
    }

    return static_cast<int>(areg::INVALID_INDEX);
}

void LoggingScopesModelBase::reviveRoot(int pos, const areg::ConnectedInstance & instance)
{
    Q_ASSERT(mLoggingModel != nullptr);
    ScopeRoot* root = mLoggingModel->getRootList()[pos];
    Q_ASSERT(root != nullptr);

    root->savePriorities();
    root->removeChildren();
    root->setRootId(instance.ciCookie);
    root->setConnected(true);
}

int LoggingScopesModelBase::applyRememberedPriorities(ScopeRoot & root)
{
    return root.restorePriorities();
}

void LoggingScopesModelBase::scheduleRevert(const QModelIndex& node, uint32_t prio, int afterMs)
{
    const ScopeNodeBase* entry{ node.isValid() ? static_cast<const ScopeNodeBase*>(node.internalPointer()) : nullptr };
    const ScopeNodeBase* treeRoot{ entry != nullptr ? entry->getTreeRoot() : nullptr };
    if ((treeRoot == nullptr) || (isLiveSession() == false))
        return;

    // A process carries an empty path, which names the process itself.
    const ITEM_ID rootId{ static_cast<const ScopeRoot *>(treeRoot)->getRootId() };
    const QString path{ entry->makePath() };

    // A second raise on the same scope replaces the first, so the walk back stays one step.
    for (int pos = 0; pos < static_cast<int>(mTempRaises.size()); ++pos)
    {
        if ((mTempRaises[pos].rootId == rootId) && (mTempRaises[pos].path == path))
        {
            prio = mTempRaises[pos].prio;
            mTempRaises[pos].timer->stop();
            delete mTempRaises[pos].timer;
            mTempRaises.removeAt(pos);
            break;
        }
    }

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(afterMs);
    connect(timer, &QTimer::timeout, this, [this, rootId, path]() { _revertTempRaise(rootId, path); });

    mTempRaises.append(sTempRaise{ rootId, path, prio, timer });
    timer->start();
    emit signalSafeguardsChanged();
}

int LoggingScopesModelBase::keepTempRaises(void)
{
    const int count{ static_cast<int>(mTempRaises.size()) };
    for (sTempRaise& raise : mTempRaises)
    {
        raise.timer->stop();
        delete raise.timer;
    }

    mTempRaises.clear();
    if (count != 0)
    {
        emit signalSafeguardsChanged();
    }

    return count;
}

void LoggingScopesModelBase::_revertTempRaise(ITEM_ID rootId, const QString& path)
{
    uint32_t prio{ static_cast<uint32_t>(areg::LogPriority::PrioNotset) };
    bool found{ false };
    for (int pos = 0; pos < static_cast<int>(mTempRaises.size()); ++pos)
    {
        if ((mTempRaises[pos].rootId == rootId) && (mTempRaises[pos].path == path))
        {
            prio = mTempRaises[pos].prio;
            found = true;
            mTempRaises[pos].timer->deleteLater();
            mTempRaises.removeAt(pos);
            break;
        }
    }

    const int at{ found ? findRoot(rootId) : static_cast<int>(areg::INVALID_INDEX) };
    if (at != static_cast<int>(areg::INVALID_INDEX))
    {
        ScopeRoot* root = mLoggingModel->getRootList()[at];
        ScopeNodeBase* node{ path.isEmpty() ? static_cast<ScopeNodeBase *>(root) : root->findChildByPath(path) };
        const QModelIndex idxNode{ _indexOfNode(node) };
        if (idxNode.isValid())
        {
            // The walk back goes through the same call the raise did, so the target hears
            // about it the same way.
            setLogPriority(idxNode, prio);
            _notifyBranchChanged(index(at, 0, mRootIndex));
        }
    }

    emit signalSafeguardsChanged();
}

QModelIndex LoggingScopesModelBase::_indexOfNode(ScopeNodeBase* node) const
{
    if (node == nullptr)
        return QModelIndex();

    int pos{ static_cast<int>(areg::INVALID_INDEX) };
    if (node->isRoot())
    {
        pos = findRoot(static_cast<ScopeRoot *>(node)->getRootId());
    }
    else
    {
        const ScopeNodeBase* parent{ node->getParent() };
        pos = parent != nullptr ? parent->getChildPosition(node->getNodeName()) : static_cast<int>(areg::INVALID_INDEX);
    }

    return (pos != static_cast<int>(areg::INVALID_INDEX)) ? createIndex(pos, 0, node) : QModelIndex();
}

void LoggingScopesModelBase::setTargetState(const QModelIndex& node, ScopeRoot::eTargetState state)
{
    ScopeNodeBase* entry{ node.isValid() ? static_cast<ScopeNodeBase*>(node.internalPointer()) : nullptr };
    ScopeNodeBase* treeRoot{ entry != nullptr ? entry->getTreeRoot() : nullptr };
    if ((treeRoot == nullptr) || (isLiveSession() == false) || (mLoggingModel == nullptr))
        return;

    ScopeRoot* root{ static_cast<ScopeRoot *>(treeRoot) };
    const int pos{ findRoot(root->getRootId()) };
    if (pos == static_cast<int>(areg::INVALID_INDEX))
        return;

    root->setTargetState(state);
    const QModelIndex idxRoot{ index(pos, LoggingScopesModelBase::ColumnName, mRootIndex) };
    emit dataChanged(idxRoot, idxRoot, QList<int>{ LoggingScopesModelBase::RoleTargetState
                                                 , LoggingScopesModelBase::RoleTargetFade
                                                 , Qt::ItemDataRole::ToolTipRole });
    _runTargetClock();
}

void LoggingScopesModelBase::_ageTargetStates(void)
{
    if (mLoggingModel == nullptr)
        return;

    LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
    for (int pos = 0; pos < static_cast<int>(roots.size()); ++pos)
    {
        ScopeRoot* root = roots[pos];
        if ((root != nullptr) && root->ageTargetState(_targetTickMs))
        {
            const QModelIndex idxRoot{ index(pos, LoggingScopesModelBase::ColumnName, mRootIndex) };
            emit dataChanged(idxRoot, idxRoot, QList<int>{ LoggingScopesModelBase::RoleTargetState
                                                         , LoggingScopesModelBase::RoleTargetFade
                                                         , Qt::ItemDataRole::ToolTipRole });
        }
    }

    _runTargetClock();
}

void LoggingScopesModelBase::_runTargetClock(void)
{
    bool needed{ false };
    if (mLoggingModel != nullptr)
    {
        const LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (const ScopeRoot* root : roots)
        {
            if ((root != nullptr) && root->isTargetAgeing())
            {
                needed = true;
                break;
            }
        }
    }

    if (needed && (mTargetClock == nullptr))
    {
        mTargetClock = new QTimer(this);
        mTargetClock->setInterval(_targetTickMs);
        connect(mTargetClock, &QTimer::timeout, this, [this]() { _ageTargetStates(); });
    }

    if (mTargetClock == nullptr)
        return;

    if (needed)
    {
        if (mTargetClock->isActive() == false)
        {
            mTargetClock->start();
        }
    }
    else
    {
        mTargetClock->stop();
    }
}

void LoggingScopesModelBase::slotLogServiceConnected()
{
    clearModel(false);
}

void LoggingScopesModelBase::slotLogServiceDisconnected()
{
    clearModel(false);
}

bool LoggingScopesModelBase::slotInstancesAvailable(const std::vector<areg::ConnectedInstance> & instances)
{
    bool result {false};
    beginResetModel();
    for (const auto & instance : instances)
    {
        if ((instance.ciSource == areg::MessageSource::SourceObserver) || existsRoot(instance.ciCookie))
            continue;

        const int gone{ findGoneRoot(instance) };
        if (gone != static_cast<int>(areg::INVALID_INDEX))
        {
            result = true;
            reviveRoot(gone, instance);
        }
        else
        {
            result = true;
            ScopeRoot* root = new ScopeRoot(instance);
            if (appendRoot(root, false) == false)
                delete root;
        }
    }

    endResetModel();
    
    if (result)
    {
        emit signalRootUpdated(mRootIndex);
    }
    
    return result;
}

void LoggingScopesModelBase::slotInstancesUnavailable(const std::vector<ITEM_ID>& instIds)
{
    bool changed{false};

    if (mLoggingModel != nullptr)
    {
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        for (auto rootId : instIds)
        {
            for (int i = 0; i < static_cast<int>(roots.size()); ++i)
            {
                ScopeRoot* root = roots[i];
                Q_ASSERT(root != nullptr);
                if (root->getRootId() == rootId)
                {
                    changed = true;
                    root->setConnected(false);
                    if (root->targetState() == ScopeRoot::eTargetState::TargetSent)
                    {
                        root->setTargetState(ScopeRoot::eTargetState::TargetPending);
                    }
                    const QModelIndex idxRoot{ index(i, 0, mRootIndex) };
                    emit dataChanged(idxRoot, idxRoot, QList<int>{ Qt::ItemDataRole::DecorationRole });
                    break;
                }
            }
        }
    }

    if (changed)
    {
        emit signalRootUpdated(mRootIndex);
    }
}

void LoggingScopesModelBase::slotScopesAvailable(ITEM_ID instId, const std::vector<areg::ScopeEntry>& scopes)
{
    int pos = scopes.empty() == false ? findRoot(instId) : static_cast<int>(areg::INVALID_INDEX);
    if (pos != static_cast<int>(areg::INVALID_INDEX))
    {
        Q_ASSERT(mLoggingModel != nullptr);
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        int count = static_cast<int>(scopes.size());
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        beginInsertRows(idxInstance, 0, count);
        // beginResetModel();

        ScopeRoot* root = roots[pos];
        Q_ASSERT(root != nullptr);
        root->resetPrioritiesRecursive(false);
        for (int i = 0; i < count; ++i)
        {
            QString scopePath(QString::fromStdString(scopes[i].scopeName.data()));
            buildScope(*root, scopePath, scopes[i].scopePrio, scopes[i].scopeId);
        }

        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        applyRememberedPriorities(*root);

        endInsertRows();
        // endResetModel();
        emit signalScopesInserted(idxInstance);
    }
}

void LoggingScopesModelBase::slotScopesUpdated(ITEM_ID instId, const std::vector<areg::ScopeEntry>& scopes)
{
    int pos = scopes.empty() == false ? findRoot(instId) : static_cast<int>(areg::INVALID_INDEX);
    if (pos != static_cast<int>(areg::INVALID_INDEX))
    {
        Q_ASSERT(mLoggingModel != nullptr);
        LoggingModelBase::RootList& roots = mLoggingModel->getRootList();
        QModelIndex idxInstance = index(pos, 0, mRootIndex);
        int count = static_cast<int>(scopes.size());
        ScopeRoot* root = roots[pos];
        Q_ASSERT(root != nullptr);
        for (int i = 0; i < count; ++i)
        {
            const areg::ScopeEntry & scope = scopes[i];
            QString scopeName{ scope.scopeName };
            root->addChildPriorityRecursive(scopeName, scope.scopePrio);
        }

        root->resetPrioritiesRecursive(true);
        root->refreshPrioritiesRecursive();
        if (root->targetState() == ScopeRoot::eTargetState::TargetSent)
        {
            root->setTargetState(ScopeRoot::eTargetState::TargetApplied);
        }

        QModelIndex entry = index(pos, 0, mRootIndex);
        emit signalScopesUpdated(idxInstance);
        emit dataChanged(entry, entry, { Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::DisplayRole });
    }
}

void LoggingScopesModelBase::_setupSignals(bool doSetup)
{
    if (doSetup)
    {
        if (mSignalsSetup)
            return;
        
        mSignalsSetup = true;
        mConSvcConnected = connect(mLoggingModel, &LoggingModelBase::signalLogServiceConnected      , this, [this]() {
            this->slotLogServiceConnected();
        });
        mConSvcDisconnected = connect(mLoggingModel, &LoggingModelBase::signalLogServiceDisconnected, this, [this]() {
            this->slotLogServiceDisconnected();
        });
        mConInstAvailable = connect(mLoggingModel, &LoggingModelBase::signalInstanceAvailable       , this, [this](const std::vector<areg::ConnectedInstance>& instances) {
            this->slotInstancesAvailable(instances);
        });
        mConInstUnavailable = connect(mLoggingModel, &LoggingModelBase::signalInstanceUnavailable   , this, [this](const std::vector<ITEM_ID>& instIds) {
            this->slotInstancesUnavailable(instIds);
        });
        mConScopesAvailable = connect(mLoggingModel, &LoggingModelBase::signalScopesAvailable       , this, [this](ITEM_ID instId, const std::vector<areg::ScopeEntry>& scopes) {
            this->slotScopesAvailable(instId, scopes);
        });
        mConScopesUnavailable = connect(mLoggingModel, &LoggingModelBase::signalScopesUpdated       , this, [this](ITEM_ID instId, const std::vector<areg::ScopeEntry>& scopes) {
            this->slotScopesUpdated(instId, scopes);
        });
    }
    else if (mSignalsSetup)
    {
        disconnect(mConSvcConnected);
        disconnect(mConSvcDisconnected);
        disconnect(mConInstAvailable);
        disconnect(mConInstUnavailable);
        disconnect(mConScopesAvailable);
        disconnect(mConScopesUnavailable);
        
        mSignalsSetup = false;
    }
}

void LoggingScopesModelBase::buildScopes()
{
    if (mLoggingModel == nullptr)
        return;
    
    beginResetModel();
    const std::vector<areg::ConnectedInstance>& instances = mLoggingModel->getLogInstances();
    slotInstancesAvailable(instances);
    
    for (const auto& inst : instances)
    {
        const std::vector<areg::ScopeEntry> & scopes = mLoggingModel->getLogInstScopes(inst.ciCookie);
        slotScopesAvailable(inst.ciCookie, scopes);
    }
    
    endResetModel();
}

void LoggingScopesModelBase::setupModel()
{
    clearModel(false);
    if (mLoggingModel != nullptr)
    {
        mLoggingModel->setupModel();
    }
}

void LoggingScopesModelBase::releaseModel()
{
    clearModel(true);
    if (mLoggingModel != nullptr)
    {
        mLoggingModel->releaseModel();
    }
}

bool LoggingScopesModelBase::isLiveSession() const
{
    return false;
}

void LoggingScopesModelBase::dataTransfer(LoggingScopesModelBase& scopeModel)
{
    beginResetModel();
    _setupSignals(false);
    mLoggingModel = scopeModel.mLoggingModel;
    if (mLoggingModel != nullptr)
    {
        if (scopeModel.mLoggingModel != nullptr)
        {
            mLoggingModel->dataTransfer(*scopeModel.mLoggingModel);
        }

        _setupSignals(true);
        slotLogServiceConnected();
    }
    
    mRootIndex = std::move(scopeModel.mRootIndex);
    scopeModel.mRootIndex = QModelIndex();

    endResetModel();
}

void LoggingScopesModelBase::refresh()
{
    beginResetModel();
    endResetModel();
}

