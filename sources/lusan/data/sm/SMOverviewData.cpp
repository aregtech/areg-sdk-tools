/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMOverviewData.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview data (name, user version, threading mode).
 *
 ************************************************************************/

#include "lusan/data/sm/SMOverviewData.hpp"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

SMOverviewData::eThreading SMOverviewData::fromThreadingString(const QString& threading)
{
    return (threading.compare(SMOverviewData::STR_THREADING_LOCAL, Qt::CaseInsensitive) == 0)
                ? eThreading::Local
                : eThreading::Shared;
}

const char* SMOverviewData::toString(SMOverviewData::eThreading threading)
{
    switch (threading)
    {
    case eThreading::Local:
        return SMOverviewData::STR_THREADING_LOCAL;
    case eThreading::Shared:
    default:
        return SMOverviewData::STR_THREADING_SHARED;
    }
}

SMOverviewData::SMOverviewData(ElementBase* parent /*= nullptr*/)
    : DocumentElem  (parent)
    , mName         ( )
    , mVersion      (1, 0, 0)
    , mThreading    (eThreading::Shared)
    , mDescription  ( )
{
}

SMOverviewData::SMOverviewData(uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : DocumentElem  (id, parent)
    , mName         (name)
    , mVersion      (1, 0, 0)
    , mThreading    (eThreading::Shared)
    , mDescription  ( )
{
}

bool SMOverviewData::isValid(void) const
{
    return (mName.isEmpty() == false);
}

bool SMOverviewData::readFromXml(QXmlStreamReader& xml)
{
    xml.skipCurrentElement();
    return true;
}

void SMOverviewData::writeToXml(QXmlStreamWriter& xml) const
{
    Q_UNUSED(xml);
}
