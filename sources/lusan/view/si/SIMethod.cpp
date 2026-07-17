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
 *  \file        lusan/view/si/SIMethod.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIMethod.hpp"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>

#include "lusan/view/common/MethodDetailsView.hpp"
#include "lusan/view/common/MethodParamDetailsView.hpp"
#include "lusan/view/common/MethodListView.hpp"
#include "lusan/view/common/TableCell.hpp"


#include "lusan/data/si/SIMethodRequest.hpp"
#include "lusan/data/si/SIMethodResponse.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/model/si/SIMethodModel.hpp"
#include "lusan/model/common/DataTypesModel.hpp"
#include "lusan/model/common/ReplyMethodModel.hpp"

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "lusan/view/si/SICommon.hpp"

SIMethodWidget::SIMethodWidget(QWidget* parent)
    : QWidget{ parent }
    , mPanels(nullptr)
{
    QVBoxLayout* root = new QVBoxLayout(this);

    QLabel* headline = new QLabel(tr("Service Interface Methods Editor ..."), this);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    mPanels = new QHBoxLayout();
    root->addLayout(mPanels, 1);
}

SIMethod::SIMethod(SIMethodModel & model, QWidget* parent)
    : QScrollArea   (parent)
    , mModel        (model)
    , mDetails      (new MethodDetailsView(MethodViewConfig{ QList<MethodTypeOption>{ { tr("Request"), QString() }, { tr("Response"), QString() }, { tr("Broadcast"), QString() } }, true, false, false, false, false }, this))
    , mList         (new MethodListView(MethodListConfig{ tr("Service Methods List:"), tr("Data Type:"), QStringList{ tr("Request"), tr("Response"), tr("Broadcast") }, true, true }, this))
    , mParams       (new MethodParamDetailsView(tr("Parameter Details:"), this))
    , mWidget       (new SIMethodWidget(this))
    , mParamTypes   (new DataTypesModel(model.getDataTypeData(), false))
    , mReplyModel   (new ReplyMethodModel(model.getMethodData()))
    , mTableCell    (nullptr)
    , mCount        (0)
{
    mParams->setHidden(true);

    // Two equal-width panels: the list on the left, and the swapped details/parameter forms
    // wrapped in a single right panel. Both columns use an Ignored horizontal size policy so
    // the row splits exactly 50/50 regardless of either side's size hint.
    QWidget* rightPanel = new QWidget(mWidget);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(mDetails);
    rightLayout->addWidget(mParams);
    mList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    rightPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    mWidget->mPanels->addWidget(mList, 1);
    mWidget->mPanels->addWidget(rightPanel, 1);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(mWidget);

    updateData();
    updateWidgets();
    setupSignals();
}

SIMethod::~SIMethod()
{
    mWidget->mPanels->removeWidget(mList);
    mWidget->mPanels->removeWidget(mDetails);
    mWidget->mPanels->removeWidget(mParams);
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
    SICommon::checkedDeprecated<MethodDetailsView, SIMethodBase>(mDetails, method, isChecked);
}

void SIMethod::onDeprecateHintChanged(const QString& newText)
{
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    SIMethodBase* method = item != nullptr ? item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>() : nullptr;
    if (method == nullptr)
        return;
   
    SICommon::setDeprecateHint<MethodDetailsView, SIMethodBase>(mDetails, method, newText);
}

void SIMethod::onDescriptionChanged()
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

void SIMethod::addNewMethod(SIMethodBase::eMethodType type)
{
    QTreeWidget* table = mList->ctrlTableList();
    QString name = genName();

    blockBasicSignals(true);

    QTreeWidgetItem* cur = table->currentItem();
    if (cur != nullptr)
    {
        cur->setSelected(false);
    }

    SIMethodBase* newMethod = mModel.addMethod(name, type);
    Q_ASSERT(newMethod != nullptr);
    mReplyModel->methodCreated(newMethod);

    int pos = table->topLevelItemCount();
    QTreeWidgetItem* item = updateMethodNode(new QTreeWidgetItem(), newMethod);
    table->addTopLevelItem(item);
    item->setSelected(true);
    table->setCurrentItem(item);
    showMethodDetails(newMethod);
    updateToolButtonsForMethod(pos, pos + 1);
    blockBasicSignals(false);
}

void SIMethod::onAddClicked()
{
    addNewMethod(SIMethodBase::eMethodType::MethodRequest);
}

