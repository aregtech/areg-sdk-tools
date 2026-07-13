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
 *  \file        lusan/view/sm/StateMachine.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine MDI window.
 *
 ************************************************************************/

#include "lusan/view/sm/StateMachine.hpp"

#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NavigationDock.hpp"
#include "lusan/view/sm/SMAttribute.hpp"
#include "lusan/view/sm/SMAttributeList.hpp"
#include "lusan/view/sm/SMConstant.hpp"
#include "lusan/view/sm/SMConstantList.hpp"
#include "lusan/view/sm/SMDataType.hpp"
#include "lusan/view/sm/SMDataTypeList.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMEvent.hpp"
#include "lusan/view/sm/SMEventList.hpp"
#include "lusan/view/sm/SMInclude.hpp"
#include "lusan/view/sm/SMMethod.hpp"
#include "lusan/view/sm/SMMethodList.hpp"
#include "lusan/view/sm/SMOverview.hpp"

#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    constexpr int TabInitStartDelayMs{ 200 };
    constexpr int TabInitDelayMs{ 50 };
}

const QString& StateMachine::fileExtension()
{
    static const QString _extFSM{ "fsml" };
    return _extFSM;
}

StateMachine::StateMachine(MdiMainWindow* wndMain, const QString& filePath /*= QString()*/, const QString& sourcePath /*= QString()*/, QWidget* parent /*= nullptr*/)
    : MdiChild           (MdiChild::eMdiWindow::MdiStateMachine, wndMain, parent)
    , mModel             (this)
    , mTabWidget         (this)
    , mOverview          (nullptr)
    , mPages             (pageCount(), nullptr)
    , mPendingInitTabs   ( )
    , mToolbarVisible    (true)
{
    if (filePath.isEmpty() == false)
    {
        mIsUntitled = false;
        if (loadDocument(filePath, sourcePath))
        {
            setCurrentFile(filePath);
        }
    }

    mTabWidget.setTabPosition(QTabWidget::South);
    mTabWidget.setTabShape(QTabWidget::Rounded);
    mTabWidget.setTabsClosable(false);
    mTabWidget.setMovable(false);
    for (int i = 0; i < pageCount(); ++i)
    {
        QWidget* holder = new QWidget(&mTabWidget);
        QVBoxLayout* holderLayout = new QVBoxLayout(holder);
        holderLayout->setContentsMargins(0, 0, 0, 0);
        mTabWidget.addTab(holder, tabTitle(i));
        if (i != static_cast<int>(PageOverview))
            mPendingInitTabs.append(i);
    }

    connect(&mTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        ensureTabInitialized(index);
        // The toolbar and the Properties/Outline panels are active only on the Design page
        // (issue #516). When they are hosted in the Navigation Window they must bind/unbind as
        // the user enters or leaves the Design tab, so let the main window re-sync them.
        if (mMainWindow != nullptr)
        {
            mMainWindow->syncDesignWidgets();
        }
    });

    ensureTabInitialized(static_cast<int>(PageOverview));
    QTimer::singleShot(TabInitStartDelayMs, this, &StateMachine::processQueuedTabInitialization);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&mTabWidget);
    setLayout(layout);

    connect(&mModel, &StateMachineModel::signalDirtyChanged, this, &MdiChild::setModified);
    connect(&mModel.getUndoStack(), &QUndoStack::canUndoChanged, this, &MdiChild::signalCanUndoChanged);
    connect(&mModel.getUndoStack(), &QUndoStack::canRedoChanged, this, &MdiChild::signalCanRedoChanged);
    setAttribute(Qt::WA_DeleteOnClose);
}

void StateMachine::newFile()
{
    MdiChild::newFile();
    if (mModel.createNewDocument(mDocName) == false)
    {
        setModified(false);
    }
}

bool StateMachine::openSucceeded() const
{
    return mModel.openSucceeded();
}

void StateMachine::undo()
{
    mModel.getUndoStack().undo();
}

void StateMachine::redo()
{
    mModel.getUndoStack().redo();
}

bool StateMachine::canUndo() const
{
    return mModel.getUndoStack().canUndo();
}

bool StateMachine::canRedo() const
{
    return mModel.getUndoStack().canRedo();
}

void StateMachine::cut()
{
    SMDesign* design = designPageIfBuilt();
    if ((design != nullptr) && isDesignPageCurrent())
    {
        design->cutSelection();
    }
}

void StateMachine::copy()
{
    SMDesign* design = designPageIfBuilt();
    if ((design != nullptr) && isDesignPageCurrent())
    {
        design->copySelection();
    }
}

void StateMachine::paste()
{
    SMDesign* design = designPageIfBuilt();
    if ((design != nullptr) && isDesignPageCurrent())
    {
        design->pasteClipboard();
    }
}

