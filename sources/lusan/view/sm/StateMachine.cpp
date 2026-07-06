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
 *  \file        lusan/view/sm/StateMachine.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine MDI window.
 *
 ************************************************************************/

#include "lusan/view/sm/StateMachine.hpp"

#include "lusan/view/sm/SMAttribute.hpp"
#include "lusan/view/sm/SMConstant.hpp"
#include "lusan/view/sm/SMDataType.hpp"
#include "lusan/view/sm/SMOverview.hpp"

#include <QLabel>
#include <QMessageBox>
#include <QTimer>
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
    });

    ensureTabInitialized(static_cast<int>(PageOverview));
    QTimer::singleShot(TabInitStartDelayMs, this, &StateMachine::processQueuedTabInitialization);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&mTabWidget);
    setLayout(layout);

    connect(&mModel, &StateMachineModel::signalDirtyChanged, this, &MdiChild::setModified);
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
    else if (index == static_cast<int>(PageConstants))
    {
        page = new SMConstant(mModel.getConstantModel(), &mTabWidget);
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
