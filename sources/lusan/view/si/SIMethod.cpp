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
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIMethod.h"

#include "lusan/view/si/SIMethodDetails.hpp"
#include "lusan/view/si/SIMethodParamDetails.hpp"
#include "lusan/view/si/SIMethodList.hpp"

#include "lusan/data/si/SIMethodBroadcast.hpp"
#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/model/si/SIMethodModel.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QIcon>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>


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

void SIMethod::onNameChanged(const QString& newName)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    if (item == nullptr)
        return;

    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (method != nullptr)
    {
        method->setName(newName);
        item->setText(0, newName);
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
    item->setText(0, newMethod->getName());
    item->setIcon(0, QIcon());
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    item->setText(3, newMethod->getMethodType() == SIMethodBase::eMethodType::MethodRequest ? static_cast<SIMethodRequest*>(newMethod)->getConectedResponseName() : QString());
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
        SIMethodResponse* resp = static_cast<SIMethodRequest *>(oldMethod)->getConectedResponse();
        int index = mDetails->ctrlConnectedResponse()->findData(QVariant::fromValue(resp),Qt::ItemDataRole::UserRole);
        mDetails->ctrlConnectedResponse()->removeItem(index);
        QList<SIMethodRequest*> requests = mModel.getConnectedRequests(resp);
        for (auto request : requests)
        {
            request->connectResponse(static_cast<SIMethodResponse*>(nullptr));
        }
    }
    
    blockBasicSignals(true);
    selectRequest(static_cast<SIMethodRequest*>(newMethod));
    blockBasicSignals(false);
    delete oldMethod;
}

void SIMethod::onResponseSelected(bool isSelected)
{
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
    item->setText(0, newMethod->getName());
    item->setIcon(0, QIcon());
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(newMethod));
    item->setText(3, QString());
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
        SIMethodResponse* resp = static_cast<SIMethodRequest*>(oldMethod)->getConectedResponse();
        int index = mDetails->ctrlConnectedResponse()->findData(QVariant::fromValue(resp), Qt::ItemDataRole::UserRole);
        mDetails->ctrlConnectedResponse()->removeItem(index);
    }

    blockBasicSignals(true);
    selectBroadcast(static_cast<SIMethodBroadcast*>(newMethod));
    blockBasicSignals(false);
    delete oldMethod;
}

void SIMethod::onConnectedResponseChanged(int index)
{
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
    selectRequest(static_cast<SIMethodRequest *>(newMethod));
    blockBasicSignals(false);    
}

void SIMethod::onRemoveClicked(void)
{
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

        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        item->setText(0, param->getName());
        item->setIcon(0, QIcon());
        item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
        item->setText(1, param->getType());
        item->setData(1, Qt::ItemDataRole::UserRole, param->getId());
        item->setText(2, param->getValue());
        parent->addChild(item);
        cur->setSelected(false);
        item->setSelected(true);
        item->setExpanded(true);
        table->setCurrentItem(item);
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
}

void SIMethod::updateData(void)
{
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
    const QList<SIMethodResponse*>& responses = mModel.getResponseMethods();
    QComboBox* combo = mDetails->ctrlConnectedResponse();
    combo->clear();
    if (responses.isEmpty() == false)
    {
        combo->addItem(QString(), QVariant::fromValue(static_cast<SIMethodResponse*>(nullptr)));
        for (SIMethodResponse* response : responses)
        {
            combo->addItem(response->getName(), QVariant::fromValue(response));
        }
    }

    showParamDetails(false);
    showMethodDetails(true);

    mDetails->ctrlName()->setEnabled(false);
    mDetails->ctrlRequest()->setEnabled(false);
    mDetails->ctrlResponse()->setEnabled(false);
    mDetails->ctrlBroadcast()->setEnabled(false);
    mDetails->ctrlConnectedResponse()->setEnabled(false);
    mList->ctrlButtonMoveUp()->setEnabled(false);
    mList->ctrlButtonMoveDown()->setEnabled(false);
    mList->ctrlButtonRemove()->setEnabled(false);
    mList->ctrlButtonParamAdd()->setEnabled(false);
    mList->ctrlButtonParamRemove()->setEnabled(false);
    mList->ctrlButtonParamInsert()->setEnabled(false);

    mList->ctrlButtonAdd()->setEnabled(true);
}

