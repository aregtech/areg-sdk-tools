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
 *  \file        lusan/model/common/OverviewModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Overview page model shared by every document editor.
 *
 ************************************************************************/

#include "lusan/model/common/OverviewModel.hpp"

namespace
{
    //!< The deprecation mark and its hint, moved as one value so one gesture is one undo step.
    struct DeprecationState
    {
        bool    flag { false };
        QString hint { };
    };
}

OverviewModel::OverviewModel(IEDocumentModel& document)
    : mDocument (document)
{
}

const QString& OverviewModel::getName(void) const
{
    return section().getName();
}

const VersionNumber& OverviewModel::getVersion(void) const
{
    return section().getVersion();
}

const QString& OverviewModel::getDescription(void) const
{
    return section().getDescription();
}

bool OverviewModel::getIsDeprecated(void) const
{
    return section().getIsDeprecated();
}

const QString& OverviewModel::getDeprecateHint(void) const
{
    return section().getDeprecateHint();
}

uint32_t OverviewModel::getOverviewId(void) const
{
    return section().getId();
}

DocModelNotifier& OverviewModel::getNotifier(void) const
{
    return mDocument.getNotifier();
}

void OverviewModel::setName(const QString& name)
{
    if (name == getName())
        return;

    IEDocumentModel* document = &mDocument;
    pushProperty<QString>( [document]() { return document->getOverviewSection().getName(); }
                         , [document](const QString& value) { document->getOverviewSection().setName(value); }
                         , name, QObject::tr("Set name"));
}

void OverviewModel::setVersion(const VersionNumber& version)
{
    if (version == getVersion())
        return;

    IEDocumentModel* document = &mDocument;
    pushProperty<VersionNumber>( [document]() { return document->getOverviewSection().getVersion(); }
                               , [document](const VersionNumber& value) { document->getOverviewSection().setVersion(value); }
                               , version, QObject::tr("Set version"));
}

void OverviewModel::setDescription(const QString& description)
{
    if (description == getDescription())
        return;

    IEDocumentModel* document = &mDocument;
    pushProperty<QString>( [document]() { return document->getOverviewSection().getDescription(); }
                         , [document](const QString& value) { document->getOverviewSection().setDescription(value); }
                         , description, QObject::tr("Set description"));
}

void OverviewModel::setIsDeprecated(bool isDeprecated)
{
    if (isDeprecated == getIsDeprecated())
        return;

    IEDocumentModel* document = &mDocument;
    auto getter = [document]() -> DeprecationState
    {
        const OverviewDataSection& overview = document->getOverviewSection();
        return DeprecationState{ overview.getIsDeprecated(), overview.getDeprecateHint() };
    };
    auto setter = [document](const DeprecationState& value)
    {
        OverviewDataSection& overview = document->getOverviewSection();
        overview.setIsDeprecated(value.flag);
        overview.setDeprecateHint(value.hint);
    };

    const DeprecationState next{ isDeprecated, isDeprecated ? getDeprecateHint() : QString() };
    pushProperty<DeprecationState>(getter, setter, next, QObject::tr("Set deprecated"));
}

void OverviewModel::setDeprecateHint(const QString& hint)
{
    if (hint == getDeprecateHint())
        return;

    IEDocumentModel* document = &mDocument;
    pushProperty<QString>( [document]() { return document->getOverviewSection().getDeprecateHint(); }
                         , [document](const QString& value) { document->getOverviewSection().setDeprecateHint(value); }
                         , hint, QObject::tr("Set deprecation hint"));
}
