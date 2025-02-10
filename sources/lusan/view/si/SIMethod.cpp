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
 *  \file        lusan/view/si/SIMethod.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIMethod.hpp"
#include "ui/ui_SIMethod.h"

#include "lusan/view/si/SIMethodDetails.hpp"
#include "lusan/view/si/SIMethodParamDetails.hpp"
#include "lusan/view/si/SIMethodList.hpp"


#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/model/si/SIMethodModel.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/common/ReplyMethodModel.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "lusan/view/si/SICommon.hpp"

SIMethodWidget::SIMethodWidget(QWidget* parent)
    : QWidget{ parent }
    , ui(new Ui::SIMethod)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIMethod::SIMethod(SIMethodModel & model, QWidget* parent)
    : QScrollArea   (parent)
    , mModel        (model)
    , mDetails      (new SIMethodDetails(this))
    , mList         (new SIMethodList(this))
    , mParams       (new SIMethodParamDetails(this))
    , mWidget       (new SIMethodWidget(this))
    , mParamTypes   (new DataTypesModel(model.getDataTypeData(), false))
    , mReplyModel   (new ReplyMethodModel(model.getMethodData()))
    , ui            (*mWidget->ui)
    , mCount        (0)
{
    mParams->setHidden(true);

    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);
    ui.horizontalLayout->addWidget(mParams);
    
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    setWidgetResizable(true);
    setWidget(mWidget);

    setWidgetResizable(true);
    setWidget(mWidget);
    
    updateData();
    updateWidgets();
    setupSignals();
}

SIMethod::~SIMethod(void)
{
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
    ui.horizontalLayout->removeWidget(mParams);
}

void SIMethod::dataTypeCreated(DataTypeCustom* dataType)
{
    mParamTypes->dataTypeCreated(dataType);
}

void SIMethod::dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    mParamTypes->dataTypeConverted(oldType, newType);
    
    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* item = table->topLevelItem(i);
        if (item != nullptr)
        {
            SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
            Q_ASSERT(method != nullptr);
            if (method->isEmpty() == false)
            {
                int childCount = item->childCount();
                for (int j = 0; j < childCount; ++j)
                {
                    QTreeWidgetItem *child = item->child(j);
                    Q_ASSERT(child != nullptr);
                    uint32_t id = child->data(1, Qt::ItemDataRole::UserRole).toUInt();
                    MethodParameter* param = static_cast<SIMethodBase*>(method)->findElement(id);
                    if ((param != nullptr) && (param->getParamType() == oldType))
                    {
                        param->setParamType(newType);
                        setNodeText(child, param);
                    }
                }
            }
        }
    }
}

void SIMethod::dataTypeDeleted(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    mParamTypes->dataTypeDeleted(dataType);

    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* item = table->topLevelItem(i);
        if (item != nullptr)
        {
            SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
            Q_ASSERT(method != nullptr);
            if (method->isEmpty() == false)
            {
                int childCount = item->childCount();
                for (int j = 0; j < childCount; ++j)
                {
                    QTreeWidgetItem *child = item->child(j);
                    Q_ASSERT(child != nullptr);
                    uint32_t id = child->data(1, Qt::ItemDataRole::UserRole).toUInt();
                    MethodParameter* param = static_cast<SIMethodBase*>(method)->findElement(id);
                    if ((param != nullptr) && (param->getParamType() == dataType))
                    {
                        param->setParamType(nullptr);
                        setNodeText(child, param);
                    }
                }
            }
        }
    }
    
    blockBasicSignals(false);
}

void SIMethod::dataTypeUpdated(DataTypeCustom* dataType)
{
    blockBasicSignals(true);
    
    mParamTypes->dataTypeUpdated(dataType);
    QTreeWidget* table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    const QString& name  = dataType->getName();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* item = table->topLevelItem(i);
        if (item != nullptr)
        {
            SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
            Q_ASSERT(method != nullptr);
            if (method->isEmpty() == false)
            {
                int childCount = item->childCount();
                for (int j = 0; j < childCount; ++j)
                {
                    QTreeWidgetItem *child = item->child(j);
                    Q_ASSERT(child != nullptr);
                    uint32_t id = child->data(1, Qt::ItemDataRole::UserRole).toUInt();
                    MethodParameter* param = static_cast<SIMethodBase*>(method)->findElement(id);
                    if ((param != nullptr) && (param->getParamType() == dataType))
                    {
                        setNodeText(child, param);
                    }
                }
            }
        }
    }
    
    blockBasicSignals(false);
}

void SIMethod::onNameChanged(const QString& newName)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;

    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (method == nullptr)
        return;
    
    method->setName(newName);
    mReplyModel->methodUpdated(method);
    setNodeText(item, method);
    if (method->getMethodType() != SIMethodBase::eMethodType::MethodResponse)
        return;
        
    int childCount = table->topLevelItemCount();
    for (int i = 0; i < childCount; ++i)
    {
        QTreeWidgetItem* top = table->topLevelItem(i);
        SIMethodBase * entry = top->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
        if (entry->getMethodType() != SIMethodBase::eMethodType::MethodRequest)
            continue;
        
        SIMethodRequest* request = static_cast<SIMethodRequest *>(entry);
        if (method == static_cast<const SIMethodBase *>(request->getConectedResponse()))
            setNodeText(top, entry);
    }
}

void SIMethod::onRequestSelected(bool isSelected)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if ((item == nullptr) || (isSelected == false))
        return;

    SIMethodBase* oldMethod = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (oldMethod->getMethodType() == SIMethodBase::eMethodType::MethodRequest)
        return;

    SIMethodBase *newMethod = mModel.convertMethod(oldMethod, SIMethodBase::eMethodType::MethodRequest);
    Q_ASSERT(oldMethod != newMethod);
    static_cast<SIMethodRequest*>(newMethod)->connectResponse(nullptr);
    setNodeText(item, newMethod);
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    item->setData(1, Qt::ItemDataRole::UserRole, 0);

    int count = item->childCount();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* child = item->child(i);
        Q_ASSERT(child != nullptr);
        child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    }

    if (oldMethod->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
    {
        responseDeleted(static_cast<SIMethodResponse*>(oldMethod));
    }
    
    mReplyModel->methodConverted(oldMethod, newMethod);
    blockBasicSignals(true);
    showMethodDetails(newMethod);
    blockBasicSignals(false);
    delete oldMethod;
}

void SIMethod::onResponseSelected(bool isSelected)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if ((item == nullptr) || (isSelected == false))
        return;

    SIMethodBase* oldMethod = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (oldMethod->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
        return;

    SIMethodBase* newMethod = mModel.convertMethod(oldMethod, SIMethodBase::eMethodType::MethodResponse);
    Q_ASSERT(oldMethod != newMethod);
    setNodeText(item, newMethod);
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    item->setData(1, Qt::ItemDataRole::UserRole, 0);

    int count = item->childCount();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* child = item->child(i);
        Q_ASSERT(child != nullptr);
        child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    }

    mReplyModel->methodConverted(oldMethod, newMethod);
    blockBasicSignals(true);
    showMethodDetails(newMethod);
    blockBasicSignals(false);
    delete oldMethod;
}

void SIMethod::onBroadcastSelected(bool isSelected)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if ((item == nullptr) || (isSelected == false))
        return;

    SIMethodBase* oldMethod = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (oldMethod->getMethodType() == SIMethodBase::eMethodType::MethodBroadcast)
        return;

    SIMethodBase* newMethod = mModel.convertMethod(oldMethod, SIMethodBase::eMethodType::MethodBroadcast);
    Q_ASSERT(oldMethod != newMethod);
    setNodeText(item, newMethod);
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    item->setData(1, Qt::ItemDataRole::UserRole, 0);

    int count = item->childCount();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* child = item->child(i);
        Q_ASSERT(child != nullptr);
        child->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    }

    if (oldMethod->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
    {
        responseDeleted(static_cast<SIMethodResponse*>(oldMethod));
    }

    mReplyModel->methodConverted(oldMethod, newMethod);
    blockBasicSignals(true);
    showMethodDetails(newMethod);
    blockBasicSignals(false);
    delete oldMethod;
}

void SIMethod::onDeprecateChecked(bool isChecked)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>() : nullptr;
    if (method == nullptr)
        return;
    
    Q_ASSERT(item->data(1, Qt::ItemDataRole::UserRole).toUInt() == 0);
    SICommon::checkedDeprecated<SIMethodDetails, SIMethodBase>(mDetails, method, isChecked);
}

void SIMethod::onDeprecateHintChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>() : nullptr;
    if (method == nullptr)
        return;
   
    SICommon::setDeprecateHint<SIMethodDetails, SIMethodBase>(mDetails, method, newText);
}

void SIMethod::onDescriptionChanged(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>() : nullptr;
    if (method != nullptr)
    {
        method->setDescription(mDetails->ctrlDescription()->toPlainText());
    }
}