void SIMethod::onInsertClicked()
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
    updateToolButtonsForMethod(row, table->topLevelItemCount());
    blockBasicSignals(false);
}

void SIMethod::onRemoveClicked()
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
    updateToolButtonsForMethod(row, rowCount);

    blockBasicSignals(false);
}

void SIMethod::onParamAddClicked()
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

        // The method row's Value column shows the parameter count, so refresh it after the add.
        setNodeText(parent, method);
        showParamDetails(method, *param);
        updateToolButtonsForParams(method, pos);
        blockBasicSignals(false);
    }
}

void SIMethod::onParamRemoveClicked()
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

    // The method row's Value column shows the parameter count, so refresh it after the removal.
    setNodeText(parent, method);

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
        method = nullptr;
    }
    
    if (next != nullptr)
    {
        next->setSelected(true);
        table->setCurrentItem(next);
        uint32_t nextId = next->data(1, Qt::ItemDataRole::UserRole).toUInt();
        SIMethodBase* nextMethod = next->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase *>();
        if (nextId != 0)
        {
            method = nextMethod;
            MethodParameter* nextParam = nextMethod->findElement(nextId);
            Q_ASSERT(nextParam != nullptr);
            showParamDetails(nextMethod, *nextParam);
        }
        else
        {
            method = nullptr;
            showMethodDetails(nextMethod);
        }
    }
    else
    {
        method = nullptr;
        showMethodDetails(nullptr);
    }
    
    delete item;
    if (method == nullptr)
    {
        updateToolButtonsForMethod(row, rowCount);
    }
    else
    {
        updateToolButtonsForParams(method, row);
    }
    
    blockBasicSignals(false);
}

void SIMethod::onParamInsertClicked()
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

        // The method row's Value column shows the parameter count, so refresh it after the insert.
        setNodeText(parent, method);
        showParamDetails(method, *param);
        updateToolButtonsForParams(method, row);
        blockBasicSignals(false);
    }
}

void SIMethod::onMoveUpClicked()
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

void SIMethod::onMoveDownClicked()
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
        updateToolButtonsForMethod(table->indexOfTopLevelItem(current), table->topLevelItemCount());
    }
    else
    {
        Q_ASSERT(method != nullptr);
        Q_ASSERT(id != 0);
        MethodParameter* param = method->findElement(id);
        Q_ASSERT(param != nullptr);
        showParamDetails(method, *param);
        updateToolButtonsForParams(method, parent->indexOfChild(current));
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
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    Q_ASSERT(item != nullptr);
    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    Q_ASSERT(method != nullptr);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    MethodParameter* param = method->makeValueDefault(id, isChecked, mParams->ctrlValue()->text());
    Q_ASSERT(param != nullptr);
    setNodeText(item, param);
    showParamDetails(method, *param);
    updateToolButtonsForParams(method, method->findIndex(id));

    if (isChecked)
    {
        QLineEdit* ctrlDefault = mParams->ctrlValue();
        if (ctrlDefault->isEnabled())
        {
            ctrlDefault->setFocus();
            if (ctrlDefault->text().isEmpty() == false)
            {
                ctrlDefault->selectAll();
            }
        }
    }
}

void SIMethod::onParamDefaultChanged(const QString& newText)
{
    Q_UNUSED(newText);
    QTreeWidget* table = mList->ctrlTableList();
    QTreeWidgetItem* item = table->currentItem();
    Q_ASSERT(item != nullptr);
    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    Q_ASSERT(method != nullptr);
    uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    Q_ASSERT(id != 0);
    MethodParameter* param = method->setDefaultValue(id, mParams->ctrlValue()->text());
    Q_ASSERT(param != nullptr);
    setNodeText(item, param);
}

void SIMethod::onParamDescriptionChanged()
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
    param->setDescription(mParams->ctrlDescription()->toPlainText());
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

    SICommon::checkedDeprecated<MethodParamDetailsView, MethodParameter>(mParams, param, isChecked);
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
    SICommon::setDeprecateHint<MethodParamDetailsView, MethodParameter>(mParams, param, newText);
}

void SIMethod::updateData()
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

    mParams->ctrlTypes()->setModel(mParamTypes);
    mDetails->ctrlReply()->setModel(mReplyModel);
}

void SIMethod::updateWidgets()
{
    showMethodDetails(nullptr);
    updateToolButtonsForMethod(-1, 0);
}

