#ifndef LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP
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
 *  \file        lusan/model/sm/SMLayoutCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM layout mutation commands.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"

#include <QList>
#include <QRectF>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMStateEntry;
class SMTransitionEntry;

/**
 * \class   SMMoveNodeCommand
 * \brief   Moves/resizes a state node. Interactive drags push one command per step; they
 *          coalesce via mergeWith() keyed by a gesture ID so the whole drag is a single
 *          undo step and the model is untouched until the gesture is applied.
 **/
class SMMoveNodeCommand : public SMCommand
{
public:
    static constexpr int CMD_ID { 0x5401 };

    /**
     * \brief   Returns a fresh gesture ID. All move sources share this counter so two
     *          different gestures can never coalesce into one undo step.
     **/
    static uint32_t takeNextGesture();

    SMMoveNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t owner, uint32_t gestureId
                      , double x, double y, double width, double height
                      , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyTo(const SMLayoutNode& geometry);

private:
    uint32_t        mOwner;
    uint32_t        mGesture;
    SMLayoutNode    mNew;
    SMLayoutNode    mOld;
    bool            mCaptured { false };
};

/**
 * \class   SMAttachNodeCommand
 * \brief   Creates the Node layout entry of a freshly added state. The owner ID is read
 *          from the state at redo time, because the ID is allocated only when the sibling
 *          add-state command inserts the entry - so this command always runs as the child
 *          following that insertion inside one composite.
 **/
class SMAttachNodeCommand : public SMCommand
{
public:
    /**
     * \brief   Creates the command.
     * \param   data        The document root.
     * \param   notifier    The change-notification hub.
     * \param   state       The state whose Node entry is created; must outlive the command.
     * \param   geometry    The initial box geometry in scene coordinates.
     * \param   text        The undo-stack display text.
     * \param   parent      The owning composite command.
     **/
    SMAttachNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                        , const SMStateEntry& state, const QRectF& geometry
                        , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    const SMStateEntry& mState;     //!< The state owning the Node entry.
    QRectF              mGeometry;  //!< The initial box geometry.
};

/**
 * \class   SMSetNodeExpandedCommand
 * \brief   Toggles a state node's expanded/collapsed flag (body visibility).
 **/
class SMSetNodeExpandedCommand : public SMCommand
{
public:
    SMSetNodeExpandedCommand(  StateMachineData& data, DocModelNotifier& notifier
                             , uint32_t owner, bool expanded
                             , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(bool expanded);

private:
    uint32_t    mOwner;                 //!< The owning state ID.
    bool        mNew;                   //!< The expanded flag to apply.
    bool        mOldExpanded { true };  //!< The previous expanded flag.
    bool        mOldHas      { false }; //!< Whether the flag was present before.
    bool        mCaptured    { false };
};

/**
 * \class   SMAttachEdgeCommand
 * \brief   Creates the Edge layout entry of a freshly added transition. The owner ID is
 *          read from the transition at redo time, because the ID is allocated only when the
 *          sibling add-transition command inserts the entry - so this command always runs as
 *          the child following that insertion inside one composite.
 **/
class SMAttachEdgeCommand : public SMCommand
{
public:
    SMAttachEdgeCommand(  StateMachineData& data, DocModelNotifier& notifier
                        , const SMTransitionEntry& transition, const SMLayoutEdge& geometry
                        , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    const SMTransitionEntry&    mTransition;    //!< The transition owning the Edge entry.
    SMLayoutEdge                mGeometry;      //!< The initial edge geometry (owner set at redo).
};

/**
 * \class   SMSetEdgeGeometryCommand
 * \brief   Sets a transition's Edge layout (points, shape, bulge, color, label). Interactive
 *          waypoint/endpoint/label drags push one command per step; they coalesce via
 *          mergeWith() keyed by a gesture ID so the whole drag is a single undo step.
 **/
class SMSetEdgeGeometryCommand : public SMCommand
{
public:
    static constexpr int CMD_ID { 0x5402 };