void SIMethod::onConnectedResponseChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>() : nullptr;
    if ((method != nullptr) && (method->getMethodType() == SIMethodBase::eMethodType::MethodRequest))
    {
        SIMethodResponse * response = newText.isEmpty() == false ? mReplyModel->findResponse(newText) : nullptr;
        static_cast<SIMethodRequest *>(method)->connectResponse(response);
        setNodeText(item, method);
    }
}

void SIMethod::onAddClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QString name = genName();

    blockBasicSignals(true);
    
    QTreeWidgetItem* cur = table->currentItem();
    if (cur != nullptr)
    {
        cur->setSelected(false);
    }
    
    SIMethodBase * newMethod = mModel.addMethod(name, SIMethodBase::eMethodType::MethodRequest);
    Q_ASSERT(newMethod != nullptr);
    int pos = table->topLevelItemCount();
    QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(), newMethod);
    table->addTopLevelItem(item);
    item->setSelected(true);
    table->setCurrentItem(item);
    showMethodDetails(newMethod);
    updateToolButtons(pos, pos + 1);
    blockBasicSignals(false);
}

void SIMethod::onInsertClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QString name = genName();
    
    blockBasicSignals(true);
    
    QTreeWidgetItem* cur = table->currentItem();
    
    uint32_t id = cur != nullptr ? cur->data(1, Qt::ItemDataRole::UserRole).toUInt() : 0;
    QTreeWidgetItem* top = id == 0 ? cur : cur->parent();
    int row = top != nullptr ? table->indexOfTopLevelItem(top) : 0;
    row = row < 0 ? 0 : row;
    SIMethodBase * newMethod = mModel.insertMethod(row, name, SIMethodBase::eMethodType::MethodRequest);
    Q_ASSERT(newMethod != nullptr);
    QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(), newMethod);
    table->insertTopLevelItem(row, item);
    if (cur != nullptr)
    {
        cur->setSelected(false);
    }

    item->setSelected(true);
    table->setCurrentItem(item);
    showMethodDetails(newMethod);
    updateToolButtons(row, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIMethod::onRemoveClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>() : nullptr;
    if (item == nullptr)
        return;

    blockBasicSignals(true);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    item = id == 0 ? item : item->parent();
    Q_ASSERT(item != nullptr);
    Q_ASSERT(item->data(1, Qt::ItemDataRole::UserRole).toUInt() == 0);

    int index = table->indexOfTopLevelItem(item);
    index = index + 1 == table->topLevelItemCount() ? index - 1 : index + 1;
    QTreeWidgetItem* next = (index >= 0) && (index < table->topLevelItemCount()) ? table->topLevelItem(index) : nullptr;
    SIMethodBase* nextMethod = next != nullptr ? next->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>(): nullptr;
    table->setCurrentItem(next);
    item->setSelected(false);

    if (method->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
    {
        responseDeleted(static_cast<SIMethodResponse*>(method));
    }

    mReplyModel->methodRemoved(method);
    mModel.removeMethod(method);
    delete item;

    int row = -1;
    int rowCount = 0;
    if (next != nullptr)
    {
        Q_ASSERT(nextMethod != nullptr);
        next->setSelected(true);
        row = table->indexOfTopLevelItem(next);
        rowCount = table->topLevelItemCount();
    }

    showMethodDetails(nextMethod);
    updateToolButtons(row, rowCount);

    blockBasicSignals(false);
}

void SIMethod::onParamAddClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* cur = table->currentItem();
    Q_ASSERT(cur != nullptr);
    QTreeWidgetItem* parent = cur->parent();
    parent = parent == nullptr ? cur : parent;
    if (parent == nullptr)
        return;

    SIMethodBase* method = cur->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    QString name = genName(method);
    
    MethodParameter* param = mModel.addParameter(method, name);
    if (param != nullptr)
    {
        blockBasicSignals(true);
        int pos = parent->childCount();
        QTreeWidgetItem* item = new QTreeWidgetItem();
        setNodeText(item, param);
        item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
        item->setData(1, Qt::ItemDataRole::UserRole, param->getId());
        parent->addChild(item);
        if (parent->isExpanded() == false)
        {
            parent->setExpanded(true);
        }
        cur->setSelected(false);
        table->setCurrentItem(item);
        item->setSelected(true);
        item->setExpanded(true);
        
        showParamDetails(method, *param);
        updateToolButtons(pos, pos + 1);
        blockBasicSignals(false);
    }
}

