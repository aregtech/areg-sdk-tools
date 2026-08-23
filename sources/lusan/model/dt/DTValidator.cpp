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
 *  \file        lusan/model/dt/DTValidator.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document validation engine.
 *
 ************************************************************************/

#include "lusan/model/common/DocRules.hpp"
#include "lusan/model/dt/DTValidator.hpp"

#include "lusan/data/common/DataTypeContainer.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/common/DataTypeStructure.hpp"
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/data/common/FieldEntry.hpp"
#include "lusan/data/common/IncludeEntry.hpp"
#include "lusan/data/dt/DataTypeDocumentData.hpp"

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
        return QCoreApplication::translate("DTValidator", text);
    }

    /**
     * \class   Ctx
     * \brief   One validation run: the document, the findings it produced, and the registry
     *          lookups the rules share.
     **/
    class Ctx
    {
    public:
        explicit Ctx(const DataTypeDocumentData& data)
            : mData     (data)
            , mIssues   ( )
            , mChecks   (mIssues, data.getDataTypeData())
            , mTypesUsed( )
        {
        }

        QList<DocIssue> run(void);

    private:
        void add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message);
        void checkType(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& what);

        void checkOverview(void);
        void checkUnknownElements(void);
        void checkDataTypes(void);
        void checkIncludes(void);
        void checkUnreferenced(void);

        //!< Records that a declared type name is in use, element types of a template included.
        void noteType(const QString& typeName);

    private:
        const DataTypeDocumentData& mData;
        QList<DocIssue>             mIssues;
        DocRuleChecks               mChecks;    //!< The rules every document kind shares.
        QSet<QString>               mTypesUsed; //!< Type names something in the document declares with.
    };

    void Ctx::add(uint32_t id, eDocElementKind kind, eSeverity sev, int rule, const QString& message)
    {
        mChecks.add(id, kind, sev, rule, message, DTValidator::explainRule(mChecks.ruleId(rule, sev), sev));
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

    void Ctx::checkType(uint32_t id, eDocElementKind kind, const QString& typeName, const QString& what)
    {
        noteType(typeName);
        mChecks.checkDeclaredType(id, kind, typeName, what, true);
    }

    void Ctx::checkOverview(void)
    {
        const OverviewDataSection& overview = mData.getOverviewData();
        const QString name = overview.getName();

        // The name becomes the namespace the generated types sit in, so it has to be a name C++
        // can spell. The file it lives in is free to be called something else.
        if (name.isEmpty())
        {
            mChecks.add(overview.getId(), eDocElementKind::Overview, eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
                       , vtr("The data type document has no name"), DocRuleChecks::explainShape(DocRuleChecks::eShape::MissingName));
        }
        else if (DocRuleChecks::isIdentifier(name) == false)
        {
            add(overview.getId(), eDocElementKind::Overview, eSeverity::Error, DocRules::RULE_INVALID_IDENTIFIER
               , vtr("'%1' cannot be a namespace, so the document has to be renamed").arg(name));
        }

        mChecks.noteFileNameMismatch(overview.getId(), name, mData.getFilePath(), DocRules::RULE_FILE_NAME_MISMATCH);

        if (overview.getVersion().isValid() == false)
        {
            mChecks.add(overview.getId(), eDocElementKind::Overview, eSeverity::Error, DocRules::RULE_MISSING_VERSION
                       , vtr("The data type document has no version")
                       , DTValidator::explainRule(DocRules::RULE_MISSING_VERSION, eSeverity::Error));
        }

        if (overview.getIsDeprecated())
        {
            mChecks.noteDeprecated(overview.getId(), eDocElementKind::Overview
                                  , vtr("The data type document"), eSeverity::Warning, overview.getDeprecateHint());
        }

        if (mData.getDataTypeData().getCustomDataTypes().isEmpty())
        {
            add(overview.getId(), eDocElementKind::Overview, eSeverity::Warning, DocRules::RULE_EMPTY_DOCUMENT
               , vtr("The document declares no data type, so nothing that includes it gains anything"));
        }
    }

    void Ctx::checkDataTypes(void)
    {
        DocNameSet names(mChecks, eDocElementKind::DataType);
        for (DataTypeCustom* dataType : mData.getDataTypeData().getCustomDataTypes())
        {
            if (dataType == nullptr)
                continue;

            const uint32_t id = dataType->getId();
            const QString name = dataType->getName();
            mChecks.checkIdentifier(id, eDocElementKind::DataType, name, vtr("The data type"));
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
                    mChecks.checkIdentifier(id, eDocElementKind::DataType, field.getName(), where);
                    checkType(id, eDocElementKind::DataType, field.getType(), where);
                    mChecks.checkLiteral(id, eDocElementKind::DataType, field.getType(), field.getValue(), where);
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
                    mChecks.checkIdentifier(id, eDocElementKind::DataType, field.getName(), where);
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

    void Ctx::checkIncludes(void)
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

            // A data type document is a leaf: it takes the C++ headers its types need and
            // nothing else, so a document included here would reach the generated header as is.
            if (includeKindOf(location, QString()) == eIncludeKind::DataType)
            {
                add(include.getId(), eDocElementKind::Include, eSeverity::Error, DocRules::RULE_NOT_A_HEADER
                   , vtr("'%1' is a data type document, and a data type document includes only C++ headers").arg(location));
            }

            locations.insert(location);
            mChecks.noteDeprecatedElement(include, eDocElementKind::Include
                                         , vtr("Include '%1'").arg(location));
        }
    }

    void Ctx::checkUnreferenced(void)
    {
        // A data type of this document is used when another type here declares with it. Unlike an
        // interface, though, a data type document exists to be used from elsewhere, so a type
        // nothing here refers to is entirely normal and is not worth a word.
    }

    void Ctx::checkUnknownElements(void)
    {
        mChecks.noteUnknownElements(eDocElementKind::Overview, DocRules::RULE_UNKNOWN_ELEMENT
                                  , mData.getUnknownElements(), QStringLiteral("dtml"));
        mChecks.noteUnknownAttributes(eDocElementKind::Overview, DocRules::RULE_UNKNOWN_ATTRIBUTE
                                    , mData.getUnknownAttributes());
    }

    QList<DocIssue> Ctx::run(void)
    {
        checkUnknownElements();
        checkOverview();
        checkDataTypes();
        checkIncludes();
        checkUnreferenced();
        return mIssues;
    }
}

