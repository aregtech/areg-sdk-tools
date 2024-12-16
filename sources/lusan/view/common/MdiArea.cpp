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
 *  \file        lusan/view/common/MdiArea.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include "lusan/view/common/MdiArea.hpp"
#include <QVBoxLayout>

MdiArea::MdiArea(QWidget* parent /*= nullptr*/)
    : QMdiArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizeAdjustPolicy(QMdiArea::SizeAdjustPolicy::AdjustToContents);
    setViewMode(ViewMode::TabbedView);
    setTabsClosable(true);
    setTabsMovable(true);
    setTabShape(QTabWidget::TabShape::Triangular);
    setTabPosition(QTabWidget::TabPosition::North);
}