void SIMethod::onParamRemoveClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;
    
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
    Q_ASSERT(id != 0);
    Q_ASSERT(method != nullptr);
    
    QTreeWidgetItem* parent = item->parent();
    int index = parent->indexOfChild(item);
    index = index + 1 == parent->childCount() ? index - 1 : index;

    blockBasicSignals(true);
    item->setSelected(false);
    parent->removeChild(item);
    method->removeElement(id);

    QTreeWidgetItem* next = nullptr;
    int row = -1;
    int rowCount = 0;
    if ((index >= 0) && (index < parent->childCount()))
    {
        next = parent->child(index);
        row = index;
        rowCount = parent->childCount();
    }
    else
    {
        next = parent;
        row = table->indexOfTopLevelItem(next);
        rowCount = table->topLevelItemCount();
    }
    
    if (next != nullptr)
    {
        next->setSelected(true);
        table->setCurrentItem(next);
        uint32_t nextId = next->data(1, Qt::ItemDataRole::UserRole).toUInt();
        SIMethodBase* nextMethod = next->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
        if (nextId != 0)
        {
            MethodParameter* nextParam = nextMethod->findElement(nextId);
            Q_ASSERT(nextParam != nullptr);
            showParamDetails(nextMethod, *nextParam);
        }
        else
        {
            showMethodDetails(nextMethod);
        }
    }
    else
    {
        showMethodDetails(nullptr);
    }
    
    delete item;
    updateToolButtons(row, rowCount);
    blockBasicSignals(false);
}

void SIMethod::onParamInsertClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* cur = table->currentItem();
    Q_ASSERT(cur != nullptr);
    QTreeWidgetItem* parent = cur->parent();
    QTreeWidgetItem* child = parent != nullptr ? cur : nullptr;
    parent = parent == nullptr ? cur : parent;
    if (parent == nullptr)
        return;
    
    SIMethodBase* method = cur->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    QString name = genName(method);
    int row = child != nullptr ? parent->indexOfChild(child) : 0;
    row = row < 0 ? 0 : row;
    MethodParameter* param = mModel.insertParameter(method, row, name);
    if (param != nullptr)
    {
        blockBasicSignals(true);
        QTreeWidgetItem* item = new QTreeWidgetItem();
        setNodeText(item, param);
        item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
        item->setData(1, Qt::ItemDataRole::UserRole, param->getId());
        parent->insertChild(row, item);
        if (parent->isExpanded() == false)
        {
            parent->setExpanded(true);
        }
        cur->setSelected(false);
        table->setCurrentItem(item);
        item->setSelected(true);
        
        const QList<MethodParameter>& list = method->getElements();
        const int count { parent->childCount() };
        Q_ASSERT(count == list.size());
        for (int i = row + 1; i < count; ++i)
        {
            QTreeWidgetItem * temp = parent->child(i);
            Q_ASSERT(temp != nullptr);
            temp->setData(1, Qt::ItemDataRole::UserRole, list[i].getId());
        }
        
        showParamDetails(method, *param);
        updateToolButtons(row, count);
        blockBasicSignals(false);
    }
}

void SIMethod::onMoveUpClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;

    blockBasicSignals(true);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if (id == 0)
    {
        moveMethodUp(item);
    }
    else
    {
        moveMethodParamUp(item);
    }

    blockBasicSignals(false);
}

void SIMethod::onMoveDownClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;

    blockBasicSignals(true);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    if (id == 0)
    {
        moveMethodDown(item);
    }
    else
    {
        moveMethodParamDown(item);
    }

    blockBasicSignals(false);
}

void SIMethod::onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == nullptr)
    {
        showMethodDetails(nullptr);
        return;
    }
    
    SIMethodBase* method = current->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
    uint32_t id = current->data(1, Qt::ItemDataRole::UserRole).toUInt();
    QTreeWidgetItem* parent = current->parent();
    
    blockBasicSignals(true);
    if (parent == nullptr)
    {
        QTreeWidget* table = mList->ctrlTableList();
        Q_ASSERT(method != nullptr);
        Q_ASSERT(id == 0);
        showMethodDetails(method);
        updateToolButtons(table->indexOfTopLevelItem(current), table->topLevelItemCount());
    }
    else
    {
        Q_ASSERT(method != nullptr);
        Q_ASSERT(id != 0);
        MethodParameter* param = method->findElement(id);
        Q_ASSERT(param != nullptr);
        showParamDetails(method, *param);
        updateToolButtons(parent->indexOfChild(current), parent->childCount());
    }
    
    blockBasicSignals(false);
}

