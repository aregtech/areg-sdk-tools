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
 *  \file        lusan/model/common/DataTypeModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Data Types page model shared by every document editor.
 *
 ************************************************************************/

#include "lusan/model/common/DataTypeModel.hpp"

#include "lusan/model/common/DocElementCommands.hpp"

#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/dt/DTDocumentCache.hpp"
#include "lusan/data/dt/DataTypeImportResolver.hpp"
#include "lusan/model/common/DocValidationController.hpp"

#include <QFileSystemWatcher>
#include <QObject>

namespace
{
    //!< Deprecation flag and hint committed as one undo step, matching the single user gesture
    //!< (toggling the checkbox resets the hint).
    struct DeprecationState
    {
        bool    flag { false };
        QString hint { };
    };

    //!< A basic-container object and the key it leaves behind, committed as one undo step:
    //!< setContainer() clears the key as a side effect, so undo must restore both together.
    struct ContainerObjectState
    {
        QString container { };
        QString key { };
    };
}

DataTypeModel::DataTypeModel(IEDocumentModel& document)
    : QObject     ( )
    , mDocument   (document)
    , mImportWatch(new QFileSystemWatcher(this))
{
    // The include list decides which data type documents this one reads types out of, so an
    // include added, dropped or repointed changes the registry as surely as a type would.
    auto onIncludesChanged = [this](uint32_t, eDocElementKind kind)
    {
        if (kind == eDocElementKind::Include)
        {
            refreshImports();
        }
    };

    connect(mImportWatch, &QFileSystemWatcher::fileChanged, this, [this](const QString& path)
    {
        // The file on disk is not what was parsed any more, whoever wrote it.
        DTDocumentCache::getInstance().invalidate(path);
        refreshImports();
    });

    DocModelNotifier& notifier = mDocument.getNotifier();
    auto onTypesChanged = [this](uint32_t, eDocElementKind kind)
    {
        if (kind == eDocElementKind::DataType)
        {
            types().refreshTypeReferences();
        }
    };

    connect(&notifier, &DocModelNotifier::elementAdded  , this, onTypesChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, onTypesChanged);
    connect(&notifier, &DocModelNotifier::elementChanged, this, onTypesChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, onIncludesChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, onIncludesChanged);
    connect(&notifier, &DocModelNotifier::elementChanged, this, onIncludesChanged);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, [this]()
    {
        types().refreshTypeReferences();
        refreshImports();
    });

    // The document was read before this model was built, so the groups are already there; only
    // the watch on them is not.
    rearmImportWatch();
}

const QList<DataTypeDataSection::ImportedTypes>& DataTypeModel::getImports() const
{
    return types().getImports();
}

void DataTypeModel::refreshImports()
{
    if (mDocument.takesDataTypeImports() == false)
    {
        return;
    }

    if (DataTypeImportResolver::refresh(types(), mDocument.getDocumentPath(), mDocument.getIncludeSection()) == false)
    {
        return;
    }

    mDocument.refreshTypeReferences();
    rearmImportWatch();

    emit importsChanged();
    mDocument.getValidationController().scheduleValidation();
}

void DataTypeModel::rearmImportWatch()
{
    const QStringList watched = mImportWatch->files();
    if (watched.isEmpty() == false)
    {
        mImportWatch->removePaths(watched);
    }

    const QStringList paths = DataTypeImportResolver::resolvedPaths(types());
    if (paths.isEmpty() == false)
    {
        mImportWatch->addPaths(paths);
    }
}

const DataTypeDataSection& DataTypeModel::getDataTypeData() const
{
    return types();
}

DataTypeDataSection& DataTypeModel::getDataTypeData()
{
    return types();
}

const QList<DataTypeCustom*>& DataTypeModel::getCustomDataTypes() const
{
    return types().getCustomDataTypes();
}

int DataTypeModel::getDataTypeCount() const
{
    return static_cast<int>(types().getCustomDataTypes().size());
}

DataTypeCustom* DataTypeModel::findDataType(const QString& name) const
{
    return types().findCustomDataType(name);
}

DataTypeCustom* DataTypeModel::findDataType(uint32_t id) const
{
    return types().findCustomDataType(id);
}

int DataTypeModel::findIndex(uint32_t id) const
{
    return types().findIndex(id);
}

