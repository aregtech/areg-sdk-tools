#ifndef LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP
#define LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP
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
 *  \file        lusan/model/log/LoggingScopesModelBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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

#include "areg/component/NEService.hpp"
#include "areg/logging/NELogging.hpp"

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
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Initializes the scope model object.
     * @param   parent  The pointer to the parent object.
     */
    explicit LoggingScopesModelBase(QObject* parent = nullptr);

    virtual ~LoggingScopesModelBase(void);

//////////////////////////////////////////////////////////////////////////
// Common operations
//////////////////////////////////////////////////////////////////////////
public:
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
     * \brief   Returns the logging model associated with this scopes model. Returns `nullptr` if not set.
     **/
    inline LoggingModelBase* getLoggingModel(void) const;

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
    
//////////////////////////////////////////////////////////////////////////
// LoggingScopesModelBase overrides
//////////////////////////////////////////////////////////////////////////
public:
/************************************************************************
 * LoggingScopesModelBase overrides
 ************************************************************************/

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
    virtual void refresh(void);

    /**
     * \brief   Builds the scopes tree for the model.
     **/
    virtual void buildScopes(void);

    /**
     * \brief   Sets up the model.
     **/
    virtual void setupModel(void);

    /**
     * \brief   Releases the model.
     **/
    virtual void releaseModel(void);

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

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:

    /**
     * \brief   Triggered, when connected to the logging service.
     */
    virtual void slotLogServiceConnected(void);

    /**
     * \brief   Triggered, when disconnected to the logging service.
     */
    virtual void slotLogServiceDisconnected(void);

    /**
     * \brief   Triggered, when one or more instances are available.
     *          This can be either instances connected to the log collector service in live mode
     *          of instances read from the log database in offline mode.
     *          In case of disconnected mode this signal is not triggered.
     * \param   instances   The list of instances available.
     * \return  Returns true if the instance was added to the root element.
     **/
    virtual bool slotInstancesAvailable(const std::vector<NEService::sServiceConnectedInstance> & instances);
    
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
    virtual void slotScopesAvailable(ITEM_ID instId, const std::vector<NELogging::sScopeInfo>& scopes);

    /**
     * \brief   Signal emitted when scopes of the specified instance are updated.
     *          This can be scopes received in the live mode.
     *          In case of offline or disconnected modes this signal is not triggered.
     * \param   instId      The ID of the instance whose scopes are updated.
     * \param   scopes      The list of updated scopes.
     **/
    virtual void slotScopesUpdated(ITEM_ID instId, const std::vector<NELogging::sScopeInfo>&scopes);
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
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
    return (index.isValid() && (index.row() >= 0) && (index.column() == 0) && (index.model() == this));
}

inline const QModelIndex& LoggingScopesModelBase::getRootIndex(void) const
{
    return mRootIndex;
}

inline LoggingModelBase* LoggingScopesModelBase::getLoggingModel(void) const
{
    return mLoggingModel;
}

#endif  // LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP
