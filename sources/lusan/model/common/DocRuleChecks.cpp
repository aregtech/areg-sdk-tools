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
 *  \file        lusan/model/common/DocRuleChecks.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the validation rules every document kind shares.
 *
 ************************************************************************/

#include "lusan/model/common/DocRuleChecks.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/model/common/LiteralValidator.hpp"

#include <QRegularExpression>
#include <QStringList>

namespace
{
    //!< The separators of a templated declared type, such as `NEMap<String, Record>`.
    const QRegularExpression& typeFragmentSeparator(void)
    {
        static const QRegularExpression _separator{ QStringLiteral("[<>,]") };
        return _separator;
    }
}

DocRuleChecks::DocRuleChecks(QList<DocIssue>& issues, const DataTypeDataSection& types, const RuleIds& rules)
    : mIssues   (issues)
    , mTypes    (types)
    , mRules    (rules)
{
}

bool DocRuleChecks::isIdentifier(const QString& name)
{
    return NELusanCommon::isValidIdentifier(name);
}

QString DocRuleChecks::explainShape(eShape shape)
{
    switch (shape)
    {
    case eShape::MissingName:
        return tr("The generated code is named after this. It cannot be left empty.");

    case eShape::InvalidIdentifier:
        return tr("Names must be usable in generated code: a letter or underscore first, then letters, digits or underscores, and no more than %1 characters.")
                    .arg(NELusanCommon::MAX_IDENTIFIER_LENGTH);

    case eShape::DuplicateName:
        return tr("Two declarations of the same kind reach one generated name this way, and the build then refuses whichever comes second. Names are unique per kind, so declarations of different kinds may share one.");

    case eShape::UnresolvedType:
        return tr("The declared type is not in the data type registry. Check the spelling, or declare the type on the Data Types page.");

    case eShape::BadLiteral:
        return tr("The value cannot be read as a value of the declared type.");

    case eShape::Unreferenced:
        return tr("Nothing in the document uses this declaration. Keep it if you are about to, or remove it.");

    default:
        return QString();
    }
}

QString DocRuleChecks::literalReason(const DataTypeDataSection& types, const QString& typeName, const QString& literal)
{
    // An absent value is never a syntax fault: a value is optional, and absence is not a spelling.
    if (literal.isEmpty() || typeName.isEmpty())
        return QString();

    DataTypeCustom* custom = types.findCustomDataType(typeName);
    if (custom == nullptr)
        return LiteralValidator::validate(typeName, literal);

    switch (custom->getCategory())
    {
    case DataTypeBase::eCategory::Enumeration:
        return (static_cast<DataTypeEnum*>(custom)->findElement(literal) != nullptr)
                    ? QString()
                    : tr("'%1' is not an enumerator of '%2'").arg(literal, typeName);

    case DataTypeBase::eCategory::Structure:
    case DataTypeBase::eCategory::Container:
        return tr("'%1' has no literal form").arg(typeName);

    default:
        // Imported: the type is defined elsewhere and opaque here, so any literal is accepted.
        return QString();
    }
}

bool DocRuleChecks::typeResolves(const QString& fragment) const
{
    // Anything that is not a plain name is left alone: this is a registry lookup, not a parser.
    if (isIdentifier(fragment) == false)
        return true;
    if (DataTypeFactory::fromString(fragment) != DataTypeBase::eCategory::Undefined)
        return true;

    return (mTypes.findCustomDataType(fragment) != nullptr);
}

QString DocRuleChecks::unresolvedFragment(const QString& typeName) const
{
    // Every name in a templated type has to exist too. Checking the whole string only would let
    // `Array<Missing>` through.
    const QStringList fragments = typeName.split(typeFragmentSeparator(), Qt::SkipEmptyParts);
    for (const QString& fragment : fragments)
    {
        const QString name = fragment.trimmed();
        if ((name.isEmpty() == false) && (typeResolves(name) == false))
            return name;
    }

    return QString();
}

int DocRuleChecks::ruleId(int rule, DocIssue::eSeverity severity) const
{
    return (severity == DocIssue::eSeverity::Error) ? rule : (DocRuleChecks::ADVISORY_RULE_BASE + rule);
}

void DocRuleChecks::add(uint32_t id, eDocElementKind kind, DocIssue::eSeverity severity, int rule
                       , const QString& message, const QString& detail)
{
    DocIssue issue;
    issue.elementId = id;
    issue.kind      = kind;
    issue.severity  = severity;
    issue.rule      = ruleId(rule, severity);
    issue.message   = message;
    issue.detail    = detail;
    mIssues.append(issue);
}

void DocRuleChecks::checkIdentifier(uint32_t id, eDocElementKind kind, const QString& name, const QString& what)
{
    if (name.isEmpty())
    {
        add(id, kind, DocIssue::eSeverity::Error, mRules.missingName
           , what.isEmpty() ? tr("A declaration has no name") : tr("%1 has no name").arg(what)
           , explainShape(eShape::MissingName));
    }
    else if (isIdentifier(name) == false)
    {
        add(id, kind, DocIssue::eSeverity::Error, mRules.invalidIdentifier
           , tr("'%1' is not a valid identifier").arg(name)
           , explainShape(eShape::InvalidIdentifier));
    }
}

QString DocRuleChecks::checkDeclaredType(uint32_t id, eDocElementKind kind, const QString& typeName
                                        , const QString& what, bool required)
{
    if (typeName.isEmpty())
    {
        if (required)
        {
            add(id, kind, DocIssue::eSeverity::Error, mRules.unresolvedType
               , what.isEmpty() ? tr("A declaration names no type") : tr("%1 declares no type").arg(what)
               , explainShape(eShape::UnresolvedType));
        }

        return QString();
    }

    // Report the fragment, not the whole string: "Foo does not resolve" is actionable,
    // "NEMap<String, Foo> does not resolve" is not.
    const QString missing = unresolvedFragment(typeName);
    if (missing.isEmpty() == false)
    {
        add(id, kind, DocIssue::eSeverity::Error, mRules.unresolvedType
           , what.isEmpty() ? tr("Data type '%1' does not resolve").arg(missing)
                            : tr("%1 declares type '%2', which does not exist").arg(what, missing)
           , explainShape(eShape::UnresolvedType));
    }

    return missing;
}

void DocRuleChecks::checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName
                                , const QString& literal, const QString& what)
{
    const QString reason = literalReason(mTypes, typeName, literal);
    if (reason.isEmpty())
        return;

    add(id, kind, DocIssue::eSeverity::Error, mRules.badLiteral
       , what.isEmpty() ? tr("Invalid %1 literal '%2': %3").arg(typeName, literal, reason)
                        : tr("%1 has value '%2': %3").arg(what, literal, reason)
       , explainShape(eShape::BadLiteral));
}

void DocRuleChecks::reportDuplicate(uint32_t id, eDocElementKind kind, const QString& subject
                                   , DocIssue::eSeverity severity)
{
    add(id, kind, severity, mRules.duplicateName
       , tr("%1 is declared more than once").arg(subject)
       , explainShape(eShape::DuplicateName));
}

void DocRuleChecks::noteUnreferenced(uint32_t id, eDocElementKind kind, const QString& subject
                                    , DocIssue::eSeverity severity, const QString& message)
{
    add(id, kind, severity, mRules.unreferenced
       , message.isEmpty() ? tr("%1 is never referenced").arg(subject) : message
       , explainShape(eShape::Unreferenced));
}

DocNameSet::DocNameSet(DocRuleChecks& checks, eDocElementKind kind, DocIssue::eSeverity severity)
    : mChecks   (checks)
    , mKind     (kind)
    , mSeverity (severity)
    , mTaken    ( )
{
}

bool DocNameSet::claim(uint32_t id, const QString& name, const QString& subject)
{
    return claimKeyed(id, name, subject);
}

bool DocNameSet::claimKeyed(uint32_t id, const QString& key, const QString& subject)
{
    // An unnamed declaration is reported by the identifier rule; counting it here would report
    // every unnamed one after the first as a duplicate of the first.
    if (key.isEmpty())
        return true;

    if (mTaken.contains(key))
    {
        mChecks.reportDuplicate(id, mKind, subject, mSeverity);
        return false;
    }

    mTaken.insert(key);
    return true;
}
