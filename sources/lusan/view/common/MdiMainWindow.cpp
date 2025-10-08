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
 *  \file        lusan/view/common/MdiMainWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/
#include "lusan/view/common/MdiMainWindow.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/view/si/ServiceInterface.hpp"
#include "lusan/view/common/ProjectSettings.hpp"
#include "lusan/view/common/OptionPageLogging.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/OfflineLogViewer.hpp"

#include "areg/base/NESocket.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QStatusBar>
#include <QToolBar>

namespace
{
    inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
    inline QString fileKey() { return QStringLiteral("file"); }

    QStringList readRecentFiles(QSettings& settings)
    {
        QStringList result;
        const int count = settings.beginReadArray(recentFilesKey());
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            result.append(settings.value(fileKey()).toString());
        }
        
        settings.endArray();
        return result;
    }

    void writeRecentFiles(const QStringList& files, QSettings& settings)
    {
        const int count = files.size();
        settings.beginWriteArray(recentFilesKey());
        for (int i = 0; i < count; ++i)
        {
            settings.setArrayIndex(i);
            settings.setValue(fileKey(), files.at(i));
        }
        
        settings.endArray();
    }
}

inline QString MdiMainWindow::_filterServiceFiles(void)
{
    return QString{"Service Interface Document (*.siml);;All Files (*.*)"};
}

inline QString MdiMainWindow::_filterLoggingFiles(void)
{
    return QString{"Log Database Files (*.sqlog);;All Files (*.*)"};
}

MdiMainWindow::MdiMainWindow()
    : QMainWindow   ( )
    , mWorkspaceRoot( )
    , mLastFile     ( )
    , mMdiArea      ( this )
    , mNaviDock     ( this )
    , mOutputDock   ( this )
    , mLogViewer    ( nullptr )
    , mLiveLogWnd   ( nullptr )

    , mFileMenu     (nullptr)
    , mEditMenu     (nullptr)
    , mViewMenu     (nullptr)
    , mDesignMenu   (nullptr)
    , mLoggingMenu  (nullptr)
    , mToolsMenu    (nullptr)
    , mWindowMenu   (nullptr)
    , mHelpMenu     (nullptr)

    , mFileToolBar  (nullptr)
    , mEditToolBar  (nullptr)
    , mViewToolBar  (nullptr)
    , mActNewWorkspace(this)
    , mActFileNewSI (this)
    , mActFileNewLog(this)
    , mActFileOfflineLog(this)
    , mActFileOpen  (this)
    , mActFileSave  (this)
    , mActFileSaveAs(this)
    , mActFileClose (this)
    , mActFileCloseAll(this)
    , mActFileExit  (this)
    , mFileSeparator(nullptr)
    , mActFileRecent(nullptr)
    , mActEditCut   (this)
    , mActEditCopy  (this)
    , mActEditPaste (this)
    , mActViewNavigator(this)
    , mActViewWokspace(this)
    , mActViewLogs  (this)
    , mActOffViewLogs(this)
    , mActViewOutput(this)
    , mActWindowsTile(this)
    , mActWindowsCascade(this)
    , mActWindowsNext(this)
    , mActWindowsPrev(this)
    , mActWindowMenuSeparator(this)
    , mActHelpAbout (nullptr)
{
    _createActions();
    _createMenus();
    _createToolBars();
    _createStatusBar();
    _createDockWindows();
    _createMdiArea();
    
    onShowMenuWindow();
    onSubWindowActivated(nullptr);
    readSettings();

    setWindowTitle(tr("Lusan"));
    setUnifiedTitleAndToolBarOnMac(true);
}

const QString& MdiMainWindow::fileFilters(void) const
{
    static const QString _filter {
        "Service Interface Document (*.siml)\n"
        "Log Database Files (*.sqlog)\n"
        "All Files (*.*)"
    };
    
    return _filter;
}
    
bool MdiMainWindow::openFile(const QString& fileName)
{
    bool result{ false };
    if (QMdiSubWindow* existing = findMdiChild(fileName))
    {
        mMdiArea.setActiveSubWindow(existing);
        result = true;
    }
    else if (loadFile(fileName))
    {
        statusBar()->showMessage(tr("File loaded"), 2000);
        result = true;
    }

    return result;
}

