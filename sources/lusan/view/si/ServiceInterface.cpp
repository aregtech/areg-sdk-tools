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
 *  \file        lusan/view/si/ServiceInterface.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include "lusan/view/si/ServiceInterface.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"

#include <QVBoxLayout>

uint32_t ServiceInterface::_count{0};

ServiceInterface::ServiceInterface(QWidget *parent)
    : MdiChild  (parent)
    , mModel    ( )
    , mTabWidget(this)
    , mOverview (mModel.getOverviewModel()  , this)
    , mDataType (mModel.getDataTypeModel()  , this)
    , mDataTopic(mModel.getDataTopicModel() , this)
    , mMethod   (this)
    , mConstant (mModel.getConstantsModel() , this)
    , mInclude  (mModel.getIncludesModel()  , this)
{
    mTabWidget.setTabPosition(QTabWidget::South);
    mTabWidget.setTabShape(QTabWidget::Triangular);
    // Add the sioverview widget as the first tab
    mTabWidget.addTab(&mOverview , tr("Overview"));
    mTabWidget.addTab(&mDataType , tr("Data Types"));
    mTabWidget.addTab(&mDataTopic, tr("Data Topics"));
    mTabWidget.addTab(&mMethod   , tr("Methods"));
    mTabWidget.addTab(&mConstant , tr("Constants"));
    mTabWidget.addTab(&mInclude  , tr("Includes"));

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&mTabWidget);
    setLayout(layout);
    
    connect(&mDataType, &SIDataType::signalDataTypeConverted, this, &ServiceInterface::slotDataTypeConverted);
    connect(&mDataType, &SIDataType::signalDataTypeCreated  , this, &ServiceInterface::slotDataTypeCreated);
    connect(&mDataType, &SIDataType::signalDataTypeRemoved  , this, &ServiceInterface::slotlDataTypeRemoved);
    connect(&mDataType, &SIDataType::signalDataTypeUpdated  , this, &ServiceInterface::slotlDataTypeUpdated);
    
    this->setWindowTitle("NewServiceInterface");    
    setAttribute(Qt::WA_DeleteOnClose);
}

ServiceInterface::~ServiceInterface(void)
{
}

void ServiceInterface::slotDataTypeCreated(DataTypeCustom* dataType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mDataTopic).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeCreated(dataType);
}

void ServiceInterface::slotDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mDataTopic).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeConverted(oldType, newType);
}

void ServiceInterface::slotlDataTypeRemoved(DataTypeCustom* dataType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeRemoved(dataType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeRemoved(dataType);
    static_cast<IEDataTypeConsumer&>(mDataTopic).dataTypeRemoved(dataType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeRemoved(dataType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeRemoved(dataType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeRemoved(dataType);
}

void ServiceInterface::slotlDataTypeUpdated(DataTypeCustom* dataType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mDataTopic).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeUpdated(dataType);
}
