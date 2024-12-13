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
#include "lusan/application/main/MdiChild.hpp"

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
    : mMdiArea(new QMdiArea)
{
    mMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mMdiArea);
    connect(mMdiArea, &QMdiArea::subWindowActivated, this, &MdiMainWindow::updateMenus);

    _createActions();
    _createStatusBar();
    updateMenus();

    _readSettings();

    setWindowTitle(tr("MDI"));
    setUnifiedTitleAndToolBarOnMac(true);
}

bool MdiMainWindow::openFile(const QString& fileName)
{
    bool result{ false };
    if (QMdiSubWindow* existing = _findMdiChild(fileName))
    {
        mMdiArea->setActiveSubWindow(existing);
        result = true;
    }
    else if (_loadFile(fileName))
    {
        statusBar()->showMessage(tr("File loaded"), 2000);
        result = true;
    }

    return result;
}

bool MdiMainWindow::_loadFile(const QString& fileName)
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

    MdiMainWindow::_prependToRecentFiles(fileName);
    return succeeded;
}

void MdiMainWindow::closeEvent(QCloseEvent* event)
{
    mMdiArea->closeAllSubWindows();
    if (mMdiArea->currentSubWindow())
    {
        event->ignore();
    }
    else
    {
        _writeSettings();
        event->accept();
    }
}

void MdiMainWindow::onFileNewSI()
{
    MdiChild* child = createMdiChild();
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
    if (_activeMdiChild() && _activeMdiChild()->save())
    {
        statusBar()->showMessage(tr("File saved"), 2000);
    }
}

void MdiMainWindow::onFileSaveAs()
{
    MdiChild* child = _activeMdiChild();
    if (child && child->saveAs())
    {
        statusBar()->showMessage(tr("File saved"), 2000);
        MdiMainWindow::_prependToRecentFiles(child->currentFile());
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
    if (_activeMdiChild())
    {
        _activeMdiChild()->cut();
    }
}

void MdiMainWindow::onEditCopy()
{
    if (_activeMdiChild())
    {
        _activeMdiChild()->copy();
    }
}

void MdiMainWindow::onEditPaste()
{
    if (_activeMdiChild())
    {
        _activeMdiChild()->paste();
    }
}

void MdiMainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About Lusan"), tr("The <b>Lusan</b> in under construction."));
}

bool MdiMainWindow::_hasRecentFiles()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MdiMainWindow::_prependToRecentFiles(const QString& fileName)
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

    _setRecentFilesVisibility(!recentFiles.isEmpty());
}

void MdiMainWindow::_setRecentFilesVisibility(bool visible)
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
    bool hasMdiChild = (_activeMdiChild() != nullptr);
    mActFileSave->setEnabled(hasMdiChild);
    mActFileSaveAs->setEnabled(hasMdiChild);
    mActEditPaste->setEnabled(hasMdiChild);
    mActFileClose->setEnabled(hasMdiChild);
    mActFileCloseAll->setEnabled(hasMdiChild);
    mActWindowsTile->setEnabled(hasMdiChild);
    mActWindowsCascade->setEnabled(hasMdiChild);
    mActWindowsNext->setEnabled(hasMdiChild);
    mActWindowsPrev->setEnabled(hasMdiChild);
    mActWindowMenuSeparator->setVisible(hasMdiChild);

    bool hasSelection = (_activeMdiChild() && _activeMdiChild()->textCursor().hasSelection());
    mActEditCut->setEnabled(hasSelection);
    mActEditCopy->setEnabled(hasSelection);
}

void MdiMainWindow::updateWindowMenu()
{
    mMenuWindow->clear();
    mMenuWindow->addAction(mActFileClose);
    mMenuWindow->addAction(mActFileCloseAll);
    mMenuWindow->addSeparator();
    mMenuWindow->addAction(mActWindowsTile);
    mMenuWindow->addAction(mActWindowsCascade);
    mMenuWindow->addSeparator();
    mMenuWindow->addAction(mActWindowsNext);
    mMenuWindow->addAction(mActWindowsPrev);
    mMenuWindow->addAction(mActWindowMenuSeparator);

    QList<QMdiSubWindow*> windows = mMdiArea->subWindowList();
    mActWindowMenuSeparator->setVisible(!windows.isEmpty());

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

        QAction* action = mMenuWindow->addAction(  text
                                                , mdiSubWindow
                                                , [this, mdiSubWindow]() {
                                                    mMdiArea->setActiveSubWindow(mdiSubWindow);
                                                    }
        );

        action->setCheckable(true);
        action->setChecked(child == _activeMdiChild());
    }
}

MdiChild* MdiMainWindow::createMdiChild()
{
    MdiChild* child = new MdiChild;
    mMdiArea->addSubWindow(child);
    connect(child, &QTextEdit::copyAvailable, mActEditCut, &QAction::setEnabled);
    connect(child, &QTextEdit::copyAvailable, mActEditCopy, &QAction::setEnabled);

    return child;
}