int DataTypeModel::findIndex(const DataTypeCustom* dataType) const
{
    return (dataType != nullptr ? types().findIndex(dataType->getId()) : -1);
}

const QList<FieldEntry>& DataTypeModel::getStructChildren(const DataTypeStructure* dataType) const
{
    static const QList<FieldEntry> _empty;
    return (dataType != nullptr ? dataType->getElements() : _empty);
}

const QList<EnumEntry>& DataTypeModel::getEnumChildren(const DataTypeEnum* dataType) const
{
    static const QList<EnumEntry> _empty;
    return (dataType != nullptr ? dataType->getElements() : _empty);
}

ElementBase* DataTypeModel::findChild(const DataTypeCustom* dataType, uint32_t childId) const
{
    if (dataType == nullptr)
        return nullptr;

    switch (dataType->getCategory())
    {
    case DataTypeBase::eCategory::Structure:
        return static_cast<const DataTypeStructure*>(dataType)->findElement(childId);
    case DataTypeBase::eCategory::Enumeration:
        return static_cast<const DataTypeEnum*>(dataType)->findElement(childId);
    default:
        return nullptr;
    }
}

int DataTypeModel::findChildIndex(const DataTypeCustom* dataType, uint32_t childId) const
{
    if (dataType == nullptr)
        return -1;

    switch (dataType->getCategory())
    {
    case DataTypeBase::eCategory::Structure:
        return static_cast<const DataTypeStructure*>(dataType)->findIndex(childId);
    case DataTypeBase::eCategory::Enumeration:
        return static_cast<const DataTypeEnum*>(dataType)->findIndex(childId);
    default:
        return -1;
    }
}

int DataTypeModel::findChildIndex(const DataTypeCustom* dataType, const QString& childName) const
{
    if (dataType == nullptr)
        return -1;

    switch (dataType->getCategory())
    {
    case DataTypeBase::eCategory::Structure:
        return static_cast<const DataTypeStructure*>(dataType)->findIndex(childName);
    case DataTypeBase::eCategory::Enumeration:
        return static_cast<const DataTypeEnum*>(dataType)->findIndex(childName);
    default:
        return -1;
    }
}

int DataTypeModel::getChildCount(const DataTypeCustom* dataType) const
{
    if (dataType == nullptr)
        return 0;

    switch (dataType->getCategory())
    {
    case DataTypeBase::eCategory::Structure:
        return static_cast<const DataTypeStructure*>(dataType)->getElementCount();
    case DataTypeBase::eCategory::Enumeration:
        return static_cast<const DataTypeEnum*>(dataType)->getElementCount();
    default:
        return 0;
    }
}

DocModelNotifier& DataTypeModel::getNotifier() const
{
    return mDocument.getNotifier();
}

IEDocumentModel& DataTypeModel::getDocument() const
{
    return mDocument;
}

DataTypeCustom* DataTypeModel::createDataType(const QString& name, DataTypeBase::eCategory category)
{
    if (findDataType(name) != nullptr)
        return nullptr;

    DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(category);
    if (dataType == nullptr)
        return nullptr;

    dataType->setName(name);
    mDocument.getUndoStack().push(new TDocAddCommand<DataTypeCustom*, DocumentElem>(getNotifier(), types(), dataType, eDocElementKind::DataType, QObject::tr("Add data type")));
    return dataType;
}

DataTypeCustom* DataTypeModel::insertDataType(int position, const QString& name, DataTypeBase::eCategory category)
{
    if (findDataType(name) != nullptr)
        return nullptr;

    DataTypeCustom* dataType = DataTypeFactory::createCustomDataType(category);
    if (dataType == nullptr)
        return nullptr;

    dataType->setName(name);
    mDocument.getUndoStack().push(buildInsertCommand<DataTypeCustom*, DocumentElem>(getNotifier(), types(), dataType, position, 0u, eDocElementKind::DataType, QObject::tr("Insert data type")));
    return dataType;
}

void DataTypeModel::deleteDataType(DataTypeCustom* dataType)
{
    if (dataType == nullptr)
        return;

    mDocument.getUndoStack().push(new TDocRemoveCommand<DataTypeCustom*, DocumentElem>(getNotifier(), types(), dataType->getId(), eDocElementKind::DataType, QObject::tr("Delete data type")));
}