void SIMethod::setupSignals(void)
{
    connect(mDetails->ctrlName(), &QLineEdit::textChanged, this, &SIMethod::onNameChanged);
    connect(mDetails->ctrlRequest(), &QRadioButton::toggled, this, &SIMethod::onRequestSelected);
    connect(mDetails->ctrlResponse(), &QRadioButton::toggled, this, &SIMethod::onResponseSelected);
    connect(mDetails->ctrlBroadcast(), &QRadioButton::toggled, this, &SIMethod::onBroadcastSelected);
    connect(mDetails->ctrlConnectedResponse(), &QComboBox::currentIndexChanged, this, &SIMethod::onConnectedResponseChanged);
    connect(mList->ctrlButtonAdd(), &QToolButton::clicked, this, &SIMethod::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked, this, &SIMethod::onRemoveClicked);
    connect(mList->ctrlButtonParamAdd(), &QToolButton::clicked, this, &SIMethod::onParamAddClicked);
    connect(mList->ctrlButtonParamRemove(), &QToolButton::clicked, this, &SIMethod::onParamRemoveClicked);
    connect(mList->ctrlButtonParamInsert(), &QToolButton::clicked, this, &SIMethod::onParamInsertClicked);
    connect(mList->ctrlButtonMoveUp(), &QToolButton::clicked, this, &SIMethod::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown(), &QToolButton::clicked, this, &SIMethod::onMoveDownClicked);
    connect(mList->ctrlTableList(), &QTreeWidget::currentItemChanged, this, &SIMethod::onCurCellChanged);
}

void SIMethod::blockBasicSignals(bool doBlock)
{
    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlRequest()->blockSignals(doBlock);
    mDetails->ctrlResponse()->blockSignals(doBlock);
    mDetails->ctrlBroadcast()->blockSignals(doBlock);
    mDetails->ctrlConnectedResponse()->blockSignals(doBlock);

    mList->ctrlTableList()->blockSignals(doBlock);
}

void SIMethod::showParamDetails(bool show)
{
}

void SIMethod::showMethodDetails(bool show)
{
}

QTreeWidgetItem* SIMethod::updateMethodNode(QTreeWidgetItem* item, SIMethodBase* method)
{
    item->setText(0, method->getName());
    item->setIcon(0, QIcon());
    item->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
    item->setText(3, method->getMethodType() == SIMethodBase::eMethodType::MethodRequest ? static_cast<SIMethodRequest*>(method)->getConectedResponseName() : QString());
    item->setData(1, Qt::ItemDataRole::UserRole, 0);

    if (method->hasElements())
    {
        const QList<MethodParameter>& params = method->getElements();
        for (const MethodParameter& param : params)
        {
            QTreeWidgetItem* paramItem = new QTreeWidgetItem(item);
            paramItem->setText(0, param.getName());
            paramItem->setIcon(0, QIcon());
            paramItem->setData(0, Qt::ItemDataRole::UserRole, QVariant::fromValue(method));
            paramItem->setText(1, param.getType());
            paramItem->setData(1, Qt::ItemDataRole::UserRole, param.getId());
            paramItem->setText(2, param.getValue());
            item->addChild(paramItem);
        }
    }

    return item;
}

void SIMethod::selectRequest(SIMethodRequest* request)
{
    if (request != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlRequest()->setChecked(true);
        mDetails->ctrlRequest()->setEnabled(true);
        mDetails->ctrlResponse()->setEnabled(true);
        mDetails->ctrlBroadcast()->setEnabled(true);
        mDetails->ctrlConnectedResponse()->setEnabled(true);
        mList->ctrlButtonRemove()->setEnabled(true);
        mList->ctrlButtonParamAdd()->setEnabled(true);
        
        mDetails->ctrlName()->setText(request->getName());
        mDetails->ctrlConnectedResponse()->setCurrentText(request->getConectedResponseName());
        mDetails->ctrlDescription()->setPlainText(request->getDescription());
        mDetails->ctrlDeprecateHint()->setText(request->getDeprecateHint());
        mDetails->ctrlIsDeprecated()->setChecked(request->isDeprecated());

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
        mDetails->ctrlDeprecateHint()->setText(QString());
        mDetails->ctrlIsDeprecated()->setChecked(false);
    }
}

void SIMethod::selectBroadcast(SIMethodBroadcast* broadcast)
{
    if (broadcast != nullptr)
    {
        mDetails->ctrlName()->setEnabled(true);
        mDetails->ctrlBroadcast()->setChecked(true);
        mDetails->ctrlRequest()->setEnabled(true);
        mDetails->ctrlResponse()->setEnabled(true);
        mDetails->ctrlBroadcast()->setEnabled(true);
        mDetails->ctrlConnectedResponse()->setEnabled(false);

        mDetails->ctrlName()->setText(broadcast->getName());
        mDetails->ctrlConnectedResponse()->setCurrentText(QString());
        mDetails->ctrlDescription()->setPlainText(broadcast->getDescription());
        mDetails->ctrlDeprecateHint()->setText(broadcast->getDeprecateHint());
        mDetails->ctrlIsDeprecated()->setChecked(broadcast->isDeprecated());

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
        mDetails->ctrlConnectedResponse()->setCurrentText(QString());
        mDetails->ctrlDescription()->setPlainText(QString());
        mDetails->ctrlDeprecateHint()->setText(QString());
        mDetails->ctrlIsDeprecated()->setChecked(false);
    }
}