void SIMethod::setupSignals()
{
    connect(mDetails                        , &MethodDetailsView::nameEdited, this, &SIMethod::onNameChanged);
    connect(mDetails->ctrlType(0)           , &QRadioButton::toggled        , this, &SIMethod::onRequestSelected);
    connect(mDetails->ctrlType(1)           , &QRadioButton::toggled        , this, &SIMethod::onResponseSelected);
    connect(mDetails->ctrlType(2)           , &QRadioButton::toggled        , this, &SIMethod::onBroadcastSelected);
    connect(mDetails->ctrlDeprecated()      , &QCheckBox::toggled           , this, &SIMethod::onDeprecateChecked);
    connect(mDetails->ctrlDeprecateHint()   , &QLineEdit::textEdited        , this, &SIMethod::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription()     , &QPlainTextEdit::textChanged  , this, &SIMethod::onDescriptionChanged);
    connect(mDetails->ctrlReply()           , &QComboBox::currentTextChanged, this, &SIMethod::onConnectedResponseChanged);

    connect(mList->ctrlButtonAdd()          , &QToolButton::clicked         , this, &SIMethod::onAddClicked);
    connect(mList->ctrlButtonInsert()       , &QToolButton::clicked         , this, &SIMethod::onInsertClicked);
    connect(mList->ctrlButtonRemove()       , &QToolButton::clicked         , this, &SIMethod::onRemoveClicked);
    connect(mList->ctrlButtonParamAdd()     , &QToolButton::clicked         , this, &SIMethod::onParamAddClicked);
    connect(mList->ctrlButtonParamInsert()  , &QToolButton::clicked         , this, &SIMethod::onParamInsertClicked);
    connect(mList->ctrlButtonParamRemove()  , &QToolButton::clicked         , this, &SIMethod::onParamRemoveClicked);
    connect(mList->ctrlButtonMoveUp()       , &QToolButton::clicked         , this, &SIMethod::onMoveUpClicked);
    connect(mList->ctrlButtonMoveDown()     , &QToolButton::clicked         , this, &SIMethod::onMoveDownClicked);
    connect(mList->typeAction(0)            , &QAction::triggered           , this, [this]() { addNewMethod(SIMethodBase::eMethodType::MethodRequest); });
    connect(mList->typeAction(1)            , &QAction::triggered           , this, [this]() { addNewMethod(SIMethodBase::eMethodType::MethodResponse); });
    connect(mList->typeAction(2)            , &QAction::triggered           , this, [this]() { addNewMethod(SIMethodBase::eMethodType::MethodBroadcast); });
    connect(mList->ctrlTableList()          , &QTreeWidget::currentItemChanged, this, &SIMethod::onCurCellChanged);

    connect(mParams                         , &MethodParamDetailsView::nameEdited, this, &SIMethod::onParamNameChanged);
    connect(mParams->ctrlTypes()            , &QComboBox::currentTextChanged, this, &SIMethod::onParamTypeChanged);
    connect(mParams->ctrlHasDefault()  , &QCheckBox::toggled           , this, &SIMethod::onParamDefaultChecked);
    connect(mParams->ctrlValue(), &QLineEdit::textChanged       , this, &SIMethod::onParamDefaultChanged);
    connect(mParams->ctrlDescription() , &QPlainTextEdit::textChanged  , this, &SIMethod::onParamDescriptionChanged);
    connect(mParams->ctrlDeprecated()       , &QCheckBox::toggled           , this, &SIMethod::onParamDeprecateChecked);
    connect(mParams->ctrlDeprecateHint()    , &QLineEdit::textEdited        , this, &SIMethod::onParamDeprecateHintChanged);

    // Inline (in-table) editing: the shared TableCell delegate opens an editor per cell. The tree
    // is heterogeneous (method rows vs parameter rows), so which cells are editable, whether a
    // cell shows a combo or a text editor, and how it is validated are resolved per cell by the
    // controller; the commit routes through cellChanged(), matching the details-panel validation.
    QTreeWidget* table = mList->ctrlTableList();
    mTableCell = new TableCell(table, this, false);
    mTableCell->setEditableCheck([this](const QModelIndex& idx) { return isCellEditable(idx); });
    mTableCell->setEditorModelResolver([this](const QModelIndex& idx) { return editorModelFor(idx); });
    mTableCell->setValidationResolver([this](const QModelIndex& idx) { return validationFor(idx); });
    for (int col = 0; col < table->columnCount(); ++col)
    {
        table->setItemDelegateForColumn(col, mTableCell);
    }
    connect(mTableCell, &TableCell::signalEditorDataChanged, this, &SIMethod::onEditorDataChanged);
}

