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
 *  \file        lusan/application/main/mainwindow.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MainWindow setup.
 *
 ************************************************************************/

#include "lusan/application/main/mainwindow.hpp"
#include "ui/ui_mainwindow.h"

#include <QDir>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent /*= nullptr*/)
    : QMainWindow   (parent)
    , mmMdiArea      (new QmMdiArea)
    // , mWndMain      (new Ui::MainWindow)
    , mWndMain      (nullptr)
    , mWorkspaceRoot(QDir::homePath())
{
    // mWndMain->setupUi(this);
    // _createWndMain();
    _createMdiArea();
}

MainWindow::MainWindow(const QString & workspaceRoot, QWidget *parent /*= nullptr*/)
    : QMainWindow   (parent)
    , mWndMain      (new Ui::MainWindow)
    , mWorkspaceRoot(workspaceRoot)
{
    // mWndMain->setupUi(this);
    // _createWndMain();
    _createMdiArea();
}

MainWindow::~MainWindow()
{
    delete mWndMain;
}

void MainWindow::_createWndMain(void)
{
    QWidget* widget = new QWidget;
    setCentralWidget(widget);

    QWidget* topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mInfoBar = new QLabel(tr("<i>Choose a menu option, or right-click to invoke a context menu</i>"));
    mInfoBar->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mInfoBar->setAlignment(Qt::AlignCenter);

    QWidget* bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(topFiller);
    layout->addWidget(mInfoBar);
    layout->addWidget(bottomFiller);
    widget->setLayout(layout);
    
    _createActions();
    _createMenus();
    
    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);
    
    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void MainWindow::_createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::_readSettings()
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

void MainWindow::_createMdiArea(void)
{
    mMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mMdiArea);
    connect(mMdiArea, &QmMdiArea::subWindowActivated, this, &MainWindow::updateMenus);
}

