#ifndef LUSAN_APPLICATION_MAIN_MDIMAINWINDOW_HPP
#define LUSAN_APPLICATION_MAIN_MDIMAINWINDOW_HPP
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
 *  \file        lusan/application/main/MdiMainWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include <QMainWindow>

class MdiChild;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
QT_END_NAMESPACE

class MdiMainWindow : public QMainWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////
public:
    MdiMainWindow();

    bool openFile(const QString& fileName);

    inline void setWorkspaceRoot(const QString& workspace);

    inline const QString& getWorkspaceRoot(void) const;

//////////////////////////////////////////////////////////////////////////
// protected methods
//////////////////////////////////////////////////////////////////////////
protected:
    void closeEvent(QCloseEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void updateRecentFileActions();
    void openRecentFile();
    void cut();
    void copy();
    void paste();
    void about();
    void updateMenus();
    void updateWindowMenu();
    MdiChild* createMdiChild();
    void switchLayoutDirection();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    enum { MaxRecentFiles = 5 };

    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool loadFile(const QString& fileName);
    static bool hasRecentFiles();
    void prependToRecentFiles(const QString& fileName);
    void setRecentFilesVisible(bool visible);
    MdiChild* activeMdiChild() const;
    QMdiSubWindow* findMdiChild(const QString& fileName) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    QString  mWorkspaceRoot;

    QMdiArea* mdiArea;

    QMenu* windowMenu;
    QAction* newAct;
    QAction* saveAct;
    QAction* saveAsAct;
    QAction* recentFileActs[MaxRecentFiles];
    QAction* recentFileSeparator;
    QAction* recentFileSubMenuAct;
    QAction* cutAct;
    QAction* copyAct;
    QAction* pasteAct;
    QAction* closeAct;
    QAction* closeAllAct;
    QAction* tileAct;
    QAction* cascadeAct;
    QAction* nextAct;
    QAction* previousAct;
    QAction* windowMenuSeparatorAct;
};

inline void MdiMainWindow::setWorkspaceRoot(const QString& workspace)
{
    mWorkspaceRoot = workspace;
}

inline const QString& MdiMainWindow::getWorkspaceRoot(void) const
{
    return mWorkspaceRoot;
}

#endif // LUSAN_APPLICATION_MAIN_MDIMAINWINDOW_HPP
