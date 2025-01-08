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
 *  \file        lusan/view/si/SIDataType.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

#include "lusan/view/si/SIDataType.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataType.h"

#include "lusan/data/common/DataTypeDefined.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeImported.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/model/si/SIDataTypeModel.hpp"
#include "lusan/view/si/SIDataTypeDetails.hpp"
#include "lusan/view/si/SIDataTypeFieldDetails.hpp"
#include "lusan/view/si/SIDataTypeList.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpacerItem>
#include <QTableWidget>
#include <QToolButton>

SIDataTypeWidget::SIDataTypeWidget(QWidget* parent)
    : QWidget{ parent }
    , ui     (new Ui::SIDataType)
{
    ui->setupUi(this);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    setMinimumSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
}

SIDataType::SIDataType(SIDataTypeModel& model, QWidget *parent)
    : QScrollArea   (parent)
    , mDetails  (new SIDataTypeDetails(this))
    , mList     (new SIDataTypeList(this))
    , mFields   (new SIDataTypeFieldDetails(this))
    , mWidget   (new SIDataTypeWidget(this))
    , ui        (*mWidget->ui)
    , mModel    (model)
    , mCount    (0)
{
    mFields->setHidden(true);
    
    ui.horizontalLayout->addWidget(mList);
    ui.horizontalLayout->addWidget(mDetails);
    
    // setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizeAdjustPolicy(QScrollArea::SizeAdjustPolicy::AdjustToContents);
    setBaseSize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT);
    resize(SICommon::FRAME_WIDTH, SICommon::FRAME_HEIGHT / 2);
    
    updateWidgets();
    setupSignals();
}

SIDataType::~SIDataType(void)
{
    mList->ctrlTableList()->setModel(nullptr);
    ui.horizontalLayout->removeWidget(mList);
    ui.horizontalLayout->removeWidget(mDetails);
}

void SIDataType::closeEvent(QCloseEvent *event)
{
    mList->ctrlTableList()->setModel(nullptr);
    mModel.setParent(nullptr);
    QScrollArea::closeEvent(event);
}

void SIDataType::hideEvent(QHideEvent *event)
{
    mList->ctrlTableList()->setModel(nullptr);
    mModel.setParent(nullptr);
    QScrollArea::hideEvent(event);
}

void SIDataType::onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
}

void SIDataType::onAddClicked(void)
{
    static const QString _defName("NewDataType");

    QTableView* table = mList->ctrlTableList();
    QString name;
    do
    {
        name = _defName + QString::number(++mCount);
    } while (mModel.findDataType(name) != nullptr);

    DataTypeCustom* dataType = mModel.addDataType(name, DataTypeBase::eCategory::Structure);
    mDetails->ctrlName()->setText(dataType->getName());
    mDetails->ctrlTypeStruct()->setChecked(true);
    mDetails->ctrlDeprecated()->setChecked(dataType->getIsDeprecated());
    mDetails->ctrlDeprecateHint()->setText(dataType->getDeprecateHint());
    mDetails->ctrlDescription()->setPlainText(dataType->getDescription());
}

void SIDataType::onRemoveClicked(void)
{
}

void SIDataType::onNameChanged(const QString& newName)
{
}

void SIDataType::onDeprectedChecked(bool isChecked)
{
}

void SIDataType::onDeprecateHintChanged(const QString& newText)
{
}

void SIDataType::updateData(void)
{
    
}

void SIDataType::onDescriptionChanged(void)
{
}

void SIDataType::showEnumDetails(bool show)
{
    static constexpr int _space{180};
    SIDataTypeDetails::CtrlGroup item{mDetails->ctrlDetailsEnum()};
    mDetails->changeSpace((show ? -1 : 1) * _space);
    item.first->setHidden(!show);
    item.second->setHidden(!show);
}

void SIDataType::showImportDetails(bool show)
{
    static constexpr int _space{120};
    SIDataTypeDetails::CtrlGroup item{mDetails->ctrlDetailsImport()};
    mDetails->changeSpace((show ? -1 : 1) * _space);
    item.first->setHidden(!show);
    item.second->setHidden(!show);
}

void SIDataType::showContainerDetails(bool show)
{
    static constexpr int _space{60};
    SIDataTypeDetails::CtrlGroup item{mDetails->ctrlDetailsContainer()};
    mDetails->changeSpace((show ? -1 : 1) * _space);
    item.first->setHidden(!show);
    item.second->setHidden(!show);
}

void SIDataType::updateWidgets(void)
{
    showEnumDetails(false);
    showContainerDetails(false);
    showImportDetails(false);
    mModel.setParent(mList->ctrlTableList());
    mList->ctrlTableList()->setModel(&mModel);
}

void SIDataType::setupSignals(void)
{
    // connect(mList->ctrlTableList(),    &QTableWidget::currentCellChanged, this, &SIDataType::onCurCellChanged);
    connect(mList->ctrlButtonAdd(),    &QToolButton::clicked        , this, &SIDataType::onAddClicked);
    connect(mList->ctrlButtonRemove(), &QToolButton::clicked        , this, &SIDataType::onRemoveClicked);
    connect(mDetails->ctrlName(),      &QLineEdit::textChanged      , this, &SIDataType::onNameChanged);
    connect(mDetails->ctrlDeprecated(),&QCheckBox::toggled          , this, &SIDataType::onDeprectedChecked);
    connect(mDetails->ctrlDeprecateHint(),&QLineEdit::textEdited    , this, &SIDataType::onDeprecateHintChanged);
    connect(mDetails->ctrlDescription(),&QPlainTextEdit::textChanged, this, &SIDataType::onDescriptionChanged);
    // connect(mTableCell                , &TableCell::editorDataChanged,this, &SIConstant::onEditorDataChanged);
}
