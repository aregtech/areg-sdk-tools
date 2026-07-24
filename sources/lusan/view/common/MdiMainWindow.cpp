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
 *  \file        lusan/view/common/MdiMainWindow.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/
#include "lusan/view/common/MdiMainWindow.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/OptionsManager.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"
#include "lusan/model/log/OfflineLogsModel.hpp"
#include "lusan/view/si/ServiceInterface.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/StateMachine.hpp"
#include "lusan/view/common/NaviDesignPanel.hpp"
#include "lusan/view/common/NaviFsmToolbar.hpp"
#include "lusan/view/common/ProjectSettings.hpp"
#include "lusan/view/common/OptionPageLogging.hpp"
#include "lusan/view/log/LiveLogViewer.hpp"
#include "lusan/view/log/OfflineLogViewer.hpp"

#include "areg/base/SocketDefs.hpp"

#include <DockAreaWidget.h>
#include <DockManager.h>
#include <DockWidget.h>

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QPushButton>

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

inline QString MdiMainWindow::_filterServiceFiles()
{
    return QString{"Service Interface Document (*.siml);;All Files (*.*)"};
}

inline QString MdiMainWindow::_filterStateMachineFiles()
{
    return QString{"State Machine Document (*.fsml);;All Files (*.*)"};
}

inline QString MdiMainWindow::_filterLoggingFiles()
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
    , mDockManager  ( nullptr )
    , mCentralDock  ( nullptr )
    , mCentralArea  ( nullptr )
    , mNaviDockWidget ( nullptr )
    , mOutputDockWidget ( nullptr )
    , mNaviToolbar  ( nullptr )
    , mNaviProperties ( nullptr )
    , mNaviOutline  ( nullptr )
    , mPlaceToolbar ( eDesignPlace::InDesign )
    , mPlaceProperties ( eDesignPlace::InDesign )
    , mPlaceOutline ( eDesignPlace::InDesign )
    , mLogViewer    ( nullptr )
    , mLiveLogWnd   ( nullptr )

    , mFileMenu     (nullptr)
    , mEditMenu     (nullptr)
    , mViewMenu     (nullptr)
    , mNavigationMenu(nullptr)
    , mViewDesignMenu(nullptr)
    , mThemeMenu    (nullptr)
    , mDesignMenu   (nullptr)
    , mLoggingMenu  (nullptr)
    , mToolsMenu    (nullptr)
    , mWindowMenu   (nullptr)
    , mHelpMenu     (nullptr)
    , mThemeActions (nullptr)

    , mFileToolBar  (nullptr)
    , mEditToolBar  (nullptr)
    , mViewToolBar  (nullptr)
    , mActNewWorkspace(this)
    , mActFileNewSI (this)
    , mActFileNewFSM(this)
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
    , mActEditUndo  (this)
    , mActEditRedo  (this)
    , mActViewNavigator(this)
    , mActViewWokspace(this)
    , mActViewLogs  (this)
    , mActOffViewLogs(this)
    , mActViewOutput(this)
    , mActNavWindow (nullptr)
    , mActNavWorkspace(nullptr)
    , mActNavLiveLogs(nullptr)
    , mActNavOfflineLogs(nullptr)
    , mActNavToolbar(nullptr)
    , mActNavProperties(nullptr)
    , mActNavOutline(nullptr)
    , mActDsgToolbar(nullptr)
    , mActDsgProperties(nullptr)
    , mActDsgOutline(nullptr)
    , mActWindowsTile(this)
    , mActWindowsCascade(this)
    , mActWindowsNext(this)
    , mActWindowsPrev(this)
    , mActWindowMenuSeparator(this)
    , mActHelpAbout (nullptr)
    , mSIWarmupDone (false)
{
    _createActions();
    _createMenus();
    _createToolBars();
    _createStatusBar();
    _createMdiArea();       // creates the ADS dock manager + central widget first
    _createDockWindows();   // then adds the navigation/output docks to it
    createDesignNavHosts(); // the Navigation Window hosts for the movable design widgets
    loadDesignPlacements(); // restore where the toolbar / Properties / Outline last lived

    onShowMenuWindow();
    onSubWindowActivated(nullptr);  // also runs the first syncDesignWidgets()
    readSettings();

    setWindowTitle(tr("Lusan"));
    setUnifiedTitleAndToolBarOnMac(true);
    QTimer::singleShot(0, this, &MdiMainWindow::onWarmupServiceInterface);
}

const QString& MdiMainWindow::fileFilters() const
{
    static const QString _filter {
        "Areg SDK Files (*.siml *.fsml *.sqlog)\n"
        "Service Interface Files (*.siml)\n"
        "State Machine Files (*.fsml)\n"
        "Log Files (*.sqlog)\n"
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

LiveLogsModel * MdiMainWindow::setupLiveLogging()
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

LiveLogsModel* MdiMainWindow::getLiveLogging() const
{
    return (mLogViewer != nullptr ? static_cast<LiveLogsModel *>(mLogViewer->getLoggingModel()) : nullptr);
}

NaviFileSystem& MdiMainWindow::getNaviFileSystem()
{
    return mNaviDock.getFileSystem();
}

NaviLiveLogsScopes& MdiMainWindow::getNaviLiveScopes()
{
    return mNaviDock.getLiveScopes();
}

NaviOfflineLogsScopes& MdiMainWindow::getNaviOfflineScopes()
{
    return mNaviDock.getOfflineScopes();
}

ScopeOutputViewer& MdiMainWindow::getOutputScopeLogs()
{
    return mOutputDock.getScopeLogsView();
}

void MdiMainWindow::setTabBarTooltip(QMdiSubWindow* subWindow, const QString& tooltip)
{
    if (subWindow == nullptr)
        return;

    QTabBar* tabBar = mMdiArea.findChild<QTabBar*>();
    if (tabBar == nullptr)
        return;

    QList<QMdiSubWindow*> windows = mMdiArea.subWindowList();
    for (int i = 0; i < static_cast<int>(windows.size()); ++i)
    {
        if (windows[i] == subWindow)
        {
            tabBar->setTabToolTip(i, tooltip);
            break;
        }
    }
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
    if ((address.isEmpty() == false) && (port != areg::InvalidPort) && (logFile.isEmpty() == false) && (logLocation.isEmpty() == false))
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
    showDock(mNaviDockWidget);
    mNaviDock.showTab(naviTab);
}

QString MdiMainWindow::openLogFile()
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

void MdiMainWindow::onFileNewFSM()
{
    StateMachine* child = createStateMachineView();
    if (child != nullptr)
    {
        child->newFile();
        child->show();
    }
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

void MdiMainWindow::onFileExit()
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

void MdiMainWindow::onEditUndo()
{
    MdiChild* active = activeMdiChild();
    if (active != nullptr)
    {
        active->undo();
    }
}

void MdiMainWindow::onEditRedo()
{
    MdiChild* active = activeMdiChild();
    if (active != nullptr)
    {
        active->redo();
    }
}

void MdiMainWindow::onShowMenuNavigation()
{
    // Refresh the checkable View submenus from the live state each time they open, so the marks
    // stay right after the user closes the dock via its title bar or moves a widget elsewhere.
    const auto set = [](QAction* act, bool checked) {
        if (act != nullptr)
        {
            QSignalBlocker block(act);
            act->setChecked(checked);
        }
    };

    set(mActNavWindow, (mNaviDockWidget != nullptr) && (mNaviDockWidget->isClosed() == false));
    set(mActNavWorkspace, mNaviDock.isNaviTabVisible(NavigationDock::NaviWorkspace));
    set(mActNavLiveLogs, mNaviDock.isNaviTabVisible(NavigationDock::NaviLiveLogs));
    set(mActNavOfflineLogs, mNaviDock.isNaviTabVisible(NavigationDock::NaviOfflineLogs));
    updatePlacementActions();
}

MdiMainWindow::eDesignPlace& MdiMainWindow::placementRef(MdiMainWindow::eDesignWidget widget)
{
    switch (widget)
    {
    case eDesignWidget::Toolbar:    return mPlaceToolbar;
    case eDesignWidget::Properties: return mPlaceProperties;
    default:                        return mPlaceOutline;
    }
}

MdiMainWindow::eDesignPlace MdiMainWindow::designWidgetPlacement(MdiMainWindow::eDesignWidget widget) const
{
    return const_cast<MdiMainWindow*>(this)->placementRef(widget);
}

void MdiMainWindow::setDesignWidgetPlacement(MdiMainWindow::eDesignWidget widget, MdiMainWindow::eDesignPlace place)
{
    placementRef(widget) = place;

    // Persist so the chosen home survives restarts (issue #516).
    static const char* const keys[] = { "smDesign/placeToolbar", "smDesign/placeProperties", "smDesign/placeOutline" };
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(QLatin1String(keys[static_cast<int>(widget)]), static_cast<int>(place));

    if (place == eDesignPlace::InNavigation)
    {
        showDock(mNaviDockWidget);      // make sure the just-moved widget is actually visible
    }

    syncDesignWidgets();
    if (place == eDesignPlace::InNavigation)
    {
        switch (widget)
        {
        case eDesignWidget::Toolbar:
            mNaviDock.showTab(NavigationDock::NaviDesignToolbar);
            break;

        case eDesignWidget::Properties:
            mNaviDock.showTab(NavigationDock::NaviDesignProperties);
            break;

        case eDesignWidget::Outline:
            mNaviDock.showTab(NavigationDock::NaviDesignOutline);
            break;
        }
    }
}

void MdiMainWindow::createDesignNavHosts()
{
    // The Navigation Window hosts for the movable design widgets are owned by the main window,
    // reparented into a navigation tab only while their widget is placed there (issue #516).
    mNaviToolbar = new NaviFsmToolbar(this, this);
    mNaviToolbar->hide();
    mNaviProperties = new NaviDesignPanel(NaviDesignPanel::eKind::Properties, this, this);
    mNaviProperties->hide();
    mNaviOutline = new NaviDesignPanel(NaviDesignPanel::eKind::Outline, this, this);
    mNaviOutline->hide();
}

void MdiMainWindow::loadDesignPlacements()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const auto load = [&settings](const QString& key) {
        // Default: all three inside the Design page, matching the Phase 1 layout (issue #516).
        return static_cast<eDesignPlace>(settings.value(key, static_cast<int>(eDesignPlace::InDesign)).toInt());
    };

    mPlaceToolbar    = load(QStringLiteral("smDesign/placeToolbar"));
    mPlaceProperties = load(QStringLiteral("smDesign/placeProperties"));
    mPlaceOutline    = load(QStringLiteral("smDesign/placeOutline"));
    updatePlacementActions();
}

void MdiMainWindow::updatePlacementActions()
{
    const auto set = [](QAction* act, bool checked) {
        if (act != nullptr)
        {
            QSignalBlocker block(act);
            act->setChecked(checked);
        }
    };

    set(mActDsgToolbar,    mPlaceToolbar    == eDesignPlace::InDesign);
    set(mActNavToolbar,    mPlaceToolbar    == eDesignPlace::InNavigation);
    set(mActDsgProperties, mPlaceProperties == eDesignPlace::InDesign);
    set(mActNavProperties, mPlaceProperties == eDesignPlace::InNavigation);
    set(mActDsgOutline,    mPlaceOutline    == eDesignPlace::InDesign);
    set(mActNavOutline,    mPlaceOutline    == eDesignPlace::InNavigation);
}

void MdiMainWindow::syncDesignWidgets()
{
    if (mNaviToolbar == nullptr)
    {
        return;     // the navigation hosts are not created yet (very early startup)
    }

    StateMachine* stateMachine = qobject_cast<StateMachine*>(activeMdiChild());
    SMDesign* design = (stateMachine != nullptr) ? stateMachine->designPageIfBuilt() : nullptr;
    const bool designCurrent = (stateMachine != nullptr) && (design != nullptr) && stateMachine->isDesignPageCurrent();

    // Let the active Design page's context menu render the correct check marks.
    if (design != nullptr)
    {
        design->setPlacementState(static_cast<int>(mPlaceToolbar), static_cast<int>(mPlaceProperties), static_cast<int>(mPlaceOutline));
    }

    // Toolbar: the drawing toolbar always shows its buttons in the Navigation Window (disabled
    // off the Design page), matching the Phase 1 stand-in behavior.
    switch (mPlaceToolbar)
    {
    case eDesignPlace::InDesign:
        mNaviDock.hideDesignTab(NavigationDock::NaviDesignToolbar);
        if (design != nullptr) { design->setToolbarVisible(true); }
        break;

    case eDesignPlace::InNavigation:
        if (design != nullptr) { design->setToolbarVisible(false); }
        {
            // The Navigation Window form of the toolbar defaults to Icon and Text (its custom
            // vector icons are hard to tell apart at nav width), independent of the in-page
            // toolbar's Icon Only default. Once the user picks any Toolbutton Mode the choice is
            // stored and honored here too (issue #516 bug 6).
            QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
            const Qt::ToolButtonStyle style = settings.contains(QStringLiteral("smDesign/naviToolbarStyle"))
                    ? static_cast<Qt::ToolButtonStyle>(settings.value(QStringLiteral("smDesign/naviToolbarStyle")).toInt())
                    : Qt::ToolButtonTextBesideIcon;
            mNaviToolbar->setDisplayStyle(style);
        }
        mNaviToolbar->bindDesign(design);
        mNaviToolbar->setToolsActive(designCurrent);
        mNaviDock.showDesignTab(NavigationDock::NaviDesignToolbar, mNaviToolbar);
        break;

    case eDesignPlace::Hidden:
        mNaviDock.hideDesignTab(NavigationDock::NaviDesignToolbar);
        if (design != nullptr) { design->setToolbarVisible(false); }
        break;
    }

    // Properties: empty in the Navigation Window unless the Design page is the current tab.
    switch (mPlaceProperties)
    {
    case eDesignPlace::InDesign:
        mNaviDock.hideDesignTab(NavigationDock::NaviDesignProperties);
        if (design != nullptr) { design->setPropertiesVisible(true); }
        break;

    case eDesignPlace::InNavigation:
        if (design != nullptr) { design->setPropertiesVisible(false); }
        mNaviProperties->bindDesign(designCurrent ? design : nullptr);
        mNaviDock.showDesignTab(NavigationDock::NaviDesignProperties, mNaviProperties);
        break;

    case eDesignPlace::Hidden:
        mNaviDock.hideDesignTab(NavigationDock::NaviDesignProperties);
        if (design != nullptr) { design->setPropertiesVisible(false); }
        break;
    }

    // Outline: same rule as Properties.
    switch (mPlaceOutline)
    {
    case eDesignPlace::InDesign:
        mNaviDock.hideDesignTab(NavigationDock::NaviDesignOutline);
        if (design != nullptr) { design->setOutlineVisible(true); }
        break;

    case eDesignPlace::InNavigation:
        if (design != nullptr) { design->setOutlineVisible(false); }
        mNaviOutline->bindDesign(designCurrent ? design : nullptr);
        mNaviDock.showDesignTab(NavigationDock::NaviDesignOutline, mNaviOutline);
        break;

    case eDesignPlace::Hidden:
        mNaviDock.hideDesignTab(NavigationDock::NaviDesignOutline);
        if (design != nullptr) { design->setOutlineVisible(false); }
        break;
    }

    updatePlacementActions();
}

void MdiMainWindow::onFsmToolbarStyle(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    // Persist the choice and apply it to the active Design page's in-page toolbar and the
    // Navigation Window toolbar host; every Design page seeds its toolbar from this stored
    // style when built (issue #516).
    const Qt::ToolButtonStyle style = static_cast<Qt::ToolButtonStyle>(action->data().toInt());
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(QStringLiteral("smDesign/toolbarStyle"), static_cast<int>(style));
    // An explicit menu choice governs the Navigation Window toolbar too, replacing its Icon
    // and Text default (issue #516 bug 6).
    settings.setValue(QStringLiteral("smDesign/naviToolbarStyle"), static_cast<int>(style));

    StateMachine* stateMachine = qobject_cast<StateMachine*>(activeMdiChild());
    SMDesign* design = (stateMachine != nullptr) ? stateMachine->designPageIfBuilt() : nullptr;
    if (design != nullptr)
    {
        design->setToolbarStyle(style);
    }

    if (mNaviToolbar != nullptr)
    {
        mNaviToolbar->setDisplayStyle(style);
    }
}

void MdiMainWindow::onShowMenuDesign()
{
    mDesignMenu->clear();

    StateMachine* stateMachine = qobject_cast<StateMachine*>(activeMdiChild());
    SMDesign* design = (stateMachine != nullptr) ? stateMachine->designPageIfBuilt() : nullptr;
    if (design == nullptr)
    {
        // No Design page yet: present the full command set as grouped, disabled stand-ins so the
        // user sees every available command (and that it is inactive) instead of a lone note
        // (issue #516). Undo/Redo belong to the Edit menu and are not repeated here. The stand-in
        // actions are parented to the menu, so the next clear() disposes of them.
        const QList<SMDesign::ToolGroup> groups = SMDesign::placeholderToolGroups(*mDesignMenu);
        const QString undoText = SMDesign::tr("Undo");
        const QString redoText = SMDesign::tr("Redo");
        for (const SMDesign::ToolGroup& group : groups)
        {
            mDesignMenu->addSection(group.title);
            for (QAction* action : group.actions)
            {
                if ((action->text() == undoText) || (action->text() == redoText))
                {
                    continue;
                }

                mDesignMenu->addAction(action);
            }
        }

        return;
    }

    mDesignMenu->addAction(design->actionAddState());
    mDesignMenu->addAction(design->actionAddFinalState());
    mDesignMenu->addAction(design->actionAddTransition());
    mDesignMenu->addAction(design->actionAddNote());
    mDesignMenu->addSeparator();
    mDesignMenu->addAction(design->actionStateColor());
    mDesignMenu->addAction(design->actionEdgeColor());
    mDesignMenu->addAction(design->actionNoteColor());
    mDesignMenu->addSeparator();
    mDesignMenu->addAction(design->actionAlignLeft());
    mDesignMenu->addAction(design->actionAlignRight());
    mDesignMenu->addAction(design->actionAlignTop());
    mDesignMenu->addAction(design->actionAlignBottom());
    mDesignMenu->addAction(design->actionDistributeHorizontal());
    mDesignMenu->addAction(design->actionDistributeVertical());
    mDesignMenu->addSeparator();
    mDesignMenu->addAction(design->actionToggleGrid());
    mDesignMenu->addAction(design->actionGridDots());
    mDesignMenu->addAction(design->actionToggleSnap());
    mDesignMenu->addAction(design->actionGridSize());
    mDesignMenu->addSeparator();
    QMenu* declareMenu = mDesignMenu->addMenu(tr("Add &Declaration"));
    declareMenu->addActions(design->declareActions());
    mDesignMenu->addSeparator();
    mDesignMenu->addAction(design->actionAddSubstate());
    mDesignMenu->addAction(design->actionEnterSubmachine());
    mDesignMenu->addAction(design->actionGoToParent());
    mDesignMenu->addSeparator();
    mDesignMenu->addAction(design->actionZoomIn());
    mDesignMenu->addAction(design->actionZoomOut());
    mDesignMenu->addAction(design->actionZoomReset());
    mDesignMenu->addAction(design->actionZoomFit());
    mDesignMenu->addSeparator();
    mDesignMenu->addAction(design->actionSelectAll());
    mDesignMenu->addAction(design->actionRename());
    mDesignMenu->addAction(design->actionDelete());
}

void MdiMainWindow::onToolsOptions()
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

    // Undo/Redo enablement follows the active child's own stack state; re-wire the live
    // connection on every activation (inert for SI/log windows, which keep the base default).
    disconnect(mCanUndoConn);
    disconnect(mCanRedoConn);
    mActEditUndo.setEnabled(hasMdiChild && mdiActive->canUndo());
    mActEditRedo.setEnabled(hasMdiChild && mdiActive->canRedo());
    if (hasMdiChild)
    {
        mCanUndoConn = connect(mdiActive, &MdiChild::signalCanUndoChanged, &mActEditUndo, &QAction::setEnabled);
        mCanRedoConn = connect(mdiActive, &MdiChild::signalCanRedoChanged, &mActEditRedo, &QAction::setEnabled);
    }

    if (mdiActive != nullptr)
    {
        mdiActive->onWindowActivated();
    }

    // Re-apply the design-widget placement to the newly active document: show/hide each widget's
    // Design-page dock and bind or empty its Navigation Window host, so the toolbar, Properties,
    // and Outline are populated only while an FSM Design page is current (issue #516).
    syncDesignWidgets();
}

void MdiMainWindow::onWarmupServiceInterface()
{
    if (mSIWarmupDone)
        return;

    mSIWarmupDone = true;
    DataTypeFactory::warmup();
}

void MdiMainWindow::onShowMenuTheme()
{
    if (mThemeActions == nullptr)
        return;

    const int current = static_cast<int>(LusanApplication::getOptions().getTheme());
    const QList<QAction*> actions = mThemeActions->actions();
    for (QAction* action : actions)
    {
        action->setChecked(action->data().toInt() == current);
    }
}

void MdiMainWindow::onThemeSelected(QAction* action)
{
    if (action == nullptr)
        return;

    OptionsManager& options = LusanApplication::getOptions();
    const OptionsManager::eAppTheme theme = static_cast<OptionsManager::eAppTheme>(action->data().toInt());
    if (theme == options.getTheme())
        return;

    options.setTheme(theme);
    options.writeOptions();
    LusanApplication::applyConfiguredTheme();
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
    else if (ext == StateMachine::fileExtension())
    {
        result = createStateMachineView(filePath);
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
    if ((filePath.isEmpty() == false) && (child->openSucceeded() == false))
    {
        delete child;
        QMessageBox::warning( this
                            , tr("Invalid Service Interface")
                            , tr("Failed to read the service interface file:\n%1\nThe file is not accessible or has invalid format.").arg(filePath));
        return nullptr;
    }

    QMdiSubWindow* mdiSub = mMdiArea.addSubWindow(child);
    child->setMdiSubwindow(mdiSub);
    mdiSub->setWindowIcon(NELusanCommon::iconServiceInterface(NELusanCommon::SizeSmall));
    child->setCurrentFile(filePath);
    mMdiArea.setActiveSubWindow(mdiSub);
    mdiSub->showMaximized();
    return child;
}

StateMachine* MdiMainWindow::createStateMachineView(const QString& filePath /*= QString()*/)
{
    QString sourcePath;
    if (filePath.isEmpty() == false)
    {
        QString autosavePath;
        if (StateMachineData::hasRecoverableAutosave(filePath, &autosavePath))
        {
            QMessageBox recovery(this);
            recovery.setWindowTitle(tr("State Machine Recovery"));
            recovery.setIcon(QMessageBox::Question);
            recovery.setText(tr("Unsaved changes were found for:\n%1").arg(filePath));
            recovery.setInformativeText(tr("Do you want to restore data from:\n%1").arg(autosavePath));
            QPushButton* restore = recovery.addButton(tr("Restore"), QMessageBox::AcceptRole);
            QPushButton* discard = recovery.addButton(tr("Discard"), QMessageBox::DestructiveRole);
            QPushButton* cancel  = recovery.addButton(QMessageBox::Cancel);
            recovery.exec();

            if (recovery.clickedButton() == cancel)
            {
                return nullptr;
            }
            else if (recovery.clickedButton() == restore)
            {
                sourcePath = autosavePath;
            }
            else if (recovery.clickedButton() == discard)
            {
                StateMachineData::removeAutosave(filePath);
            }
        }
    }

    StateMachine* child = new StateMachine(this, filePath, sourcePath, &mMdiArea);
    if ((filePath.isEmpty() == false) && (child->openSucceeded() == false))
    {
        delete child;
        QMessageBox::warning(this,
                             tr("Invalid State Machine"),
                             tr("Failed to read the state machine file:\n%1\nThe file is not accessible or has invalid format.")
                                 .arg(sourcePath.isEmpty() ? filePath : sourcePath));
        return nullptr;
    }

    QMdiSubWindow* mdiSub = mMdiArea.addSubWindow(child);
    child->setMdiSubwindow(mdiSub);
    mdiSub->setWindowIcon(NELusanCommon::iconStateMachine(NELusanCommon::SizeSmall));
    child->setCurrentFile(filePath);
    mMdiArea.setActiveSubWindow(mdiSub);
    mdiSub->showMaximized();
    return child;
}

LiveLogViewer* MdiMainWindow::createLogViewerView(const QString& filePath /*= QString()*/)
{
    LiveLogViewer* child = new LiveLogViewer(this, &mMdiArea);
    QMdiSubWindow* mdiSub = mMdiArea.addSubWindow(child);
    child->setMdiSubwindow(mdiSub);
    mdiSub->setWindowIcon(NELusanCommon::iconLiveLogWindow(NELusanCommon::SizeSmall));
    child->setCurrentFile(filePath);
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

    initAction(mActFileNewFSM, NELusanCommon::iconStateMachine(NELusanCommon::SizeBig), tr("New State &Machine"));
    mActFileNewFSM.setShortcut(QKeyCombination(Qt::Modifier::CTRL | Qt::Modifier::SHIFT, Qt::Key::Key_M));
    mActFileNewFSM.setStatusTip(tr("Create a new state machine file"));
    connect(&mActFileNewFSM, &QAction::triggered, this, &MdiMainWindow::onFileNewFSM);

    initAction(mActFileNewLog, NELusanCommon::iconNewLiveLogs(NELusanCommon::SizeBig), tr("New &Live Logs"));
    mActFileNewLog.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_L));
    mActFileNewLog.setStatusTip(tr("Create a new live logs"));
    connect(&mActFileNewLog, &QAction::triggered, this, [this]() {mNaviDock.showTab(NavigationDock::NaviLiveLogs); signalNewLiveLog();});
    
    initAction(mActFileOpen, NELusanCommon::iconOpenFile(NELusanCommon::SizeBig), tr("&Open..."));
    mActFileOpen.setShortcuts(QKeySequence::Open);
    mActFileOpen.setStatusTip(tr("Open an existing file"));
    connect(&mActFileOpen, &QAction::triggered, this, &MdiMainWindow::onFileOpen);
    
    initAction(mActFileOfflineLog, NELusanCommon::iconNewOfflineLogs(NELusanCommon::SizeBig), tr("Open O&ffline Logs"));
    mActFileOfflineLog.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_F));
    mActFileOfflineLog.setStatusTip(tr("Open offline logs"));
    connect(&mActFileOfflineLog, &QAction::triggered, this, [this]() {mNaviDock.showTab(NavigationDock::NaviOfflineLogs); signalOpenOfflineLog();});
    
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

    initAction(mActEditUndo, NELusanCommon::iconEditUndo(NELusanCommon::SizeBig), tr("&Undo"));
    mActEditUndo.setShortcuts(QKeySequence::Undo);
    mActEditUndo.setStatusTip(tr("Undo the last edit"));
    mActEditUndo.setEnabled(false);
    connect(&mActEditUndo, &QAction::triggered, this, &MdiMainWindow::onEditUndo);
    
    initAction(mActEditRedo, NELusanCommon::iconEditRedo(NELusanCommon::SizeBig), tr("&Redo"));
    mActEditRedo.setShortcuts(QKeySequence::Redo);
    mActEditRedo.setStatusTip(tr("Redo the last undone edit"));
    mActEditRedo.setEnabled(false);
    connect(&mActEditRedo, &QAction::triggered, this, &MdiMainWindow::onEditRedo);

    initAction(mActViewNavigator, NELusanCommon::iconViewNavigationWindow(NELusanCommon::SizeBig), tr("&Navigation Window"));
    mActViewNavigator.setStatusTip(tr("View Navigation Window"));
    connect(&mActViewNavigator, &QAction::triggered, this, [this](){
        showDock(mNaviDockWidget);
    });

    initAction(mActViewWokspace, NELusanCommon::iconViewWorkspace(NELusanCommon::SizeBig), tr("&Workspace Explorer"));
    mActViewWokspace.setStatusTip(tr("View Workspace Navigator Window"));
    connect(&mActViewWokspace, &QAction::triggered, this, [this]() {
        showDock(mNaviDockWidget);
        mNaviDock.showTab(NavigationDock::TabNameFileSystem);
    });

    initAction(mActViewLogs, NELusanCommon::iconViewLiveLogs(NELusanCommon::SizeBig), tr("Live &Logs Navigator"));
    mActViewLogs.setStatusTip(tr("View Live Logs Navigator Window"));
    connect(&mActViewLogs, &QAction::triggered, this, [this] () {
        showDock(mNaviDockWidget);
        mNaviDock.showTab(NavigationDock::TabLiveLogsExplorer);
        if (mLiveLogWnd != nullptr) mLiveLogWnd->activateWindow();
    });

    initAction(mActOffViewLogs, NELusanCommon::iconViewOfflineLogs(NELusanCommon::SizeBig), tr("Offline &Logs Navigator"));
    mActOffViewLogs.setStatusTip(tr("View Offline Logs Navigator Window"));
    connect(&mActOffViewLogs, &QAction::triggered, this, [this] () {
        showDock(mNaviDockWidget);
        mNaviDock.showTab(NavigationDock::TabOfflineLogsExplorer);
    });

    initAction(mActViewOutput, NELusanCommon::iconViewOutputWindow(NELusanCommon::SizeBig), tr("&Output Window"));
    mActViewOutput.setStatusTip(tr("View Output Window"));
    connect(&mActViewOutput, &QAction::triggered, this, [this](){
        showDock(mOutputDockWidget);
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
    mFileMenu->addAction(&mActFileNewFSM);
    mFileMenu->addAction(&mActFileNewLog);
    mFileMenu->addAction(&mActFileOpen);
    mFileMenu->addAction(&mActFileOfflineLog);
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
    mEditMenu->addAction(&mActEditUndo);
    mEditMenu->addAction(&mActEditRedo);
    mEditMenu->addSeparator();
    mEditMenu->addAction(&mActEditCut);
    mEditMenu->addAction(&mActEditCopy);
    mEditMenu->addAction(&mActEditPaste);

    mViewMenu = menuBar()->addMenu(tr("&View"));

    // View > Navigation: the navigation dock, its explorer tabs, and the three State Machine
    // design widgets when they are moved to the Navigation Window (issue #516). Every entry is
    // checkable; onShowMenuNavigation() refreshes the check marks from the live state on open.
    mNavigationMenu = mViewMenu->addMenu(tr("&Navigation"));
    connect(mNavigationMenu, &QMenu::aboutToShow, this, &MdiMainWindow::onShowMenuNavigation);

    mActNavWindow = mNavigationMenu->addAction(tr("Navigation &Window"));
    mActNavWindow->setCheckable(true);
    connect(mActNavWindow, &QAction::toggled, this, [this](bool on) {
        if (mNaviDockWidget != nullptr)
        {
            mNaviDockWidget->toggleView(on);
        }
    });

    mActNavWorkspace = mNavigationMenu->addAction(tr("Wor&kspace Explorer"));
    mActNavWorkspace->setCheckable(true);
    connect(mActNavWorkspace, &QAction::toggled, this, [this](bool on) {
        mNaviDock.setNaviTabVisible(NavigationDock::NaviWorkspace, on);
    });

    mActNavLiveLogs = mNavigationMenu->addAction(tr("&Live Logs"));
    mActNavLiveLogs->setCheckable(true);
    connect(mActNavLiveLogs, &QAction::toggled, this, [this](bool on) {
        mNaviDock.setNaviTabVisible(NavigationDock::NaviLiveLogs, on);
    });

    mActNavOfflineLogs = mNavigationMenu->addAction(tr("&Offline Logs"));
    mActNavOfflineLogs->setCheckable(true);
    connect(mActNavOfflineLogs, &QAction::toggled, this, [this](bool on) {
        mNaviDock.setNaviTabVisible(NavigationDock::NaviOfflineLogs, on);
    });

    mNavigationMenu->addSeparator();

    mActNavToolbar = mNavigationMenu->addAction(tr("&Design Toolbar"));
    mActNavToolbar->setCheckable(true);
    connect(mActNavToolbar, &QAction::toggled, this, [this](bool on) {
        setDesignWidgetPlacement(eDesignWidget::Toolbar, on ? eDesignPlace::InNavigation : eDesignPlace::Hidden);
    });

    mActNavProperties = mNavigationMenu->addAction(tr("State Machine &Properties"));
    mActNavProperties->setCheckable(true);
    connect(mActNavProperties, &QAction::toggled, this, [this](bool on) {
        setDesignWidgetPlacement(eDesignWidget::Properties, on ? eDesignPlace::InNavigation : eDesignPlace::Hidden);
    });

    mActNavOutline = mNavigationMenu->addAction(tr("State Machine &Outline"));
    mActNavOutline->setCheckable(true);
    connect(mActNavOutline, &QAction::toggled, this, [this](bool on) {
        setDesignWidgetPlacement(eDesignWidget::Outline, on ? eDesignPlace::InNavigation : eDesignPlace::Hidden);
    });

    // View > Design: the same three widgets when docked inside the active FSM Design page. Each
    // Design entry is mutually exclusive with its Navigation counterpart (handled in
    // setDesignWidgetPlacement); unchecking a widget hides it from both homes.
    mViewDesignMenu = mViewMenu->addMenu(tr("&Design"));
    connect(mViewDesignMenu, &QMenu::aboutToShow, this, &MdiMainWindow::onShowMenuNavigation);

    mActDsgToolbar = mViewDesignMenu->addAction(tr("&Toolbar"));
    mActDsgToolbar->setCheckable(true);
    connect(mActDsgToolbar, &QAction::toggled, this, [this](bool on) {
        setDesignWidgetPlacement(eDesignWidget::Toolbar, on ? eDesignPlace::InDesign : eDesignPlace::Hidden);
    });

    mActDsgProperties = mViewDesignMenu->addAction(tr("&Properties"));
    mActDsgProperties->setCheckable(true);
    connect(mActDsgProperties, &QAction::toggled, this, [this](bool on) {
        setDesignWidgetPlacement(eDesignWidget::Properties, on ? eDesignPlace::InDesign : eDesignPlace::Hidden);
    });

    mActDsgOutline = mViewDesignMenu->addAction(tr("&Outline"));
    mActDsgOutline->setCheckable(true);
    connect(mActDsgOutline, &QAction::toggled, this, [this](bool on) {
        setDesignWidgetPlacement(eDesignWidget::Outline, on ? eDesignPlace::InDesign : eDesignPlace::Hidden);
    });

    mViewMenu->addSeparator();
    mViewMenu->addAction(&mActViewOutput);
    mViewMenu->addSeparator();

    // Toolbutton Mode: how the FSM drawing toolbar renders its buttons (default Icon Only).
    QSettings toolbarSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const Qt::ToolButtonStyle savedStyle = static_cast<Qt::ToolButtonStyle>(
            toolbarSettings.value(QStringLiteral("smDesign/toolbarStyle"), static_cast<int>(Qt::ToolButtonIconOnly)).toInt());

    QMenu* modeMenu = mViewMenu->addMenu(tr("Toolbutton &Mode"));
    QActionGroup* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    const QList<QPair<QString, Qt::ToolButtonStyle>> modes{
          { tr("&Icon Only"),     Qt::ToolButtonIconOnly }
        , { tr("&Text Only"),     Qt::ToolButtonTextOnly }
        , { tr("Icon &and Text"), Qt::ToolButtonTextBesideIcon }
    };
    for (const QPair<QString, Qt::ToolButtonStyle>& mode : modes)
    {
        QAction* modeAction = modeMenu->addAction(mode.first);
        modeAction->setCheckable(true);
        modeAction->setData(static_cast<int>(mode.second));
        modeAction->setChecked(mode.second == savedStyle);
        modeGroup->addAction(modeAction);
    }

    // The chosen style is persisted; each Design page seeds its in-page toolbar from it when
    // built, and onFsmToolbarStyle() applies a change to the active page's toolbar (issue #516).
    connect(modeGroup, &QActionGroup::triggered, this, &MdiMainWindow::onFsmToolbarStyle);

    mViewMenu->addSeparator();
    mThemeMenu = mViewMenu->addMenu(tr("&Theme"));
    mThemeActions = new QActionGroup(this);
    mThemeActions->setExclusive(true);
    const QList<OptionsManager::eAppTheme> themes = NEAppThemes::allThemes();
    for (OptionsManager::eAppTheme theme : themes)
    {
        QAction* action = mThemeMenu->addAction(NEAppThemes::themeDisplayName(theme));
        action->setCheckable(true);
        action->setData(static_cast<int>(theme));
        mThemeActions->addAction(action);
    }

    connect(mThemeActions, &QActionGroup::triggered, this, &MdiMainWindow::onThemeSelected);
    connect(mThemeMenu, &QMenu::aboutToShow, this, &MdiMainWindow::onShowMenuTheme);

    mDesignMenu = menuBar()->addMenu(tr("&Design"));
    connect(mDesignMenu, &QMenu::aboutToShow, this, &MdiMainWindow::onShowMenuDesign);

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
    mFileToolBar->addAction(&mActFileNewFSM);
    mFileToolBar->addAction(&mActFileNewLog);
    mFileToolBar->addAction(&mActFileOpen);
    mFileToolBar->addAction(&mActFileOfflineLog);
    mFileToolBar->addAction(&mActFileSave);
    mFileToolBar->addSeparator();

    mEditToolBar = addToolBar(tr("Edit"));
    mEditToolBar->addAction(&mActEditUndo);
    mEditToolBar->addAction(&mActEditRedo);
    mEditToolBar->addSeparator();
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
    // The navigation and output contents are hosted in ADS dock widgets on the shared manager
    // (issue #516), so they float, tab, and drag together with the State Machine design panels.
    mNaviDockWidget = new ads::CDockWidget(tr("Navigation"), mDockManager);
    mNaviDockWidget->setObjectName(QStringLiteral("NavigationDock"));
    // Insert the content directly (ForceNoScrollArea): the default AutoScrollArea wraps the
    // content in a QScrollArea whose own size hints hide NavigationDock's overrides, so ADS
    // sizes the dock generically (~half the window) and it will not narrow. Inserted directly,
    // NavigationDock's sizeHint (MIN_NAVI_WIDTH) and minimumSize (MIN_NAVI_WIDTH_ABS) govern the
    // dock width, so it opens at the preferred width and shrinks down to 64 px (issue #516).
    mNaviDockWidget->setWidget(&mNaviDock, ads::CDockWidget::ForceNoScrollArea);
    mNaviDockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContentMinimumSize);
    mDockManager->addDockWidget(ads::LeftDockWidgetArea, mNaviDockWidget);

    mOutputDockWidget = new ads::CDockWidget(tr("Output"), mDockManager);
    mOutputDockWidget->setObjectName(QStringLiteral("OutputDock"));
    mOutputDockWidget->setWidget(&mOutputDock, ads::CDockWidget::ForceNoScrollArea);
    mDockManager->addDockWidget(ads::BottomDockWidgetArea, mOutputDockWidget);

    // The editor area starts empty: only Navigation (left) and Output (bottom) surround it. The
    // State Machine drawing toolbar and the Properties/Outline panels are NOT global docks; each
    // Design page hosts its own inside itself and they appear only while that page is shown
    // (issue #516).
}

void MdiMainWindow::_createMdiArea()
{
    // One ADS dock manager owns the whole window (issue #516): the MDI area is its non-closable
    // central widget, and every panel (navigation, output, and the State Machine design panels)
    // is an ADS dock that can float, tab, or dock anywhere against it.
    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    mDockManager = new ads::CDockManager(this);

    mCentralDock = new ads::CDockWidget(tr("Editor"), mDockManager);
    mCentralDock->setObjectName(QStringLiteral("EditorCentral"));
    mCentralDock->setWidget(&mMdiArea);
    mCentralDock->setFeature(ads::CDockWidget::NoTab, true);
    mCentralArea = mDockManager->setCentralWidget(mCentralDock);

    connect(&mMdiArea, &QMdiArea::subWindowActivated, this, &MdiMainWindow::onSubWindowActivated);
}

void MdiMainWindow::showDock(ads::CDockWidget* dock)
{
    if (dock != nullptr)
    {
        dock->toggleView(true);     // opens the dock if it was closed; no-op when already open
        dock->raise();              // brings it to the front of its tab group
    }
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
