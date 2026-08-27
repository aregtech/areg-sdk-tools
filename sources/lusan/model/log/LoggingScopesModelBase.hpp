#ifndef LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP
#define LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP
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
 *  \file        lusan/model/log/LoggingScopesModelBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Base class for log scopes models.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QSet>

#include "areg/component/ServiceDefs.hpp"
#include "areg/logging/areg_log.h"

/************************************************************************
 * Dependencies
 ************************************************************************/
class TableModelBase;
class LoggingModelBase;
class ScopeNodeBase;
class ScopeRoot;

/**
 * \brief   Base class for log scope models (live and offline).
 *          Provides common functionality for scope tree navigation.
 **/
class LoggingScopesModelBase : public QAbstractItemModel
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< The leading column, which carries the show and hide box of the scope.
    static constexpr int    ColumnShow  { 0 };

    //!< The column of the scope name and its charger. The tree lives in this column.
    static constexpr int    ColumnName  { 1 };

    //!< The number of columns the scope tree has.
    static constexpr int    ColumnCount { 2 };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Initializes the scope model object.
     * @param   parent  The pointer to the parent object.
     */
    explicit LoggingScopesModelBase(QObject* parent = nullptr);

    virtual ~LoggingScopesModelBase();

//////////////////////////////////////////////////////////////////////////
// Common operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Checks if the given index belongs to this model and addresses an existing cell.
     * \param   index   The index to check.
     * \return  True if the index is valid, false otherwise.
     * \note    It accepts every column of the model. A caller that serves one column has to
     *          test the column itself.
     **/
    inline bool isValidIndex(const QModelIndex& index) const;

    /**
     * \brief   Returns root index.
     **/
    inline const QModelIndex& getRootIndex() const;

    /**
     * \brief   Returns the logging model associated with this scopes model. Returns `nullptr` if not set.
     **/
    inline LoggingModelBase* getLoggingModel() const;

    /**
     * \brief   Call to set the index of the expanded scope node.
     * \param   idxNode     The index of the expanded scope node.
     **/
    void nodeExpanded(const QModelIndex& idxNode);

    /**
     * \brief   Call to set the index of the collapsed scope node.
     * \param   idxNode     The index of the collapsed scope node.
     **/
    void nodeCollapsed(const QModelIndex& idxNode);

    /**
     * \brief   Call to set the index of the selected scope node.
     * \param   idxNode     The index of the selected scope node.
     */
    void nodeSelected(const QModelIndex& idxNode);
    
    /**
     * \brief   Sets the node and all child nodes tree in the expanded state.
     * \param   idxNode     The index of the expanded scope node.
     **/
    void nodeTreeExpanded(const QModelIndex& idxNode);
    
    /**
     * \brief   Sets the node and all child nodes tree in the collapsed state.
     * \param   idxNode     The index of the collapsed scope node.
     **/
    void nodeTreeCollapsed(const QModelIndex& idxNode);
    
/************************************************************************
 * Signals
 ************************************************************************/
signals:

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

    /**
     * \brief   Signal emitted when the set of scopes the log window should draw has changed.
     **/
    void signalScopeVisibilityChanged();
    
