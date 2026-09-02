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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/AttributeListView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "attribute list" panel implementation.
 *
 ************************************************************************/

#include "lusan/view/common/AttributeListView.hpp"

#include <QStringList>

namespace
{
    // The third column depends on the editor: a state machine attribute carries a default value, a
    // service interface attribute an update notification kind. Exactly one config flag is set.
    ElementListConfig listConfigOf(const AttributeViewConfig& config)
    {
        QStringList headers{ QObject::tr("Name:"), QObject::tr("Data Type:") };
        if (config.hasValue)
        {
            headers.append(QObject::tr("Value:"));
        }
        if (config.hasNotification)
        {
            headers.append(QObject::tr("Notification Type:"));
        }

        return ElementListConfig{ QObject::tr("Attributes List:"), headers, QObject::tr("attribute"), QString(), true };
    }
}

AttributeListView::AttributeListView(const AttributeViewConfig& config, QWidget* parent /*= nullptr*/)
    : ElementListView(listConfigOf(config), parent)
{
}
