#ifndef LUSAN_MODEL_COMMON_DOCRULECHECKS_HPP
#define LUSAN_MODEL_COMMON_DOCRULECHECKS_HPP
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
 *  \file        lusan/model/common/DocRuleChecks.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the validation rules every document kind shares.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/EnumEntry.hpp"
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/model/common/DocUnknownScan.hpp"
#include "lusan/model/common/DocRules.hpp"

#include <QCoreApplication>
#include <QList>
#include <QSet>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeDataSection;

/**
 * \class   DocRuleChecks
 * \brief   The checks that ask the same question of every document: is this name usable in
 *          generated code, is it taken already, does this declared type exist, does this value
 *          read as its type, and does anything use this declaration.
 *
 *          A document engine holds one of these, tells it which rule number it files each shape
 *          under, and calls it instead of carrying its own copy. The number stays the engine's,
 *          because the number is what an author reads and what the code generator files the same
 *          fault under; the wording and the explanation come from here, so one defect never
 *          reaches the results panel described two ways.
 **/
class DocRuleChecks
{
    Q_DECLARE_TR_FUNCTIONS(DocRuleChecks)

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   The shared shapes, used to look up the one explanation each of them has.
     **/
    enum class eShape
    {
          MissingName
        , InvalidIdentifier
        , DuplicateName
        , UnresolvedType
        , BadLiteral
        , Unreferenced
        , DuplicateEnumValue
        , Deprecated
        , BrokenImport
        , UnusedImport
        , FileNameMismatch
        , UnknownElement
        , RetiredElement
        , UnknownAttribute
        , DroppedElement
    };

    /**
     * \brief   The rule id of a warning: rule `n` reported at warning severity is `100 + n`.
     *          A band, not a second severity of the same rule, so `4` and `104` are two
     *          unrelated rules.
     **/
    static constexpr int WARNING_RULE_BASE      { 100 };

    /**
     * \brief   The rule id of an information finding: rule `n` reported at information
     *          severity is `200 + n`. Held apart from the warning band so a finding that
     *          blocks nothing never enters the error and warning comparison.
     **/
    static constexpr int INFORMATION_RULE_BASE  { 200 };

    /**
     * \brief   The lowest rule id of any banded finding. An id below it is an error.
     **/
    static constexpr int LOWEST_BANDED_RULE     { WARNING_RULE_BASE };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the checks to the finding list they append to and to the registry every
     *          type question is answered against.
     **/
    DocRuleChecks(QList<DocIssue>& issues, const DataTypeDataSection& types);

//////////////////////////////////////////////////////////////////////////
// Operations, engine independent answers
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   True when the text is a name the generated code can carry.
     **/
    static bool isIdentifier(const QString& name);

    /**
     * \brief   Why a finding of the given shape is a finding, and what resolves it.
     **/
    static QString explainShape(eShape shape);

    /**
     * \brief   Why the literal does not fit the declared type, or an empty string when it does.
     *          An enumeration takes its own enumerators, a structure and a container take no
     *          literal at all, an imported type is opaque and takes anything, and everything
     *          else is parsed against the type's syntax.
     **/
    static QString literalReason(const DataTypeDataSection& types, const QString& typeName, const QString& literal);

    /**
     * \brief   True when the name fragment is a predefined type, a declared one, or not a plain
     *          name at all -- this is a registry lookup, not a C++ parser.
     **/
    bool typeResolves(const QString& fragment) const;

    /**
     * \brief   The first name in a declared type that answers to nothing, or an empty string.
     *          A templated type is checked fragment by fragment, so `Array<Missing>` reports
     *          `Missing` rather than the whole string.
     **/
    QString unresolvedFragment(const QString& typeName) const;

    /**
     * \brief   The rule id a finding of this severity carries: the bare number for an error,
     *          the warning band for a warning, the information band for information.
     **/
    int ruleId(int rule, DocIssue::eSeverity severity) const;

    /**
     * \brief   The rule number behind a finding's id, with the band of its severity removed.
     * \param   ruleId  The id a finding carries.
     **/
    static int bareRule(int ruleId);

