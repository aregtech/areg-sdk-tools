#ifndef LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
#define LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/NaviFileSystem.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The view of the workspace related file system.
 *
 ************************************************************************/

#include <QList>
#include <QString>
#include <QWidget>

namespace Ui {
    class NaviFileSystem;
}

class NaviFileSystem : public QWidget
{
private:
    static constexpr uint32_t  MIN_WIDTH{280};
    static constexpr uint32_t  MIN_HEIGHT{280};
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NaviFileSystem(QWidget* parent = nullptr);
    
    NaviFileSystem(const QList<QString> & filters, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// public methods
//////////////////////////////////////////////////////////////////////////
public:

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    QList<QString>      mListFilters;
    Ui::NaviFileSystem* ui;
};

#endif  // LUSAN_VIEW_COMMON_NAVIFILESYSTEM_HPP