bool MdiMainWindow::loadFile(const QString& fileName)
{
    bool succeeded{false};
    MdiChild* child = createMdiChild(fileName);
    if (child != nullptr)
    {
        succeeded = true;
        child->show();
        mLastFile = fileName;
        MdiMainWindow::appendToRecentFiles(fileName);
    }

    return succeeded;
}

void MdiMainWindow::logCollecttorConnected(bool isConnected, const QString& address, uint16_t port, const QString& dbPath)
{
    if (mLogViewer != nullptr)
    {
        mLogViewer->logServiceConnected(isConnected, address, port, dbPath);
        if (isConnected == false)
        {
            // Copy logs to offline log viewer
            OfflineLogViewer* offlineLog = createOfflineLogViewer(QString(), true);
            mNaviDock.getLiveScopes().setLoggingModel(nullptr);
            mNaviDock.showTab(NavigationDock::eNaviWindow::NaviOfflineLogs);
            offlineLog->show();

            // Properly close and delete the live log window and viewer
            if (mLiveLogWnd != nullptr)
            {
                mLiveLogWnd->close();
                mMdiArea.removeSubWindow(mLiveLogWnd);
                mLiveLogWnd = nullptr;
            }

            if (mLogViewer != nullptr)
            {
                mLogViewer->close();
                delete mLogViewer;
                mLogViewer = nullptr;
            }
        }
        else if (mLogViewer != nullptr)
        {
            mNaviDock.showTab(NavigationDock::eNaviWindow::NaviLiveLogs);
            mLogViewer->show();
        }
    }
}

LiveLogsModel * MdiMainWindow::setupLiveLogging(void)
{
    if (mLogViewer == nullptr)
    {
        onFileNewLiveLog();

        Q_ASSERT(mLogViewer != nullptr);
        Q_ASSERT(mLiveLogWnd != nullptr);

        mLiveLogWnd->activateWindow();
    }
    else
    {
        Q_ASSERT(mLiveLogWnd != nullptr);
        mMdiArea.setActiveSubWindow(mLiveLogWnd);
    }

    return static_cast<LiveLogsModel *>(mLogViewer->getLoggingModel());
}

LiveLogsModel* MdiMainWindow::getLiveLogging(void) const
{
    return (mLogViewer != nullptr ? static_cast<LiveLogsModel *>(mLogViewer->getLoggingModel()) : nullptr);
}

NaviFileSystem& MdiMainWindow::getNaviFileSystem(void)
{
    return mNaviDock.getFileSystem();
}

NaviLiveLogsScopes& MdiMainWindow::getNaviLiveScopes(void)
{
    return mNaviDock.getLiveScopes();
}

NaviOfflineLogsScopes& MdiMainWindow::getNaviOfflineScopes(void)
{
    return mNaviDock.getOfflineScopes();
}

ScopeOutputViewer& MdiMainWindow::getOutputScopeLogs(void)
{
    return mOutputDock.getScopeLogsView();
}

void MdiMainWindow::logDatabaseCreated(const QString& dbPath)
{
    if (mLogViewer != nullptr)
    {
        mLogViewer->logDatabaseCreated(dbPath);
    }
}

int MdiMainWindow::showOptionPageLogging(const QString& address, const QString& hostName, uint16_t port, const QString& logFile, const QString& logLocation)
{
    ProjectSettings settings(this);

    emit signalOptionsOpening();
    if ((address.isEmpty() == false) && (port != NESocket::InvalidPort) && (logFile.isEmpty() == false) && (logLocation.isEmpty() == false))
    {
        settings.getSettingLog()->setData(address, hostName, port, logFile, logLocation);
    }
    
    settings.activatePage(ProjectSettings::eOptionPage::PageLogging);
    int result = settings.exec();
    emit signalOptionsClosed(result == static_cast<int>(QDialog::Accepted));

    return result;
}

void MdiMainWindow::showNaviTab(NavigationDock::eNaviWindow naviTab)
{
    mNaviDock.showTab(naviTab);
}

QString MdiMainWindow::openLogFile(void)
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Log Database"), LusanApplication::getWorkspaceLogs(), _filterLoggingFiles());
    return (filePath.isEmpty() == false && openFile(filePath) ? filePath : QString(""));
}

void MdiMainWindow::closeEvent(QCloseEvent* event)
{
    emit signalMainwindowClosing();
    mMdiArea.closeAllSubWindows();
    if (mMdiArea.currentSubWindow())
    {
        event->ignore();
    }
    else
    {
        writeSettings();
        event->accept();
    }
}

void MdiMainWindow::onFileNewSI()
{
    ServiceInterface* child = createServiceInterfaceView();
    child->newFile();
    child->show();
}

void MdiMainWindow::onFileNewLiveLog()
{
    if (mLogViewer != nullptr)
    {
        bool found{false};
        QList<QMdiSubWindow *> subwindows = mMdiArea.subWindowList();
        for (QList<QMdiSubWindow *>::const_iterator sub = subwindows.constBegin(); sub != subwindows.constEnd(); ++sub)
        {
            if ((*sub) == mLiveLogWnd)
            {
                found = true;
                break;
            }
        }
        
        mLogViewer = found ? mLogViewer : nullptr;
    }
    
    if (mLogViewer == nullptr)
    {
        mLogViewer = new LiveLogViewer(this, &mMdiArea);
        mLiveLogWnd = mMdiArea.addSubWindow(mLogViewer);
        mLiveLogWnd->setWindowIcon(NELusanCommon::iconLiveLogWindow(NELusanCommon::SizeSmall));
        mLogViewer->setMdiSubwindow(mLiveLogWnd);
        mMdiArea.showMaximized();
        mLogViewer->show();
    }
    else
    {
        mMdiArea.setActiveSubWindow(mLiveLogWnd);
    }
}

void MdiMainWindow::onFileOpen()
{
    QFileInfo info(mLastFile);
    QString dir(info.absoluteDir().canonicalPath());
    QString filter(fileFilters());
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open Document"), dir, filter);
    if (!fileName.isEmpty())
    {
        openFile(fileName);
    }
}

void MdiMainWindow::onFileSave()
{
    MdiChild * active = activeMdiChild();
    if ( (active != nullptr) && active->save())
    {
        mLastFile = active->currentFile();
        statusBar()->showMessage(tr("File saved"), 2000);
    }
}

void MdiMainWindow::onFileSaveAs()
{
    MdiChild* child = activeMdiChild();
    if ((child != nullptr) && child->saveAs())
    {
        statusBar()->showMessage(tr("File saved"), 2000);
        MdiMainWindow::appendToRecentFiles(child->currentFile());
    }
}

void MdiMainWindow::onFileOpenRecent()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
    {
        openFile(action->data().toString());
    }
}

void MdiMainWindow::onFileExit(void)
{
    QMainWindow::close();
}

void MdiMainWindow::onEditCut()
{
    MdiChild* active = activeMdiChild();
    if (active != nullptr)
    {
        active->cut();
    }
}

void MdiMainWindow::onEditCopy()
{
    MdiChild* active = activeMdiChild();
    if (active != nullptr)
    {
        active->copy();
    }
}

void MdiMainWindow::onEditPaste()
{
    MdiChild* active = activeMdiChild();
    if (active != nullptr)
    {
        active->paste();
    }
}

void MdiMainWindow::onToolsOptions(void)
{
    ProjectSettings settings(this);
    emit signalOptionsOpening();
    int result = settings.exec();
    emit signalOptionsClosed(result == static_cast<int>(QDialog::Accepted));
}

void MdiMainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About Lusan"), tr("The <b>Lusan</b> in under construction."));
}

bool MdiMainWindow::hasRecentFiles()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MdiMainWindow::appendToRecentFiles(const QString& fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
    {
        writeRecentFiles(recentFiles, settings);
    }

    setRecentFilesVisibility(!recentFiles.isEmpty());
}

void MdiMainWindow::setRecentFilesVisibility(bool visible)
{
    mActFileRecent->setVisible(visible);
    mFileSeparator->setVisible(visible);
}

void MdiMainWindow::onShowMenuRecent()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for (; i < count; ++i)
    {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        mActsRecentFiles[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        mActsRecentFiles[i]->setData(recentFiles.at(i));
        mActsRecentFiles[i]->setVisible(true);
    }

    for (; i < MaxRecentFiles; ++i)
    {
        mActsRecentFiles[i]->setVisible(false);
    }
}

void MdiMainWindow::onShowMenuWindow()
{
    mWindowMenu->clear();
    mWindowMenu->addAction(&mActFileClose);
    mWindowMenu->addAction(&mActFileCloseAll);
    mWindowMenu->addSeparator();
    mWindowMenu->addAction(&mActWindowsTile);
    mWindowMenu->addAction(&mActWindowsCascade);
    mWindowMenu->addSeparator();
    mWindowMenu->addAction(&mActWindowsNext);
    mWindowMenu->addAction(&mActWindowsPrev);
    mWindowMenu->addAction(&mActWindowMenuSeparator);

    QList<QMdiSubWindow*> windows = mMdiArea.subWindowList();
    mActWindowMenuSeparator.setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i)
    {
        QMdiSubWindow* mdiSubWindow = windows.at(i);
        MdiChild* child = qobject_cast<MdiChild*>(mdiSubWindow->widget());

        QString text;
        if (i < 9)
        {
            text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }
        else
        {
            text = tr("%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }

        QAction* action = mWindowMenu->addAction(  text
                                                , mdiSubWindow
                                                , [this, mdiSubWindow]() {
                                                    mMdiArea.setActiveSubWindow(mdiSubWindow);
                                                    }
        );

        action->setCheckable(true);
        action->setChecked(child == activeMdiChild());
    }
}

void MdiMainWindow::onMdiChildClosed(MdiChild* mdiChild)
{
    if ((mdiChild != nullptr) && (MdiChild::MdiLogViewer == mdiChild->getMdiWindowType()))
    {
        mLogViewer = nullptr;
        mLiveLogWnd = nullptr;
    }

    emit signalMdiWindowClosed(mdiChild);
}

void MdiMainWindow::onSubWindowActivated(QMdiSubWindow* mdiSubWindow)
{
    MdiChild * mdiActive = mdiSubWindow != nullptr ? qobject_cast<MdiChild *>(mdiSubWindow->widget()) : nullptr;
    bool hasMdiChild = (mdiActive != nullptr);
    mActFileSave.setEnabled(hasMdiChild);
    mActFileSaveAs.setEnabled(hasMdiChild);
    mActEditPaste.setEnabled(hasMdiChild);
    mActFileClose.setEnabled(hasMdiChild);
    mActFileCloseAll.setEnabled(hasMdiChild);
    mActWindowsTile.setEnabled(hasMdiChild);
    mActWindowsCascade.setEnabled(hasMdiChild);
    mActWindowsNext.setEnabled(hasMdiChild);
    mActWindowsPrev.setEnabled(hasMdiChild);
    mActWindowMenuSeparator.setVisible(hasMdiChild);

    bool hasSelection = false;
    mActEditCut.setEnabled(hasSelection);
    mActEditCopy.setEnabled(hasSelection);

    if (mdiActive != nullptr)
    {
        mdiActive->onWindowActivated();
    }
}

MdiChild* MdiMainWindow::createMdiChild(const QString& filePath /*= QString()*/)
{
    MdiChild* result{nullptr};
    QFileInfo info(filePath);
    QString ext{info.suffix()};
    if (ext == ServiceInterface::fileExtension())
    {
        result = createServiceInterfaceView(filePath);
    }
    else if (ext == OfflineLogViewer::fileExtension())
    {
        result = createOfflineLogViewer(filePath, false);
    }
    
    return result;
}

ServiceInterface* MdiMainWindow::createServiceInterfaceView(const QString& filePath /*= QString()*/)
{
    ServiceInterface* child = new ServiceInterface(this, filePath, &mMdiArea);
    QMdiSubWindow* mdiSub = mMdiArea.addSubWindow(child);
    child->setMdiSubwindow(mdiSub);
    mdiSub->setWindowIcon(NELusanCommon::iconServiceInterface(NELusanCommon::SizeSmall));
    mdiSub->setWindowModified(true);
    mdiSub->setWindowFilePath(filePath);
    mdiSub->setToolTip(filePath);
    if (filePath.isEmpty() == false)
    {
        std::string stdFilePath{filePath.toStdString()};
        mdiSub->setWindowTitle(QString::fromStdString(File::getFileNameWithExtension(stdFilePath.c_str()).getData()));
    }

    mMdiArea.showMaximized();
    return child;
}

LiveLogViewer* MdiMainWindow::createLogViewerView(const QString& filePath /*= QString()*/)
{
    LiveLogViewer* child = new LiveLogViewer(this, &mMdiArea);
    QMdiSubWindow* mdiSub = mMdiArea.addSubWindow(child);
    child->setMdiSubwindow(mdiSub);
    mdiSub->setWindowIcon(NELusanCommon::iconLiveLogWindow(NELusanCommon::SizeSmall));
    mdiSub->setWindowFilePath(filePath);
    mdiSub->setToolTip(filePath);
    if (filePath.isEmpty() == false)
    {
        std::string stdFilePath{filePath.toStdString()};
        mdiSub->setWindowTitle(QString::fromStdString(File::getFileNameWithExtension(stdFilePath.c_str()).getData()));
    }
    mMdiArea.showMaximized();
    mNaviDock.showTab(NavigationDock::eNaviWindow::NaviLiveLogs);
    return child;
}

OfflineLogViewer* MdiMainWindow::createOfflineLogViewer(const QString& filePath, bool cloneLive)
{
    OfflineLogViewer* child = cloneLive && (mLogViewer != nullptr) ? new OfflineLogViewer(this, *mLogViewer, &mMdiArea) : new OfflineLogViewer(this, &mMdiArea);
    QMdiSubWindow* mdiSub = mMdiArea.addSubWindow(child);
    child->setMdiSubwindow(mdiSub);
    mdiSub->setWindowIcon(NELusanCommon::iconOfflineLogWindow(NELusanCommon::SizeSmall));
    mdiSub->setWindowFilePath(filePath);
    if (filePath.isEmpty() == false)
    {
        std::string stdFilePath{filePath.toStdString()};
        mdiSub->setWindowTitle(QString::fromStdString(File::getFileNameWithExtension(stdFilePath.c_str()).getData()));
    }

    mMdiArea.showMaximized();
    mNaviDock.showTab(NavigationDock::NaviOfflineLogs);
    OfflineLogsModel* logModel = static_cast<OfflineLogsModel *>(child->getLoggingModel());
    static_cast<NaviOfflineLogsScopes *>(mNaviDock.getTab(NavigationDock::TabOfflineLogsExplorer))->setLoggingModel(logModel);
    if (filePath.isEmpty() == false)
    {
        child->openDatabase(filePath);
    }

    return child;
}

inline void MdiMainWindow::initAction(QAction& act, const QIcon& icon, QString txt)
{
    act.setParent(this);
    act.setText(txt);
    act.setIcon(icon);
}

void MdiMainWindow::_createActions()
{
    initAction(mActNewWorkspace, NELusanCommon::iconNewWorkspace(NELusanCommon::SizeBig), tr("New &Workspace"));
    mActNewWorkspace.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_W));
    mActNewWorkspace.setStatusTip(tr("Create a new workspace, restarts application"));
    connect(&mActNewWorkspace, &QAction::triggered, this, [this](){LusanApplication::newWorkspace();});
    
    initAction(mActFileNewSI, NELusanCommon::iconServiceInterface(NELusanCommon::SizeBig), tr("New Service &Interface"));
    mActFileNewSI.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_I));
    mActFileNewSI.setStatusTip(tr("Create a new service interface file"));
    connect(&mActFileNewSI, &QAction::triggered, this, &MdiMainWindow::onFileNewSI);

    initAction(mActFileNewLog, NELusanCommon::iconNewLiveLogs(NELusanCommon::SizeBig), tr("&Live Logs"));
    mActFileNewLog.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_L));
    mActFileNewLog.setStatusTip(tr("Create a new live logs"));
    connect(&mActFileNewLog, &QAction::triggered, this, [this]() {mNaviDock.showTab(NavigationDock::NaviLiveLogs); signalNewLiveLog();});
    
    initAction(mActFileOfflineLog, NELusanCommon::iconNewOfflineLogs(NELusanCommon::SizeBig), tr("O&ffline Logs"));
    mActFileOfflineLog.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_F));
    mActFileOfflineLog.setStatusTip(tr("Open offline logs"));
    connect(&mActFileOfflineLog, &QAction::triggered, this, [this]() {mNaviDock.showTab(NavigationDock::NaviOfflineLogs); signalOpenOfflineLog();});
    
    initAction(mActFileOpen, NELusanCommon::iconOpenDocument(NELusanCommon::SizeBig), tr("&Open..."));
    mActFileOpen.setShortcuts(QKeySequence::Open);
    mActFileOpen.setStatusTip(tr("Open an existing file"));
    connect(&mActFileOpen, &QAction::triggered, this, &MdiMainWindow::onFileOpen);

    initAction(mActFileSave, NELusanCommon::iconSaveDocument(NELusanCommon::SizeBig), tr("&Save"));
    mActFileSave.setShortcuts(QKeySequence::Save);
    mActFileSave.setStatusTip(tr("Save the document to disk"));
    connect(&mActFileSave, &QAction::triggered, this, &MdiMainWindow::onFileSave);

    initAction(mActFileSaveAs, NELusanCommon::iconSaveAsDocument(NELusanCommon::SizeBig), tr("Save &As..."));
    mActFileSaveAs.setShortcuts(QKeySequence::SaveAs);
    mActFileSaveAs.setStatusTip(tr("Save the document under a new name"));
    connect(&mActFileSaveAs, &QAction::triggered, this, &MdiMainWindow::onFileSaveAs);

    initAction(mActFileClose, QIcon(), tr("Cl&ose"));
    mActFileClose.setStatusTip(tr("Close the active window"));
    connect(&mActFileClose, &QAction::triggered, &mMdiArea, &QMdiArea::closeActiveSubWindow);

    initAction(mActFileCloseAll, QIcon(), tr("Close &All"));
    mActFileCloseAll.setStatusTip(tr("Close all the windows"));
    connect(&mActFileCloseAll, &QAction::triggered, &mMdiArea, &QMdiArea::closeAllSubWindows);

    initAction(mActFileExit, NELusanCommon::iconApplicationExit(NELusanCommon::SizeBig), tr("E&xit"));
    mActFileExit.setParent(qApp);
    mActFileExit.setShortcuts(QKeySequence::Quit);
    mActFileExit.setStatusTip(tr("Exit the application"));
    connect(&mActFileExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    
    initAction(mActEditCut, NELusanCommon::iconCut(NELusanCommon::SizeBig), tr("Cu&t"));
    mActEditCut.setShortcuts(QKeySequence::Cut);
    mActEditCut.setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(&mActEditCut, &QAction::triggered, this, &MdiMainWindow::onEditCut);

    initAction(mActEditCopy, NELusanCommon::iconCopy(NELusanCommon::SizeBig), tr("&Copy"));
    mActEditCopy.setShortcuts(QKeySequence::Copy);
    mActEditCopy.setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(&mActEditCopy, &QAction::triggered, this, &MdiMainWindow::onEditCopy);

    initAction(mActEditPaste, NELusanCommon::iconPaste(NELusanCommon::SizeBig), tr("&Paste"));
    mActEditPaste.setShortcuts(QKeySequence::Paste);
    mActEditPaste.setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(&mActEditPaste, &QAction::triggered, this, &MdiMainWindow::onEditPaste);
    
    initAction(mActViewNavigator, NELusanCommon::iconViewNavigationWindow(NELusanCommon::SizeBig), tr("&Navigation Window"));
    mActViewNavigator.setStatusTip(tr("View Navigation Window"));
    connect(&mActViewNavigator, &QAction::triggered, this, [this](){
        if (mNaviDock.isHidden()) mNaviDock.show();
    });
    
    initAction(mActViewWokspace, NELusanCommon::iconViewWorkspace(NELusanCommon::SizeBig), tr("&workspace-explorer"));
    mActViewWokspace.setStatusTip(tr("View Workspace Navigator Window"));
    connect(&mActViewWokspace, &QAction::triggered, this, [this]() {
        if (mNaviDock.isHidden()) mNaviDock.show();
        mNaviDock.showTab(NavigationDock::TabNameFileSystem);
    });
    
    initAction(mActViewLogs, NELusanCommon::iconViewLiveLogs(NELusanCommon::SizeBig), tr("Live &Logs Navigator"));
    mActViewLogs.setStatusTip(tr("View Live Logs Navigator Window"));
    connect(&mActViewLogs, &QAction::triggered, this, [this] () {
        if (mNaviDock.isHidden()) mNaviDock.show();
        mNaviDock.showTab(NavigationDock::TabLiveLogsExplorer);
        if (mLiveLogWnd != nullptr) mLiveLogWnd->activateWindow();
    });
    
    initAction(mActOffViewLogs, NELusanCommon::iconViewOfflineLogs(NELusanCommon::SizeBig), tr("Offline &Logs Navigator"));
    mActOffViewLogs.setStatusTip(tr("View Offline Logs Navigator Window"));
    connect(&mActOffViewLogs, &QAction::triggered, this, [this] () {
        if (mNaviDock.isHidden()) mNaviDock.show();
        mNaviDock.showTab(NavigationDock::TabOfflineLogsExplorer);
    });
    
    initAction(mActViewOutput, NELusanCommon::iconViewOutputWindow(NELusanCommon::SizeBig), tr("&Output Window"));
    mActViewOutput.setStatusTip(tr("View Output Window"));
    connect(&mActViewOutput, &QAction::triggered, this, [this](){
        if (mOutputDock.isHidden()) mOutputDock.show();
    });

    initAction(mActToolsOptions, NELusanCommon::iconSettings(NELusanCommon::SizeBig), tr("&Options"));
    mActToolsOptions.setStatusTip(tr("View Workspace Options"));
    connect(&mActToolsOptions, &QAction::triggered, this, &MdiMainWindow::onToolsOptions);
    
    initAction(mActWindowsTile, QIcon(), tr("&Tile"));
    mActWindowsTile.setStatusTip(tr("Tile the windows"));
    connect(&mActWindowsTile, &QAction::triggered, &mMdiArea, &QMdiArea::tileSubWindows);

    initAction(mActWindowsCascade, QIcon(), tr("&Cascade"));
    mActWindowsCascade.setStatusTip(tr("Cascade the windows"));
    connect(&mActWindowsCascade, &QAction::triggered, &mMdiArea, &QMdiArea::cascadeSubWindows);

    initAction(mActWindowsNext, QIcon(), tr("Ne&xt"));
    mActWindowsNext.setShortcuts(QKeySequence::NextChild);
    mActWindowsNext.setStatusTip(tr("Move the focus to the next window"));
    connect(&mActWindowsNext, &QAction::triggered, &mMdiArea, &QMdiArea::activateNextSubWindow);

    initAction(mActWindowsPrev, QIcon(), tr("Pre&vious"));
    mActWindowsPrev.setShortcuts(QKeySequence::PreviousChild);
    mActWindowsPrev.setStatusTip(tr("Move the focus to the previous window"));
    connect(&mActWindowsPrev, &QAction::triggered, &mMdiArea, &QMdiArea::activatePreviousSubWindow);
    
    mActWindowMenuSeparator.setSeparator(true);
}

void MdiMainWindow::_createMenus()
{
    mFileMenu = menuBar()->addMenu(tr("&File"));
    mFileMenu->addAction(&mActNewWorkspace);
    mFileMenu->addAction(&mActFileNewSI);
    mFileMenu->addAction(&mActFileNewLog);
    mFileMenu->addAction(&mActFileOfflineLog);
    mFileMenu->addAction(&mActFileOpen);
    mFileMenu->addAction(&mActFileSave);
    mFileMenu->addAction(&mActFileSaveAs);
    mFileSeparator = mFileMenu->addSeparator();

    QMenu* recentMenu = mFileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MdiMainWindow::onShowMenuRecent);
    mActFileRecent = recentMenu->menuAction();
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        mActsRecentFiles[i] = recentMenu->addAction(QString(), this, &MdiMainWindow::onFileOpenRecent);
        mActsRecentFiles[i]->setVisible(false);
    }

    setRecentFilesVisibility(MdiMainWindow::hasRecentFiles());
    mFileMenu->addSeparator();
    mFileMenu->addAction(&mActFileExit);

    mEditMenu = menuBar()->addMenu(tr("&Edit"));
    mEditMenu->addAction(&mActEditCut);
    mEditMenu->addAction(&mActEditCopy);
    mEditMenu->addAction(&mActEditPaste);

    mViewMenu = menuBar()->addMenu(tr("&View"));
    mViewMenu->addAction(&mActViewNavigator);
    mViewMenu->addAction(&mActViewWokspace);
    mViewMenu->addAction(&mActViewLogs);
    mViewMenu->addAction(&mActViewOutput);

    mToolsMenu = menuBar()->addMenu(tr("&Tools"));
    mToolsMenu->addAction(&mActToolsOptions);

    mWindowMenu = menuBar()->addMenu(tr("&Window"));
    connect(mWindowMenu, &QMenu::aboutToShow, this, &MdiMainWindow::onShowMenuWindow);

    menuBar()->addSeparator();
    mHelpMenu = menuBar()->addMenu(tr("&Help"));
    mActHelpAbout = mHelpMenu->addAction(tr("&About"), this, &MdiMainWindow::onHelpAbout);
    mActHelpAbout->setStatusTip(tr("Show the application's About box"));
}

void MdiMainWindow::_createToolBars()
{
    mFileToolBar = addToolBar(tr("File"));
    mFileToolBar->addAction(&mActFileNewSI);
    mFileToolBar->addAction(&mActFileNewLog);
    mFileToolBar->addAction(&mActFileOfflineLog);
    mFileToolBar->addAction(&mActFileOpen);
    mFileToolBar->addAction(&mActFileSave);
    mFileToolBar->addSeparator();

    mEditToolBar = addToolBar(tr("Edit"));
    mEditToolBar->addAction(&mActEditCut);
    mEditToolBar->addAction(&mActEditCopy);
    mEditToolBar->addAction(&mActEditPaste);
    mEditToolBar->addSeparator();
    
    mViewToolBar = addToolBar(tr("View"));
    mViewToolBar->addAction(&mActViewNavigator);
    mViewToolBar->addAction(&mActViewOutput);
}

void MdiMainWindow::_createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MdiMainWindow::_createDockWindows()
{
    mNaviDock.setAllowedAreas(Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea    , &mNaviDock,  Qt::Orientation::Vertical);
    
    mOutputDock.setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea  , &mOutputDock);
}

void MdiMainWindow::_createMdiArea()
{
    setCentralWidget(&mMdiArea);
    connect(&mMdiArea, &QMdiArea::subWindowActivated, this, &MdiMainWindow::onSubWindowActivated);
}

void MdiMainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    }
    else 
    {
        restoreGeometry(geometry);
    }
}

void MdiMainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

MdiChild* MdiMainWindow::activeMdiChild() const
{
    QMdiSubWindow* activeSubWindow = mMdiArea.activeSubWindow();
    return (activeSubWindow != nullptr ? qobject_cast<MdiChild*>(activeSubWindow->widget()) : nullptr);
}

QMdiSubWindow* MdiMainWindow::findMdiChild(const QString& fileName) const
{
    QMdiSubWindow* result{ nullptr };
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    const QList<QMdiSubWindow*> subWindows = mMdiArea.subWindowList();
    for (QMdiSubWindow* window : subWindows)
    {
        MdiChild* mdiChild = qobject_cast<MdiChild*>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
        {
            result = window;
            break;
        }
    }

    return result;
}
