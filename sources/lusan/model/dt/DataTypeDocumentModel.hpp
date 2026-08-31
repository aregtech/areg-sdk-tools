#ifndef LUSAN_MODEL_DT_DATATYPEDOCUMENTMODEL_HPP
#define LUSAN_MODEL_DT_DATATYPEDOCUMENTMODEL_HPP
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
 *  \file        lusan/model/dt/DataTypeDocumentModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/dt/DataTypeDocumentData.hpp"
#include "lusan/model/common/DataTypeModel.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/DocUndoStack.hpp"
#include "lusan/model/common/DocValidationController.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"
#include "lusan/model/common/IncludeModel.hpp"
#include "lusan/model/common/OverviewModel.hpp"

#include "lusan/data/common/AttributeDataSection.hpp"
#include "lusan/data/common/ConstantDataSection.hpp"
#include "lusan/data/common/MethodDataSection.hpp"

/**
 * \class   DataTypeDocumentModel
 * \brief   The model of a `.dtml` document: its Overview, its data types and its includes,
 *          each edited through the same shared page model the other two documents use.
 **/
class DataTypeDocumentModel : public IEDocumentModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    DataTypeDocumentModel(const QString& filePath = QString());

    virtual ~DataTypeDocumentModel(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline OverviewModel& getOverviewModel(void);
    inline DataTypeModel& getDataTypeModel(void);
    inline IncludeModel& getIncludesModel(void);

    /**
     * \brief   Writes the document to the given file.
     **/
    inline bool saveToFile(const QString& filePath);

    inline QString getFileFormatVersion(void) const;
    inline const QString& getName(void) const;
    inline const VersionNumber& getVersion(void) const;

    /**
     * \brief   True when the file the model was built with was read.
     **/
    inline bool openSucceeded(void) const;

    inline DataTypeDocumentData& getData(void);
    inline const DataTypeDocumentData& getData(void) const;

    /**
     * \brief   True while the document holds edits that have not been saved.
     **/
    inline bool isDirty(void) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    inline DocModelNotifier& getNotifier(void) override;
    inline DocUndoStack& getUndoStack(void) override;
    inline const DocUndoStack& getUndoStack(void) const;

    const QList<DataTypeCustom*>& getCustomDataTypes(void) const override;

    inline OverviewDataSection& getOverviewSection(void) override;
    inline DataTypeDataSection& getDataTypeSection(void) override;
    inline IncludeDataSection& getIncludeSection(void) override;

    /**
     * \brief   A data type document declares no constant, attribute or method, so these three
     *          answer with sections that stay empty. They exist because the document facade is
     *          one interface for every document kind; nothing here builds a page over them, and
     *          the writer never sees them.
     **/
    inline ConstantDataSection& getConstantSection(void) override;
    inline AttributeDataSection& getAttributeSection(void) override;
    inline MethodDataSection& getMethodSection(void) override;

    inline DocValidationController& getValidationController(void) override;

    /**
     * \brief   The file the document lives in, empty while it has never been saved.
     **/
    inline QString getDocumentPath(void) const override;

    /**
     * \brief   A data type document is a leaf: it includes C++ headers and no document of any
     *          kind, so nothing in its include list contributes a type here.
     **/
    inline bool takesDataTypeImports(void) const override;

    /**
     * \brief   Re-resolves the document's declared types. It has nothing else that declares one.
     **/
    inline void refreshTypeReferences(void) override;

//////////////////////////////////////////////////////////////////////////
// Hidden class members
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeDocumentData    mDTData;        //!< The document data.
    DocModelNotifier        mNotifier;      //!< The document's change notifier.
    DocUndoStack            mUndoStack;     //!< The document's undo stack.
    OverviewModel           mModelOverview; //!< The Overview page model.
    DataTypeModel           mModelDataType; //!< The Data Types page model.
    IncludeModel            mModelInclude;  //!< The Includes page model.
    ConstantDataSection     mNoConstants;   //!< Never filled; see the accessors above.
    AttributeDataSection    mNoAttributes;  //!< Never filled; see the accessors above.
    MethodDataSection       mNoMethods;     //!< Never filled; see the accessors above.
    DocValidationController mValidation;    //!< Background structural validation.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeDocumentModel(const DataTypeDocumentModel& /*src*/) = delete;
    DataTypeDocumentModel& operator = (const DataTypeDocumentModel& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// DataTypeDocumentModel inline methods
//////////////////////////////////////////////////////////////////////////

inline OverviewModel& DataTypeDocumentModel::getOverviewModel(void)
{
    return mModelOverview;
}

inline DataTypeModel& DataTypeDocumentModel::getDataTypeModel(void)
{
    return mModelDataType;
}

inline IncludeModel& DataTypeDocumentModel::getIncludesModel(void)
{
    return mModelInclude;
}

inline bool DataTypeDocumentModel::saveToFile(const QString& filePath)
{
    return mDTData.writeToFile(filePath);
}

inline QString DataTypeDocumentModel::getFileFormatVersion(void) const
{
    return mDTData.getFileFormatVersion();
}

inline const QString& DataTypeDocumentModel::getName(void) const
{
    return mDTData.getOverviewData().getName();
}

inline const VersionNumber& DataTypeDocumentModel::getVersion(void) const
{
    return mDTData.getOverviewData().getVersion();
}

inline bool DataTypeDocumentModel::openSucceeded(void) const
{
    return mDTData.openSucceeded();
}

inline DataTypeDocumentData& DataTypeDocumentModel::getData(void)
{
    return mDTData;
}

inline const DataTypeDocumentData& DataTypeDocumentModel::getData(void) const
{
    return mDTData;
}

inline bool DataTypeDocumentModel::isDirty(void) const
{
    return (mUndoStack.isClean() == false);
}

inline DocModelNotifier& DataTypeDocumentModel::getNotifier(void)
{
    return mNotifier;
}

inline DocUndoStack& DataTypeDocumentModel::getUndoStack(void)
{
    return mUndoStack;
}

inline const DocUndoStack& DataTypeDocumentModel::getUndoStack(void) const
{
    return mUndoStack;
}

inline OverviewDataSection& DataTypeDocumentModel::getOverviewSection(void)
{
    return mDTData.getOverviewData();
}

inline DataTypeDataSection& DataTypeDocumentModel::getDataTypeSection(void)
{
    return mDTData.getDataTypeData();
}

inline IncludeDataSection& DataTypeDocumentModel::getIncludeSection(void)
{
    return mDTData.getIncludeData();
}

inline ConstantDataSection& DataTypeDocumentModel::getConstantSection(void)
{
    return mNoConstants;
}

inline AttributeDataSection& DataTypeDocumentModel::getAttributeSection(void)
{
    return mNoAttributes;
}

inline MethodDataSection& DataTypeDocumentModel::getMethodSection(void)
{
    return mNoMethods;
}

inline DocValidationController& DataTypeDocumentModel::getValidationController(void)
{
    return mValidation;
}

inline QString DataTypeDocumentModel::getDocumentPath(void) const
{
    return mDTData.getFilePath();
}

inline bool DataTypeDocumentModel::takesDataTypeImports(void) const
{
    return false;
}

inline void DataTypeDocumentModel::refreshTypeReferences(void)
{
    mDTData.getDataTypeData().refreshTypeReferences();
}

#endif  // LUSAN_MODEL_DT_DATATYPEDOCUMENTMODEL_HPP
