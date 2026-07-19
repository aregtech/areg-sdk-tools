#ifndef LUSAN_MODEL_SM_SMSYMBOLINDEX_HPP
#define LUSAN_MODEL_SM_SMSYMBOLINDEX_HPP
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
 *  \file        lusan/model/sm/SMSymbolIndex.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM verbatim-code symbol assistance (SM-21-06).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \namespace   SMSymbolIndex
 * \brief   Symbol-aware assistance for the verbatim code cells (expression rows, lambda
 *          blocks, embedded condition bodies, inline code). It never parses C++: it only
 *          (1) offers the machine's own in-scope identifiers as a completion list, and
 *          (2) scans a verbatim block for the known identifiers it references, so
 *          validation (SM-24/25) can flag a reference to a deleted element and rename
 *          (SM-26) can find-and-replace into verbatim text. The token surface stays
 *          opaque -- only whole-word identifier tokens that match a declared element name
 *          are recognized; keywords, unknown tokens, literals, and member accesses are
 *          ignored.
 **/
namespace SMSymbolIndex
{
    /**
     * \enum    eSymbolKind
     * \brief   The registry kind of a known machine symbol (for categorization / icons).
     **/
    enum class eSymbolKind
    {
          Attribute     //!< A machine attribute (referenced by its generated getter).
        , Constant      //!< A declared constant.
        , Method        //!< A declared method (trigger / action / condition).
        , Param         //!< An in-scope stimulus parameter (transition-scoped cells only).
    };

    /**
     * \struct  KnownSymbol
     * \brief   One declared identifier the machine exposes to verbatim code.
     **/
    struct KnownSymbol
    {
        QString     name;   //!< The element name -- the token a verbatim block references.
        eSymbolKind kind;   //!< The registry kind.
    };

    /**
     * \struct  VerbatimRef
     * \brief   One verbatim block that references a queried identifier.
     **/
    struct VerbatimRef
    {
        uint32_t    elementId;  //!< The owning element ID (condition row, method, operation).
        QString     location;   //!< A human-readable description of the block.
    };

    /**
     * \struct  VerbatimBlock
     * \brief   One verbatim code block found anywhere in the document.
     **/
    struct VerbatimBlock
    {
        uint32_t    elementId;  //!< The owning element ID.
        QString     text;       //!< The verbatim body / expression text.
        QString     location;   //!< A human-readable description of the block.
    };

    /**
     * \brief   The machine-wide known symbols: every attribute, constant and method.
     *          Stimulus parameters are scope-specific and are not included here.
     **/
    QList<KnownSymbol> machineSymbols(const StateMachineData& data);

    /**
     * \brief   The set of machine-wide known identifier names (attributes, constants,
     *          methods). Used as the recognition universe for the token scan.
     **/
    QSet<QString> machineSymbolNames(const StateMachineData& data);

    /**
     * \brief   The in-scope stimulus parameter names of a transition (the trigger's or
     *          event's payload). Empty for a Timer stimulus or an unresolved stimulus.
     **/
    QStringList stimulusParams(const StateMachineData& data, uint32_t transitionId);

    /**
     * \brief   The completion words offered inside a verbatim code cell: attribute getters
     *          (`Name()`), method calls (`Name()`), constant names, and -- when the cell is
     *          transition-scoped (\p includeParams) -- the stimulus parameters (`name`).
     *          De-duplicated and sorted for a stable completer model.
     * \param   data            The document model.
     * \param   transitionId    The transition owning the cell (0 = none / not scoped).
     * \param   includeParams   True for condition/operation cells (Param scope), false
     *                          for embedded condition-method bodies and free code.
     **/
    QStringList completionWords(const StateMachineData& data, uint32_t transitionId, bool includeParams);

    /**
     * \brief   Scans verbatim \p text for the identifiers it references, keeping only
     *          tokens that match a name in \p knownNames. Comments, string/char literals
     *          and member accesses (`obj.name`, `p->name`) are ignored. The result is in
     *          first-appearance order with duplicates removed.
     **/
    QStringList usedSymbols(const QString& text, const QSet<QString>& knownNames);

    /**
     * \brief   True if verbatim \p text references the identifier \p name as a whole-word
     *          token (outside comments / literals / member accesses).
     **/
    bool referencesSymbol(const QString& text, const QString& name);

    /**
     * \brief   Every verbatim code block in the document (condition expression/lambda rows,
     *          embedded condition-method bodies, inline-code and expression operations), in
     *          document order.
     **/
    QList<VerbatimBlock> collectVerbatimBlocks(const StateMachineData& data);

    /**
     * \brief   Every verbatim block that references the identifier \p name. The answer
     *          rename (SM-26) and validation (SM-24/25) consume so a deleted or renamed
     *          element can be resolved to the blocks that still mention it.
     **/
    QList<VerbatimRef> findReferences(const StateMachineData& data, const QString& name);
}

#endif  // LUSAN_MODEL_SM_SMSYMBOLINDEX_HPP
