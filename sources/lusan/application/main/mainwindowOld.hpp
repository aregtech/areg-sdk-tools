#ifndef LUSAN_APPLICATION_MAIN_MAINWINDOW_HPP
#define LUSAN_APPLICATION_MAIN_MAINWINDOW_HPP
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
 *  \file        lusan/application/main/mainwindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MainWindow setup.
 *
 ************************************************************************/

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QMdiArea;
class QMdiSubWindow;

QT_END_NAMESPACE
class MdiChild;

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

    bool openFile(const QString& fileName);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:

    void onFileNewSI(void);
    void onFileNewLog(void);
    void onFileSave(void);
    void onFileSaveAs(void);
    void onFileOpen(void);
    void onFileExit(void);
    void onHelpAbout(void);

private:
    void _createActionsOld(void);
    void _createMenus(void);
    void _createWndMain(void);



    void _createMdiArea(void);
    void _createActions();
    void _createStatusBar();
    void _readSettings();
    void _writeSettings();

    void _updateMenus(void);
    void _updateWindowMenu(void);
    MdiChild* createMdiChild();
    void switchLayoutDirection(); 

    bool loadFile(const QString& fileName);
    static bool hasRecentFiles();
    void prependToRecentFiles(const QString& fileName);
    void setRecentFilesVisible(bool visible);
    MdiChild* activeMdiChild() const;
    QMdiSubWindow* findMdiChild(const QString& fileName) const; 

private:
    Ui::MainWindow *mWndMain;
    QMdiArea*       mMdiArea;
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

#endif // LUSAN_APPLICATION_MAIN_MAINWINDOW_HPP