void MdiMainWindow::_createActions()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar* fileToolBar = addToolBar(tr("File"));

    mActFileNewSI = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::AppointmentNew), tr("Service &Interface"), this);
    mActFileNewSI->setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_I));
    mActFileNewSI->setStatusTip(tr("Create a new service interface file"));
    connect(mActFileNewSI, &QAction::triggered, this, &MdiMainWindow::onFileNewSI);
    fileMenu->addAction(mActFileNewSI);
    fileToolBar->addAction(mActFileNewSI);

    mActFileNewLog = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ContactNew), tr("&Logs"), this);
    mActFileNewLog->setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_L));
    mActFileNewLog->setStatusTip(tr("Create a new logs"));
    connect(mActFileNewLog, &QAction::triggered, this, &MdiMainWindow::onFileNewLog);
    fileMenu->addAction(mActFileNewLog);
    fileToolBar->addAction(mActFileNewLog);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction* openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MdiMainWindow::onFileOpen);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    mActFileSave = new QAction(saveIcon, tr("&Save"), this);
    mActFileSave->setShortcuts(QKeySequence::Save);
    mActFileSave->setStatusTip(tr("Save the document to disk"));
    connect(mActFileSave, &QAction::triggered, this, &MdiMainWindow::onFileSave);
    fileToolBar->addAction(mActFileSave);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    mActFileSaveAs = new QAction(saveAsIcon, tr("Save &As..."), this);
    mActFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    mActFileSaveAs->setStatusTip(tr("Save the document under a new name"));
    connect(mActFileSaveAs, &QAction::triggered, this, &MdiMainWindow::onFileSaveAs);
    fileMenu->addAction(mActFileSaveAs);

    fileMenu->addSeparator();

    QMenu* recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MdiMainWindow::updateRecentFileActions);
    mActRecentFilesSubMenu = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        mActsRecentFiles[i] = recentMenu->addAction(QString(), this, &MdiMainWindow::onFileOpenRecent);
        mActsRecentFiles[i]->setVisible(false);
    }

    mFileSeparator = fileMenu->addSeparator();

    _setRecentFilesVisibility(MdiMainWindow::_hasRecentFiles());

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction* exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), qApp, &QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar* editToolBar = addToolBar(tr("Edit"));

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    mActEditCut = new QAction(cutIcon, tr("Cu&t"), this);
    mActEditCut->setShortcuts(QKeySequence::Cut);
    mActEditCut->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(mActEditCut, &QAction::triggered, this, &MdiMainWindow::onEditCut);
    editMenu->addAction(mActEditCut);
    editToolBar->addAction(mActEditCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    mActEditCopy = new QAction(copyIcon, tr("&Copy"), this);
    mActEditCopy->setShortcuts(QKeySequence::Copy);
    mActEditCopy->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(mActEditCopy, &QAction::triggered, this, &MdiMainWindow::onEditCopy);
    editMenu->addAction(mActEditCopy);
    editToolBar->addAction(mActEditCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    mActEditPaste = new QAction(pasteIcon, tr("&Paste"), this);
    mActEditPaste->setShortcuts(QKeySequence::Paste);
    mActEditPaste->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(mActEditPaste, &QAction::triggered, this, &MdiMainWindow::onEditPaste);
    editMenu->addAction(mActEditPaste);
    editToolBar->addAction(mActEditPaste);

    mMenuWindow = menuBar()->addMenu(tr("&Window"));
    connect(mMenuWindow, &QMenu::aboutToShow, this, &MdiMainWindow::updateWindowMenu);

    mActFileClose = new QAction(tr("Cl&ose"), this);
    mActFileClose->setStatusTip(tr("Close the active window"));
    connect(mActFileClose, &QAction::triggered,mMdiArea, &QMdiArea::closeActiveSubWindow);

    mActFileCloseAll = new QAction(tr("Close &All"), this);
    mActFileCloseAll->setStatusTip(tr("Close all the windows"));
    connect(mActFileCloseAll, &QAction::triggered, mMdiArea, &QMdiArea::closeAllSubWindows);

    mActWindowsTile = new QAction(tr("&Tile"), this);
    mActWindowsTile->setStatusTip(tr("Tile the windows"));
    connect(mActWindowsTile, &QAction::triggered, mMdiArea, &QMdiArea::tileSubWindows);

    mActWindowsCascade = new QAction(tr("&Cascade"), this);
    mActWindowsCascade->setStatusTip(tr("Cascade the windows"));
    connect(mActWindowsCascade, &QAction::triggered, mMdiArea, &QMdiArea::cascadeSubWindows);

    mActWindowsNext = new QAction(tr("Ne&xt"), this);
    mActWindowsNext->setShortcuts(QKeySequence::NextChild);
    mActWindowsNext->setStatusTip(tr("Move the focus to the next window"));
    connect(mActWindowsNext, &QAction::triggered, mMdiArea, &QMdiArea::activateNextSubWindow);

    mActWindowsPrev = new QAction(tr("Pre&vious"), this);
    mActWindowsPrev->setShortcuts(QKeySequence::PreviousChild);
    mActWindowsPrev->setStatusTip(tr("Move the focus to the previous window"));
    connect(mActWindowsPrev, &QAction::triggered, mMdiArea, &QMdiArea::activatePreviousSubWindow);

    mActWindowMenuSeparator = new QAction(this);
    mActWindowMenuSeparator->setSeparator(true);

    updateWindowMenu();

    menuBar()->addSeparator();

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction* aboutAct = helpMenu->addAction(tr("&About"), this, &MdiMainWindow::onHelpAbout);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}

void MdiMainWindow::_createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MdiMainWindow::_readSettings()
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

void MdiMainWindow::_writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

MdiChild* MdiMainWindow::_activeMdiChild() const
{
    QMdiSubWindow* activeSubWindow = mMdiArea->activeSubWindow();
    return (activeSubWindow != nullptr ? qobject_cast<MdiChild*>(activeSubWindow->widget()) : nullptr);
}

QMdiSubWindow* MdiMainWindow::_findMdiChild(const QString& fileName) const
{
    QMdiSubWindow* result{ nullptr };
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    const QList<QMdiSubWindow*> subWindows = mMdiArea->subWindowList();
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
