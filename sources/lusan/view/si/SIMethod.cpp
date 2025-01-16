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
#include "lusan/data/si/SIMethodData.hpp"
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
    if (item == nullptr)
        return;

    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (method->getMethodType() == SIMethodBase::eMethodType::MethodRequest)
        return;

    mModel.convertMethod(method->getName(), SIMethodBase::eMethodType::MethodRequest);
    {
        static_cast<SIMethodRequest*>(method)->setConnectedResponse(nullptr);
    }
    else
    {
        method->setMethodType(SIMethodBase::eMethodType::MethodRequest);
    }
}

void SIMethod::onResponseSelected(bool isSelected)
{
}

void SIMethod::onBroadcastSelected(bool isSelected)
{
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

    SIMethodBase * newMethod = mModel.createMethod(name, SIMethodBase::eMethodType::MethodRequest);
    Q_ASSERT(newMethod != nullptr);
    QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(table), newMethod);
    table->addTopLevelItem(item);
    mDetails->ctrlName()->setFocus();
    mDetails->ctrlName()->selectAll();
}

void SIMethod::onRemoveClicked(void)
{
}

void SIMethod::onParamAddClicked(void)
{
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
    item->setText(1, method->getMethodType() == SIMethodBase::eMethodType::MethodRequest ? static_cast<SIMethodRequest*>(method)->getConectedResponseName() : QString());
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