    /**
     * \brief   True when the id carries a band, which is to say the finding is not an error.
     * \param   ruleId  The id a finding carries.
     **/
    static bool isBanded(int ruleId);

//////////////////////////////////////////////////////////////////////////
// Operations, the shared rules
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Appends a finding, applying the advisory band to the rule number.
     **/
    void add(uint32_t id, eDocElementKind kind, DocIssue::eSeverity severity, int rule
            , const QString& message, const QString& detail);

    /**
     * \brief   The name has to be there and has to be usable in generated code.
     * \param   what    The subject of the message, such as `Attribute 'speed'`. Used when the
     *                  name is missing, which is the case a name cannot describe itself.
     **/
    void checkIdentifier(uint32_t id, eDocElementKind kind, const QString& name, const QString& what);

    /**
     * \brief   The declared type has to exist, and every fragment of a templated one with it.
     * \param   required    True when the declaration cannot be left without a type.
     * \return  The fragment that answered to nothing, or an empty string.
     **/
    QString checkDeclaredType(uint32_t id, eDocElementKind kind, const QString& typeName
                             , const QString& what, bool required);

    /**
     * \brief   The value has to read as a value of the declared type.
     * \param   what    The subject of the message. Empty where the call site has none, and the
     *                  message then names the type instead.
     **/
    void checkLiteral(uint32_t id, eDocElementKind kind, const QString& typeName
                     , const QString& literal, const QString& what);

    /**
     * \brief   Files a name that is already taken.
     * \param   subject The whole subject, such as `Attribute 'speed'`.
     **/
    void reportDuplicate(uint32_t id, eDocElementKind kind, const QString& subject
                        , DocIssue::eSeverity severity = DocIssue::eSeverity::Error);

    /**
     * \brief   Files a declaration nothing in the document uses.
     * \param   message An alternative wording, for a declaration that is used in one named way.
     **/
    void noteUnreferenced(uint32_t id, eDocElementKind kind, const QString& subject
                         , DocIssue::eSeverity severity, const QString& message = QString());

    /**
     * \brief   Two enumerators of one enumeration have to count differently, otherwise a value
     *          read back cannot be told apart from the other one.
     *
     *          Enumerators without a value of their own count on from the one before, as they do
     *          in C++, so `{ one, two = 0 }` collides just as `{ one = 1, two = 1 }` does. An
     *          enumerator whose value is not a plain number -- a constant name, say -- is left
     *          out, and so is everything counting on from it, because what it counts is not
     *          knowable here.
     * \param   typeName    The enumeration, for the message.
     * \param   entries     Its enumerators, in declaration order.
     **/
    void checkEnumeratorValues(eDocElementKind kind, const QString& typeName, const QList<EnumEntry>& entries);

    /**
     * \brief   Notes a declaration its author marked deprecated, so what still uses it is worth
     *          a second look.
     * \param   subject The whole subject, such as `Data type 'Unit'`.
     * \param   hint    The author's own note about what to use instead. Left out when empty.
     **/
    void noteDeprecated(uint32_t id, eDocElementKind kind, const QString& subject
                       , DocIssue::eSeverity severity, const QString& hint = QString());

    /**
     * \brief   Notes one declaration inside the document when its author marked it deprecated,
     *          and does nothing otherwise. Reported as information: the document around it is
     *          current, and only this one declaration is not.
     * \param   subject The whole subject, such as `Attribute 'LegacyFlag'`.
     * \param   marked  Whether the declaration carries the mark.
     * \param   hint    The author's own note about what to use instead. Left out when empty.
     **/
    template<typename Element>
    inline void noteDeprecatedElement(const Element& element, eDocElementKind kind, const QString& subject);

    /**
     * \brief   Every data type document the host includes has to lead to a file that reads as
     *          one, and no two of them may carry one namespace: the namespace is the file's base
     *          name, and two files of that name generate one namespace twice.
     * \param   kind    The element kind a finding on an include row carries.
     * \param   rule    The number this engine files a broken import under.
     **/
    void checkImportedDocuments(eDocElementKind kind, int rule);

    /**
     * \brief   Notes an included data type document the host takes no type from. Including one
     *          costs a generated include and a namespace, so an unused one is worth removing.
     * \param   typesUsed   Every type name the document declares with, qualified ones included.
     * \param   rule        The number this engine files an unused import under.
     **/
    void noteUnusedImports(eDocElementKind kind, int rule, const QSet<QString>& typesUsed);