void SIMethod::blockBasicSignals(bool doBlock)
{
    mDetails->ctrlName()->blockSignals(doBlock);
    mDetails->ctrlType(0)->blockSignals(doBlock);
    mDetails->ctrlType(1)->blockSignals(doBlock);
    mDetails->ctrlType(2)->blockSignals(doBlock);
    mDetails->ctrlReply()->blockSignals(doBlock);
    
    mParams->ctrlName()->blockSignals(doBlock);
    mParams->ctrlTypes()->blockSignals(doBlock);
    mParams->ctrlHasDefault()->blockSignals(doBlock);
    mParams->ctrlValue()->blockSignals(doBlock);

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
        mDetails->ctrlType(0)->setEnabled(true);
        mDetails->ctrlType(1)->setEnabled(true);
        mDetails->ctrlType(2)->setEnabled(true);
        mList->ctrlButtonRemove()->setEnabled(true);
        mList->ctrlButtonParamAdd()->setEnabled(true);
        mList->ctrlButtonParamRemove()->setEnabled(false);
        mList->ctrlButtonParamInsert()->setEnabled(true);

        mDetails->ctrlName()->setText(method->getName());
        mDetails->ctrlDescription()->setPlainText(method->getDescription());

        SICommon::enableDeprecated<MethodDetailsView, SIMethodBase>(mDetails, method, true);

        switch (method->getMethodType())
        {
        case SIMethodBase::eMethodType::MethodRequest:
            mDetails->ctrlReply()->setEnabled(true);
            mDetails->ctrlType(0)->setChecked(true);
            if (static_cast<SIMethodRequest *>(method)->hasValidResponse())
            {
                mDetails->ctrlReply()->setCurrentText(static_cast<SIMethodRequest *>(method)->getConectedResponseName());
            }
            else
            {
                mDetails->ctrlReply()->setCurrentText(QString());
            }
            break;

        case SIMethodBase::eMethodType::MethodResponse:
            mDetails->ctrlReply()->setEnabled(false);
            mDetails->ctrlReply()->setCurrentText(QString());
            mDetails->ctrlType(1)->setChecked(true);
            break;

        case SIMethodBase::eMethodType::MethodBroadcast:
            mDetails->ctrlReply()->setEnabled(false);
            mDetails->ctrlReply()->setCurrentText(QString());
            mDetails->ctrlType(2)->setChecked(true);
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
        mDetails->ctrlType(0)->setChecked(false);
        mDetails->ctrlType(0)->setEnabled(false);
        mDetails->ctrlType(1)->setEnabled(false);
        mDetails->ctrlType(2)->setEnabled(false);
        mDetails->ctrlReply()->setEnabled(false);
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(false);
        mList->ctrlButtonParamRemove()->setEnabled(false);
        mList->ctrlButtonParamInsert()->setEnabled(false);
        
        mDetails->ctrlReply()->setCurrentText(QString());
        mDetails->ctrlDescription()->setPlainText(QString());
        SICommon::enableDeprecated<MethodDetailsView, SIMethodBase>(mDetails, nullptr, false);
    }
}

