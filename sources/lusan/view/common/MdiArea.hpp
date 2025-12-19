#ifndef LUSAN_VIEW_COMMON_MDIAREA_HPP
#define LUSAN_VIEW_COMMON_MDIAREA_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/MdiArea.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, MDI Area.
 *
 ************************************************************************/

#include <QMdiArea>
#include <QWidget>

class MdiArea : public QMdiArea
{
    Q_OBJECT
public:
    MdiArea(QWidget * parent = nullptr);
};

#endif // LUSAN_VIEW_COMMON_MDIAREA_HPP