void SIMethod::onParamNameChanged(const QString& newText)
{
    
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    Q_ASSERT(item != nullptr);
    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
    Q_ASSERT(method != nullptr);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    MethodParameter* param = method->findElement(id);
    Q_ASSERT(param != nullptr);
    
    param->setName(newText);
    setNodeText(item, param);
}

void SIMethod::onParamTypeChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    Q_ASSERT(item != nullptr);
    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
    Q_ASSERT(method != nullptr);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    MethodParameter* param = method->findElement(id);
    Q_ASSERT(param != nullptr);
    DataTypeBase* dataType = mModel.getDataTypeData().findDataType(newText);
    Q_ASSERT(dataType != nullptr);
    param->setParamType(dataType);
    setNodeText(item, param);
}

void SIMethod::onParamDefaultChecked(bool isChecked)
{
}

void SIMethod::onParamDefaultChanged(const QString& newText)
{
}

void SIMethod::onParamDescriptionChanged(void)
{
}

void SIMethod::onParamDeprecateChecked(bool isChecked)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>() : nullptr;
    if (method == nullptr)
        return;

    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    MethodParameter * param = method->findElement(id);
    Q_ASSERT(param != nullptr);

    SICommon::checkedDeprecated<SIMethodParamDetails, MethodParameter>(mParams, param, isChecked);
}

void SIMethod::onParamDeprecateHintChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>() : nullptr;
    if (method == nullptr)
        return;
    
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    MethodParameter * param = method->findElement(id);
    SICommon::setDeprecateHint<SIMethodParamDetails, MethodParameter>(mParams, param, newText);
}

void SIMethod::updateData(void)
{
    mParamTypes->setFilter(QList<DataTypeBase::eCategory>{DataTypeBase::eCategory::BasicContainer});
    mParamTypes->updateDataTypeLists();
    mReplyModel->updateList();
    
    const QList<SIMethodBase*>& list = mModel.getMethodList();
    QTreeWidget* table = mList->ctrlTableList();
    table->clear();
    for (SIMethodBase* method : list)
    {
        QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(), method);
        table->addTopLevelItem(item);
    }

    mParams->ctrlParamType()->setModel(mParamTypes);
    mDetails->ctrlConnectedResponse()->setModel(mReplyModel);
}

void SIMethod::updateWidgets(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    table->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
    table->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Interactive);
    table->header()->setSectionResizeMode(2, QHeaderView::ResizeMode::Interactive);
    table->header()->setSectionResizeMode(3, QHeaderView::ResizeMode::Stretch);
    
    showMethodDetails(nullptr);
}

void SIMethod::setupSignals(void)
{
    connect(mDetails->ctrlName()            , &QLineEdit::textChanged       , this, &SIMethod::onNameChanged);
    connect(mDetails->ctrlRequest()         , &QRadioButton::toggled        , this, &SIMethod::onRequestSelected);
    connect(mDetails->ctrlResponse()        , &QRadioButton::toggled        , this, &SIMethod::onResponseSelected);
    connect(mDetails->ctrlBroadcast()       , &QRadioButton::toggled        , this, &SIMethod::onBroadcastSelected);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled           , this, &SIMethod::onDeprecateChecked);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::textEdited        , this, &SIMethod::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription()     , &QPlainTextEdit::textChanged  , this, &SIMethod::onDescriptionChanged);
    connect(mDetails->ctrlConnectedResponse(), &QComboBox::currentTextChanged, this, &SIMethod::onConnectedResponseChanged);

    connect(mList->ctrlButtonAdd()          , &QToolButton::clicked         , this, &SIMethod::onAddClicked);
    connect(mList->ctrlButtonInsert()       , &QToolButton::clicked         , this, &SIMethod::onInsertClicked);
    connect(mList->ctrlButtonRemove()       , &QToolButton::clicked         , this, &SIMethod::onRemoveClicked);
    connect(mList->ctrlButtonParamAdd()     , &QToolButton::clicked         , this, &SIMethod::onParamAddClicked);
    connect(mList->ctrlButtonParamInsert()  , &QToolButton::clicked         , this, &SIMethod::onParamInsertClicked);
    connect(mList->ctrlButtonParamRemove()  , &QToolButton::clicked         , this, &SIMethod::onParamRemoveClicked);
    connect(mList->ctrlButtonMoveUp()       , &QToolButton::clicked         , this, &SIMethod::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown()     , &QToolButton::clicked         , this, &SIMethod::onMoveDownClicked);
    connect(mList->ctrlTableList()          , &QTreeWidget::currentItemChanged, this, &SIMethod::onCurCellChanged);

    connect(mParams->ctrlParamName()        , &QLineEdit::textChanged       , this, &SIMethod::onParamNameChanged);
    connect(mParams->ctrlParamType()        , &QComboBox::currentTextChanged, this, &SIMethod::onParamTypeChanged);
    connect(mParams->ctrlParamHasDefault()  , &QCheckBox::toggled           , this, &SIMethod::onParamDefaultChecked);
    connect(mParams->ctrlParamDefaultValue(), &QLineEdit::textChanged       , this, &SIMethod::onParamDefaultChanged);
    connect(mParams->ctrlParamDescription() , &QPlainTextEdit::textChanged  , this, &SIMethod::onParamDescriptionChanged);
    connect(mParams->ctrlDeprecated()       , &QCheckBox::toggled           , this, &SIMethod::onParamDeprecateChecked);
    connect(mParams->ctrlDeprecateHint()    , &QLineEdit::textEdited        , this, &SIMethod::onParamDeprecateHintChanged);
}

