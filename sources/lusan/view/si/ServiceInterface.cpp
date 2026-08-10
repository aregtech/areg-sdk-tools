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
 *  \file        lusan/view/si/ServiceInterface.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include "lusan/view/si/ServiceInterface.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/view/common/AttributePage.hpp"
#include "lusan/view/common/ConstantPage.hpp"
#include "lusan/view/common/DataTypePage.hpp"
#include "lusan/view/common/IncludePage.hpp"
#include "lusan/view/si/SIMethod.hpp"
#include "lusan/view/si/SIOverview.hpp"
#include "lusan/model/si/SIValidator.hpp"

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
    
    // The pages that offer the document's data types, or declare their elements with them, follow
    // the Data Types page through the document notifier, so an undo or a redo reaches them too.
    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &ServiceInterface::slotDataTypesChanged);
    connect(&notifier, &DocModelNotifier::elementAdded  , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) slotDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) slotDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) slotDataTypesChanged(); });
    connect(&notifier, &DocModelNotifier::listReordered , this, [this](uint32_t, eDocElementKind kind) { if (kind == eDocElementKind::DataType) slotDataTypesChanged(); });

    // The title mark and the global Edit menu follow how far the history stands from the point
    // the document was last saved at.
    connect(&mModel.getUndoStack(), &QUndoStack::canUndoChanged, this, &MdiChild::signalCanUndoChanged);
    connect(&mModel.getUndoStack(), &QUndoStack::canRedoChanged, this, &MdiChild::signalCanRedoChanged);
    connect(&mModel.getUndoStack(), &QUndoStack::cleanChanged  , this, [this](bool clean) { setModified(clean == false); });

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

IEDocumentModel* ServiceInterface::documentModel()
{
    return &mModel;
}

void ServiceInterface::navigateToIssue(uint32_t elementId, eDocElementKind kind, int rule)
{
    int pageIndex = -1;
    switch (kind)
    {
    case eDocElementKind::Overview:     pageIndex = static_cast<int>(PageOverview);     break;
    case eDocElementKind::DataType:     pageIndex = static_cast<int>(PageDataTypes);    break;
    case eDocElementKind::Attribute:    pageIndex = static_cast<int>(PageAttributes);   break;
    case eDocElementKind::Method:       pageIndex = static_cast<int>(PageMethods);      break;
    case eDocElementKind::Constant:     pageIndex = static_cast<int>(PageConstants);    break;
    case eDocElementKind::Include:      pageIndex = static_cast<int>(PageIncludes);     break;
    default:                            return;
    }

    ensureTabInitialized(pageIndex);
    mTabWidget.setCurrentIndex(pageIndex);

    // The element alone says which entry to fix; the check often also says which of its fields.
    const eIssueField field = SIValidator::fieldOfRule(rule);
    switch (kind)
    {
    case eDocElementKind::Overview:     if (mOverview  != nullptr) mOverview->revealField(field);                break;
    case eDocElementKind::DataType:     if (mDataType  != nullptr) mDataType->revealElement(elementId, field);   break;
    case eDocElementKind::Attribute:    if (mAttribute != nullptr) mAttribute->revealElement(elementId, field);  break;
    case eDocElementKind::Method:       if (mMethod    != nullptr) mMethod->revealElement(elementId, field);     break;
    case eDocElementKind::Constant:     if (mConstant  != nullptr) mConstant->revealElement(elementId, field);   break;
    case eDocElementKind::Include:      if (mInclude   != nullptr) mInclude->revealElement(elementId, field);    break;
    default:                                                                                                     break;
    }
}

bool ServiceInterface::openSucceeded() const
{
    return mModel.openSucceeded();
}

QTabWidget* ServiceInterface::pageTabWidget()
{
    return &mTabWidget;
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

void ServiceInterface::undo()
{
    mModel.getUndoStack().undo();
}

void ServiceInterface::redo()
{
    mModel.getUndoStack().redo();
}

bool ServiceInterface::canUndo() const
{
    return mModel.getUndoStack().canUndo();
}

bool ServiceInterface::canRedo() const
{
    return mModel.getUndoStack().canRedo();
}

bool ServiceInterface::writeToFile(const QString& filePath)
{
    if (mModel.saveToFile(filePath))
    {
        mModel.getUndoStack().setClean();
        if (mOverview != nullptr)
            mOverview->setServiceInterfaceName(mModel.getName());
        
        return true;
    }
    else
    {
        return false;
    }
}

void ServiceInterface::slotDataTypesChanged()
{
    // The Data Types page and the Constants page follow the notifier themselves; the rest are
    // told here, because they still hold their own copies of the type list.
    if (mOverview != nullptr)  static_cast<IEDataTypeConsumer&>(*mOverview).dataTypesChanged();
    if (mAttribute != nullptr) static_cast<IEDataTypeConsumer&>(*mAttribute).dataTypesChanged();
    if (mMethod != nullptr)    static_cast<IEDataTypeConsumer&>(*mMethod).dataTypesChanged();
}

void ServiceInterface::commitPendingEdits(void)
{
    // A description box gives its text to the document on focus loss, and Ctrl+S leaves the caret
    // where it is. Collect from every page, so the file gets the text the author already wrote.
    const QList<QWidget*> widgets = findChildren<QWidget*>();
    for (QWidget* widget : widgets)
    {
        IEditCommit* editor = dynamic_cast<IEditCommit*>(widget);
        if (editor != nullptr)
        {
            editor->commitPendingEdits();
        }
    }
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
            mDataType = new DataTypePage(mModel.getDataTypeModel(), tr("Service Interface Data Type Editor ..."), &mTabWidget);
            attachPage(index, mDataType);
        }
        break;

    case eSIPages::PageAttributes:
        if (mAttribute == nullptr)
        {
            mAttribute = new AttributePage(mModel.getAttributeModel(), tr("Service Interface Data Attribute Editor ..."), &mTabWidget);
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
            mConstant = new ConstantPage(mModel.getConstantsModel(), tr("Service Interface Constants Editor ..."), &mTabWidget);
            attachPage(index, mConstant);
        }
        break;

    case eSIPages::PageIncludes:
        if (mInclude == nullptr)
        {
            // An interface is an API contract and includes no other contract, so the page is
            // configured with no document kind of its own -- only headers and data types.
            mInclude = new IncludePage(mModel.getIncludesModel(), IncludeTypeConfig{}
                                      , tr("Service Interface Includes Editor ..."), &mTabWidget);
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
