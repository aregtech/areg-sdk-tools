/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/MdiArea.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include "lusan/view/common/MdiArea.hpp"
#include <QVBoxLayout>

MdiArea::MdiArea(QWidget* parent /*= nullptr*/)
    : QMdiArea(parent)
{
    setSizeAdjustPolicy(QMdiArea::SizeAdjustPolicy::AdjustIgnored);
    setDocumentMode(true);
    setViewMode(ViewMode::TabbedView);
    setTabsClosable(true);
    setTabsMovable(true);
    setTabShape(QTabWidget::TabShape::Rounded);
    setTabPosition(QTabWidget::TabPosition::North);
}
