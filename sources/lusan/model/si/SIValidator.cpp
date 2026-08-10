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
 *  \file        lusan/model/si/SIValidator.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, service interface structural and reference validation engine.
 *
 ************************************************************************/

#include "lusan/model/si/SIValidator.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeFactory.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/data/common/MethodDataSection.hpp"
#include "lusan/model/common/LiteralValidator.hpp"

#include <QCoreApplication>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

namespace
{
    using eSeverity = DocIssue::eSeverity;

    //!< Every user-facing string of this engine lives under one translation context.
    inline QString vtr(const char* text)
    {
        return QCoreApplication::translate("SIValidator", text);
    }

    //!< A short label for a method kind, used where a message has to tell two entries apart.
    QString methodKindWord(const MethodEntry& method)
    {
        const QString& label = method.kind().label;
        return label.isEmpty() ? vtr("Method") : label;
    }

    /**
     * \class   Ctx
     * \brief   One validation run: the document, the findings it produced, and the registry
     *          lookups the rules share.
     **/
    class Ctx
    {
    public:
        explicit Ctx(const ServiceInterfaceData& data)
            : mData     (data)
            , mIssues   ( )
            , mTypesUsed( )
            , mConstsUsed( )
        {
        }

        QList<DocIssue> run();

    private:
        void add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message);

        //!< True when the name is a primitive, a basic object/container or a declared custom type.
        bool typeResolves(const QString& typeName) const;

        //!< The first fragment of a declared type that resolves to nothing, or an empty string.
        QString unresolvedFragment(const QString& typeName) const;

        void checkName(uint32_t id, eDocElementKind kind, const QString& name, const QString& what);
        void checkType(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& what);
        void checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& literal, const QString& what);
        void checkParameters(const MethodEntry& method);

        void checkOverview();
        void checkDataTypes();
        void checkAttributes();
        void checkMethods();
        void checkConstants();
        void checkIncludes();
        void checkUnreferenced();

        //!< Records that a declared type name is in use, element types of a template included.
        void noteType(const QString& typeName);

    private:
        const ServiceInterfaceData& mData;
        QList<DocIssue>             mIssues;
        QSet<QString>               mTypesUsed;     //!< Type names something in the document declares with.
        QSet<QString>               mConstsUsed;    //!< Constant names something in the document reads.
        bool                        mUnresolvedSeen { false };  //!< A declared type answered to nothing here.
    };

    void Ctx::add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message)
    {
        DocIssue issue;
        issue.elementId = id;
        issue.kind      = kind;
        issue.severity  = sev;
        issue.rule      = (sev == eSeverity::Error) ? rule : (SIValidator::ADVISORY_RULE_BASE + rule);
        issue.message   = message;
        issue.detail    = SIValidator::explainRule(issue.rule, sev);
        mIssues.append(issue);
    }

    bool Ctx::typeResolves(const QString& typeName) const
    {
        if (DataTypeFactory::fromString(typeName) != DataTypeBase::eCategory::Undefined)
            return true;

        return (mData.getDataTypeData().findDataType(typeName) != nullptr);
    }

    QString Ctx::unresolvedFragment(const QString& typeName) const
    {
        // A declared type may be written as a template, and every name in it has to exist. Checking
        // the whole string only would let `Array<Missing>` through.
        const QStringList fragments = typeName.split(QRegularExpression(QStringLiteral("[<>,]")), Qt::SkipEmptyParts);
        for (const QString& fragment : fragments)
        {
            const QString name = fragment.trimmed();
            // Anything that is not a plain name is left alone: this is a registry lookup, not a
            // C++ parser.
            if (name.isEmpty() || (NELusanCommon::isValidIdentifier(name) == false))
                continue;
            if (typeResolves(name) == false)
                return name;
        }

        return QString();
    }

    void Ctx::noteType(const QString& typeName)
    {
        const QStringList fragments = typeName.split(QRegularExpression(QStringLiteral("[<>,]")), Qt::SkipEmptyParts);
        for (const QString& fragment : fragments)
        {
            const QString name = fragment.trimmed();
            if (name.isEmpty() == false)
            {
                mTypesUsed.insert(name);
            }
        }
    }

    void Ctx::checkName(uint32_t id, eDocElementKind kind, const QString& name, const QString& what)
    {
        if (name.isEmpty())
        {
            add(id, kind, eSeverity::Error, SIValidator::RULE_MISSING_NAME, vtr("%1 has no name").arg(what));
        }
        else if (NELusanCommon::isValidIdentifier(name) == false)
        {
            add(id, kind, eSeverity::Error, SIValidator::RULE_INVALID_IDENTIFIER, vtr("'%1' is not a valid identifier").arg(name));
        }
    }

    void Ctx::checkType(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& what)
    {
        if (typeName.isEmpty())
        {
            add(id, kind, eSeverity::Error, SIValidator::RULE_UNRESOLVED_TYPE, vtr("%1 declares no type").arg(what));
            return;
        }

        noteType(typeName);
        const QString missing = unresolvedFragment(typeName);
        if (missing.isEmpty() == false)
        {
            mUnresolvedSeen = true;
            add(id, kind, eSeverity::Error, SIValidator::RULE_UNRESOLVED_TYPE, vtr("%1 declares type '%2', which does not exist").arg(what, missing));
        }
    }

    void Ctx::checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& literal, const QString& what)
    {
        // A declared type carries its own literal form, which this check does not read.
        if (literal.isEmpty() || (mData.getDataTypeData().findCustomDataType(typeName) != nullptr))
            return;

        const QString reason = LiteralValidator::validate(typeName, literal);
        if (reason.isEmpty() == false)
        {
            add(id, kind, eSeverity::Error, SIValidator::RULE_BAD_LITERAL, vtr("%1 has value '%2': %3").arg(what, literal, reason));
        }
    }

    void Ctx::checkParameters(const MethodEntry& method)
    {
        const QString kindWord = methodKindWord(method);
        QSet<QString> names;
        bool defaulted = false;
        for (const MethodParameter& param : method.getElements())
        {
            const QString where = vtr("Parameter '%1' of %2 '%3'").arg(param.getName(), kindWord.toLower(), method.getName());
            checkName(method.getId(), eDocElementKind::Method, param.getName(), where);
            checkType(method.getId(), eDocElementKind::Method, param.getType(), where);
            checkLiteral(method.getId(), eDocElementKind::Method, param.getType(), param.getValue(), where);

            if (names.contains(param.getName()))
            {
                add(method.getId(), eDocElementKind::Method, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                   , vtr("%1 '%2' declares parameter '%3' twice").arg(kindWord, method.getName(), param.getName()));
            }

            names.insert(param.getName());

            // A caller may only leave out the trailing parameters, so once one carries a default
            // every parameter after it has to carry one too.
            if (param.hasDefault())
            {
                defaulted = true;
            }
            else if (defaulted)
            {
                add(method.getId(), eDocElementKind::Method, eSeverity::Error, SIValidator::RULE_DEFAULT_ORDER
                   , vtr("Parameter '%1' of %2 '%3' has no default value, but an earlier parameter has one").arg(param.getName(), kindWord.toLower(), method.getName()));
            }
        }
    }

    void Ctx::checkOverview()
    {
        const SIOverviewData& overview = mData.getOverviewData();
        checkName(overview.getId(), eDocElementKind::Overview, overview.getName(), vtr("The service interface"));

        if (overview.getVersion().isValid() == false)
        {
            add(overview.getId(), eDocElementKind::Overview, eSeverity::Error, SIValidator::RULE_MISSING_NAME
               , vtr("The service interface has no version"));
        }

        // An interface a client cannot do anything with is worth saying out loud, but it is a
        // perfectly good starting point for a document being written.
        if (mData.getAttributeData().getElementCount() == 0 && mData.getMethodData().getElements().isEmpty())
        {
            add(overview.getId(), eDocElementKind::Overview, eSeverity::Info, SIValidator::RULE_EMPTY_INTERFACE
               , vtr("The interface declares no attribute and no method, so it offers nothing to a client"));
        }
    }

    void Ctx::checkDataTypes()
    {
        QSet<QString> names;
        for (DataTypeCustom* dataType : mData.getDataTypeData().getCustomDataTypes())
        {
            if (dataType == nullptr)
                continue;

            const uint32_t id = dataType->getId();
            const QString name = dataType->getName();
            checkName(id, eDocElementKind::DataType, name, vtr("The data type"));
            if (names.contains(name))
            {
                add(id, eDocElementKind::DataType, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                   , vtr("Data type '%1' is declared more than once").arg(name));
            }

            names.insert(name);

            if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
            {
                DataTypeStructure* structType = static_cast<DataTypeStructure*>(dataType);
                QSet<QString> fields;
                for (const FieldEntry& field : structType->getElements())
                {
                    const QString where = vtr("Field '%1' of structure '%2'").arg(field.getName(), name);
                    checkName(id, eDocElementKind::DataType, field.getName(), where);
                    checkType(id, eDocElementKind::DataType, field.getType(), where);
                    checkLiteral(id, eDocElementKind::DataType, field.getType(), field.getValue(), where);
                    if (fields.contains(field.getName()))
                    {
                        add(id, eDocElementKind::DataType, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                           , vtr("Structure '%1' declares field '%2' twice").arg(name, field.getName()));
                    }

                    fields.insert(field.getName());
                }

                if (structType->getElementCount() == 0)
                {
                    add(id, eDocElementKind::DataType, eSeverity::Warning, SIValidator::RULE_EMPTY_TYPE
                       , vtr("Structure '%1' has no fields").arg(name));
                }
            }
            else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
            {
                DataTypeEnum* enumType = static_cast<DataTypeEnum*>(dataType);
                QSet<QString> fields;
                for (const EnumEntry& field : enumType->getElements())
                {
                    checkName(id, eDocElementKind::DataType, field.getName(), vtr("Value '%1' of enumeration '%2'").arg(field.getName(), name));
                    if (fields.contains(field.getName()))
                    {
                        add(id, eDocElementKind::DataType, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                           , vtr("Enumeration '%1' declares value '%2' twice").arg(name, field.getName()));
                    }

                    fields.insert(field.getName());
                }

                if (enumType->getElementCount() == 0)
                {
                    add(id, eDocElementKind::DataType, eSeverity::Warning, SIValidator::RULE_EMPTY_TYPE
                       , vtr("Enumeration '%1' has no values").arg(name));
                }

                // The derived type is what the generated enumeration counts in.
                if (enumType->getDerived().isEmpty() == false)
                {
                    checkType(id, eDocElementKind::DataType, enumType->getDerived(), vtr("Enumeration '%1'").arg(name));
                }
            }
            else if (dataType->getCategory() == DataTypeBase::eCategory::Container)
            {
                DataTypeContainer* container = static_cast<DataTypeContainer*>(dataType);
                checkType(id, eDocElementKind::DataType, container->getValue(), vtr("The value of container '%1'").arg(name));
                if (container->canHaveKey())
                {
                    checkType(id, eDocElementKind::DataType, container->getKey(), vtr("The key of container '%1'").arg(name));
                }
            }
        }
    }

    void Ctx::checkAttributes()
    {
        QSet<QString> names;
        for (const AttributeEntry& attribute : mData.getAttributeData().getElements())
        {
            const uint32_t id = attribute.getId();
            const QString name = attribute.getName();
            checkName(id, eDocElementKind::Attribute, name, vtr("The attribute"));
            checkType(id, eDocElementKind::Attribute, attribute.getType(), vtr("Attribute '%1'").arg(name));
            if (names.contains(name))
            {
                add(id, eDocElementKind::Attribute, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                   , vtr("Attribute '%1' is declared more than once").arg(name));
            }

            names.insert(name);
        }
    }

    void Ctx::checkMethods()
    {
        // Names are unique per method kind: a request, a response and a broadcast may share one
        // name, but two requests may not.
        QHash<int, QSet<QString> > names;
        for (MethodEntry* method : mData.getMethodData().getElements())
        {
            if (method == nullptr)
                continue;

            const uint32_t id = method->getId();
            const QString name = method->getName();
            const QString kindWord = methodKindWord(*method);
            checkName(id, eDocElementKind::Method, name, vtr("The %1").arg(kindWord.toLower()));
            checkParameters(*method);

            QSet<QString>& taken = names[static_cast<int>(method->getKind())];
            if (taken.contains(name))
            {
                add(id, eDocElementKind::Method, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                   , vtr("%1 '%2' is declared more than once").arg(kindWord, name));
            }

            taken.insert(name);
        }

        // A request either answers with a declared response or answers with nothing at all; a name
        // that leads nowhere means the caller waits for a reply the interface never sends.
        const QList<MethodEntry*> requests  = mData.getMethodData().methodsOfKind(NEMethod::SiRequest);
        const QList<MethodEntry*> responses = mData.getMethodData().methodsOfKind(NEMethod::SiResponse);
        for (MethodEntry* request : requests)
        {
            if (request == nullptr)
                continue;

            const QString response = request->getReply();
            if ((response.isEmpty() == false) && (mData.getMethodData().findMethod(response, NEMethod::SiResponse) == nullptr))
            {
                add(request->getId(), eDocElementKind::Method, eSeverity::Error, SIValidator::RULE_RESPONSE_LINK
                   , vtr("Request '%1' answers with response '%2', which is not declared").arg(request->getName(), response));
            }
        }

        for (MethodEntry* response : responses)
        {
            if (response == nullptr)
                continue;

            bool bound = false;
            for (MethodEntry* request : requests)
            {
                bound = bound || ((request != nullptr) && (request->getReply() == response->getName()));
            }

            if (bound == false)
            {
                add(response->getId(), eDocElementKind::Method, eSeverity::Warning, SIValidator::RULE_UNBOUND_RESPONSE
                   , vtr("Response '%1' is not the answer to any request").arg(response->getName()));
            }
        }
    }

    void Ctx::checkConstants()
    {
        QSet<QString> names;
        for (const ConstantEntry& constant : mData.getConstantData().getElements())
        {
            const uint32_t id = constant.getId();
            const QString name = constant.getName();
            checkName(id, eDocElementKind::Constant, name, vtr("The constant"));
            checkType(id, eDocElementKind::Constant, constant.getType(), vtr("Constant '%1'").arg(name));
            checkLiteral(id, eDocElementKind::Constant, constant.getType(), constant.getValue(), vtr("Constant '%1'").arg(name));
            if (names.contains(name))
            {
                add(id, eDocElementKind::Constant, eSeverity::Error, SIValidator::RULE_DUPLICATE_NAME
                   , vtr("Constant '%1' is declared more than once").arg(name));
            }

            names.insert(name);
        }
    }

    void Ctx::checkIncludes()
    {
        QSet<QString> locations;
        QList<const IncludeEntry*> dataTypeDocuments;
        for (const IncludeEntry& include : mData.getIncludeData().getElements())
        {
            const QString location = include.getLocation();
            if (location.isEmpty())
            {
                add(include.getId(), eDocElementKind::Include, eSeverity::Error, SIValidator::RULE_MISSING_NAME
                   , vtr("An include row names no file"));
            }
            else if (locations.contains(location))
            {
                add(include.getId(), eDocElementKind::Include, eSeverity::Warning, SIValidator::RULE_DUPLICATE_NAME
                   , vtr("'%1' is included more than once").arg(location));
            }

            // A service interface includes no other service interface, so it has no document
            // extension of its own to classify against.
            if (includeKindOf(location, QString()) == eIncludeKind::DataType)
            {
                dataTypeDocuments.append(&include);
            }

            locations.insert(location);
        }

        // A type an imported data type document declares is not declared here, so it reaches the
        // interface as a name that answers to nothing in this registry. When every declared type
        // does answer, no type of the imported document is in use.
        if (mUnresolvedSeen == false)
        {
            for (const IncludeEntry* include : dataTypeDocuments)
            {
                add(include->getId(), eDocElementKind::Include, eSeverity::Warning, SIValidator::RULE_UNUSED_IMPORT
                   , vtr("Data types are imported from '%1', but the interface uses none of them").arg(include->getLocation()));
            }
        }
    }

    void Ctx::checkUnreferenced()
    {
        // A data type is used when an attribute, a method parameter, a constant or another data
        // type declares with it. Anything else in the registry generates code nothing can reach.
        for (DataTypeCustom* dataType : mData.getDataTypeData().getCustomDataTypes())
        {
            if ((dataType != nullptr) && (mTypesUsed.contains(dataType->getName()) == false))
            {
                add(dataType->getId(), eDocElementKind::DataType, eSeverity::Warning, SIValidator::RULE_UNREFERENCED
                   , vtr("Data type '%1' is never referenced").arg(dataType->getName()));
            }
        }

        for (const ConstantEntry& constant : mData.getConstantData().getElements())
        {
            if (mConstsUsed.contains(constant.getName()) == false)
            {
                add(constant.getId(), eDocElementKind::Constant, eSeverity::Info, SIValidator::RULE_UNREFERENCED
                   , vtr("Constant '%1' is never referenced").arg(constant.getName()));
            }
        }
    }

    QList<DocIssue> Ctx::run()
    {
        // The type-use record is filled by the declaration checks, so they all run before the
        // unreferenced pass reads it.
        checkOverview();
        checkDataTypes();
        checkAttributes();
        checkMethods();
        checkConstants();
        checkIncludes();

        // A default value written as a constant name is the one way a service interface reads a
        // constant, and it is the same for a parameter default, a field default and an attribute.
        for (const ConstantEntry& constant : mData.getConstantData().getElements())
        {
            mConstsUsed.insert(constant.getValue().trimmed());
        }

        for (MethodEntry* method : mData.getMethodData().getElements())
        {
            if (method == nullptr)
                continue;
            for (const MethodParameter& param : method->getElements())
            {
                mConstsUsed.insert(param.getValue().trimmed());
            }
        }

        for (DataTypeCustom* dataType : mData.getDataTypeData().getCustomDataTypes())
        {
            if ((dataType == nullptr) || (dataType->getCategory() != DataTypeBase::eCategory::Structure))
                continue;
            for (const FieldEntry& field : static_cast<DataTypeStructure*>(dataType)->getElements())
            {
                mConstsUsed.insert(field.getValue().trimmed());
            }
        }

        checkUnreferenced();
        return mIssues;
    }
}

QList<DocIssue> SIValidator::validate(const ServiceInterfaceData& data)
{
    Ctx ctx(data);
    return ctx.run();
}

eIssueField SIValidator::fieldOfRule(int rule)
{
    switch (rule)
    {
    case SIValidator::RULE_MISSING_NAME:
    case SIValidator::RULE_INVALID_IDENTIFIER:
    case SIValidator::RULE_DUPLICATE_NAME:
        return eIssueField::Name;

    case SIValidator::RULE_UNRESOLVED_TYPE:
        return eIssueField::Type;

    case SIValidator::RULE_BAD_LITERAL:
        return eIssueField::Value;

    case SIValidator::RULE_RESPONSE_LINK:
        return eIssueField::Link;

    default:
        return eIssueField::None;
    }
}

QString SIValidator::explainRule(int rule, DocIssue::eSeverity severity)
{
    if (rule > SIValidator::ADVISORY_RULE_BASE)
    {
        switch (rule - SIValidator::ADVISORY_RULE_BASE)
        {
        case SIValidator::RULE_EMPTY_TYPE:
            return QCoreApplication::translate("SIValidator", "The type generates an empty declaration. Give it its members, or remove it.");
        case SIValidator::RULE_UNREFERENCED:
            return QCoreApplication::translate("SIValidator", "Nothing in the interface uses this declaration. Keep it if you are about to, or remove it.");
        case SIValidator::RULE_UNBOUND_RESPONSE:
            return QCoreApplication::translate("SIValidator", "A response is what a request answers with. Connect it to the request it belongs to, or remove it.");
        case SIValidator::RULE_EMPTY_INTERFACE:
            return QCoreApplication::translate("SIValidator", "A client reaches an interface through its attributes and methods. Until there is one, there is nothing to generate.");
        case SIValidator::RULE_UNUSED_IMPORT:
            return QCoreApplication::translate("SIValidator", "Every type the interface declares with is either built in or declared here, so nothing comes from the imported document. Use a type from it, or remove the include.");
        default:
            return QCoreApplication::translate("SIValidator", "Advisory only. The interface still generates.");
        }
    }

    switch (rule)
    {
    case SIValidator::RULE_MISSING_NAME:
        return QCoreApplication::translate("SIValidator", "The generated code is named after this. It cannot be left empty.");
    case SIValidator::RULE_INVALID_IDENTIFIER:
        return QCoreApplication::translate("SIValidator", "Names must be usable in generated code: a letter or underscore first, then letters, digits or underscores.");
    case SIValidator::RULE_DUPLICATE_NAME:
        return QCoreApplication::translate("SIValidator", "Two declarations reach one generated name this way, and the build then refuses whichever comes second.");
    case SIValidator::RULE_UNRESOLVED_TYPE:
        return QCoreApplication::translate("SIValidator", "The declared type is not in the data type registry. Check the spelling, or declare the type on the Data Types page.");
    case SIValidator::RULE_BAD_LITERAL:
        return QCoreApplication::translate("SIValidator", "The value cannot be read as a value of the declared type.");
    case SIValidator::RULE_RESPONSE_LINK:
        return QCoreApplication::translate("SIValidator", "A caller waits for the named response. Declare it, or clear the connection so the request answers with nothing.");
    case SIValidator::RULE_DEFAULT_ORDER:
        return QCoreApplication::translate("SIValidator", "A caller may only leave out trailing arguments, so every parameter after a defaulted one needs a default too.");
    default:
        return (severity == DocIssue::eSeverity::Error)
                    ? QCoreApplication::translate("SIValidator", "The interface will not generate until this is resolved.")
                    : QString();
    }
}