    /**
     * \brief   Notes that the document declares one name and lives in a file called another.
     *          Both are allowed -- the generated files follow the declared name -- but the two
     *          drifting apart is worth saying once. Silent for a document not saved yet.
     * \param   id          The overview element the finding points at.
     * \param   name        The name the document declares.
     * \param   filePath    The file the document was read from or written to.
     * \param   rule        The number this engine files the mismatch under.
     **/
    void noteFileNameMismatch(uint32_t id, const QString& name, const QString& filePath, int rule);

    /**
     * \brief   Files every element of the opened file that the format does not place, whether
     *          the tag is unknown or sits somewhere the format does not allow. The block itself
     *          survives the save, so the finding is what tells the author it is there.
     * \param   kind    The element kind these findings carry; they point at no element of their own.
     * \param   rule    The number this engine files an unknown element under.
     * \param   unknown The blocks the read could not place, in document order.
     **/
    void noteUnknownElements(eDocElementKind kind, int rule, const QList<DocUnknownElement>& unknown
                            , const QString& document = QString());

    /**
     * \brief   Files every attribute of the opened file that the format does not define on an
     *          element it does define. Nothing reads the value and the save drops it, so the
     *          finding is what tells the author it is going.
     * \param   kind    The element kind these findings carry; they point at no element of their own.
     * \param   rule    The number this engine files an unknown attribute under.
     * \param   unknown The attributes the read could not place, in document order.
     **/
    void noteUnknownAttributes(eDocElementKind kind, int rule, const QList<DocUnknownAttribute>& unknown);

    /**
     * \brief   Returns the retired element matching the given tag, or nullptr when the format
     *          never defined it there and the tag is simply unknown.
     * \param   tag         The element as the document spells it.
     * \param   parent      The element it was read inside.
     * \param   document    The document extension, empty to match any.
     **/
    static const DocRules::Retired* retiredElement(const QString& tag, const QString& parent
                                                  , const QString& document);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
private:
    QList<DocIssue>&            mIssues;    //!< The run's findings.
    const DataTypeDataSection&  mTypes;     //!< The document's data type registry.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DocRuleChecks(void) = delete;
    DocRuleChecks(const DocRuleChecks& /*src*/) = delete;
    DocRuleChecks& operator = (const DocRuleChecks& /*src*/) = delete;
};

/**
 * \class   DocNameSet
 * \brief   One name space of a document: the names already taken in it, and the finding filed
 *          on the second declaration that claims one. A section has one of these; a section
 *          whose names are unique per kind keys them by kind and still needs only one.
 **/
class DocNameSet
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    DocNameSet(DocRuleChecks& checks, eDocElementKind kind
              , DocIssue::eSeverity severity = DocIssue::eSeverity::Error);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Claims the name for the element. Files the duplicate rule and returns false when
     *          the name was claimed before.
     * \param   subject The whole subject of the message, such as `Attribute 'speed'`.
     **/
    bool claim(uint32_t id, const QString& name, const QString& subject);

    /**
     * \brief   The same, for a name space keyed by something the message does not show, such as
     *          the kind a method was declared as.
     **/
    bool claimKeyed(uint32_t id, const QString& key, const QString& subject);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
private:
    DocRuleChecks&      mChecks;
    eDocElementKind     mKind;
    DocIssue::eSeverity mSeverity;
    QSet<QString>       mTaken;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DocNameSet(void) = delete;
    DocNameSet(const DocNameSet& /*src*/) = delete;
    DocNameSet& operator = (const DocNameSet& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// DocRuleChecks inline methods
//////////////////////////////////////////////////////////////////////////

template<typename Element>
inline void DocRuleChecks::noteDeprecatedElement(const Element& element, eDocElementKind kind, const QString& subject)
{
    if (element.getIsDeprecated())
    {
        noteDeprecated(element.getId(), kind, subject, DocIssue::eSeverity::Info, element.getDeprecateHint());
    }
}

#endif  // LUSAN_MODEL_COMMON_DOCRULECHECKS_HPP
