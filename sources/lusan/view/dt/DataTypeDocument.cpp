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
 *  \file        lusan/view/dt/DataTypeDocument.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document window.
 *
 ************************************************************************/

#include "lusan/view/dt/DataTypeDocument.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/dt/DTValidator.hpp"
#include "lusan/view/common/DataTypePage.hpp"
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/view/common/IncludePage.hpp"
#include "lusan/view/common/OverviewPage.hpp"

#include <QTimer>
#include <QVBoxLayout>

namespace
{
    constexpr int TabInitStartDelayMs{ 200 };
    constexpr int TabInitDelayMs{ 50 };

    const QIcon& tabIcon(void)
    {
        static const QIcon _icon{ NELusanCommon::iconDataTypeDocument(NELusanCommon::SizeSmall) };
        return _icon;
    }

    OverviewPageConfig makeOverviewConfig(void)
    {
        OverviewPageConfig config;
        config.headline         = QObject::tr("Data Type Document Overview ...");
        config.versionTitle     = QObject::tr("Document Version:");
        config.descriptionHint  = QObject::tr("Describe the data types collected here");
        // The document is named by the file it lives in, and a save writes that name back.
        config.nameEditable     = false;
        config.links            =
        {
              { static_cast<int>(DataTypeDocument::eDTPages::PageDataTypes), QStringLiteral("linkDataTypes")
              , QObject::tr("Data Types ..."), QObject::tr("Click to open the Data Types page")
              , QObject::tr("Open Data Types Page ...") }
            , { static_cast<int>(DataTypeDocument::eDTPages::PageIncludes) , QStringLiteral("linkIncludes")
              , QObject::tr("Includes ...") , QObject::tr("Click to open the Includes page")
              , QObject::tr("Open Includes Page ...") }
        };

        return config;
    }
}

const QString& DataTypeDocument::fileExtension(void)
{
    static const QString _extDT{ "dtml" };
    return _extDT;
}

DataTypeDocument::DataTypeDocument(MdiMainWindow* wndMain, const QString& filePath /*= QString()*/, QWidget* parent /*= nullptr*/)
    : MdiChild          (MdiChild::eMdiWindow::MdiDataType, wndMain, parent)

    , mModel            (filePath)
    , mTabWidget        (this)
    , mOverview         (nullptr)
    , mDataType         (nullptr)
    , mInclude          (nullptr)
    , mPendingInitTabs  ( )
    , mPendingEdits     (mModel.getNotifier(), this)
{
    mTabWidget.setTabPosition(QTabWidget::South);
    const int pageCount{ static_cast<int>(eDTPages::PageIncludes) + 1 };
    for (int i = 0; i < pageCount; ++i)
    {
        QWidget* holder = new QWidget(&mTabWidget);
        QVBoxLayout* holderLayout = new QVBoxLayout(holder);
        holderLayout->setContentsMargins(0, 0, 0, 0);
        mTabWidget.addTab(holder, tabIcon(), tabTitle(i));
        if (i != static_cast<int>(eDTPages::PageOverview))
            mPendingInitTabs.append(i);
    }

    mTabWidget.setTabShape(QTabWidget::Rounded);
    mTabWidget.setTabsClosable(false);
    mTabWidget.setMovable(false);
    connect(&mTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        ensureTabInitialized(index);
    });

    ensureTabInitialized(static_cast<int>(eDTPages::PageOverview));
    QTimer::singleShot(TabInitStartDelayMs, this, &DataTypeDocument::processQueuedTabInitialization);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&mTabWidget);
    setLayout(layout);

    // The title mark and the global Edit menu follow how far the history stands from the point
    // the document was last saved at.
    connect(&mModel.getUndoStack(), &QUndoStack::canUndoChanged, this, &MdiChild::signalCanUndoChanged);
    connect(&mModel.getUndoStack(), &QUndoStack::canRedoChanged, this, &MdiChild::signalCanRedoChanged);
    connect(&mModel.getUndoStack(), &QUndoStack::cleanChanged  , this, &DataTypeDocument::refreshModified);
    // A field gives its text to the document when it loses the focus, so between the first
    // keystroke and that moment the document is changed while the history still says otherwise.
    connect(&mPendingEdits, &PendingEditWatcher::signalPendingEditChanged, this, &DataTypeDocument::refreshModified);

    setAttribute(Qt::WA_DeleteOnClose);

    if (filePath.isEmpty() == false)
    {
        mIsUntitled = false;
        setCurrentFile(filePath);
    }
}

IEDocumentModel* DataTypeDocument::documentModel(void)
{
    return &mModel;
}

void DataTypeDocument::navigateToIssue(uint32_t elementId, eDocElementKind kind, int rule)
{
    int pageIndex = -1;
    switch (kind)
    {
    case eDocElementKind::Overview:     pageIndex = static_cast<int>(PageOverview);     break;
    case eDocElementKind::DataType:     pageIndex = static_cast<int>(PageDataTypes);    break;
    case eDocElementKind::Include:      pageIndex = static_cast<int>(PageIncludes);     break;
    default:                            return;
    }

    ensureTabInitialized(pageIndex);
    mTabWidget.setCurrentIndex(pageIndex);

    // The element alone says which entry to fix; the check often also says which of its fields.
    const eIssueField field = DTValidator::fieldOfRule(rule);
    switch (kind)
    {
    case eDocElementKind::Overview:     if (mOverview != nullptr) mOverview->revealField(field);                break;
    case eDocElementKind::DataType:     if (mDataType != nullptr) mDataType->revealElement(elementId, field);   break;
    case eDocElementKind::Include:      if (mInclude  != nullptr) mInclude->revealElement(elementId, field);    break;
    default:                                                                                                   break;
    }
}