void MainWindow::_createActions()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar* fileToolBar = addToolBar(tr("File"));
    
    mActFileNewSI = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::AppointmentNew), tr("Service &Interface"), this);
    mActFileNewSI->setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_I));
    mActFileNewSI->setStatusTip(tr("Create a new service interface file"));
    connect(mActFileNewSI, &QAction::triggered, this, &MainWindow::onFileNewSI);
    fileMenu->addAction(mActFileNewSI);
    fileToolBar->addAction(mActFileNewSI);
    
    mActFileNewLog = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ContactNew), tr("&Logs"), this);
    mActFileNewLog->setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_L));
    mActFileNewLog->setStatusTip(tr("Create a new logs"));
    connect(mActFileNewLog, &QAction::triggered, this, &MainWindow::onFileNewLog);
    fileMenu->addAction(mActFileNewLog);
    fileToolBar->addAction(mActFileNewLog);
    
    mActFileOpen = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen), tr("&Open ..."), this);
    mActFileOpen->setShortcuts(QKeySequence::StandardKey::Open);
    mActFileOpen->setStatusTip(tr("Open an existing file"));
    connect(mActFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    fileMenu->addAction(mActFileOpen);
    fileToolBar->addAction(mActFileOpen);
    
    mActFileSave = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), tr("&Save"), this);
    mActFileSave->setShortcuts(QKeySequence::StandardKey::Save);
    mActFileSave->setStatusTip(tr("Save the document to disk"));
    connect(mActFileSave, &QAction::triggered, this, &MainWindow::onFileSave);
    fileMenu->addAction(mActFileSave);
    fileToolBar->addAction(mActFileSave);
    
    mActFileSaveAs = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs), tr("Save &As ..."), this);
    mActFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    mActFileSaveAs->setStatusTip(tr("Save the document to disk and change the name"));
    connect(mActFileSaveAs, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    fileMenu->addAction(mActFileSaveAs);
    fileToolBar->addAction(mActFileSaveAs);

    fileMenu->addSeparator();

    QMenu* recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::onRecentFileUpdate);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::onOpenRecentFile);
        recentFileActs[i]->setVisible(false);
    }

    recentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());

    fileMenu->addAction(tr("Switch layout direction"), this, &MainWindow::switchLayoutDirection);
    fileMenu->addSeparator();
    
    mActFileExit = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit), tr("E&xit"), this);
    // mActFileExit->setShortcuts(QKeySequence::Quit);
    mActFileExit->setShortcut(QKeyCombination(Qt::Modifier::ALT, Qt::Key::Key_F4));
    mActFileExit->setStatusTip(tr("Exit the application"));
    connect(mActFileExit, &QAction::triggered, this, &MainWindow::onFileExit);
    fileMenu->addAction(mActFileExit);

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar* editToolBar = addToolBar(tr("Edit"));

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    mActEditCut = new QAction(cutIcon, tr("Cu&t"), this);
    mActEditCut->setShortcuts(QKeySequence::Cut);
    mActEditCut->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(mActEditCut, &QAction::triggered, this, &MainWindow::onEditCut);
    editMenu->addAction(mActEditCut);
    editToolBar->addAction(mActEditCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    mActEditCopy = new QAction(copyIcon, tr("&Copy"), this);
    mActEditCopy->setShortcuts(QKeySequence::Copy);
    mActEditCopy->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(mActEditCopy, &QAction::triggered, this, &MainWindow::onEditCopy);
    editMenu->addAction(mActEditCopy);
    editToolBar->addAction(mActEditCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    mActEditPaste = new QAction(pasteIcon, tr("&Paste"), this);
    mActEditPaste->setShortcuts(QKeySequence::Paste);
    mActEditPaste->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(mActEditPaste, &QAction::triggered, this, &MainWindow::onEditPaste);
    editMenu->addAction(mActEditPaste);
    editToolBar->addAction(mActEditPaste);

    windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

    mActWindowClose = new QAction(tr("Cl&ose"), this);
    mActWindowClose->setStatusTip(tr("Close the active window"));
    connect(mActWindowClose, &QAction::triggered, mMdiArea, &QMdiArea::onWindowCloseActive);

    mActWindowCloseAll = new QAction(tr("Close &All"), this);
    mActWindowCloseAll->setStatusTip(tr("Close all the windows"));
    connect(mActWindowCloseAll, &QAction::triggered, mMdiArea, &QMdiArea::onWindowCloseAll);

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, &QAction::triggered, mMdiArea, &QMdiArea::tileSubWindows);

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, &QAction::triggered, mMdiArea, &QMdiArea::cascadeSubWindows);

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, &QAction::triggered, mMdiArea, &QMdiArea::activateNextSubWindow);

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
        "window"));
    connect(previousAct, &QAction::triggered, mMdiArea, &QMdiArea::activatePreviousSubWindow);

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);

    updateWindowMenu();

    menuBar()->addSeparator();

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction* aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction* aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::_createActionsOld(void)
{
    mActFileNewSI = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::AppointmentNew), tr("Service &Interface"), this);
    mActFileNewSI->setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_I));
    mActFileNewSI->setStatusTip(tr("Create a new service interface file"));
    connect(mActFileNewSI, &QAction::triggered, this, &MainWindow::onFileNewSI);
    
    mActFileNewLog = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ContactNew), tr("&Logs"), this);
    mActFileNewLog->setShortcut(QKeyCombination(Qt::Modifier::CTRL, Qt::Key::Key_L));
    mActFileNewLog->setStatusTip(tr("Create a new logs"));
    connect(mActFileNewLog, &QAction::triggered, this, &MainWindow::onFileNewLog);
    
    mActFileOpen = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen), tr("&Open ..."), this);
    mActFileOpen->setShortcuts(QKeySequence::StandardKey::Open);
    mActFileOpen->setStatusTip(tr("Open an existing file"));
    connect(mActFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    
    mActFileSave = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), tr("&Save"), this);
    mActFileSave->setShortcuts(QKeySequence::StandardKey::Save);
    mActFileSave->setStatusTip(tr("Save the document to disk"));
    connect(mActFileSave, &QAction::triggered, this, &MainWindow::onFileSave);
    
    mActFileSaveAs = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs), tr("Save &As ..."), this);
    mActFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    mActFileSaveAs->setStatusTip(tr("Save the document to disk and change the name"));
    connect(mActFileSaveAs, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    
    mActFileExit = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit), tr("E&xit"), this);
    // mActFileExit->setShortcuts(QKeySequence::Quit);
    mActFileExit->setShortcut(QKeyCombination(Qt::Modifier::ALT, Qt::Key::Key_F4));
    mActFileExit->setStatusTip(tr("Exit the application"));
    connect(mActFileExit, &QAction::triggered, this, &MainWindow::onFileExit);
    
    mActHelpAbout = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout), tr("&About"), this);
    mActHelpAbout->setStatusTip(tr("About Lusan tool"));
    connect(mActHelpAbout, &QAction::triggered, this, &MainWindow::onHelpAbout);
}

void MainWindow::_createMenus(void)
{
    mMenuFile = menuBar()->addMenu(tr("&File"));
    auto * fileNew = mMenuFile->addMenu(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), tr("&New"));
    fileNew->addAction(mActFileNewSI);
    fileNew->addAction(mActFileNewLog);
    mMenuFile->addAction(mActFileOpen);
    mMenuFile->addAction(mActFileSave);
    mMenuFile->addAction(mActFileSaveAs);
    mMenuFile->addSeparator();
    mMenuFile->addAction(mActFileExit);
    
    mMenuHelp = menuBar()->addMenu(tr("&Help"));
    mMenuHelp->addAction(mActHelpAbout);
}

void MainWindow::onFileNewSI()
{
    mInfoBar->setText(tr("Invoked <b>File|New|Service Interface</b>"));
}

void MainWindow::onFileNewLog()
{
    mInfoBar->setText(tr("Invoked <b>File|New|Log</b>"));
}

void MainWindow::onFileOpen()
{
    mInfoBar->setText(tr("Invoked <b>File|Open</b>"));
}

void MainWindow::onFileSave()
{
    mInfoBar->setText(tr("Invoked <b>File|Save</b>"));
}

void MainWindow::onFileSaveAs()
{
    mInfoBar->setText(tr("Invoked <b>File|Save As ...</b>"));
}

void MainWindow::onFileExit()
{
    QMainWindow::close();
}

void MainWindow::onHelpAbout()
{
    mInfoBar->setText(tr("<b>About AREG SDK tool</b>"));
    QMessageBox::about(  this
                       , tr("About Lusan")
                       , tr("<b>Lusan</b> is an user interface tool for applications using"
                            "<b>AREG Communication Framework</b> to create service interface files and analyze logs."));
}
