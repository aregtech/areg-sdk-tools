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
 *  \file        lusan/model/si/SIOverviewModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview Model.
 *
 ************************************************************************/

#include "lusan/model/si/SIOverviewModel.hpp"

SIOverviewModel::SIOverviewModel(IEDocumentModel& document)
    : OverviewModel(document)
{
}

SIOverviewData::eCategory SIOverviewModel::getCategory(void) const
{
    return static_cast<const SIOverviewData&>(section()).getCategory();
}

void SIOverviewModel::setCategory(SIOverviewData::eCategory category)
{
    if (category == getCategory())
        return;

    IEDocumentModel* document = &getDocument();
    pushProperty<SIOverviewData::eCategory>
    (
          [document]() { return static_cast<SIOverviewData&>(document->getOverviewSection()).getCategory(); }
        , [document](const SIOverviewData::eCategory& value) { static_cast<SIOverviewData&>(document->getOverviewSection()).setCategory(value); }
        , category, QObject::tr("Set service category")
    );
}
