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
 *  \file        lusan/application/main/Workspace.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Workspace setup dialog.
 *
 ************************************************************************/

#ifndef LUSAN_APPLICATION_MAIN_WORKSPACE_HPP
#define LUSAN_APPLICATION_MAIN_WORKSPACE_HPP

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

#endif // LUSAN_APPLICATION_MAIN_WORKSPACE_HPP
