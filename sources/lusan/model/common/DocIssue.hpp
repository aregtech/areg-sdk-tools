#ifndef LUSAN_MODEL_COMMON_DOCISSUE_HPP
#define LUSAN_MODEL_COMMON_DOCISSUE_HPP
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
 *  \file        lusan/model/common/DocIssue.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the one validation finding type shared by every document.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocModelNotifier.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/**
 * \enum    eIssueField
 * \brief   Which editable field of an element a finding blames, when the check knows that much.
 *          A results view navigates to the element by id; this says what to put the caret on
 *          once it is there, so the author lands on the thing they have to change instead of
 *          hunting for it across the form. \a None means the element itself is the whole answer.
 **/
enum class eIssueField
{
      None          //!< Nothing finer than the element; selecting it is the reveal.
    , Name          //!< The name field.
    , Type          //!< The data type field.
    , Value         //!< The literal value field.
    , Description   //!< The description field.
};

/**
 * \struct  DocIssue
 * \brief   One validation finding, in the single vocabulary every checker speaks.
 *
 *          Before this type each checker had its own result struct and its own severity
 *          enum, and a results widget reconciled them by hand -- which put the merge in a
 *          view, let the vocabularies drift, and left checkers that returned a bare severity
 *          unable to say what was wrong. Every check now yields this, so a finding can be
 *          shown, counted, sorted and navigated without knowing which check produced it.
 **/
struct DocIssue
{
    /**
     * \enum    eSeverity
     * \brief   Finding severity, ordered so a worst-of comparison is a simple `max`.
     **/
    enum class eSeverity
    {
          Info      //!< Advisory.
        , Warning   //!< Should fix.
        , Error     //!< Must fix; blocks code generation.
    };

    uint32_t        elementId { 0 };                        //!< The offending element (navigation target).
    eDocElementKind kind      { eDocElementKind::State };   //!< The element kind (page routing).
    eSeverity       severity  { eSeverity::Error };         //!< The severity.
    int             rule      { 0 };                        //!< The identifier of the failed check.
    QString         message;                                //!< What is wrong.
    QString         location;                               //!< Where, in the document's own words.
    QString         detail;                                 //!< Why that is wrong, and what resolves it.
};

/**
 * \class   IDocValidator
 * \brief   What every document validator is: something that names a document and yields its
 *          findings. A results view holds these, not concrete validators, so one panel can
 *          list a State Machine, a Service Interface and a data-type document side by side.
 *
 *          Implementations stay headless -- no widget or scheduling dependency -- so a code
 *          generator can run the same checks the editor runs.
 **/
class IDocValidator
{
public:
    virtual ~IDocValidator() = default;

    /**
     * \brief   The document's display name, used as the root label when findings from
     *          several open documents are listed together.
     **/
    virtual QString documentName() const = 0;

    /**
     * \brief   Every finding of the document, worst-first ordering not required (the
     *          consumer sorts).
     **/
    virtual QList<DocIssue> validate() const = 0;
};

/**
 * \brief   The worst of two severities.
 **/
inline DocIssue::eSeverity worstOf(DocIssue::eSeverity left, DocIssue::eSeverity right)
{
    return (static_cast<int>(left) >= static_cast<int>(right)) ? left : right;
}

/**
 * \brief   The worst severity in a list of findings; \p fallback when the list is empty.
 **/
inline DocIssue::eSeverity worstOf(const QList<DocIssue>& issues, DocIssue::eSeverity fallback)
{
    DocIssue::eSeverity worst = fallback;
    for (const DocIssue& issue : issues)
    {
        worst = worstOf(worst, issue.severity);
    }

    return worst;
}

#endif  // LUSAN_MODEL_COMMON_DOCISSUE_HPP