DataTypeCustom* DataTypeModel::convertDataType(DataTypeCustom* dataType, DataTypeBase::eCategory category)
{
    if ((dataType == nullptr) || (dataType->getCategory() == category))
        return dataType;

    DataTypeCustom* newType = DataTypeFactory::createCustomDataType(category);
    if (newType == nullptr)
        return dataType;

    newType->setName(dataType->getName());
    newType->setDescription(dataType->getDescription());
    newType->setIsDeprecated(dataType->getIsDeprecated(), dataType->getDeprecateHint());

    const int index = types().findIndex(dataType->getId());
    const int appendIndexAfterRemove = types().getElementCount() - 1;
    DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), QObject::tr("Convert data type"));
    new TDocRemoveCommand<DataTypeCustom*, DocumentElem>(getNotifier(), types(), dataType->getId(), eDocElementKind::DataType, QObject::tr("Convert data type"), composite);
    buildInsertCommandAt<DataTypeCustom*, DocumentElem>(getNotifier(), types(), newType, index, appendIndexAfterRemove, 0u, eDocElementKind::DataType, QObject::tr("Convert data type"), composite);
    mDocument.getUndoStack().push(composite);
    return newType;
}

void DataTypeModel::swapDataTypes(uint32_t firstId, uint32_t secondId)
{
    const int index1 = types().findIndex(firstId);
    const int index2 = types().findIndex(secondId);
    if ((index1 < 0) || (index2 < 0))
        return;

    mDocument.getUndoStack().push(new TDocReorderCommand<DataTypeCustom*, DocumentElem>(getNotifier(), types(), index1, index2, 0u, eDocElementKind::DataType, QObject::tr("Reorder data types")));
}

void DataTypeModel::renameDataType(DataTypeCustom* dataType, const QString& newName)
{
    if ((dataType == nullptr) || (newName == dataType->getName()))
        return;

    const uint32_t id = dataType->getId();
    const QString oldName{ dataType->getName() };
    auto getter = [dataType]() -> QString { return dataType->getName(); };
    auto setter = [dataType](const QString& value) { dataType->setName(value); };

    const QString text{ QObject::tr("Rename data type") };
    DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
    new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, newName, text, composite);
    // A document that reaches its data types by name repairs those references in the same step.
    mDocument.createRenameSideEffects(eDocElementKind::DataType, id, oldName, newName, composite);
    mDocument.getUndoStack().push(composite);
}

void DataTypeModel::setDescription(DataTypeCustom* dataType, const QString& text)
{
    if ((dataType == nullptr) || (text == dataType->getDescription()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getDescription(); };
    auto setter = [dataType](const QString& value) { dataType->setDescription(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, text, QObject::tr("Set description")));
}

void DataTypeModel::setDeprecated(DataTypeCustom* dataType, bool deprecated)
{
    if (dataType == nullptr)
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> DeprecationState { return DeprecationState{ dataType->getIsDeprecated(), dataType->getDeprecateHint() }; };
    auto setter = [dataType](const DeprecationState& value) { dataType->setIsDeprecated(value.flag, value.hint); };
    const DeprecationState next{ deprecated, deprecated ? dataType->getDeprecateHint() : QString() };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), id, eDocElementKind::DataType, getter, setter, next, QObject::tr("Set deprecated")));
}