    SMSetEdgeGeometryCommand(  StateMachineData& data, DocModelNotifier& notifier
                             , uint32_t owner, uint32_t gestureId, const SMLayoutEdge& geometry
                             , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyTo(const SMLayoutEdge& geometry, bool present);

private:
    uint32_t        mOwner;
    uint32_t        mGesture;
    SMLayoutEdge    mNew;
    SMLayoutEdge    mOld;
    bool            mHadOld { false };  //!< Whether an Edge entry existed before the first redo.
    bool            mCaptured { false };
};

/**
 * \class   SMSetViewCommand
 * \brief   Sets a machine level's View layout entry: the zoom and the scene point at the
 *          viewport center. Continuous viewport changes push one command per update; they
 *          coalesce via mergeWith() keyed by a gesture ID, so one visit of a level stays a
 *          single undo step. Undo removes the entry again when none existed before.
 **/
class SMSetViewCommand : public SMCommand
{
public:
    static constexpr int CMD_ID { 0x5403 };

    SMSetViewCommand(  StateMachineData& data, DocModelNotifier& notifier
                     , uint32_t owner, uint32_t gestureId, const SMLayoutView& view
                     , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyTo(const SMLayoutView& view, bool present);

private:
    uint32_t        mOwner;
    uint32_t        mGesture;
    SMLayoutView    mNew;
    SMLayoutView    mOld;
    bool            mHadOld   { false };    //!< Whether a View entry existed before the first redo.
    bool            mCaptured { false };
};

/**
 * \class   SMAutoPlaceNodesCommand
 * \brief   Adds the Node entries a document is missing, computed once when it is loaded.
 *          Missing layout is legal in the file format, so the placement is applied as an
 *          ordinary undoable edit: the user can undo it, and saving makes it permanent.
 **/
class SMAutoPlaceNodesCommand : public SMCommand
{
public:
    /**
     * \brief   Creates the command.
     * \param   data        The document root.
     * \param   notifier    The change-notification hub.
     * \param   nodes       The Node entries to add; each owner must have no entry yet.
     * \param   text        The undo-stack display text.
     * \param   parent      The owning composite command, if any.
     **/
    SMAutoPlaceNodesCommand(  StateMachineData& data, DocModelNotifier& notifier
                            , const QList<SMLayoutNode>& nodes
                            , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<SMLayoutNode> mNodes;     //!< The placed Node entries.
    QList<uint32_t>     mIds;       //!< The owners of the placed entries.
};

/**
 * \class   SMRemoveLayoutCommand
 * \brief   Removes the View/Node/Edge layout entries owned by a set of element IDs, and the
 *          notes drawn on any of those IDs (a deleted composite's sublevel), restoring all of
 *          them on undo. Used as a child of the delete-state composite so a logical element
 *          and its layout are deleted (and undeleted) in one step.
 **/
class SMRemoveLayoutCommand : public SMCommand
{
public:
    SMRemoveLayoutCommand(StateMachineData& data, DocModelNotifier& notifier, const QList<uint32_t>& ownerIds, const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<uint32_t>         mIds;
    QList<SMLayoutView>     mViews;
    QList<SMLayoutNode>     mNodes;
    QList<SMLayoutEdge>     mEdges;
    QList<SMLayoutNote>     mNotes;
    bool                    mCaptured { false };
};

/**
 * \class   SMSetGridSizeCommand
 * \brief   Sets the document's grid cell size (`Layout@GridSize`).
 **/
class SMSetGridSizeCommand : public SMCommand
{
public:
    SMSetGridSizeCommand(StateMachineData& data, DocModelNotifier& notifier, int gridSize, const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    int     mNew;
    int     mOld { 0 };
};

/**
 * \class   SMSetGridVisibleCommand
 * \brief   Sets the document's grid visibility (`Layout@GridVisible`).
 **/
class SMSetGridVisibleCommand : public SMCommand
{
public:
    SMSetGridVisibleCommand(StateMachineData& data, DocModelNotifier& notifier, bool visible, const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    bool    mNew;
    bool    mOld { true };
};

/**
 * \class   SMSetNodeColorCommand
 * \brief   Sets (or clears, with an empty string) a state's body color (`Node@Color`); the
 *          header shade stays derived unless the state also has a `HeaderColor`.
 **/
class SMSetNodeColorCommand : public SMCommand
{
public:
    SMSetNodeColorCommand(  StateMachineData& data, DocModelNotifier& notifier
                          , uint32_t owner, const QString& color
                          , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t    mOwner;
    QString     mNew;
    QString     mOld;
};

/**
 * \class   SMSetEdgeColorCommand
 * \brief   Sets (or clears, with an empty string) a transition edge's display color
 *          (`Edge@Color`); analogous to SMSetNodeColorCommand.
 **/
class SMSetEdgeColorCommand : public SMCommand
{
public:
    SMSetEdgeColorCommand(  StateMachineData& data, DocModelNotifier& notifier
                          , uint32_t owner, const QString& color
                          , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t    mOwner;
    QString     mNew;
    QString     mOld;
};

/**
 * \class   SMAddNoteCommand
 * \brief   Creates a diagram note: allocates its document-unique ID on first
 *          redo and re-inserts it under that exact ID on later redo (history navigation
 *          never allocates new IDs).
 **/
class SMAddNoteCommand : public SMCommand
{
public:
    SMAddNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t level, const QRectF& geometry, const QString& text
                      , const QString& undoText, QUndoCommand* parent = nullptr);

