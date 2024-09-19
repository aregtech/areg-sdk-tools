#ifndef LUSAN_APPLICATION_MAIN_WORKSPACE_HPP
#define LUSAN_APPLICATION_MAINWORKSPACE_HPP

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class DialogWorkspace;
}
QT_END_NAMESPACE

class Workspace : public QDialog
{
    Q_OBJECT
public:
    Workspace(QWidget * parent = nullptr);
    virtual ~Workspace(void);
    
public:
    inline const QString & getRootDirectory( void ) const;
    
protected:
    void onAccept(void);
    
    void onReject(void);
    
    void onWorskpacePathChanged(const QString & newText);
    
    void onBrowseClicked(bool checked = true);
    
private:
    Ui::DialogWorkspace * mWorkspace;
    QString mRoot;
    QString mDescribe;
};

inline const QString & Workspace::getRootDirectory( void ) const
{
    return mRoot;
}

#endif // LUSAN_APPLICATION_MAINWORKSPACE_HPP
