#ifndef LUSAN_VIEW_COMMON_WORKSPACE_HPP
#define LUSAN_VIEW_COMMON_WORKSPACE_HPP
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
 *  \file        lusan/view/common/Workspace.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Workspace setup dialog.
 *
 ************************************************************************/
#include <QDialog>

namespace Ui {
class DialogWorkspace;
}
QT_BEGIN_NAMESPACE
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

#endif // LUSAN_VIEW_COMMON_WORKSPACE_HPP
