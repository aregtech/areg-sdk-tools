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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/StateMachineModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM document facade.
 *
 ************************************************************************/

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocUndoStack.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"
#include "lusan/model/sm/SMOverviewModel.hpp"
#include "lusan/model/common/DataTypeModel.hpp"
#include "lusan/model/sm/SMAttributeModel.hpp"
#include "lusan/model/sm/SMEventModel.hpp"
#include "lusan/model/sm/SMTimerModel.hpp"
#include "lusan/model/sm/SMMethodModel.hpp"
#include "lusan/model/common/ConstantModel.hpp"
#include "lusan/model/sm/SMIncludeModel.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/common/DocValidationController.hpp"

#include <QObject>
#include <QTimer>
#include <memory>

class StateMachineModel : public QObject
                        , public IEDocumentModel
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
    void publishStateNamePreview(uint32_t stateId, const QString& text);

    inline bool openSucceeded() const;
    inline bool isDirty() const;
    inline const QString& getFilePath() const;

    /**
     * \brief   Marks the document read-only: the undo stack refuses every command and saving is
     *          declined. Used for a submachine import opened from its host -- the import is
     *          edited by opening it as its own document, never through a host.
     * \param   readOnly    Whether the document may be changed.
     * \param   origin      Where the read-only view was opened from (host document and alias),
     *                      shown by the editor so the user knows which document they are in.
     **/
    void setReadOnly(bool readOnly, const QString& origin = QString());

    inline bool isReadOnly() const;
    inline const QString& getReadOnlyOrigin() const;

    inline StateMachineData& getData();
    inline const StateMachineData& getData() const;
    inline DocUndoStack& getUndoStack() override;
    inline const DocUndoStack& getUndoStack() const;
    inline DocModelNotifier& getNotifier() override;

    /**
     * \brief   The document's custom data types, so a declared type name can be resolved.
     **/
    const QList<DataTypeCustom*>& getCustomDataTypes() const override;

    /**
     * \brief   The document's `ConstantList` section. Read from the data object that is current
     *          now: opening or reloading a file replaces it.
     **/
    inline ConstantDataSection& getConstantSection() override;

    /**
     * \brief   The document's `DataTypeList` section, read from the data object that is current
     *          now for the same reason as the constants section above.
     **/
    inline DataTypeDataSection& getDataTypeSection() override;

    /**
     * \brief   Builds the command that rewrites whatever refers to a renamed element by name.
     *          A state machine reaches guards, operations and transition stimuli this way, so
     *          unlike a service interface it always has repair work to do.
     **/
    QUndoCommand* createRenameSideEffects( eDocElementKind kind, uint32_t id
                                         , const QString& oldName, const QString& newName
                                         , QUndoCommand* parent) override;

    /**
     * \brief   Names a state or a transition for a results row. A transition has no name of its
     *          own, so it is identified by what it reacts to and where it leads, the way it is
     *          labelled on the canvas. Everything else is left to its kind: those messages quote
     *          the name that failed already.
     **/
    QString describeElement(uint32_t id, eDocElementKind kind) const override;

    inline SMOverviewModel& getOverviewModel();
    inline DataTypeModel& getDataTypeModel();
    inline SMAttributeModel& getAttributeModel();
    inline SMEventModel& getEventModel();
    inline SMTimerModel& getTimerModel();
    inline SMMethodModel& getMethodModel();
    inline ConstantModel& getConstantModel();
    inline SMIncludeModel& getIncludeModel();
    inline SMSelectionModel& getSelectionModel();
    inline DocValidationController& getValidationController() override;

signals:
    void signalDirtyChanged(bool dirty);
    void signalStateNamePreview(uint32_t stateId, const QString& text);

private slots:
    void onAutosaveTimeout();
    void onUndoCleanChanged(bool clean);

private:
    void markDirty();
    void updateAutosaveTimer();

private:
    std::unique_ptr<StateMachineData> mData;
    DocModelNotifier mNotifier;
    DocUndoStack    mUndoStack;
    QTimer          mAutosaveTimer;
    SMOverviewModel mOverviewModel;
    DataTypeModel mDataTypeModel;
    SMAttributeModel mAttributeModel;
    SMEventModel    mEventModel;
    SMTimerModel    mTimerModel;
    SMMethodModel   mMethodModel;
    ConstantModel mConstantModel;
    SMIncludeModel  mIncludeModel;
    SMSelectionModel mSelectionModel;
    bool            mOpenSuccess;
    QString         mReadOnlyOrigin;    //!< Non-empty only for a read-only import view.
    DocValidationController mValidationController; //!< Background structural/reference validation.
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

inline ConstantDataSection& StateMachineModel::getConstantSection()
{
    return mData->getConstants();
}

inline DataTypeDataSection& StateMachineModel::getDataTypeSection()
{
    return mData->getDataTypes();
}

inline bool StateMachineModel::isReadOnly() const
{
    return mUndoStack.isReadOnly();
}

inline const QString& StateMachineModel::getReadOnlyOrigin() const
{
    return mReadOnlyOrigin;
}

inline DocUndoStack& StateMachineModel::getUndoStack()
{
    return mUndoStack;
}

inline const DocUndoStack& StateMachineModel::getUndoStack() const
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

inline DataTypeModel& StateMachineModel::getDataTypeModel()
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

inline ConstantModel& StateMachineModel::getConstantModel()
{
    return mConstantModel;
}

inline SMIncludeModel& StateMachineModel::getIncludeModel()
{
    return mIncludeModel;
}

inline SMSelectionModel& StateMachineModel::getSelectionModel()
{
    return mSelectionModel;
}

inline DocValidationController& StateMachineModel::getValidationController()
{
    return mValidationController;
}

#endif  // LUSAN_MODEL_SM_STATEMACHINEMODEL_HPP