void DataTypeModel::setDeprecateHint(DataTypeCustom* dataType, const QString& hint)
{
    if ((dataType == nullptr) || (dataType->getIsDeprecated() == false) || (hint == dataType->getDeprecateHint()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getDeprecateHint(); };
    auto setter = [dataType](const QString& value) { dataType->setDeprecateHint(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, hint, QObject::tr("Set deprecation hint")));
}

void DataTypeModel::setEnumDerived(DataTypeEnum* dataType, const QString& derived)
{
    if ((dataType == nullptr) || (derived == dataType->getDerived()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getDerived(); };
    auto setter = [dataType](const QString& value) { dataType->setDerived(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, derived, QObject::tr("Set enumeration derived type")));
}

void DataTypeModel::setImportLocation(DataTypeImported* dataType, const QString& location)
{
    if ((dataType == nullptr) || (location == dataType->getLocation()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getLocation(); };
    auto setter = [dataType](const QString& value) { dataType->setLocation(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, location, QObject::tr("Set import location")));
}

void DataTypeModel::setImportNamespace(DataTypeImported* dataType, const QString& space)
{
    if ((dataType == nullptr) || (space == dataType->getNamespace()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getNamespace(); };
    auto setter = [dataType](const QString& value) { dataType->setNamespace(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, space, QObject::tr("Set import namespace")));
}

void DataTypeModel::setImportObject(DataTypeImported* dataType, const QString& object)
{
    if ((dataType == nullptr) || (object == dataType->getObject()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getObject(); };
    auto setter = [dataType](const QString& value) { dataType->setObject(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, object, QObject::tr("Set import object")));
}

void DataTypeModel::setImportQualifiedName(DataTypeImported* dataType, const QString& qualified)
{
    if (dataType == nullptr)
        return;

    const int pos = qualified.lastIndexOf(QStringLiteral("::"));
    const QString space { pos >= 0 ? qualified.left(pos) : QString() };
    const QString object{ pos >= 0 ? qualified.mid(pos + 2) : qualified };
    if ((space == dataType->getNamespace()) && (object == dataType->getObject()))
        return;

    const uint32_t id = dataType->getId();
    const QString text{ QObject::tr("Set imported type") };
    DocCompositeCommand* composite = new DocCompositeCommand(getNotifier(), text);
    {
        auto getter = [dataType]() -> QString { return dataType->getNamespace(); };
        auto setter = [dataType](const QString& value) { dataType->setNamespace(value); };
        new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, space, text, composite);
    }
    {
        auto getter = [dataType]() -> QString { return dataType->getObject(); };
        auto setter = [dataType](const QString& value) { dataType->setObject(value); };
        new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, object, text, composite);
    }

    mDocument.getUndoStack().push(composite);
}

void DataTypeModel::setContainerObject(DataTypeContainer* dataType, const QString& basicName)
{
    if ((dataType == nullptr) || (basicName == dataType->getContainer()))
        return;

    // Reproduce setContainer()'s own default/clear-key side effect on a scratch copy, so the
    // recorded "new" state matches exactly what the real setter will produce.
    DataTypeContainer probe(*dataType);
    probe.setContainer(basicName);
    const QString nextKey = probe.getKey();

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> ContainerObjectState { return ContainerObjectState{ dataType->getContainer(), dataType->getKey() }; };
    auto setter = [dataType](const ContainerObjectState& value) { dataType->setContainer(value.container); dataType->setKey(value.key); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<ContainerObjectState>(getNotifier(), id, eDocElementKind::DataType, getter, setter, ContainerObjectState{ basicName, nextKey }, QObject::tr("Set container object")));
}

void DataTypeModel::setContainerKey(DataTypeContainer* dataType, const QString& typeName)
{
    if ((dataType == nullptr) || (typeName == dataType->getKey()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getKey(); };
    auto setter = [dataType](const QString& value) { dataType->setKey(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, typeName, QObject::tr("Set container key type")));
}

void DataTypeModel::setContainerValue(DataTypeContainer* dataType, const QString& typeName)
{
    if ((dataType == nullptr) || (typeName == dataType->getValue()))
        return;

    const uint32_t id = dataType->getId();
    auto getter = [dataType]() -> QString { return dataType->getValue(); };
    auto setter = [dataType](const QString& value) { dataType->setValue(value); };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), id, eDocElementKind::DataType, getter, setter, typeName, QObject::tr("Set container value type")));
}

ElementBase* DataTypeModel::createField(DataTypeCustom* dataType, const QString& name)
{
    if (dataType == nullptr)
        return nullptr;

    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        DataTypeStructure* structType = static_cast<DataTypeStructure*>(dataType);
        FieldEntry field(0, name, structType);
        // Resolve the type pointer before the command's redo() fires the notifier -- a page
        // rebuilds its row synchronously inside push(), before this function regains control.
        field.validate(types().getResolutionTypes());
        mDocument.getUndoStack().push(new TDocAddCommand<FieldEntry, DataTypeCustom>(getNotifier(), *structType, field, eDocElementKind::DataType, QObject::tr("Add field")));
        return structType->findElement(name);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        DataTypeEnum* enumType = static_cast<DataTypeEnum*>(dataType);
        EnumEntry field(0, name, QString(), enumType);
        mDocument.getUndoStack().push(new TDocAddCommand<EnumEntry, DataTypeCustom>(getNotifier(), *enumType, field, eDocElementKind::DataType, QObject::tr("Add field")));
        return enumType->findElement(name);
    }

    return nullptr;
}

