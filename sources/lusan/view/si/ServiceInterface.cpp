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
 *  \file        lusan/view/si/ServiceInterface.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include "lusan/view/si/ServiceInterface.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/view/si/SIAttribute.hpp"
#include "lusan/view/si/SIConstant.hpp"
#include "lusan/view/si/SIDataType.hpp"
#include "lusan/view/si/SIInclude.hpp"
#include "lusan/view/si/SIMethod.hpp"
#include "lusan/view/si/SIOverview.hpp"

#include <QTimer>
#include <QVBoxLayout>

namespace
{
    constexpr int TabInitStartDelayMs{ 200 };
    constexpr int TabInitDelayMs{ 50 };

    const QIcon& tabIcon()
    {
        static const QIcon _icon{ NELusanCommon::iconServiceInterfaceTab(NELusanCommon::SizeSmall) };
        return _icon;
    }
}

const QString& ServiceInterface::fileExtension()
{
    static const QString _extSI{ "siml" };
    return _extSI;
}

uint32_t ServiceInterface::_count{0};

ServiceInterface::ServiceInterface(MdiMainWindow *wndMain, const QString & filePath /*= QString()*/, QWidget *parent /*= nullptr*/)
    : MdiChild      (MdiChild::eMdiWindow::MdiServiceInterface, wndMain, parent)

    , mModel    (filePath)
    , mTabWidget(this)
    , mOverview (nullptr)
    , mDataType (nullptr)
    , mAttribute(nullptr)
    , mMethod   (nullptr)
    , mConstant (nullptr)
    , mInclude  (nullptr)
    , mPendingInitTabs( )
{
    mTabWidget.setTabPosition(QTabWidget::South);
    const int pageCount{ static_cast<int>(eSIPages::PageIncludes) + 1 };
    for (int i = 0; i < pageCount; ++i)
    {
        QWidget* holder = new QWidget(&mTabWidget);
        QVBoxLayout* holderLayout = new QVBoxLayout(holder);
        holderLayout->setContentsMargins(0, 0, 0, 0);
        mTabWidget.addTab(holder, tabIcon(), tabTitle(i));
        if (i != static_cast<int>(eSIPages::PageOverview))
            mPendingInitTabs.append(i);
    }

    mTabWidget.setTabShape(QTabWidget::Rounded);
    mTabWidget.setTabsClosable(false);
    mTabWidget.setMovable(false);
    connect(&mTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        ensureTabInitialized(index);
    });

    ensureTabInitialized(static_cast<int>(eSIPages::PageOverview));
    QTimer::singleShot(TabInitStartDelayMs, this, &ServiceInterface::processQueuedTabInitialization);

    // Set the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&mTabWidget);
    setLayout(layout);
    
    SIDataTypeData& data = mModel.getData().getDataTypeData();
    connect(&data, &SIDataTypeData::signalDataTypeCreated   , this, &ServiceInterface::slotDataTypeCreated);
    connect(&data, &SIDataTypeData::signalDataTypeDeleted   , this, &ServiceInterface::slotDataTypeDeleted);
    connect(&data, &SIDataTypeData::signalDataTypeConverted , this, &ServiceInterface::slotDataTypeConverted);
    connect(&data, &SIDataTypeData::signalDataTypeUpdated   , this, &ServiceInterface::slotDataTypeUpdated);
    
    setAttribute(Qt::WA_DeleteOnClose);

    if (filePath.isEmpty() == false)
    {
        mIsUntitled = false;
        setCurrentFile(filePath);
    }
}

ServiceInterface::~ServiceInterface()
{
}

bool ServiceInterface::openSucceeded() const
{
    return mModel.openSucceeded();
}

QString ServiceInterface::newDocumentName()
{
    static uint32_t _seqNr{0};
    mDocName = newDocument() + QString::number(++_seqNr);
    return mDocName + newDocumentExt();
}

const QString& ServiceInterface::newDocument() const
{
    static const QString _newSIDoc{"NewServiceInterface"};
    return _newSIDoc;
}