//////////////////////////////////////////////////////////////////////////
// LoggingScopesModelBase overrides
//////////////////////////////////////////////////////////////////////////
public:
/************************************************************************
 * LoggingScopesModelBase overrides
 ************************************************************************/

    /**
     * \brief   Draws or stops drawing the rows of the given node and of everything under it.
     * \param   index   The index of the node.
     * \param   shown   True to draw the rows, false to leave them out.
     **/
    void setScopeShown(const QModelIndex& index, bool shown);

    /**
     * \brief   Draws the rows of the given node and of everything under it, and of nothing
     *          else in any process.
     * \param   index   The index of the node to keep.
     **/
    void showScopeAlone(const QModelIndex& index);

    /**
     * \brief   Draws the rows of every scope of every process again.
     **/
    void showAllScopes(void);

    /**
     * \brief   Returns true if at least one scope of any process is hidden.
     **/
    bool hasHiddenScopes(void) const;

    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the specified node has scope priority, it will not be changed.
     * \param   index   The index of the log scope to change priority.
     * \param   prio    The new priority to set for the log scope on target.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    virtual bool setLogPriority(const QModelIndex& index, uint32_t prio) = 0;

    /**
     * \brief   Adds the specified log priority to the log scope at the given index.
     *          The request to change the log priority is sent to the target module.
     *          If the log scope already has this priority, it will not be added again.
     * \param   index   The index of the log scope to add priority.
     * \param   prio    The log priority to add to the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    virtual bool addLogPriority(const QModelIndex& index, uint32_t prio) = 0;

    /**
     * \brief   Removes the specified log priority from the log scope at the given index.
     *          The request to remove the log priority is sent to the target module.
     *          If the log scope does not have this priority, it will not be removed.
     * \param   index   The index of the log scope to remove priority.
     * \param   prio    The log priority to remove from the log scope.
     * \return  True if succeeded to sent the request to update log priority on target module.
     **/
    virtual bool removLogPriority(const QModelIndex& index, uint32_t prio) = 0;

    /**
     * \brief   Saves the log scope priority for the given target index.
     *          If the target index is invalid, it saves the log scope priority for all instances.
     * \param   target  The target index to save log scope priority. If invalid, saves for root index.
     * \return  True if succeeded to save log scope priority, false otherwise.
     **/
    virtual bool saveLogScopePriority(const QModelIndex& target = QModelIndex()) const = 0;
    
    /**
     * \brief   Sets the logging model object used to retrieve logging scopes data.
     * \param   model   The logging model to set. This can be `nullptr` to reset the model.
     *                  If can be either live logging model or offline logging model.
     **/
    virtual void setLoggingModel(LoggingModelBase* model);

    /**
     * \brief   Transfers the data from given model. Copies the list of root elements and builds the scope tree.
     * \param   scopeModel  The source of data to copy. On output the list of existing data may be empty.
     **/
    virtual void dataTransfer(LoggingScopesModelBase& scopeModel);
    
    /**
     * \brief   Refreshes the model, clearing all data and rebuilding the scope tree.
     **/
    virtual void refresh();

    /**
     * \brief   Builds the scopes tree for the model.
     **/
    virtual void buildScopes();

    /**
     * \brief   Sets up the model.
     **/
    virtual void setupModel();

    /**
     * \brief   Releases the model.
     **/
    virtual void releaseModel();

    /**
     * \brief   Returns true if the model follows running targets. An archive returns false,
     *          so a process that is no longer reachable is never marked gone in it.
     **/
    virtual bool isLiveSession() const;

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
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    /**
     * \brief   Returns the parent index of the given child index.
     * \param   child   The child index.
     * \return  The parent index.
     **/
    QModelIndex parent(const QModelIndex& child) const override;

    /**
     * \brief   Returns the number of rows under the given parent.
     * \param   parent  The parent index.
     * \return  The number of rows.
     **/
    int rowCount(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the number of columns for the children of the given parent.
     * \param   parent  The parent index.
     * \return  The number of columns.
     **/
    int columnCount(const QModelIndex& parent) const override;

    /**
     * \brief   Returns the data stored under the given role for the item referred to by the index.
     * \param   index   The index of the item.
     * \param   role    The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    QVariant data(const QModelIndex& index, int role) const override;

    /**
     * \brief   Returns the data for the given role and section in the header with the specified orientation.
     * \param   section     The section of the header.
     * \param   orientation The orientation of the header.
     * \param   role        The role for which data is requested.
     * \return  The data for the given role and section.
     **/
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * \brief   Returns the flags for the item at the given index.
     * \param   index   The index of the item.
     * \return  The flags of the item.
     **/
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * \brief   Takes the click on the show and hide box of the leading column.
     *          A node that is only partly shown becomes fully shown; a fully shown node hides.
     * \param   index   The index that was clicked.
     * \param   value   Ignored: the new state follows from the current one.
     * \param   role    Only the check state role is handled.
     * \return  True if the state changed.
     **/
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::ItemDataRole::EditRole) override;
    
//////////////////////////////////////////////////////////////////////////
// Internal overrides
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Build scopes from a single scope path
     **/
    virtual void buildScope(ScopeRoot& root, QString& scopePath, uint32_t scopePrio, uint32_t scopeId);

