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
 *  \file        lusan/model/sm/SMOverviewModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page model.
 *
 ************************************************************************/

#include "lusan/model/sm/SMOverviewModel.hpp"

SMOverviewModel::SMOverviewModel(IEDocumentModel& document)
    : OverviewModel(document)
{
}

SMOverviewData::eThreading SMOverviewModel::getThreading(void) const
{
    return static_cast<const SMOverviewData&>(section()).getThreading();
}

void SMOverviewModel::setThreading(SMOverviewData::eThreading threading)
{
    if (threading == getThreading())
        return;

    IEDocumentModel* document = &getDocument();
    pushProperty<SMOverviewData::eThreading>
    (
          [document]() { return static_cast<SMOverviewData&>(document->getOverviewSection()).getThreading(); }
        , [document](const SMOverviewData::eThreading& value) { static_cast<SMOverviewData&>(document->getOverviewSection()).setThreading(value); }
        , threading, QObject::tr("Set threading mode")
    );
}