const QString& ServiceInterface::newDocumentExt() const
{
    static const QString _extSI {".siml"};
    return _extSI;
}

const QString& ServiceInterface::fileSuffix() const
{
    static const QString _suffixSI{ "siml" };
    return _suffixSI;
}

const QString& ServiceInterface::fileFilter() const
{
    static const QString _filterSI{ "Service Interface document (*.siml)\nAll Files (*.*)" };
    return _filterSI;
}

bool ServiceInterface::writeToFile(const QString& filePath)
{
    if (mModel.saveToFile(filePath))
    {
        if (mOverview != nullptr)
            mOverview->setServiceInterfaceName(mModel.getName());
        
        return true;
    }
    else
    {
        return false;
    }
}

void ServiceInterface::slotDataTypeCreated(DataTypeCustom* dataType)
{
    if (mOverview != nullptr)  static_cast<IEDataTypeConsumer&>(*mOverview).dataTypeCreated(dataType);
    if (mDataType != nullptr)  static_cast<IEDataTypeConsumer&>(*mDataType).dataTypeCreated(dataType);
    if (mAttribute != nullptr) static_cast<IEDataTypeConsumer&>(*mAttribute).dataTypeCreated(dataType);
    if (mMethod != nullptr)    static_cast<IEDataTypeConsumer&>(*mMethod).dataTypeCreated(dataType);
    if (mConstant != nullptr)  static_cast<IEDataTypeConsumer&>(*mConstant).dataTypeCreated(dataType);
    if (mInclude != nullptr)   static_cast<IEDataTypeConsumer&>(*mInclude).dataTypeCreated(dataType);
}

void ServiceInterface::slotDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType)
{
    if (mOverview != nullptr)  static_cast<IEDataTypeConsumer&>(*mOverview).dataTypeConverted(oldType, newType);
    if (mDataType != nullptr)  static_cast<IEDataTypeConsumer&>(*mDataType).dataTypeConverted(oldType, newType);
    if (mAttribute != nullptr) static_cast<IEDataTypeConsumer&>(*mAttribute).dataTypeConverted(oldType, newType);
    if (mMethod != nullptr)    static_cast<IEDataTypeConsumer&>(*mMethod).dataTypeConverted(oldType, newType);
    if (mConstant != nullptr)  static_cast<IEDataTypeConsumer&>(*mConstant).dataTypeConverted(oldType, newType);
    if (mInclude != nullptr)   static_cast<IEDataTypeConsumer&>(*mInclude).dataTypeConverted(oldType, newType);
}

void ServiceInterface::slotDataTypeDeleted(DataTypeCustom* dataType)
{
    if (mOverview != nullptr)  static_cast<IEDataTypeConsumer&>(*mOverview).dataTypeDeleted(dataType);
    if (mDataType != nullptr)  static_cast<IEDataTypeConsumer&>(*mDataType).dataTypeDeleted(dataType);
    if (mAttribute != nullptr) static_cast<IEDataTypeConsumer&>(*mAttribute).dataTypeDeleted(dataType);
    if (mMethod != nullptr)    static_cast<IEDataTypeConsumer&>(*mMethod).dataTypeDeleted(dataType);
    if (mConstant != nullptr)  static_cast<IEDataTypeConsumer&>(*mConstant).dataTypeDeleted(dataType);
    if (mInclude != nullptr)   static_cast<IEDataTypeConsumer&>(*mInclude).dataTypeDeleted(dataType);
}

void ServiceInterface::slotDataTypeUpdated(DataTypeCustom* dataType)
{
    if (mOverview != nullptr)  static_cast<IEDataTypeConsumer&>(*mOverview).dataTypeUpdated(dataType);
    if (mDataType != nullptr)  static_cast<IEDataTypeConsumer&>(*mDataType).dataTypeUpdated(dataType);
    if (mAttribute != nullptr) static_cast<IEDataTypeConsumer&>(*mAttribute).dataTypeUpdated(dataType);
    if (mMethod != nullptr)    static_cast<IEDataTypeConsumer&>(*mMethod).dataTypeUpdated(dataType);
    if (mConstant != nullptr)  static_cast<IEDataTypeConsumer&>(*mConstant).dataTypeUpdated(dataType);
    if (mInclude != nullptr)   static_cast<IEDataTypeConsumer&>(*mInclude).dataTypeUpdated(dataType);
}

