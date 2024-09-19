#include "lusan/application/main/mainwindow.hpp"
#include "ui/ui_mainwindow.h"

#include <QDir>

MainWindow::MainWindow(QWidget *parent /*= nullptr*/)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mWorkspaceRoot(QDir::homePath())
{
    ui->setupUi(this);
}

MainWindow::MainWindow(const QString & workspaceRoot, QWidget *parent /*= nullptr*/)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mWorkspaceRoot(workspaceRoot)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
