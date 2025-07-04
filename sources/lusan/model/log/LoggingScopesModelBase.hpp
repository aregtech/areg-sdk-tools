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

/************************************************************************
 * Dependencies
 ************************************************************************/
class ScopeNodeBase;
class ScopeRoot;

/**
 * \brief   Base class for log scope models (live and offline).
 *          Provides common functionality for scope tree navigation.
 **/
class LoggingScopesModelBase : public QAbstractItemModel
{
    Q_OBJECT

protected:
    using RootList = QList< ScopeRoot*>;

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
     * \brief   Releases the model, disconnects the signals.
     **/
    virtual void release(void) = 0;
    
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
    
protected:

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
// Protected member variables
//////////////////////////////////////////////////////////////////////////
protected:
    RootList        mRootList;      // The list of root nodes
    QModelIndex     mRootIndex;     // The root index of the model
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

#endif  // LUSAN_MODEL_LOG_LOGGINGSCOPESMODELBASE_HPP