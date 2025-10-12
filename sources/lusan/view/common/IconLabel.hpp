#ifndef LUSAN_VIEW_COMMON_ICONLABEL_HPP
#define LUSAN_VIEW_COMMON_ICONLABEL_HPP
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
 *  \file        lusan/view/common/IconLabel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, QLabel instance with icon.
 *
 ************************************************************************/

#include "areg/base/GEGlobal.h"
#include <QLabel>

class IconLabel : public QLabel
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    IconLabel(QWidget* parent = nullptr);
    virtual ~IconLabel(void) = default;

public:

    void setText(const QString& text);

    void setText(const QIcon& icon, const QString& text);

private:
    QIcon   mIcon;
};

#endif  // LUSAN_VIEW_COMMON_ICONLABEL_HPP
