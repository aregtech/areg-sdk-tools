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
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/model/common/DocRules.hpp"
#include "lusan/model/common/LiteralValidator.hpp"

#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

namespace
{
    //!< Every word C++ owns, in one table. The C++17 keywords, the alternative tokens and the
    //!< words C++20 added, because generated code may be compiled as C++20. The code generator
    //!< carries the same list, so the two tools refuse the same names.
    const QSet<QString>& cppKeywords(void)
    {
        static const QSet<QString> _keywords
        {
              QStringLiteral("alignas")     , QStringLiteral("alignof")     , QStringLiteral("and")
            , QStringLiteral("and_eq")      , QStringLiteral("asm")         , QStringLiteral("auto")
            , QStringLiteral("bitand")      , QStringLiteral("bitor")       , QStringLiteral("bool")
            , QStringLiteral("break")       , QStringLiteral("case")        , QStringLiteral("catch")
            , QStringLiteral("char")        , QStringLiteral("char8_t")     , QStringLiteral("char16_t")
            , QStringLiteral("char32_t")    , QStringLiteral("class")       , QStringLiteral("compl")
            , QStringLiteral("concept")     , QStringLiteral("const")       , QStringLiteral("const_cast")
            , QStringLiteral("constexpr")   , QStringLiteral("constinit")   , QStringLiteral("continue")
            , QStringLiteral("co_await")    , QStringLiteral("co_return")   , QStringLiteral("co_yield")
            , QStringLiteral("decltype")    , QStringLiteral("default")     , QStringLiteral("delete")
            , QStringLiteral("do")          , QStringLiteral("double")      , QStringLiteral("dynamic_cast")
            , QStringLiteral("else")        , QStringLiteral("enum")        , QStringLiteral("explicit")
            , QStringLiteral("export")      , QStringLiteral("extern")      , QStringLiteral("false")
            , QStringLiteral("float")       , QStringLiteral("for")         , QStringLiteral("friend")
            , QStringLiteral("goto")        , QStringLiteral("if")          , QStringLiteral("inline")
            , QStringLiteral("int")         , QStringLiteral("long")        , QStringLiteral("mutable")
            , QStringLiteral("namespace")   , QStringLiteral("new")         , QStringLiteral("noexcept")
            , QStringLiteral("not")         , QStringLiteral("not_eq")      , QStringLiteral("nullptr")
            , QStringLiteral("operator")    , QStringLiteral("or")          , QStringLiteral("or_eq")
            , QStringLiteral("private")     , QStringLiteral("protected")   , QStringLiteral("public")
            , QStringLiteral("register")    , QStringLiteral("reinterpret_cast")
            , QStringLiteral("requires")    , QStringLiteral("return")      , QStringLiteral("short")
            , QStringLiteral("signed")      , QStringLiteral("sizeof")      , QStringLiteral("static")
            , QStringLiteral("static_assert"), QStringLiteral("static_cast"), QStringLiteral("struct")
            , QStringLiteral("switch")      , QStringLiteral("template")    , QStringLiteral("this")
            , QStringLiteral("thread_local"), QStringLiteral("throw")       , QStringLiteral("true")
            , QStringLiteral("try")         , QStringLiteral("typedef")     , QStringLiteral("typeid")
            , QStringLiteral("typename")    , QStringLiteral("union")       , QStringLiteral("unsigned")
            , QStringLiteral("using")       , QStringLiteral("virtual")     , QStringLiteral("void")
            , QStringLiteral("volatile")    , QStringLiteral("wchar_t")     , QStringLiteral("while")
            , QStringLiteral("xor")         , QStringLiteral("xor_eq")
        };

        return _keywords;
    }

    //!< The separators of a templated declared type, such as `NEMap<String, Record>`.
    const QRegularExpression& typeFragmentSeparator(void)
    {
        static const QRegularExpression _separator{ QStringLiteral("[<>,]") };
        return _separator;
    }

    //!< True when a type that is not a structure can be a key: one with a hash when hash is
    //!< true, one with an ordering otherwise.
    bool isKeyLeaf(const DataTypeBase& type, bool hash)
    {
        if (type.isPrimitive() || type.isEnumeration())
            return true;

        if (type.isBasicObject() == false)
            return false;

        const QString& name = type.getName();
        return (name == QStringLiteral("String")) || (name == QStringLiteral("WideString"))
            || ((hash == false) && (name == QStringLiteral("DateTime")));
    }

    //!< The type a structure field names. A custom field type of an included structure resolves
    //!< only under that document's namespace, never to a type of the host document.
    const DataTypeBase* fieldType(const DataTypeDataSection& types, const DataTypeStructure& owner, const FieldEntry& field)
    {
        const QString& space = owner.getImportSpace();
        if (space.isEmpty())
            return types.findDataType(field.getType());

        const DataTypeBase* result = types.findDataType(space + QStringLiteral("::") + field.getType());
        if (result == nullptr)
        {
            result = types.findDataType(field.getType());
            result = ((result != nullptr) && result->isCustomDefined()) ? nullptr : result;
        }

        return result;
    }

    //!< True when something stops the type from being a key. The path then names the field chain
    //!< down to the type that stops it, and stays empty when the type itself does.
    bool keyObstacle(const DataTypeDataSection& types, const DataTypeBase* type, bool hash
                    , QList<const DataTypeBase*>& visited, QString& path)
    {
        if (type == nullptr)
            return false;

        if (type->isStructure() == false)
            return (isKeyLeaf(*type, hash) == false);

        if (visited.contains(type))
            return true;

        visited.append(type);
        const DataTypeStructure& structure = static_cast<const DataTypeStructure&>(*type);
        bool result{ false };
        for (const FieldEntry& field : structure.getElements())
        {
            QString inner;
            if (keyObstacle(types, fieldType(types, structure, field), hash, visited, inner))
            {
                path = DocRuleChecks::tr(", whose field '%1' is '%2'").arg(field.getName(), field.getType()) + inner;
                result = true;
                break;
            }
        }

        visited.removeOne(type);
        return result;
    }

    //!< True for the exponent letter of a decimal or a hexadecimal number.
    bool opensExponent(QChar symbol)
    {
        return (symbol == QLatin1Char('e')) || (symbol == QLatin1Char('E'))
            || (symbol == QLatin1Char('p')) || (symbol == QLatin1Char('P'));
    }

    //!< True when the value is spelled as one number: a digit, or a sign before a digit, and then
    //!< only what a single number carries. An operator, a bracket or a space makes it an expression.
    bool writtenAsNumber(const QString& text)
    {
        const qsizetype start = ((text.front() == QLatin1Char('-')) || (text.front() == QLatin1Char('+'))) ? 1 : 0;
        if ((start == text.size()) || (text.at(start).isDigit() == false))
            return false;

        for (qsizetype i = start + 1; i < text.size(); ++ i)
        {
            const QChar symbol = text.at(i);
            const bool signsExponent = ((symbol == QLatin1Char('-')) || (symbol == QLatin1Char('+')))
                                     && opensExponent(text.at(i - 1));
            if ((symbol.isLetterOrNumber() == false) && (symbol != QLatin1Char('.'))
             && (symbol != QLatin1Char('\'')) && (signsExponent == false))
                return false;
        }

        return true;
    }
}

DocRuleChecks::DocRuleChecks(QList<DocIssue>& issues, const DataTypeDataSection& types)
    : mIssues   (issues)
    , mTypes    (types)
{
}

bool DocRuleChecks::isIdentifier(const QString& name)
{
    return NELusanCommon::isValidIdentifier(name);
}

bool DocRuleChecks::isKeyword(const QString& name)
{
    return cppKeywords().contains(name);
}

QString DocRuleChecks::toSnakeCase(const QString& name)
{
    // The conversion the code generator applies, step for step. A name that already carries an
    // underscore is taken as written and only lower-cased, which is why `my_Value` and
    // `my_value` reach one function.
    if (name.isEmpty() || name.contains(QLatin1Char('_')))
        return name.toLower();

    static const QRegularExpression _acronym{ QStringLiteral("([A-Z]+)([A-Z][a-z])") };
    static const QRegularExpression _word   { QStringLiteral("([a-z0-9])([A-Z])") };

    QString result{ name };
    result.replace(_acronym, QStringLiteral("\\1_\\2"));
    result.replace(_word   , QStringLiteral("\\1_\\2"));
    return result.toLower();
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

    case eShape::KeywordName:
        return tr("The generated code spells this name the way the document writes it, so a word C++ owns lands where a declaration has to stand and the file does not compile. Rename the declaration.");

    case eShape::KeywordAccessor:
        return tr("An attribute is reached through functions named after it, and the name is converted rather than copied. Rename the attribute so the converted name is not a word C++ owns.");

    case eShape::DuplicateAccessor:
        return tr("An attribute name is converted rather than copied, so two spellings can reach one function the generated class then declares twice. Rename the later attribute until the two converted names differ.");

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
                  "The block is kept while the document is open and is dropped when it is saved.");

    case eShape::RetiredElement:
        return tr("The format used to define this element here and no longer does, so the document reads as one written for an earlier version. "
                  "The block is kept while the document is open and is dropped when it is saved.");

    case eShape::UnknownAttribute:
        return tr("Nothing reads the attribute, so it reaches no generated code. It is dropped the next time the document is saved.");

    case eShape::DroppedElement:
        return tr("The block is kept only while the document is open. Take what you need out of it before saving, or open the document in a build that defines the element.");

    case eShape::ContainerKey:
        return tr("A HashMap needs a key with a hash and a Map needs a key with an ordering. A primitive, an enumeration, String and WideString have both, DateTime has only an ordering, BinaryBuffer and a container have neither, and a structure has what all of its fields have. Change the key, or the field that stops it.");

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
        return (static_cast<const DataTypeEnum*>(custom)->enumeratorOf(literal).isEmpty() == false)
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

QString DocRuleChecks::declaredValueReason(const DataTypeDataSection& types, const QString& typeName, const QString& value)
{
    const QString text = value.trimmed();
    if (text.isEmpty() || typeName.isEmpty())
        return QString();

    const DataTypeCustom* custom = types.findCustomDataType(typeName);
    if ((custom != nullptr) && (custom->getCategory() == DataTypeBase::eCategory::Enumeration))
    {
        // A cast, a call or any other expression is generated as written and is not judged.
        for (const QString& part : text.split(QStringLiteral("::")))
        {
            if (isIdentifier(part) == false)
                return QString();
        }
    }
    else
    {
        // A name or an expression may be a constant an included header declares.
        const QChar first = text.front();
        const bool literal = (first == QLatin1Char('\'')) || (first == QLatin1Char('"'))
                          || (text == QStringLiteral("true")) || (text == QStringLiteral("false"))
                          || writtenAsNumber(text);
        if (literal == false)
            return QString();
    }

    return literalReason(types, typeName, text);
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
    switch (severity)
    {
    case DocIssue::eSeverity::Warning:
        return (DocRuleChecks::WARNING_RULE_BASE + rule);

    case DocIssue::eSeverity::Info:
        return (DocRuleChecks::INFORMATION_RULE_BASE + rule);

    default:
        return rule;
    }
}

int DocRuleChecks::bareRule(int ruleId)
{
    if (ruleId >= DocRuleChecks::INFORMATION_RULE_BASE)
        return (ruleId - DocRuleChecks::INFORMATION_RULE_BASE);
    else if (ruleId >= DocRuleChecks::WARNING_RULE_BASE)
        return (ruleId - DocRuleChecks::WARNING_RULE_BASE);
    else
        return ruleId;
}

bool DocRuleChecks::isBanded(int ruleId)
{
    return (ruleId >= DocRuleChecks::LOWEST_BANDED_RULE);
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
    checkIdentifierShape(id, kind, name, what);

    if (isKeyword(name))
    {
        add(id, kind, DocIssue::eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
           , tr("'%1' is a C++ keyword, and the generated code spells this name as written").arg(name)
           , explainShape(eShape::KeywordName));
    }
}

void DocRuleChecks::checkIdentifierShape(uint32_t id, eDocElementKind kind, const QString& name, const QString& what)
{
    if (name.isEmpty())
    {
        add(id, kind, DocIssue::eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
           , what.isEmpty() ? tr("A declaration has no name") : tr("%1 has no name").arg(what)
           , explainShape(eShape::MissingName));
    }
    else if (isIdentifier(name) == false)
    {
        add(id, kind, DocIssue::eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
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
            add(id, kind, DocIssue::eSeverity::Error, DocRules::RULE_UNRESOLVED_TYPE
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
        add(id, kind, DocIssue::eSeverity::Error, DocRules::RULE_UNRESOLVED_TYPE
           , what.isEmpty() ? tr("Data type '%1' does not resolve").arg(missing)
                            : tr("%1 declares type '%2', which does not exist").arg(what, missing)
           , explainShape(eShape::UnresolvedType));
    }

    return missing;
}

void DocRuleChecks::checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName
                                , const QString& literal, const QString& what)
{
    const QString reason = declaredValueReason(mTypes, typeName, literal);
    if (reason.isEmpty())
        return;

    add(id, kind, DocIssue::eSeverity::Warning, DocRules::RULE_BAD_LITERAL
       , what.isEmpty() ? tr("Invalid %1 literal '%2': %3").arg(typeName, literal, reason)
                        : tr("%1 has value '%2': %3").arg(what, literal, reason)
       , explainShape(eShape::BadLiteral));
}

void DocRuleChecks::reportDuplicate(uint32_t id, eDocElementKind kind, const QString& subject
                                   , DocIssue::eSeverity severity)
{
    add(id, kind, severity, DocRules::RULE_DUPLICATE_NAME
       , tr("%1 is declared more than once").arg(subject)
       , explainShape(eShape::DuplicateName));
}

void DocRuleChecks::noteUnreferenced(uint32_t id, eDocElementKind kind, const QString& subject
                                    , DocIssue::eSeverity severity, const QString& message)
{
    add(id, kind, severity, DocRules::RULE_UNREFERENCED
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
            add(entry.getId(), kind, DocIssue::eSeverity::Error, DocRules::RULE_DUPLICATE_ENUM_VALUE
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

void DocRuleChecks::checkContainerKey(uint32_t id, eDocElementKind kind, const DataTypeContainer& container)
{
    const bool hash{ container.getContainer() == QStringLiteral("HashMap") };
    if ((hash == false) && (container.getContainer() != QStringLiteral("Map")))
        return;

    const DataTypeBase* keyType = mTypes.findDataType(container.getKey());
    if ((keyType == nullptr) || keyType->isImported())
        return;

    QList<const DataTypeBase*> visited;
    QString path;
    if (keyObstacle(mTypes, keyType, hash, visited, path) == false)
        return;

    add(id, kind, DocIssue::eSeverity::Error, DocRules::RULE_CONTAINER_KEY
       , hash ? tr("Container '%1' cannot hash its key '%2'%3; a HashMap finds a key by its hash").arg(container.getName(), container.getKey(), path)
              : tr("Container '%1' cannot order its key '%2'%3; a Map keeps its keys ordered").arg(container.getName(), container.getKey(), path)
       , explainShape(eShape::ContainerKey));
}

void DocRuleChecks::noteDeprecated(uint32_t id, eDocElementKind kind, const QString& subject
                                  , DocIssue::eSeverity severity, const QString& hint)
{
    add(id, kind, severity, DocRules::RULE_DEPRECATED
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

const DocRules::Retired* DocRuleChecks::retiredElement(const QString& tag, const QString& parent, const QString& document)
{
    for (const DocRules::Retired& entry : DocRules::RETIRED)
    {
        if (tag.compare(QLatin1String(entry.tag), Qt::CaseInsensitive) != 0)
            continue;

        const QLatin1String owner{ entry.parent };
        if ((owner.size() != 0) && (parent.compare(owner, Qt::CaseInsensitive) != 0))
            continue;

        const QLatin1String formats{ entry.documents };
        if ((formats.size() != 0) && (document.isEmpty() == false)
            && (QString(formats).split(QLatin1Char(' '), Qt::SkipEmptyParts).contains(document, Qt::CaseInsensitive) == false))
        {
            continue;
        }

        return &entry;
    }

    return nullptr;
}

void DocRuleChecks::noteUnknownElements(eDocElementKind kind, int rule, const QList<DocUnknownElement>& unknown
                                       , const QString& document)
{
    for (const DocUnknownElement& entry : unknown)
    {
        // The block does not reach the file the next time it is written, so the loss is said
        // before it happens, beside the fault itself.
        add(0u, kind, DocIssue::eSeverity::Warning, DocRules::RULE_DROPPED_ELEMENT
           , tr("The <%1> block, line %2, is removed when the document is saved").arg(entry.name).arg(entry.line)
           , explainShape(eShape::DroppedElement));
        mIssues.last().location = entry.parent.isEmpty() ? QString() : tr("in <%1>").arg(entry.parent);

        // An element the format used to define here is a different fault from one it never had:
        // the author is moving something rather than correcting a spelling, so the finding says
        // where it went instead of only that the tag is not recognised.
        const DocRules::Retired* retired = retiredElement(entry.name, entry.parent, document);
        if (retired != nullptr)
        {
            add(0u, kind, DocIssue::eSeverity::Error, DocRules::RULE_RETIRED_ELEMENT
               , tr("'%1' is no longer a child of <%2>, line %3. %4")
                    .arg(entry.name, entry.parent).arg(entry.line).arg(QLatin1String(retired->fix))
               , explainShape(eShape::RetiredElement));
            mIssues.last().location = entry.parent.isEmpty() ? QString() : tr("in <%1>").arg(entry.parent);
            continue;
        }

        // The tag is the only thing to point at: an element the format does not define has no
        // document element behind it, and so nothing to select. The line is what lets the author
        // find the first one, which matters because a mistyped tag travels by copy.
        const QString message = tr("Unknown tag '%1', line %2").arg(entry.name).arg(entry.line);
        add(0u, kind, DocIssue::eSeverity::Error, rule, message, explainShape(eShape::UnknownElement));
        mIssues.last().location = entry.parent.isEmpty() ? QString() : tr("in <%1>").arg(entry.parent);
    }
}

void DocRuleChecks::noteUnknownAttributes(eDocElementKind kind, int rule, const QList<DocUnknownAttribute>& unknown)
{
    for (const DocUnknownAttribute& entry : unknown)
    {
        add(0u, kind, DocIssue::eSeverity::Warning, rule
           , tr("Unknown attribute '%1' on <%2>. The value is not used and is removed when the document is saved.")
                .arg(entry.name, entry.element)
           , explainShape(eShape::UnknownAttribute));
        mIssues.last().location = tr("in <%1>").arg(entry.element);
    }
}

void DocRuleChecks::checkImportedDocuments(eDocElementKind kind, int rule)
{
    for (const DataTypeDataSection::ImportedTypes& group : mTypes.getImports())
    {
        switch (group.state)
        {
        case DataTypeDataSection::eImportState::NotFound:
        {
            QString detail{ explainShape(eShape::BrokenImport) };
            if (group.triedPaths.isEmpty() == false)
            {
                detail += QLatin1Char('\n') + tr("Looked for it at:") + QLatin1Char('\n')
                        + group.triedPaths.join(QLatin1Char('\n'));
            }

            add(group.id, kind, DocIssue::eSeverity::Error, rule
               , tr("The data type document '%1' is not there").arg(group.location)
               , detail);
            break;
        }

        case DataTypeDataSection::eImportState::ParseFailed:
            add(group.id, kind, DocIssue::eSeverity::Error, rule
               , tr("'%1' does not read as a data type document").arg(group.location)
               , explainShape(eShape::BrokenImport));
            break;

        case DataTypeDataSection::eImportState::DuplicateSpace:
            add(group.id, kind, DocIssue::eSeverity::Error, DocRules::RULE_DUPLICATE_NAME
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

DocAccessorSet::DocAccessorSet(DocRuleChecks& checks, eDocElementKind kind)
    : mChecks   (checks)
    , mKind     (kind)
    , mTaken    ( )
{
}

bool DocAccessorSet::claim(uint32_t id, const QString& name)
{
    // A name that is not an identifier at all is reported where the attribute name itself is
    // judged; converting it would name a function nothing generates.
    if (DocRuleChecks::isIdentifier(name) == false)
        return true;

    const QString accessor = DocRuleChecks::toSnakeCase(name);
    bool result = true;

    if (DocRuleChecks::isKeyword(accessor))
    {
        mChecks.add(id, mKind, DocIssue::eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
                   , DocRuleChecks::tr("Attribute '%1' generates the accessor %2(), and '%2' is a C++ keyword").arg(name, accessor)
                   , DocRuleChecks::explainShape(DocRuleChecks::eShape::KeywordAccessor));
        result = false;
    }

    // Two attributes spelled the same are an ordinary duplicate and are reported as one; only
    // two different spellings meeting in the accessor belong here.
    const auto found = mTaken.constFind(accessor);
    if ((found != mTaken.constEnd()) && (found.value() != name))
    {
        mChecks.add(id, mKind, DocIssue::eSeverity::Error, DocRules::RULE_DUPLICATE_NAME
                   , DocRuleChecks::tr("Attribute '%1' and attribute '%2' both generate the accessor %3()").arg(name, found.value(), accessor)
                   , DocRuleChecks::explainShape(DocRuleChecks::eShape::DuplicateAccessor));
        result = false;
    }
    else
    {
        mTaken.insert(accessor, name);
    }

    return result;
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