void SIMethod::showParamDetails(SIMethodBase* method, const MethodParameter& param)
{
    mDetails->hide();
    mParams->setVisible(true);
    if (method != nullptr)
    {
        mParams->ctrlName()->setEnabled(true);
        mParams->ctrlTypes()->setEnabled(true);
            
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(true);
        mList->ctrlButtonParamRemove()->setEnabled(true);
        mList->ctrlButtonParamInsert()->setEnabled(true);
        
        mParams->ctrlName()->setText(param.getName());
        mParams->ctrlTypes()->setCurrentText(param.getType());
        const bool canEditDefault = method->canSwitchDefaultValue(param.getId());
        if (param.hasDefault())
        {
            mParams->ctrlHasDefault()->setChecked(true);
            mParams->ctrlValue()->setText(param.getValue());
            mParams->ctrlValue()->setEnabled(canEditDefault);
        }
        else
        {
            mParams->ctrlHasDefault()->setChecked(false);
            mParams->ctrlValue()->setText(QString());
            mParams->ctrlValue()->setEnabled(false);
        }
        
        mParams->ctrlDescription()->setPlainText(param.getDescription());
        SICommon::enableDeprecated<MethodParamDetailsView, MethodParameter>(mParams, &param, true);
        mParams->ctrlName()->setFocus();
        mParams->ctrlName()->selectAll();        
    }
    else
    {
        mList->ctrlButtonRemove()->setEnabled(false);
        mList->ctrlButtonParamAdd()->setEnabled(false);
        mList->ctrlButtonParamRemove()->setEnabled(false);
        mList->ctrlButtonParamInsert()->setEnabled(false);
        
        mParams->ctrlName()->setText(QString());
        mParams->ctrlTypes()->setCurrentText(QString());
        mParams->ctrlHasDefault()->setChecked(false);
        mParams->ctrlValue()->setText(QString());
        mParams->ctrlValue()->setEnabled(false);
        mParams->ctrlDescription()->setPlainText(QString());
        SICommon::enableDeprecated<MethodParamDetailsView, MethodParameter>(mParams, nullptr, false);
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

    // Let the TableCell delegate open an inline editor; which cells actually edit is gated per
    // cell by isCellEditable() (tree items are non-editable by default, unlike table items).
    node->setFlags(node->flags() | Qt::ItemIsEditable);

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
    updateToolButtonsForMethod(moveRow, table->topLevelItemCount());
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
    updateToolButtonsForParams(method, moveRow);
}

inline void SIMethod::updateToolButtonsForMethod(int row, int rowCount)
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

inline void SIMethod::updateToolButtonsForParams(SIMethodBase * method, int row)
{
    int rowCount = method != nullptr ? method->getElementCount() : 0;
    if ((row >= 0) && (row < rowCount) && (method != nullptr))
    {
        if ((row == 0) && (rowCount == 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else if (row == 0)
        {
            mList->ctrlButtonMoveUp()->setEnabled(false);
            mList->ctrlButtonMoveDown()->setEnabled(method->canSwapParamRight(row));
        }
        else if (row == (rowCount - 1))
        {
            mList->ctrlButtonMoveUp()->setEnabled(method->canSwapParamLeft(row));
            mList->ctrlButtonMoveDown()->setEnabled(false);
        }
        else
        {
            mList->ctrlButtonMoveUp()->setEnabled(method->canSwapParamLeft(row));
            mList->ctrlButtonMoveDown()->setEnabled(method->canSwapParamRight(row));
        }
    }
    else
    {
        mList->ctrlButtonMoveUp()->setEnabled(false);
        mList->ctrlButtonMoveDown()->setEnabled(false);
    }
}

inline QString SIMethod::genName()
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

int SIMethod::getColumnCount() const
{
    return mList->ctrlTableList()->columnCount();
}

QString SIMethod::getCellText(const QModelIndex& cell) const
{
    // The display text already stored on the tree cell seeds the inline editor.
    return cell.isValid() ? cell.data(Qt::DisplayRole).toString() : QString();
}

bool SIMethod::isCellEditable(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return false;

    SIMethodBase* method = index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (method == nullptr)
        return false;

    // The id (0 for a method row, the parameter id for a child row) lives at column 1's UserRole.
    const uint32_t id = index.sibling(index.row(), static_cast<int>(MethodListView::ColType)).data(Qt::ItemDataRole::UserRole).toUInt();
    const int col = index.column();
    if (id == 0)
    {
        // Method row: the name is always editable; the response link only for a request. The
        // data-type and value columns are read-only (the kind is set by the details radios and
        // the value column shows the parameter count).
        if (col == static_cast<int>(MethodListView::ColName))
            return true;
        if (col == static_cast<int>(MethodListView::ColResponse))
            return (method->getMethodType() == SIMethodBase::eMethodType::MethodRequest);
        return false;
    }

    // Parameter row: name and data type are editable; the value only when a default is allowed
    // (mirroring the details panel, where the value field is enabled under the same condition).
    if (col == static_cast<int>(MethodListView::ColName))
        return true;
    if (col == static_cast<int>(MethodListView::ColType))
        return true;
    if (col == static_cast<int>(MethodListView::ColValue))
    {
        MethodParameter* param = method->findElement(id);
        return (param != nullptr) && param->hasDefault() && method->canSwitchDefaultValue(id);
    }

    return false;
}

QAbstractItemModel* SIMethod::editorModelFor(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return nullptr;

    SIMethodBase* method = index.sibling(index.row(), 0).data(Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (method == nullptr)
        return nullptr;

    const uint32_t id = index.sibling(index.row(), static_cast<int>(MethodListView::ColType)).data(Qt::ItemDataRole::UserRole).toUInt();
    const int col = index.column();
    if (id == 0)
    {
        // Method row: the Response column offers the responses (its blank first row = no response).
        if ((col == static_cast<int>(MethodListView::ColResponse)) && (method->getMethodType() == SIMethodBase::eMethodType::MethodRequest))
            return mReplyModel;

        return nullptr;
    }

    // Parameter row: the Data Type column offers the parameter types (same model as the details).
    return (col == static_cast<int>(MethodListView::ColType)) ? mParamTypes : nullptr;
}

TableCell::eCellValidation SIMethod::validationFor(const QModelIndex& index) const
{
    // Only the Name column is a free-text C++ identifier; the other editable columns are combos.
    return (index.isValid() && (index.column() == static_cast<int>(MethodListView::ColName)))
        ? TableCell::eCellValidation::Identifier
        : TableCell::eCellValidation::NoValidation;
}

void SIMethod::onEditorDataChanged(const QModelIndex& index, const QString& newValue)
{
    if (index.isValid() == false)
        return;

    // Inline editing starts on the current item, so the edited row is the current one.
    QTreeWidgetItem* item = mList->ctrlTableList()->currentItem();
    if (item != nullptr)
    {
        cellChanged(item, index.column(), newValue);
    }
}

void SIMethod::cellChanged(QTreeWidgetItem* item, int column, const QString& newValue)
{
    if (item == nullptr)
        return;

    SIMethodBase* method = item->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
    if (method == nullptr)
        return;

    const uint32_t id = item->data(1, Qt::ItemDataRole::UserRole).toUInt();
    blockBasicSignals(true);

    if (id == 0)
    {
        if (column == static_cast<int>(MethodListView::ColName))
        {
            if (method->getName() != newValue)
            {
                method->setName(newValue);
                mReplyModel->methodUpdated(method);
                setNodeText(item, method);
                mDetails->ctrlName()->setText(method->getName());

                // A renamed response must refresh every request that links to it.
                if (method->getMethodType() == SIMethodBase::eMethodType::MethodResponse)
                {
                    QTreeWidget* table = mList->ctrlTableList();
                    const int count = table->topLevelItemCount();
                    for (int i = 0; i < count; ++i)
                    {
                        QTreeWidgetItem* top = table->topLevelItem(i);
                        SIMethodBase* entry = top->data(0, Qt::ItemDataRole::UserRole).value<SIMethodBase*>();
                        if ((entry == nullptr) || (entry->getMethodType() != SIMethodBase::eMethodType::MethodRequest))
                            continue;

                        SIMethodRequest* request = static_cast<SIMethodRequest*>(entry);
                        if (method == static_cast<const SIMethodBase*>(request->getConectedResponse()))
                            setNodeText(top, entry);
                    }
                }
            }
        }
        else if (column == static_cast<int>(MethodListView::ColResponse))
        {
            if (method->getMethodType() == SIMethodBase::eMethodType::MethodRequest)
            {
                SIMethodResponse* response = (newValue.isEmpty() == false) ? mReplyModel->findResponse(newValue) : nullptr;
                static_cast<SIMethodRequest*>(method)->connectResponse(response);
                setNodeText(item, method);
                mDetails->ctrlReply()->setCurrentText(response != nullptr ? response->getName() : QString());
            }
        }
    }
    else
    {
        MethodParameter* param = method->findElement(id);
        if (param != nullptr)
        {
            if (column == static_cast<int>(MethodListView::ColName))
            {
                if (param->getName() != newValue)
                {
                    param->setName(newValue);
                    setNodeText(item, param);
                    mParams->ctrlName()->setText(param->getName());
                }
            }
            else if (column == static_cast<int>(MethodListView::ColType))
            {
                DataTypeBase* dataType = mModel.getDataTypeData().findDataType(newValue);
                if ((dataType != nullptr) && (param->getParamType() != dataType))
                {
                    param->setParamType(dataType);
                    setNodeText(item, param);
                    mParams->ctrlTypes()->setCurrentText(param->getType());
                }
            }
            else if (column == static_cast<int>(MethodListView::ColValue))
            {
                MethodParameter* updated = method->setDefaultValue(id, newValue);
                if (updated != nullptr)
                {
                    setNodeText(item, updated);
                    mParams->ctrlValue()->setText(updated->getValue());
                }
            }
        }
    }

    blockBasicSignals(false);
}