void SIMethod::blockBasicSignals(bool doBlock)
{
    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlRequest()->blockSignals(doBlock);
    mDetails->ctrlResponse()->blockSignals(doBlock);
    mDetails->ctrlBroadcast()->blockSignals(doBlock);
    mDetails->ctrlConnectedResponse()->blockSignals(doBlock);
    
    mParams->ctrlParamName()->blockSignals(doBlock);
    mParams->ctrlParamType()->blockSignals(doBlock);
    mParams->ctrlParamHasDefaultValue()->blockSignals(doBlock);
    mParams->ctrlParamDefaultValue()->blockSignals(doBlock);

    mList->ctrlTableList()->blockSignals(doBlock);
}

QTreeWidgetItem* SIMethod::updateMethodNode(QTreeWidgetItem* item, SIMethodBase* method)
{
    setNodeText(item, method);
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
    item->setData(1, Qt::ItemDataRole::UserRole, 0);

    if (method->hasElements())
    {
        const QList<MethodParameter>& params = method->getElements();
        for (const MethodParameter& param : params)
        {
            QTreeWidgetItem* paramItem = new QTreeWidgetItem();
            setNodeText(paramItem, &param);
            paramItem->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
            paramItem->setData(1, Qt::ItemDataRole::UserRole, param.getId());
            item->addChild(paramItem);
        }
    }

    return item;
}

void SIMethod::showMethodDetails(SIMethodBase* method)
{
    mParams->hide();
    mDetails->setVisible(true);
    if (method != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlRequest()->setEnabled(true);
        mDetails->ctrlResponse()->setEnabled(true);
        mDetails->ctrlBroadcast()->setEnabled(true);
        mList->ctrlButtonRemove()->setEnabled(true);
        mList->ctrlButtonParamAdd()->setEnabled(true);
        mList->ctrlButtonParamRemove()->setEnabled(false);
        mList->ctrlButtonParamInsert()->setEnabled(true);

        mDetails->ctrlName()->setText(method->getName());
        mDetails->ctrlDescription()->setPlainText(method->getDescription());

        SICommon::enableDeprecated<SIMethodDetails, SIMethodBase>(mDetails, method, true);

        switch (method->getMethodType())
        {
        case SIMethodBase::eMethodType::MethodRequest:
            mDetails->ctrlConnectedResponse()->setEnabled(true);
            mDetails->ctrlRequest()->setChecked(true);
            if (static_cast<SIMethodRequest *>(method)->hasValidResponse())
            {
                mDetails->ctrlConnectedResponse()->setCurrentText(static_cast<SIMethodRequest *>(method)->getConectedResponseName());
            }
            else
            {
                mDetails->ctrlConnectedResponse()->setCurrentText(QString());
            }
            break;

        case SIMethodBase::eMethodType::MethodResponse:
            mDetails->ctrlConnectedResponse()->setEnabled(false);
            mDetails->ctrlConnectedResponse()->setCurrentText(QString());
            mDetails->ctrlResponse()->setChecked(true);
            break;

        case SIMethodBase::eMethodType::MethodBroadcast:
            mDetails->ctrlConnectedResponse()->setEnabled(false);
            mDetails->ctrlConnectedResponse()->setCurrentText(QString());
            mDetails->ctrlBroadcast()->setChecked(true);
            break;

        default:
            break;
        }

        mDetails->ctrlName()->setFocus();
        mDetails->ctrlName()->selectAll();
    }
    else
    {
        mDetails->ctrlName()->setText(QString());
        mDetails->ctrlName()->setEnabled(false);
        mDetails->ctrlRequest()->setChecked(false);
        mDetails->ctrlRequest()->setEnabled(false);
        mDetails->ctrlResponse()->setEnabled(false);
        mDetails->ctrlBroadcast()->setEnabled(false);
        mDetails->ctrlConnectedResponse()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(false);
        mList->ctrlButtonParamRemove()->setEnabled(false);
        mList->ctrlButtonParamInsert()->setEnabled(false);
        
        mDetails->ctrlConnectedResponse()->setCurrentText(QString());
        mDetails->ctrlDescription()->setPlainText(QString());
        SICommon::enableDeprecated<SIMethodDetails, SIMethodBase>(mDetails, nullptr, false);
    }
}