void ServiceInterface::attachPage(int index, QWidget* page)
{
    QWidget* holder = mTabWidget.widget(index);
    if ((holder != nullptr) && (holder->layout() != nullptr) && (page != nullptr))
    {
        holder->layout()->addWidget(page);
    }
}

bool ServiceInterface::isValidTabIndex(int index)
{
    return (index >= 0) && (index < static_cast<int>(eSIPages::PageIncludes) + 1);
}

bool ServiceInterface::isTabInitialized(int index) const
{
    switch (static_cast<eSIPages>(index))
    {
    case eSIPages::PageOverview:   return mOverview != nullptr;
    case eSIPages::PageDataTypes:  return mDataType != nullptr;
    case eSIPages::PageAttributes: return mAttribute != nullptr;
    case eSIPages::PageMethods:    return mMethod != nullptr;
    case eSIPages::PageConstants:  return mConstant != nullptr;
    case eSIPages::PageIncludes:   return mInclude != nullptr;
    default:                       return false;
    }
}

QString ServiceInterface::tabTitle(int index) const
{
    switch (static_cast<eSIPages>(index))
    {
    case eSIPages::PageOverview:   return tr("Overview");
    case eSIPages::PageDataTypes:  return tr("Data Types");
    case eSIPages::PageAttributes: return tr("Data Attributes");
    case eSIPages::PageMethods:    return tr("Methods");
    case eSIPages::PageConstants:  return tr("Constants");
    case eSIPages::PageIncludes:   return tr("Includes");
    default:                       return QString();
    }
}

void ServiceInterface::processQueuedTabInitialization()
{
    if (mPendingInitTabs.isEmpty())
        return;

    ensureTabInitialized(mPendingInitTabs.first());
    if (mPendingInitTabs.isEmpty() == false)
    {
        QTimer::singleShot(TabInitDelayMs, this, &ServiceInterface::processQueuedTabInitialization);
    }
}

void ServiceInterface::ensureTabInitialized(int index)
{
    if (isValidTabIndex(index) == false)
        return;

    mPendingInitTabs.removeAll(index);
    if (isTabInitialized(index))
        return;

    switch (static_cast<eSIPages>(index))
    {
    case eSIPages::PageOverview:
        if (mOverview == nullptr)
        {
            mOverview = new SIOverview(mModel.getOverviewModel(), &mTabWidget);
            connect(mOverview, &SIOverview::signalPageLinkClicked, this, &ServiceInterface::slotPageLinkClicked);
            attachPage(index, mOverview);
        }
        break;

    case eSIPages::PageDataTypes:
        if (mDataType == nullptr)
        {
            mDataType = new SIDataType(mModel.getDataTypeModel(), &mTabWidget);
            attachPage(index, mDataType);
        }
        break;

    case eSIPages::PageAttributes:
        if (mAttribute == nullptr)
        {
            mAttribute = new SIAttribute(mModel.getAttributeModel(), &mTabWidget);
            attachPage(index, mAttribute);
        }
        break;

    case eSIPages::PageMethods:
        if (mMethod == nullptr)
        {
            mMethod = new SIMethod(mModel.getMethodsModel(), &mTabWidget);
            attachPage(index, mMethod);
        }
        break;

    case eSIPages::PageConstants:
        if (mConstant == nullptr)
        {
            mConstant = new SIConstant(mModel.getConstantsModel(), &mTabWidget);
            attachPage(index, mConstant);
        }
        break;

    case eSIPages::PageIncludes:
        if (mInclude == nullptr)
        {
            mInclude = new SIInclude(mModel.getIncludesModel(), &mTabWidget);
            attachPage(index, mInclude);
        }
        break;

    default:
        break;
    }
}

void ServiceInterface::slotPageLinkClicked(int page)
{
    mTabWidget.setCurrentIndex(page);
}