void StateMachine::setToolbarVisible(bool visible)
{
    mToolbarVisible = visible;
    if (isTabInitialized(static_cast<int>(PageDesign)))
    {
        SMDesign* design = qobject_cast<SMDesign*>(mPages.at(static_cast<int>(PageDesign)));
        if (design != nullptr)
        {
            design->setToolbarVisible(visible);
        }
    }
}

bool StateMachine::isToolbarVisible() const
{
    return mToolbarVisible;
}

SMDesign* StateMachine::designPageIfBuilt() const
{
    return isTabInitialized(static_cast<int>(PageDesign))
            ? qobject_cast<SMDesign*>(mPages.at(static_cast<int>(PageDesign)))
            : nullptr;
}

bool StateMachine::isDesignPageCurrent() const
{
    return (mTabWidget.currentIndex() == static_cast<int>(PageDesign));
}

void StateMachine::onDeclareRequested(SMDesign::eDeclareKind kind)
{
    int pageIndex = static_cast<int>(PageOverview);
    switch (kind)
    {
    case SMDesign::eDeclareKind::Trigger:
    case SMDesign::eDeclareKind::Action:
    case SMDesign::eDeclareKind::Condition:
        pageIndex = static_cast<int>(PageMethods);
        break;

    case SMDesign::eDeclareKind::Event:
    case SMDesign::eDeclareKind::Timer:
        pageIndex = static_cast<int>(PageEvents);
        break;

    case SMDesign::eDeclareKind::Attribute:
        pageIndex = static_cast<int>(PageAttributes);
        break;

    case SMDesign::eDeclareKind::Constant:
        pageIndex = static_cast<int>(PageConstants);
        break;

    case SMDesign::eDeclareKind::DataType:
        pageIndex = static_cast<int>(PageDataTypes);
        break;
    }

    ensureTabInitialized(pageIndex);
    mTabWidget.setCurrentIndex(pageIndex);

    QWidget* page = mPages.at(pageIndex);
    switch (kind)
    {
    case SMDesign::eDeclareKind::Trigger:
    {
        SMMethodList* list = static_cast<SMMethod*>(page)->getList();
        if (list->actionNewTrigger() != nullptr) list->actionNewTrigger()->trigger();
        break;
    }
    case SMDesign::eDeclareKind::Action:
    {
        SMMethodList* list = static_cast<SMMethod*>(page)->getList();
        if (list->actionNewAction() != nullptr) list->actionNewAction()->trigger();
        break;
    }
    case SMDesign::eDeclareKind::Condition:
    {
        SMMethodList* list = static_cast<SMMethod*>(page)->getList();
        if (list->actionNewCondition() != nullptr) list->actionNewCondition()->trigger();
        break;
    }
    case SMDesign::eDeclareKind::Event:
    {
        SMEventList* list = static_cast<SMEvent*>(page)->getList();
        if (list->actionNewEvent() != nullptr) list->actionNewEvent()->trigger();
        break;
    }
    case SMDesign::eDeclareKind::Timer:
    {
        SMEventList* list = static_cast<SMEvent*>(page)->getList();
        if (list->actionNewTimer() != nullptr) list->actionNewTimer()->trigger();
        break;
    }
    case SMDesign::eDeclareKind::Attribute:
    {
        QToolButton* button = static_cast<SMAttribute*>(page)->getList()->ctrlButtonAdd();
        if (button != nullptr) button->click();
        break;
    }
    case SMDesign::eDeclareKind::Constant:
    {
        QToolButton* button = static_cast<SMConstant*>(page)->getList()->ctrlButtonAdd();
        if (button != nullptr) button->click();
        break;
    }
    case SMDesign::eDeclareKind::DataType:
    {
        QToolButton* button = static_cast<SMDataType*>(page)->getList()->ctrlButtonAdd();
        if (button != nullptr) button->click();
        break;
    }
    }
}

QString StateMachine::newDocumentName()
{
    static uint32_t _seqNr{ 0 };
    mDocName = newDocument() + QString::number(++_seqNr);
    return mDocName + newDocumentExt();
}

const QString& StateMachine::newDocument() const
{
    static const QString _newFSMDoc{ "NewStateMachine" };
    return _newFSMDoc;
}

const QString& StateMachine::newDocumentExt() const
{
    static const QString _extFSM{ ".fsml" };
    return _extFSM;
}

const QString& StateMachine::fileSuffix() const
{
    static const QString _suffixFSM{ "fsml" };
    return _suffixFSM;
}

const QString& StateMachine::fileFilter() const
{
    static const QString _filterFSM{ "State Machine document (*.fsml)\nAll Files (*.*)" };
    return _filterFSM;
}