void SIMethod::showParamDetails(SIMethodBase* method, const MethodParameter& param)
{
    mDetails->hide();
    mParams->setVisible(true);
    if (method != nullptr)
    {
        mParams->ctrlParamName()->setEnabled(true);
        mParams->ctrlParamType()->setEnabled(true);
            
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(true);
        mList->ctrlButtonParamRemove()->setEnabled(true);
        mList->ctrlButtonParamInsert()->setEnabled(true);
        
        mParams->ctrlParamName()->setText(param.getName());
        mParams->ctrlParamType()->setCurrentText(param.getType());
        if (param.hasDefault())
        {
            mParams->ctrlParamHasDefaultValue()->setChecked(true);
            mParams->ctrlParamDefaultValue()->setText(param.getValue());
            mParams->ctrlParamDefaultValue()->setEnabled(true);
        }
        else
        {
            mParams->ctrlParamHasDefaultValue()->setChecked(false);
            mParams->ctrlParamDefaultValue()->setText(QString());
            mParams->ctrlParamDefaultValue()->setEnabled(false);
        }
        
        mParams->ctrlParamDescription()->setPlainText(param.getDescription());
        SICommon::enableDeprecated<SIMethodParamDetails, MethodParameter>(mParams, &param, true);
        mParams->ctrlParamName()->setFocus();
        mParams->ctrlParamName()->selectAll();        
    }
    else
    {
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(false);
        mList->ctrlButtonParamRemove()->setEnabled(false);
        mList->ctrlButtonParamInsert()->setEnabled(false);
        
        mParams->ctrlParamName()->setText(QString());
        mParams->ctrlParamType()->setCurrentText(QString());
        mParams->ctrlParamDefaultValue()->setText(QString());
        mParams->ctrlParamDescription()->setPlainText(QString());
        SICommon::enableDeprecated<SIMethodParamDetails, MethodParameter>(mParams, nullptr, false);
    }
}

bool SIMethod::getCurrentMethod(QTreeWidgetItem*& item, SIMethodBase*& method)
{
    bool result{ false };
    QTreeWidget* table = mList->ctrlTableList();
    item = table->currentItem();
    method = nullptr;
    if (item != nullptr)
    {
        method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
        result = (method != nullptr);
    }

    return result;
}

bool SIMethod::getCurrentParam(QTreeWidgetItem*& item, SIMethodBase*& method, MethodParameter*& param)
{
    bool result{ false };
    QTreeWidget* table = mList->ctrlTableList();
    item = table->currentItem();
    method = nullptr;
    param = nullptr;
    if (item != nullptr)
    {
        method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
        uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
        if (id != 0)
        {
            Q_ASSERT(item->parent() != nullptr);
            param = method->findElement(id);
            result = (param != nullptr);
        }
    }

    return result;
}

void SIMethod::setNodeText(QTreeWidgetItem* node, const ElementBase* elem)
{
    if ((node == nullptr) || (elem == nullptr))
        return;

    node->setIcon(0, elem->getIcon(ElementBase::eDisplay::DisplayName));
    node->setText(0, elem->getString(ElementBase::eDisplay::DisplayName));

    node->setIcon(1, elem->getIcon(ElementBase::eDisplay::DisplayType));
    node->setText(1, elem->getString(ElementBase::eDisplay::DisplayType));
    
    node->setIcon(2, elem->getIcon(ElementBase::eDisplay::DisplayValue));
    node->setText(2, elem->getString(ElementBase::eDisplay::DisplayValue));
    
    node->setIcon(3, elem->getIcon(ElementBase::eDisplay::DisplayLink));
    node->setText(3, elem->getString(ElementBase::eDisplay::DisplayLink));
}

