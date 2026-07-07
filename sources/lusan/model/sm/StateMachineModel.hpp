#ifndef LUSAN_MODEL_SM_STATEMACHINEMODEL_HPP
#define LUSAN_MODEL_SM_STATEMACHINEMODEL_HPP
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
 *  \file        lusan/model/sm/StateMachineModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document facade.
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMOverviewModel.hpp"
#include "lusan/model/sm/SMDataTypeModel.hpp"
#include "lusan/model/sm/SMAttributeModel.hpp"
#include "lusan/model/sm/SMEventModel.hpp"
#include "lusan/model/sm/SMTimerModel.hpp"
#include "lusan/model/sm/SMMethodModel.hpp"
#include "lusan/model/sm/SMConstantModel.hpp"

#include <QObject>
#include <QTimer>
#include <QUndoStack>
#include <memory>

class StateMachineModel : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit StateMachineModel(QObject* parent = nullptr);
    virtual ~StateMachineModel() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    bool createNewDocument(const QString& machineName);
    bool loadFromFile(const QString& documentPath, const QString& sourcePath = QString());
    bool saveToFile(const QString& filePath = QString());
    bool writeAutosave();
    bool removeAutosave();

    inline bool openSucceeded() const;
    inline bool isDirty() const;
    inline const QString& getFilePath() const;

    inline StateMachineData& getData();
    inline const StateMachineData& getData() const;
    inline QUndoStack& getUndoStack();
    inline const QUndoStack& getUndoStack() const;
    inline DocModelNotifier& getNotifier();
    inline SMOverviewModel& getOverviewModel();
    inline SMDataTypeModel& getDataTypeModel();
    inline SMAttributeModel& getAttributeModel();
    inline SMEventModel& getEventModel();
    inline SMTimerModel& getTimerModel();
    inline SMMethodModel& getMethodModel();
    inline SMConstantModel& getConstantModel();

signals:
    void signalDirtyChanged(bool dirty);

private slots:
    void onAutosaveTimeout();
    void onUndoCleanChanged(bool clean);

private:
    void markDirty();
    void updateAutosaveTimer();

private:
    std::unique_ptr<StateMachineData> mData;
    DocModelNotifier mNotifier;
    QUndoStack      mUndoStack;
    QTimer          mAutosaveTimer;
    SMOverviewModel mOverviewModel;
    SMDataTypeModel mDataTypeModel;
    SMAttributeModel mAttributeModel;
    SMEventModel    mEventModel;
    SMTimerModel    mTimerModel;
    SMMethodModel   mMethodModel;
    SMConstantModel mConstantModel;
    bool            mOpenSuccess;
};

inline bool StateMachineModel::openSucceeded() const
{
    return mOpenSuccess;
}

inline bool StateMachineModel::isDirty() const
{
    return mUndoStack.isClean() == false;
}

inline const QString& StateMachineModel::getFilePath() const
{
    static const QString _empty;
    return (mData != nullptr ? mData->getFilePath() : _empty);
}

inline StateMachineData& StateMachineModel::getData()
{
    return *mData;
}

inline const StateMachineData& StateMachineModel::getData() const
{
    return *mData;
}

inline QUndoStack& StateMachineModel::getUndoStack()
{
    return mUndoStack;
}

inline const QUndoStack& StateMachineModel::getUndoStack() const
{
    return mUndoStack;
}

inline DocModelNotifier& StateMachineModel::getNotifier()
{
    return mNotifier;
}

inline SMOverviewModel& StateMachineModel::getOverviewModel()
{
    return mOverviewModel;
}

inline SMDataTypeModel& StateMachineModel::getDataTypeModel()
{
    return mDataTypeModel;
}

inline SMAttributeModel& StateMachineModel::getAttributeModel()
{
    return mAttributeModel;
}

inline SMEventModel& StateMachineModel::getEventModel()
{
    return mEventModel;
}

inline SMTimerModel& StateMachineModel::getTimerModel()
{
    return mTimerModel;
}

inline SMMethodModel& StateMachineModel::getMethodModel()
{
    return mMethodModel;
}

inline SMConstantModel& StateMachineModel::getConstantModel()
{
    return mConstantModel;
}

#endif  // LUSAN_MODEL_SM_STATEMACHINEMODEL_HPP