//////////////////////////////////////////////////////////////////////////
// Internal operations
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Clears the model and deletes all nodes.
     **/
    void clearModel(bool notify = false);

    /**
     * \brief   Checks if the root with the given ID exists in the model.
     * \param   rootId  The ID of the root to check.
     * \return  True if the root exists, false otherwise.
     **/
    bool existsRoot(ITEM_ID rootId) const;

    /**
     * \brief   Appends the root to the model.
     * \param   root    The root to append.
     * \param   unique  If true, checks if the root is unique before appending.
     * \return  True if the root is appended, false otherwise.
     **/
    bool appendRoot(ScopeRoot* root, bool unique = true);

    /**
     * \brief   Finds the root with the given ID in the model.
     * \param   rootId  The ID of the root to find.
     * \return  The position of the root in the list, or NECommon::INVALID_INDEX if not found.
     **/
    int findRoot(ITEM_ID rootId) const;

    /**
     * \brief   Collects the identifiers of every scope the log window should not draw.
     * \param   scopeIds    The set to add the identifiers to.
     * \return  The number of identifiers added.
     **/
    int collectHiddenScopes(QSet<uint32_t>& scopeIds) const;


    /**
     * \brief   Finds a root that stands for the same program in the same place and is
     *          currently marked gone. The cookie is not compared, because a restarted
     *          process is handed a new one.
     * \param   instance    The connection that has just arrived.
     * \return  The position of the root in the list, or NECommon::INVALID_INDEX if not found.
     **/
    int findGoneRoot(const areg::ConnectedInstance & instance) const;

    /**
     * \brief   Puts a gone root back in service under its new cookie. The scopes it held
     *          are dropped, because the target reports them again; the priorities that were
     *          set on them are remembered and reapplied once they arrive.
     * \param   pos         The position of the root in the list.
     * \param   instance    The connection that has just arrived.
     * \note    Emits no row change of its own. Call it inside a model reset.
     **/
    void reviveRoot(int pos, const areg::ConnectedInstance & instance);

    /**
     * \brief   Applies the priorities a revived root remembered, after its scopes arrived.
     *          The live model additionally sends them to the target.
     * \param   root        The root whose scopes have just been rebuilt.
     * \return  The number of scopes the priority was applied to.
     **/
    virtual int applyRememberedPriorities(ScopeRoot & root);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:

    /**
     * \brief   Triggered, when connected to the logging service.
     */
    virtual void slotLogServiceConnected();

    /**
     * \brief   Triggered, when disconnected to the logging service.
     */
    virtual void slotLogServiceDisconnected();

    /**
     * \brief   Triggered, when one or more instances are available.
     *          This can be either instances connected to the log collector service in live mode
     *          of instances read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instances   The list of instances available.
     * \return  Returns true if the instance was added to the root element.
     **/
    virtual bool slotInstancesAvailable(const std::vector<areg::ConnectedInstance> & instances);
    
    /**
     * \brief   Signal emitted when one or more instances are disconnected.
     *          This can be instances disconnected from the log collector service in live mode.
     *          In case of offline or disconnected modes this signal is not triggered.
     * \param   instances   The list of instances updated.
     **/
    virtual void slotInstancesUnavailable(const std::vector<ITEM_ID>& instIds);

    /**
     * \brief   Signal emitted when scopes of the specified instance are available.
     *          This can be either scopes received in the live mode
     *          or scopes read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instIds     The list of IDs of disconnected instances.
     **/
    virtual void slotScopesAvailable(ITEM_ID instId, const std::vector<areg::ScopeEntry>& scopes);

    /**
     * \brief   Signal emitted when scopes of the specified instance are updated.
     *          This can be scopes received in the live mode.
     *          In case of offline or disconnected modes this signal is not triggered.
     * \param   instId      The ID of the instance whose scopes are updated.
     * \param   scopes      The list of updated scopes.
     **/
    virtual void slotScopesUpdated(ITEM_ID instId, const std::vector<areg::ScopeEntry>&scopes);
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Tells the view that the show and hide box of everything below the given index has changed.
    void _notifyBranchChanged(const QModelIndex& parent);

    //!< Tells the view that the show and hide box of every parent of the given index has changed.
    void _notifyParentsChanged(const QModelIndex& child);

private:
    
    /**
     * \brief   Sets up the signals for the logging model.
     *          If `doSetup` is true, connects to the signals of the logging model.
     *          If `doSetup` is false, disconnects from the signals of the logging model.
     * \param   doSetup  If true, sets up signals; if false, tears down signals.
     **/
    void _setupSignals(bool doSetup);

//////////////////////////////////////////////////////////////////////////
// Protected member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QModelIndex             mRootIndex;             //!< The root index of the model
    LoggingModelBase*       mLoggingModel;          //!< The logging model associated with this scopes model
    
//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    bool                    mSignalsSetup;          // The flag, indicating whether the signals are setup or not
    QMetaObject::Connection mConSvcConnected;       // The connection to the log collector service
    QMetaObject::Connection mConSvcDisconnected;    // The connection to the log collector service disconnection
    QMetaObject::Connection mConInstAvailable;      // The connection to instances available signal
    QMetaObject::Connection mConInstUnavailable;    // The connection to instances unavailable signal
    QMetaObject::Connection mConScopesAvailable;    // The connection to scopes available signal
    QMetaObject::Connection mConScopesUnavailable;  // The connection to scopes unavailable signal
};

//////////////////////////////////////////////////////////////////////////
// LoggingScopesModelBase class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LoggingScopesModelBase::isValidIndex(const QModelIndex& index) const
{
    return ( index.isValid()
          && (index.row() >= 0)
          && (index.column() >= 0)
          && (index.column() < LoggingScopesModelBase::ColumnCount)
          && (index.model() == this) );
}

inline const QModelIndex& LoggingScopesModelBase::getRootIndex() const
{
    return mRootIndex;
}

inline LoggingModelBase* LoggingScopesModelBase::getLoggingModel() const
{
    return mLoggingModel;
}

#endif  // LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP
