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
 *  \file        lusan/application/main/MdiMainWindow.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/
#include "lusan/application/main/MdiMainWindow.hpp"
#include "lusan/application/si/ServiceInterfaceView.hpp"
#include "lusan/common/MdiChild.hpp"

#include <QtWidgets>

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

MdiMainWindow::MdiMainWindow()
    : QMainWindow   ( )
    , mWorkspaceRoot( )
    , mMdiArea      ( this )
    , mNavigation   ( &self() )
    , mStatusDock   ( nullptr )
    , mListView     ( nullptr )
    , mStatusTabs   ( nullptr )
    , mFileMenu     (nullptr)
    , mEditMenu     (nullptr)
    , mWindowMenu   (nullptr)
    , mHelpMenu     (nullptr)
    , mFileToolBar  (nullptr)
    , mEditToolBar  (nullptr)
    , mActFileNewSI (&self())
    , mActFileNewLog(&self())
    , mActFileOpen  (&self())
    , mActFileSave  (&self())
    , mActFileSaveAs(&self())
    , mActFileClose (&self())
    , mActFileCloseAll(&self())
    , mActFileExit  (&self())
    , mActEditCut   (&self())
    , mActEditCopy  (&self())
    , mActEditPaste (&self())
    , mActWindowsTile(&self())
    , mActWindowsCascade(&self())
    , mActWindowsNext(&self())
    , mActWindowsPrev(&self())
    , mActWindowMenuSeparator(&self())
    , mActHelpAbout (nullptr)
    , mActRecentFilesSubMenu(nullptr)
    , mFileSeparator(nullptr)
{
    _createActions();
    _createMenus();
    _createToolBars();
    _createStatusBar();
    _createDockWindows();
    _createMdiArea();
    
    updateWindowMenu();
    updateMenus();
    readSettings();

    setWindowTitle(tr("Lusan"));
    setUnifiedTitleAndToolBarOnMac(true);
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
    MdiChild* child = createMdiChild();
    const bool succeeded = child->loadFile(fileName);
    if (succeeded)
    {
        child->show();
    }
    else
    {
        child->close();
    }

    MdiMainWindow::prependToRecentFiles(fileName);
    return succeeded;
}

void MdiMainWindow::closeEvent(QCloseEvent* event)
{
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
    ServiceInterfaceView* child = createServiceInterfaceView();
    child->newFile();
    child->show();
}

void MdiMainWindow::onFileNewLog()
{
    MdiChild* child = createMdiChild();
    child->newFile();
    child->show();
}

void MdiMainWindow::onFileOpen()
{
    const QString fileName = QFileDialog::getOpenFileName(this);
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
        statusBar()->showMessage(tr("File saved"), 2000);
    }
}

void MdiMainWindow::onFileSaveAs()
{
    MdiChild* child = activeMdiChild();
    if ((child != nullptr) && child->saveAs())
    {
        statusBar()->showMessage(tr("File saved"), 2000);
        MdiMainWindow::prependToRecentFiles(child->currentFile());
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

void MdiMainWindow::prependToRecentFiles(const QString& fileName)
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
    mActRecentFilesSubMenu->setVisible(visible);
    mFileSeparator->setVisible(visible);
}

void MdiMainWindow::updateRecentFileActions()
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

void MdiMainWindow::updateMenus()
{
    MdiChild* active = activeMdiChild();    
    bool hasMdiChild = (active != nullptr);
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
}

void MdiMainWindow::updateWindowMenu()
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

MdiChild* MdiMainWindow::createMdiChild()
{
    MdiChild* child = new MdiChild;
    mMdiArea.addSubWindow(child);
    connect(child, &MdiChild::copyAvailable, &mActEditCut, &QAction::setEnabled);
    connect(child, &MdiChild::copyAvailable, &mActEditCopy, &QAction::setEnabled);

    return child;
}

ServiceInterfaceView* MdiMainWindow::createServiceInterfaceView()
{
    ServiceInterfaceView* child = new ServiceInterfaceView;
    mMdiArea.addSubWindow(child);
    connect(child, &ServiceInterfaceView::copyAvailable, &mActEditCut, &QAction::setEnabled);
    connect(child, &ServiceInterfaceView::copyAvailable, &mActEditCopy, &QAction::setEnabled);
    
    child->showMaximized();
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
    initAction(mActFileNewSI, QIcon::fromTheme(QIcon::ThemeIcon::AppointmentNew), tr("Service &Interface"));
    mActFileNewSI.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_I));
    mActFileNewSI.setStatusTip(tr("Create a new service interface file"));
    connect(&mActFileNewSI, &QAction::triggered, this, &MdiMainWindow::onFileNewSI);

    initAction(mActFileNewLog, QIcon::fromTheme(QIcon::ThemeIcon::ContactNew), tr("&Logs"));
    mActFileNewLog.setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_L));
    mActFileNewLog.setStatusTip(tr("Create a new logs"));
    connect(&mActFileNewLog, &QAction::triggered, this, &MdiMainWindow::onFileNewLog);

    initAction(mActFileOpen, QIcon::fromTheme("document-open", QIcon(":/images/open.png")), tr("&Open..."));
    mActFileOpen.setShortcuts(QKeySequence::Open);
    mActFileOpen.setStatusTip(tr("Open an existing file"));
    connect(&mActFileOpen, &QAction::triggered, this, &MdiMainWindow::onFileOpen);

    initAction(mActFileSave, QIcon::fromTheme("document-save", QIcon(":/images/save.png")), tr("&Save"));
    mActFileSave.setShortcuts(QKeySequence::Save);
    mActFileSave.setStatusTip(tr("Save the document to disk"));
    connect(&mActFileSave, &QAction::triggered, this, &MdiMainWindow::onFileSave);

    initAction(mActFileSaveAs, QIcon::fromTheme("document-save-as"), tr("Save &As..."));
    mActFileSaveAs.setShortcuts(QKeySequence::SaveAs);
    mActFileSaveAs.setStatusTip(tr("Save the document under a new name"));
    connect(&mActFileSaveAs, &QAction::triggered, this, &MdiMainWindow::onFileSaveAs);

    initAction(mActFileExit, QIcon::fromTheme("application-exit"), tr("E&xit"));
    mActFileExit.setParent(qApp);
    mActFileExit.setShortcuts(QKeySequence::Quit);
    mActFileExit.setStatusTip(tr("Exit the application"));
    connect(&mActFileExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    
    initAction(mActEditCut, QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png")), tr("Cu&t"));
    mActEditCut.setShortcuts(QKeySequence::Cut);
    mActEditCut.setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(&mActEditCut, &QAction::triggered, this, &MdiMainWindow::onEditCut);

    initAction(mActEditCopy, QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png")), tr("&Copy"));
    mActEditCopy.setShortcuts(QKeySequence::Copy);
    mActEditCopy.setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(&mActEditCopy, &QAction::triggered, this, &MdiMainWindow::onEditCopy);

    initAction(mActEditPaste, QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png")), tr("&Paste"));
    mActEditPaste.setShortcuts(QKeySequence::Paste);
    mActEditPaste.setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(&mActEditPaste, &QAction::triggered, this, &MdiMainWindow::onEditPaste);

    initAction(mActFileClose, QIcon(), tr("Cl&ose"));
    mActFileClose.setStatusTip(tr("Close the active window"));
    connect(&mActFileClose, &QAction::triggered, &mMdiArea, &QMdiArea::closeActiveSubWindow);

    initAction(mActFileCloseAll, QIcon(), tr("Close &All"));
    mActFileCloseAll.setStatusTip(tr("Close all the windows"));
    connect(&mActFileCloseAll, &QAction::triggered, &mMdiArea, &QMdiArea::closeAllSubWindows);

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
    mFileMenu->addAction(&mActFileNewSI);
    mFileMenu->addAction(&mActFileNewLog);
    mFileMenu->addAction(&mActFileOpen);
    mFileMenu->addAction(&mActFileSaveAs);
    mFileSeparator = mFileMenu->addSeparator();

    QMenu* recentMenu = mFileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MdiMainWindow::updateRecentFileActions);
    mActRecentFilesSubMenu = recentMenu->menuAction();
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

    mWindowMenu = menuBar()->addMenu(tr("&Window"));
    connect(mWindowMenu, &QMenu::aboutToShow, this, &MdiMainWindow::updateWindowMenu);

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
    mFileToolBar->addAction(&mActFileOpen);
    mFileToolBar->addAction(&mActFileSave);

    mEditToolBar = addToolBar(tr("Edit"));
    mEditToolBar->addAction(&mActEditCut);
    mEditToolBar->addAction(&mActEditCopy);
    mEditToolBar->addAction(&mActEditPaste);
}

void MdiMainWindow::_createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MdiMainWindow::_createDockWindows()
{
    addDockWidget(Qt::LeftDockWidgetArea, &mNavigation);

    mStatusDock = new QDockWidget(tr("Status"), this);
    mStatusTabs = new QTabWidget;
    mListView = new QListView;
    mStatusTabs->addTab(mListView, tr("Output"));
    mStatusDock->setWidget(mStatusTabs);
    addDockWidget(Qt::BottomDockWidgetArea, mStatusDock);
}

void MdiMainWindow::_createMdiArea()
{
    setCentralWidget(&mMdiArea);
    connect(&mMdiArea, &QMdiArea::subWindowActivated, this, &MdiMainWindow::updateMenus);
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
