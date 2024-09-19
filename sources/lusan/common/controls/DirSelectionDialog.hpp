#ifndef LUSAN_COMMON_CONTROLS_DIRSELECTIONDIALOG_HPP
#define LUSAN_COMMON_CONTROLS_DIRSELECTIONDIALOG_HPP

#include <QDialog>
#include <QDir>

class QTreeView;
class QFileSystemModel;
class QLineEdit;
class QPushButton;

class DirSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    DirSelectionDialog(QWidget* parent = nullptr);
        
    DirSelectionDialog(const QString & curDir, QWidget* parent = nullptr);
    
public:
    QDir getDirectory() const;

protected:
    virtual void onCurrentDirChanged();
    
private:
    
    void _initialize(const QString & curDir);
    
private:
    QTreeView* mTreeViewDirs;
    QFileSystemModel* mModel;
    QLineEdit* mDirName;
    QPushButton* mButtonOK;
}; 

#endif  // LUSAN_COMMON_CONTROLS_DIRSELECTIONDIALOG_HPP
