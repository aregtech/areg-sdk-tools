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
#include "lusan/data/si/ServiceInterfaceData.hpp"

#include <QVBoxLayout>
#include <QIcon>

const QString& ServiceInterface::fileExtension(void)
{
    static const QString _extSI{ "siml" };
    return _extSI;
}

uint32_t ServiceInterface::_count{0};

ServiceInterface::ServiceInterface(const QString & filePath /*= QString()*/, QWidget *parent /*= nullptr*/)
    : MdiChild      (parent)
    , IEMdiWindow   (IEMdiWindow::eMdiWindow::MdiServiceInterface)

    , mModel    (filePath)
    , mTabWidget(this)
    , mOverview (mModel.getOverviewModel()  , this)
    , mDataType (mModel.getDataTypeModel()  , this)
    , mAttribute(mModel.getAttributeModel() , this)
    , mMethod   (mModel.getMethodsModel()   , this)
    , mConstant (mModel.getConstantsModel() , this)
    , mInclude  (mModel.getIncludesModel()  , this)
{
    mTabWidget.setTabPosition(QTabWidget::South);
    // Add the SIOverview widget as the first tab
    mTabWidget.addTab(&mOverview , QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview), tr("Overview"));
    mTabWidget.addTab(&mDataType , QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview), tr("Data Types"));
    mTabWidget.addTab(&mAttribute, QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview), tr("Data Attributes"));
    mTabWidget.addTab(&mMethod   , QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview), tr("Methods"));
    mTabWidget.addTab(&mConstant , QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview), tr("Constants"));
    mTabWidget.addTab(&mInclude  , QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrintPreview), tr("Includes"));
    mTabWidget.setTabShape(QTabWidget::Triangular);
    mTabWidget.setTabsClosable(false);
    mTabWidget.setMovable(false);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&mTabWidget);
    setLayout(layout);
    
    SIDataTypeData& data = mModel.getData().getDataTypeData();
    connect(&data, &SIDataTypeData::signalDataTypeCreated   , this, &ServiceInterface::slotDataTypeCreated);
    connect(&data, &SIDataTypeData::signalDataTypeDeleted   , this, &ServiceInterface::slotDataTypeDeleted);
    connect(&data, &SIDataTypeData::signalDataTypeConverted , this, &ServiceInterface::slotDataTypeConverted);
    connect(&data, &SIDataTypeData::signalDataTypeUpdated   , this, &ServiceInterface::slotDataTypeUpdated);
    connect(&mOverview, &SIOverview::signalPageLinkClicked  , this, &ServiceInterface::slotPageLinkClicked);
    
    setAttribute(Qt::WA_DeleteOnClose);

    if (filePath.isEmpty() == false)
    {
        mIsUntitled = false;
        setCurrentFile(filePath);
    }
}

ServiceInterface::~ServiceInterface(void)
{
}

bool ServiceInterface::openSucceeded(void) const
{
    return mModel.openSucceeded();
}

QString ServiceInterface::newDocumentName(void)
{
    static uint32_t _seqNr{0};
    mDocName = newDocument() + QString::number(++_seqNr);
    return mDocName + newDocumentExt();
}

const QString& ServiceInterface::newDocument(void) const
{
    static const QString _newSIDoc{"NewServiceInterface"};
    return _newSIDoc;
}

const QString& ServiceInterface::newDocumentExt(void) const
{
    static const QString _extSI {".siml"};
    return _extSI;
}

const QString& ServiceInterface::fileSuffix(void) const
{
    static const QString _suffixSI{ "siml" };
    return _suffixSI;
}

const QString& ServiceInterface::fileFilter(void) const
{
    static const QString _filterSI{ "Service Interface document (*.siml)\nAll Files (*.*)" };
    return _filterSI;
}

bool ServiceInterface::writeToFile(const QString& filePath)
{
    bool result = mModel.saveToFile(filePath);
    mOverview.setServiceInterfaceName(mModel.getName());
    return result;
}

void ServiceInterface::slotDataTypeCreated(DataTypeCustom* dataType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mAttribute).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeCreated(dataType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeCreated(dataType);
}

void ServiceInterface::slotDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mAttribute).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeConverted(oldType, newType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeConverted(oldType, newType);
}

void ServiceInterface::slotDataTypeDeleted(DataTypeCustom* dataType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeDeleted(dataType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeDeleted(dataType);
    static_cast<IEDataTypeConsumer&>(mAttribute).dataTypeDeleted(dataType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeDeleted(dataType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeDeleted(dataType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeDeleted(dataType);
}

void ServiceInterface::slotDataTypeUpdated(DataTypeCustom* dataType)
{
    static_cast<IEDataTypeConsumer&>(mOverview).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mDataType).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mAttribute).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mMethod).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mConstant).dataTypeUpdated(dataType);
    static_cast<IEDataTypeConsumer&>(mInclude).dataTypeUpdated(dataType);
}

void ServiceInterface::slotPageLinkClicked(int page)
{
    mTabWidget.setCurrentIndex(page);
}
