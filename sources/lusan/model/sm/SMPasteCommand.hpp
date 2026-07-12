#ifndef LUSAN_MODEL_SM_SMPASTECOMMAND_HPP
#define LUSAN_MODEL_SM_SMPASTECOMMAND_HPP
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
 *  \file        lusan/model/sm/SMPasteCommand.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM paste command: inserts a parsed clipboard
 *               payload as one undo step with fresh IDs and collision renames.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/data/sm/SMClipboard.hpp"

#include <QHash>
#include <QList>
#include <QPointF>
#include <memory>

/**
 * \class   SMPasteCommand
 * \brief   Pastes a clipboard payload into a machine level as one undo step. On the
 *          first redo every element receives a fresh ID from the target document, name
 *          collisions are resolved by suffixing the new ID (`Name_27`), references
 *          inside the pasted subtrees are remapped, referenced registry entries are
 *          merged by name (an identical entry is reused, a conflicting one becomes an
 *          ID-suffixed copy), and the layout is inserted with a position offset.
 *          Undo removes everything; later redo re-inserts the identical elements under
 *          their recorded IDs, so history navigation never allocates.
 **/
class SMPasteCommand : public SMCommand
{
//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \class   RegistryEntryBase
     * \brief   One planned registry insertion; the typed template lives in the source file.
     **/
    class RegistryEntryBase;

private:
    /**
     * \struct  PastedState
     * \brief   One pasted top-level state and its replay bookkeeping.
     **/
    struct PastedState
    {
        SMStateEntry*   state   { nullptr };    //!< The subtree; owned while out of the model.
        bool            owned   { true };       //!< True when the command owns the subtree.
        uint32_t        oldId   { 0 };          //!< The clipboard (source) ID.
        uint32_t        newId   { 0 };          //!< The target-document ID, set on first redo.
        QList<uint32_t> oldTxIds;               //!< The state's direct transition IDs (source).
        QList<uint32_t> newTxIds;               //!< The same transitions' target-document IDs.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the command, taking the payload apart and resolving the registry
     *          merge plan against the target document.
     * \param   data            The target document root.
     * \param   notifier        The change-notification hub.
     * \param   content         The parsed clipboard payload; consumed.
     * \param   targetLevelId   The machine level receiving the states and notes.
     * \param   offset          The position offset applied to the pasted top-level boxes.
     * \param   text            The undo-stack display text.
     * \param   parent          The owning composite command, if any.
     **/
    SMPasteCommand(  StateMachineData& data, DocModelNotifier& notifier
                   , std::unique_ptr<SMClipboardContent> content
                   , uint32_t targetLevelId, const QPointF& offset
                   , const QString& text, QUndoCommand* parent = nullptr);

    virtual ~SMPasteCommand() override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   True when the payload carries something to paste and the target level
     *          exists (a payload of registry entries alone needs no level).
     **/
    bool isEffective() const;

    /**
     * \brief   The pasted top-level element IDs (states, free notes, registry entries),
     *          for selection; valid after the first redo (push).
     **/
    inline const QList<uint32_t>& getPastedIds() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual void redo() override;
    virtual void undo() override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Resolves the registry merge plan against the target document: reuse an
     *          identical referenced entry, otherwise plan an insertion, marked for an
     *          ID-suffix rename when the name is taken in its name space.
     **/
    void buildRegistryPlan(SMClipboardContent& content);

    /**
     * \brief   First-redo work after the registry insertions: allocates every subtree
     *          and note ID, applies the state collision renames, remaps references,
     *          and builds the final layout entries with the paste offset.
     **/
    void allocateContent(SMStateData& level);

    /**
     * \brief   Inserts the recorded layout entries into the document.
     **/
    void insertLayout();

    /**
     * \brief   Emits the added notifications for states, level transitions and notes.
     **/
    void notifyInserted();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<RegistryEntryBase*>   mRegistry;      //!< The planned registry insertions, in plan order.
    QList<PastedState>          mStates;        //!< The pasted top-level states.
    QList<SMLayoutNode>         mSrcNodes;      //!< Clipboard Node entries (source-keyed).
    QList<SMLayoutEdge>         mSrcEdges;      //!< Clipboard Edge entries (source-keyed).
    QList<SMLayoutNote>         mSrcNotes;      //!< Clipboard notes (source-keyed).
    QList<SMLayoutNode>         mNodes;         //!< Final Node entries, built on first redo.
    QList<SMLayoutEdge>         mEdges;         //!< Final Edge entries, built on first redo.
    QList<SMLayoutNote>         mNotes;         //!< Final notes with allocated IDs.
    QHash<uint32_t, uint32_t>   mOldToNew;      //!< Source ID -> target ID of every pasted element.
    QList<uint32_t>             mPastedIds;     //!< The top-level pasted IDs, for selection.
    uint32_t                    mTargetLevel;   //!< The receiving machine level's owner ID.
    QPointF                     mOffset;        //!< The paste position offset.
    bool                        mAllocated;     //!< True once the first redo allocated IDs.
};

//////////////////////////////////////////////////////////////////////////
// SMPasteCommand inline methods
//////////////////////////////////////////////////////////////////////////

inline const QList<uint32_t>& SMPasteCommand::getPastedIds() const
{
    return mPastedIds;
}

#endif  // LUSAN_MODEL_SM_SMPASTECOMMAND_HPP
