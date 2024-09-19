#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    
    MainWindow(const QString & workspaceRoot, QWidget *parent = nullptr);
    
    ~MainWindow();
    
public:
    inline void setWorkspaceRoot(const QString & rootDir);
    
    inline const QString & getWorkspaceRoot(void) const;

private:
    Ui::MainWindow *ui;
    QString         mWorkspaceRoot;
};

inline void MainWindow::setWorkspaceRoot(const QString & rootDir)
{
    mWorkspaceRoot = rootDir;
}

inline const QString & MainWindow::getWorkspaceRoot(void) const
{
    return mWorkspaceRoot;
}

#endif // MAINWINDOW_HPP