ElementBase* DataTypeModel::insertField(DataTypeCustom* dataType, int position, const QString& name)
{
    if (dataType == nullptr)
        return nullptr;

    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        DataTypeStructure* structType = static_cast<DataTypeStructure*>(dataType);
        FieldEntry field(0, name, structType);
        field.validate(types().getResolutionTypes());
        mDocument.getUndoStack().push(buildInsertCommand<FieldEntry, DataTypeCustom>(getNotifier(), *structType, field, position, dataType->getId(), eDocElementKind::DataType, QObject::tr("Insert field")));
        return structType->findElement(name);
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        DataTypeEnum* enumType = static_cast<DataTypeEnum*>(dataType);
        EnumEntry field(0, name, QString(), enumType);
        mDocument.getUndoStack().push(buildInsertCommand<EnumEntry, DataTypeCustom>(getNotifier(), *enumType, field, position, dataType->getId(), eDocElementKind::DataType, QObject::tr("Insert field")));
        return enumType->findElement(name);
    }

    return nullptr;
}

void DataTypeModel::deleteField(DataTypeCustom* dataType, uint32_t fieldId)
{
    if (dataType == nullptr)
        return;

    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        mDocument.getUndoStack().push(new TDocRemoveCommand<FieldEntry, DataTypeCustom>(getNotifier(), *static_cast<DataTypeStructure*>(dataType), fieldId, eDocElementKind::DataType, QObject::tr("Delete field")));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        mDocument.getUndoStack().push(new TDocRemoveCommand<EnumEntry, DataTypeCustom>(getNotifier(), *static_cast<DataTypeEnum*>(dataType), fieldId, eDocElementKind::DataType, QObject::tr("Delete field")));
    }
}

void DataTypeModel::swapFields(DataTypeCustom* dataType, uint32_t firstId, uint32_t secondId)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        DataTypeStructure* structType = static_cast<DataTypeStructure*>(dataType);
        const int index1 = structType->findIndex(firstId);
        const int index2 = structType->findIndex(secondId);
        if ((index1 >= 0) && (index2 >= 0))
        {
            mDocument.getUndoStack().push(new TDocReorderCommand<FieldEntry, DataTypeCustom>(getNotifier(), *structType, index1, index2, ownerId, eDocElementKind::DataType, QObject::tr("Reorder fields")));
        }
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        DataTypeEnum* enumType = static_cast<DataTypeEnum*>(dataType);
        const int index1 = enumType->findIndex(firstId);
        const int index2 = enumType->findIndex(secondId);
        if ((index1 >= 0) && (index2 >= 0))
        {
            mDocument.getUndoStack().push(new TDocReorderCommand<EnumEntry, DataTypeCustom>(getNotifier(), *enumType, index1, index2, ownerId, eDocElementKind::DataType, QObject::tr("Reorder fields")));
        }
    }
}

void DataTypeModel::setFieldName(DataTypeCustom* dataType, uint32_t fieldId, const QString& name)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        auto getter = [dataType, fieldId]() -> QString { FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); return (f != nullptr ? f->getName() : QString()); };
        auto setter = [dataType, fieldId](const QString& value) { FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); if (f != nullptr) f->setName(value); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, name, QObject::tr("Rename field")));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        auto getter = [dataType, fieldId]() -> QString { EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); return (f != nullptr ? f->getName() : QString()); };
        auto setter = [dataType, fieldId](const QString& value) { EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); if (f != nullptr) f->setName(value); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, name, QObject::tr("Rename field")));
    }
}

void DataTypeModel::setFieldType(DataTypeStructure* dataType, uint32_t fieldId, const QString& typeName)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    auto getter = [dataType, fieldId]() -> QString { FieldEntry* f = dataType->findElement(fieldId); return (f != nullptr ? f->getType() : QString()); };
    auto setter = [this, dataType, fieldId](const QString& value) { FieldEntry* f = dataType->findElement(fieldId); if (f != nullptr) { f->setType(value); f->validate(types().getResolutionTypes()); } };
    mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, typeName, QObject::tr("Set field type")));
}