    /**
     * \brief   Creates a note bound to a state/transition owner (0 for a free note). A
     *          bound note is drawn as a badge on its owner, not as a free canvas box.
     **/
    SMAddNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t level, uint32_t owner, const QRectF& geometry, const QString& text
                      , const QString& undoText, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

    /**
     * \brief   The created note's element ID; valid after the first redo (the constructor
     *          runs before the initial push).
     **/
    inline uint32_t getNoteId() const;

private:
    SMLayoutNote    mNote;
    bool            mAllocated { false };
};

/**
 * \class   SMMoveNoteCommand
 * \brief   Moves/resizes a note. Interactive drags push one command per step; they coalesce
 *          via mergeWith() keyed by a gesture ID (shared with SMMoveNodeCommand's counter) so
 *          the whole drag is a single undo step.
 **/
class SMMoveNoteCommand : public SMCommand
{
public:
    static constexpr int CMD_ID { 0x5404 };

    SMMoveNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t noteId, uint32_t gestureId
                      , double x, double y, double width, double height
                      , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyTo(double x, double y, double width, double height);

private:
    uint32_t    mNoteId;
    uint32_t    mGesture;
    double      mNewX, mNewY, mNewWidth, mNewHeight;
    double      mOldX { 0.0 }, mOldY { 0.0 }, mOldWidth { 0.0 }, mOldHeight { 0.0 };
    bool        mCaptured { false };
};

/**
 * \class   SMSetNoteTextCommand
 * \brief   Sets a note's text; committed once when the inline editor loses focus.
 **/
class SMSetNoteTextCommand : public SMCommand
{
public:
    SMSetNoteTextCommand(  StateMachineData& data, DocModelNotifier& notifier
                         , uint32_t noteId, const QString& text
                         , const QString& undoText, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t    mNoteId;
    QString     mNew;
    QString     mOld;
};

/**
 * \class   SMSetNoteColorCommand
 * \brief   Sets (or clears, with an empty string) a note's display color.
 **/
class SMSetNoteColorCommand : public SMCommand
{
public:
    SMSetNoteColorCommand(  StateMachineData& data, DocModelNotifier& notifier
                          , uint32_t noteId, const QString& color
                          , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t    mNoteId;
    QString     mNew;
    QString     mOld;
};

/**
 * \class   SMRemoveNoteCommand
 * \brief   Removes one note, restoring it under its original ID on undo.
 **/
class SMRemoveNoteCommand : public SMCommand
{
public:
    SMRemoveNoteCommand(  StateMachineData& data, DocModelNotifier& notifier
                        , uint32_t noteId, const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    uint32_t        mNoteId;
    SMLayoutNote    mCaptured;
};

//////////////////////////////////////////////////////////////////////////
// SMAddNoteCommand inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMAddNoteCommand::getNoteId() const
{
    return mNote.id;
}

#endif  // LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP
