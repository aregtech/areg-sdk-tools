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

#include "lusan/model/common/DocRules.hpp"
#include "lusan/model/si/SIValidator.hpp"

#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/si/ServiceInterfaceData.hpp"
#include "lusan/data/common/MethodDataSection.hpp"
#include "lusan/model/common/DocRuleChecks.hpp"

#include <QCoreApplication>
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
            , mChecks   (mIssues, data.getDataTypeData())
            , mTypesUsed( )
            , mConstsUsed( )
        {
        }

        QList<DocIssue> run();

    private:
        void add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message);

        void checkName(uint32_t id, eDocElementKind kind, const QString& name, const QString& what);
        void checkType(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& what);
        void checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& literal, const QString& what);
        void checkParameters(const MethodEntry& method);

        void checkOverview();
        void checkUnknownElements();
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
        DocRuleChecks               mChecks;        //!< The rules every document kind shares.
        QSet<QString>               mTypesUsed;     //!< Type names something in the document declares with.
        QSet<QString>               mConstsUsed;    //!< Constant names something in the document reads.
    };

    void Ctx::add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message)
    {
        mChecks.add(id, kind, sev, rule, message, SIValidator::explainRule(mChecks.ruleId(rule, sev), sev));
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
        mChecks.checkIdentifier(id, kind, name, what);
    }

    void Ctx::checkType(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& what)
    {
        noteType(typeName);
        mChecks.checkDeclaredType(id, kind, typeName, what, true);
    }

    void Ctx::checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& literal, const QString& what)
    {
        mChecks.checkLiteral(id, kind, typeName, literal, what);
    }

    void Ctx::checkParameters(const MethodEntry& method)
    {
        const QString kindWord = methodKindWord(method);
        DocNameSet names(mChecks, eDocElementKind::Method);
        bool defaulted = false;
        for (const MethodParameter& param : method.getElements())
        {
            const QString where = vtr("Parameter '%1' of %2 '%3'").arg(param.getName(), kindWord.toLower(), method.getName());
            checkName(method.getId(), eDocElementKind::Method, param.getName(), where);
            checkType(method.getId(), eDocElementKind::Method, param.getType(), where);
            checkLiteral(method.getId(), eDocElementKind::Method, param.getType(), param.getValue(), where);
            names.claim(method.getId(), param.getName(), where);
            mChecks.noteDeprecatedElement(param, eDocElementKind::Method, where);

            // A caller may only leave out the trailing parameters, so once one carries a default
            // every parameter after it has to carry one too.
            if (param.hasDefault())
            {
                defaulted = true;
            }
            else if (defaulted)
            {
                add(method.getId(), eDocElementKind::Method, eSeverity::Error, DocRules::RULE_DEFAULT_ORDER
                   , vtr("Parameter '%1' of %2 '%3' has no default value, but an earlier parameter has one").arg(param.getName(), kindWord.toLower(), method.getName()));
            }
        }
    }

    void Ctx::checkUnknownElements()
    {
        mChecks.noteUnknownElements(eDocElementKind::Overview, DocRules::RULE_UNKNOWN_ELEMENT
                                  , mData.getUnknownElements(), QStringLiteral("siml"));
        mChecks.noteUnknownAttributes(eDocElementKind::Overview, DocRules::RULE_UNKNOWN_ATTRIBUTE
                                    , mData.getUnknownAttributes());
    }

    void Ctx::checkOverview()
    {
        const SIOverviewData& overview = mData.getOverviewData();
        checkName(overview.getId(), eDocElementKind::Overview, overview.getName(), vtr("The service interface"));
        mChecks.noteFileNameMismatch(overview.getId(), overview.getName(), mData.getFilePath(), DocRules::RULE_FILE_NAME_MISMATCH);

        if (overview.getVersion().isValid() == false)
        {
            mChecks.add(overview.getId(), eDocElementKind::Overview, eSeverity::Error, DocRules::RULE_MISSING_VERSION
                       , vtr("The service interface has no version"), SIValidator::explainRule(DocRules::RULE_MISSING_VERSION, eSeverity::Error));
        }

        if (overview.getIsDeprecated())
        {
            mChecks.noteDeprecated(overview.getId(), eDocElementKind::Overview
                                  , vtr("The service interface"), eSeverity::Warning, overview.getDeprecateHint());
        }

        // An interface a client cannot do anything with is worth saying out loud, but it is a
        // perfectly good starting point for a document being written.
        if (mData.getAttributeData().getElementCount() == 0 && mData.getMethodData().getElements().isEmpty())
        {
            add(overview.getId(), eDocElementKind::Overview, eSeverity::Warning, DocRules::RULE_EMPTY_DOCUMENT
               , vtr("The interface declares no attribute and no method, so it offers nothing to a client"));
        }
    }

    void Ctx::checkDataTypes()
    {
        DocNameSet names(mChecks, eDocElementKind::DataType);
        for (DataTypeCustom* dataType : mData.getDataTypeData().getCustomDataTypes())
        {
            if (dataType == nullptr)
                continue;

            const uint32_t id = dataType->getId();
            const QString name = dataType->getName();
            checkName(id, eDocElementKind::DataType, name, vtr("The data type"));
            names.claim(id, name, vtr("Data type '%1'").arg(name));

            if (dataType->getIsDeprecated())
            {
                mChecks.noteDeprecated(id, eDocElementKind::DataType, vtr("Data type '%1'").arg(name)
                                      , eSeverity::Info, dataType->getDeprecateHint());
            }

            if (dataType->getCategory() == DataTypeBase::eCategory::Structure)
            {
                DataTypeStructure* structType = static_cast<DataTypeStructure*>(dataType);
                DocNameSet fields(mChecks, eDocElementKind::DataType);
                for (const FieldEntry& field : structType->getElements())
                {
                    const QString where = vtr("Field '%1' of structure '%2'").arg(field.getName(), name);
                    checkName(id, eDocElementKind::DataType, field.getName(), where);
                    checkType(id, eDocElementKind::DataType, field.getType(), where);
                    checkLiteral(id, eDocElementKind::DataType, field.getType(), field.getValue(), where);
                    fields.claim(id, field.getName(), where);
                    mChecks.noteDeprecatedElement(field, eDocElementKind::DataType, where);
                }

                if (structType->getElementCount() == 0)
                {
                    add(id, eDocElementKind::DataType, eSeverity::Warning, DocRules::RULE_EMPTY_TYPE
                       , vtr("Structure '%1' has no fields").arg(name));
                }
            }
            else if (dataType->getCategory() == DataTypeBase::eCategory::Enumeration)
            {
                DataTypeEnum* enumType = static_cast<DataTypeEnum*>(dataType);
                DocNameSet fields(mChecks, eDocElementKind::DataType);
                for (const EnumEntry& field : enumType->getElements())
                {
                    const QString where = vtr("Value '%1' of enumeration '%2'").arg(field.getName(), name);
                    checkName(id, eDocElementKind::DataType, field.getName(), where);
                    fields.claim(id, field.getName(), where);
                    mChecks.noteDeprecatedElement(field, eDocElementKind::DataType, where);
                }

                mChecks.checkEnumeratorValues(eDocElementKind::DataType, name, enumType->getElements());

                if (enumType->getElementCount() == 0)
                {
                    add(id, eDocElementKind::DataType, eSeverity::Warning, DocRules::RULE_EMPTY_TYPE
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
        DocNameSet names(mChecks, eDocElementKind::Attribute);
        for (const AttributeEntry& attribute : mData.getAttributeData().getElements())
        {
            const uint32_t id = attribute.getId();
            const QString name = attribute.getName();
            const QString where = vtr("Attribute '%1'").arg(name);
            checkName(id, eDocElementKind::Attribute, name, vtr("The attribute"));
            checkType(id, eDocElementKind::Attribute, attribute.getType(), where);
            names.claim(id, name, where);
            mChecks.noteDeprecatedElement(attribute, eDocElementKind::Attribute, where);
        }
    }

    void Ctx::checkMethods()
    {
        // Names are unique per method kind: a request, a response and a broadcast may share one
        // name, but two requests may not.
        DocNameSet names(mChecks, eDocElementKind::Method);
        for (MethodEntry* method : mData.getMethodData().getElements())
        {
            if (method == nullptr)
                continue;

            const uint32_t id = method->getId();
            const QString name = method->getName();
            const QString kindWord = methodKindWord(*method);
            checkName(id, eDocElementKind::Method, name, vtr("The %1").arg(kindWord.toLower()));
            checkParameters(*method);
            names.claimKeyed(id, QString::number(method->getKind()) + QLatin1Char(':') + name
                            , vtr("%1 '%2'").arg(kindWord, name));
            mChecks.noteDeprecatedElement(*method, eDocElementKind::Method
                                         , vtr("%1 '%2'").arg(kindWord, name));
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
                add(request->getId(), eDocElementKind::Method, eSeverity::Error, DocRules::RULE_RESPONSE_LINK
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
                add(response->getId(), eDocElementKind::Method, eSeverity::Warning, DocRules::RULE_UNBOUND_RESPONSE
                   , vtr("Response '%1' is not the answer to any request").arg(response->getName()));
            }
        }
    }

    void Ctx::checkConstants()
    {
        DocNameSet names(mChecks, eDocElementKind::Constant);
        for (const ConstantEntry& constant : mData.getConstantData().getElements())
        {
            const uint32_t id = constant.getId();
            const QString name = constant.getName();
            const QString where = vtr("Constant '%1'").arg(name);
            checkName(id, eDocElementKind::Constant, name, vtr("The constant"));
            checkType(id, eDocElementKind::Constant, constant.getType(), where);
            checkLiteral(id, eDocElementKind::Constant, constant.getType(), constant.getValue(), where);
            names.claim(id, name, where);
            mChecks.noteDeprecatedElement(constant, eDocElementKind::Constant, where);
        }
    }

    void Ctx::checkIncludes()
    {
        QSet<QString> locations;
        for (const IncludeEntry& include : mData.getIncludeData().getElements())
        {
            const QString location = include.getLocation();
            if (location.isEmpty())
            {
                mChecks.add(include.getId(), eDocElementKind::Include, eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
                           , vtr("An include row names no file"), DocRuleChecks::explainShape(DocRuleChecks::eShape::MissingName));
            }
            else if (locations.contains(location))
            {
                // An include is keyed by its location, so the duplicate rule reads the path here.
                mChecks.add(include.getId(), eDocElementKind::Include, eSeverity::Warning, DocRules::RULE_DUPLICATE_NAME
                           , vtr("'%1' is included more than once").arg(location)
                           , DocRuleChecks::explainShape(DocRuleChecks::eShape::DuplicateName));
            }

            locations.insert(location);
            mChecks.noteDeprecatedElement(include, eDocElementKind::Include
                                         , vtr("Include '%1'").arg(location));
        }

        // The type-use record is complete by now, so an imported document that contributes
        // nothing is known rather than guessed at.
        mChecks.checkImportedDocuments(eDocElementKind::Include, DocRules::RULE_BROKEN_IMPORT);
        mChecks.noteUnusedImports(eDocElementKind::Include, DocRules::RULE_UNREFERENCED, mTypesUsed);
    }

    void Ctx::checkUnreferenced()
    {
        // A data type is used when an attribute, a method parameter, a constant or another data
        // type declares with it. Anything else in the registry generates code nothing can reach.
        for (DataTypeCustom* dataType : mData.getDataTypeData().getCustomDataTypes())
        {
            if ((dataType != nullptr) && (mTypesUsed.contains(dataType->getName()) == false))
            {
                mChecks.noteUnreferenced(dataType->getId(), eDocElementKind::DataType
                                        , vtr("Data type '%1'").arg(dataType->getName()), eSeverity::Warning);
            }
        }

    }

    QList<DocIssue> Ctx::run()
    {
        // The type-use record is filled by the declaration checks, so they all run before the
        // unreferenced pass reads it.
        checkUnknownElements();
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
    case DocRules::RULE_INVALID_IDENTIFIER:
    case DocRules::RULE_DUPLICATE_NAME:
    case DocRuleChecks::INFORMATION_RULE_BASE + DocRules::RULE_FILE_NAME_MISMATCH:
        return eIssueField::Name;

    case DocRules::RULE_UNRESOLVED_TYPE:
        return eIssueField::Type;

    case DocRules::RULE_BAD_LITERAL:
    case DocRules::RULE_DUPLICATE_ENUM_VALUE:
        return eIssueField::Value;

    case DocRules::RULE_RESPONSE_LINK:
        return eIssueField::Link;

    default:
        return eIssueField::None;
    }
}

QString SIValidator::explainRule(int rule, DocIssue::eSeverity severity)
{
    if (DocRuleChecks::isBanded(rule))
    {
        switch (DocRuleChecks::bareRule(rule))
        {
        case DocRules::RULE_EMPTY_TYPE:
            return QCoreApplication::translate("SIValidator", "The type generates an empty declaration. Give it its members, or remove it.");
        case DocRules::RULE_UNREFERENCED:
            return DocRuleChecks::explainShape(DocRuleChecks::eShape::Unreferenced);
        case DocRules::RULE_UNBOUND_RESPONSE:
            return QCoreApplication::translate("SIValidator", "A response is what a request answers with. Connect it to the request it belongs to, or remove it.");
        case DocRules::RULE_EMPTY_DOCUMENT:
            return QCoreApplication::translate("SIValidator", "A client reaches an interface through its attributes and methods. Until there is one, there is nothing to generate.");
        case DocRules::RULE_DEPRECATED:
            return DocRuleChecks::explainShape(DocRuleChecks::eShape::Deprecated);
        default:
            return QCoreApplication::translate("SIValidator", "Advisory only. The interface still generates.");
        }
    }

    switch (rule)
    {
    case DocRules::RULE_INVALID_IDENTIFIER:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::InvalidIdentifier);
    case DocRules::RULE_DUPLICATE_NAME:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::DuplicateName);
    case DocRules::RULE_UNRESOLVED_TYPE:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::UnresolvedType);
    case DocRules::RULE_BAD_LITERAL:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::BadLiteral);
    case DocRules::RULE_DUPLICATE_ENUM_VALUE:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::DuplicateEnumValue);
    case DocRules::RULE_RESPONSE_LINK:
        return QCoreApplication::translate("SIValidator", "A caller waits for the named response. Declare it, or clear the connection so the request answers with nothing.");
    case DocRules::RULE_MISSING_VERSION:
        return QCoreApplication::translate("SIValidator", "The version is generated into the interface and tells a client which contract it was built against. Give the document one.");
    case DocRules::RULE_DEFAULT_ORDER:
        return QCoreApplication::translate("SIValidator", "A caller may only leave out trailing arguments, so every parameter after a defaulted one needs a default too.");
    case DocRules::RULE_BROKEN_IMPORT:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::BrokenImport);
    case DocRules::RULE_UNKNOWN_ELEMENT:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::UnknownElement);
    default:
        return (severity == DocIssue::eSeverity::Error)
                    ? QCoreApplication::translate("SIValidator", "The interface will not generate until this is resolved.")
                    : QString();
    }
}
