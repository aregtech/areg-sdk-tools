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
 *  \file        lusan/data/common/DataTypeCustom.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Custom Data Type.
 *
 ************************************************************************/
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/common/XmlSI.hpp"

DataTypeCustom::DataTypeCustom(ElementBase * parent /*= nullptr*/)
    : DataTypeBase(eCategory::CustomDefined, "", 0, parent)
    , mDescription  ()
    , mDeprecateHint()
    , mIsDeprecated (false)
    , mImportSpace  ()
{
}

DataTypeCustom::DataTypeCustom(uint32_t id, ElementBase* parent)
    : DataTypeBase(eCategory::CustomDefined, "", id, parent)
    , mDescription  ()
    , mDeprecateHint()
    , mIsDeprecated (false)
    , mImportSpace  ()
{
}

DataTypeCustom::DataTypeCustom(DataTypeBase::eCategory category, ElementBase* parent /*= nullptr*/)
    : DataTypeBase(category, "", 0, parent)
    , mDescription  ()
    , mDeprecateHint()
    , mIsDeprecated (false)
    , mImportSpace  ()
{
}

DataTypeCustom::DataTypeCustom(DataTypeBase::eCategory category, uint32_t id, const QString& name, ElementBase* parent /*= nullptr*/)
    : DataTypeBase(category, name, id, parent)
    , mDescription  ()
    , mDeprecateHint()
    , mIsDeprecated (false)
    , mImportSpace  ()
{
}

DataTypeCustom::DataTypeCustom(const DataTypeCustom& src)
    : DataTypeBase  (src)
    , mDescription  (src.mDescription)
    , mDeprecateHint(src.mDeprecateHint)
    , mIsDeprecated (src.mIsDeprecated)
    , mImportSpace  (src.mImportSpace)
{
}

DataTypeCustom::DataTypeCustom(DataTypeCustom&& src) noexcept
    : DataTypeBase  (std::move(src))
    , mDescription  (std::move(src.mDescription))
    , mDeprecateHint(std::move(src.mDeprecateHint))
    , mIsDeprecated (src.mIsDeprecated)
    , mImportSpace  (std::move(src.mImportSpace))
{
}

DataTypeCustom::~DataTypeCustom()
{
}

DataTypeCustom& DataTypeCustom::operator = (const DataTypeCustom& other)
{
    if (this != &other)
    {
        DataTypeBase::operator = (other);
        mDescription    = other.mDescription;
        mDeprecateHint  = other.mDeprecateHint;
        mIsDeprecated   = other.mIsDeprecated;
        mImportSpace    = other.mImportSpace;
    }

    return *this;
}

DataTypeCustom& DataTypeCustom::operator = (DataTypeCustom&& other) noexcept
{
    if (this != &other)
    {
        DataTypeBase::operator = (std::move(other));
        mDescription    = std::move(other.mDescription);
        mDeprecateHint  = std::move(other.mDeprecateHint);
        mIsDeprecated   = other.mIsDeprecated;
        mImportSpace    = std::move(other.mImportSpace);
    }

    return *this;
}

const QString& DataTypeCustom::getDescription() const
{
    return mDescription;
}

void DataTypeCustom::setDescription(const QString& description)
{
    mDescription = description;
}

bool DataTypeCustom::isValid() const
{
    return (getId() != 0) && DataTypeBase::isValid();
}

bool DataTypeCustom::hasTypeName(const QString& typeName) const
{
    if (mImportSpace.isEmpty())
    {
        return DataTypeBase::hasTypeName(typeName);
    }

    // Compared in place rather than by building `Space::Name`: every lookup of every declared
    // type walks this, and a temporary string per candidate is a cost the editor would feel.
    const qsizetype split = mImportSpace.size();
    return (typeName.size() == (split + 2 + mName.size()))
        && (typeName.at(split) == QLatin1Char(':'))
        && (typeName.at(split + 1) == QLatin1Char(':'))
        && typeName.startsWith(mImportSpace)
        && typeName.endsWith(mName);
}

void DataTypeCustom::setImportSpace(const QString& space)
{
    mImportSpace = space;
}

bool DataTypeCustom::isDocumentImport() const
{
    return (mImportSpace.isEmpty() == false);
}

QString DataTypeCustom::getQualifiedName() const
{
    return (mImportSpace.isEmpty() ? mName : (mImportSpace + QStringLiteral("::") + mName));
}

bool DataTypeCustom::getIsDeprecated() const
{
    return mIsDeprecated;
}

void DataTypeCustom::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
}

const QString& DataTypeCustom::getDeprecateHint() const
{
    return mIsDeprecated ? mDeprecateHint : ElementBase::EmptyString;
}

void DataTypeCustom::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = mIsDeprecated ? hint : QString();
}

void DataTypeCustom::setIsDeprecated(bool isDeprecated, const QString& reason)
{
    mIsDeprecated = isDeprecated;
    mDeprecateHint = reason;
}

QString DataTypeCustom::getType() const
{
    return DataTypeCustom::getType(mCategory);
}

QString DataTypeCustom::getType(DataTypeBase::eCategory category)
{
    switch (category)
    {
    case DataTypeBase::eCategory::Enumeration:
        return XmlSI::xmlSIValueEnumeration;
    case DataTypeBase::eCategory::Structure:
        return XmlSI::xmlSIValueStructure;
    case DataTypeBase::eCategory::Imported:
        return XmlSI::xmlSIValueImported;
    case DataTypeBase::eCategory::Container:
        return XmlSI::xmlSIValueContainer;
    default:
        return "";
    }
}

DataTypeBase::eCategory DataTypeCustom::fromTypeString(const QString& type)
{
    if (type == XmlSI::xmlSIValueEnumeration)
    {
        return DataTypeBase::eCategory::Enumeration;
    }
    else if (type == XmlSI::xmlSIValueStructure)
    {
        return DataTypeBase::eCategory::Structure;
    }
    else if (type == XmlSI::xmlSIValueImported)
    {
        return DataTypeBase::eCategory::Imported;
    }
    else if (type == XmlSI::xmlSIValueContainer)
    {
        return DataTypeBase::eCategory::Container;
    }
    else if (type.compare(QLatin1StringView("Enumerate"), Qt::CaseInsensitive) == 0)
    {
        // The two names the first published service interface format used.
        return DataTypeBase::eCategory::Enumeration;
    }
    else if (type.compare(QLatin1StringView("DefinedType"), Qt::CaseInsensitive) == 0)
    {
        return DataTypeBase::eCategory::Container;
    }
    else
    {
        // An unknown `Type` comes from a hand-edited or newer document, never from a programming
        // error: the readers skip the element and validation reports it.
        return DataTypeBase::eCategory::CustomDefined;
    }
}

