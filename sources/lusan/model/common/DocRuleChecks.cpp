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

#include <QFileInfo>
#include <QHash>
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

    case eShape::DuplicateEnumValue:
        return tr("Two enumerators counting the same cannot be told apart once a value is read back. Give one of them a value of its own. An enumerator with no value written counts on from the one before it.");

    case eShape::Deprecated:
        return tr("The author marked this deprecated. What still uses it keeps working, but it is meant to go.");

    case eShape::BrokenImport:
        return tr("A data type document contributes its types under its own file name, so a row that leads nowhere leaves every '<name>::<type>' in this document unresolved. Point the row at the file, or remove it.");

    case eShape::UnusedImport:
        return tr("Including a data type document pulls its generated header in. Take a type from it, writing '<name>::<type>', or drop the row.");

    case eShape::FileNameMismatch:
        return tr("The generated header and source are named after the declared name, not after the file, so the two may differ. Rename one of them if you expected them to match.");

    case eShape::UnknownElement:
        return tr("The code generator refuses the whole document, so nothing generates until the tag is removed or corrected. "
                  "The block is kept as written until then.");

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
    // A qualified name whose first part is a data type document this one includes has to name a
    // type that document declares
    const qsizetype scope = fragment.indexOf(QStringLiteral("::"));
    if (scope > 0)
    {
        return (mTypes.hasImportSpace(fragment.left(scope)) == false)
            || (mTypes.findCustomDataType(fragment) != nullptr);
    }

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

void DocRuleChecks::checkEnumeratorValues(eDocElementKind kind, const QString& typeName
                                         , const QList<EnumEntry>& entries)
{
    // The counting follows C++: an enumerator with no value of its own is one past the previous.
    QHash<qint64, QString> taken;
    qint64 next{ 0 };
    bool known{ true };

    for (const EnumEntry& entry : entries)
    {
        qint64 value{ next };
        const QString written = entry.getValue().trimmed();
        if (written.isEmpty() == false)
        {
            bool parsed{ false };
            // Written the way the author wrote it: 0x10 and 16 are one value, and both count.
            value = written.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)
                        ? written.mid(2).toLongLong(&parsed, 16)
                        : written.toLongLong(&parsed, 10);
            known = parsed;
        }

        if (known == false)
            continue;

        const auto found = taken.constFind(value);
        if (found != taken.constEnd())
        {
            add(entry.getId(), kind, DocIssue::eSeverity::Error, mRules.duplicateEnumValue
               , tr("Value '%1' of enumeration '%2' counts %3, the same as '%4'")
                    .arg(entry.getName(), typeName).arg(value).arg(found.value())
               , explainShape(eShape::DuplicateEnumValue));
        }
        else
        {
            taken.insert(value, entry.getName());
        }

        next = value + 1;
    }
}

void DocRuleChecks::noteDeprecated(uint32_t id, eDocElementKind kind, const QString& subject
                                  , DocIssue::eSeverity severity, const QString& hint)
{
    add(id, kind, severity, mRules.deprecated
       , hint.trimmed().isEmpty() ? tr("%1 is deprecated").arg(subject)
                                  : tr("%1 is deprecated: %2").arg(subject, hint.trimmed())
       , explainShape(eShape::Deprecated));
}

void DocRuleChecks::noteFileNameMismatch(uint32_t id, const QString& name, const QString& filePath, int rule)
{
    if (name.isEmpty() || filePath.isEmpty())
        return;

    const QString fromFile{ NELusanCommon::toDocumentName(QFileInfo(filePath).completeBaseName()) };
    if (fromFile.isEmpty() || (fromFile == name))
        return;

    add(id, eDocElementKind::Overview, DocIssue::eSeverity::Info, rule
       , tr("Different document name: the document is called '%1', the file '%2'").arg(name, QFileInfo(filePath).fileName())
       , explainShape(eShape::FileNameMismatch));
}

void DocRuleChecks::noteUnknownElements(DocElementTable::eDocument doc, eDocElementKind kind, int rule
                                      , const QList<DocUnknownElement>& unknown)
{
    for (const DocUnknownElement& entry : unknown)
    {
        // The tag is the only thing to point at: an element the format does not define has no
        // document element behind it, and so nothing to select. The line is what lets the author
        // find the first one, which matters because a mistyped tag travels by copy.
        const QString message = entry.removed
            ? tr("Unknown tag '%1', line %2. The format no longer defines it").arg(entry.name).arg(entry.line)
            : tr("Unknown tag '%1', line %2").arg(entry.name).arg(entry.line);

        const DocElementTable::Row* row = DocElementTable::find(doc, entry.name);
        const QString detail = ((row != nullptr) && (row->replacement.isEmpty() == false))
            ? tr("The format no longer defines this element; use %1 instead. "
                 "Nothing generates until the block is removed or replaced. The block is kept as written until then.")
                    .arg(row->replacement)
            : explainShape(eShape::UnknownElement);

        add(0u, kind, DocIssue::eSeverity::Error, rule, message, detail);
        mIssues.last().location = entry.parent.isEmpty() ? QString() : tr("in <%1>").arg(entry.parent);
    }
}

void DocRuleChecks::checkImportedDocuments(eDocElementKind kind, int rule)
{
    for (const DataTypeDataSection::ImportedTypes& group : mTypes.getImports())
    {
        switch (group.state)
        {
        case DataTypeDataSection::eImportState::NotFound:
            add(group.id, kind, DocIssue::eSeverity::Error, rule
               , tr("The data type document '%1' is not there").arg(group.location)
               , explainShape(eShape::BrokenImport));
            break;

        case DataTypeDataSection::eImportState::ParseFailed:
            add(group.id, kind, DocIssue::eSeverity::Error, rule
               , tr("'%1' does not read as a data type document").arg(group.location)
               , explainShape(eShape::BrokenImport));
            break;

        case DataTypeDataSection::eImportState::DuplicateSpace:
            add(group.id, kind, DocIssue::eSeverity::Error, mRules.duplicateName
               , tr("'%1' and an earlier include both carry the name '%2', so both generate one namespace")
                    .arg(group.location, group.space)
               , explainShape(eShape::DuplicateName));
            break;

        default:
            break;
        }
    }
}

void DocRuleChecks::noteUnusedImports(eDocElementKind kind, int rule, const QSet<QString>& typesUsed)
{
    for (const DataTypeDataSection::ImportedTypes& group : mTypes.getImports())
    {
        if (group.isResolved() == false)
            continue;

        const QString prefix = group.space + QStringLiteral("::");
        bool used = false;
        for (const QString& name : typesUsed)
        {
            // Anywhere in the spelling, not only at the front: a container declares its element
            // type inside its own name, as `Array<Shared::Reading>` does.
            if (name.contains(prefix))
            {
                used = true;
                break;
            }
        }

        if (used == false)
        {
            add(group.id, kind, DocIssue::eSeverity::Warning, rule
               , tr("Data types are imported from '%1', but nothing here declares with '%2'")
                    .arg(group.location, prefix)
               , explainShape(eShape::UnusedImport));
        }
    }
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