void SIMethod::responseDeleted(SIMethodResponse* response)
{
    QTreeWidget *table = mList->ctrlTableList();
    int count = table->topLevelItemCount();
    for (int i = 0; i < count; ++i)
    {
        QTreeWidgetItem* node = table->topLevelItem(i);
        SIMethodBase * method = node->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
        if (method->getMethodType() != SIMethodBase::eMethodType::MethodRequest)
            continue;
        
        if (static_cast<SIMethodRequest *>(method)->getConectedResponse() == response)
        {
            static_cast<SIMethodRequest *>(method)->connectResponse(nullptr);
            setNodeText(node, method);
        }
    }
}

inline void SIMethod::moveMethodUp(QTreeWidgetItem* node)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(node);
    if (row > 0)
    {
        bool isExpanded = node->isExpanded();
        swapMethods(node, row, row - 1);
        node->setExpanded(isExpanded);
    }
}

inline void SIMethod::moveMethodParamUp(QTreeWidgetItem* node)
{
    QTreeWidgetItem* parent = node->parent();
    Q_ASSERT(parent != nullptr);
    int row = parent->indexOfChild(node);
    if (row > 0)
    {
        swapMethodParams(node, parent, row, row - 1);
    }
}

inline void SIMethod::moveMethodDown(QTreeWidgetItem* node)
{
    QTreeWidget* table = mList->ctrlTableList();
    int row = table->indexOfTopLevelItem(node);
    if ((row >= 0) && (row < (table->topLevelItemCount() - 1)))
    {
        bool isExpanded = node->isExpanded();
        swapMethods(node, row, row + 1);
        node->setExpanded(isExpanded);
    }
}

inline void SIMethod::moveMethodParamDown(QTreeWidgetItem* node)
{
    QTreeWidgetItem* parent = node->parent();
    Q_ASSERT(parent != nullptr);
    int row = parent->indexOfChild(node);
    if ((row >= 0) && (row < (parent->childCount() - 1)))
    {
        swapMethodParams(node, parent, row, row + 1);
    }
}

inline void SIMethod::swapMethods(QTreeWidgetItem* node, int row, int moveRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* nodeSecond = table->topLevelItem(moveRow);
    Q_ASSERT(nodeSecond != nullptr);
    SIMethodBase* first = node->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    SIMethodBase* second = nodeSecond->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    mModel.swapMethods(*first, *second);

    table->takeTopLevelItem(row);
    table->insertTopLevelItem(moveRow, node);
    table->setCurrentItem(node);
    nodeSecond->setSelected(false);
    node->setSelected(true);
    updateToolButtons(moveRow, table->topLevelItemCount());
}

inline void SIMethod::swapMethodParams(QTreeWidgetItem* node, QTreeWidgetItem* parent, int row, int moveRow)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* nodeSecond = parent->child(moveRow);
    uint32_t firstId = node->data(1, Qt::ItemDataRole::UserRole).toUInt();
    uint32_t secondId = nodeSecond->data(1, Qt::ItemDataRole::UserRole).toUInt();
    SIMethodBase* method = parent->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    Q_ASSERT(method != nullptr);
    mModel.swapMethodParams(*method, firstId, secondId);
    node->setData(1, Qt::ItemDataRole::UserRole, secondId);
    nodeSecond->setData(1, Qt::ItemDataRole::UserRole, firstId);
    parent->takeChild(row);
    parent->insertChild(moveRow, node);
    table->setCurrentItem(node);
    nodeSecond->setSelected(false);
    node->setSelected(true);
    updateToolButtons(moveRow, parent->childCount());
}

inline void SIMethod::updateToolButtons(int row, int rowCount)
{
    if ((row >= 0) && (row < rowCount))
    {
        if ((row == 0) && (rowCount == 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else if (row == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
        else if (row == (rowCount - 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else
        {
            mList->ctrlButtonMoveUp()->setEnabled(true);
            mList->ctrlButtonMoveDown()->setEnabled(true);
        }
    }
    else
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

inline QString SIMethod::genName(void)
{
    static const QString _defName("NewMethod");
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (mModel.findMethod(name, SIMethodBase::eMethodType::MethodRequest) != nullptr);

    return name;
}

inline QString SIMethod::genName(SIMethodBase* method)
{
    static const QString _defName("newParam");
    uint32_t cnt{ 0 };
    QString name;
    do
    {
        name = _defName + QString::number(++cnt);
    } while (method->findElement(name) != nullptr);

    return name;
}
