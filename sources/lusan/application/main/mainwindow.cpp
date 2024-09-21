#include "lusan/application/main/mainwindow.hpp"
#include "ui/ui_mainwindow.h"

#include <QDir>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent /*= nullptr*/)
    : QMainWindow   (parent)
    , mWndMain      (new Ui::MainWindow)
    , mWorkspaceRoot(QDir::homePath())
{
    mWndMain->setupUi(this);
    _createWndMain();
}

MainWindow::MainWindow(const QString & workspaceRoot, QWidget *parent /*= nullptr*/)
    : QMainWindow   (parent)
    , mWndMain      (new Ui::MainWindow)
    , mWorkspaceRoot(workspaceRoot)
{
    mWndMain->setupUi(this);
    _createWndMain();
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

void MainWindow::_createActions(void)
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