bool DataTypeDocument::openSucceeded(void) const
{
    return mModel.openSucceeded();
}

QTabWidget* DataTypeDocument::pageTabWidget(void)
{
    return &mTabWidget;
}

QString DataTypeDocument::newDocumentName(void)
{
    static uint32_t _seqNr{ 0 };
    mDocName = newDocument() + QString::number(++_seqNr);
    return mDocName + newDocumentExt();
}

const QString& DataTypeDocument::newDocument(void) const
{
    static const QString _newDTDoc{ "NewDataTypes" };
    return _newDTDoc;
}

const QString& DataTypeDocument::newDocumentExt(void) const
{
    static const QString _extDT{ ".dtml" };
    return _extDT;
}

const QString& DataTypeDocument::fileSuffix(void) const
{
    return DataTypeDocument::fileExtension();
}

const QString& DataTypeDocument::fileFilter(void) const
{
    static const QString _filterDT{ "Data Type document (*.dtml)\nAll Files (*.*)" };
    return _filterDT;
}

void DataTypeDocument::undo(void)
{
    mModel.getUndoStack().undo();
}

void DataTypeDocument::redo(void)
{
    mModel.getUndoStack().redo();
}

bool DataTypeDocument::canUndo(void) const
{
    return mModel.getUndoStack().canUndo();
}

bool DataTypeDocument::canRedo(void) const
{
    return mModel.getUndoStack().canRedo();
}

bool DataTypeDocument::writeToFile(const QString& filePath)
{
    if (mModel.saveToFile(filePath) == false)
        return false;

    mModel.getUndoStack().setClean();
    // Saving under another file name renames the document, without going through the history.
    if (mOverview != nullptr)
    {
        mOverview->refreshAll();
    }

    return true;
}

void DataTypeDocument::commitPendingEdits(void)
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

    // What the caret still sits in has just been handed over with the rest.
    mPendingEdits.acceptPendingEdit();
}

void DataTypeDocument::refreshModified(void)
{
    setModified(mModel.isDirty() || mPendingEdits.hasPendingEdit());
}

void DataTypeDocument::attachPage(int index, QWidget* page)
{
    QWidget* holder = mTabWidget.widget(index);
    if ((holder != nullptr) && (holder->layout() != nullptr) && (page != nullptr))
    {
        holder->layout()->addWidget(page);
    }
}

bool DataTypeDocument::isValidTabIndex(int index)
{
    return (index >= 0) && (index < static_cast<int>(eDTPages::PageIncludes) + 1);
}

bool DataTypeDocument::isTabInitialized(int index) const
{
    switch (static_cast<eDTPages>(index))
    {
    case eDTPages::PageOverview:   return mOverview != nullptr;
    case eDTPages::PageDataTypes:  return mDataType != nullptr;
    case eDTPages::PageIncludes:   return mInclude != nullptr;
    default:                       return false;
    }
}

QString DataTypeDocument::tabTitle(int index) const
{
    switch (static_cast<eDTPages>(index))
    {
    case eDTPages::PageOverview:   return tr("Overview");
    case eDTPages::PageDataTypes:  return tr("Data Types");
    case eDTPages::PageIncludes:   return tr("Includes");
    default:                       return QString();
    }
}

void DataTypeDocument::processQueuedTabInitialization(void)
{
    if (mPendingInitTabs.isEmpty())
        return;

    ensureTabInitialized(mPendingInitTabs.first());
    if (mPendingInitTabs.isEmpty() == false)
    {
        QTimer::singleShot(TabInitDelayMs, this, &DataTypeDocument::processQueuedTabInitialization);
    }
}

void DataTypeDocument::ensureTabInitialized(int index)
{
    if (isValidTabIndex(index) == false)
        return;

    mPendingInitTabs.removeAll(index);
    if (isTabInitialized(index))
        return;

    switch (static_cast<eDTPages>(index))
    {
    case eDTPages::PageOverview:
        if (mOverview == nullptr)
        {
            mOverview = new OverviewPage(mModel.getOverviewModel(), makeOverviewConfig(), &mTabWidget);
            connect(mOverview, &OverviewPage::signalPageLinkClicked, this, &DataTypeDocument::slotPageLinkClicked);
            attachPage(index, mOverview);
        }
        break;

    case eDTPages::PageDataTypes:
        if (mDataType == nullptr)
        {
            mDataType = new DataTypePage(mModel.getDataTypeModel(), tr("Data Type Editor ..."), &mTabWidget);
            attachPage(index, mDataType);
        }
        break;

    case eDTPages::PageIncludes:
        if (mInclude == nullptr)
        {
            // A data type document is a leaf: it takes the C++ headers its types need, and
            // neither another data type document nor a document of any other kind.
            IncludeTypeConfig config{};
            config.takesDataTypes = false;
            mInclude = new IncludePage(mModel.getIncludesModel(), config
                                      , tr("Data Type Includes Editor ..."), &mTabWidget);
            attachPage(index, mInclude);
        }
        break;

    default:
        break;
    }
}

void DataTypeDocument::slotPageLinkClicked(int page)
{
    mTabWidget.setCurrentIndex(page);
}
