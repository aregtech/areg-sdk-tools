#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}

class QAction;
class QActionGroup;
class QLabel;
class QMenu;

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

private slots:

    void onFileNewSI(void);
    void onFileNewLog(void);
    void onFileSave(void);
    void onFileSaveAs(void);
    void onFileOpen(void);
    void onFileExit(void);
    void onHelpAbout(void);

private:
    void _createActions(void);
    void _createMenus(void);
    void _createWndMain(void);

private:
    Ui::MainWindow *mWndMain;
    QLabel *        mInfoBar;
    QMenu *         mMenuFile;
    QMenu *         mMenuHelp;
    QAction *       mActFileNewSI;
    QAction *       mActFileNewLog;
    QAction *       mActFileSave;
    QAction *       mActFileSaveAs;
    QAction *       mActFileOpen;
    QAction *       mActFileExit;
    QAction *       mActHelpAbout;

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