QList<DocIssue> DTValidator::validate(const DataTypeDocumentData& data)
{
    Ctx ctx(data);
    return ctx.run();
}

eIssueField DTValidator::fieldOfRule(int rule)
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

    default:
        return eIssueField::None;
    }
}

QString DTValidator::explainRule(int rule, DocIssue::eSeverity severity)
{
    if (DocRuleChecks::isBanded(rule))
    {
        switch (DocRuleChecks::bareRule(rule))
        {
        case DocRules::RULE_EMPTY_TYPE:
            return QCoreApplication::translate("DTValidator", "The type generates an empty declaration. Give it its members, or remove it.");
        case DocRules::RULE_UNREFERENCED:
            return DocRuleChecks::explainShape(DocRuleChecks::eShape::Unreferenced);
        case DocRules::RULE_DEPRECATED:
            return DocRuleChecks::explainShape(DocRuleChecks::eShape::Deprecated);
        case DocRules::RULE_EMPTY_DOCUMENT:
            return QCoreApplication::translate("DTValidator", "A data type document exists to be included by others. Until it declares a type, including it gains nothing.");
        default:
            return QCoreApplication::translate("DTValidator", "Advisory only. The document still generates.");
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
    case DocRules::RULE_NOT_A_HEADER:
        return QCoreApplication::translate("DTValidator", "Data types are shared by including this document, not by chaining one into another. Move the types you need in here, or include the header they come from.");
    case DocRules::RULE_MISSING_VERSION:
        return QCoreApplication::translate("DTValidator", "The version is generated into the code and tells a client which contract it was built against. Give the document one.");
    case DocRules::RULE_UNKNOWN_ELEMENT:
        return DocRuleChecks::explainShape(DocRuleChecks::eShape::UnknownElement);
    default:
        return (severity == DocIssue::eSeverity::Error)
                    ? QCoreApplication::translate("DTValidator", "The document will not generate until this is resolved.")
                    : QString();
    }
}
