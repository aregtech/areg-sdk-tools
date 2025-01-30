﻿/************************************************************************
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

void SIMethod::onDeprecateChanged(const QString& newText)
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
    static const QString _defName("NewMethod");
    QTreeWidget* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (mModel.findMethod(name, SIMethodBase::eMethodType::MethodRequest) != nullptr);

    blockBasicSignals(true);
    
    QTreeWidgetItem* cur = table->currentItem();
    if (cur != nullptr)
    {
        cur->setSelected(false);
    }
    
    SIMethodBase * newMethod = mModel.createMethod(name, SIMethodBase::eMethodType::MethodRequest);
    Q_ASSERT(newMethod != nullptr);
    QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(table), newMethod);
    table->addTopLevelItem(item);
    item->setSelected(true);
    table->setCurrentItem(item);
    showMethodDetails(newMethod);
    blockBasicSignals(false);    
}

void SIMethod::onRemoveClicked(void)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>() : nullptr;
    if (item == nullptr)
        return;

    Q_ASSERT(item->data(1, Qt::ItemDataRole::UserRole).toUInt() == 0);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    item = id == 0 ? item : item->parent();

    int index = table->indexOfTopLevelItem(item);
    index = index + 1 == table->topLevelItemCount() ? index - 1 : index + 1;
    QTreeWidgetItem* next = (index >= 0) && (index < table->topLevelItemCount()) ? table->topLevelItem(index) : nullptr;
    table->setCurrentItem(next);
    item->setSelected(false);

    if (method->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
    {
        responseDeleted(static_cast<SIMethodResponse*>(method));
    }
    mModel.removeMethod(method);
    delete item;
    showMethodDetails(nullptr);
}

void SIMethod::onParamAddClicked(void)
{
    static const QString _defName("newParam");
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* cur = table->currentItem();
    Q_ASSERT(cur != nullptr);
    QTreeWidgetItem* parent = cur->parent();
    parent = parent == nullptr ? cur : parent;
    if (parent == nullptr)
        return;

    SIMethodBase* method = cur->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    uint32_t cnt{ 0 };
    QString name;
    do
    {
        name = _defName + QString::number(++cnt);
    } while (method->findElement(name) != nullptr);
    
    MethodParameter* param = mModel.addParameter(method, name);
    if (param != nullptr)
    {
        
        blockBasicSignals(true);
        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        setNodeText(item, param);
        item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
        item->setData(1, Qt::ItemDataRole::UserRole, param->getId());
        parent->addChild(item);
        cur->setSelected(false);
        table->setCurrentItem(item);
        item->setSelected(true);
        item->setExpanded(true);
        
        showParamDetails(method, *param);
        blockBasicSignals(false);
    }
}

void SIMethod::onParamRemoveClicked(void)
{
}

void SIMethod::onParamInsertClicked(void)
{
}

void SIMethod::onMoveUpClicked(void)
{
}

void SIMethod::onMoveDownClicked(void)
{
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
        Q_ASSERT(method != nullptr);
        Q_ASSERT(id == 0);
        showMethodDetails(method);
    }
    else
    {
        Q_ASSERT(method != nullptr);
        Q_ASSERT(id != 0);
        MethodParameter* param = method->findElement(id);
        Q_ASSERT(param != nullptr);
        showParamDetails(method, *param);
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
        QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(table), method);
        table->addTopLevelItem(item);
    }
}

void SIMethod::updateWidgets(void)
{
    
    showMethodDetails(nullptr);

    mList->ctrlButtonAdd()->setEnabled(true);
    mParams->ctrlParamType()->setModel(mParamTypes);
    mDetails->ctrlConnectedResponse()->setModel(mReplyModel);
}

void SIMethod::setupSignals(void)
{
    connect(mDetails->ctrlName()            , &QLineEdit::textChanged       , this, &SIMethod::onNameChanged);
    connect(mDetails->ctrlRequest()         , &QRadioButton::toggled        , this, &SIMethod::onRequestSelected);
    connect(mDetails->ctrlResponse()        , &QRadioButton::toggled        , this, &SIMethod::onResponseSelected);
    connect(mDetails->ctrlBroadcast()       , &QRadioButton::toggled        , this, &SIMethod::onBroadcastSelected);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled           , this, &SIMethod::onDeprecateChecked);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::textEdited        , this, &SIMethod::onDeprecateChanged);
    connect(mDetails->ctrlDescription()     , &QPlainTextEdit::textChanged  , this, &SIMethod::onDescriptionChanged);
    connect(mDetails->ctrlConnectedResponse(), &QComboBox::currentTextChanged, this, &SIMethod::onConnectedResponseChanged);
    
    connect(mList->ctrlButtonAdd()          , &QToolButton::clicked         , this, &SIMethod::onAddClicked);
    connect(mList->ctrlButtonRemove()       , &QToolButton::clicked         , this, &SIMethod::onRemoveClicked);
    connect(mList->ctrlButtonParamAdd()     , &QToolButton::clicked         , this, &SIMethod::onParamAddClicked);
    connect(mList->ctrlButtonParamRemove()  , &QToolButton::clicked         , this, &SIMethod::onParamRemoveClicked);
    connect(mList->ctrlButtonParamInsert()  , &QToolButton::clicked         , this, &SIMethod::onParamInsertClicked);
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
            QTreeWidgetItem* paramItem = new QTreeWidgetItem(item);
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
        mList->ctrlButtonParamInsert()->setEnabled(false);

        mDetails->ctrlName()->setText(method->getName());
        mDetails->ctrlDescription()->setPlainText(method->getDescription());

        SICommon::enableDeprecated<SIMethodDetails, SIMethodBase>(mDetails, method, true);

        switch (method->getMethodType())
        {
        case SIMethodBase::eMethodType::MethodRequest:
            mDetails->ctrlConnectedResponse()->setEnabled(true);
            mDetails->ctrlConnectedResponse()->setCurrentText(static_cast<SIMethodRequest *>(method)->getConectedResponseName());
            mDetails->ctrlRequest()->setChecked(true);
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
        mDetails->ctrlRequest()->setChecked(false);
        mDetails->ctrlRequest()->setEnabled(false);
        mDetails->ctrlResponse()->setEnabled(false);
        mDetails->ctrlBroadcast()->setEnabled(false);
        mDetails->ctrlConnectedResponse()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(false);
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

    return false;
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
        SIMethodBase * method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
        if (method->getMethodType() != SIMethodBase::eMethodType::MethodRequest)
            continue;
        
        if (static_cast<SIMethodRequest *>(method)->getConectedResponse() == response)
        {
            static_cast<SIMethodRequest *>(method)->connectResponse(nullptr);
            setNodeText(node, method);
        }
    }
}
