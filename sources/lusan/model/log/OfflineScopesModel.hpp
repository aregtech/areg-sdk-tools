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
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    
    virtual void setLoggingModel(LoggingModelBase* model);

signals:

/************************************************************************
 * Signals
 ************************************************************************/

     /**
      * \brief   Signal emitted when the root of the model is updated.
      * \param   root    The index of the root that is updated.
      **/
    void signalRootUpdated(const QModelIndex& root);

private:

    /**
     * \brief   Builds the scope tree from offline model data.
     **/
    void _buildScopeTree(void);
};

//////////////////////////////////////////////////////////////////////////
// OfflineScopesModel class inline methods
//////////////////////////////////////////////////////////////////////////

#endif  // LUSAN_MODEL_LOG_OFFLINESCOPESMODEL_HPP