bool StateMachine::writeToFile(const QString& filePath)
{
    return mModel.saveToFile(filePath);
}

bool StateMachine::maybeSave()
{
    if (isModified() == false)
    {
        return true;
    }

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("MDI"),
                               tr("'%1' has been modified.\nDo you want to save your changes?")
                                   .arg(userFriendlyCurrentFile()),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret)
    {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Discard:
        mModel.removeAutosave();
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        return true;
    }
}

void StateMachine::onWindowClosing(bool isActive)
{
    Q_UNUSED(isActive);
    if (isModified() == false)
    {
        mModel.removeAutosave();
    }
}

bool StateMachine::loadDocument(const QString& documentPath, const QString& sourcePath /*= QString()*/)
{
    return mModel.loadFromFile(documentPath, sourcePath);
}

bool StateMachine::isValidTabIndex(int index)
{
    return (index >= 0) && (index < pageCount());
}

bool StateMachine::isTabInitialized(int index) const
{
    return isValidTabIndex(index) && (mPages.at(index) != nullptr);
}

QString StateMachine::tabTitle(int index) const
{
    switch (static_cast<eSMPages>(index))
    {
    case PageOverview:   return tr("Overview");
    case PageDataTypes:  return tr("Data Types");
    case PageAttributes: return tr("Attributes");
    case PageEvents:     return tr("Events");
    case PageMethods:    return tr("Methods");
    case PageConstants:  return tr("Constants");
    case PageIncludes:   return tr("Includes");
    case PageDesign:     return tr("Design");
    default:             return QString();
    }
}

void StateMachine::processQueuedTabInitialization()
{
    if (mPendingInitTabs.isEmpty())
        return;

    ensureTabInitialized(mPendingInitTabs.first());
    if (mPendingInitTabs.isEmpty() == false)
    {
        QTimer::singleShot(TabInitDelayMs, this, &StateMachine::processQueuedTabInitialization);
    }
}

void StateMachine::ensureTabInitialized(int index)
{
    if (isValidTabIndex(index) == false)
        return;

    mPendingInitTabs.removeAll(index);
    if (isTabInitialized(index))
        return;

    QWidget* page = nullptr;
    if (index == static_cast<int>(PageOverview))
    {
        mOverview = new SMOverview(mModel.getOverviewModel(), &mTabWidget);
        connect(mOverview, &SMOverview::signalPageLinkClicked, this, [this](int page) {
            mTabWidget.setCurrentIndex(page);
        });
        page = mOverview;
    }
    else if (index == static_cast<int>(PageDataTypes))
    {
        page = new SMDataType(mModel.getDataTypeModel(), &mTabWidget);
    }
    else if (index == static_cast<int>(PageAttributes))
    {
        page = new SMAttribute(mModel.getAttributeModel(), &mTabWidget);
    }
    else if (index == static_cast<int>(PageEvents))
    {
        page = new SMEvent(mModel.getEventModel(), mModel.getTimerModel(), &mTabWidget);
    }
    else if (index == static_cast<int>(PageMethods))
    {
        page = new SMMethod(mModel.getMethodModel(), &mTabWidget);
    }
    else if (index == static_cast<int>(PageConstants))
    {
        page = new SMConstant(mModel.getConstantModel(), &mTabWidget);
    }
    else if (index == static_cast<int>(PageIncludes))
    {
        page = new SMInclude(mModel.getIncludeModel(), &mTabWidget);
    }
    else if (index == static_cast<int>(PageDesign))
    {
        SMDesign* design = new SMDesign(mModel, &mTabWidget);
        design->setToolbarVisible(mToolbarVisible);
        connect(design, &SMDesign::signalDeclareRequested, this, &StateMachine::onDeclareRequested);
        // The canvas View submenu moves the toolbar / Properties / Outline between the Design
        // page and the Navigation Window; the main window owns that global placement (issue #516).
        connect(design, &SMDesign::signalPlaceDesignWidget, this, [this](int widget, int place) {
            if (mMainWindow != nullptr)
            {
                mMainWindow->setDesignWidgetPlacement(static_cast<MdiMainWindow::eDesignWidget>(widget)
                                                    , static_cast<MdiMainWindow::eDesignPlace>(place));
            }
        });
        page = design;
    }
    else
    {
        // Placeholder until the owning task builds the page.
        page = new QLabel(tabTitle(index), &mTabWidget);
    }

    mPages[index] = page;
    attachPage(index, page);
}

void StateMachine::attachPage(int index, QWidget* page)
{
    QWidget* holder = mTabWidget.widget(index);
    if ((holder != nullptr) && (holder->layout() != nullptr) && (page != nullptr))
    {
        holder->layout()->addWidget(page);
    }
}
