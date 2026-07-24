#ifndef LUSAN_DATA_SM_SMCLIPBOARD_HPP
#define LUSAN_DATA_SM_SMCLIPBOARD_HPP
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
 *  \file        lusan/data/sm/SMClipboard.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM copy/paste clipboard payload:
 *               XML (de)serialization of selected elements and the subtree
 *               walkers used to re-allocate IDs and remap references on paste.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/ElementBase.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMIncludeData.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTimerData.hpp"

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \class   SMClipboardContent
 * \brief   The parsed clipboard payload: the copied states (full subtrees), their
 *          layout entries and notes, the explicitly copied registry entries, and the
 *          registry entries the copied states reference. All elements are parented to
 *          an internal scratch root, so parsing never touches any document's ID counter;
 *          the paste command re-allocates every ID from the target document.
 **/
class SMClipboardContent
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \class   ClipRoot
     * \brief   A standalone ID-counter root for the parsed elements.
     **/
    class ClipRoot : public ElementBase
    {
    public:
        ClipRoot();
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMClipboardContent();
    ~SMClipboardContent() = default;

    SMClipboardContent(const SMClipboardContent& /*src*/) = delete;
    SMClipboardContent& operator = (const SMClipboardContent& /*src*/) = delete;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   True when the payload carries nothing to paste.
     **/
    bool isEmpty() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    ClipRoot        mRoot;              //!< The scratch ID-counter root of all parsed elements.

public:
    SMStateData     mStates;            //!< The copied top-level states with their subtrees.
    SMLayoutData    mLayout;            //!< The Node/Edge entries of the copied set and the copied notes.
    SMDataTypeData  mDataTypes;         //!< Explicitly copied data types.
    SMAttributeData mAttributes;        //!< Explicitly copied attributes.
    SMEventData     mEvents;            //!< Explicitly copied events.
    SMTimerData     mTimers;            //!< Explicitly copied timers.
    SMMethodData    mMethods;           //!< Explicitly copied methods.
    SMConstantData  mConstants;         //!< Explicitly copied constants.
    SMIncludeData   mIncludes;          //!< Explicitly copied includes.
    SMAttributeData mRefAttributes;     //!< Attributes referenced by the copied states.
    SMEventData     mRefEvents;         //!< Events referenced by the copied states.
    SMTimerData     mRefTimers;         //!< Timers referenced by the copied states.
    SMMethodData    mRefMethods;        //!< Methods referenced by the copied states.
    SMConstantData  mRefConstants;      //!< Constants referenced by the copied states.
};

//////////////////////////////////////////////////////////////////////////
// SMClipboard class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMClipboard
 * \brief   Serializes a selection of FSM elements into a self-contained XML clipboard
 *          payload and parses it back. Also provides the state-subtree walkers the
 *          paste command uses: ID re-allocation, name collection, and reference
 *          remapping after collision renames.
 **/
class SMClipboard
{
public:
    //!< The clipboard MIME format of the FSM element payload.
    static constexpr const char* const  MIME_TYPE   { "application/x-lusan-fsml-elements" };

    /**
     * \struct  ReferenceNames
     * \brief   The registry names a copied state subtree references, per registry.
     **/
    struct ReferenceNames
    {
        QSet<QString>   events;
        QSet<QString>   timers;
        QSet<QString>   methods;
        QSet<QString>   attributes;
        QSet<QString>   constants;
    };

    /**
     * \struct  RenameMaps
     * \brief   The collision renames (original name -> pasted name) applied on paste,
     *          per referenced name space. Used to remap references inside the pasted
     *          subtrees only.
     **/
    struct RenameMaps
    {
        QHash<QString, QString> states;
        QHash<QString, QString> events;
        QHash<QString, QString> timers;
        QHash<QString, QString> methods;
        QHash<QString, QString> attributes;
        QHash<QString, QString> constants;
    };

public:
    /**
     * \brief   Serializes the given elements of a document into the clipboard XML.
     *          The IDs may name states (Start states and states nested inside another
     *          selected state are skipped), notes, and registry entries; any other ID
     *          is ignored. The payload also carries the layout of the copied set and
     *          the registry entries the copied states reference.
     * \param   doc         The source document.
     * \param   elementIds  The selected element IDs.
     * \return  The XML text, or an empty string when nothing is copyable.
     **/
    static QString serialize(const StateMachineData& doc, const QList<uint32_t>& elementIds);

    /**
     * \brief   Parses a clipboard XML payload produced by serialize().
     * \param   xml     The XML text.
     * \return  The parsed content, or nullptr when the payload is not recognized.
     **/
    static std::unique_ptr<SMClipboardContent> parse(const QString& xml);

    /**
     * \brief   Re-allocates the ID of every element in the state subtree (the state,
     *          its operations and their arguments, transitions, conditions, and nested
     *          states, in document order) from the given counter root.
     * \param   state       The subtree root; its parent chain must already reach \p counter.
     * \param   counter     The ID-counter root (the target document).
     * \param   oldToNew    Receives the old ID -> new ID mapping of every walked element.
     **/
    static void reallocateIds(SMStateEntry& state, const ElementBase& counter, QHash<uint32_t, uint32_t>& oldToNew);

    /**
     * \brief   Collects every state of the subtree, the root included, in document order.
     **/
    static void collectStates(SMStateEntry& state, QList<SMStateEntry*>& out);

    /**
     * \brief   Collects the IDs owning layout entries in the subtree: every state
     *          and every transition, the root state included.
     **/
    static void collectOwnedIds(const SMStateEntry& state, QList<uint32_t>& out);

    /**
     * \brief   Collects the registry names referenced anywhere in the state subtree:
     *          stimuli, OnFinal events, operation targets, condition operands, and
     *          argument value sources.
     **/
    static void collectReferences(const SMStateEntry& state, ReferenceNames& out);

    /**
     * \brief   Rewrites every reference inside the state subtree according to the
     *          rename maps: transition targets and stimuli, OnFinal events, operation
     *          targets, condition operands, and argument value sources. Names not in
     *          the maps are left untouched.
     **/
    static void remapReferences(SMStateEntry& state, const RenameMaps& renames);

    /**
     * \brief   Remaps the transition targets inside the state subtree by the old-ID -> new-ID
     *          map produced when the pasted subtree's IDs are re-allocated. A target that
     *          names a state inside the pasted set is re-pointed to that state's new ID; a
     *          target outside the pasted set (not in the map) is cleared to internal, since
     *          its original state does not belong to the paste.
     * \param   state       The subtree root whose transitions are remapped.
     * \param   oldToNew    The old element ID -> new element ID map of the pasted set.
     **/
    static void remapTransitionTargets(SMStateEntry& state, const QHash<uint32_t, uint32_t>& oldToNew);

    /**
     * \brief   True when two registry entries are structurally identical: their
     *          serialized XML matches with the element IDs ignored. Used by the
     *          cross-document merge to decide reuse versus ID-suffixed copy.
     **/
    static bool structurallyEqual(const DocumentElem& lhs, const DocumentElem& rhs);
};

#endif  // LUSAN_DATA_SM_SMCLIPBOARD_HPP