void DataTypeModel::setFieldValue(DataTypeCustom* dataType, uint32_t fieldId, const QString& value)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        auto getter = [dataType, fieldId]() -> QString { FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); return (f != nullptr ? f->getValue() : QString()); };
        auto setter = [dataType, fieldId](const QString& val) { FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); if (f != nullptr) f->setValue(val); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, value, QObject::tr("Set field value")));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        auto getter = [dataType, fieldId]() -> QString { EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); return (f != nullptr ? f->getValue() : QString()); };
        auto setter = [dataType, fieldId](const QString& val) { EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); if (f != nullptr) f->setValue(val); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, value, QObject::tr("Set field value")));
    }
}

void DataTypeModel::setFieldDescription(DataTypeCustom* dataType, uint32_t fieldId, const QString& text)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        auto getter = [dataType, fieldId]() -> QString { FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); return (f != nullptr ? f->getDescription() : QString()); };
        auto setter = [dataType, fieldId](const QString& val) { FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); if (f != nullptr) f->setDescription(val); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, text, QObject::tr("Set field description")));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        auto getter = [dataType, fieldId]() -> QString { EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); return (f != nullptr ? f->getDescription() : QString()); };
        auto setter = [dataType, fieldId](const QString& val) { EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); if (f != nullptr) f->setDescription(val); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, text, QObject::tr("Set field description")));
    }
}

void DataTypeModel::setFieldDeprecated(DataTypeCustom* dataType, uint32_t fieldId, bool deprecated)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        auto getter = [dataType, fieldId]() -> DeprecationState
        {
            FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId);
            return (f != nullptr ? DeprecationState{ f->getIsDeprecated(), f->getDeprecateHint() } : DeprecationState{});
        };
        auto setter = [dataType, fieldId](const DeprecationState& value)
        {
            FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId);
            if (f != nullptr) { f->setIsDeprecated(value.flag); f->setDeprecateHint(value.hint); }
        };
        FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId);
        const DeprecationState next{ deprecated, (deprecated && (f != nullptr)) ? f->getDeprecateHint() : QString() };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, next, QObject::tr("Set field deprecated")));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        auto getter = [dataType, fieldId]() -> DeprecationState
        {
            EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId);
            return (f != nullptr ? DeprecationState{ f->getIsDeprecated(), f->getDeprecateHint() } : DeprecationState{});
        };
        auto setter = [dataType, fieldId](const DeprecationState& value)
        {
            EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId);
            if (f != nullptr) { f->setIsDeprecated(value.flag); f->setDeprecateHint(value.hint); }
        };
        EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId);
        const DeprecationState next{ deprecated, (deprecated && (f != nullptr)) ? f->getDeprecateHint() : QString() };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<DeprecationState>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, next, QObject::tr("Set field deprecated")));
    }
}

void DataTypeModel::setFieldDeprecateHint(DataTypeCustom* dataType, uint32_t fieldId, const QString& hint)
{
    if (dataType == nullptr)
        return;

    const uint32_t ownerId = dataType->getId();
    if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
    {
        FieldEntry* f = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId);
        if ((f == nullptr) || (f->getIsDeprecated() == false))
            return;

        auto getter = [dataType, fieldId]() -> QString { FieldEntry* fe = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); return (fe != nullptr ? fe->getDeprecateHint() : QString()); };
        auto setter = [dataType, fieldId](const QString& value) { FieldEntry* fe = static_cast<DataTypeStructure*>(dataType)->findElement(fieldId); if (fe != nullptr) fe->setDeprecateHint(value); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, hint, QObject::tr("Set field deprecation hint")));
    }
    else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
    {
        EnumEntry* f = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId);
        if ((f == nullptr) || (f->getIsDeprecated() == false))
            return;

        auto getter = [dataType, fieldId]() -> QString { EnumEntry* fe = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); return (fe != nullptr ? fe->getDeprecateHint() : QString()); };
        auto setter = [dataType, fieldId](const QString& value) { EnumEntry* fe = static_cast<DataTypeEnum*>(dataType)->findElement(fieldId); if (fe != nullptr) fe->setDeprecateHint(value); };
        mDocument.getUndoStack().push(new TDocSetPropertyCommand<QString>(getNotifier(), ownerId, eDocElementKind::DataType, getter, setter, hint, QObject::tr("Set field deprecation hint")));
    }
}
